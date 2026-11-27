import io
import json
import os
import re
import shutil
import subprocess
import tempfile
import tomllib
from importlib.resources import as_file, files
from importlib.resources.abc import Traversable
from pathlib import Path
from typing import Any

import pandas as pd

import pygambit as gbt

from ._catalog_fetch import CatalogError, fetch, is_release

READERS = {
    ".nfg": gbt.read_nfg,
    ".efg": gbt.read_efg,
}

_REPO_ROOT = Path(__file__).resolve().parent.parent.parent
_REPO_PIN = _REPO_ROOT / "build_support" / "catalog.toml"

_MANIFEST_SCHEMA_VERSION = 0
_ENDPOINTS: dict[str, str] = {}
# A path relative to the games directory, as the catalog's manifest schema defines it.
_RELPATH = re.compile(r"[A-Za-z0-9_.-]+(/[A-Za-z0-9_.-]+)*")


def _games_dir(path: Path) -> Path:
    """Return the games directory of the catalog at *path*, a catalog root or games directory."""
    if (path / "games" / "manifest.json").is_file():
        return path / "games"
    if (path / "manifest.json").is_file():
        return path
    raise CatalogError(f"{path} holds no games catalog (no games/manifest.json or manifest.json)")


def _load_manifest(root: Traversable) -> Any:
    return json.loads(root.joinpath("manifest.json").read_text(encoding="utf-8"))


def _locate(ref: str, cache_dir: str | os.PathLike | None) -> tuple[str, Traversable]:
    """Return the key and games directory of the catalog named by *ref*."""
    if is_release(ref):
        try:
            installed = _installed_root()
            if _load_manifest(installed).get("catalog_version") == ref:
                return ref, installed
        except (AttributeError, ValueError, OSError):
            pass
    key, unpacked = fetch(ref, None if cache_dir is None else Path(cache_dir), **_ENDPOINTS)
    return key, unpacked / "games"


def _read_manifest(root: Traversable, name: str) -> dict:
    """Read the manifest in *root* and check it is one this pygambit understands."""
    try:
        manifest = _load_manifest(root)
    except (ValueError, OSError) as e:
        raise CatalogError(f"The manifest of {name} cannot be read: {e}") from None
    if not isinstance(manifest, dict):
        raise CatalogError(f"The manifest of {name} is not a JSON object")
    schema = manifest.get("schema_version")
    if type(schema) is not int:
        raise CatalogError(f"The manifest of {name} has no integer 'schema_version'")
    # Checked before anything else, so that a catalog in a newer format gets this message.
    if schema != _MANIFEST_SCHEMA_VERSION:
        raise CatalogError(
            f"The manifest of {name} has schema version {schema}, but this version of "
            f"pygambit reads schema version {_MANIFEST_SCHEMA_VERSION}; use a different "
            "catalog or upgrade pygambit"
        )
    version = manifest.get("catalog_version")
    if not isinstance(version, str) or not is_release(version):
        raise CatalogError(f"The manifest of {name} has no 'catalog_version' like '1.2.3'")
    if not isinstance(manifest.get("games"), list):
        raise CatalogError(f"The manifest of {name} has no list of 'games'")
    for entry in manifest["games"]:
        if not _is_valid_entry(entry):
            raise CatalogError(f"The manifest of {name} has an invalid game entry: {entry!r}")
    return manifest


def _is_valid_entry(entry: Any) -> bool:
    """Return whether *entry* names a game file this pygambit can read."""
    if not isinstance(entry, dict):
        return False
    slug, game, fmt = entry.get("slug"), entry.get("game"), entry.get("format")
    return (
        isinstance(slug, str)
        and _RELPATH.fullmatch(slug) is not None
        and isinstance(game, str)
        and _RELPATH.fullmatch(game) is not None
        and ".." not in game.split("/")
        and f".{fmt}" in READERS
        and game.endswith(f".{fmt}")
    )


def _catalog_root() -> Traversable:
    """Return the games directory of the catalog in use.

    In order: the directory named by the environment variable ``GAMBIT_CATALOG_DIR``;
    the catalog bundled with pygambit; the pinned release fetched into a source checkout.
    """
    env = os.environ.get("GAMBIT_CATALOG_DIR")
    if env:
        return Path(env)
    return _installed_root()


def _installed_root() -> Traversable:
    """Return the games directory of the release installed with pygambit.

    That is the catalog bundled with pygambit or, in a source checkout, the pinned release
    fetched into it.
    """
    bundled = files("pygambit") / "catalog_data"
    if bundled.joinpath("manifest.json").is_file():
        return bundled
    if _REPO_PIN.is_file():
        with open(_REPO_PIN, "rb") as f:
            version = tomllib.load(f)["version"]
        fetched = _REPO_ROOT / ".catalog" / version / f"gambit-catalog-{version}" / "games"
        if (fetched / "manifest.json").is_file():
            return fetched
        raise FileNotFoundError(
            f"The games catalog {version} has not been fetched into this source checkout; "
            "run 'python build_support/catalog_release.py fetch'"
        )
    raise FileNotFoundError("No games catalog is installed with pygambit")


def _manifest() -> tuple[Traversable, dict]:
    """Return the games directory of the default catalog and its manifest."""
    catalog = Catalog()
    return catalog.path, catalog._manifest


def version() -> str:
    """Return the version of the default games catalog.

    Returns
    -------
    str
        The catalog's version, e.g. ``"0.2.0"``.
    """
    return Catalog()._manifest["catalog_version"]


def _read(root: Traversable, entry: dict) -> gbt.Game:
    """Read the game file of the manifest *entry*."""
    resource = root.joinpath(*entry["game"].split("/"))
    with as_file(resource) as path:
        return READERS[path.suffix](str(path))


class Catalog:
    """
    A version of the games catalog.

    A release version or a full commit SHA always gives the same games, so passing one makes
    results reproducible.  A branch, tag or short SHA gives whatever commit it names when the
    catalog is created; record the full SHA it resolves to, `ref`, and pass that to reproduce
    the result.  By default the catalog is the release bundled with pygambit, which can
    change when pygambit is upgraded.

    Parameters
    ----------
    ref : str, optional
        A release version of the catalog (e.g. ``"0.2.0"``), or a commit of its repository:
        a full SHA, or a branch, tag or short SHA, which is resolved to the full SHA.
        Catalogs not bundled with pygambit are downloaded once and kept in a cache.
    path : str or pathlib.Path, optional
        A local directory to read the catalog from instead, without caching.  It is either
        the root of a catalog checkout or release, or its ``games`` directory.
        Cannot be combined with `ref`.
    cache_dir : str or pathlib.Path, optional
        Where downloaded catalogs are kept.  Defaults to a per-user cache directory.

    Attributes
    ----------
    ref : str or None
        The release version or full commit SHA of the catalog; ``None`` for a catalog read
        from a local directory.
    path : pathlib.Path
        The directory holding the catalog's games.

    Raises
    ------
    CatalogError
        If the catalog cannot be fetched or read, or is not in a format this version of
        pygambit supports.
    ValueError
        If both `ref` and `path` are given.
    """

    ref: str | None

    def __init__(
        self,
        ref: str | None = None,
        *,
        path: str | os.PathLike | None = None,
        cache_dir: str | os.PathLike | None = None,
    ):
        if ref is not None and path is not None:
            raise ValueError("Give either a ref or a path to a catalog, not both")
        if path is not None:
            self.ref = None
            self.path = _games_dir(Path(path))
            name = f"the games catalog at {self.path}"
        elif ref is not None:
            self.ref, self.path = _locate(ref, cache_dir)
            name = f"the games catalog {ref!r}"
        else:
            self.ref = None
            self.path = _catalog_root()
            name = f"the games catalog at {self.path}"
        self._manifest = _read_manifest(self.path, name)
        if ref is None and path is None and not os.environ.get("GAMBIT_CATALOG_DIR"):
            version = self._manifest["catalog_version"]
            self.ref = version if isinstance(version, str) and is_release(version) else None

    def __repr__(self) -> str:
        if self.ref is not None:
            return f"Catalog({self.ref!r})"
        return f"Catalog(path={str(self.path)!r})"

    def load(self, slug: str) -> gbt.Game:
        """
        Load a game from the catalog.

        Parameters
        ----------
        slug : str
            The slug of the game to load.

        Returns
        -------
        gbt.Game
            The loaded game.

        Raises
        ------
        FileNotFoundError
            If the game does not exist in the catalog.
        """
        slug = str(Path(slug)).replace("\\", "/")
        for entry in self._manifest["games"]:
            if entry["slug"] == slug:
                return _read(self.path, entry)
        raise FileNotFoundError(f"No catalog entry called {slug}")

    def games(
        self,
        n_actions: int | None = None,
        n_contingencies: int | None = None,
        n_infosets: int | None = None,
        is_const_sum: bool | None = None,
        is_perfect_recall: bool | None = None,
        is_tree: bool | None = None,
        min_payoff: float | None = None,
        max_payoff: float | None = None,
        n_nodes: int | None = None,
        n_outcomes: int | None = None,
        n_players: int | None = None,
        n_strategies: int | None = None,
        include_descriptions: bool = False,
    ) -> pd.DataFrame:
        """
        List games available in the catalog.

        Most arguments are treated as filters on the
        attributes of the Game objects.

        Parameters
        ----------
        n_actions: int, optional
            The number of actions in the game. Only extensive games are returned.
        n_contingencies: int, optional
            The number of contingencies in the game.
        n_infosets: int, optional
            The number of information sets in the game. Only extensive games are returned.
        is_const_sum: bool, optional
            Whether the game is constant-sum.
        is_perfect_recall: bool, optional
            Whether the game has perfect recall.
        is_tree: bool, optional
            Whether the game is an extensive game (a tree).
        min_payoff: float, optional
            The minimum payoff in the game. Games returned have `min_payoff >= value`.
        max_payoff: float, optional
            The maximum payoff in the game. Games returned have `max_payoff <= value`.
        n_nodes: int, optional
            The number of nodes in the game. Only extensive games are returned.
        n_outcomes: int, optional
            The number of outcomes in the game.  Only extensive and strategic games, which
            have outcomes, are returned.
        n_players: int, optional
            The number of players in the game.
        n_strategies: int, optional
            The number of pure strategies in the game.
        include_descriptions: bool, optional
            Whether to include the description of each game in the returned DataFrame.
            Defaults to False.

        Returns
        -------
        pd.DataFrame
            A DataFrame with columns "Game" and "Title", where "Game" is the slug to load the game.
            If `include_descriptions=True`, the DataFrame will also include a "Description" column.
        """
        records: list[dict[str, Any]] = []

        def check_filters(game: gbt.Game) -> bool:
            if n_actions is not None:
                if not isinstance(game, gbt.ExtensiveGame):
                    return False
                n_game_actions = sum(
                    len(game.get_actions(gbt.H.path(*history.actions)))
                    for player in game.get_players()
                    for history in game.get_infosets(player)
                )
                if n_game_actions != n_actions:
                    return False
            if n_contingencies is not None and len(game.get_contingencies()) != n_contingencies:
                return False
            if n_infosets is not None:
                if not isinstance(game, gbt.ExtensiveGame):
                    return False
                if sum(len(game.get_infosets(p)) for p in game.get_players()) != n_infosets:
                    return False
            if is_const_sum is not None and game.is_const_sum() != is_const_sum:
                return False
            if is_perfect_recall is not None and game.is_perfect_recall() != is_perfect_recall:
                return False
            if is_tree is not None and isinstance(game, gbt.ExtensiveGame) != is_tree:
                return False
            if min_payoff is not None and game.get_min_payoff() < min_payoff:
                return False
            if max_payoff is not None and game.get_max_payoff() > max_payoff:
                return False
            if n_nodes is not None:
                if not isinstance(game, gbt.ExtensiveGame):
                    return False
                if len(game.get_histories(gbt.H.after())) != n_nodes:
                    return False
            if n_outcomes is not None and (
                not isinstance(game, (gbt.ExtensiveGame, gbt.StrategicGame))
                or len(game.get_outcomes()) != n_outcomes
            ):
                return False
            if n_players is not None and len(game.get_players()) != n_players:
                return False
            total_strategies = sum(len(game.get_strategies(p)) for p in game.get_players())
            return not (n_strategies is not None and total_strategies != n_strategies)

        def append_record(
            slug: str,
            game: gbt.Game,
            entry: dict,
        ) -> None:
            record = {
                "Game": slug,
                "Title": game.title,
            }
            if include_descriptions:
                record["Description"] = game.description
                record["Download"] = (
                    f":download:`{slug}.{entry['format']} <../games/{entry['game']}>`"
                )
                record["Format"] = entry["format"]
            records.append(record)

        for entry in sorted(self._manifest["games"], key=lambda e: e["slug"]):
            game = _read(self.path, entry)
            if check_filters(game):
                append_record(entry["slug"], game, entry)

        if include_descriptions:
            return pd.DataFrame.from_records(
                records, columns=["Game", "Title", "Description", "Download", "Format"]
            )
        return pd.DataFrame.from_records(records, columns=["Game", "Title"])


def generate_openspiel(game_name: str, params: dict | None = None) -> gbt.Game:
    """
    Generate a game using the OpenSpiel library.

    Parameters
    ----------
    game_name : str
        The short name of the OpenSpiel game (e.g. ``"matrix_rps"``,
        ``"tiny_hanabi"``). Passed directly to ``pyspiel.load_game``.
    params : dict, optional
        Game parameters forwarded to ``pyspiel.load_game``
        (e.g. ``{"players": 2, "coins": 3, "fields": 2}`` for ``"blotto"``).
        See the `OpenSpiel game list
        <https://openspiel.readthedocs.io/en/latest/games.html>`_ for
        available parameters per game.

    Returns
    -------
    gbt.Game
        The generated game.

    Raises
    ------
    ImportError
        If ``open_spiel`` is not installed.
    ValueError
        If the game's dynamics type is not supported for export, or if the
        format exporter raises an error for this specific game.
    Other exceptions from ``pyspiel.load_game`` propagate directly.
        For example, ``pyspiel.SpielError`` is raised for unknown game names
        or invalid/missing parameters.
    """
    try:
        import pyspiel
        from open_spiel.python.algorithms.gambit import export_gambit
    except ImportError as exc:
        raise ImportError(
            "open_spiel is required to load OpenSpiel games. "
            "Install it with: pip install open_spiel"
        ) from exc

    # Let pyspiel's own exceptions propagate unchanged — they already carry
    # informative messages ("Unknown game '...'", "Unknown parameter '...'", etc.)
    game = pyspiel.load_game(game_name, params or {})

    dynamics = game.get_type().dynamics

    # OpenSpiel's SEQUENTIAL corresponds to extensive-form (tree) games in Gambit;
    # SIMULTANEOUS corresponds to normal-form (strategic-form) games.
    if dynamics == pyspiel.GameType.Dynamics.SEQUENTIAL:
        try:
            efg_str = export_gambit(game)
        except Exception as exc:
            raise ValueError(
                f"OpenSpiel game '{game_name}' could not be exported to EFG format: {exc}"
            ) from exc
        return gbt.read_efg(io.StringIO(efg_str))

    elif dynamics == pyspiel.GameType.Dynamics.SIMULTANEOUS:
        try:
            nfg_str = pyspiel.game_to_nfg_string(game)
        except Exception as exc:
            raise ValueError(
                f"OpenSpiel game '{game_name}' could not be exported to NFG format: {exc}"
            ) from exc
        return gbt.read_nfg(io.StringIO(nfg_str))

    else:
        raise ValueError(
            f"OpenSpiel game '{game_name}' has unsupported dynamics type "
            f"'{dynamics}' and cannot be exported to Gambit format."
        )


def generate_gamut(
    game_class: str,
    params: dict | None = None,
    gamut_jar: Path | str | None = None,
) -> gbt.Game:
    """
    Generate a game using the GAMUT game generator.

    GAMUT (http://gamut.stanford.edu) is a Java-based suite of parameterised
    game generators. This function calls GAMUT as an external process and
    returns the resulting game as a :class:`~pygambit.Game` object.

    Parameters
    ----------
    game_class : str
        The GAMUT game class name (e.g. ``"RandomGame"``, ``"CovariantGame"``).
        See the `GAMUT documentation <http://gamut.stanford.edu/documentation.htm>`_
        for available game classes and their parameters.
    params : dict, optional
        Parameters forwarded to GAMUT as command-line flags. Each key becomes
        ``-key``.  Values are handled as follows:

        - ``True`` → flag with no value token
          (e.g. ``{"normalize": True}`` → ``-normalize``)
        - list or tuple → space-separated tokens
          (e.g. ``{"actions": [3, 3]}`` → ``-actions 3 3``)
        - anything else → single token
          (e.g. ``{"players": 2}`` → ``-players 2``)

        Multi-word GAMUT flags use underscores:
        ``{"min_payoff": 0, "max_payoff": 100}``.
    gamut_jar : pathlib.Path or str, optional
        Path to ``gamut.jar``. If omitted, the ``GAMUT_JAR`` environment
        variable is used. Raises :class:`FileNotFoundError` if neither is
        supplied.

    Returns
    -------
    Game
        The generated game.

    Raises
    ------
    RuntimeError
        If ``java`` is not found on the system PATH.
    FileNotFoundError
        If ``gamut.jar`` cannot be located.
    ValueError
        If GAMUT exits with a non-zero return code (e.g. invalid game class
        or parameters).
    """
    if shutil.which("java") is None:
        raise RuntimeError(
            "Java is required to run GAMUT. "
            "Install Java and ensure 'java' is on your PATH."
        )
    if gamut_jar is None:
        env_jar = os.environ.get("GAMUT_JAR")
        if env_jar is None:
            raise FileNotFoundError(
                "gamut.jar not found. Provide the path via the 'gamut_jar' argument "
                "or set the GAMUT_JAR environment variable. "
                "Download GAMUT from http://gamut.stanford.edu/."
            )
        gamut_jar = Path(env_jar).expanduser()
    else:
        gamut_jar = Path(gamut_jar).expanduser()
    if not gamut_jar.is_file():
        raise FileNotFoundError(f"gamut.jar not found at {gamut_jar}")

    cmd = ["java", "-jar", str(gamut_jar), "-g", game_class, "-output", "GambitOutput"]
    if params:
        for key, value in params.items():
            cmd.append(f"-{key}")
            if value is True:
                pass  # boolean flags like -normalize, -int_payoffs take no value token
            elif isinstance(value, (list, tuple)):
                cmd.extend(str(v) for v in value)
            else:
                cmd.append(str(value))

    with tempfile.NamedTemporaryFile(suffix=".nfg", delete=False) as tmp:
        out_path = Path(tmp.name)
    try:
        cmd.extend(["-f", str(out_path)])
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode != 0:
            raise ValueError(
                f"GAMUT failed for game class '{game_class}': "
                f"{result.stderr.strip() or result.stdout.strip()}"
            )
        return gbt.read_nfg(str(out_path))
    finally:
        out_path.unlink(missing_ok=True)


_GAMUT_GAMES = [
    {"Class": "ArmsRace",
     "Description": "Arms race with cost and demand functions chosen by players.",
     "Players": "n"},
    {"Class": "BattleOfTheSexes",
     "Description": "Coordination game where players prefer to meet but disagree on the venue.",
     "Players": "2"},
    {"Class": "BertrandOligopoly",
     "Description": "Bertrand oligopoly with arbitrary cost and demand functions.",
     "Players": "n"},
    {"Class": "BidirectionalLEG",
     "Description": "Bidirectional local-effect game on a specified graph.",
     "Players": "n"},
    {"Class": "Chicken",
     "Description": "Classic 2x2 Chicken game.",
     "Players": "2"},
    {"Class": "CollaborationGame",
     "Description": "Game where all players choosing the same action yields the highest payoffs.",
     "Players": "n"},
    {"Class": "CongestionGame",
     "Description": "Congestion game where payoffs depend on how many players share facilities.",
     "Players": "n"},
    {"Class": "CoordinationGame",
     "Description": "Pure coordination game rewarding matching action choices.",
     "Players": "n"},
    {"Class": "CournotDuopoly",
     "Description": "Cournot duopoly with arbitrary cost and inverse demand functions.",
     "Players": "2"},
    {"Class": "CovariantGame",
     "Description": "Game with payoffs whose cross-player correlation is set by a parameter r.",
     "Players": "n"},
    {"Class": "DispersionGame",
     "Description": "Game where dispersed action choices are rewarded.",
     "Players": "n"},
    {"Class": "GrabTheDollar",
     "Description": "Simultaneous competition to claim a prize first.",
     "Players": "2"},
    {"Class": "GreedyGame",
     "Description": "Players each choose a subset from a set; payoffs depend on overlap.",
     "Players": "2"},
    {"Class": "GuessThirdsAve",
     "Description": "Players guess a number trying to reach 2/3 of the average.",
     "Players": "n"},
    {"Class": "HawkAndDove",
     "Description": "Classic 2x2 Hawk and Dove game.",
     "Players": "2"},
    {"Class": "LocationGame",
     "Description": "Hotelling-style two-player location game on a street.",
     "Players": "2"},
    {"Class": "MajorityVoting",
     "Description": "Majority voting game with arbitrary candidate utilities.",
     "Players": "n"},
    {"Class": "MatchingPennies",
     "Description": "Classic 2x2 Matching Pennies game.",
     "Players": "2"},
    {"Class": "MinimumEffortGame",
     "Description": "Payoffs depend on the minimum effort exerted across all players.",
     "Players": "n"},
    {"Class": "NPlayerChicken",
     "Description": "N-player Chicken game with cooperation costs and collective rewards.",
     "Players": "n"},
    {"Class": "NPlayerPrisonersDilemma",
     "Description": "N-player Prisoner's Dilemma with parameterised payoff functions.",
     "Players": "n"},
    {"Class": "PolymatrixGame",
     "Description": "Polymatrix game formed by two-player edge games on a graph.",
     "Players": "n"},
    {"Class": "PrisonersDilemma",
     "Description": "Classic 2x2 Prisoner's Dilemma.",
     "Players": "2"},
    {"Class": "RandomCompoundGame",
     "Description": "Compound game whose payoffs are sums of random 2x2 sub-games.",
     "Players": "n"},
    {"Class": "RandomGame",
     "Description": "Game with payoffs drawn uniformly at random.",
     "Players": "n"},
    {"Class": "RandomGraphicalGame",
     "Description": "Random graphical game on a specified graph.",
     "Players": "n"},
    {"Class": "RandomLEG",
     "Description": "Random local-effect game on a specified graph.",
     "Players": "n"},
    {"Class": "RandomZeroSum",
     "Description": "Two-player zero-sum game with random payoffs.",
     "Players": "2"},
    {"Class": "RockPaperScissors",
     "Description": "Classic Rock, Paper, Scissors game.",
     "Players": "2"},
    {"Class": "ShapleyGame",
     "Description": "Shapley's original game; no stable equilibrium under replicator dynamics.",
     "Players": "2"},
    {"Class": "SimpleInspectionGame",
     "Description": "Inspection game where players choose from different-sized subsets.",
     "Players": "2"},
    {"Class": "TravelersDilemma",
     "Description": "Players claim a value; the lowest claim "
                    "wins a reward, others receive a penalty.",
     "Players": "n"},
    {"Class": "TwoByTwoGame",
     "Description": "2x2 game of a specified type per Rapoport's classification (type in 1-85).",
     "Players": "2"},
    {"Class": "UniformLEG",
     "Description": "Local-effect game where all edges share the same local-effect function.",
     "Players": "n"},
    {"Class": "WarOfAttrition",
     "Description": "Players choose concession times; payoffs "
                    "depend on valuations and decrements.",
     "Players": "2"},
]


def gamut_games() -> pd.DataFrame:
    """
    Return a DataFrame listing all 35 GAMUT game classes.

    Each row describes one game class that can be passed to
    :func:`generate_gamut` as the ``game_class`` argument.

    Returns
    -------
    pd.DataFrame
        DataFrame with columns:

        - ``"Class"``: the game class name to pass to :func:`generate_gamut`
        - ``"Description"``: a short description of the game family
        - ``"Players"``: ``"2"`` for two-player only, ``"n"`` for n-player
    """
    return pd.DataFrame.from_records(_GAMUT_GAMES, columns=["Class", "Description", "Players"])


def load(slug: str) -> gbt.Game:
    """
    Load a game from the default catalog.

    Parameters
    ----------
    slug : str
        The slug of the game to load.

    Returns
    -------
    gbt.Game
        The loaded game.

    Raises
    ------
    FileNotFoundError
        If the game does not exist in the catalog.
    """
    return Catalog().load(slug)


def games(
    n_actions: int | None = None,
    n_contingencies: int | None = None,
    n_infosets: int | None = None,
    is_const_sum: bool | None = None,
    is_perfect_recall: bool | None = None,
    is_tree: bool | None = None,
    min_payoff: float | None = None,
    max_payoff: float | None = None,
    n_nodes: int | None = None,
    n_outcomes: int | None = None,
    n_players: int | None = None,
    n_strategies: int | None = None,
    include_descriptions: bool = False,
) -> pd.DataFrame:
    """
    List games available in the default catalog.

    Most arguments are treated as filters on the
    attributes of the Game objects.

    Parameters
    ----------
    n_actions: int, optional
        The number of actions in the game. Only extensive games are returned.
    n_contingencies: int, optional
        The number of contingencies in the game.
    n_infosets: int, optional
        The number of information sets in the game. Only extensive games are returned.
    is_const_sum: bool, optional
        Whether the game is constant-sum.
    is_perfect_recall: bool, optional
        Whether the game has perfect recall.
    is_tree: bool, optional
        Whether the game is an extensive game (a tree).
    min_payoff: float, optional
        The minimum payoff in the game. Games returned have `min_payoff >= value`.
    max_payoff: float, optional
        The maximum payoff in the game. Games returned have `max_payoff <= value`.
    n_nodes: int, optional
        The number of nodes in the game. Only extensive games are returned.
    n_outcomes: int, optional
        The number of outcomes in the game.  Only extensive and strategic games, which
        have outcomes, are returned.
    n_players: int, optional
        The number of players in the game.
    n_strategies: int, optional
        The number of pure strategies in the game.
    include_descriptions: bool, optional
        Whether to include the description of each game in the returned DataFrame.
        Defaults to False.

    Returns
    -------
    pd.DataFrame
        A DataFrame with columns "Game" and "Title", where "Game" is the slug to load the game.
        If `include_descriptions=True`, the DataFrame will also include a "Description" column.
    """
    return Catalog().games(
        n_actions=n_actions,
        n_contingencies=n_contingencies,
        n_infosets=n_infosets,
        is_const_sum=is_const_sum,
        is_perfect_recall=is_perfect_recall,
        is_tree=is_tree,
        min_payoff=min_payoff,
        max_payoff=max_payoff,
        n_nodes=n_nodes,
        n_outcomes=n_outcomes,
        n_players=n_players,
        n_strategies=n_strategies,
        include_descriptions=include_descriptions,
    )
