import argparse
from pathlib import Path

import pandas as pd
from draw_tree import generate_pdf, generate_png, generate_svg, generate_tex

import pygambit as gbt

CATALOG_RST_TABLE = Path(__file__).parent.parent.parent / "doc" / "catalog_table.rst"
CATALOG_DIR = Path(__file__).parent.parent.parent / "catalog"
MAKEFILE_AM = Path(__file__).parent.parent.parent / "Makefile.am"


def catalog_draw_tree_settings(slug: str) -> dict:
    """Return the draw_tree settings for a given catalog slug."""
    settings = {
        "color_scheme": "gambit",
        "font_family": "sffamily",
        "font_italic": True,
        "shared_terminal_depth": True,
        "sublevel_scaling": 0,
    }
    if slug == "bagwell1995" or "watson2013" in slug:
        settings["sublevel_scaling"] = 1
    elif slug == "myerson1991/fig2_1" or slug == "reiley2008/fig1":
        settings["action_label_position"] = 0.4
    elif "selten1975" in slug:
        settings["shared_terminal_depth"] = False
    return settings


def generate_rst_table(df: pd.DataFrame, rst_path: Path, regenerate_images: bool = False):
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
        f.write("   * - **Catalog of games**\n")

        for _, row in df.iterrows():
            slug = row["Game"]
            title = str(row.get("Title", "")).strip()
            description = str(row.get("Description", "")).strip()
            # Skip any games which lack a description
            if description:
                all_exts = []
                all_paths = []
                if row["Format"] == "efg":
                    ef_path = CATALOG_DIR / "img" / f"{slug}.ef"
                    all_exts.append("ef")
                    all_paths.append(ef_path)
                all_exts = all_exts + ["tex", "png", "pdf", "svg"]
                tex_path = CATALOG_DIR / "img" / f"{slug}.tex"
                all_paths.append(tex_path)
                all_paths.append(CATALOG_DIR / "img" / f"{slug}.png")
                all_paths.append(CATALOG_DIR / "img" / f"{slug}.pdf")
                all_paths.append(CATALOG_DIR / "img" / f"{slug}.svg")
                missing_any = not all(p.exists() for p in all_paths)

                if regenerate_images or missing_any:
                    g = gbt.catalog.load(slug)
                    viz_path = CATALOG_DIR / "img" / f"{slug}"
                    viz_path.parent.mkdir(parents=True, exist_ok=True)
                    for func in [generate_tex, generate_png, generate_pdf, generate_svg]:
                        func(g, save_to=str(viz_path), **catalog_draw_tree_settings(slug))

                # Main dropdown
                f.write(f"   * - .. dropdown:: {title}\n")
                f.write("          :open:\n")
                f.write("          \n")
                for line in description.splitlines():
                    f.write(f"          {line}\n")

                f.write("          \n")
                f.write("          **Load in PyGambit:**\n")
                f.write("          \n")
                f.write("          .. code-block:: python\n")
                f.write("             \n")
                f.write(f'             pygambit.catalog.load("{slug}")\n')
                f.write("          \n")

                # Download links
                download_links = [row["Download"]]
                for ext in all_exts:
                    download_links.append(
                        f":download:`{slug}.{ext} <../catalog/img/{slug}.{ext}>`"
                    )
                f.write("          **Download game and image files:**\n")
                f.write("          \n")
                f.write(f"          {' '.join(download_links)}\n")
                f.write("          \n")

                # Draw image
                f.write("          .. jupyter-execute::\n")
                f.write("             :hide-code:\n")
                f.write("             \n")
                f.write("             import pygambit\n")
                f.write("             from draw_tree import draw_tree\n")
                if row["Format"] == "efg":
                    settings = catalog_draw_tree_settings(slug)
                    settings_str = ", ".join(f"{k}={v!r}" for k, v in settings.items())
                    f.write(
                        f"             draw_tree("
                        f'pygambit.catalog.load("{slug}"), '
                        f"{settings_str})\n"
                    )
                elif row["Format"] == "nfg":
                    f.write(
                        f"             draw_tree("
                        f'pygambit.catalog.load("{slug}"), '
                        f'save_to="../catalog/img/{slug}.png")\n'
                    )
                f.write("          \n")


def update_makefile():
    """Update the catalog.am with all games from the catalog."""

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

    am_path = Path(__file__).parent / "catalog.am"

    if am_path.exists():
        with open(am_path, encoding="utf-8") as f:
            content = f.read()
    else:
        content = ""

    updated_content = "CATALOG_FILES = \\\n"
    for gf in game_files:
        if gf == game_files[-1]:
            updated_content += f"\t{gf}\n"
        else:
            updated_content += f"\t{gf} \\\n"

    if content != updated_content:
        with open(am_path, "w", encoding="utf-8") as f:
            f.write(updated_content)
        print(f"Updated {str(am_path)}")
    else:
        print(f"No changes to add to {str(am_path)}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--build", action="store_true")
    parser.add_argument("--regenerate-images", action="store_true")
    args = parser.parse_args()

    # Create RST list-table used by doc/catalog.rst
    df = gbt.catalog.games(include_descriptions=True)
    generate_rst_table(df, CATALOG_RST_TABLE, regenerate_images=args.regenerate_images)
    print(f"Generated {CATALOG_RST_TABLE} for use in local docs build. DO NOT COMMIT.")
    if args.build:
        # Update the Makefile.am with the current list of catalog files
        update_makefile()
