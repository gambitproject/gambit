import pygambit as gbt

CATALOG_CSV = "doc/catalog.csv"

if __name__ == "__main__":
    # Create CSV used by RST docs page
    gbt.catalog.games().to_csv(CATALOG_CSV, index=False)
