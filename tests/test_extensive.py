import typing
import pytest

import pygambit as gbt

from . import games

@pytest.mark.parametrize(
    "players,title",
    [([], "New game"),
     (["Alice", "Bob"], "A poker game")]
)
def test_new_tree(players: list, title: typing.Optional[str]):
    game = gbt.Game.new_tree(players=players, title=title)
    assert len(game.players) == len(players)
    for (player, label) in zip(game.players, players):
        assert player.label == label
    assert game.title == title


@pytest.mark.parametrize(
    "title", ["My game's new title"]
)
def test_game_title(title: str):
    game = gbt.Game.new_tree()
    game.title = title
    assert game.title == title


@pytest.mark.parametrize(
    "comment", ["This is a comment describing the game in more detail"]
)
def test_game_comment(comment: str):
    game = gbt.Game.new_tree()
    game.comment = comment
    assert game.comment == comment


@pytest.mark.parametrize(
    "players",
    [["Alice"], ["Oscar", "Felix"]]
)
def test_game_add_players_label(players: list):
    game = gbt.Game.new_tree()
    for player in players:
        game.add_player(player)
    for (player, label) in zip(game.players, players):
        assert player.label == label


def test_game_add_players_nolabel():
    game = gbt.Game.new_tree()
    game.add_player()


def test_game_num_nodes():
    game = games.read_from_file("basic_extensive_game.efg")
    assert len(game.nodes()) == 15


def test_game_is_perfect_recall():
    game = games.read_from_file("perfect_recall.efg")
    assert game.is_perfect_recall


def test_game_is_not_perfect_recall():
    game = games.read_from_file("not_perfect_recall.efg")
    assert not game.is_perfect_recall


def test_getting_payoff_by_label_string():
    game = games.read_from_file("sample_extensive_game.efg")
    assert game[[0, 0]]['Player 1'] == 2
    assert game[[0, 1]]['Player 1'] == 2
    assert game[[1, 0]]['Player 1'] == 4
    assert game[[1, 1]]['Player 1'] == 6
    assert game[[0, 0]]['Player 2'] == 3
    assert game[[0, 1]]['Player 2'] == 3
    assert game[[1, 0]]['Player 2'] == 5
    assert game[[1, 1]]['Player 2'] == 7


def test_getting_payoff_by_player():
    game = games.read_from_file("sample_extensive_game.efg")
    player1 = game.players[0]
    player2 = game.players[1]
    assert game[[0, 0]][player1] == 2
    assert game[[0, 1]][player1] == 2
    assert game[[1, 0]][player1] == 4
    assert game[[1, 1]][player1] == 6
    assert game[[0, 0]][player2] == 3
    assert game[[0, 1]][player2] == 3
    assert game[[1, 0]][player2] == 5
    assert game[[1, 1]][player2] == 7


def test_outcome_index_exception_label():
    game = games.read_from_file("sample_extensive_game.efg")
    with pytest.raises(KeyError):
        _ = game[[0, 0]]["Not a player"]
