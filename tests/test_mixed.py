import typing
from itertools import product

import pygambit as gbt
import pytest

from . import games

TOL = 1e-13  # tolerance for floating point assertions
ZERO = gbt.Rational(0)  # tolerance for rational assertions
P1 = "Player 1"
P2 = "Player 2"
P3 = "Player 3"


def _set_action_probs(profile: gbt.MixedStrategyProfile, probs: list, rational_flag: bool):
    """Set the action probabilities in a strategy profile called ```profile``` according to a
    list with probabilities in the order of ```profile.game.strategies```
    """
    for i, p in enumerate(probs):
        # assumes rationals given as strings
        profile[profile.game.strategies[i]] = gbt.Rational(p) if rational_flag else p


@pytest.mark.parametrize(
    "game,profile_data,rational_flag",
    [
        ###############################################################################
        # 4x4 coordination nfg
        (games.create_coord_4x4_nfg(), [[0, 0, 0, 0], ["1/3", "1/3", "1/3", 0]], True),
        (games.create_coord_4x4_nfg(), [[1, 0, 0, 0], [0, 0, 0, 0]], True),
        (games.create_coord_4x4_nfg(), [[0, 0, 0, 0], [1.0, 1.0, 1.0, 1.0]], False),
        (games.create_coord_4x4_nfg(), [[1.0, 1.0, 1.0, 1.0], [0, 0, 0, 0]], False),
        ###############################################################################
        # centipede with chance efg
        (games.create_centipede_game_with_chance_efg(), [[0, 0, 0, 0], [1, 0, 0, 0]], True),
        (games.create_centipede_game_with_chance_efg(), [[1, 0, 0, 0], [0, 0, 0, 0]], True),
        (games.create_centipede_game_with_chance_efg(), [[0, 0, 0, 0], [1, 0, 0, 0]], False),
        (games.create_centipede_game_with_chance_efg(), [[1, 0, 0, 0], [0, 0, 0, 0]], False),
    ],
)
def test_normalize_zero_value_error(game, profile_data, rational_flag):
    profile = game.mixed_strategy_profile(rational=rational_flag, data=profile_data)
    with pytest.raises(ValueError, match="zero"):
        profile.normalize()


@pytest.mark.parametrize(
    "game,profile_data,rational_flag",
    [
        ###############################################################################
        # 4x4 coordination nfg
        (games.create_coord_4x4_nfg(), [[1, 1, 0, -1], ["1/3", "1/3", "1/3", 0]], True),
        (games.create_coord_4x4_nfg(), [[0, 0, 0, -1.0], [1.0, 1.0, 1.0, 1.0]], False),
        ###############################################################################
        # zero matrix nfg
        (games.create_2x2_zero_nfg(), [[1, 0], [0, -1]], True),
        (games.create_2x2_zero_nfg(), [[1.0, 1.0], [0, -1.0]], False),
        ###############################################################################
        # centipede with chance efg
        (games.create_centipede_game_with_chance_efg(), [[-1, 0, 0, 0], [1, 0, 0, 0]], True),
        (games.create_centipede_game_with_chance_efg(), [[1, 0, 0, 0], [-1, 0, 0, 0]], True),
        (games.create_centipede_game_with_chance_efg(), [[-1, 0, 0, 0], [1, 0, 0, 0]], False),
        (games.create_centipede_game_with_chance_efg(), [[1, 0, 0, 0], [-1, 0, 0, 0]], False),
    ],
)
def test_normalize_neg_entry_value_error(game, profile_data, rational_flag):
    profile = game.mixed_strategy_profile(rational=rational_flag, data=profile_data)
    with pytest.raises(ValueError, match="negative"):
        profile.normalize()


@pytest.mark.parametrize(
    "game,profile_data,expected_data,rational_flag",
    [
        ###############################################################################
        # 4x4 coordination nfg
        (games.create_coord_4x4_nfg(), [[1, 2, 3, 14], [1, 1, 1, 1]],
            [["1/20", "2/20", "3/20", "14/20"], ["1/4", "1/4", "1/4", "1/4"]], True),
        (games.create_coord_4x4_nfg(), [[1.0, 2.0, 3.0, 14.0], [1, 1, 1, 1]],
            [[1 / 20, 2 / 20, 3 / 20, 14 / 20], [0.25, 0.25, 0.25, 0.25]], False),
        ###############################################################################
        # centipede with chance efg
        (games.create_centipede_game_with_chance_efg(), [[1, 2, 3, 14], [1, 1, 1, 1]],
            [["1/20", "2/20", "3/20", "14/20"], ["1/4", "1/4", "1/4", "1/4"]], True),
        (games.create_centipede_game_with_chance_efg(), [[1.0, 2.0, 3.0, 14.0], [1, 1, 1, 1]],
            [[1 / 20, 2 / 20, 3 / 20, 14 / 20], [0.25, 0.25, 0.25, 0.25]], False),
    ],
)
def test_normalize(game, profile_data, expected_data, rational_flag):
    assert (
        game.mixed_strategy_profile(data=profile_data, rational=rational_flag).normalize() ==
        game.mixed_strategy_profile(data=expected_data, rational=rational_flag)
    )


@pytest.mark.parametrize(
    "game,strategy_label,rational_flag,prob",
    [
     ##############################################################################
     # zero matrix nfg
     (games.create_2x2_zero_nfg(), "cooperate", False, 0.72),
     (games.create_2x2_zero_nfg(), "cooperate", True, "7/9"),
     ###############################################################################
     # coordination 4x4 nfg outcome version with strategy labels
     (games.create_coord_4x4_nfg(outcome_version=True), "1-1", 0.25, False),
     (games.create_coord_4x4_nfg(outcome_version=True), "1-1", "1/4", True),
     ###############################################################################
     # myerson 2 card poker efg
     (games.create_myerson_2_card_poker_efg(), "11", 0.25, False),
     (games.create_myerson_2_card_poker_efg(), "12", 0.15, False),
     (games.create_myerson_2_card_poker_efg(), "21", 0.99, False),
     (games.create_myerson_2_card_poker_efg(), "11", "1/4", True),
     (games.create_myerson_2_card_poker_efg(), "12", "3/4", True),
     (games.create_myerson_2_card_poker_efg(), "21", "7/9", True),
    ],
)
def test_set_and_get_probability_by_strategy_label(game: gbt.Game, strategy_label: str,
                                                   rational_flag: bool,
                                                   prob: typing.Union[float, str]):
    profile = game.mixed_strategy_profile(rational=rational_flag)
    prob = gbt.Rational(prob) if rational_flag else prob
    profile[strategy_label] = prob
    assert profile[strategy_label] == prob


@pytest.mark.parametrize(
    "game,player_label,rational_flag,profile_data",
    [
     ##############################################################################
     # zero matrix nfg
     (games.create_2x2_zero_nfg(), "Joe", False, [0.72, 0.28]),
     (games.create_2x2_zero_nfg(), "Joe", True, ["7/9", "2/9"]),
     ##############################################################################
     # coordination 4x4 nfg outcome version with strategy labels
     (games.create_coord_4x4_nfg(), P1, False, [0.25, 0, 0, 0.75]),
     (games.create_coord_4x4_nfg(), P1, True, ["1/4", 0, 0, "3/4"]),
     ##############################################################################
     # myerson 2 card poker efg
     (games.create_myerson_2_card_poker_efg(), P1, False, [0.25, 0.75, 0, 0]),
     (games.create_myerson_2_card_poker_efg(), P2, False, [1, 0]),
     (games.create_myerson_2_card_poker_efg(), P1, True, ["1/4", "3/4", 0, 0]),
     (games.create_myerson_2_card_poker_efg(), P2, True, [1, 0]),
    ],
)
def test_set_and_get_probabilities_by_player_label(game: gbt.Game, player_label: str,
                                                   rational_flag: bool, profile_data: list):
    profile_data = [gbt.Rational(p) for p in profile_data] if rational_flag else profile_data
    profile = game.mixed_strategy_profile(rational=rational_flag)
    profile[player_label] = profile_data
    assert profile[player_label] == profile_data


@pytest.mark.parametrize(
    "game,player_label,strategy_label,prob,rational_flag",
    [
     ##############################################################################
     # myerson 2 card poker efg
     # Player 1
     (games.create_myerson_2_card_poker_efg(), P1, "11", 0.25, False),
     (games.create_myerson_2_card_poker_efg(), P1, "12", 0.25, False),
     (games.create_myerson_2_card_poker_efg(), P1, "21", 0.25, False),
     (games.create_myerson_2_card_poker_efg(), P1, "22", 0.25, False),
     (games.create_myerson_2_card_poker_efg(), P1, "11", "1/4", True),
     (games.create_myerson_2_card_poker_efg(), P1, "12", "1/4", True),
     (games.create_myerson_2_card_poker_efg(), P1, "21", "1/4", True),
     (games.create_myerson_2_card_poker_efg(), P1, "22", "1/4", True),
     # Player 2
     (games.create_myerson_2_card_poker_efg(), P2, "1", 0.5, False),
     (games.create_myerson_2_card_poker_efg(), P2, "2", 0.5, False),
     (games.create_myerson_2_card_poker_efg(), P2, "1", "1/2", True),
     (games.create_myerson_2_card_poker_efg(), P2, "2", "1/2", True),
     ##############################################################################
     # coordination 4x4 nfg outcome version with strategy labels
     (games.create_coord_4x4_nfg(outcome_version=True), P1, "1-1", "1/4", True),
     (games.create_coord_4x4_nfg(outcome_version=True), P2, "2-1", "1/4", True),
    ]
)
def test_profile_indexing_by_player_and_strategy_label_reference(game: gbt.Game, player_label: str,
                                                                 strategy_label: str,
                                                                 prob: typing.Union[str, float],
                                                                 rational_flag: bool):
    profile = game.mixed_strategy_profile(rational=rational_flag)
    prob = gbt.Rational(prob) if rational_flag else prob
    assert profile[player_label][strategy_label] == prob


@pytest.mark.parametrize(
    "game,player_label,strategy_label,rational_flag",
    [
     ##############################################################################
     # myerson 2 card poker efg
     (games.create_myerson_2_card_poker_efg(), P2, "11", True),
     (games.create_myerson_2_card_poker_efg(), P2, "11", False),
     (games.create_myerson_2_card_poker_efg(), P1, "1", True),
     (games.create_myerson_2_card_poker_efg(), P1, "1", False),
     (games.create_myerson_2_card_poker_efg(), P1, "2", True),
     (games.create_myerson_2_card_poker_efg(), P1, "2", False),
     ##############################################################################
     # coordination 4x4 nfg outcome version with strategy labels
     (games.create_coord_4x4_nfg(outcome_version=True), P1, "2-1", True),
     (games.create_coord_4x4_nfg(outcome_version=True), P2, "1-1", True),
    ]
)
def test_profile_indexing_by_player_and_invalid_strategy_label(game: gbt.Game,
                                                               player_label: str,
                                                               strategy_label: str,
                                                               rational_flag: bool):
    """Test that we get a KeyError and that "player" appears in the error message"""
    with pytest.raises(KeyError, match="for player"):
        game.mixed_strategy_profile(rational=rational_flag)[player_label][strategy_label]


@pytest.mark.parametrize(
    "game,strategy_label,rational_flag,error,message",
    [
     ##############################################################################
     # myerson 2 card poker efg
     (games.create_myerson_2_card_poker_efg(), "13", True, KeyError, "player or strategy"),
     ##############################################################################
     # coordination 4x4 nfg payoff version (default strategy labels created with duplicates)
     (games.create_coord_4x4_nfg(), "1", True, ValueError, "multiple strategies"),
     (games.create_coord_4x4_nfg(), "2", True, ValueError, "multiple strategies"),
     (games.create_coord_4x4_nfg(), "3", True, ValueError, "multiple strategies"),
     (games.create_coord_4x4_nfg(), "4", True, ValueError, "multiple strategies"),
     (games.create_coord_4x4_nfg(), "5", True, KeyError, "player or strategy"),
     ]
)
def test_profile_indexing_by_invalid_strategy_label(game: gbt.Game, strategy_label: str,
                                                    rational_flag: bool,
                                                    error: typing.Union[ValueError, KeyError],
                                                    message: typing.Union[str, None]):
    """Check that we get a ValueError for an ambigious strategy label and a KeyError for one that
    is neither a player or strategy label in the game
    """
    with pytest.raises(error, match=message):
        game.mixed_strategy_profile(rational=rational_flag)[strategy_label]


def test_profile_indexing_by_player_and_duplicate_strategy_label():
    game = games.create_2x2_zero_nfg()
    profile = game.mixed_strategy_profile()
    with pytest.raises(ValueError):
        profile["Dan"]["defect"]


@pytest.mark.parametrize(
    "game,strategy_label,prob,rational_flag",
    [
     ###########################################################################
     # myerson 2 card poker efg
     # Player 1
     (games.create_myerson_2_card_poker_efg(), "11", 0.25, False),
     (games.create_myerson_2_card_poker_efg(), "12", 0.25, False),
     (games.create_myerson_2_card_poker_efg(), "21", 0.25, False),
     (games.create_myerson_2_card_poker_efg(), "22", 0.25, False),
     (games.create_myerson_2_card_poker_efg(), "11", "1/4", True),
     (games.create_myerson_2_card_poker_efg(), "12", "1/4", True),
     (games.create_myerson_2_card_poker_efg(), "21", "1/4", True),
     (games.create_myerson_2_card_poker_efg(), "22", "1/4", True),
     # Player 2
     (games.create_myerson_2_card_poker_efg(), "1", 0.5, False),
     (games.create_myerson_2_card_poker_efg(), "2", 0.5, False),
     (games.create_myerson_2_card_poker_efg(), "1", "1/2", True),
     (games.create_myerson_2_card_poker_efg(), "2", "1/2", True),
     ############################################################################
     # coordination 4x4 nfg outcome version with strategy labels
     # Player 1
     (games.create_coord_4x4_nfg(outcome_version=True), "1-1", "1/4", True),
     (games.create_coord_4x4_nfg(outcome_version=True), "1-1", 0.25, False),
     # Player 2
     (games.create_coord_4x4_nfg(outcome_version=True), "2-1", "1/4", True),
     (games.create_coord_4x4_nfg(outcome_version=True), "2-1", 0.25, False),
    ]
)
def test_profile_indexing_by_strategy_label_reference(game: gbt.Game, strategy_label: str,
                                                      prob: typing.Union[str, float],
                                                      rational_flag: bool):
    profile = game.mixed_strategy_profile(rational=rational_flag)
    prob = gbt.Rational(prob) if rational_flag else prob
    assert profile[strategy_label] == prob


@pytest.mark.parametrize(
    "game,player_label,strategy_data,rational_flag",
    [
     ############################################################################
     # mixed behav efg
     (games.create_mixed_behav_game_efg(), P1, [0.5, 0.5], False),
     (games.create_mixed_behav_game_efg(), P2, [0.5, 0.5], False),
     (games.create_mixed_behav_game_efg(), P3, [0.5, 0.5], False),
     (games.create_mixed_behav_game_efg(), P1, ["1/2", "1/2"], True),
     (games.create_mixed_behav_game_efg(), P2, ["1/2", "1/2"], True),
     (games.create_mixed_behav_game_efg(), P3, ["1/2", "1/2"], True),
     ############################################################################
     # myerson 2 card poker efg
     (games.create_myerson_2_card_poker_efg(), P1, [0.25, 0.25, 0.25, 0.25], False),
     (games.create_myerson_2_card_poker_efg(), P2, [0.5, 0.5], False),
     (games.create_myerson_2_card_poker_efg(), P1, ["1/4", "1/4", "1/4", "1/4"], True),
     (games.create_myerson_2_card_poker_efg(), P2, ["1/2", "1/2"], True),
     ############################################################################
     # coordination 4x4 nfg
     (games.create_coord_4x4_nfg(), P1, [0.25, 0.25, 0.25, 0.25], False),
     (games.create_coord_4x4_nfg(), P2, [0.25, 0.25, 0.25, 0.25], False),
     (games.create_coord_4x4_nfg(), P1, ["1/4", "1/4", "1/4", "1/4"], True),
     (games.create_coord_4x4_nfg(), P2, ["1/4", "1/4", "1/4", "1/4"], True),
    ]
)
def test_profile_indexing_by_player_label_reference(game: gbt.Game, player_label: str,
                                                    strategy_data: list, rational_flag: bool):
    profile = game.mixed_strategy_profile(rational=rational_flag)
    if rational_flag:
        strategy_data = [gbt.Rational(prob) for prob in strategy_data]
    assert profile[player_label] == strategy_data


@pytest.mark.parametrize(
    "game,rational_flag,profile_data,label,payoff",
    [
     #########################################################################
     # zero matrix nfg
     (games.create_2x2_zero_nfg(), False, None, "Joe", 0),
     (games.create_2x2_zero_nfg(), True,  None, "Joe", 0),
     #########################################################################
     # coordination 4x4 nfg
     (games.create_coord_4x4_nfg(), False, None, P1, 0.25),
     (games.create_coord_4x4_nfg(), True,  None, P1, "1/4"),
     (games.create_coord_4x4_nfg(), False, None, P2, 0.25),
     (games.create_coord_4x4_nfg(), True,  None, P2, "1/4"),
     (games.create_coord_4x4_nfg(), False, [[1, 0, 0, 0], [1, 0, 0, 0]], P1, 1),
     (games.create_coord_4x4_nfg(), True,  [[1, 0, 0, 0], [1, 0, 0, 0]], P1, 1),
     (games.create_coord_4x4_nfg(), False, [[1, 0, 0, 0], [1, 0, 0, 0]], P2, 1),
     (games.create_coord_4x4_nfg(), True,  [[1, 0, 0, 0], [1, 0, 0, 0]], P2, 1),
     (games.create_coord_4x4_nfg(), False, [[1, 0, 0, 0], [0, 1, 0, 0]], P1, 0),
     (games.create_coord_4x4_nfg(), True,  [[1, 0, 0, 0], [0, 1, 0, 0]], P1, 0),
     (games.create_coord_4x4_nfg(), False, [[1, 0, 0, 0], [0, 1, 0, 0]], P2, 0),
     (games.create_coord_4x4_nfg(), True,  [[1, 0, 0, 0], [0, 1, 0, 0]], P2, 0),
     #########################################################################
     # myerson 2 card poker efg
     (games.create_myerson_2_card_poker_efg(), False, None, P1, -1.25),
     (games.create_myerson_2_card_poker_efg(), False, None, P2, 1.25),
     (games.create_myerson_2_card_poker_efg(), True,  None, P1, "-5/4"),
     (games.create_myerson_2_card_poker_efg(), True,  None, P2, "5/4"),
     # Raise/Raise for player 1
     (games.create_myerson_2_card_poker_efg(), False, [[1, 0, 0, 0], [1, 0]], P1, -1),
     (games.create_myerson_2_card_poker_efg(), False, [[1, 0, 0, 0], [1, 0]], P2, 1),
     (games.create_myerson_2_card_poker_efg(), True,  [[1, 0, 0, 0], [1, 0]], P1, -1),
     (games.create_myerson_2_card_poker_efg(), True,  [[1, 0, 0, 0], [1, 0]], P2, 1),
     # Fold/Fold for player 1 (player 2's strategy is payoff-irrelevant)
     (games.create_myerson_2_card_poker_efg(), False, [[0, 0, 0, 1], [1, 0]], P1, -2),
     (games.create_myerson_2_card_poker_efg(), False, [[0, 0, 0, 1], [1, 0]], P2, 2),
     (games.create_myerson_2_card_poker_efg(), True,  [[0, 0, 0, 1], [1, 0]], P1, -2),
     (games.create_myerson_2_card_poker_efg(), True,  [[0, 0, 0, 1], [1, 0]], P2, 2),
     (games.create_myerson_2_card_poker_efg(), False, [[0, 0, 0, 1], [0.5, 0.5]], P1, -2),
     (games.create_myerson_2_card_poker_efg(), False, [[0, 0, 0, 1], [0.5, 0.5]], P2, 2),
     (games.create_myerson_2_card_poker_efg(), True,  [[0, 0, 0, 1], ["1/2", "1/2"]], P1, -2),
     (games.create_myerson_2_card_poker_efg(), True,  [[0, 0, 0, 1], ["1/2", "1/2"]], P2, 2),
     #########################################################################
     (games.create_mixed_behav_game_efg(), False, None, P1, 3.0),
     (games.create_mixed_behav_game_efg(), False, None, P2, 3.0),
     (games.create_mixed_behav_game_efg(), False, None, P3, 3.25),
     (games.create_mixed_behav_game_efg(), True, None,  P1, 3),
     (games.create_mixed_behav_game_efg(), True, None,  P2, 3),
     (games.create_mixed_behav_game_efg(), True, None,  P3, "13/4"),
    ]
)
def test_payoff_by_label_reference(game: gbt.Game, rational_flag: bool, profile_data: list,
                                   label: str, payoff: typing.Union[float, str]):
    payoff = gbt.Rational(payoff) if rational_flag else payoff
    profile = game.mixed_strategy_profile(rational=rational_flag, data=profile_data)
    assert profile.payoff(label) == payoff


@pytest.mark.parametrize(
    "game,rational_flag,label,value",
    [
     ##############################################################################
     # zero matrix nfg
     (games.create_2x2_zero_nfg(), False, "cooperate", 0),
     (games.create_2x2_zero_nfg(), True, "cooperate", 0),
     ##############################################################################
     # coordination 4x4 nfg
     (games.create_coord_4x4_nfg(outcome_version=True), False, "1-1", 0.25),
     (games.create_coord_4x4_nfg(outcome_version=True), True, "1-1", "1/4"),
     ##############################################################################
     # myerson 2 card poker efg
     (games.create_myerson_2_card_poker_efg(), False, "11", -0.5),  # Raise/Raise
     (games.create_myerson_2_card_poker_efg(), False, "12", -0.75),  # Raise Red/Fold Black
     (games.create_myerson_2_card_poker_efg(), False, "21", -1.75),  # Fold Red/Raise Black
     (games.create_myerson_2_card_poker_efg(), False, "22", -2),  # Fold/Fold
     (games.create_myerson_2_card_poker_efg(), True, "11", "-1/2"),
     (games.create_myerson_2_card_poker_efg(), True, "12", "-3/4"),
     (games.create_myerson_2_card_poker_efg(), True, "21", "-7/4"),
     (games.create_myerson_2_card_poker_efg(), True, "22", -2),
    ]
)
def test_strategy_value_by_label_reference(game: gbt.Game, rational_flag: bool, label: str,
                                           value: typing.Union[float, str]):
    value = gbt.Rational(value) if rational_flag else value
    assert game.mixed_strategy_profile(rational=rational_flag).strategy_value(label) == value


@pytest.mark.parametrize(
    "game,rational_flag",
    [
        (games.create_mixed_behav_game_efg(), False),
        (games.create_mixed_behav_game_efg(), True),
        (games.create_centipede_game_with_chance_efg(), False),
        (games.create_centipede_game_with_chance_efg(), True),
    ]
)
def test_as_behavior_roundtrip(game: gbt.Game, rational_flag: bool):
    assert (
        game.mixed_strategy_profile(rational=rational_flag).as_behavior().as_strategy() ==
        game.mixed_strategy_profile(rational=rational_flag)
    )


@pytest.mark.parametrize(
    "game,rational_flag",
    [
        (games.create_2x2_zero_nfg(), False),
        (games.create_2x2_zero_nfg(), True),
        (games.create_coord_4x4_nfg(), False),
        (games.create_coord_4x4_nfg(), True),
    ]
)
def test_as_behavior_error(game: gbt.Game, rational_flag: bool):
    with pytest.raises(gbt.UndefinedOperationError):
        _ = game.mixed_strategy_profile(rational=rational_flag).as_behavior()


@pytest.mark.parametrize(
    "game,profile_data,rational_flag,payoffs",
    [
     ###############################################################################
     # zero matrix nfg
     (games.create_2x2_zero_nfg(), None, True, (0, 0)),
     ###############################################################################
     # 4x4 coordination nfg
     (games.create_coord_4x4_nfg(), None, False, (0.25, 0.25)),
     (games.create_coord_4x4_nfg(), None, True, ("1/4", "1/4")),
     (games.create_coord_4x4_nfg(),
         [["1/3", "1/3", "1/3", 0], ["1/3", "1/3", "1/3", 0]], True, ("1/3", "1/3")),
     (games.create_coord_4x4_nfg(),
         [["1/3", "1/3", 0, "1/3"], ["1/3", "1/3", "1/3", 0]], True, ("2/9", "2/9")),
     ###############################################################################
     # 2x2x2 nfg
     (games.create_2x2x2_nfg(), None, True, ("4/8", "16/8", "4/8")),
     (games.create_2x2x2_nfg(), [[1, 0], [1, 0], [1, 0]], True, (0, 0, 0)),
     (games.create_2x2x2_nfg(), [[0, 1], [1, 0], [1, 0]], True, (1, 2, -1)),
     (games.create_2x2x2_nfg(), [["1/2", "1/2"], [1, 0], [1, 0]], True, ("1/2", 1, "-1/2")),
    ],
)
def test_payoffs_reference(game: gbt.Game, profile_data: list, rational_flag: bool,
                           payoffs: tuple):
    profile = game.mixed_strategy_profile(rational=rational_flag, data=profile_data)
    for payoff, player in zip(payoffs, profile.game.players):
        payoff = gbt.Rational(payoff) if rational_flag else payoff
        assert profile.payoff(player) == payoff


@pytest.mark.parametrize(
    "game,profile_data,rational_flag,strategy_values",
    [
     ###############################################################################
     # zero matrix nfg
     (games.create_2x2_zero_nfg(), None, False, ([0, 0], [0, 0])),
     (games.create_2x2_zero_nfg(), None, True, ([0, 0], [0, 0])),
     ###############################################################################
     # 4x4 coordination nfg
     (games.create_coord_4x4_nfg(), None, False,
      ([0.25, 0.25, 0.25, 0.25], [0.25, 0.25, 0.25, 0.25])),
     (games.create_coord_4x4_nfg(), None, True,
      ([0.25, 0.25, 0.25, 0.25], [0.25, 0.25, 0.25, 0.25])),
     (games.create_coord_4x4_nfg(), [["1", "0", "0", "0"], ["1", "0", "0", "0"]],
         True, (["1", "0", "0", "0"], ["1", "0", "0", "0"])),
     (games.create_coord_4x4_nfg(), [["3/7", "0", "0", "4/7"], ["1/3", "1/3", "1/3", "0"]],
         True, (["1/3", "1/3", "1/3", "0"], ["3/7", "0", "0", "4/7"])),
     ###############################################################################
     # 2x2x2 nfg
     (games.create_2x2x2_nfg(), None, True, (["1/2", "1/2"], [2, 2], ["1/2", "1/2"])),
     (games.create_2x2x2_nfg(), [[1, 0], [1, 0], [1, 0]], True, ([0, 1], [0, 4], [0, 1])),
     ###############################################################################
     # myerson 2 card poker efg
     (games.create_myerson_2_card_poker_efg(), None, False, [(-0.5, -0.75, -1.75, -2), (1.5, 1)]),
    ]
)
def test_strategy_value_reference(game: gbt.Game, profile_data: list, rational_flag: bool,
                                  strategy_values: list):
    profile = game.mixed_strategy_profile(rational=rational_flag, data=profile_data)
    for strategy_values_for_player, player in zip(
        strategy_values, profile.game.players
    ):
        for i, s in enumerate(player.strategies):
            sv = strategy_values_for_player[i]
            sv = gbt.Rational(sv) if rational_flag else sv
            assert profile.strategy_value(s) == sv


@pytest.mark.parametrize(
    "game,profile_data,liap_expected,tol,rational_flag",
    [
     ##############################################################################
     # Zero matrix nfg, all liap_values are zero
     (games.create_2x2_zero_nfg(), [["3/4", "1/4"], ["2/5", "3/5"]], 0, ZERO, True),
     (games.create_2x2_zero_nfg(), [["1/2", "1/2"], ["1/2", "1/2"]], 0, ZERO, True),
     (games.create_2x2_zero_nfg(), [[1, 0], [1, 0]], 0, ZERO, True),
     (games.create_2x2_zero_nfg(), [[1/4, 3/4], [2/5, 3/5]], 0, TOL, False),
     ##############################################################################
     # 4x4 coordination nfg
     (games.create_coord_4x4_nfg(), None, 0, ZERO, True),
     (games.create_coord_4x4_nfg(), None, 0, TOL, False),
     (games.create_coord_4x4_nfg(),
      [["1/3", "1/2", "1/12", "1/12"], ["3/8", "1/8", "1/4", "1/4"]], "245/2304", ZERO, True),
     (games.create_coord_4x4_nfg(),
      [["1/4", "1/4", "1/4", "1/4"], ["1/4", "1/4", "1/4", "1/4"]], 0, ZERO, True),
     (games.create_coord_4x4_nfg(), [[1, 0, 0, 0], [1, 0, 0, 0]], 0, ZERO, True),
     (games.create_coord_4x4_nfg(), [[1/3, 1/2, 1/12, 1/12], [3/8, 1/8, 1/4, 1/4]],
      245/2304, TOL, False),
     ##############################################################################
     # El Farol bar game efg
     (games.create_el_farol_bar_game_efg(),
      [["1/2", "1/2"], ["1/2", "1/2"], ["1/2", "1/2"], ["1/2", "1/2"], ["1/2", "1/2"]], "0",
      ZERO, True),
     (games.create_el_farol_bar_game_efg(),
      [["1/1", "0/1"], ["1/1", "0/1"], ["0/1", "1/1"], ["0/1", "1/1"], ["0/1", "1/1"]], "0",
      ZERO, True),
     ##############################################################################
     # 2x2x2 nfg with 2 pure and 1 mixed eq
     (games.create_2x2x2_nfg(), [[1, 0], [1, 0], [1, 0]], 18, ZERO, True),  # 4^2+1+1
     (games.create_2x2x2_nfg(), [[0, 1], [0, 1], [0, 1]], 18, ZERO, True),  # 4^2+1+1
     (games.create_2x2x2_nfg(), [[1, 0], [0, 1], [1, 0]], 0, ZERO, True),
     (games.create_2x2x2_nfg(), [[0, 1], [1, 0], [0, 1]], 0, ZERO, True),
     (games.create_2x2x2_nfg(), None, 0, ZERO, True),  # uniform is Nash
    ]
)
def test_liapunov_value_reference(game: gbt.Game, profile_data: list,
                                  liap_expected: typing.Union[float, str],
                                  tol: typing.Union[float, gbt.Rational, int],
                                  rational_flag: bool):
    liap_expected = gbt.Rational(liap_expected) if rational_flag else liap_expected
    profile = game.mixed_strategy_profile(rational=rational_flag, data=profile_data)
    assert abs(profile.liap_value() - liap_expected) <= tol


@pytest.mark.parametrize(
    "game,rational_flag",
    [
     #################################################################################
     # 4x4 coordination nfg
     (games.create_coord_4x4_nfg(), False),
     (games.create_coord_4x4_nfg(), True),
     #################################################################################
     # Zero matrix nfg
     (games.create_2x2_zero_nfg(), False),
     (games.create_2x2_zero_nfg(), True),
     #################################################################################
     # El Farol bar game efg
     (games.create_el_farol_bar_game_efg(), False),
     (games.create_el_farol_bar_game_efg(), True),
     #################################################################################
     # Centipede with chance efg
     (games.create_centipede_game_with_chance_efg(), False),
     (games.create_centipede_game_with_chance_efg(), True),
     #################################################################################
     # 2x2x2 nfg
     (games.create_2x2x2_nfg(), False),
     (games.create_2x2x2_nfg(), True),
    ]
)
def test_strategy_regret_consistency(game: gbt.Game, rational_flag: bool):
    profile = game.mixed_strategy_profile(rational=False)
    for player in game.players:
        for strategy in player.strategies:
            assert (
                profile.strategy_regret(strategy) ==
                (
                    max(profile.strategy_value(s) for s in player.strategies)
                    - profile.strategy_value(strategy)
                )
            )


@pytest.mark.parametrize(
    "game,profile_data,tol,rational_flag",
    [
     #################################################################################
     # 4x4 coordination nfg
     (games.create_coord_4x4_nfg(),
      [["1/5", "2/5", "0/5", "2/5"], ["3/8", "1/4", "3/8", "0/4"]], ZERO, True),
     (games.create_coord_4x4_nfg(),
      [[1/3, 1/3, 0/3, 1/3], [1/4, 1/4, 3/8, 1/8]], TOL, False),
     #################################################################################
     # Centipede with chance efg
     (games.create_centipede_game_with_chance_efg(),
      [["1/3", "1/3", "1/3", "0/1"], ["1/10", "3/5", "3/10", "0/1"]], ZERO, True),
     (games.create_centipede_game_with_chance_efg(),
      [[1/3, 1/3, 1/3, 0], [.10, 3/5, .3, 0]], TOL, False),
     #################################################################################
     # El Faor bar game efg
     (games.create_el_farol_bar_game_efg(),
      [[1, 0], ["1/2", "1/2"], ["1/3", "2/3"], ["1/5", "4/5"], ["1/8", "7/8"]], ZERO, True),
     (games.create_el_farol_bar_game_efg(),
      [[1, 0], [1/2, 1/2], [1/3, 2/3], [1/5, 4/5], [1/8, 7/8]], TOL, False),
     #################################################################################
     # 2x2x2 nfg with 2 pure and 1 mixed eq
     (games.create_2x2x2_nfg(), None, ZERO, True),
     (games.create_2x2x2_nfg(), [[1, 0], [1, 0], [1, 0]], ZERO, True),
     (games.create_2x2x2_nfg(), None, TOL, False),
     (games.create_2x2x2_nfg(), [[1, 0], [1, 0], [1, 0]], TOL, False),
    ]
)
def test_liapunov_value_consistency(game: gbt.Game, profile_data: list,
                                    tol: typing.Union[float, gbt.Rational],
                                    rational_flag: bool):
    profile = game.mixed_strategy_profile(rational=rational_flag, data=profile_data)

    assert (
        abs(profile.liap_value() -
            sum([max(profile.strategy_value(strategy) - profile.payoff(player), 0)**2
                for player in game.players for strategy in player.strategies])) <= tol
    )


@pytest.mark.parametrize(
    "game,profile1,profile2,alpha,tol,rational_flag",
    [
     #################################################################################
     # 4x4 coordination nfg
     (games.create_coord_4x4_nfg(),
      [["1/5", "2/5", "0/5", "2/5"], ["3/8", "1/4", "3/8", "0/4"]],
      [["1/5", "2/5", "0/5", "2/5"], ["1/4", "3/8", "0/4", "3/8"]],
      gbt.Rational("3/5"), ZERO, True),
     (games.create_coord_4x4_nfg(),
      [[1/5, 2/5, 0/5, 2/5], [3/8, 1/4, 3/8, 0/4]], [[1/5, 2/5, 0/5, 2/5], [1/4, 3/8, 0/4, 3/8]],
      3/5, TOL, False),
     #################################################################################
     # Zero matrix nfg
     (games.create_2x2_zero_nfg(),
      [["1/4", "3/4"], ["3/5", "2/5"]], [["1/2", "1/2"], ["3/5", "2/5"]],
      gbt.Rational("5/6"), ZERO, True),
     #################################################################################
     # Centipede game with chance
     (games.create_centipede_game_with_chance_efg(),
      [["1/3", "1/3", "1/3", "0/1"], ["1/10", "3/5", "3/10", "0/1"]],
      [["1/3", "1/3", "1/3", "0/1"], ["1/5", "2/5", "1/5", "1/5"]],
      gbt.Rational("1/12"), ZERO, True),
     (games.create_centipede_game_with_chance_efg(),
      [[1/3, 1/3, 1/3, 0/1], [1/10, 3/5, 3/10, 0/1]], [[1/3, 1/3, 1/3, 0/1], [1/5, 2/5, 1/5, 1/5]],
      1/12, TOL, False),
     #################################################################################
     # Selten's horse game
     (games.create_selten_horse_game_efg(),
      [["4/9", "5/9"], ["1/11", "10/11"], ["8/9", "1/9"]],
      [["4/9", "5/9"], ["10/11", "1/11"], ["8/9", "1/9"]],
      gbt.Rational("4/9"), ZERO, True),
     #################################################################################
     # El Farol bar game
     (games.create_el_farol_bar_game_efg(),
      [["4/9", "5/9"], ["1/3", "2/3"], ["1/2", "1/2"], ["11/12", "1/12"], ["1/2", "1/2"]],
      [["4/9", "5/9"], ["1/3", "2/3"], ["1/2", "1/2"], ["1/12", "11/12"], ["1/2", "1/2"]],
      gbt.Rational("1/2"), ZERO, True),
    ]
)
def test_linearity_payoff_property(game: gbt.Game, profile1: list, profile2: list,
                                   alpha: typing.Union[float, gbt.Rational],
                                   tol: typing.Union[float, gbt.Rational], rational_flag: bool):
    profile1 = game.mixed_strategy_profile(rational=rational_flag, data=profile1)
    profile2 = game.mixed_strategy_profile(rational=rational_flag, data=profile2)

    profile_data = [[alpha*profile1[strategy] + (1-alpha)*profile2[strategy]
                    for strategy in player.strategies] for player in game.players]
    profile3 = game.mixed_strategy_profile(rational=rational_flag, data=profile_data)

    for player in game.players:
        assert (
           abs(alpha*profile1.payoff(player) + (1 - alpha)*profile2.payoff(player) -
               profile3.payoff(player)) <= tol
        )


@pytest.mark.parametrize(
    "game,profile_data,tol,rational_flag",
    [
     #################################################################################
     # 4x4 coordination nfg
     (games.create_coord_4x4_nfg(),
      [["1/5", "2/5", "0/5", "2/5"], ["1/4", "3/8", "0/4", "3/8"]], ZERO, True),
     (games.create_coord_4x4_nfg(), [[0.2, 0.4, 0, 0.4], [1/4, 3/8, 0, 3/8]], TOL, False),
     (gbt.Game.from_arrays([[1, 2], [-3, 4]], [[-4, 3], [2, 1]]), [[1/2, 1/2], [3/5, 2/5]],
      TOL, False),
     #################################################################################
     # Zero matrix nfg
     (games.create_2x2_zero_nfg(), [["4/5", "1/5"], ["4/7", "3/7"]], ZERO, True),
     #################################################################################
     # Centipede game with chance
     (games.create_centipede_game_with_chance_efg(),
      [["1/5", "2/5", "1/5", "1/5"], ["1/10", "3/5", "3/10", "0/1"]], ZERO, True),
     (games.create_centipede_game_with_chance_efg(),
      [[1/3, 1/3, 1/3, 0/1], [1/10, 3/5, 3/10, 0/1]], TOL, False),
     #################################################################################
     # Selten's horse
     (games.create_selten_horse_game_efg(), [["4/9", "5/9"], ["6/11", "5/11"], ["4/7", "3/7"]],
      ZERO, True),
     (games.create_selten_horse_game_efg(), [[4/9, 5/9], [6/11, 5/11], [4/7, 3/7]], TOL, False),
     #################################################################################
     # El Farol bar game
     (games.create_el_farol_bar_game_efg(),
      [["4/9", "5/9"], ["1/3", "2/3"], ["0/1", "1/1"], ["11/12", "1/12"], ["1/3", "2/3"]],
      ZERO, True),
    ]
)
def test_payoff_and_strategy_value_consistency(game: gbt.Game, profile_data: list,
                                               tol: typing.Union[float, gbt.Rational],
                                               rational_flag: bool):
    profile = game.mixed_strategy_profile(rational=rational_flag, data=profile_data)
    for player in game.players:
        assert (
           abs(sum([profile[player][strategy] * profile.strategy_value(strategy)
                    for strategy in player.strategies]) - profile.payoff(player)) <= tol
        )


@pytest.mark.parametrize(
    "game,profile1,profile2,alpha,rational_flag,tol",
    [
     #################################################################################
     # 4x4 coordination nfg
     (games.create_coord_4x4_nfg(),
      [["1/1111", "10/1111", "100/1111", "1000/1111"], ["1/4", "1/8", "3/8", "1/4"]],
      [["1/1111", "10/1111", "99/1111", "1001/1111"], ["1/4", "1/8", "3/8", "1/4"]], "1/2", True,
      ZERO),
     (games.create_coord_4x4_nfg(),
      [[1/1111, 10/1111, 100/1111, 1000/1111], [1/4, 1/8, 3/8, 1/4]],
      [[1/1111, 10/1111, 99/1111, 1001/1111], [1/4, 1/8, 3/8, 1/4]], 1/2, False, TOL),
     #################################################################################
     # centipede game with chance
     (games.create_centipede_game_with_chance_efg(),
      [["1/3", "1/3", "1/3", "0"], ["1/10", "3/5", "3/10", "0"]],
      [["1/3", "1/3", "1/3", "0"], ["1/10", "3/5", "3/10", "0"]], "82943/62500", True, ZERO),
     (games.create_centipede_game_with_chance_efg(),
      [[1/3, 1/3, 1/3, 0], [1/10, 3/5, 3/10, 0]],
      [[1/3, 1/3, 1/3, 0], [1/10, 3/5, 3/10, 0]], 82943/62500, False, TOL),
    ]
)
def test_property_linearity_strategy_value(game: gbt.Game, profile1: list, profile2: list,
                                           alpha: typing.Union[float, str], rational_flag: bool,
                                           tol: typing.Union[float, gbt.Rational]):

    alpha = gbt.Rational(alpha) if rational_flag else alpha

    profile1 = game.mixed_strategy_profile(rational=rational_flag, data=profile1)
    profile2 = game.mixed_strategy_profile(rational=rational_flag, data=profile2)

    profile_data = [[alpha*profile1[strategy] + (1-alpha)*profile2[strategy]
                    for strategy in player.strategies] for player in game.players]
    profile3 = game.mixed_strategy_profile(rational=rational_flag, data=profile_data)

    for player in game.players:
        for strategy in player.strategies:
            convex_comb = alpha * profile1.strategy_value(strategy) +\
                         (1-alpha) * profile2.strategy_value(strategy)
            assert abs(profile3.strategy_value(strategy) - convex_comb) <= tol


def _get_answers_one_order(game: gbt.Game, action_probs_1st: tuple, action_probs_2nd: tuple,
                           rational_flag: bool, func_to_test: typing.Callable,
                           object_to_test_on: typing.Any):
    """helper function for the 'profile_order' caching tests"""
    ret = dict()
    profile = game.mixed_strategy_profile(rational=rational_flag)
    _set_action_probs(profile, action_probs_1st, rational_flag)
    ret[action_probs_1st] = func_to_test(profile, object_to_test_on)
    _set_action_probs(profile, action_probs_2nd, rational_flag)  # change the profile in place
    ret[action_probs_2nd] = func_to_test(profile, object_to_test_on)
    return ret


def _get_and_check_answers(game: gbt.Game, action_probs1: tuple, action_probs2: tuple,
                           rational_flag: bool, func_to_test: typing.Callable,
                           objects_to_test_on: typing.Collection):
    """helper function for the 'profile_order' caching tests"""
    order1_answers = {o: _get_answers_one_order(game, action_probs1, action_probs2, rational_flag,
                                                func_to_test, o) for o in objects_to_test_on}
    order2_answers = {o: _get_answers_one_order(game, action_probs2, action_probs1, rational_flag,
                                                func_to_test, o) for o in objects_to_test_on}
    assert order1_answers == order2_answers


# For 4x4 coord nfg:
PROBS_1A_doub = (0.25, 0.25, 0.25, 0.25)
PROBS_2A_doub = (0.5, 0, 0.5, 0)
PROBS_1A_rat = ("1/4", "1/4", "1/4", "1/4")
PROBS_2A_rat = ("1/2", "0", "1/2", "0")
# For 2x2x2 nfg and Myserson 2-card poker efg (both have 6 strategies in total):
PROBS_1B_doub = (0.5, 0.5, 0.5, 0.5, 0.5, 0.5)
PROBS_2B_doub = (1.0, 0.0, 1.0, 0.0, 1.0, 0.0)
PROBS_1B_rat = ("1/2", "1/2", "1/2", "1/2", "1/2", "1/2")
PROBS_2B_rat = ("1", "0", "1", "0", "1", "0")


@pytest.mark.parametrize(
    "game,action_probs1,action_probs2,rational_flag,func_to_test,objects_to_test",
    [
     #################################################################################
     # payoffs (for players)
     #######################
     # 4x4 coordination nfg
     pytest.param(games.create_coord_4x4_nfg(), PROBS_1A_doub, PROBS_2A_doub, False,
                  lambda profile, player: profile.payoff(player), lambda game: game.players,
                  id="payoffs_coord_doub"),
     pytest.param(games.create_coord_4x4_nfg(), PROBS_1A_rat, PROBS_2A_rat, True,
                  lambda profile, player: profile.payoff(player), lambda game: game.players,
                  id="payoffs_coord_rat"),
     # 2x2x2 nfg
     pytest.param(games.create_2x2x2_nfg(), PROBS_1B_doub, PROBS_2B_doub, False,
                  lambda profile, player: profile.payoff(player), lambda game: game.players,
                  id="payoffs_2x2x2_doub"),
     pytest.param(games.create_2x2x2_nfg(), PROBS_1B_rat, PROBS_2B_rat, True,
                  lambda profile, player: profile.payoff(player), lambda game: game.players,
                  id="payoffs_2x2x2_rat"),
     # Myerson 2-card poker efg
     pytest.param(games.create_myerson_2_card_poker_efg(), PROBS_1B_doub, PROBS_2B_doub, False,
                  lambda profile, player: profile.payoff(player), lambda game: game.players,
                  id="payoffs_poker_doub"),
     pytest.param(games.create_myerson_2_card_poker_efg(), PROBS_1B_rat, PROBS_2B_rat, True,
                  lambda profile, player: profile.payoff(player), lambda game: game.players,
                  id="payoffs_poker_rat"),
     #################################################################################
     # regret (for strategies)
     # 4x4 coordination nfg
     pytest.param(games.create_coord_4x4_nfg(), PROBS_1A_doub, PROBS_2A_doub, False,
                  lambda profile, strategy: profile.strategy_regret(strategy),
                  lambda game: game.strategies, id="regret_coord_doub"),
     pytest.param(games.create_coord_4x4_nfg(), PROBS_1A_rat, PROBS_2A_rat, True,
                  lambda profile, strategy: profile.strategy_regret(strategy),
                  lambda game: game.strategies, id="regret_coord_rat"),
     # 2x2x2 nfg
     pytest.param(games.create_2x2x2_nfg(), PROBS_1B_doub, PROBS_2B_doub, False,
                  lambda profile, strategy: profile.strategy_regret(strategy),
                  lambda game: game.strategies, id="regret_2x2x2_doub"),
     pytest.param(games.create_2x2x2_nfg(), PROBS_1B_rat, PROBS_2B_rat, True,
                  lambda profile, strategy: profile.strategy_regret(strategy),
                  lambda game: game.strategies, id="regret_2x2x2_rat"),
     # Myerson 2-card poker efg
     pytest.param(games.create_myerson_2_card_poker_efg(), PROBS_1B_doub, PROBS_2B_doub, False,
                  lambda profile, strategy: profile.strategy_regret(strategy),
                  lambda game: game.strategies, id="regret_poker_doub"),
     pytest.param(games.create_myerson_2_card_poker_efg(), PROBS_1B_rat, PROBS_2B_rat, True,
                  lambda profile, strategy: profile.strategy_regret(strategy),
                  lambda game: game.strategies, id="regret_poker_rat"),
     #################################################################################
     # strategy_value (for strategies)
     # 4x4 coordination nfg
     pytest.param(games.create_coord_4x4_nfg(), PROBS_1A_doub, PROBS_2A_doub, False,
                  lambda profile, strategy: profile.strategy_value(strategy),
                  lambda game: game.strategies, id="strat_value_coord_doub"),
     pytest.param(games.create_coord_4x4_nfg(), PROBS_1A_rat, PROBS_2A_rat, True,
                  lambda profile, strategy: profile.strategy_value(strategy),
                  lambda game: game.strategies, id="strat_value_coord_rat"),
     # 2x2x2 nfg
     pytest.param(games.create_2x2x2_nfg(), PROBS_1B_doub, PROBS_2B_doub, False,
                  lambda profile, strategy: profile.strategy_value(strategy),
                  lambda game: game.strategies, id="strat_value_2x2x2_doub"),
     pytest.param(games.create_2x2x2_nfg(), PROBS_1B_rat, PROBS_2B_rat, True,
                  lambda profile, strategy: profile.strategy_value(strategy),
                  lambda game: game.strategies, id="strat_value_2x2x2_rat"),
     # Myerson 2-card poker efg
     pytest.param(games.create_myerson_2_card_poker_efg(), PROBS_1B_doub, PROBS_2B_doub, False,
                  lambda profile, strategy: profile.strategy_value(strategy),
                  lambda game: game.strategies, id="strat_value_poker_doub"),
     pytest.param(games.create_myerson_2_card_poker_efg(), PROBS_1B_rat, PROBS_2B_rat, True,
                  lambda profile, strategy: profile.strategy_value(strategy),
                  lambda game: game.strategies, id="strat_value_poker_rat"),
     #################################################################################
     # strategy_value_deriv (for strategies * strategies)
     # 4x4 coordination nfg
     pytest.param(games.create_coord_4x4_nfg(), PROBS_1A_doub, PROBS_2A_doub, False,
                  lambda profile, strat_pair: profile.strategy_value_deriv(strategy=strat_pair[0],
                  other=strat_pair[1]),
                  lambda game: list(product(game.strategies, game.strategies)),
                  id="strat_value_deriv_coord_doub"),
     pytest.param(games.create_coord_4x4_nfg(), PROBS_1A_rat, PROBS_2A_rat, True,
                  lambda profile, strat_pair: profile.strategy_value_deriv(strategy=strat_pair[0],
                  other=strat_pair[1]),
                  lambda game: list(product(game.strategies, game.strategies)),
                  id="strat_value_deriv_coord_rat"),
     # 2x2x2 nfg
     pytest.param(games.create_2x2x2_nfg(), PROBS_1B_doub, PROBS_2B_doub, False,
                  lambda profile, strat_pair: profile.strategy_value_deriv(strategy=strat_pair[0],
                  other=strat_pair[1]),
                  lambda game: list(product(game.strategies, game.strategies)),
                  id="strat_value_deriv_2x2x2_doub"),
     pytest.param(games.create_2x2x2_nfg(), PROBS_1B_rat, PROBS_2B_rat, True,
                  lambda profile, strat_pair: profile.strategy_value_deriv(strategy=strat_pair[0],
                  other=strat_pair[1]),
                  lambda game: list(product(game.strategies, game.strategies)),
                  id="strat_value_deriv_2x2x2_rat"),
     # Myerson 2-card poker efg
     pytest.param(games.create_myerson_2_card_poker_efg(), PROBS_1B_doub, PROBS_2B_doub, False,
                  lambda profile, strat_pair: profile.strategy_value_deriv(strategy=strat_pair[0],
                  other=strat_pair[1]),
                  lambda game: list(product(game.strategies, game.strategies)),
                  id="strat_value_deriv_poker_doub"),
     pytest.param(games.create_myerson_2_card_poker_efg(), PROBS_1B_rat, PROBS_2B_rat, True,
                  lambda profile, strat_pair: profile.strategy_value_deriv(strategy=strat_pair[0],
                  other=strat_pair[1]),
                  lambda game: list(product(game.strategies, game.strategies)),
                  id="strat_value_deriv_poker_rat"),
     #################################################################################
     # liap_value (of profile, hence [1] for objects_to_test, any singleton collection would do)
     # 4x4 coordination nfg
     pytest.param(games.create_coord_4x4_nfg(), PROBS_1A_doub, PROBS_2A_doub, False,
                  lambda profile, y: profile.liap_value(), lambda x: [1],
                  id="liap_value_coord_doub"),
     pytest.param(games.create_coord_4x4_nfg(), PROBS_1A_rat, PROBS_2A_rat, True,
                  lambda profile, y: profile.liap_value(), lambda x: [1],
                  id="liap_value_coord_rat"),
     # 2x2x2 nfg
     pytest.param(games.create_2x2x2_nfg(), PROBS_1B_doub, PROBS_2B_doub, False,
                  lambda profile, y: profile.liap_value(), lambda x: [1],
                  id="liap_value_2x2x2_doub"),
     pytest.param(games.create_2x2x2_nfg(), PROBS_1B_rat, PROBS_2B_rat, True,
                  lambda profile, y: profile.liap_value(), lambda x: [1],
                  id="liap_value_2x2x2_rat"),
     # Myerson 2-card poker efg
     pytest.param(games.create_myerson_2_card_poker_efg(), PROBS_1B_doub, PROBS_2B_doub, False,
                  lambda profile, y: profile.liap_value(), lambda x: [1],
                  id="liap_value_poker_doub"),
     pytest.param(games.create_myerson_2_card_poker_efg(), PROBS_1B_rat, PROBS_2B_rat, True,
                  lambda profile, y: profile.liap_value(), lambda x: [1],
                  id="liap_value_poker_rat"),
     ]
)
def test_profile_order_consistency(game: gbt.Game,
                                   action_probs1: tuple,
                                   action_probs2: tuple, rational_flag: bool,
                                   func_to_test: typing.Callable,
                                   objects_to_test: typing.Callable):
    _get_and_check_answers(game, action_probs1, action_probs2, rational_flag, func_to_test,
                           objects_to_test(game))
