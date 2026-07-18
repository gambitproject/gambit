import dataclasses
import functools
import itertools
import typing

import pytest

import pygambit as gbt

from . import games


@pytest.mark.parametrize("label", games.VALID_LABELS)
def test_infoset_set_label(label):
    game = games.read_from_file("basic_extensive_game.efg")
    game.root.infoset.label = label
    assert game.root.infoset.label == label


@pytest.mark.parametrize("label", games.INVALID_LABELS)
def test_infoset_label_invalid_raises_valueerror(label):
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(ValueError):
        game.root.infoset.label = label


@pytest.mark.parametrize("label", games.NON_ASCII_LABELS)
def test_infoset_label_non_ascii_rejected(label):
    """ASCII-only for 16.7 (#944); Unicode deferred to #862 (17.0)."""
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(UnicodeEncodeError):
        game.root.infoset.label = label


def test_infoset_label_duplicate_within_player_raises_valueerror():
    game = games.read_from_file("subgames.efg")
    player = next(p for p in game.players if sum(1 for _ in p.infosets) >= 2)
    first, second = itertools.islice(player.infosets, 2)
    first.label = "shared"
    with pytest.raises(ValueError):
        second.label = "shared"


def test_infoset_player_retrieval():
    game = games.read_from_file("basic_extensive_game.efg")
    p1, *_ = game.players
    assert p1 == game.root.infoset.player


def test_infoset_node_precedes():
    game = games.read_from_file("basic_extensive_game.efg")
    assert not game.root.infoset.precedes(game.root)
    assert game.root.children["U1"].infoset.precedes(game.root.children["U1"])


def test_infoset_set_player():
    """`set_player` reassigns the player; the label and membership ride along.
    """
    game = games.read_from_file("basic_extensive_game.efg")
    _, p2, *_ = game.players
    game.root.infoset.label = "moved"
    members = list(game.root.infoset.members)
    game.set_player(game.root.infoset, p2)
    assert game.root.infoset.player == p2
    assert game.root.infoset.label == "moved"
    assert list(game.root.infoset.members) == members


def test_infoset_set_player_mismatch():
    game = games.read_from_file("basic_extensive_game.efg")
    game2 = gbt.Game.new_tree(["Frank"])
    with pytest.raises(gbt.MismatchError):
        game.set_player(game.root.infoset, next(iter(game2.players)))


def test_infoset_add_action_end():
    game = games.read_from_file("basic_extensive_game.efg")
    actions = list(game.root.infoset.actions)
    game.add_action(game.root.infoset)
    assert list(game.root.infoset.actions)[:-1] == actions


def test_infoset_add_action_before():
    game = games.read_from_file("basic_extensive_game.efg")
    actions = list(game.root.infoset.actions)
    game.add_action(game.root.infoset, actions[0])
    assert list(game.root.infoset.actions)[1:] == actions


def test_infoset_add_action_error():
    game = games.read_from_file("basic_extensive_game.efg")
    _, p2, *_ = game.players
    with pytest.raises(gbt.MismatchError):
        game.add_action(game.root.infoset, next(iter(p2.infosets)).actions["U2"])


def test_infoset_plays():
    """Verify `infoset.plays` returns plays reachable from a given infoset.
    """
    game = gbt.catalog.load("journals/ijgt/selten1975/fig1")
    list_nodes = list(game.nodes)
    list_infosets = list(game.infosets)

    test_infoset = list_infosets[2]  # members' paths=[1, 0], [1]

    expected_set_of_plays = {
        list_nodes[4], list_nodes[5], list_nodes[7], list_nodes[8]
    }  # paths=[0, 1, 0], [1, 1, 0], [0, 1], [1, 1]

    assert set(test_infoset.plays) == expected_set_of_plays


@dataclasses.dataclass
class PriorActionsTestCase:
    """TestCase for testing own_prior_actions."""
    factory: typing.Callable[[], gbt.Game]
    # each tuple is (action_path_to_a_member_node, expected_prior_actions_set)
    expected_results: list[tuple[list[str], set]]


@dataclasses.dataclass
class AbsentMindednessTestCase:
    """TestCase for testing is_absent_minded."""
    factory: typing.Callable[[], gbt.Game]
    expected_am_paths: list[list[str]]


PRIOR_ACTIONS_CASES = [
    pytest.param(
        PriorActionsTestCase(
            factory=functools.partial(games.read_from_file, "binary_3_levels_generic_payoffs.efg"),
            expected_results=[
                ([], {None}),
                (["Left", "Left"], {("Player 1", 0, "Left")}),
                (["Right", "Left"], {("Player 1", 0, "Right")}),
                (["Left"], {None}),
            ]
        ),
        id="perfect_recall"
    ),
    pytest.param(
        PriorActionsTestCase(
            factory=functools.partial(gbt.catalog.load, "journals/geb/wichardt2008"),
            expected_results=[
                ([], {None}),
                (["R"], {("Player 1", 0, "L"), ("Player 1", 0, "R")}),
                (["R", "r"], {None}),
            ]
        ),
        id="wichardt_forgetting_action"
    ),
    pytest.param(
        PriorActionsTestCase(
            factory=functools.partial(games.read_from_file, "subgames.efg"),
            expected_results=[
                (["1"], {None}),
                (["2"], {None}),
                (["2", "1", "2"], {("Player 1", 1, "1")}),
                (["2", "2", "1", "1"], {("Player 1", 5, "1"), ("Player 1", 1, "2")}),
                (["2", "2", "1", "2"], {("Player 1", 1, "2")}),
                (["2", "2", "1", "2", "2", "1"], {("Player 1", 4, "2")}),
                (["2", "2", "2"], {("Player 1", 1, "2")}),
                ([], {None}),
                (["2", "1"], {("Player 2", 0, "2")}),
                (["2", "2", "1"], {("Player 2", 1, "1")}),
                (["2", "2", "1", "1", "1"], {("Player 2", 2, "1")}),
                (["2", "2", "1", "2", "1"], {("Player 2", 2, "2")}),
                (["2", "2", "1", "2", "2", "1", "1"], {("Player 2", 4, "1")}),
            ]
        ),
        id="four_subgames"
    ),
    pytest.param(
        PriorActionsTestCase(
            factory=functools.partial(games.read_from_file, "AM-driver-subgame.efg"),
            expected_results=[
                ([], {None, ("Player 1", 0, "S")}),
                (["S", "T"], {None}),
            ]
        ),
        id="AM_driver"
    ),
]

ABSENT_MINDEDNESS_CASES = [
    # Games without absent-mindedness
    pytest.param(
        AbsentMindednessTestCase(
            factory=functools.partial(gbt.catalog.load, "journals/ijgt/selten1975/fig2"),
            expected_am_paths=[]
        ),
        id="short_centipede_perfect_info"
    ),
    pytest.param(
        AbsentMindednessTestCase(
            factory=functools.partial(games.read_from_file, "stripped_down_poker.efg"),
            expected_am_paths=[]
        ),
        id="poker_stripped"
    ),
    pytest.param(
        AbsentMindednessTestCase(
            factory=functools.partial(games.read_from_file, "basic_extensive_game.efg"),
            expected_am_paths=[]
        ),
        id="basic_extensive"
    ),
    pytest.param(
        AbsentMindednessTestCase(
            factory=functools.partial(games.read_from_file, "gilboa_two_am_agents.efg"),
            expected_am_paths=[]
        ),
        id="gilboa_forgetting_info"
    ),
    pytest.param(
        AbsentMindednessTestCase(
            factory=functools.partial(gbt.catalog.load, "journals/geb/wichardt2008"),
            expected_am_paths=[]
        ),
        id="wichardt_forgetting_action"
    ),
    # Games with absent-mindedness
    pytest.param(
        AbsentMindednessTestCase(
            factory=functools.partial(games.read_from_file, "noPR-AM-driver-two-players.efg"),
            expected_am_paths=[[]]
        ),
        id="AM_driver_two_players"
    ),
    pytest.param(
        AbsentMindednessTestCase(
            factory=functools.partial(games.read_from_file, "noPR-action-AM.efg"),
            expected_am_paths=[[]]
        ),
        id="AM_forgetting_action"
    ),
    pytest.param(
        AbsentMindednessTestCase(
            factory=functools.partial(games.read_from_file, "noPR-action-AM-two-hops.efg"),
            expected_am_paths=[["2", "1", "1", "1", "1"], ["1", "1", "1"]]
        ),
        id="AM_infoset_takes_two_hops"
    ),
]


def _get_node_by_path(game, path: list[str]) -> gbt.Node:
    """
    Helper to find a node by following a sequence of action labels.
    """
    node = game.root
    for action_label in reversed(path):
        node = node.children[action_label]
    return node


@pytest.mark.parametrize("test_case", PRIOR_ACTIONS_CASES)
def test_infoset_own_prior_actions(test_case: PriorActionsTestCase):
    """
    Test `infoset.own_prior_actions`.

    Verifies that the set of prior actions (as player-infoset-label tuples)
    matches the expected results.  Each infoset is identified by an action
    path to one of its member nodes.
    """
    game = test_case.factory()

    for path, expected_set in test_case.expected_results:
        node = game.root
        for action_label in path:
            node = node.children[action_label]
        infoset = node.infoset

        actual_actions = infoset.own_prior_actions

        actual_details = {
            (a.infoset.player.label, a.infoset.number, a.label) if a is not None else None
            for a in actual_actions
        }

        assert actual_details == expected_set


@pytest.mark.parametrize("test_case", ABSENT_MINDEDNESS_CASES)
def test_infoset_is_absent_minded(test_case: AbsentMindednessTestCase):
    """
    Test `infoset.is_absent_minded`.

    Verifies that the set of infosets marked as absent-minded matches the
    expected set derived from action paths.
    """
    game = test_case.factory()

    expected_infosets = {
        _get_node_by_path(game, path).infoset
        for path in test_case.expected_am_paths
        }
    actual_infosets = {infoset for infoset in game.infosets if infoset.is_absent_minded}

    assert actual_infosets == expected_infosets


def test_make_infoset_across_different_source_players():
    """Nodes drawn from different players all land under the target player."""
    game = gbt.Game.new_tree(players=["1", "2", "3"])
    game.append_move(game.root, "1", ["a", "b"])
    game.append_move(game.root.children["a"], "2", ["a", "b"])   # player 2
    game.append_move(game.root.children["b"], "3", ["a", "b"])   # player 3
    n2 = game.root.children["a"]
    n3 = game.root.children["b"]
    assert n2.infoset.player == game.players["2"]
    assert n3.infoset.player == game.players["3"]
    game.make_infoset([n2, n3], "1")
    assert n2.infoset == n3.infoset
    assert n2.infoset.player == game.players["1"]
    assert n3.infoset.player == game.players["1"]
