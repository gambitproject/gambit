from pathlib import Path

import pygambit as gbt

CATALOG_CSV = "doc/catalog.csv"  # Relative to where script run from (root)
MAKEFILE_AM = Path(__file__).parent.parent.parent / "Makefile.am"
ALL_GAMES = gbt.catalog.games()
CATALOG_DIR = Path(__file__).parent.parent.parent / "catalog"
efg_files = list(CATALOG_DIR.rglob("*.efg"))
nfg_files = list(CATALOG_DIR.rglob("*.nfg"))


def update_makefile():
    """Update the Makefile.am with all games from the catalog."""

    game_files = []
    for entry in efg_files + nfg_files:
        filename = str(entry).split("/")[-1]
        game_files.append(f"catalog/{filename}")
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


if __name__ == "__main__":

    # Create CSV used by RST docs page
    ALL_GAMES.to_csv(CATALOG_CSV, index=False)
    print(f"Generated {CATALOG_CSV} for use in docs build.")

    # Update the Makefile.am with the current list of catalog files
    update_makefile()
