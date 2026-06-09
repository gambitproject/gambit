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
    pl1, pl2 = game.players
    assert len(game.players) == 2
    assert len(pl1.strategies) == 2
    assert len(pl2.strategies) == 2


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
    pl1, pl2 = game.players
    assert len(game.players) == 2
    assert len(pl1.strategies) == 2
    assert len(pl2.strategies) == 2
    assert pl1.label == "a"
    assert pl2.label == "b"


def test_game_get_outcome_by_index():
    game = gbt.Game.new_table([2, 2])
    assert game[0, 0] == next(iter(game.outcomes))


def test_game_get_outcome_by_label():
    game = gbt.Game.new_table([2, 2])
    pl1, pl2 = game.players
    next(iter(pl1.strategies)).label = "defect"
    next(iter(pl2.strategies)).label = "cooperate"
    assert game["defect", "cooperate"] == next(iter(game.outcomes))


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
    pl1, pl2 = game.players
    next(iter(pl1.strategies)).label = "defect"
    next(iter(pl2.strategies)).label = "cooperate"
    with pytest.raises(IndexError):
        _ = game["defect", "defect"]


def test_game_get_outcome_with_strategies():
    game = gbt.Game.new_table([2, 2])
    pl1, pl2 = game.players
    assert (
        game[next(iter(pl1.strategies)), next(iter(pl2.strategies))]
        == next(iter(game.outcomes))
    )


def test_game_get_outcome_with_bad_strategies():
    game = gbt.Game.new_table([2, 2])
    player = next(iter(game.players))
    strategy = next(iter(player.strategies))
    with pytest.raises(IndexError):
        _ = game[strategy, strategy]


def test_game_dereference_invalid():
    game = gbt.Game.new_tree()
    player = game.add_player("One")
    strategy = next(iter(player.strategies))
    game.append_move(game.root, player, ["a", "b"])
    with pytest.raises(RuntimeError):
        _ = strategy.label


def test_mixed_strategy_profile_game_structure_changed_no_tree():
    game = gbt.Game.from_arrays([[2, 2], [0, 0]], [[0, 0], [1, 1]])
    profiles = [game.mixed_strategy_profile(rational=b) for b in [False, True]]
    player = next(iter(game.players))
    strategy1, strategy2, *_ = game.strategies
    next(iter(game.outcomes))[player] = 3
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
            profile.payoff(player)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.player_regret(player)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.strategy_regret(strategy1)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.strategy_value(strategy1)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.strategy_value_deriv(strategy1, strategy2)
        with pytest.raises(gbt.GameStructureChangedError):
            # triggers error via __getitem__
            next(profile.__iter__())
        with pytest.raises(gbt.GameStructureChangedError):
            profile.__setitem__(strategy1, 0)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.__getitem__(strategy1)


def test_mixed_strategy_profile_game_structure_changed_tree():
    game = games.read_from_file("basic_extensive_game.efg")
    profiles = [game.mixed_strategy_profile(rational=b) for b in [False, True]]
    player = next(iter(game.players))
    action_to_delete = game.root.infoset.actions["U1"]
    game.delete_action(action_to_delete)
    strategy1, strategy2, *_ = game.strategies
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
            profile.payoff(player)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.player_regret(player)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.strategy_regret(strategy1)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.strategy_value(strategy1)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.strategy_value_deriv(strategy1, strategy2)
        with pytest.raises(gbt.GameStructureChangedError):
            # triggers error via __getitem__
            next(profile.__iter__())
        with pytest.raises(gbt.GameStructureChangedError):
            profile.__setitem__(strategy1, 0)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.__getitem__(strategy1)


def test_mixed_behavior_profile_game_structure_changed():
    game = games.read_from_file("basic_extensive_game.efg")
    profiles = [game.mixed_behavior_profile(rational=b) for b in [False, True]]
    player = next(iter(game.players))
    action_to_delete = game.root.infoset.actions["U1"]
    game.delete_action(action_to_delete)
    action = next(iter(game.actions))
    infoset = next(iter(game.infosets))
    infoset_action = next(iter(infoset.actions))
    for profile in profiles:
        with pytest.raises(gbt.GameStructureChangedError):
            profile.action_regret(action)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.action_value(action)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.as_strategy()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.belief(next(iter(game.nodes)))
        with pytest.raises(gbt.GameStructureChangedError):
            profile.copy()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.infoset_prob(infoset)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.infoset_regret(infoset)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.infoset_value(infoset)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.is_defined_at(infoset)
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
            profile.node_value(player, game.root)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.normalize()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.payoff(player)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.realiz_prob(game.root)
        with pytest.raises(gbt.GameStructureChangedError):
            # triggers error via __getitem__
            next(profile.__iter__())
        with pytest.raises(gbt.GameStructureChangedError):
            profile.__setitem__(infoset_action, 0)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.__getitem__(infoset)
