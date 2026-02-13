
import pygambit as gbt


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
    g = gbt.Game.new_tree(players=["Buyer", "Seller"], title="One-shot trust game")
    g.description = "One-shot trust game with binary actions, originally from Kreps (1990)."
    g.append_move(g.root, "Buyer", ["Trust", "Not trust"])
    g.append_move(g.root.children[0], "Seller", ["Honor", "Abuse"])
    g.set_outcome(g.root.children[0].children[0], g.add_outcome([1, 1], label="Trustworthy"))
    if unique_NE_variant:
        g.set_outcome(
            g.root.children[0].children[1], g.add_outcome(["1/2", 2], label="Untrustworthy")
        )
    else:
        g.set_outcome(
            g.root.children[0].children[1], g.add_outcome([-1, 2], label="Untrustworthy")
        )
    g.set_outcome(g.root.children[1], g.add_outcome([0, 0], label="Opt-out"))
    return g
