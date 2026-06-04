import contextlib
import io
import os
from importlib.resources import as_file, files
from pathlib import Path
from typing import Any

import pandas as pd

import pygambit as gbt

# Use the full string path to where the catalog data are placed in the package
_CATALOG_RESOURCE = files("pygambit") / "catalog_data"
# This ensures that catalog files are included in editable installs too
if not _CATALOG_RESOURCE.is_dir():
    _repo_catalog = Path(__file__).parent.parent.parent / "catalog"
    if _repo_catalog.is_dir():
        _CATALOG_RESOURCE = _repo_catalog

READERS = {
    ".nfg": gbt.read_nfg,
    ".efg": gbt.read_efg,
}

_OPENSPIEL_PREFIX = "open_spiel/"


@contextlib.contextmanager
def _suppress_c_stderr():
    """Redirect C-level stderr (fd 2) to /dev/null for the duration of the block.

    This prevents OpenSpiel's C++ code from printing error messages (e.g.
    "OpenSpiel exception: Must be a normal-form game") to the user's terminal or
    notebook when we speculatively attempt an export that may not be supported.
    Falls back silently if the fd-level redirect is unavailable (e.g. Windows).
    """
    try:
        devnull_fd = os.open(os.devnull, os.O_WRONLY)
        saved_fd = os.dup(2)
        os.dup2(devnull_fd, 2)
        os.close(devnull_fd)
    except OSError:
        yield
        return
    try:
        yield
    finally:
        os.dup2(saved_fd, 2)
        os.close(saved_fd)


def _load_from_openspiel(game_name: str) -> gbt.Game:
    """
    Load a game from the OpenSpiel library by name.

    Tries NFG export first; falls back to EFG export via the
    open_spiel.python.algorithms.gambit exporter. Raises ImportError
    if open_spiel is not installed, ValueError if the game cannot be
    exported to either format.
    """
    try:
        import pyspiel
        from open_spiel.python.algorithms.gambit import export_gambit
    except ImportError as exc:
        raise ImportError(
            "open_spiel is required to load OpenSpiel games. "
            "Install it with: pip install open_spiel"
        ) from exc

    try:
        game = pyspiel.load_game(game_name)
    except Exception as exc:
        raise ValueError(f"Could not load OpenSpiel game '{game_name}': {exc}") from exc

    # Try EFG first (works for extensive-form games).
    # Suppress C-level stderr so any C++ error messages for games that don't
    # support EFG export are not shown when we fall through to the NFG path.
    try:
        with _suppress_c_stderr():
            efg_str = export_gambit(game)
        return gbt.read_efg(io.StringIO(efg_str))
    except Exception:
        pass

    # Fall back to NFG export (works for normal-form games)
    try:
        nfg_str = pyspiel.game_to_nfg_string(game)
        return gbt.read_nfg(io.StringIO(nfg_str))
    except Exception as exc:
        raise ValueError(
            f"OpenSpiel game '{game_name}' could not be exported to EFG or NFG format."
        ) from exc


def load(slug: str) -> gbt.Game:
    """
    Load a game from the package catalog.

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

    if slug.startswith(_OPENSPIEL_PREFIX):
        return _load_from_openspiel(slug[len(_OPENSPIEL_PREFIX):])

    # Try to load from file
    for suffix, reader in READERS.items():
        resource_path = _CATALOG_RESOURCE / f"{slug}{suffix}"
        if resource_path.is_file():
            with as_file(resource_path) as path:
                return reader(str(path))

    # Raise error if game does not exist
    raise FileNotFoundError(f"No catalog entry called {slug}")


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
    List games available in the package catalog.

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
        The number of outcomes in the game.
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
            if not game.is_tree:
                return False
            if len(game.actions) != n_actions:
                return False
        if n_contingencies is not None and len(game.contingencies) != n_contingencies:
            return False
        if n_infosets is not None:
            if not game.is_tree:
                return False
            if len(game.infosets) != n_infosets:
                return False
        if is_const_sum is not None and game.is_const_sum != is_const_sum:
            return False
        if is_perfect_recall is not None and game.is_perfect_recall != is_perfect_recall:
            return False
        if is_tree is not None and game.is_tree != is_tree:
            return False
        if min_payoff is not None and game.min_payoff < min_payoff:
            return False
        if max_payoff is not None and game.max_payoff > max_payoff:
            return False
        if n_nodes is not None:
            if not game.is_tree:
                return False
            if len(game.nodes) != n_nodes:
                return False
        if n_outcomes is not None and len(game.outcomes) != n_outcomes:
            return False
        if n_players is not None and len(game.players) != n_players:
            return False
        return not (n_strategies is not None and len(game.strategies) != n_strategies)

    def append_record(
        slug: str,
        game: gbt.Game,
    ) -> None:
        record = {
            "Game": slug,
            "Title": game.title,
        }
        if include_descriptions:
            record["Description"] = game.description
            ext = "efg" if game.is_tree else "nfg"
            record["Download"] = f":download:`{slug}.{ext} <../catalog/{slug}.{ext}>`"
            record["Format"] = ext
        records.append(record)

    # Add all the games stored as EFG/NFG files.
    # Collect paths matching each supported extension, sort together to preserve
    # a consistent alphabetical order, then load using the known reader.
    for resource_path in sorted(
        path
        for suffix in READERS
        for path in _CATALOG_RESOURCE.rglob(f"*{suffix}")
        if path.is_file()
    ):
        reader = READERS[resource_path.suffix]
        rel_path = resource_path.relative_to(_CATALOG_RESOURCE)
        slug = rel_path.with_suffix("").as_posix()

        with as_file(resource_path) as path:
            game = reader(str(path))
            if check_filters(game):
                append_record(slug, game)

    if include_descriptions:
        return pd.DataFrame.from_records(
            records, columns=["Game", "Title", "Description", "Download", "Format"]
        )
    return pd.DataFrame.from_records(records, columns=["Game", "Title"])
