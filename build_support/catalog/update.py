import argparse
from pathlib import Path

import pygambit as gbt

CATALOG_CSV = Path(__file__).parent.parent.parent / "doc" / "catalog.csv"
CATALOG_DIR = Path(__file__).parent.parent.parent / "catalog"
MAKEFILE_AM = Path(__file__).parent.parent.parent / "Makefile.am"


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

    # Create CSV used by RST docs page
    gbt.catalog.games().to_csv(CATALOG_CSV, index=False)
    print(f"Generated {CATALOG_CSV} for use in local docs build. DO NOT COMMIT.")

    # Update the Makefile.am with the current list of catalog files
    if args.build:
        update_makefile()
