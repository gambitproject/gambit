"""Tests of the partial derivatives of payoffs with respect to strategy probabilities.

Through private helpers, the derivatives computed one at a time are compared with central
differences of the payoffs, and MixedStrategyProfile::GetPayoffDerivBlock, which computes a
player's payoffs and derivatives together, is compared with the derivatives computed one at a
time, on the full support and on supports with strategies removed.
"""
import random

import numpy as np
import pytest

import pygambit as gbt

from . import games


def _random_table_game(shape: tuple[int, ...], seed: int) -> gbt.Game:
    rng = np.random.default_rng(seed)
    return gbt.StrategicGame.from_arrays(*[rng.integers(0, 100, shape) for _ in shape])


def _interior_profile(game: gbt.Game, seed: int):
    rng = random.Random(seed)
    if isinstance(game, gbt.BayesianActionGraphGame):
        profile = game.mixed_behavior_profile()
        for player in game.players:
            for infoset in game.get_infosets(player):
                actions = [action for action, _ in profile[player, infoset]]
                weights = [rng.uniform(0.2, 1.0) for _ in actions]
                profile[player, infoset] = {
                    action: w / sum(weights) for action, w in zip(actions, weights, strict=True)
                }
        return profile
    data = []
    for player in game.players:
        weights = [rng.uniform(0.2, 1.0) for _ in game.get_strategies(player)]
        data.append([w / sum(weights) for w in weights])
    return game.mixed_strategy_profile(data=data, rational=False)


def _bagg_removals(game: gbt.BayesianActionGraphGame, profile) -> list[list[tuple]]:
    """As `_removals`, for the agents (player, information set) of a Bayesian game, whose
    strategies are the information set's actions."""
    agents = [
        (player, infoset, [action for action, _ in profile[player, infoset]])
        for player in game.players
        for infoset in game.get_infosets(player)
    ]
    agents = [agent for agent in agents if len(agent[2]) > 1]
    return [
        [],
        [(player, infoset, actions[0]) for player, infoset, actions in agents],
        [(agents[0][0], agents[0][1], agents[0][2][-1])] if agents else [],
    ]


def _removals(game: gbt.Game) -> list[list[tuple[str, str]]]:
    """No removals; the first strategy of every player with more than one; and the last
    strategy of the first such player only."""
    players = [p for p in game.players if len(game.get_strategies(p)) > 1]
    return [
        [],
        [(p, game.get_strategies(p)[0]) for p in players],
        [(players[0], game.get_strategies(players[0])[-1])] if players else [],
    ]


GAMES = [
    pytest.param(_random_table_game((3, 4), seed=1), 0.0, id="table-3x4"),
    pytest.param(_random_table_game((3, 3, 4), seed=2), 0.0, id="table-3x3x4"),
    pytest.param(_random_table_game((2, 3, 2, 3), seed=3), 0.0, id="table-2x3x2x3"),
    pytest.param(games.read_from_file("2x2.agg"), 1.0e-12, id="agg-2x2"),
    pytest.param(games.read_from_file("2x2.bagg"), 1.0e-12, id="bagg-2x2"),
    pytest.param(games.read_from_file("2x2_fraction_types.bagg"), 1.0e-12,
                 id="bagg-2x2-fraction-types"),
    pytest.param(games.read_from_file("Bayesian-Coffee-3-2-2-3.bagg"), 1.0e-12,
                 id="bagg-coffee"),
    pytest.param(games.create_stripped_down_poker_efg(), 1.0e-12, id="tree-poker"),
    pytest.param(games.read_from_file("chance_in_middle_with_nonterm_outcomes.efg"), 1.0e-12,
                 id="tree-chance-in-middle"),
]


@pytest.mark.parametrize("game,block_tolerance", GAMES)
def test_payoff_derivs_are_partial_derivatives(game: gbt.Game, block_tolerance: float):
    profile = _interior_profile(game, seed=4)
    assert profile._payoff_deriv_finite_difference_discrepancy(1.0e-3) <= 1.0e-8


@pytest.mark.parametrize("game,block_tolerance", GAMES)
def test_payoff_deriv_block_matches_separate_computation(game: gbt.Game, block_tolerance: float):
    profile = _interior_profile(game, seed=4)
    removals = (
        _bagg_removals(game, profile)
        if isinstance(game, gbt.BayesianActionGraphGame)
        else _removals(game)
    )
    for removed in removals:
        assert profile._payoff_deriv_block_discrepancy(removed) <= block_tolerance
