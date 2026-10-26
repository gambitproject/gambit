"""Tests that `maxregret` is scaled by the same payoff range whether a game is held as a tree
or as an explicit table (issue #1168)."""

import io

import pytest

import pygambit as gbt

from . import games


def _as_table(game: gbt.Game) -> gbt.Game:
    return gbt.read_nfg(io.StringIO(game.to_nfg()))


def _solve_logit_strategic(game: gbt.Game, maxregret: float):
    events = []
    result = gbt.nash.logit_solve(
        game, use_strategic=True, maxregret=maxregret, event_callback=events.append
    )
    return result, sum(isinstance(e, gbt.LogitPathEvent) for e in events)


@pytest.mark.parametrize(
    "factory",
    [games.create_stripped_down_poker_efg, games.create_kuhn_poker_efg],
    ids=["stripped_down_poker", "kuhn_poker"],
)
@pytest.mark.parametrize("maxregret", [1.0e-1, 1.0e-2, 1.0e-4])
def test_logit_strategic_termination_same_for_tree_and_table(factory, maxregret):
    tree = factory()
    table = _as_table(tree)
    # The ranges of single plays differ; only the expected payoff range is common to both.
    assert (tree.min_payoff, tree.max_payoff) != (table.min_payoff, table.max_payoff)

    tree_result, tree_points = _solve_logit_strategic(tree, maxregret)
    table_result, table_points = _solve_logit_strategic(table, maxregret)

    assert tree_result.success and table_result.success
    assert tree_points == table_points
    for player in table.players:
        for strategy in table.get_strategies(player):
            assert tree_result.equilibrium[player][strategy] == pytest.approx(
                table_result.equilibrium[player][strategy], abs=1.0e-9
            )


@pytest.mark.parametrize("maxregret", [1.0e-1, 1.0e-2, 1.0e-4])
def test_hp_termination_same_for_tree_and_table(maxregret):
    tree = games.create_stripped_down_poker_efg()
    table = _as_table(tree)

    def solve(game):
        events = []
        result = gbt.nash.hp_solve(
            game.mixed_strategy_profile(), maxregret=maxregret, event_callback=events.append
        )
        return result, len(events)

    tree_result, tree_points = solve(tree)
    table_result, table_points = solve(table)
    assert tree_result.success and table_result.success
    assert tree_points == table_points
