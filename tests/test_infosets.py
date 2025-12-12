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


@pytest.mark.parametrize("game_file, expected_results", [
    # Perfect recall game
    (
        "binary_3_levels_generic_payoffs.efg",
        [
            # Player 1, Infoset 0 (Root):
            # No prior history.
            ("Player 1", 0, {None}),

            # Player 1, Infoset 1:
            # Reached via "Left" from Infoset 0.
            ("Player 1", 1, {("Player 1", 0, "Left")}),

            # Player 1, Infoset 2:
            # Reached via "Right" from Infoset 0.
            ("Player 1", 2, {("Player 1", 0, "Right")}),

            # Player 2, Infoset 0:
            # No prior history.
            ("Player 2", 0, {None}),
        ]
    ),
    # Imperfect recall games, no absent-mindedness
    (
        "wichardt.efg",
        [
            # Player 1, Infoset 0 (Root):
            # No prior history.
            ("Player 1", 0, {None}),

            # Player 1, Infoset 1:
            # Reachable via "L" or "R" from Infoset 0.
            ("Player 1", 1, {("Player 1", 0, "L"), ("Player 1", 0, "R")}),

            # Player 2, Infoset 0:
            # No prior history.
            ("Player 2", 0, {None}),
        ]
    ),
    (
        "subgames.efg",
        [
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
    # An absent-minded driver game
    (
        "AM-driver-subgame.efg",
        [
            # Player 1, Infoset 0:
            # One member is the root (no prior history),
            # the other is reached via "S" from this same infoset.
            ("Player 1", 0, {None, ("Player 1", 0, "S")}),

            # Player 2, Infoset 0:
            # No prior history.
            ("Player 2", 0, {None}),
        ]
    ),
])
def test_infoset_own_prior_actions(game_file, expected_results):
    """
    Tests `infoset.own_prior_actions` by collecting the action details
    (player label, infoset num, label) and comparing against expected sets.
    """
    game = games.read_from_file(game_file)

    for player_label, infoset_num, expected_set in expected_results:
        player = game.players[player_label]
        infoset = player.infosets[infoset_num]

        actual_actions = infoset.own_prior_actions

        actual_details = {
            (a.infoset.player.label, a.infoset.number, a.label) if a is not None else None
            for a in actual_actions
        }

        assert actual_details == expected_set


def _get_node_by_path(game, path: list[str]) -> gbt.Node:
    """
    Helper to find a node by following a sequence of action labels.

    Parameters
    ----------
    path : list[str]
        A list of action labels in Node->Root order.
    """
    node = game.root
    for action_label in reversed(path):
        node = node.children[action_label]

    return node


@pytest.mark.parametrize("game_input, expected_am_paths", [
    # Games without absent-mindedness
    ("e02.efg", []),
    ("stripped_down_poker.efg", []),
    ("basic_extensive_game.efg", []),
    ("gilboa_two_am_agents.efg", []),  # forgetting past information; Gilboa (GEB, 1997)
    ("wichardt.efg", []),              # forgetting past action; Wichardt (GEB, 2008)

    # Games with absent-mindedness
    ("noPR-AM-driver-two-players.efg", [[]]),
    ("noPR-action-AM.efg", [[]]),
    ("noPR-action-AM-two-hops.efg", [["2", "1", "1", "1", "1"], ["1", "1", "1"]]),
])
def test_infoset_is_absent_minded(game_input, expected_am_paths):
    """
    Verify the is_absent_minded property of information sets.
    """
    game = games.read_from_file(game_input)

    expected_infosets = {_get_node_by_path(game, path).infoset for path in expected_am_paths}
    actual_infosets = {infoset for infoset in game.infosets if infoset.is_absent_minded}

    assert actual_infosets == expected_infosets
