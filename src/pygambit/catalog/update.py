#!/usr/bin/env python3

from pathlib import Path

from ruamel.yaml import YAML

_CATALOG_YAML = Path(__file__).parent / "catalog.yml"
_GAMEFILES_DIR = Path(__file__).parent.parent.parent.parent / "contrib/games"


def make_class_name(filename: str) -> str:
    """
    Convert a filename (without extension) into a class name.
    - Replace hyphens with underscores
    - Capitalise
    - Prepend 'Game' if it starts with a digit
    """
    name = filename.replace("-", "_")

    # Capitalise in a simple, predictable way
    name = name[0].upper() + name[1:] if name else name

    if name and name[0].isdigit():
        name = f"Game{name}"

    return name


if __name__ == "__main__":
    # Use ruamel.yaml to preserve comments
    yaml = YAML()
    yaml.preserve_quotes = True
    yaml.default_flow_style = False

    efg_files = list(_GAMEFILES_DIR.rglob("*.efg"))
    nfg_files = list(_GAMEFILES_DIR.rglob("*.nfg"))

    print(f"Found {len(efg_files)} .efg files in contrib/games")
    print(f"Found {len(nfg_files)} .nfg files in contrib/games")

    all_files = sorted(efg_files + nfg_files)

    # Get the current class names from the catalog
    with open(_CATALOG_YAML, encoding="utf-8") as f:
        catalog = yaml.load(f) or {}
    file_names = [entry["file"] for entry in catalog.values() if "file" in entry]

    # Iterate through contrib/games and update the catalog
    # with new/missing entries
    new_entries_counter = 0
    new_entries = {}
    for path in all_files:
        stem = path.stem
        class_name = make_class_name(stem)

        # Avoid duplicates by appending EFG or NFG
        if class_name in new_entries:
            class_name += path.suffix.split(".")[-1].upper()

        if path.name not in file_names:
            new_entries[class_name] = {
                "file": path.name,
                "metadata": {},
            }
            new_entries_counter += 1

    # Update the catalog
    catalog.update(new_entries)
    with _CATALOG_YAML.open("w", encoding="utf-8") as f:
        yaml.dump(catalog, f)

    print(f"Added {new_entries_counter} new entries to the catalog")
    print(f"Output written to: {_CATALOG_YAML}")
    print("Done.")
