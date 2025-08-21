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


@pytest.mark.parametrize("game_input,expected_result", [
    # Games with perfect recall from files (game_input is a string)
    ("e01.efg", True),
    ("e02.efg", True),
    ("cent3.efg", True),
    ("poker.efg", True),
    ("basic_extensive_game.efg", True),

    # Games with perfect recall from generated games (game_input is a gbt.Game object)
    # - Centipede games
    (games.Centipede.get_test_data(N=3, m0=2, m1=7)[0], True),
    (games.Centipede.get_test_data(N=4, m0=2, m1=7)[0], True),
    # - Two-player binary tree games
    (games.BinEfgTwoPlayer.get_test_data(level=3)[0], True),
    (games.BinEfgTwoPlayer.get_test_data(level=4)[0], True),
    # - Three-player binary tree games
    (games.BinEfgThreePlayer.get_test_data(level=3)[0], True),

    # Games with imperfect recall from files (game_input is a string)
    #
    # - imperfect recall without absent-mindedness
    # Wichardt (GEB, 2008): 2 players; Player 1 in Infoset 1:2 forgets past action
    ("wichardt.efg", False),
    # variation of the Selten's Horse (1975): Player 1 in Infoset 1:2 forgets past action
    ("noPR-action-selten-horse.efg", False),
    # deflate-stable game (cf. Thompson); Player 2 in Infoset 2:2 forgets past information
    ("noPR-information-no-deflate.efg", False),
    # Gilboa (GEB, 1997) 2 players; agents of Player 1 forget past information
    ("gilboa-two-am-agents-deterministic.efg", False),
    #
    # - imperfect recall with absent-mindedness
    # Classic AM-driver game: Player 2 does not move, but gets payoffs assigned
    ("noPR-AM-driver-one-player.efg", False),
    # forgetting past action; Player 1 in Infoset 1:1 has AM; unreachable: Infosets 2:2, 2:3
    ("noPR-action-AM.efg", False),
    # forgetting past action; Player 1 in Infoset 1:1 has AM; unreachable: Infosets 1:3, 2:1
    ("noPR-action-AM2.efg", False),
    # Player 1 in Infoset 1:1 has AM; unreachable: Infosets 1:3, 2:2, 2:3, 2:4, 2:5
    ("noPR-action-big-AM.efg", False),
    # one AM-infoset each, unreachable: Infoset 2:2
    ("noPR-action-AM-two-hops.efg", False),
    # one AM-infoset each, unreachable: Infosets 1:2, 2:2, Node 7 in Infoset 1:1
    ("noPR-action-AM-two-hops-unreached.efg", False),

    # Games with imperfect recall from generated games (game_input is a gbt.Game object)
    # - One-player binary tree games
    (games.BinEfgOnePlayerIR.get_test_data(level=3)[0], False),
    (games.BinEfgOnePlayerIR.get_test_data(level=4)[0], False),
])
def test_is_perfect_recall(game_input, expected_result: bool):
    """
    Verify the IsPerfectRecall implementation against a suite of games
    with and without the perfect recall, from both files and generation.
    """
    game = None
    if isinstance(game_input, str):
        game = games.read_from_file(game_input)
    elif isinstance(game_input, gbt.Game):
        game = game_input
    else:
        pytest.fail(f"Unknown type for game_input: {type(game_input)}")

    assert game.is_perfect_recall == expected_result


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
        # 1 player; reduction; generic payoffs
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
                ["1*1", "1*2", "211", "212", "221", "222"],
                ["11*", "12*", "2**", "3*1", "3*2", "4**"],
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
                    "1*11",
                    "1*12",
                    "1*21",
                    "1*22",
                    "2111",
                    "2112",
                    "2121",
                    "2122",
                    "2211",
                    "2212",
                    "2221",
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
        # Centipede
        (games.Centipede.get_test_data(N=3, m0=2, m1=7)),
        (games.Centipede.get_test_data(N=4, m0=2, m1=7)),
        (games.Centipede.get_test_data(N=5, m0=2, m1=7)),
        (games.Centipede.get_test_data(N=3, m0=1, m1=3)),
        (games.Centipede.get_test_data(N=4, m0=1, m1=3)),
        (games.Centipede.get_test_data(N=5, m0=1, m1=3)),
        (games.Centipede.get_test_data(N=9, m0=3, m1=11)),
        # Two player binary tree
        (games.BinEfgTwoPlayer.get_test_data(level=1)),
        (games.BinEfgTwoPlayer.get_test_data(level=2)),
        (games.BinEfgTwoPlayer.get_test_data(level=3)),
        (games.BinEfgTwoPlayer.get_test_data(level=4)),
        (games.BinEfgTwoPlayer.get_test_data(level=5)),
        (games.BinEfgTwoPlayer.get_test_data(level=6)),
        # Three player binary tree
        (games.BinEfgThreePlayer.get_test_data(level=1)),
        (games.BinEfgThreePlayer.get_test_data(level=2)),
        (games.BinEfgThreePlayer.get_test_data(level=3)),
        (games.BinEfgThreePlayer.get_test_data(level=4)),
        (games.BinEfgThreePlayer.get_test_data(level=5)),
        # One player IR binary tree
        (games.BinEfgOnePlayerIR.get_test_data(level=1)),
        (games.BinEfgOnePlayerIR.get_test_data(level=2)),
        (games.BinEfgOnePlayerIR.get_test_data(level=3)),
        (games.BinEfgOnePlayerIR.get_test_data(level=4)),
        (games.BinEfgOnePlayerIR.get_test_data(level=5)),
        (games.BinEfgOnePlayerIR.get_test_data(level=6)),
        #
        # I M P E R F E C T   R E C A L L --- no absent-mindedness
        # Wichardt (2008)
        (
           games.read_from_file("wichardt.efg"),
           [["11", "12", "21", "22"], ["1", "2"]],
           [
               np.array([[1, -1], [-5, -5], [-5, -5], [-1, 1]]),
               np.array([[-1, 1], [5, 5], [5, 5], [1, -1]]),
           ],
        ),
        # variation of the Selten's Horse (1975): 2 players
        (
           games.read_from_file("noPR-action-selten-horse.efg"),
           [["11", "12", "21", "22"], ["1", "2"]],
           [
               np.array([[1, 4], [1, 0], [3, 3], [0, 0]]),
               np.array([[1, 4], [1, 0], [2, 2], [0, 0]]),
           ],
        ),
        # deflate-stable game
        (
           games.read_from_file("noPR-information-no-deflate.efg"),
           [["1", "2", "3"], ["11", "12", "21", "22"]],
           [
               np.array([[8, 8, 7, 7], [0, 0, 4, 3], [2, 1, 2, 1]]),
               np.array([[-8, -8, -7, -7], [0, 0, -4, -3], [-2, -1, -2, -1]]),
           ],
        ),
        # Gilboa (1997)
        (
           games.read_from_file("gilboa-two-am-agents-deterministic.efg"),
           [["11", "12", "21", "22"], ["1", "2"]],
           [
               np.array([[1, 4], [1, 6], [2, 5], [3, 6]]),
               np.array([[-1, -4], [-1, -6], [-2, -5], [-3, -6]]),
           ],
        ),
        #
        # A B S E N T   M I N D E D N E S S
        # AM-driver ---- NB: the legacy algorithm works well, pytest.mark.xfail is thus not added
        (
           games.read_from_file("noPR-AM-driver-one-player.efg"),
           [["11*", "12*", "2**"], ["*"]],
           [
               np.array([[1], [3], [4]]),
               np.array([[-1], [-3], [-4]]),
           ],
        ),
        pytest.param(
           games.read_from_file("noPR-action-AM.efg"),
           [["1", "2"], ["1**1", "1**2", "2**1", "2**2"]],
           [
               np.array([[1, 1, 2, 2], [7, 8, 7, 8]]),
               np.array([[-1, -1, -2, -2], [-7, -8, -7, -8]]),
           ],
           marks=pytest.mark.xfail
        ),
        pytest.param(
           games.read_from_file("noPR-action-AM2.efg"),
           [["11*", "12*", "2**"], ["*1", "*2"]],
           [
               np.array([[1, 1], [2, 2], [7, 8]]),
               np.array([[-1, -1], [-2, -2], [-7, -8]]),
           ],
           marks=pytest.mark.xfail
        ),
        pytest.param(
           games.read_from_file("noPR-action-big-AM.efg"),
           [["1***", "21**", "22*1", "22*2"], ["1****1", "1****2", "2****1", "2****2"]],
           [
               np.array(
                    [
                       [1, 1, 2, 2],
                       [17, 17, 17, 17],
                       [18, 19, 18, 19],
                       [18, 20, 18, 20]
                    ]
                ),
               np.array(
                    [
                        [-1, -1, -2, -2],
                        [-17, -17, -17, -17],
                        [-18, -19, -18, -19],
                        [-18, -20, -18, -20]
                    ]
                ),
           ],
           marks=pytest.mark.xfail
        ),
        pytest.param(
           games.read_from_file("noPR-action-AM-two-hops.efg"),
           [["11", "12", "2*"], ["1*", "2*"]],
           [
               np.array([[1, 0], [10, 0], [2, 2]]),
               np.array([[1, 3], [-10, 3], [0, 0]]),
           ],
           marks=pytest.mark.xfail
        ),
        pytest.param(
           games.read_from_file("noPR-action-AM-two-hops-unreached.efg"),
           [["1*", "2*"], ["1*", "2*"]],
           [
               np.array([[1, 0], [2, 2]]),
               np.array([[1, 3], [0, 0]]),
           ],
           marks=pytest.mark.xfail
        ),
    ],
)
def test_reduced_strategic_form(
    game: gbt.Game, strategy_labels: list, np_arrays_of_rsf: typing.Union[list, None]
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
        if np_arrays_of_rsf is not None:
            # convert strings to rationals
            exp_array = games.vectorized_make_rational(np_arrays_of_rsf[i])
            assert (arrays[i] == exp_array).all()
