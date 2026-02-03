from importlib.resources import files

import pandas as pd

import pygambit as gbt

_GAMEFILES_DIR = files(__package__) / "catalog"
READERS = {
    ".nfg": gbt.read_nfg,
    ".efg": gbt.read_efg,
}


def load(slug: str) -> gbt.Game:
    """
    Load a game from the package catalog.

    The function looks for a catalog entry matching the given ``slug`` in the
    ``catalog`` resource directory. Files are tried in the following order:

    1. ``<slug>.nfg`` (normal-form game)
    2. ``<slug>.efg`` (extensive-form game)

    The first matching file found is loaded and returned as a
    :class:`pygambit.Game`.

    Parameters
    ----------
    slug : str
        Base name of the catalog entry, without file extension.

    Returns
    -------
    pygambit.Game
        The loaded game.

    Raises
    ------
    FileNotFoundError
        If no ``.nfg`` or ``.efg`` file exists for the given slug.
    """
    for suffix, reader in READERS.items():
        path = _GAMEFILES_DIR / f"{slug}{suffix}"
        if path.is_file():
            return reader(str(path))

    raise FileNotFoundError(f"No catalog entry called {slug}.nfg or {slug}.efg")


def games() -> pd.DataFrame:
    """
    List games available in the package catalog.

    Iterates over ``.nfg`` and ``.efg`` files found in the catalog resource
    directory, loads each game, and returns a pandas DataFrame summarising
    the results.

    The returned DataFrame has two columns:
    - ``slug``: the filename without its extension
    - ``title``: the game's ``title`` attribute

    Returns
    -------
    pandas.DataFrame
        A DataFrame with columns ``slug`` and ``title``.
    """
    records: list[dict[str, str]] = []

    for path in sorted(_GAMEFILES_DIR.iterdir()):
        reader = READERS.get(path.suffix)
        if reader is not None and path.is_file():
            game = reader(str(path))
            records.append(
                {
                    "slug": path.stem,
                    "title": game.title,
                }
            )

    return pd.DataFrame.from_records(records, columns=["slug", "title"])
