import pytest

import pygambit as gbt


def _extensive_game() -> gbt.Game:
    return gbt.Game.read_game("test_games/basic_extensive_game.efg")


def _create_game() -> gbt.Game:
    game = gbt.Game.new_tree(players=["Player 1", "Player 2", "Player 3"])
    game.append_move([game.root], "Player 1", ["U", "M", "D"])
    game.append_move([game.root.children[0]], "Player 2", ["L", "R"])
    game.append_infoset([game.root.children[2]], game.root.children[0].infoset)
    return game


def test_infoset_set_label():
    game = _extensive_game()
    game.players[0].infosets[0].label = "infoset 1"
    assert game.players[0].infosets[0].label == "infoset 1"


def test_infoset_player_retrieval():
    game = _extensive_game()
    assert game.players[0] == game.players[0].infosets[0].player


def test_infoset_node_precedes():
    game = _extensive_game()
    assert not game.players[0].infosets[0].precedes(game.root)
    assert game.players[1].infosets[0].precedes(game.root.children[0])


def test_infoset_set_player():
    game = _extensive_game()
    game.set_player(game.root.infoset, game.players[1])
    assert game.root.infoset.player == game.players[1]


def test_infoset_set_player_mismatch():
    game = _extensive_game()
    game2 = gbt.Game.new_tree(["Frank"])
    with pytest.raises(gbt.MismatchError):
        game.set_player(game.root.infoset, game2.players[0])


def test_infoset_add_action_end():
    game = _extensive_game()
    actions = list(game.players[0].infosets[0].actions)
    game.add_action(game.players[0].infosets[0])
    assert list(game.players[0].infosets[0].actions)[:-1] == actions


def test_infoset_add_action_before():
    game = _extensive_game()
    actions = list(game.players[0].infosets[0].actions)
    game.add_action(game.players[0].infosets[0], actions[0])
    assert list(game.players[0].infosets[0].actions)[1:] == actions


def test_infoset_add_action_error():
    game = _extensive_game()
    with pytest.raises(gbt.MismatchError):
        game.add_action(game.players[0].infosets[0], game.players[1].infosets[0].actions[0])


def test_append_infoset_node_is_not_terminal():
    game = _create_game()
    game.append_move([game.root.children[0].children[0]], "Player 3", ["B", "F"])

    with pytest.raises(gbt.UndefinedOperationError):
        _ = game.append_infoset([game.root.children[2]],
                                game.root.children[0].children[0].infoset)


def test_append_infoset_node_list_with_not_terminal_node():
    game = _create_game()
    game.append_move([game.root.children[0].children[0]], "Player 3", ["B", "F"])

    with pytest.raises(gbt.UndefinedOperationError):
        _ = game.append_infoset([game.root.children[1],
                                game.root.children[2]],
                                game.root.children[0].children[0].infoset)


def test_append_infoset_node_list_with_duplciate_nodes():
    game = _create_game()
    game.append_move([game.root.children[0].children[0]], "Player 3", ["B", "F"])

    with pytest.raises(gbt.UndefinedOperationError):
        _ = game.append_infoset([game.root.children[0].children[1],
                                game.root.children[1],
                                game.root.children[0].children[1]],
                                game.root.children[0].children[0].infoset)
