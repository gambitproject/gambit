import numpy as np
import pytest

import pygambit as gbt

from . import games


def test_constructor_fail():
    with pytest.raises(ValueError):
        gbt.Game()


def test_from_arrays():
    m = np.array([[8, 2], [10, 5]])
    game = gbt.Game.from_arrays(m, m.transpose())
    assert len(game.players) == 2
    assert len(game.players[0].strategies) == 2
    assert len(game.players[1].strategies) == 2


def test_empty_array_to_arrays():
    game = gbt.Game.from_arrays([])
    a = game.to_arrays()
    assert len(a) == 1
    assert (a[0] == np.array([])).all()


def test_to_arrays_wrong_type():
    m = np.array([[8, 2], [10, 5]])
    game = gbt.Game.from_arrays(m, m.transpose())
    with pytest.raises(ValueError):
        _ = game.to_arrays(dtype=dict)


def test_different_num_representations_to_arrays_fraction():
    game = gbt.Game.from_arrays([1, 2 / 1, "6/2", 0.25, ".99"])
    A = game.to_arrays()[0]
    correct_output = [gbt.Rational(1, 1), gbt.Rational(2, 1), gbt.Rational(3, 1),
                      gbt.Rational(1, 4), gbt.Rational(99, 100)]
    assert (correct_output == A).all()


def test_different_num_representations_to_arrays_float():
    game = gbt.Game.from_arrays([1, 2 / 1, "6/2", 0.25, ".99"])
    A = game.to_arrays(dtype=float)[0]
    correct_output = [1.0, 2.0, 3.0, 0.25, 0.99]
    assert (correct_output == A).all()


def test_2d_to_arrays():
    m = np.array([[8, 2], [10, 5]])
    game = gbt.Game.from_arrays(m, m.transpose())
    payoff, payoff_t = game.to_arrays()
    assert (m == payoff).all()
    assert (m.transpose() == payoff_t).all()


def test_3d_to_arrays():
    a = np.array(
        [
            [[1, -1], [4, -4], [100, -100]],
            [[2, -2], [5, -5], [101, -101]],
            [[3, -3], [6, -6], [102, -102]],
        ]
    )
    b = np.array(
        [
            [[7, -7], [10, -10], [103, -103]],
            [[8, -8], [11, -11], [104, -104]],
            [[9, -9], [12, -12], [105, -105]],
        ]
    )
    c = np.array(
        [
            [[13, -13], [16, -16], [106, -106]],
            [[14, -14], [17, -17], [107, -107]],
            [[15, -15], [18, -18], [108, -108]],
        ]
    )
    game = gbt.Game.from_arrays(a, b, c)
    a_, b_, c_ = game.to_arrays()
    assert (a == a_).all()
    assert (b == b_).all()
    assert (c == c_).all()


def test_from_dict():
    m = np.array([[8, 2], [10, 5]])
    game = gbt.Game.from_dict({"a": m, "b": m.transpose()})
    assert len(game.players) == 2
    assert len(game.players[0].strategies) == 2
    assert len(game.players[1].strategies) == 2
    assert game.players[0].label == "a"
    assert game.players[1].label == "b"


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
    assert game[game.players[0].strategies[0], game.players[1].strategies[0]] == game.outcomes[0]


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


def test_mixed_strategy_profile_game_structure_changed_no_tree():
    g = gbt.Game.from_arrays([[2, 2], [0, 0]], [[0, 0], [1, 1]])
    profiles = [g.mixed_strategy_profile(rational=b) for b in [False, True]]
    g.outcomes[0][g.players[0]] = 3
    for profile in profiles:
        with pytest.raises(gbt.GameStructureChangedError):
            profile.copy()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.liap_value()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.max_regret()
        with pytest.raises(gbt.GameStructureChangedError):
            # triggers error via __getitem__
            next(profile.mixed_strategies())
        with pytest.raises(gbt.GameStructureChangedError):
            profile.normalize()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.copy()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.liap_value()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.max_regret()
        with pytest.raises(gbt.GameStructureChangedError):
            # triggers error via __getitem__
            next(profile.mixed_strategies())
        with pytest.raises(gbt.GameStructureChangedError):
            profile.normalize()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.payoff(g.players[0])
        with pytest.raises(gbt.GameStructureChangedError):
            profile.player_regret(g.players[0])
        with pytest.raises(gbt.GameStructureChangedError):
            profile.strategy_regret(g.strategies[0])
        with pytest.raises(gbt.GameStructureChangedError):
            profile.strategy_value(g.strategies[0])
        with pytest.raises(gbt.GameStructureChangedError):
            profile.strategy_value_deriv(g.strategies[0], g.strategies[1])
        with pytest.raises(gbt.GameStructureChangedError):
            # triggers error via __getitem__
            next(profile.__iter__())
        with pytest.raises(gbt.GameStructureChangedError):
            profile.__setitem__(g.strategies[0], 0)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.__getitem__(g.strategies[0])


def test_mixed_strategy_profile_game_structure_changed_tree():
    g = games.read_from_file("basic_extensive_game.efg")
    profiles = [g.mixed_strategy_profile(rational=b) for b in [False, True]]
    g.delete_action(g.players[0].infosets[0].actions[0])
    for profile in profiles:
        with pytest.raises(gbt.GameStructureChangedError):
            profile.as_behavior()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.copy()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.liap_value()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.max_regret()
        with pytest.raises(gbt.GameStructureChangedError):
            # triggers error via __getitem__
            next(profile.mixed_strategies())
        with pytest.raises(gbt.GameStructureChangedError):
            profile.normalize()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.payoff(g.players[0])
        with pytest.raises(gbt.GameStructureChangedError):
            profile.player_regret(g.players[0])
        with pytest.raises(gbt.GameStructureChangedError):
            profile.strategy_regret(g.strategies[0])
        with pytest.raises(gbt.GameStructureChangedError):
            profile.strategy_value(g.strategies[0])
        with pytest.raises(gbt.GameStructureChangedError):
            profile.strategy_value_deriv(g.strategies[0], g.strategies[1])
        with pytest.raises(gbt.GameStructureChangedError):
            # triggers error via __getitem__
            next(profile.__iter__())
        with pytest.raises(gbt.GameStructureChangedError):
            profile.__setitem__(g.strategies[0], 0)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.__getitem__(g.strategies[0])


def test_mixed_behavior_profile_game_structure_changed():
    g = games.read_from_file("basic_extensive_game.efg")
    profiles = [g.mixed_behavior_profile(rational=b) for b in [False, True]]
    g.delete_action(g.players[0].infosets[0].actions[0])
    for profile in profiles:
        with pytest.raises(gbt.GameStructureChangedError):
            profile.action_regret(g.actions[0])
        with pytest.raises(gbt.GameStructureChangedError):
            profile.action_value(g.actions[0])
        with pytest.raises(gbt.GameStructureChangedError):
            profile.as_strategy()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.belief(list(g.nodes)[0])
        with pytest.raises(gbt.GameStructureChangedError):
            profile.copy()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.infoset_prob(g.infosets[0])
        with pytest.raises(gbt.GameStructureChangedError):
            profile.infoset_regret(g.infosets[0])
        with pytest.raises(gbt.GameStructureChangedError):
            profile.infoset_value(g.infosets[0])
        with pytest.raises(gbt.GameStructureChangedError):
            profile.is_defined_at(g.infosets[0])
        with pytest.raises(gbt.GameStructureChangedError):
            profile.agent_liap_value()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.liap_value()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.agent_max_regret()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.max_regret()
        with pytest.raises(gbt.GameStructureChangedError):
            # triggers error via __getitem__
            next(profile.mixed_actions())
        with pytest.raises(gbt.GameStructureChangedError):
            # triggers error via __getitem__
            next(profile.mixed_behaviors())
        with pytest.raises(gbt.GameStructureChangedError):
            profile.node_value(g.players[0], g.root)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.normalize()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.payoff(g.players[0])
        with pytest.raises(gbt.GameStructureChangedError):
            profile.liap_value()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.max_regret()
        with pytest.raises(gbt.GameStructureChangedError):
            # triggers error via __getitem__
            next(profile.mixed_actions())
        with pytest.raises(gbt.GameStructureChangedError):
            # triggers error via __getitem__
            next(profile.mixed_behaviors())
        with pytest.raises(gbt.GameStructureChangedError):
            profile.node_value(g.players[0], g.root)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.normalize()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.payoff(g.players[0])
        with pytest.raises(gbt.GameStructureChangedError):
            profile.realiz_prob(g.root)
        with pytest.raises(gbt.GameStructureChangedError):
            # triggers error via __getitem__
            next(profile.__iter__())
        with pytest.raises(gbt.GameStructureChangedError):
            profile.__setitem__(g.infosets[0].actions[0], 0)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.__getitem__(g.infosets[0])
