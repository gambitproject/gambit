import pytest

import pygambit as gbt

from . import games


def _find_history(game, label):
    """The History of a member node of the infoset with the given label."""
    return games._INFOSET_LABEL_HISTORIES[(game.title, label)]


def _find_selector(game, label):
    """A `Selector` resolving to the infoset with the given label."""
    return gbt.H.path(*_find_history(game, label))


def _branching_game() -> gbt.Game:
    """A small tree where P1 chooses L/R, each leading to a separate P2 decision, so
    that removing an action can make a whole subtree's information set unreachable.
    """
    game = gbt.Game.new_tree(players=["P1", "P2"])
    game.append_move(gbt.H.path(), "P1", ["L", "R"])
    game.append_move(gbt.H.path("L"), "P2", ["A", "B"])
    game.append_move(gbt.H.path("R"), "P2", ["A", "B"])
    return game


def test_getitem_by_selector():
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.behavior_support_profile()
    support = profile[_find_selector(game, "Infoset 1:1")]
    assert set(support) == {"U1", "D1"}
    assert support.history == _find_history(game, "Infoset 1:1")
    assert "U1" in support
    assert "not-a-label" not in support


def test_getitem_by_player_label():
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.behavior_support_profile()
    support = profile["Player 1"]
    assert support.player == "Player 1"
    assert set(support[_find_selector(game, "Infoset 1:1")]) == {"U1", "D1"}


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


def test_getitem_setitem_reject_history():
    """Profile indexing is `Selector`-only: a bare `History` tuple is no longer
    accepted, following the same pattern as `Game.get_minimal_subgame`.
    """
    game = _branching_game()
    profile = game.behavior_support_profile()
    with pytest.raises(TypeError):
        profile[("L",)]
    with pytest.raises(TypeError):
        profile[("L",)] = ["A"]


def test_predicate_construction():
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.behavior_support_profile(lambda history, a: a != "D1")
    assert set(profile[_find_selector(game, "Infoset 1:1")]) == {"U1"}


def test_predicate_construction_error():
    """A predicate that excludes every action at some information set (here, the
    root's) triggers "attempted to remove the last action"."""
    game = games.read_from_file("mixed_behavior_game.efg")
    with pytest.raises(ValueError):
        game.behavior_support_profile(lambda history, a: len(history) > 0)


def test_iter_yields_one_support_per_player():
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.behavior_support_profile()
    supports = list(profile)
    assert len(supports) == len(game.players)
    assert {s.player for s in supports} == set(game.players)


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
    selector = _find_selector(game, "Infoset 1:1")
    profile[selector] = ["U1"]
    assert set(profile[selector]) == {"U1"}


def test_setitem_unknown_label():
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.behavior_support_profile()
    with pytest.raises(ValueError):
        profile[_find_selector(game, "Infoset 1:1")] = ["not-a-label"]


def test_setitem_empty():
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.behavior_support_profile()
    with pytest.raises(ValueError):
        profile[_find_selector(game, "Infoset 1:1")] = []


def test_setitem_rejects_non_selector():
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.behavior_support_profile()
    with pytest.raises(TypeError):
        profile["Infoset 1:1"] = ["U1"]


def test_copy_is_independent():
    game = games.read_from_file("mixed_behavior_game.efg")
    original = game.behavior_support_profile()
    copy = original.copy()
    selector = _find_selector(game, "Infoset 1:1")
    copy[selector] = ["U1"]
    assert set(copy[selector]) == {"U1"}
    assert set(original[selector]) == {"U1", "D1"}


def test_actionsupport_is_snapshot():
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.behavior_support_profile()
    selector = _find_selector(game, "Infoset 1:1")
    snapshot = profile[selector]
    profile[selector] = ["U1"]
    assert set(snapshot) == {"U1", "D1"}


def test_getitem_setitem_use_selector():
    game = _branching_game()
    profile = game.behavior_support_profile()
    root_selector = gbt.H.path()
    assert set(profile[root_selector]) == {"L", "R"}
    profile[root_selector] = ["R"]
    assert set(profile[root_selector]) == {"R"}


def test_is_infoset_reachable():
    game = _branching_game()
    profile = game.behavior_support_profile()
    left_selector = gbt.H.path("L")
    right_selector = gbt.H.path("R")
    assert profile.is_infoset_reachable(left_selector)
    assert profile.is_infoset_reachable(right_selector)

    copy = profile.copy()
    copy[gbt.H.path()] = ["R"]
    assert not copy.is_infoset_reachable(left_selector)
    assert copy.is_infoset_reachable(right_selector)
    # the original, un-mutated profile is unaffected
    assert profile.is_infoset_reachable(left_selector)
