import pytest
import numpy as np

import pygambit as gbt


def test_from_arrays():
    m = np.array([[8, 2], [10, 5]])
    game = gbt.Game.from_arrays(m, m.transpose())
    assert len(game.players) == 2
    assert len(game.players[0].strategies) == 2
    assert len(game.players[1].strategies) == 2


def test_game_get_outcome_by_index():
    game = gbt.Game.new_table([2, 2])
    assert game[0, 0] == game.outcomes[0]


def test_game_get_outcome_by_label():
    game = gbt.Game.new_table([2, 2])
    game.players[0].strategies[0].label = "defect"
    game.players[1].strategies[0].label = "cooperate"
    assert game["defect", "cooperate"] == game.outcomes[0]


def test_game_get_outcome_invalid_tuple_size():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(KeyError):
        _ = game[0, 0, 0]


def test_game_outcomes_non_tuple():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(TypeError):
        _ = game[42]


def test_game_outcomes_type_exception():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(TypeError):
        _ = game[1.23, 1]


def test_game_get_outcome_index_out_of_range():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(IndexError):
        _ = game[0, 3]


def test_game_get_outcome_unmatched_label():
    game = gbt.Game.new_table([2, 2])
    game.players[0].strategies[0].label = "defect"
    game.players[1].strategies[0].label = "cooperate"
    with pytest.raises(IndexError):
        _ = game["defect", "defect"]


def test_game_get_outcome_with_strategies():
    game = gbt.Game.new_table([2, 2])
    assert (
        game[game.players[0].strategies[0], game.players[1].strategies[0]] ==
        game.outcomes[0]
    )


def test_game_get_outcome_with_bad_strategies():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(IndexError):
        _ = game[game.players[0].strategies[0], game.players[0].strategies[0]]


def test_game_dereference_invalid():
    game = gbt.Game.new_tree()
    game.add_player("One")
    strategy = game.players[0].strategies[0]
    game.append_move(game.root, game.players[0], ["a", "b"])
    with pytest.raises(RuntimeError):
        _ = strategy.label


def test_exceptions():
    """Tests for certain exceptions that are caught at the cython level via 'except +'
    The tests here are only for function calls that do not give a RuntimeError without 'except +'
    """
    # table
    g = gbt.Game.new_table([2, 2])
    p = g.mixed_strategy_profile()
    p_rat = g.mixed_strategy_profile(rational=True)
    g.delete_strategy(g.players[0].strategies[0])
    with pytest.raises(RuntimeError):
        p.payoff(g.players[0])
    with pytest.raises(RuntimeError):
        p.liap_value()
    with pytest.raises(RuntimeError):
        p_rat.payoff(g.players[0])
    with pytest.raises(RuntimeError):
        p_rat.liap_value()
    # tree
    g = gbt.Game.read_game("test_games/basic_extensive_game.efg")
    p_behav = g.mixed_behavior_profile()
    p_behav_rat = g.mixed_behavior_profile(rational=True)
    iset = g.players[0].infosets[0]
    g.delete_action(iset.actions[0])
    with pytest.raises(RuntimeError):
        p_behav.payoff(g.players[0])
    with pytest.raises(RuntimeError):
        p_behav.liap_value()
    with pytest.raises(RuntimeError):
        p_behav_rat.payoff(g.players[0])
    with pytest.raises(RuntimeError):
        p_behav_rat.liap_value()
