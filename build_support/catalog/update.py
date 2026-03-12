import argparse
from pathlib import Path

import pandas as pd
from draw_tree import draw_tree, generate_pdf, generate_png, generate_tex

import pygambit as gbt

CATALOG_RST_TABLE = Path(__file__).parent.parent.parent / "doc" / "catalog_table.rst"
CATALOG_DIR = Path(__file__).parent.parent.parent / "catalog"
MAKEFILE_AM = Path(__file__).parent.parent.parent / "Makefile.am"


def generate_rst_table(df: pd.DataFrame, rst_path: Path):
    """Generate a list-table RST file with dropdowns for long descriptions."""
    with open(rst_path, "w", encoding="utf-8") as f:
        f.write(".. list-table::\n")
        f.write("   :header-rows: 1\n")
        f.write("   :widths: 100\n")
        f.write("   :class: tight-table\n")
        f.write("\n")

        f.write("   * - **Game**\n")

        for _, row in df.iterrows():
            slug = row["Game"]
            g = gbt.catalog.load(slug)

            # Common arguments for visualization generation
            viz_args = {
                "color_scheme": "gambit",
                "sublevel_scaling": 0,
                "shared_terminal_depth": True,
            }

            tikz = draw_tree(g, **viz_args)

            # Generate extra formats
            for func in [
                generate_tex,
                generate_png,
                generate_pdf,
            ]:
                viz_path = CATALOG_DIR / f"{slug}"
                func(g, save_to=str(viz_path), **viz_args)

            title = str(row.get("Title", "")).strip()
            description = str(row.get("Description", "")).strip()

            # Column 1: Game Details
            f.write(f"   * - **{title}**\n")
            f.write("       \n")
            if description:
                f.write("       .. dropdown:: Game description\n")
                f.write("          \n")
                for line in description.splitlines():
                    f.write(f"          {line}\n")
                f.write("       \n")

            f.write("       .. dropdown:: PyGambit\n")
            f.write("          \n")
            f.write("          .. code-block:: python\n")
            f.write("             \n")
            f.write(f'             pygambit.catalog.load("{slug}")\n')
            f.write("       \n")

            # Prepare download links for the dropdown
            download_links = [row["Download"]]
            for ext in ["tex", "png", "pdf"]:
                download_links.append(f":download:`{slug}.{ext} <../catalog/{slug}.{ext}>`")

            f.write("       .. dropdown:: Downloads\n")
            f.write("          \n")
            f.write(f"          {' '.join(download_links)}\n")
            f.write("       \n")

            # Visualization below dropdowns in the same cell
            f.write("       .. tikz::\n")
            f.write("          \n")
            for line in tikz.splitlines():
                f.write(f"          {line}\n")


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
