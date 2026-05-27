import argparse
import shutil
from pathlib import Path

import pandas as pd
import yaml
from draw_tree import generate_pdf, generate_png, generate_svg, generate_tex

import pygambit as gbt

CATALOG_RST_TABLE = Path(__file__).parent.parent.parent / "doc" / "catalog_table.rst"
CATALOG_DIR = Path(__file__).parent.parent.parent / "catalog"
MAKEFILE_AM = Path(__file__).parent.parent.parent / "Makefile.am"
DRAW_TREE_SETTINGS_CONFIG = Path(__file__).parent / "draw_tree_settings.yaml"
SUPPORTED_GAME_FORMATS = {"efg", "nfg"}


def catalog_draw_tree_settings(slug: str) -> dict:
    """Return the draw_tree settings for a given catalog slug."""
    with open(DRAW_TREE_SETTINGS_CONFIG, encoding="utf-8") as f:
        config = yaml.safe_load(f)
    settings = dict(config["defaults"])
    overrides = config.get("overrides", {})
    # Apply overrides shortest-key-first so that more specific (longer) entries
    # such as "myerson1991/fig2_1" win over group-level entries like "myerson1991".
    for key in sorted(overrides, key=len):
        if slug == key or slug.startswith(key + "/"):
            settings.update(overrides[key])
    return settings


def generate_rst_table(
    df: pd.DataFrame,
    rst_path: Path,
    regenerate_images: bool = False,
    catalog_dir: Path | None = None,
):
    """Generate RST output with a list-table for games."""
    catalog_dir = catalog_dir or CATALOG_DIR
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
            # Skip rows with unrecognised formats (defensive guard).
            if row["Format"] not in SUPPORTED_GAME_FORMATS:
                continue
            # Skip any games which lack a description
            if description:
                # Build the list of expected image files so we can check whether
                # any are missing and need to be generated.
                all_exts = []
                all_paths = []
                if row["Format"] == "efg":
                    ef_path = catalog_dir / "img" / f"{slug}.ef"
                    all_exts.append("ef")
                    all_paths.append(ef_path)
                all_exts = all_exts + ["tex", "png", "pdf", "svg"]
                tex_path = catalog_dir / "img" / f"{slug}.tex"
                all_paths.append(tex_path)
                all_paths.append(catalog_dir / "img" / f"{slug}.png")
                all_paths.append(catalog_dir / "img" / f"{slug}.pdf")
                all_paths.append(catalog_dir / "img" / f"{slug}.svg")
                missing_any = not all(p.exists() for p in all_paths)

                if regenerate_images or missing_any:
                    viz_path = catalog_dir / "img" / f"{slug}"
                    viz_path.parent.mkdir(parents=True, exist_ok=True)
                    # Use a committed curated .ef file if present; otherwise derive
                    # the layout automatically from the game object.
                    curated_ef = catalog_dir / f"{slug}.ef"
                    source = str(curated_ef) if curated_ef.exists() else gbt.catalog.load(slug)
                    for func in [generate_tex, generate_png, generate_pdf, generate_svg]:
                        func(source, save_to=str(viz_path), **catalog_draw_tree_settings(slug))
                    # DrawTree may not write catalog/img/{slug}.ef when its input is
                    # already an .ef file, so copy it if the img copy is still absent.
                    img_ef = catalog_dir / "img" / f"{slug}.ef"
                    if not img_ef.exists() and curated_ef.exists():
                        shutil.copy2(curated_ef, img_ef)

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
                    curated_ef = catalog_dir / f"{slug}.ef"
                    if curated_ef.exists():
                        f.write(
                            f'             draw_tree("../catalog/{slug}.ef", {settings_str})\n'
                        )
                    else:
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


def update_makefile(
    catalog_dir: Path | None = None,
    am_path: Path | None = None,
):
    """Update the catalog.am with all games from the catalog."""
    catalog_dir = catalog_dir or CATALOG_DIR
    am_path = am_path or Path(__file__).parent / "catalog.am"

    slugs = []
    for resource_path in sorted(catalog_dir.rglob("*.efg")):
        if resource_path.is_file():
            rel_path = resource_path.relative_to(catalog_dir)
            slugs.append(str(rel_path))
    for resource_path in sorted(catalog_dir.rglob("*.nfg")):
        if resource_path.is_file():
            rel_path = resource_path.relative_to(catalog_dir)
            slugs.append(str(rel_path))
    for resource_path in sorted(catalog_dir.rglob("*.ef")):
        # Exclude the generated .ef files under catalog/img/; only curated
        # .ef files committed alongside game files should be distributed.
        if resource_path.is_file() and catalog_dir / "img" not in resource_path.parents:
            rel_path = resource_path.relative_to(catalog_dir)
            slugs.append(str(rel_path))

    game_files = []
    for slug in slugs:
        game_files.append(f"catalog/{slug}")
    game_files.sort()

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
    parser = argparse.ArgumentParser(
        description=(
            "Update Gambit catalog documentation and build files. "
            "Always regenerates doc/catalog_table.rst from the current catalog. "
            "Run from the repo root or build_support/catalog/."
        )
    )
    parser.add_argument(
        "--build",
        action="store_true",
        help=(
            "Also update build_support/catalog/catalog.am with the current list of "
            "catalog game files. Required after adding or removing games."
        ),
    )
    parser.add_argument(
        "--regenerate-images",
        action="store_true",
        help=(
            "Force regeneration of all game visualisation images (PNG, PDF, SVG, TeX), "
            "even if they already exist. Use this to pick up changes to game files or "
            "draw_tree_settings.yaml."
        ),
    )
    args = parser.parse_args()

    # Create RST list-table used by doc/catalog.rst
    df = gbt.catalog.games(include_descriptions=True)
    generate_rst_table(df, CATALOG_RST_TABLE, regenerate_images=args.regenerate_images)
    print(f"Generated {CATALOG_RST_TABLE} for use in local docs build. DO NOT COMMIT.")
    if args.build:
        # Update the Makefile.am with the current list of catalog files
        update_makefile()
