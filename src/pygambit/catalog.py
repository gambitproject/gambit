import io
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


def load_openspiel(game_name: str, params: dict | None = None) -> gbt.Game:
    """
    Load a game from the OpenSpiel library.

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
        available parameters per game. Defaults to an empty dict.

    Returns
    -------
    gbt.Game
        The loaded game.

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
