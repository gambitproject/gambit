"""Fixtures for testing the Python CLI tools (src/pygambit/cli/)."""

import pytest
from click.testing import CliRunner

import pygambit as gbt

from ..games import create_2x2_zero_sum_efg, root_node


@pytest.fixture
def cli_runner() -> CliRunner:
    """A `click.testing.CliRunner` for invoking CLI tools' `main` in-process."""
    return CliRunner()


def _table_game(payoffs: dict, title: str) -> gbt.Game:
    game = gbt.Game.new_table([2, 2])
    game.title = title
    p1, p2 = game.players
    s1a, s1b = game.get_strategies(p1)
    s2a, s2b = game.get_strategies(p2)
    strategies = {"a": s1a, "b": s1b, "A": s2a, "B": s2b}
    for (row, col), (v1, v2) in payoffs.items():
        game.make_outcome(
            {p1: strategies[row], p2: strategies[col]}, {p1: v1, p2: v2}, f"{row}{col}"
        )
    return game


@pytest.fixture
def nfg_matching_pennies_text() -> str:
    """A 2x2 constant-sum table game with a unique, fully mixed equilibrium, and no
    pure-strategy equilibria.  Suitable for tools that require a constant-sum game
    (e.g. `gambit-lp`), or where an empty pure-equilibrium result is expected.
    """
    game = _table_game(
        {
            ("a", "A"): (1, -1), ("a", "B"): (-1, 1),
            ("b", "A"): (-1, 1), ("b", "B"): (1, -1),
        },
        "Matching Pennies",
    )
    return game.to_nfg()


@pytest.fixture
def nfg_coordination_text() -> str:
    """A 2x2 table game with two pure-strategy equilibria and one mixed equilibrium."""
    game = _table_game(
        {
            ("a", "A"): (1, 1), ("a", "B"): (0, 0),
            ("b", "A"): (0, 0), ("b", "B"): (1, 1),
        },
        "Coordination",
    )
    return game.to_nfg()


@pytest.fixture
def nfg_asymmetric_table_text() -> str:
    """A 2x2 coordination-style table game whose payoffs aren't symmetric across
    players or strategies, so that a QRE branch traced from the centroid takes many
    steps to converge (unlike `nfg_coordination_text`/`nfg_matching_pennies_text`,
    whose symmetry lets the trace converge in 1-2 points) -- useful for exercising
    `gambit-logit`/`gambit-gnm`/`gambit-ipa`'s streamed intermediate output.
    """
    game = _table_game(
        {
            ("a", "A"): (3, 1), ("a", "B"): (0, 0),
            ("b", "A"): (0, 0), ("b", "B"): (1, 3),
        },
        "Asymmetric coordination",
    )
    return game.to_nfg()


@pytest.fixture
def efg_small_tree_text() -> str:
    """A small 2-player zero-sum extensive-form game, with a unique fully mixed
    equilibrium and no pure-strategy equilibria.
    """
    return create_2x2_zero_sum_efg().to_efg()


@pytest.fixture
def efg_asymmetric_tree_text() -> str:
    """A 2-player extensive-form game where player 1 has one information set (2
    actions) and player 2 has two information sets (3 and 2 actions), so that its
    behavior-form profile (2 + 3 + 2 = 7 values) and reduced-strategic-form profile
    (2 + 3*2 = 8 values) have different lengths -- useful for confirming that a
    -S/-A-style switch actually changes which representation is reported, rather
    than just checking the tool doesn't crash.

    Has several pure-strategy equilibria (all agreeing that player 1 plays "L" and
    player 2 plays "x" after it; player 2's action after the off-path "R" is
    payoff-irrelevant and so free to vary across equilibria).
    """
    game = gbt.Game.new_tree(players=["1", "2"], title="Asymmetric multi-infoset game")
    game.append_move(root_node(game), "1", ["L", "R"])
    left, right = root_node(game).children
    game.append_move(left, "2", ["x", "y", "z"])
    game.append_move(right, "2", ["p", "q"])
    for node in left.children:
        payoff = [1, 1] if node.prior_action.label == "x" else [0, 0]
        game.make_outcome(node, {"1": payoff[0], "2": payoff[1]}, node.prior_action.label)
    for node in right.children:
        game.make_outcome(node, {"1": 0, "2": 0}, node.prior_action.label)
    return game.to_efg()
