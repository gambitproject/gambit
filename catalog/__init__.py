from importlib.resources import as_file, files
from pathlib import Path

import pandas as pd

import pygambit as gbt

# Use the full string path to the virtual package we created
_CATALOG_RESOURCE = files(__name__)

READERS = {
    ".nfg": gbt.read_nfg,
    ".efg": gbt.read_efg,
}


def load(slug: str) -> gbt.Game:
    """
    Load a game from the package catalog.
    """
    slug = str(Path(slug)).replace("\\", "/")

    for suffix, reader in READERS.items():
        resource_path = _CATALOG_RESOURCE / f"{slug}{suffix}"

        if resource_path.is_file():
            # as_file ensures we have a real filesystem path for the reader
            with as_file(resource_path) as path:
                return reader(str(path))

    raise FileNotFoundError(f"No catalog entry called {slug}.nfg or {slug}.efg")


def games() -> pd.DataFrame:
    """
    List games available in the package catalog, including subdirectories.
    """
    records: list[dict[str, str]] = []

    # Using rglob("*") to find files in all subdirectories
    for resource_path in sorted(_CATALOG_RESOURCE.rglob("*")):
        reader = READERS.get(resource_path.suffix)

        if reader is not None and resource_path.is_file():

            # Calculate the path relative to the root resource
            # and remove the suffix to get the "slug"
            rel_path = resource_path.relative_to(_CATALOG_RESOURCE)
            slug = rel_path.with_suffix("").as_posix()

            with as_file(resource_path) as path:
                game = reader(str(path))
                records.append(
                    {
                        "Game": slug,
                        "Title": game.title,
                    }
                )

    return pd.DataFrame.from_records(records, columns=["Game", "Title"])
