import pygambit as gbt

DOC = "../../doc"

if __name__ == "__main__":
    # Create CSV used by RST docs page
    gbt.catalog.games().to_csv(DOC + "/catalog.csv", index=False)
