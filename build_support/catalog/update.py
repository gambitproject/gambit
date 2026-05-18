import argparse
from pathlib import Path

import pandas as pd

import pygambit as gbt

CATALOG_RST_TABLE = Path(__file__).parent.parent.parent / "doc" / "catalog_table.rst"
CATALOG_DIR = Path(__file__).parent.parent.parent / "catalog"
MAKEFILE_AM = Path(__file__).parent.parent.parent / "Makefile.am"


def generate_rst_table(df: pd.DataFrame, rst_path: Path):
    """Generate RST output with a list-table for games."""

    with open(rst_path, "w", encoding="utf-8") as f:
        # TOC linking to both sections
        f.write(".. contents::\n")
        f.write("   :local:\n")
        f.write("   :depth: 1\n")
        f.write("\n")
        f.write(".. list-table::\n")
        f.write("   :header-rows: 1\n")
        f.write("   :widths: 100\n")
        f.write("   :class: tight-table\n")
        f.write("\n")
        f.write("   * - **Extensive form games**\n")

        for _, row in df.iterrows():
            slug = row["Game"]
            title = str(row.get("Title", "")).strip()
            description = str(row.get("Description", "")).strip()
            # Skip any games which lack a description
            if description:
                # Main dropdown
                f.write(f"   * - .. dropdown:: {title}\n")
                f.write("          :open:\n")
                f.write("          \n")
                for line in description.splitlines():
                    f.write(f"          {line}\n")

                f.write("          \n")
                f.write("          **Load in PyGambit:**\n")
                f.write("          \n")
                if row["Format"] == "efg":
                    f.write("          .. code-block:: python\n")
                    f.write("             \n")
                    f.write(f'             pygambit.catalog.load("{slug}")\n')
                    f.write("          \n")
                elif row["Format"] == "nfg":
                    f.write("          .. jupyter-execute::\n")
                    f.write("             \n")
                    f.write("             import pygambit\n")
                    f.write(f'             pygambit.catalog.load("{slug}")\n')
                    f.write("          \n")

                # Download links (inside the dropdown)
                download_links = [row["Download"]]
                f.write("          **Download:**\n")
                f.write("          \n")
                f.write(f"          {' '.join(download_links)}\n")
                f.write("       \n")


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
    if args.build:
        # Update the Makefile.am with the current list of catalog files
        update_makefile()
