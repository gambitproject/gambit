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


def test_game_get_outcome():
    game = gbt.Game.new_table([2, 2])
    game.make_outcome({"1": "1", "2": "1"}, {"1": 0, "2": 0}, "top left")
    assert game.get_outcome({"1": "1", "2": "1"}) == next(iter(game.outcomes))


def test_game_get_outcome_by_relabeled_strategies():
    game = gbt.Game.new_table([2, 2])
    pl1, pl2 = game.players
    game.relabel_strategies(pl1, {next(iter(pl1.strategies)): "defect"})
    game.relabel_strategies(pl2, {next(iter(pl2.strategies)): "cooperate"})
    game.make_outcome({pl1.label: "defect", pl2.label: "cooperate"}, {"1": 0, "2": 0}, "corner")
    assert game.get_outcome({pl1.label: "defect", pl2.label: "cooperate"}) == \
        next(iter(game.outcomes))


def test_game_get_outcome_incomplete_contingency_raises():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(ValueError):
        _ = game.get_outcome({"1": "1"})


def test_game_get_outcome_unknown_player_raises():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(KeyError):
        _ = game.get_outcome({"1": "1", "2": "1", "3": "1"})


def test_game_get_outcome_non_mapping_raises():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(TypeError):
        _ = game.get_outcome(42)


def test_game_get_outcome_non_str_value_raises():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(TypeError):
        _ = game.get_outcome({"1": 1.23, "2": "1"})


def test_game_get_outcome_unknown_strategy_label_raises():
    game = gbt.Game.new_table([2, 2])
    with pytest.raises(KeyError):
        _ = game.get_outcome({"1": "1", "2": "99"})


def test_game_get_outcome_unmatched_label_after_relabel_raises():
    game = gbt.Game.new_table([2, 2])
    pl1, pl2 = game.players
    game.relabel_strategies(pl1, {next(iter(pl1.strategies)): "defect"})
    game.relabel_strategies(pl2, {next(iter(pl2.strategies)): "cooperate"})
    with pytest.raises(KeyError):
        _ = game.get_outcome({pl1.label: "defect", pl2.label: "defect"})


def test_game_get_outcome_player_object_key_raises():
    game = gbt.Game.new_table([2, 2])
    pl1, pl2 = game.players
    with pytest.raises(TypeError):
        _ = game.get_outcome({pl1: "1", pl2.label: "1"})


def test_game_get_outcome_tree_raises():
    game = gbt.Game.new_tree(["Alice"])
    game.append_move(game.root, "Alice", ["a", "b"])
    with pytest.raises(gbt.UndefinedOperationError):
        _ = game.get_outcome({"Alice": "a"})


def test_game_get_payoffs():
    game = gbt.Game.new_table([2, 2])
    game.make_outcome({"1": "1", "2": "1"}, {"1": 3, "2": -3}, "top left")
    payoffs = game.get_payoffs({"1": "1", "2": "1"})
    assert payoffs["1"] == 3
    assert payoffs["2"] == -3


def test_game_get_payoffs_tree():
    game = gbt.Game.new_tree(["Alice"])
    game.append_move(game.root, "Alice", ["a", "b"])
    alice = game.players["Alice"]
    infoset = game.root.infoset
    strategy = next(
        s for s in alice.strategies if game.get_behavior("Alice", s).get(infoset).label == "a"
    )
    game.make_outcome(game.root.children["a"], {"Alice": 1}, "a-outcome")
    payoffs = game.get_payoffs({"Alice": strategy})
    assert payoffs["Alice"] == 1


def test_mixed_strategy_profile_game_structure_changed_no_tree():
    game = gbt.Game.from_arrays([[2, 2], [0, 0]], [[0, 0], [1, 1]])
    profiles = [game.mixed_strategy_profile(rational=b) for b in [False, True]]
    player = next(iter(game.players))
    distribution = {s: 0 for s in player.strategies}
    next(iter(game.outcomes))[player] = 3
    for profile in profiles:
        with pytest.raises(gbt.GameStructureChangedError):
            profile.copy()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.liap_value()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.max_regret()
        with pytest.raises(gbt.GameStructureChangedError):
            profile.normalize()
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.payoffs
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.player_regrets
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.strategy_regrets
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.strategy_values
        with pytest.raises(gbt.GameStructureChangedError):
            # triggers error via __getitem__
            next(profile.__iter__())
        with pytest.raises(gbt.GameStructureChangedError):
            profile.__setitem__(player.label, distribution)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.set_mixed_strategy(player.label, distribution)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.__getitem__(player.label)


def test_mixed_strategy_profile_game_structure_changed_tree():
    game = games.read_from_file("basic_extensive_game.efg")
    profiles = [game.mixed_strategy_profile(rational=b) for b in [False, True]]
    player = next(iter(game.players))
    game.set_move_actions(game.root, ["D1"], drop=True)
    distribution = {s: 0 for s in player.strategies}
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
            profile.normalize()
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.payoffs
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.player_regrets
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.strategy_regrets
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.strategy_values
        with pytest.raises(gbt.GameStructureChangedError):
            # triggers error via __getitem__
            next(profile.__iter__())
        with pytest.raises(gbt.GameStructureChangedError):
            profile.__setitem__(player.label, distribution)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.set_mixed_strategy(player.label, distribution)
        with pytest.raises(gbt.GameStructureChangedError):
            profile.__getitem__(player.label)


def test_mixed_behavior_profile_game_structure_changed():
    game = games.read_from_file("basic_extensive_game.efg")
    profiles = [game.mixed_behavior_profile(rational=b) for b in [False, True]]
    game.set_move_actions(game.root, ["D1"], drop=True)
    infoset = game.root
    for profile in profiles:
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.action_regrets
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.action_values
        with pytest.raises(gbt.GameStructureChangedError):
            profile.as_strategy()
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.beliefs
        with pytest.raises(gbt.GameStructureChangedError):
            profile.copy()
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.infoset_probs
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.infoset_regrets
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.infoset_values
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
            next(profile.__iter__())
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.node_values
        with pytest.raises(gbt.GameStructureChangedError):
            profile.normalize()
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.payoffs
        with pytest.raises(gbt.GameStructureChangedError):
            _ = profile.realiz_probs
        with pytest.raises(gbt.GameStructureChangedError):
            # triggers error via __getitem__
            next(profile.__iter__())
        with pytest.raises(gbt.GameStructureChangedError):
            profile.__setitem__(game.root, {})
        with pytest.raises(gbt.GameStructureChangedError):
            profile.set_mixed_action(game.root, {})
        with pytest.raises(gbt.GameStructureChangedError):
            profile.__getitem__(game.root)


def _bob_response_infoset(g):
    return next(
        n.infoset for n in g.get_infosets("Bob") if n.infoset.label == "Bob's response"
    )


COLLECTION_GETTERS = [
    pytest.param(lambda g: g.players, id="GamePlayers"),
    pytest.param(lambda g: g.outcomes, id="GameOutcomes"),
    pytest.param(lambda g: g.actions, id="GameActions"),
    pytest.param(lambda g: g.players["Alice"].strategies, id="PlayerStrategies"),
    pytest.param(lambda g: g.players["Alice"].actions, id="PlayerActions"),
    pytest.param(lambda g: _bob_response_infoset(g).actions, id="InfosetActions"),
    pytest.param(lambda g: _bob_response_infoset(g).members, id="InfosetMembers"),
]


@pytest.mark.parametrize("getter", COLLECTION_GETTERS)
def test_collection_rejects_integer_indexing(getter):
    collection = getter(games.create_stripped_down_poker_efg())
    with pytest.raises(TypeError):
        _ = collection[0]
