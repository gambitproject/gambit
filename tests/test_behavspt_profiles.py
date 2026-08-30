import pytest

import pygambit as gbt

from . import games


def _find_infoset(game, label):
    """Find the Infoset with the given label, searching across all players."""
    for player in game.players:
        for node in game.get_infosets(player.label):
            if node.infoset.label == label:
                return node.infoset
    raise KeyError(label)


def _branching_game():
    """A small tree where P1 chooses L/R, each leading to a separate P2 decision, so
    that removing an action can make a whole subtree's information set unreachable.
    """
    game = gbt.Game.new_tree(players=["P1", "P2"])
    root = game.root
    game.append_move(root, "P1", ["L", "R"])
    left = root.children["L"]
    right = root.children["R"]
    game.append_move(left, "P2", ["A", "B"])
    game.append_move(right, "P2", ["A", "B"])
    root.infoset.label = "P1 infoset"
    left.infoset.label = "P2 left infoset"
    right.infoset.label = "P2 right infoset"
    return game, root.infoset, left.infoset, right.infoset


def test_getitem_by_infoset():
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.behavior_support_profile()
    infoset = _find_infoset(game, "Infoset 1:1")
    support = profile[infoset]
    assert set(support) == {"U1", "D1"}
    assert support.infoset == infoset
    assert "U1" in support
    assert "not-a-label" not in support


def test_getitem_by_player_label():
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.behavior_support_profile()
    support = profile["Player 1"]
    assert support.player == game.players["Player 1"]
    infoset = _find_infoset(game, "Infoset 1:1")
    assert set(support[infoset]) == {"U1", "D1"}


def test_getitem_unknown_player():
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.behavior_support_profile()
    with pytest.raises(KeyError):
        profile["not-a-player"]


def test_getitem_rejects_other_types():
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.behavior_support_profile()
    with pytest.raises(TypeError):
        profile[0]


def test_getitem_infoset_wrong_game():
    game = games.read_from_file("mixed_behavior_game.efg")
    other = games.read_from_file("mixed_behavior_game.efg")
    profile = game.behavior_support_profile()
    with pytest.raises(gbt.MismatchError):
        profile[_find_infoset(other, "Infoset 1:1")]


def test_predicate_construction():
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.behavior_support_profile(lambda node, a: a != "D1")
    infoset = _find_infoset(game, "Infoset 1:1")
    assert set(profile[infoset]) == {"U1"}


def test_predicate_construction_error():
    game = games.read_from_file("mixed_behavior_game.efg")
    with pytest.raises(ValueError):
        game.behavior_support_profile(lambda node, a: node.infoset.label != "Infoset 1:1")


def test_iter_yields_one_support_per_player():
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.behavior_support_profile()
    supports = list(profile)
    assert len(supports) == len(game.players)
    assert {s.player.label for s in supports} == {p.label for p in game.players}


def test_behaviorsupport_iter():
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.behavior_support_profile()
    support = profile["Player 1"]
    action_supports = list(support)
    assert len(action_supports) == len(game.get_infosets("Player 1"))
    assert all(isinstance(s, gbt.ActionSupport) for s in action_supports)


def test_setitem_replaces_support():
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.behavior_support_profile()
    infoset = _find_infoset(game, "Infoset 1:1")
    profile[infoset] = ["U1"]
    assert set(profile[infoset]) == {"U1"}


def test_setitem_unknown_label():
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.behavior_support_profile()
    infoset = _find_infoset(game, "Infoset 1:1")
    with pytest.raises(ValueError):
        profile[infoset] = ["not-a-label"]


def test_setitem_empty():
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.behavior_support_profile()
    infoset = _find_infoset(game, "Infoset 1:1")
    with pytest.raises(ValueError):
        profile[infoset] = []


def test_setitem_rejects_non_infoset():
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.behavior_support_profile()
    with pytest.raises(TypeError):
        profile["Infoset 1:1"] = ["U1"]


def test_setitem_infoset_wrong_game():
    game = games.read_from_file("mixed_behavior_game.efg")
    other = games.read_from_file("mixed_behavior_game.efg")
    profile = game.behavior_support_profile()
    with pytest.raises(gbt.MismatchError):
        profile[_find_infoset(other, "Infoset 1:1")] = ["U1"]


def test_copy_is_independent():
    game = games.read_from_file("mixed_behavior_game.efg")
    original = game.behavior_support_profile()
    copy = original.copy()
    infoset = _find_infoset(game, "Infoset 1:1")
    copy[infoset] = ["U1"]
    assert set(copy[infoset]) == {"U1"}
    assert set(original[infoset]) == {"U1", "D1"}


def test_actionsupport_is_snapshot():
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.behavior_support_profile()
    infoset = _find_infoset(game, "Infoset 1:1")
    snapshot = profile[infoset]
    profile[infoset] = ["U1"]
    assert set(snapshot) == {"U1", "D1"}


def test_getitem_setitem_accept_node_infoset():
    game, root_infoset, left_infoset, right_infoset = _branching_game()
    profile = game.behavior_support_profile()
    # game.root.infoset is a live, node-anchored Infoset view -- both __getitem__ and
    # __setitem__ must resolve it the same way Node.infoset is used everywhere else.
    assert set(profile[game.root.infoset]) == {"L", "R"}
    profile[game.root.infoset] = ["R"]
    assert set(profile[game.root.infoset]) == {"R"}


def test_is_reachable():
    game, root_infoset, left_infoset, right_infoset = _branching_game()
    profile = game.behavior_support_profile()
    assert profile.is_reachable(left_infoset)
    assert profile.is_reachable(right_infoset)

    copy = profile.copy()
    copy[root_infoset] = ["R"]
    assert not copy.is_reachable(left_infoset)
    assert copy.is_reachable(right_infoset)
    # the original, un-mutated profile is unaffected
    assert profile.is_reachable(left_infoset)


def test_is_reachable_by_label():
    """`is_reachable` resolves a string as a node's own label, not an infoset's label."""
    game, root_infoset, left_infoset, right_infoset = _branching_game()
    left = next(iter(left_infoset.members))
    left.label = "left"
    profile = game.behavior_support_profile()
    assert profile.is_reachable(left.label)


def test_is_reachable_wrong_game():
    _, _, left_infoset, _ = _branching_game()
    other, *_ = _branching_game()
    with pytest.raises(gbt.MismatchError):
        other.behavior_support_profile().is_reachable(left_infoset)
