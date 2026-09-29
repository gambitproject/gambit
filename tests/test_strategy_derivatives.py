"""Tests of MixedStrategyProfile::GetPayoffDerivBlock, which computes the payoffs of a player's
strategies and their derivatives with respect to other players' strategies together.

The block is compared, through a private helper, with the same quantities computed one at a
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
    data = []
    for player in game.players:
        weights = [rng.uniform(0.2, 1.0) for _ in game.get_strategies(player)]
        data.append([w / sum(weights) for w in weights])
    return game.mixed_strategy_profile(data=data, rational=False)


def _removals(game: gbt.Game) -> list[list[tuple[str, str]]]:
    """No removals; the first strategy of every player with more than one; and the last
    strategy of the first such player only."""
    players = [p for p in game.players if len(game.get_strategies(p)) > 1]
    return [
        [],
        [(p, game.get_strategies(p)[0]) for p in players],
        [(players[0], game.get_strategies(players[0])[-1])] if players else [],
    ]


@pytest.mark.parametrize(
    "game,tolerance",
    [
        pytest.param(_random_table_game((3, 4), seed=1), 0.0, id="table-3x4"),
        pytest.param(_random_table_game((3, 3, 4), seed=2), 0.0, id="table-3x3x4"),
        pytest.param(_random_table_game((2, 3, 2, 3), seed=3), 0.0, id="table-2x3x2x3"),
        pytest.param(games.read_from_file("2x2.agg"), 1.0e-12, id="agg-2x2"),
        pytest.param(games.read_from_file("Bayesian-Coffee-3-2-2-3.bagg"), 0.0,
                     id="bagg-coffee"),
        pytest.param(games.create_stripped_down_poker_efg(), 0.0, id="tree-poker"),
    ],
)
def test_payoff_deriv_block_matches_separate_computation(game: gbt.Game, tolerance: float):
    profile = _interior_profile(game, seed=4)
    for removed in _removals(game):
        assert profile._payoff_deriv_block_discrepancy(removed) <= tolerance
