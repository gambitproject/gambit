#!/usr/bin/env python3

from pathlib import Path

import yaml

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
    efg_files = list(_GAMEFILES_DIR.rglob("*.efg"))
    nfg_files = list(_GAMEFILES_DIR.rglob("*.nfg"))

    print(f"Found {len(efg_files)} .efg files in contrib/games")
    print(f"Found {len(nfg_files)} .nfg files in contrib/games")

    all_files = sorted(efg_files + nfg_files)

    # Get the current class names from the catalog
    with open(_CATALOG_YAML, encoding="utf-8") as f:
        catalog = yaml.safe_load(f) or {}
    file_names = [entry["file"] for entry in catalog.values() if "file" in entry]

    # Iterate through contrib/games and update the catalog
    # with new/missing entries
    lines = []
    class_names = []
    new_entries_counter = 0
    for path in all_files:
        stem = path.stem

        class_name = make_class_name(stem)
        # Avoid duplicates by appending EFG or NFG
        if class_name in class_names:
            class_name += path.suffix.split(".")[-1].upper()
        class_names.append(class_name)

        # Add any new entries to the catalog
        if path.name not in file_names:
            lines.append(f"{class_name}:")
            lines.append(f'  file: "{path.name}"')
            lines.append("  metadata:\n")
            new_entries_counter += 1

    # Update the yml
    new_entries = yaml.safe_load("\n".join(lines)) or {}
    catalog.update(new_entries)
    with _CATALOG_YAML.open("w", encoding="utf-8") as f:
        yaml.safe_dump(catalog, f, sort_keys=False)

    print(f"Added {new_entries_counter} new entries to the catalog")
    print(f"Output written to: {_CATALOG_YAML}")
    print("Done.")
