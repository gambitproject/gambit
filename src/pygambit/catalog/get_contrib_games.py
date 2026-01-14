#!/usr/bin/env python3

from pathlib import Path

OUTPUT_FILE = "generated_catalog_games.py"
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

    print(f"Found {len(efg_files)} .efg files")
    print(f"Found {len(nfg_files)} .nfg files")

    all_files = sorted(efg_files + nfg_files)

    lines = []
    lines.append("from .catalog_game import CatalogGameFromContrib\n\n")

    class_names = []
    for path in all_files:
        stem = path.stem
        class_name = make_class_name(stem)

        # Avoid duplicates (some EFG and NFG have same name)
        if class_name in class_names:
            class_name += path.suffix.split(".")[-1].upper()

        class_names.append(class_name)
        lines.append(f"class {class_name}(CatalogGameFromContrib):")
        lines.append(f'    game_file = "{path.name}"')
        lines.append("\n")

    output_path = Path(__file__).parent / OUTPUT_FILE
    output_path.write_text("\n".join(lines), encoding="utf-8")

    print(f"Generated {len(all_files)} classes")
    print(f"Output written to: {output_path}")
    print("Done.")
