import argparse
from pathlib import Path

import pandas as pd
from draw_tree import draw_tree

import pygambit as gbt

CATALOG_RST_TABLE = Path(__file__).parent.parent.parent / "doc" / "catalog_table.rst"
CATALOG_DIR = Path(__file__).parent.parent.parent / "catalog"
MAKEFILE_AM = Path(__file__).parent.parent.parent / "Makefile.am"


def generate_rst_table(df: pd.DataFrame, rst_path: Path):
    """Generate a list-table RST file with dropdowns for long descriptions."""
    with open(rst_path, "w", encoding="utf-8") as f:
        f.write(".. list-table::\n")
        f.write("   :header-rows: 1\n")
        f.write("   :widths: 60 40\n")
        f.write("   :class: tight-table\n")
        f.write("\n")

        f.write("   * - **Game**\n")
        f.write("     - **Details**\n")

        for _, row in df.iterrows():
            g = gbt.catalog.load(row["Game"])
            tikz = draw_tree(
                g, color_scheme="gambit", sublevel_scaling=0, shared_terminal_depth=True
            )

            f.write(f"   * - **{row['Game']}**\n")
            f.write("       \n")
            f.write("       .. tikz::\n")
            f.write("          \n")
            for line in tikz.splitlines():
                f.write(f"          {line}\n")

            description_cell_lines = []
            title = str(row.get("Title", "")).strip()
            description = str(row.get("Description", "")).strip()
            if description:
                description_cell_lines.append(f".. dropdown:: {title}")
                description_cell_lines.append("   ")  # Indented blank line
                for line in description.splitlines():
                    description_cell_lines.append(f"   {line}")
            else:
                description_cell_lines.append(title)

            f.write(f"     - {description_cell_lines[0]}\n")
            for line in description_cell_lines[1:]:
                f.write(f"       {line}\n")
            f.write("       \n")
            f.write(f"       {row['Download']}\n")


def update_makefile():
    """Update the Makefile.am with all games from the catalog."""

    # Using rglob("*") to find files in all subdirectories
    slugs = []
    for resource_path in sorted(CATALOG_DIR.rglob("*.efg")):
        if resource_path.is_file():
            rel_path = resource_path.relative_to(CATALOG_DIR)
            slugs.append(str(rel_path))
    for resource_path in sorted(CATALOG_DIR.rglob("*.nfg")):
        if resource_path.is_file():
            rel_path = resource_path.relative_to(CATALOG_DIR)
            slugs.append(str(rel_path))

    game_files = []
    for slug in slugs:
        game_files.append(f"catalog/{slug}")
    game_files.sort()

    with open(MAKEFILE_AM, encoding="utf-8") as f:
        content = f.readlines()

    with open(MAKEFILE_AM, "w", encoding="utf-8") as f:
        in_gamefiles_section = False
        for line in content:
            # Add to the EXTRA_DIST after the README.rst line
            if line.startswith("	src/README.rst \\"):
                in_gamefiles_section = True
                f.write("	src/README.rst \\\n")
                for gf in game_files:
                    if gf == game_files[-1]:
                        f.write(f"\t{gf}\n")
                    else:
                        f.write(f"\t{gf} \\\n")
                f.write("\n")
            elif in_gamefiles_section:
                if line.strip() == "":
                    in_gamefiles_section = False
                continue  # Skip old gamefiles lines
            else:
                f.write(line)

    with open(MAKEFILE_AM, encoding="utf-8") as f:
        updated_content = f.readlines()

    if content != updated_content:
        print(f"Updated {str(MAKEFILE_AM)}")
    else:
        print(f"No changes to add to {str(MAKEFILE_AM)}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--build", action="store_true")
    args = parser.parse_args()

    # Create RST list-table used by doc/catalog.rst
    df = gbt.catalog.games(include_descriptions=True)
    generate_rst_table(df, CATALOG_RST_TABLE)
    print(f"Generated {CATALOG_RST_TABLE} for use in local docs build. DO NOT COMMIT.")

    # Update the Makefile.am with the current list of catalog files
    if args.build:
        update_makefile()
