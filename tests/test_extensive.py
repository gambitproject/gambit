import typing

import numpy as np
import pytest

import pygambit as gbt

from . import games


@pytest.mark.parametrize(
    "players,title", [([], "New game"), (["Alice", "Bob"], "A poker game")]
)
def test_new_tree(players: list, title: typing.Optional[str]):
    game = gbt.Game.new_tree(players=players, title=title)
    assert len(game.players) == len(players)
    for player, label in zip(game.players, players):
        assert player.label == label
    assert game.title == title


@pytest.mark.parametrize("title", ["My game's new title"])
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


@pytest.mark.parametrize("players", [["Alice"], ["Oscar", "Felix"]])
def test_game_add_players_label(players: list):
    game = gbt.Game.new_tree()
    for player in players:
        game.add_player(player)
    for player, label in zip(game.players, players):
        assert player.label == label


def test_game_add_players_nolabel():
    game = gbt.Game.new_tree()
    game.add_player()


def test_game_is_perfect_recall():
    game = games.read_from_file("perfect_recall.efg")
    assert game.is_perfect_recall


def test_game_is_not_perfect_recall():
    game = games.read_from_file("not_perfect_recall.efg")
    assert not game.is_perfect_recall


def test_getting_payoff_by_label_string():
    game = games.read_from_file("sample_extensive_game.efg")
    assert game[[0, 0]]["Player 1"] == 2
    assert game[[0, 1]]["Player 1"] == 2
    assert game[[1, 0]]["Player 1"] == 4
    assert game[[1, 1]]["Player 1"] == 6
    assert game[[0, 0]]["Player 2"] == 3
    assert game[[0, 1]]["Player 2"] == 3
    assert game[[1, 0]]["Player 2"] == 5
    assert game[[1, 1]]["Player 2"] == 7


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


@pytest.mark.parametrize(
    "game,strategy_labels,np_arrays_of_rsf",
    [
        ###############################################################################
        # # 1 player; reduction; generic payoffs
        (
            games.create_reduction_one_player_generic_payoffs_efg(),
            [["11", "12", "2*", "3*", "4*"]],
            [np.array(range(1, 6))],
        ),
        # 2 players; reduction possible for player 1; payoff ties
        (
            games.read_from_file("e02.efg"),
            [["1*", "21", "22"], ["1", "2"]],
            [
                np.array([[1, 1], [0, 0], [0, 2]]),
                np.array([[1, 1], [2, 3], [2, 0]]),
            ],
        ),
        # 2 players; 1 move each so no reduction possible
        (
            games.read_from_file("sample_extensive_game.efg"),
            [["1", "2"], ["11", "12", "21", "22"]],
            [
                np.array([[2, 2, 2, 2], [4, 6, 4, 6]]),
                np.array([[3, 3, 3, 3], [5, 7, 5, 7]]),
            ],
        ),
        # Selten's Horse: game with three players
        (
            games.read_from_file("e01.efg"),
            [["1", "2"], ["1", "2"], ["1", "2"]],
            [
                np.array([[[1, 1], [4, 0]], [[3, 0], [3, 0]]]),
                np.array([[[1, 1], [4, 0]], [[2, 0], [2, 0]]]),
                np.array([[[1, 1], [0, 1]], [[2, 0], [2, 0]]]),
            ],
        ),
        # 2-player (zero-sum) game; reduction for both players; generic payoffs
        (
            games.create_reduction_generic_payoffs_efg(),
            [
                ["11*", "12*", "211", "221", "212", "222"],
                ["1*1", "1*2", "2**", "31*", "32*", "4**"],
            ],
            [
                np.array(
                    [
                        [1, 1, 6, 8, 9, 12],
                        [1, 1, 7, 10, 11, 12],
                        [2, 4, 6, 8, 9, 12],
                        [2, 4, 7, 10, 11, 12],
                        [3, 5, 6, 8, 9, 12],
                        [3, 5, 7, 10, 11, 12],
                    ]
                ),
                np.array(
                    [
                        [-1, -1, -6, -8, -9, -12],
                        [-1, -1, -7, -10, -11, -12],
                        [-2, -4, -6, -8, -9, -12],
                        [-2, -4, -7, -10, -11, -12],
                        [-3, -5, -6, -8, -9, -12],
                        [-3, -5, -7, -10, -11, -12],
                    ]
                ),
            ],
        ),
        # 2-player (zero-sum) game; binary tree; reduction for player 1; generic payoffs
        (
            games.read_from_file("binary_3_levels_generic_payoffs.efg"),
            [
                ["11*", "12*", "2*1", "2*2"],
                ["1", "2"],
            ],
            [
                np.array([[1, 3], [2, 4], [5, 7], [6, 8]]),
                np.array([[-1, -3], [-2, -4], [-5, -7], [-6, -8]]),
            ],
        ),
        # # 2-player game from GTE survey; reduction for both players; payoff ties
        (
            games.create_reduction_both_players_payoff_ties_efg(),
            [
                ["1*", "2*", "31", "32", "4*"],
                [
                    "111*",
                    "112*",
                    "121*",
                    "122*",
                    "2111",
                    "2121",
                    "2211",
                    "2221",
                    "2112",
                    "2122",
                    "2212",
                    "2222",
                ],
            ],
            [
                np.array(
                    [
                        [2, 2, 2, 2, 0, 0, 0, 0, 5, 5, 5, 5],
                        [7, 7, 4, 4, 7, 7, 4, 4, 7, 7, 4, 4],
                        [3, 7, 3, 7, 3, 7, 3, 7, 3, 7, 3, 7],
                        [8, 2, 8, 2, 8, 2, 8, 2, 8, 2, 8, 2],
                        [6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6],
                    ]
                ),
                np.array(
                    [
                        [8, 8, 8, 8, 1, 1, 1, 1, 2, 2, 2, 2],
                        [6, 6, 2, 2, 6, 6, 2, 2, 6, 6, 2, 2],
                        [7, 8, 7, 8, 7, 8, 7, 8, 7, 8, 7, 8],
                        [3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2],
                        [4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4],
                    ]
                ),
            ],
        ),
        ###########################################################################
        # Games with chance nodes
        ###########################################################################
        # 2-player long centipede game with chance playing at the root twice
        (
            games.read_from_file("cent3.efg"),
            [
                ["1**111", "21*111", "221111", "222111"],
                ["1**111", "21*111", "221111", "222111"],
            ],
            [
                np.array(
                    [
                        ["20027/25000", "5081/6250", "2689/3125", "163/125"],
                        ["541/1250", "19931/6250", "10114/3125", "92/25"],
                        ["3299/6250", "5362/3125", "39814/3125", "1648/125"],
                        ["227/250", "262/125", "856/125", "256/5"],
                    ]
                ),
                np.array(
                    [
                        ["2689/12500", "3283/12500", "5659/12500", "163/500"],
                        ["3983/2500", "2677/3125", "3271/3125", "23/25"],
                        ["5053/3125", "19903/3125", "10696/3125", "412/125"],
                        ["214/125", "808/125", "3184/125", "64/5"],
                    ]
                ),
            ],
        ),
        # Stripped down "Myerson" 2-card poker; 2 player zero-sum game with chance at the root
        (
            games.create_myerson_2_card_poker_efg(),
            [["11", "12", "21", "22"], ["1", "2"]],
            [
                np.array([[-1, 0], ["-1/2", -1], ["-5/2", -1], [-2, -2]]),
                np.array([[1, 0], ["1/2", 1], ["5/2", 1], [2, 2]]),
            ],
        ),
        # Nature playing at the root, 2 players, no reduction, non-generic payoffs
        (
            games.read_from_file("nature_rooted_nongeneric.efg"),
            [["1", "2"], ["11", "12", "21", "22"]],
            [
                np.array([[-1, -1, 2, 2], [0, 0, 0, 0]]),
                np.array([[-1, -1, 2, 2], [3, 4, 3, 4]]),
            ],
        ),
        # Nature playing at the root, 2 players, no reduction, generic payoffs
        (
            games.read_from_file("nature_rooted_generic.efg"),
            [["1", "2"], ["11", "12", "21", "22"]],
            [
                np.array([[3, 3, 4, 4], [5, 6, 5, 6]]),
                np.array([[-3, -3, -4, -4], [-5, -6, -5, -6]]),
            ],
        ),
        # Nature playing last determining the payoffs, 2 players, no reduction, non-generic payoffs
        (
            games.read_from_file("nature_leaves_nongeneric.efg"),
            [["1", "2"], ["11", "12", "21", "22"]],
            [
                np.array([[-1, -1, 2, 2], [0, 0, 0, 0]]),
                np.array([[-1, -1, 2, 2], [3, 4, 3, 4]]),
            ],
        ),
        # Nature playing last determining the payoffs, 2 players, no reduction, generic payoffs
        (
            games.read_from_file("nature_leaves_generic.efg"),
            [["1", "2"], ["11", "12", "21", "22"]],
            [
                np.array(
                    [["3/2", "3/2", "7/2", "7/2"], ["11/2", "15/2", "11/2", "15/2"]]
                ),
                np.array(
                    [
                        ["-3/2", "-3/2", "-7/2", "-7/2"],
                        ["-11/2", "-15/2", "-11/2", "-15/2"],
                    ]
                ),
            ],
        ),
        # I M P E R F E C T   R E C A L L --- commented out in the test suite
        # Wichardt (2008): binary tree of height 3; 2 players; the root player forgets the action
        # (
        #    games.read_from_file("wichardt.efg"),
        #    [["11", "12", "21", "22"], ["1", "2"]],
        #    [
        #        np.array([[1, -1], [-5, -5], [-5, -5], [-1, 1]]),
        #        np.array([[-1, 1], [5, 5], [5, 5], [1, -1]]),
        #    ],
        # ),
    ],
)
def test_reduced_strategic_form(
    game: gbt.Game, strategy_labels: list, np_arrays_of_rsf: list
):
    """
    We test two things:
        - that the strategy labels are as expected
          (these use positive integers and '*'s, rather than labels of moves even if they exist)
        - that the payoff tables are correct, which is done via game.to_arrays()
    """
    arrays = game.to_arrays()

    for i, player in enumerate(game.players):
        assert strategy_labels[i] == [s.label for s in player.strategies]
        # convert strings to rationals
        exp_array = games.vectorized_make_rational(np_arrays_of_rsf[i])
        assert (arrays[i] == exp_array).all()
