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
