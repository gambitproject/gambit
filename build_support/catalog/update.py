import argparse
import re
from pathlib import Path

import pandas as pd
from draw_tree import generate_pdf, generate_png, generate_tex

import pygambit as gbt

CATALOG_RST_TABLE = Path(__file__).parent.parent.parent / "doc" / "catalog_table.rst"
CATALOG_DIR = Path(__file__).parent.parent.parent / "catalog"
MAKEFILE_AM = Path(__file__).parent.parent.parent / "Makefile.am"

# Common arguments for visualization generation
draw_tree_args = {
    "color_scheme": "gambit",
    "sublevel_scaling": 0,
    "shared_terminal_depth": True,
}


def _write_efg_table(df: pd.DataFrame, f, tikz_re, regenerate_images: bool):
    """Write the EFG games list-table to file handle f."""
    f.write(".. list-table::\n")
    f.write("   :header-rows: 1\n")
    f.write("   :widths: 100\n")
    f.write("   :class: tight-table\n")
    f.write("\n")
    f.write("   * - **Extensive form games**\n")

    efg_df = df[df["Format"] == "efg"]
    for _, row in efg_df.iterrows():
        slug = row["Game"]
        title = str(row.get("Title", "")).strip()
        description = str(row.get("Description", "")).strip()

        tex_path = CATALOG_DIR / "img" / f"{slug}.tex"
        if regenerate_images or not tex_path.exists():
            g = gbt.catalog.load(slug)
            viz_path = CATALOG_DIR / "img" / f"{slug}"
            viz_path.parent.mkdir(parents=True, exist_ok=True)
            for func in [generate_tex, generate_png, generate_pdf]:
                func(g, save_to=str(viz_path), **draw_tree_args)

        with open(tex_path, encoding="utf-8") as tex_f:
            tex_content = tex_f.read()
        match = tikz_re.search(tex_content)
        tikz = match.group(1).strip() if match else "% Could not extract tikzpicture from tex file"

        # Main dropdown
        f.write(f"   * - .. dropdown:: {title}\n")
        f.write("          \n")
        if description:
            for line in description.splitlines():
                f.write(f"          {line}\n")
            f.write("          \n")
        f.write("          **Load in PyGambit:**\n")
        f.write("          \n")
        f.write("          .. code-block:: python\n")
        f.write("             \n")
        f.write(f'             pygambit.catalog.load("{slug}")\n')
        f.write("          \n")

        # Download links (inside the dropdown)
        download_links = [row["Download"]]
        for ext in ["efg", "ef", "tex", "png", "pdf"]:
            download_links.append(f":download:`{slug}.{ext} <../catalog/img/{slug}.{ext}>`")
        f.write("          **Download game and image files:**\n")
        f.write("          \n")
        f.write(f"          {' '.join(download_links)}\n")
        f.write("       \n")

        # TiKZ image (outside dropdown)
        f.write("       .. tikz::\n")
        f.write("          :align: center\n")
        f.write("          \n")
        for line in tikz.splitlines():
            f.write(f"          {line}\n")
        f.write("       \n")


def _write_nfg_table(df: pd.DataFrame, f):
    """Write the NFG games list-table to file handle f."""
    f.write(".. list-table::\n")
    f.write("   :header-rows: 1\n")
    f.write("   :widths: 100\n")
    f.write("   :class: tight-table\n")
    f.write("\n")
    f.write("   * - **Strategic form games**\n")

    nfg_df = df[df["Format"] == "nfg"]
    for _, row in nfg_df.iterrows():
        slug = row["Game"]
        title = str(row.get("Title", "")).strip()

        # Title as plain text header
        f.write(f"   * - **{title}**\n")
        f.write("       \n")

        # Jupyter-execute block (no dropdown)
        f.write("       .. jupyter-execute::\n")
        f.write("          \n")
        f.write("          import pygambit\n")
        f.write(f'          pygambit.catalog.load("{slug}")\n')
        f.write("       \n")

        # Download link (plain, no dropdown)
        f.write(f"       :download:`{slug}.nfg <../catalog/{slug}.nfg>`\n")
        f.write("       \n")


def generate_rst_table(df: pd.DataFrame, rst_path: Path, regnerate_images: bool = False):
    """Generate RST output with two list-tables: one for EFG and one for NFG games."""
    tikz_re = re.compile(r"\\begin\{document\}(.*?)\\end\{document\}", re.DOTALL)

    with open(rst_path, "w", encoding="utf-8") as f:
        # TOC linking to both sections
        f.write(".. contents::\n")
        f.write("   :local:\n")
        f.write("   :depth: 1\n")
        f.write("\n")

        # EFG section
        f.write("Extensive form games\n")
        f.write("--------------------\n")
        f.write("\n")
        _write_efg_table(df, f, tikz_re, regnerate_images)
        f.write("\n")

        # NFG section
        f.write("Strategic form games\n")
        f.write("--------------------\n")
        f.write("\n")
        _write_nfg_table(df, f)


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
    parser.add_argument("--regenerate-images", action="store_true")
    args = parser.parse_args()

    # Create RST list-table used by doc/catalog.rst
    df = gbt.catalog.games(include_descriptions=True)
    generate_rst_table(df, CATALOG_RST_TABLE, regnerate_images=args.regenerate_images)
    print(f"Generated {CATALOG_RST_TABLE} for use in local docs build. DO NOT COMMIT.")
    if args.build:
        # Update the Makefile.am with the current list of catalog files
        update_makefile()
