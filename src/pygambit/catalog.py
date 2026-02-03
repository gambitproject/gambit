from importlib.resources import files

import pygambit as gbt

_GAMEFILES_DIR = files(__package__) / "catalog"


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
    candidates = {
        ".nfg": gbt.read_nfg,
        ".efg": gbt.read_efg,
    }

    for suffix, reader in candidates.items():
        path = _GAMEFILES_DIR / f"{slug}{suffix}"
        if path.is_file():
            return reader(str(path))

    raise FileNotFoundError(f"No catalog entry called {slug}.nfg or {slug}.efg")
