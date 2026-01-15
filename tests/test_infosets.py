import dataclasses
import functools
import typing

import pytest

import pygambit as gbt

from . import games


def test_infoset_set_label():
    game = games.read_from_file("basic_extensive_game.efg")
    game.players[0].infosets[0].label = "infoset 1"
    assert game.players[0].infosets[0].label == "infoset 1"


def test_infoset_player_retrieval():
    game = games.read_from_file("basic_extensive_game.efg")
    assert game.players[0] == game.players[0].infosets[0].player


def test_infoset_node_precedes():
    game = games.read_from_file("basic_extensive_game.efg")
    assert not game.players[0].infosets[0].precedes(game.root)
    assert game.players[1].infosets[0].precedes(game.root.children[0])


def test_infoset_set_player():
    game = games.read_from_file("basic_extensive_game.efg")
    game.set_player(game.root.infoset, game.players[1])
    assert game.root.infoset.player == game.players[1]


def test_infoset_set_player_mismatch():
    game = games.read_from_file("basic_extensive_game.efg")
    game2 = gbt.Game.new_tree(["Frank"])
    with pytest.raises(gbt.MismatchError):
        game.set_player(game.root.infoset, game2.players[0])


def test_infoset_add_action_end():
    game = games.read_from_file("basic_extensive_game.efg")
    actions = list(game.players[0].infosets[0].actions)
    game.add_action(game.players[0].infosets[0])
    assert list(game.players[0].infosets[0].actions)[:-1] == actions


def test_infoset_add_action_before():
    game = games.read_from_file("basic_extensive_game.efg")
    actions = list(game.players[0].infosets[0].actions)
    game.add_action(game.players[0].infosets[0], actions[0])
    assert list(game.players[0].infosets[0].actions)[1:] == actions


def test_infoset_add_action_error():
    game = games.read_from_file("basic_extensive_game.efg")
    with pytest.raises(gbt.MismatchError):
        game.add_action(game.players[0].infosets[0], game.players[1].infosets[0].actions[0])


def test_infoset_plays():
    """Verify `infoset.plays` returns plays reachable from a given infoset.
    """
    game = games.read_from_file("e01.efg")
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
    expected_results: list[tuple]


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
                ("Player 1", 0, {None}),
                ("Player 1", 1, {("Player 1", 0, "Left")}),
                ("Player 1", 2, {("Player 1", 0, "Right")}),
                ("Player 2", 0, {None}),
            ]
        ),
        id="perfect_recall"
    ),
    pytest.param(
        PriorActionsTestCase(
            factory=functools.partial(games.read_from_file, "wichardt.efg"),
            expected_results=[
                ("Player 1", 0, {None}),
                ("Player 1", 1, {("Player 1", 0, "L"), ("Player 1", 0, "R")}),
                ("Player 2", 0, {None}),
            ]
        ),
        id="wichardt_forgetting_action"
    ),
    pytest.param(
        PriorActionsTestCase(
            factory=functools.partial(games.read_from_file, "subgames.efg"),
            expected_results=[
                ("Player 1", 0, {None}),
                ("Player 1", 1, {None}),
                ("Player 1", 2, {("Player 1", 1, "1")}),
                ("Player 1", 3, {("Player 1", 5, "1"), ("Player 1", 1, "2")}),
                ("Player 1", 4, {("Player 1", 1, "2")}),
                ("Player 1", 5, {("Player 1", 4, "2")}),
                ("Player 1", 6, {("Player 1", 1, "2")}),
                ("Player 2", 0, {None}),
                ("Player 2", 1, {("Player 2", 0, "2")}),
                ("Player 2", 2, {("Player 2", 1, "1")}),
                ("Player 2", 3, {("Player 2", 2, "1")}),
                ("Player 2", 4, {("Player 2", 2, "2")}),
                ("Player 2", 5, {("Player 2", 4, "1")}),
            ]
        ),
        id="four_subgames"
    ),
    pytest.param(
        PriorActionsTestCase(
            factory=functools.partial(games.read_from_file, "AM-driver-subgame.efg"),
            expected_results=[
                ("Player 1", 0, {None, ("Player 1", 0, "S")}),
                ("Player 2", 0, {None}),
            ]
        ),
        id="AM_driver"
    ),
]

ABSENT_MINDEDNESS_CASES = [
    # Games without absent-mindedness
    pytest.param(
        AbsentMindednessTestCase(
            factory=functools.partial(games.read_from_file, "e02.efg"),
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
            factory=functools.partial(games.read_from_file, "wichardt.efg"),
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
    matches the expected results.
    """
    game = test_case.factory()

    for player_label, infoset_num, expected_set in test_case.expected_results:
        player = game.players[player_label]
        infoset = player.infosets[infoset_num]

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
        _get_node_by_path(game, path).infoset for path in test_case.expected_am_paths
        }
    actual_infosets = {infoset for infoset in game.infosets if infoset.is_absent_minded}

    assert actual_infosets == expected_infosets
