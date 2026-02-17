from importlib.resources import as_file, files
from pathlib import Path

import pandas as pd

import pygambit as gbt

# Use the full string path to the virtual package we created
_CATALOG_RESOURCE = files(__name__)

READERS = {
    ".nfg": gbt.read_nfg,
    ".efg": gbt.read_efg,
}


def load(slug: str) -> gbt.Game:
    """
    Load a game from the package catalog.
    """
    slug = str(Path(slug)).replace("\\", "/")

    # Try to load from file
    for suffix, reader in READERS.items():
        resource_path = _CATALOG_RESOURCE / f"{slug}{suffix}"
        if resource_path.is_file():
            with as_file(resource_path) as path:
                return reader(str(path))

    # Try loading from family games
    fg = family_games()
    if slug in fg:
        return fg[slug]

    # Raise error if game does not exist
    raise FileNotFoundError(f"No catalog entry called {slug}")


def games() -> pd.DataFrame:
    """
    List games available in the package catalog, including subdirectories.
    """
    records: list[dict[str, str]] = []

    # Add all the games stored as EFG/NFG files
    for resource_path in sorted(_CATALOG_RESOURCE.rglob("*")):
        reader = READERS.get(resource_path.suffix)

        if reader is not None and resource_path.is_file():
            # Calculate the path relative to the root resource
            # and remove the suffix to get the "slug"
            rel_path = resource_path.relative_to(_CATALOG_RESOURCE)
            slug = rel_path.with_suffix("").as_posix()

            with as_file(resource_path) as path:
                game = reader(str(path))
                records.append(
                    {
                        "Game": slug,
                        "Title": game.title,
                    }
                )

    # Add all the games from families
    for slug, game in family_games().items():
        # Throw an error if there's a slug collision between family games and file-based games
        if slug in records:
            raise ValueError(
                f"Slug collision: {slug} is present in both file-based and "
                "family games."
            )
        records.append(
            {
                "Game": slug,
                "Title": game.title,
            }
        )

    return pd.DataFrame.from_records(records, columns=["Game", "Title"])


def family_games() -> dict[str, gbt.Game]:
    """
    Generate a dict of games for inclusion in the catalog,
    using the game families in this module.
    """
    return {
        "one_shot_trust": one_shot_trust(),
        "oneshot_trust_unique_NE": one_shot_trust(unique_NE_variant=True),
    }


################################################################################################
# Families


def one_shot_trust(unique_NE_variant: bool = False) -> gbt.Game:
    """
    The unique_NE_variant makes Trust a dominant strategy, replacing the
    non-singleton equilibrium component from the standard version of the game
    where the Buyer plays "Not Trust" and the seller can play any mixture with
    < 0.5 probability on Honor with a unique NE where the Buyer plays Trust and
    the Seller plays Abuse.

    Parameters
    ----------
    unique_NE_variant : bool, optional
        Whether to modify the game so that it has a unique Nash equilibrium.
        Defaults to False.

    Returns
    -------
    gbt.Game
        The constructed extensive-form game.
    """
    g = gbt.Game.new_tree(players=["Buyer", "Seller"])
    g.description = "One-shot trust game with binary actions, originally from Kreps (1990)."
    g.append_move(g.root, "Buyer", ["Trust", "Not trust"])
    g.append_move(g.root.children[0], "Seller", ["Honor", "Abuse"])
    g.set_outcome(g.root.children[0].children[0], g.add_outcome([1, 1], label="Trustworthy"))
    if unique_NE_variant:
        g.title = "One-shot trust game with unique NE"
        g.set_outcome(
            g.root.children[0].children[1], g.add_outcome(["1/2", 2], label="Untrustworthy")
        )
    else:
        g.title = "One-shot trust game"
        g.set_outcome(
            g.root.children[0].children[1], g.add_outcome([-1, 2], label="Untrustworthy")
        )
    g.set_outcome(g.root.children[1], g.add_outcome([0, 0], label="Opt-out"))
    return g
