import pytest

import pygambit as gbt


def test_strategic_game_actions():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(gbt.UndefinedOperationError):
        _ = game.actions


def test_strategic_game_infosets():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(gbt.UndefinedOperationError):
        _ = game.infosets


def test_strategic_game_root():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(gbt.UndefinedOperationError):
        _ = game.root


def test_strategic_game_nodes():
    game = gbt.Game.new_table([2, 2])
    assert game.nodes() == []


def test_game_behav_profile_error():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(gbt.UndefinedOperationError):
        _ = game.mixed_behavior_profile()


def test_game_is_const_sum():
    game = gbt.Game.read_game("test_games/const_sum_game.nfg")
    assert game.is_const_sum


def test_game_is_not_const_sum():
    game = gbt.Game.read_game("test_games/non_const_sum_game.nfg")
    assert not game.is_const_sum


def test_game_get_min_payoff():
    game = gbt.Game.read_game("test_games/payoff_game.nfg")
    assert game.min_payoff == 1


def test_game_get_max_payoff():
    game = gbt.Game.read_game("test_games/payoff_game.nfg")
    assert game.max_payoff == 10
