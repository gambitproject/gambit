import argparse
import shutil
import sys
from pathlib import Path

import pandas as pd
import yaml
from gtdraw import pdf, png, svg, tex

import pygambit as gbt

CATALOG_RST_TABLE = Path(__file__).parent.parent.parent / "doc" / "catalog_table.rst"
CATALOG_DIR = Path(__file__).parent.parent.parent / "catalog"
MAKEFILE_AM = Path(__file__).parent.parent.parent / "Makefile.am"
GTDRAW_SETTINGS_CONFIG = Path(__file__).parent / "gtdraw_settings.yaml"
CATALOG_HIERARCHY_CONFIG = Path(__file__).parent / "catalog_hierarchy.yaml"
SUPPORTED_GAME_FORMATS = {"efg", "nfg"}

def use_catalog_dir(catalog_dir: Path | None = None) -> Path:
    """Point pygambit's catalog helpers at this checkout's catalog directory.

    update.py is a source-tree maintenance script, so it should not read stale
    package-data copies from an installed or previously built pygambit.
    """
    catalog_dir = catalog_dir or CATALOG_DIR
    gbt.catalog._CATALOG_RESOURCE = catalog_dir
    return catalog_dir

def catalog_gtdraw_settings(slug: str) -> dict:
    """Return the gtdraw settings for a given catalog slug."""
    with open(GTDRAW_SETTINGS_CONFIG, encoding="utf-8") as f:
        config = yaml.safe_load(f)
    settings = dict(config["defaults"])
    overrides = config.get("overrides", {})
    # Apply overrides shortest-key-first so that more specific (longer) entries
    # such as "myerson1991/fig2_1" win over group-level entries like "myerson1991".
    for key in sorted(overrides, key=len):
        if slug == key or slug.startswith(key + "/"):
            settings.update(overrides[key])
    return settings


def catalog_ef_file_variants(slug: str, catalog_dir: Path) -> list[dict] | None:
    """Scan catalog_dir for multiple curated .ef files for a game slug.

    Returns a list of dicts with keys ``label``, ``ef_path``, and
    ``variant_key`` when multiple .ef variants are detected alongside the game;
    returns None when zero or one .ef file exists and no suffix variants are present
    (no tabs needed).

    File-naming convention::

        catalog/{slug}.ef            primary variant → label "Default"
        catalog/{slug}__{suffix}.ef  additional variant → label derived from suffix

    The suffix part (after ``__``) is title-cased with underscores replaced by
    spaces, e.g. ``fig1__very_wide.ef`` → label "Very Wide".
    """
    game_dir = (catalog_dir / slug).parent
    stem = Path(slug).name

    primary_ef = game_dir / f"{stem}.ef"
    additional_efs = sorted(game_dir.glob(f"{stem}__*.ef"))

    # If there are no additional/suffix variants, it's not a multi-variant game.
    if not additional_efs:
        return None

    variants = []
    # Add the primary/default variant
    variants.append(
        {
            "label": "Default",
            "ef_path": primary_ef,
            "variant_key": slug,
        }
    )

    # Add the additional suffix variants
    for ef_file in additional_efs:
        suffix = ef_file.stem[len(stem) + 2 :]  # noqa: E203
        label = suffix.replace("_", " ").title()
        variant_key = f"{slug}__{suffix}"
        variants.append({"label": label, "ef_path": ef_file, "variant_key": variant_key})
    return variants


def load_hierarchy_labels() -> dict[str, str]:
    """Return the human-readable label mapping for catalog hierarchy nodes."""
    with open(CATALOG_HIERARCHY_CONFIG, encoding="utf-8") as f:
        config = yaml.safe_load(f)
    return config.get("labels", {})


def _node_label(prefix: str, labels: dict[str, str]) -> str:
    """Return the display label for a catalog hierarchy node.

    Falls back to title-casing the last path component when the prefix is not
    listed in the YAML config.
    """
    if prefix in labels:
        return labels[prefix]
    return Path(prefix).name.replace("_", " ").title()


def _warn_missing_descriptions(df: pd.DataFrame) -> None:
    """Print a warning to stderr for each game that lacks a description.

    Games without descriptions are silently excluded from the catalog page by
    ``_build_slug_tree``.  This function makes that visible so contributors
    know to add a description before the game will appear.
    """
    for _, row in df.iterrows():
        if str(row.get("Description", "")).strip():
            continue
        print(
            f"WARNING: '{row['Game']}' has no description and will not appear in the catalog.\n"
            f"Add a description to the game file to include it.",
            file=sys.stderr,
        )


def _build_slug_tree(df: pd.DataFrame) -> dict:
    """Build a nested dict tree from the slugs in *df*.

    Intermediate nodes are plain ``dict``s.  Leaf nodes (individual games) store
    the corresponding ``pd.Series`` row.  Slugs are split on ``"/"`` to produce
    the nesting.
    """
    tree: dict = {}
    for _, row in df.iterrows():
        if row.get("Format") not in SUPPORTED_GAME_FORMATS:
            continue
        if not str(row.get("Description", "")).strip():
            continue
        parts = row["Game"].split("/")
        node = tree
        for part in parts[:-1]:
            node = node.setdefault(part, {})
        node[parts[-1]] = row
    return tree


def _write_game_entry(
    f,
    row: pd.Series,
    slug: str,
    catalog_dir: Path,
    indent: str,
    regenerate_images: bool = False,
) -> None:
    """Write RST for a single game entry (dropdown + content) at *indent* depth."""
    i0 = indent  # .. dropdown:: title line
    i1 = indent + "   "  # content inside game dropdown
    i2 = indent + "      "  # nested directives (Download, tab-set, jupyter-execute)
    i3 = indent + "         "  # content inside nested directives
    i4 = indent + "            "  # tab-item content

    title = str(row.get("Title", "")).strip()
    description = str(row.get("Description", "")).strip()

    ef_variants = catalog_ef_file_variants(slug, catalog_dir) if row["Format"] == "efg" else None

    # ── Image generation ────────────────────────────────────────────────────
    if ef_variants:
        _variant_img_exts = ["ef", "tex", "png", "pdf", "svg"]
        for variant in ef_variants:
            vkey = variant["variant_key"]
            variant_paths = [catalog_dir / "img" / f"{vkey}.{ext}" for ext in _variant_img_exts]
            if regenerate_images or not all(p.exists() for p in variant_paths):
                viz_path = catalog_dir / "img" / vkey
                viz_path.parent.mkdir(parents=True, exist_ok=True)
                source = (
                    str(variant["ef_path"])
                    if variant["ef_path"].exists()
                    else gbt.catalog.load(slug)
                )
                for func in [tex, png, pdf, svg]:
                    func(source, save_to=str(viz_path), **catalog_gtdraw_settings(vkey))
                img_ef = catalog_dir / "img" / f"{vkey}.ef"
                if not img_ef.exists() and variant["ef_path"].exists():
                    shutil.copy2(variant["ef_path"], img_ef)
    else:
        all_exts = []
        all_paths = []
        if row["Format"] == "efg":
            all_exts.append("ef")
            all_paths.append(catalog_dir / "img" / f"{slug}.ef")
        all_exts += ["tex", "png", "pdf", "svg"]
        for ext in ["tex", "png", "pdf", "svg"]:
            all_paths.append(catalog_dir / "img" / f"{slug}.{ext}")
        if regenerate_images or not all(p.exists() for p in all_paths):
            viz_path = catalog_dir / "img" / slug
            viz_path.parent.mkdir(parents=True, exist_ok=True)
            curated_ef = catalog_dir / f"{slug}.ef"
            source = str(curated_ef) if curated_ef.exists() else gbt.catalog.load(slug)
            for func in [tex, png, pdf, svg]:
                func(source, save_to=str(viz_path), **catalog_gtdraw_settings(slug))
            img_ef = catalog_dir / "img" / f"{slug}.ef"
            if not img_ef.exists() and curated_ef.exists():
                shutil.copy2(curated_ef, img_ef)

    # ── RST output ──────────────────────────────────────────────────────────
    f.write(f"{i0}.. dropdown:: {title}\n")
    f.write(f"{i0}   :open:\n")
    f.write(f"{i0}\n")
    for line in description.splitlines():
        f.write(f"{i1}{line}\n")
    f.write(f"{i1}\n")
    f.write(f"{i1}**Load in PyGambit:**\n")
    f.write(f"{i1}\n")
    f.write(f"{i1}.. code-block:: python\n")
    f.write(f"{i1}   \n")
    f.write(f'{i1}   pygambit.catalog.load("{slug}")\n')
    f.write(f"{i1}\n")

    # Download links
    download_links = [row["Download"]]
    if ef_variants:
        for variant in ef_variants:
            vkey = variant["variant_key"]
            for ext in ["ef", "tex", "png", "pdf", "svg"]:
                download_links.append(f":download:`{vkey}.{ext} <../catalog/img/{vkey}.{ext}>`")
    else:
        for ext in all_exts:
            download_links.append(f":download:`{slug}.{ext} <../catalog/img/{slug}.{ext}>`")
    f.write(f"{i1}.. dropdown:: Download game and image files\n")
    f.write(f"{i1}   \n")
    f.write(f"{i2}{' '.join(download_links)}\n")
    f.write(f"{i1}\n")

    # Visualization
    if ef_variants:
        f.write(f"{i1}.. tab-set::\n")
        f.write(f"{i1}\n")
        for variant in ef_variants:
            label = variant["label"]
            vkey = variant["variant_key"]
            settings_str = ", ".join(
                f"{k}={v!r}" for k, v in catalog_gtdraw_settings(vkey).items()
            )
            f.write(f"{i2}.. tab-item:: {label}\n")
            f.write(f"{i2}\n")
            f.write(f"{i3}.. jupyter-execute::\n")
            f.write(f"{i3}   :hide-code:\n")
            f.write(f"{i3}   \n")
            f.write(f"{i4}import pygambit\n")
            f.write(f"{i4}from gtdraw import draw\n")
            if variant["ef_path"].exists():
                f.write(f'{i4}draw("../catalog/{vkey}.ef", {settings_str})\n')
            else:
                f.write(f'{i4}draw(pygambit.catalog.load("{slug}"), {settings_str})\n')
            f.write(f"{i2}\n")
        f.write(f"{i1}\n")
    else:
        f.write(f"{i1}.. jupyter-execute::\n")
        f.write(f"{i1}   :hide-code:\n")
        f.write(f"{i1}   \n")
        f.write(f"{i2}import pygambit\n")
        f.write(f"{i2}from gtdraw import draw\n")
        if row["Format"] == "efg":
            settings_str = ", ".join(
                f"{k}={v!r}" for k, v in catalog_gtdraw_settings(slug).items()
            )
            curated_ef = catalog_dir / f"{slug}.ef"
            if curated_ef.exists():
                f.write(f'{i2}draw("../catalog/{slug}.ef", {settings_str})\n')
            else:
                f.write(f'{i2}draw(pygambit.catalog.load("{slug}"), {settings_str})\n')
        elif row["Format"] == "nfg":
            f.write(
                f'{i2}draw(pygambit.catalog.load("{slug}"), '
                f'save_to="../catalog/img/{slug}.png")\n'
            )
        f.write(f"{i1}\n")


def _write_tree_level(
    f,
    subtree: dict,
    path_prefix: str,
    labels: dict[str, str],
    catalog_dir: Path,
    indent: str,
    regenerate_images: bool = False,
) -> None:
    """Recursively write RST nested dropdowns for *subtree*.

    Intermediate nodes (``dict`` values) become collapsible ``.. dropdown::``
    sections.  Top-level nodes (those whose ``path_prefix`` contains no ``/``)
    are rendered open by default so the first level of the hierarchy is
    immediately visible.  Leaf nodes (``pd.Series`` values) are individual
    games rendered via :func:`_write_game_entry`.
    """
    for key in sorted(subtree):
        value = subtree[key]
        child_prefix = f"{path_prefix}/{key}" if path_prefix else key

        if isinstance(value, dict):
            label = _node_label(child_prefix, labels)
            is_top_level = "/" not in child_prefix
            f.write(f"{indent}.. dropdown:: {label}\n")
            if is_top_level:
                f.write(f"{indent}   :open:\n")
            f.write(f"{indent}\n")
            _write_tree_level(
                f,
                value,
                child_prefix,
                labels,
                catalog_dir,
                indent + "   ",
                regenerate_images=regenerate_images,
            )
            f.write(f"{indent}\n")
        else:
            _write_game_entry(
                f,
                value,
                child_prefix,
                catalog_dir,
                indent,
                regenerate_images=regenerate_images,
            )


def generate_rst_table(
    df: pd.DataFrame,
    rst_path: Path,
    regenerate_images: bool = False,
    catalog_dir: Path | None = None,
):
    """Generate RST output with nested dropdowns grouped by catalog hierarchy."""
    catalog_dir = catalog_dir or CATALOG_DIR
    labels = load_hierarchy_labels()
    tree = _build_slug_tree(df)
    with open(rst_path, "w", encoding="utf-8") as f:
        _write_tree_level(
            f, tree, "", labels, catalog_dir, indent="", regenerate_images=regenerate_images
        )


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
            slugs.append(rel_path.as_posix())
    for resource_path in sorted(catalog_dir.rglob("*.nfg")):
        if resource_path.is_file():
            rel_path = resource_path.relative_to(catalog_dir)
            slugs.append(rel_path.as_posix())
    for resource_path in sorted(catalog_dir.rglob("*.ef")):
        # Exclude the generated .ef files under catalog/img/; only curated
        # .ef files committed alongside game files should be distributed.
        if resource_path.is_file() and catalog_dir / "img" not in resource_path.parents:
            rel_path = resource_path.relative_to(catalog_dir)
            slugs.append(rel_path.as_posix())

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
            "gtdraw_settings.yaml."
        ),
    )
    args = parser.parse_args()

    catalog_dir = use_catalog_dir()

    # Create RST list-table used by doc/catalog.rst
    df = gbt.catalog.games(include_descriptions=True)
    _warn_missing_descriptions(df)
    generate_rst_table(
        df,
        CATALOG_RST_TABLE,
        regenerate_images=args.regenerate_images,
        catalog_dir=catalog_dir,
    )
    print(f"Generated {CATALOG_RST_TABLE} for use in local docs build. DO NOT COMMIT.")
    if args.build:
        # Update the Makefile.am with the current list of catalog files
        update_makefile(catalog_dir=catalog_dir)
