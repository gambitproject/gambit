"""Tests of generic Game operations on action graph game (AGG/BAGG) representations.
"""

import pytest

from . import games

AGG_BAGG_GAME_PATHS = [
    "2x2.agg",
    "2x2_small_payoffs.agg",
    "2x2.bagg",
    "Bayesian-Coffee-3-2-2-3.bagg",
]


@pytest.mark.parametrize("game_path", AGG_BAGG_GAME_PATHS)
def test_agg_bagg_is_const_sum(game_path):
    game = games.read_from_file(game_path)
    assert not game.is_const_sum


@pytest.mark.parametrize(
    "game_path,exp_min,exp_max",
    [
        ("2x2.agg", -10, 95),
        ("2x2.bagg", -10, 95),
        ("Bayesian-Coffee-3-2-2-3.bagg", 0, 99),
    ],
)
def test_agg_bagg_get_min_max_payoff(game_path, exp_min, exp_max):
    game = games.read_from_file(game_path)
    assert game.min_payoff == exp_min
    assert game.max_payoff == exp_max


@pytest.mark.parametrize("game_path", AGG_BAGG_GAME_PATHS)
def test_agg_bagg_to_nfg(game_path):
    game = games.read_from_file(game_path)
    serialized_game = game.to_nfg()
    assert serialized_game[:3] == "NFG"
