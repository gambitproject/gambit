import typing
from itertools import product

import pygambit as gbt
import pytest

from . import games

TOL = 1e-13  # tolerance for floating point assertions
ZERO = gbt.Rational(0)  # tolerance for rational assertions


def _set_action_probs(profile: gbt.MixedBehaviorProfile, probs: list, rational_flag: bool):
    """Set the action probabilities in a behavior profile called ```profile``` according to a
    list with probabilities in the order of ```profile.game.actions```
    """
    for i, p in enumerate(probs):
        # assumes rationals given as strings
        profile[profile.game.actions[i]] = gbt.Rational(p) if rational_flag else p


@pytest.mark.parametrize(
    "game,player_idx,payoff,rational_flag",
    [(games.create_mixed_behav_game(), 0, 3.0, False),
     (games.create_mixed_behav_game(), 1, 3.0, False),
     (games.create_mixed_behav_game(), 2, 3.25, False),
     (games.create_mixed_behav_game(), 0, "3", True),
     (games.create_mixed_behav_game(), 1, "3", True),
     (games.create_mixed_behav_game(), 2, "13/4", True),
     (games.create_myerson_2_card_poker(), 0, -1.25, False),
     (games.create_myerson_2_card_poker(), 1, 1.25, True),
     (games.create_myerson_2_card_poker(), 0, "-5/4", True),
     (games.create_myerson_2_card_poker(), 1, "5/4", True)
     ]
)
def test_payoff_reference(game: gbt.Game, player_idx: int, payoff: typing.Union[str, float],
                          rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    payoff = gbt.Rational(payoff) if rational_flag else payoff
    assert profile.payoff(game.players[player_idx]) == payoff


@pytest.mark.parametrize(
    "game,label,payoff,rational_flag",
    [(games.create_mixed_behav_game(), "Player 1", 3.0, False),
     (games.create_mixed_behav_game(), "Player 2", 3.0, False),
     (games.create_mixed_behav_game(), "Player 3", 3.25, False),
     (games.create_mixed_behav_game(), "Player 1", "3", True),
     (games.create_mixed_behav_game(), "Player 2", "3", True),
     (games.create_mixed_behav_game(), "Player 3", "13/4", True),
     (games.create_myerson_2_card_poker(), "Player 1", -1.25, False),
     (games.create_myerson_2_card_poker(), "Player 2", 1.25, False),
     (games.create_myerson_2_card_poker(), "Player 1", "-5/4", True),
     (games.create_myerson_2_card_poker(), "Player 2", "5/4", True),
     ]
)
def test_payoff_by_label_reference(game: gbt.Game, label: str, payoff: typing.Union[str, float],
                                   rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    payoff = gbt.Rational(payoff) if rational_flag else payoff
    assert profile.payoff(label) == payoff


@pytest.mark.parametrize(
    "game,rational_flag",
    [(games.create_mixed_behav_game(), False),
     (games.create_mixed_behav_game(), True),
     (games.create_myerson_2_card_poker(), False),
     (games.create_myerson_2_card_poker(), True),
     ]
)
def test_is_defined_at(game: gbt.Game, rational_flag: bool):
    """Test to check if infoset are all defined"""
    profile = game.mixed_behavior_profile(rational=rational_flag)
    for infoset in game.infosets:
        assert profile.is_defined_at(infoset)


@pytest.mark.parametrize(
    "game,label,rational_flag",
    [(games.create_mixed_behav_game(), "Infoset 1:1", False),
     (games.create_mixed_behav_game(), "Infoset 2:1", False),
     (games.create_mixed_behav_game(), "Infoset 3:1", False),
     (games.create_mixed_behav_game(), "Infoset 1:1", True),
     (games.create_mixed_behav_game(), "Infoset 2:1", True),
     (games.create_mixed_behav_game(), "Infoset 3:1", True),
     (games.create_myerson_2_card_poker(), "(1,1)", False),
     (games.create_myerson_2_card_poker(), "(1,2)", False),
     (games.create_myerson_2_card_poker(), "(2,1)", False),
     (games.create_myerson_2_card_poker(), "(1,1)", True),
     (games.create_myerson_2_card_poker(), "(1,2)", True),
     (games.create_myerson_2_card_poker(), "(2,1)", True),
     ]
)
def test_is_defined_at_by_label(game: gbt.Game, label: str, rational_flag: bool):
    """Test to check if an infoset is defined by string labels"""
    profile = game.mixed_behavior_profile(rational=rational_flag)
    assert profile.is_defined_at(label)


@pytest.mark.parametrize(
    "game,player_idx,infoset_idx,action_idx,prob,rational_flag",
    [(games.create_mixed_behav_game(), 0, 0, 0, 0.5, False),
     (games.create_mixed_behav_game(), 0, 0, 1, 0.5, False),
     (games.create_mixed_behav_game(), 1, 0, 0, 0.5, False),
     (games.create_mixed_behav_game(), 1, 0, 1, 0.5, False),
     (games.create_mixed_behav_game(), 2, 0, 0, 0.5, False),
     (games.create_mixed_behav_game(), 2, 0, 1, 0.5, False),
     (games.create_mixed_behav_game(), 0, 0, 0, "1/2", True),
     (games.create_mixed_behav_game(), 0, 0, 1, "1/2", True),
     (games.create_mixed_behav_game(), 1, 0, 0, "1/2", True),
     (games.create_mixed_behav_game(), 1, 0, 1, "1/2", True),
     (games.create_mixed_behav_game(), 2, 0, 0, "1/2", True),
     (games.create_mixed_behav_game(), 2, 0, 1, "1/2", True),
     (games.create_myerson_2_card_poker(), 0, 0, 0, 0.5, False),
     (games.create_myerson_2_card_poker(), 0, 0, 1, 0.5, False),
     (games.create_myerson_2_card_poker(), 0, 1, 0, 0.5, False),
     (games.create_myerson_2_card_poker(), 0, 1, 1, 0.5, False),
     (games.create_myerson_2_card_poker(), 1, 0, 0, 0.5, False),
     (games.create_myerson_2_card_poker(), 1, 0, 1, 0.5, False),
     (games.create_myerson_2_card_poker(), 0, 0, 0, "1/2", True),
     (games.create_myerson_2_card_poker(), 0, 0, 1, "1/2", True),
     (games.create_myerson_2_card_poker(), 0, 1, 0, "1/2", True),
     (games.create_myerson_2_card_poker(), 0, 1, 1, "1/2", True),
     (games.create_myerson_2_card_poker(), 1, 0, 0, "1/2", True),
     (games.create_myerson_2_card_poker(), 1, 0, 1, "1/2", True),
     ]
)
def test_get_probabilities_action_reference(game: gbt.Game, player_idx: int, infoset_idx: int,
                                            action_idx: int, prob: typing.Union[str, float],
                                            rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    action = game.players[player_idx].infosets[infoset_idx].actions[action_idx]
    prob = gbt.Rational(prob) if rational_flag else prob
    assert profile[action] == prob


@pytest.mark.parametrize(
    "game,label,prob,rational_flag",
    [(games.create_mixed_behav_game(), "U1", 0.5, False),
     (games.create_mixed_behav_game(), "D1", 0.5, False),
     (games.create_mixed_behav_game(), "U2", 0.5, False),
     (games.create_mixed_behav_game(), "D2", 0.5, False),
     (games.create_mixed_behav_game(), "U3", 0.5, False),
     (games.create_mixed_behav_game(), "D3", 0.5, False),
     (games.create_mixed_behav_game(), "U1", "1/2", True),
     (games.create_mixed_behav_game(), "D1", "1/2", True),
     (games.create_mixed_behav_game(), "U2", "1/2", True),
     (games.create_mixed_behav_game(), "D2", "1/2", True),
     (games.create_mixed_behav_game(), "U3", "1/2", True),
     (games.create_mixed_behav_game(), "D3", "1/2", True),
     (games.create_myerson_2_card_poker(), "MEET", 0.5, False),
     (games.create_myerson_2_card_poker(), "PASS", 0.5, False),
     (games.create_myerson_2_card_poker(), "MEET", "1/2", True),
     (games.create_myerson_2_card_poker(), "PASS", "1/2", True),
     ]
)
def test_get_action_probabilities_by_action_label_reference(game: gbt.Game, label: str,
                                                            prob: typing.Union[str, float],
                                                            rational_flag: bool):
    """Here we only use the action label, which are all valid"""
    profile = game.mixed_behavior_profile(rational=rational_flag)
    prob = gbt.Rational(prob) if rational_flag else prob
    assert profile[label] == prob


@pytest.mark.parametrize(
    "game,action_label,rational_flag,error",
    [(games.create_mixed_behav_game(), "U4", True, KeyError),
     (games.create_mixed_behav_game(), "U4", False, KeyError),
     (games.create_myerson_2_card_poker(), "RAISE", True, ValueError),
     (games.create_myerson_2_card_poker(), "RAISE", False, ValueError),
     (games.create_myerson_2_card_poker(), "FOLD", True, ValueError),
     (games.create_myerson_2_card_poker(), "FOLD", False, ValueError),
     (games.create_myerson_2_card_poker(), "RAISEFOLD", True, KeyError),
     (games.create_myerson_2_card_poker(), "RAISEFOLD", False, KeyError),
     (games.create_myerson_2_card_poker(), "MISSING", True, KeyError),
     (games.create_myerson_2_card_poker(), "MISSING", False, KeyError),
     ]
)
def test_profile_indexing_by_invalid_action_label(game: gbt.Game, action_label: str,
                                                  rational_flag: bool,
                                                  error: typing.Union[ValueError, KeyError]):
    """Test that we get a KeyError for a missing label, and a ValueError for an ambigiuous label
    """
    with pytest.raises(error):
        game.mixed_behavior_profile(rational=rational_flag)[action_label]


@pytest.mark.parametrize(
    "game,infoset_label,action_label,prob,rational_flag",
    [(games.create_mixed_behav_game(), "Infoset 1:1", "U1", 0.5, False),
     (games.create_mixed_behav_game(), "Infoset 1:1", "D1", 0.5, False),
     (games.create_mixed_behav_game(), "Infoset 1:1", "U1", "1/2", True),
     (games.create_mixed_behav_game(), "Infoset 1:1", "D1", "1/2", True),
     (games.create_myerson_2_card_poker(), "(1,1)", "RAISE", 0.5, False),
     (games.create_myerson_2_card_poker(), "(1,1)", "FOLD", 0.5, False),
     (games.create_myerson_2_card_poker(), "(1,2)", "RAISE", 0.5, False),
     (games.create_myerson_2_card_poker(), "(1,2)", "FOLD", 0.5, False),
     (games.create_myerson_2_card_poker(), "(2,1)", "MEET", 0.5, False),
     (games.create_myerson_2_card_poker(), "(2,1)", "PASS", 0.5, False),
     (games.create_myerson_2_card_poker(), "(2,1)", "MEET", "1/2", True),
     (games.create_myerson_2_card_poker(), "(2,1)", "PASS", "1/2", True),
     ]
)
def test_get_action_probs_by_infoset_and_action_labels_reference(game: gbt.Game,
                                                                 infoset_label: str,
                                                                 action_label: str,
                                                                 prob: typing.Union[str, float],
                                                                 rational_flag: bool):
    """Here we use the infoset label and action label, with some exampels where the action label
    alone throws a ValueError (checked in a separate test)
    """
    profile = game.mixed_behavior_profile(rational=rational_flag)
    prob = gbt.Rational(prob) if rational_flag else prob
    assert profile[infoset_label][action_label] == prob


@pytest.mark.parametrize(
    "game,infoset_label,action_label,rational_flag",
    [(games.create_mixed_behav_game(), "1:1", "U2", True),  # U2 is at a different iset
     (games.create_mixed_behav_game(), "1:1", "U2", False),
     (games.create_mixed_behav_game(), "1:1", "U4", True),  # U4 isn't in the game
     (games.create_mixed_behav_game(), "1:1", "U4", False),
     (games.create_myerson_2_card_poker(), "(1,1)", "MEET", True),  # MEET at different iset
     (games.create_myerson_2_card_poker(), "(1,1)", "MEET", False),
     ]
)
def test_profile_indexing_by_invalid_infoset_or_action_label(game: gbt.Game, infoset_label: str,
                                                             action_label: str,
                                                             rational_flag: bool):
    with pytest.raises(KeyError):
        game.mixed_behavior_profile(rational=rational_flag)[infoset_label][action_label]


@pytest.mark.parametrize(
    "game,player_idx,infoset_idx,probs,rational_flag",
    [(games.create_mixed_behav_game(), 0, 0, [0.5, 0.5], False),
     (games.create_mixed_behav_game(), 1, 0, [0.5, 0.5], False),
     (games.create_mixed_behav_game(), 2, 0, [0.5, 0.5], False),
     (games.create_mixed_behav_game(), 0, 0, ["1/2", "1/2"], True),
     (games.create_mixed_behav_game(), 1, 0, ["1/2", "1/2"], True),
     (games.create_mixed_behav_game(), 2, 0, ["1/2", "1/2"], True),
     (games.create_myerson_2_card_poker(), 0, 0, [0.5, 0.5], False),
     (games.create_myerson_2_card_poker(), 0, 1, [0.5, 0.5], False),
     (games.create_myerson_2_card_poker(), 1, 0, [0.5, 0.5], False),
     (games.create_myerson_2_card_poker(), 0, 0, ["1/2", "1/2"], True),
     (games.create_myerson_2_card_poker(), 0, 1, ["1/2", "1/2"], True),
     (games.create_myerson_2_card_poker(), 1, 0, ["1/2", "1/2"], True),
     ]
)
def test_get_probabilities_infoset_reference(game: gbt.Game, player_idx: int, infoset_idx: int,
                                             probs: list, rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    infoset = game.players[player_idx].infosets[infoset_idx]
    probs = [gbt.Rational(prob) for prob in probs] if rational_flag else probs
    assert profile[infoset] == probs


@pytest.mark.parametrize(
    "game,player_idx,infoset_label,probs,rational_flag",
    [(games.create_mixed_behav_game(), 0, "Infoset 1:1", [0.5, 0.5], False),
     (games.create_mixed_behav_game(), 1, "Infoset 2:1", [0.5, 0.5], False),
     (games.create_mixed_behav_game(), 2, "Infoset 3:1", [0.5, 0.5], False),
     (games.create_mixed_behav_game(), 0, "Infoset 1:1", ["1/2", "1/2"], True),
     (games.create_mixed_behav_game(), 1, "Infoset 2:1", ["1/2", "1/2"], True),
     (games.create_mixed_behav_game(), 2, "Infoset 3:1", ["1/2", "1/2"], True),
     (games.create_myerson_2_card_poker(), 0, "(1,1)", [0.5, 0.5], False),
     (games.create_myerson_2_card_poker(), 0, "(1,2)", [0.5, 0.5], False),
     (games.create_myerson_2_card_poker(), 1, "(2,1)", [0.5, 0.5], False),
     (games.create_myerson_2_card_poker(), 0, "(1,1)", ["1/2", "1/2"], True),
     (games.create_myerson_2_card_poker(), 0, "(1,2)", ["1/2", "1/2"], True),
     (games.create_myerson_2_card_poker(), 1, "(2,1)", ["1/2", "1/2"], True),
     ]
)
def test_get_probabilities_infoset_by_label_reference(game: gbt.Game, player_idx: int,
                                                      infoset_label: str, probs: list,
                                                      rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    player = game.players[player_idx]
    probs = [gbt.Rational(prob) for prob in probs] if rational_flag else probs
    assert profile[player][infoset_label] == probs
    assert profile[infoset_label] == probs


@pytest.mark.parametrize(
    "game,player_label,infoset_label,rational_flag",
    [(games.create_mixed_behav_game(), "Player 1", "1:1", True),  # correct: "Infoset 1:1"
     (games.create_mixed_behav_game(), "Player 1", "1:1", False),
     (games.create_myerson_2_card_poker(), "Player 1", "(2,1)", True),  # wrong player
     (games.create_myerson_2_card_poker(), "Player 1", "(2,1)", False),
     ]
)
def test_profile_indexing_by_player_and_invalid_infoset_label(game: gbt.Game,
                                                              player_label: str,
                                                              infoset_label: str,
                                                              rational_flag: bool):
    """Test that we get a KeyError and that "player" appears in the error message"""
    with pytest.raises(KeyError, match="player"):
        game.mixed_behavior_profile(rational=rational_flag)[player_label][infoset_label]


@pytest.mark.parametrize(
    "game,player_label,action_label,rational_flag",
    [(games.create_mixed_behav_game(), "Player 1", "U2", True),
     (games.create_mixed_behav_game(), "Player 1", "U2", False),
     (games.create_myerson_2_card_poker(), "Player 1", "MEET", True),
     (games.create_myerson_2_card_poker(), "Player 1", "MEET", False),
     ]
)
def test_profile_indexing_by_player_and_invalid_action_label(game: gbt.Game,
                                                              player_label: str,
                                                              action_label: str,
                                                              rational_flag: bool):
    """Test that we get a KeyError and that "player" appears in the error message"""
    with pytest.raises(KeyError, match="player"):
        game.mixed_behavior_profile(rational=rational_flag)[player_label][action_label]


@pytest.mark.parametrize(
    "game,player_idx,behav_data,rational_flag",
    [(games.create_mixed_behav_game(), 0, [[0.5, 0.5]], False),
     (games.create_mixed_behav_game(), 1, [[0.5, 0.5]], False),
     (games.create_mixed_behav_game(), 2, [[0.5, 0.5]], False),
     (games.create_mixed_behav_game(), 0, [["1/2", "1/2"]], True),
     (games.create_mixed_behav_game(), 1, [["1/2", "1/2"]], True),
     (games.create_mixed_behav_game(), 2, [["1/2", "1/2"]], True),
     (games.create_myerson_2_card_poker(), 0, [[0.5, 0.5], [0.5, 0.5]], False),
     (games.create_myerson_2_card_poker(), 1, [[0.5, 0.5]], False),
     (games.create_myerson_2_card_poker(), 0, [["1/2", "1/2"], ["1/2", "1/2"]], True),
     (games.create_myerson_2_card_poker(), 1, [["1/2", "1/2"]], True),
     ]
)
def test_get_probabilities_player_reference(game: gbt.Game, player_idx: int, behav_data: list,
                                            rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    player = game.players[player_idx]
    if rational_flag:
        behav_data = [[gbt.Rational(prob) for prob in probs] for probs in behav_data]
    assert profile[player] == behav_data


@pytest.mark.parametrize(
    "game,player_label,behav_data,rational_flag",
    [(games.create_mixed_behav_game(), "Player 1", [[0.5, 0.5]], False),
     (games.create_mixed_behav_game(), "Player 2", [[0.5, 0.5]], False),
     (games.create_mixed_behav_game(), "Player 3", [[0.5, 0.5]], False),
     (games.create_mixed_behav_game(), "Player 1", [["1/2", "1/2"]], True),
     (games.create_mixed_behav_game(), "Player 2", [["1/2", "1/2"]], True),
     (games.create_mixed_behav_game(), "Player 3", [["1/2", "1/2"]], True),
     (games.create_myerson_2_card_poker(), "Player 1", [[0.5, 0.5], [0.5, 0.5]], False),
     (games.create_myerson_2_card_poker(), "Player 2", [[0.5, 0.5]], False),
     (games.create_myerson_2_card_poker(), "Player 1", [["1/2", "1/2"], ["1/2", "1/2"]],
      True),
     (games.create_myerson_2_card_poker(), "Player 2", [["1/2", "1/2"]], True),
     ]
)
def test_get_probabilities_player_by_label_reference(game: gbt.Game, player_label: str,
                                                     behav_data: list, rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    if rational_flag:
        behav_data = [[gbt.Rational(prob) for prob in probs] for probs in behav_data]
    assert profile[player_label] == behav_data


@pytest.mark.parametrize(
    "game,action_idx,prob,rational_flag",
    [(games.create_mixed_behav_game(), 0, 0.72, False),
     (games.create_mixed_behav_game(), 1, 0.28, False),
     (games.create_mixed_behav_game(), 2, 0.42, False),
     (games.create_mixed_behav_game(), 3, 0.58, False),
     (games.create_mixed_behav_game(), 4, 0.02, False),
     (games.create_mixed_behav_game(), 5, 0.98, False),
     (games.create_mixed_behav_game(), 0, "2/9", True),
     (games.create_mixed_behav_game(), 1, "7/9", True),
     (games.create_mixed_behav_game(), 2, "4/13", True),
     (games.create_mixed_behav_game(), 3, "9/13", True),
     (games.create_mixed_behav_game(), 4, "1/98", True),
     (games.create_mixed_behav_game(), 5, "97/98", True),
     (games.create_myerson_2_card_poker(), 0, 0.1, False),
     (games.create_myerson_2_card_poker(), 1, 0.2, False),
     (games.create_myerson_2_card_poker(), 2, 0.3, False),
     (games.create_myerson_2_card_poker(), 3, 0.4, False),
     (games.create_myerson_2_card_poker(), 4, 0.5, False),
     (games.create_myerson_2_card_poker(), 5, 0.6, False),
     (games.create_myerson_2_card_poker(), 0, "1/10", True),
     (games.create_myerson_2_card_poker(), 1, "2/10", True),
     (games.create_myerson_2_card_poker(), 2, "3/10", True),
     (games.create_myerson_2_card_poker(), 3, "4/10", True),
     (games.create_myerson_2_card_poker(), 4, "5/10", True),
     (games.create_myerson_2_card_poker(), 5, "6/10", True),
     ]
)
def test_set_probabilities_action(game: gbt.Game, action_idx: int, prob: typing.Union[str, float],
                                  rational_flag: bool):
    """Test to set probabilities of actions by action index"""
    profile = game.mixed_behavior_profile(rational=rational_flag)
    prob = gbt.Rational(prob) if rational_flag else prob
    action = game.actions[action_idx]
    profile[action] = prob
    assert profile[action] == prob


@pytest.mark.parametrize(
    "game,label,prob,rational_flag",
    [(games.create_mixed_behav_game(), "U1", 0.72, False),
     (games.create_mixed_behav_game(), "D1", 0.28, False),
     (games.create_mixed_behav_game(), "U2", 0.42, False),
     (games.create_mixed_behav_game(), "D2", 0.58, False),
     (games.create_mixed_behav_game(), "U3", 0.02, False),
     (games.create_mixed_behav_game(), "D3", 0.98, False),
     (games.create_mixed_behav_game(), "U1", "2/9", True),
     (games.create_mixed_behav_game(), "D1", "7/9", True),
     (games.create_mixed_behav_game(), "U2", "4/13", True),
     (games.create_mixed_behav_game(), "D2", "9/13", True),
     (games.create_mixed_behav_game(), "U3", "1/98", True),
     (games.create_mixed_behav_game(), "D3", "97/98", True),
     (games.create_myerson_2_card_poker(), "MEET", 0.3, False),
     (games.create_myerson_2_card_poker(), "PASS", 0.4, False),
     (games.create_myerson_2_card_poker(), "MEET", "3/10", True),
     (games.create_myerson_2_card_poker(), "PASS", "4/10", True),
     ]
)
def test_set_probabilities_action_by_label(game: gbt.Game, label: str,
                                           prob: typing.Union[str, float], rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    prob = gbt.Rational(prob) if rational_flag else prob
    profile[label] = prob
    assert profile[label] == prob


@pytest.mark.parametrize(
    "game,player_idx,infoset_idx,probs,rational_flag",
    [(games.create_mixed_behav_game(), 0, 0, [0.72, 0.28], False),
     (games.create_mixed_behav_game(), 1, 0, [0.42, 0.58], False),
     (games.create_mixed_behav_game(), 2, 0, [0.02, 0.98], False),
     (games.create_mixed_behav_game(), 0, 0, ["7/9", "2/9"], True),
     (games.create_mixed_behav_game(), 1, 0, ["4/13", "9/13"], True),
     (games.create_mixed_behav_game(), 2, 0, ["1/98", "97/98"], True),
     (games.create_myerson_2_card_poker(), 0, 0, [0.1, 0.9], False),
     (games.create_myerson_2_card_poker(), 0, 1, [0.2, 0.8], False),
     (games.create_myerson_2_card_poker(), 1, 0, [0.3, 0.7], False),
     (games.create_myerson_2_card_poker(), 0, 0, ["1/10", "9/10"], True),
     (games.create_myerson_2_card_poker(), 0, 1, ["2/10", "8/10"], True),
     (games.create_myerson_2_card_poker(), 1, 0, ["3/10", "7/10"], True),
     ]
)
def test_set_probabilities_infoset(game: gbt.Game, player_idx: int, infoset_idx: int, probs: list,
                                   rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    if rational_flag:
        probs = [gbt.Rational(p) for p in probs]
    infoset = game.players[player_idx].infosets[infoset_idx]
    profile[infoset] = probs
    assert profile[infoset] == probs


@pytest.mark.parametrize(
    "game,infoset_label,probs,rational_flag",
    [(games.create_mixed_behav_game(), "Infoset 1:1", [0.72, 0.28], False),
     (games.create_mixed_behav_game(), "Infoset 2:1", [0.42, 0.58], False),
     (games.create_mixed_behav_game(), "Infoset 3:1", [0.02, 0.98], False),
     (games.create_mixed_behav_game(), "Infoset 1:1", ["7/9", "2/9"], True),
     (games.create_mixed_behav_game(), "Infoset 2:1", ["4/13", "9/13"], True),
     (games.create_mixed_behav_game(), "Infoset 3:1", ["1/98", "97/98"], True),
     (games.create_myerson_2_card_poker(), "(1,1)", [0.1, 0.9], False),
     (games.create_myerson_2_card_poker(), "(1,2)", [0.2, 0.8], False),
     (games.create_myerson_2_card_poker(), "(2,1)", [0.3, 0.7], False),
     (games.create_myerson_2_card_poker(), "(1,1)", ["1/10", "9/10"], True),
     (games.create_myerson_2_card_poker(), "(1,2)", ["2/10", "8/10"], True),
     (games.create_myerson_2_card_poker(), "(2,1)", ["3/10", "7/10"], True),
     ]
)
def test_set_probabilities_infoset_by_label(game: gbt.Game, infoset_label: str, probs: list,
                                            rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    if rational_flag:
        probs = [gbt.Rational(p) for p in probs]
    profile[infoset_label] = probs
    assert profile[infoset_label] == probs


@pytest.mark.parametrize(
    "game,player_idx,behav_data,rational_flag",
    [(games.create_mixed_behav_game(), 0, [[0.72, 0.28]], False),
     (games.create_mixed_behav_game(), 1, [[0.42, 0.58]], False),
     (games.create_mixed_behav_game(), 2, [[0.02, 0.98]], False),
     (games.create_mixed_behav_game(), 0, [["7/9", "2/9"]], True),
     (games.create_mixed_behav_game(), 1, [["4/13", "9/13"]], True),
     (games.create_mixed_behav_game(), 2, [["1/98", "97/98"]], True),
     (games.create_myerson_2_card_poker(), 0, [[0.1, 0.9], [0.5, 0.5]], False),
     (games.create_myerson_2_card_poker(), 1, [[0.6, 0.4]], False),
     (games.create_myerson_2_card_poker(), 0, [["1/3", "2/3"], ["1/2", "1/2"]], True),
     (games.create_myerson_2_card_poker(), 1, [["2/3", "1/3"]], True),
     ]
)
def test_set_probabilities_player(game: gbt.Game, player_idx: int, behav_data: list,
                                  rational_flag: bool):
    player = game.players[player_idx]
    profile = game.mixed_behavior_profile(rational=rational_flag)
    if rational_flag:
        behav_data = [[gbt.Rational(prob) for prob in probs] for probs in behav_data]
    profile[player] = behav_data
    assert profile[player] == behav_data


@pytest.mark.parametrize(
    "game,player_label,behav_data,rational_flag",
    [(games.create_mixed_behav_game(), "Player 1", [[0.72, 0.28]], False),
     (games.create_mixed_behav_game(), "Player 2", [[0.42, 0.58]], False),
     (games.create_mixed_behav_game(), "Player 3", [[0.02, 0.98]], False),
     (games.create_mixed_behav_game(), "Player 1", [["7/9", "2/9"]], True),
     (games.create_mixed_behav_game(), "Player 2", [["4/13", "9/13"]], True),
     (games.create_mixed_behav_game(), "Player 3", [["1/98", "97/98"]], True),
     (games.create_myerson_2_card_poker(), "Player 1", [[0.1, 0.9], [0.5, 0.5]], False),
     (games.create_myerson_2_card_poker(), "Player 2", [[0.6, 0.4]], False),
     (games.create_myerson_2_card_poker(), "Player 1", [["1/3", "2/3"], ["1/2", "1/2"]],
      True),
     (games.create_myerson_2_card_poker(), "Player 2", [["2/3", "1/3"]], True),
     ]
)
def test_set_probabilities_player_by_label(game: gbt.Game, player_label: str, behav_data: list,
                                           rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    if rational_flag:
        behav_data = [[gbt.Rational(prob) for prob in probs] for probs in behav_data]
    profile[player_label] = behav_data
    assert profile[player_label] == behav_data


@pytest.mark.parametrize(
    "game,node_idx,realiz_prob,rational_flag",
    [(games.create_mixed_behav_game(), 0, "1", True),
     (games.create_mixed_behav_game(), 1, "1/2", True),
     (games.create_mixed_behav_game(), 2, "1/4", True),
     (games.create_mixed_behav_game(), 3, "1/8", True),
     (games.create_mixed_behav_game(), 4, "1/8", True),
     (games.create_mixed_behav_game(), 5, "1/4", True),
     (games.create_mixed_behav_game(), 6, "1/8", True),
     (games.create_mixed_behav_game(), 7, "1/8", True),
     (games.create_mixed_behav_game(), 8, "1/2", True),
     (games.create_mixed_behav_game(), 9, "1/4", True),
     (games.create_mixed_behav_game(), 10, "1/8", True),
     (games.create_mixed_behav_game(), 11, "1/8", True),
     (games.create_mixed_behav_game(), 12, "1/4", True),
     (games.create_mixed_behav_game(), 13, "1/8", True),
     (games.create_mixed_behav_game(), 14, "1/8", True),
     (games.create_mixed_behav_game(), 0, 1.0, False),
     (games.create_mixed_behav_game(), 1, 0.5, False),
     (games.create_mixed_behav_game(), 2, 0.25, False),
     (games.create_mixed_behav_game(), 3, 0.125, False),
     (games.create_mixed_behav_game(), 4, 0.125, False),
     (games.create_mixed_behav_game(), 5, 0.25, False),
     (games.create_mixed_behav_game(), 6, 0.125, False),
     (games.create_mixed_behav_game(), 7, 0.125, False),
     (games.create_mixed_behav_game(), 8, 0.5, False),
     (games.create_mixed_behav_game(), 9, 0.25, False),
     (games.create_mixed_behav_game(), 10, 0.125, False),
     (games.create_mixed_behav_game(), 11, 0.125, False),
     (games.create_mixed_behav_game(), 12, 0.25, False),
     (games.create_mixed_behav_game(), 13, 0.125, False),
     (games.create_mixed_behav_game(), 14, 0.125, False),
     (games.create_myerson_2_card_poker(), 0, "1", True),
     (games.create_myerson_2_card_poker(), 1, "1/2", True),
     (games.create_myerson_2_card_poker(), 2, "1/4", True),
     (games.create_myerson_2_card_poker(), 3, "1/8", True),
     (games.create_myerson_2_card_poker(), 4, "1/8", True),
     (games.create_myerson_2_card_poker(), 5, "1/4", True),
     (games.create_myerson_2_card_poker(), 6, "1/2", True),
     (games.create_myerson_2_card_poker(), 7, "1/4", True),
     (games.create_myerson_2_card_poker(), 8, "1/8", True),
     (games.create_myerson_2_card_poker(), 9, "1/8", True),
     (games.create_myerson_2_card_poker(), 10, "1/4", True),
     (games.create_myerson_2_card_poker(), 0, 1.0, False),
     (games.create_myerson_2_card_poker(), 1, 0.5, False),
     (games.create_myerson_2_card_poker(), 2, 0.25, False),
     (games.create_myerson_2_card_poker(), 3, 0.125, False),
     (games.create_myerson_2_card_poker(), 4, 0.125, False),
     (games.create_myerson_2_card_poker(), 5, 0.25, False),
     (games.create_myerson_2_card_poker(), 6, 0.5, False),
     (games.create_myerson_2_card_poker(), 7, 0.25, False),
     (games.create_myerson_2_card_poker(), 8, 0.125, False),
     (games.create_myerson_2_card_poker(), 9, 0.125, False),
     (games.create_myerson_2_card_poker(), 10, 0.25, False)]
)
def test_realiz_prob_nodes_reference(game: gbt.Game, node_idx: int,
                                     realiz_prob: typing.Union[str, float], rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    realiz_prob = (gbt.Rational(realiz_prob) if rational_flag else realiz_prob)
    node = game.nodes()[node_idx]
    assert profile.realiz_prob(node) == realiz_prob


@pytest.mark.parametrize(
    "game,player_idx,infoset_idx,prob,rational_flag",
    [(games.create_mixed_behav_game(), 0, 0, 1.0, False),
     (games.create_mixed_behav_game(), 1, 0, 1.0, False),
     (games.create_mixed_behav_game(), 2, 0, 1.0, False),
     (games.create_mixed_behav_game(), 0, 0, "1", True),
     (games.create_mixed_behav_game(), 1, 0, "1", True),
     (games.create_mixed_behav_game(), 2, 0, "1", True),
     (games.create_myerson_2_card_poker(), 0, 0, 0.5, False),
     (games.create_myerson_2_card_poker(), 0, 1, 0.5, False),
     (games.create_myerson_2_card_poker(), 1, 0, 0.5, False),
     (games.create_myerson_2_card_poker(), 0, 0, "1/2", True),
     (games.create_myerson_2_card_poker(), 0, 1, "1/2", True),
     (games.create_myerson_2_card_poker(), 1, 0, "1/2", True),
     ]
)
def test_infoset_prob_reference(game: gbt.Game, player_idx: int, infoset_idx: int,
                                prob: typing.Union[str, float], rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    ip = profile.infoset_prob(game.players[player_idx].infosets[infoset_idx])
    assert ip == (gbt.Rational(prob) if rational_flag else prob)


@pytest.mark.parametrize(
    "game,label,prob,rational_flag,",
    [(games.create_mixed_behav_game(), "Infoset 1:1", 1.0, False),
     (games.create_mixed_behav_game(), "Infoset 2:1", 1.0, False),
     (games.create_mixed_behav_game(), "Infoset 3:1", 1.0, False),
     (games.create_mixed_behav_game(), "Infoset 1:1", "1", True),
     (games.create_mixed_behav_game(), "Infoset 2:1", "1", True),
     (games.create_mixed_behav_game(), "Infoset 3:1", "1", True),
     (games.create_myerson_2_card_poker(), "(1,1)", 0.5, False),
     (games.create_myerson_2_card_poker(), "(1,2)", 0.5, False),
     (games.create_myerson_2_card_poker(), "(2,1)", 0.5, False),
     (games.create_myerson_2_card_poker(), "(1,1)", "1/2", True),
     (games.create_myerson_2_card_poker(), "(1,2)", "1/2", True),
     (games.create_myerson_2_card_poker(), "(2,1)", "1/2", True),
     ]
)
def test_infoset_prob_by_label_reference(game: gbt.Game, label: str,
                                         prob: typing.Union[str, float], rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    assert profile.infoset_prob(label) == (gbt.Rational(prob) if rational_flag else prob)


@pytest.mark.parametrize(
    "game,player_idx,infoset_idx,payoff,rational_flag",
    [(games.create_mixed_behav_game(), 0, 0, 3.0, False),
     (games.create_mixed_behav_game(), 1, 0, 3.0, False),
     (games.create_mixed_behav_game(), 2, 0, 3.25, False),
     (games.create_mixed_behav_game(), 0, 0, "3", True),
     (games.create_mixed_behav_game(), 1, 0, "3", True),
     (games.create_mixed_behav_game(), 2, 0, "13/4", True),
     (games.create_myerson_2_card_poker(), 0, 0, -0.75, False),
     (games.create_myerson_2_card_poker(), 0, 1, -1.75, False),
     (games.create_myerson_2_card_poker(), 1, 0, 0.5, False),
     (games.create_myerson_2_card_poker(), 0, 0, "-3/4", True),
     (games.create_myerson_2_card_poker(), 0, 1, "-7/4", True),
     (games.create_myerson_2_card_poker(), 1, 0, "1/2", True),
     ]
)
def test_infoset_payoff_reference(game: gbt.Game, player_idx: int, infoset_idx: int,
                                  payoff: typing.Union[str, float], rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    iv = profile.infoset_value(game.players[player_idx].infosets[infoset_idx])
    assert iv == (gbt.Rational(payoff) if rational_flag else payoff)


@pytest.mark.parametrize(
    "game,label,payoff,rational_flag",
    [(games.create_mixed_behav_game(), "Infoset 1:1", 3.0, False),
     (games.create_mixed_behav_game(), "Infoset 2:1", 3.0, False),
     (games.create_mixed_behav_game(), "Infoset 3:1", 3.25, False),
     (games.create_mixed_behav_game(), "Infoset 1:1", "3", True),
     (games.create_mixed_behav_game(), "Infoset 2:1", "3", True),
     (games.create_mixed_behav_game(), "Infoset 3:1", "13/4", True),
     (games.create_myerson_2_card_poker(), "(1,1)", -0.75, False),
     (games.create_myerson_2_card_poker(), "(1,2)", -1.75, False),
     (games.create_myerson_2_card_poker(), "(2,1)", 0.5, False),
     (games.create_myerson_2_card_poker(), "(1,1)", "-3/4", True),
     (games.create_myerson_2_card_poker(), "(1,2)", "-7/4", True),
     (games.create_myerson_2_card_poker(), "(2,1)", "1/2", True),
     ]
)
def test_infoset_payoff_by_label_reference(game: gbt.Game, label: str,
                                           payoff: typing.Union[str, float], rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    assert profile.infoset_value(label) == (gbt.Rational(payoff) if rational_flag else payoff)


@pytest.mark.parametrize(
    "game,player_idx,infoset_idx,action_idx,payoff,rational_flag",
    [(games.create_mixed_behav_game(), 0, 0, 0, 3.0, False),
     (games.create_mixed_behav_game(), 0, 0, 1, 3.0, False),
     (games.create_mixed_behav_game(), 1, 0, 0, 3.0, False),
     (games.create_mixed_behav_game(), 1, 0, 1, 3.0, False),
     (games.create_mixed_behav_game(), 2, 0, 0, 3.5, False),
     (games.create_mixed_behav_game(), 2, 0, 1, 3.0, False),
     (games.create_mixed_behav_game(), 2, 0, 1, 3.0, False),
     (games.create_mixed_behav_game(), 0, 0, 0, "3/1", True),
     (games.create_mixed_behav_game(), 0, 0, 1, "3/1", True),
     (games.create_mixed_behav_game(), 1, 0, 0, "3/1", True),
     (games.create_mixed_behav_game(), 1, 0, 1, "3/1", True),
     (games.create_mixed_behav_game(), 2, 0, 0, "7/2", True),
     (games.create_mixed_behav_game(), 2, 0, 1, "3/1", True),
     (games.create_myerson_2_card_poker(), 0, 0, 0, 0.5, False),
     (games.create_myerson_2_card_poker(), 0, 0, 1, -2, False),
     (games.create_myerson_2_card_poker(), 0, 1, 0, -1.5, False),
     (games.create_myerson_2_card_poker(), 0, 1, 1, -2, False),
     (games.create_myerson_2_card_poker(), 1, 0, 0, 1, False),
     (games.create_myerson_2_card_poker(), 1, 0, 1, 0, False),
     (games.create_myerson_2_card_poker(), 0, 0, 0, "1/2", True),
     (games.create_myerson_2_card_poker(), 0, 0, 1, "-2", True),
     (games.create_myerson_2_card_poker(), 0, 1, 0, "-3/2", True),
     (games.create_myerson_2_card_poker(), 0, 1, 1, "-2", True),
     (games.create_myerson_2_card_poker(), 1, 0, 0, "1", True),
     (games.create_myerson_2_card_poker(), 1, 0, 1, "0", True),
     ]
)
def test_action_payoff_reference(game: gbt.Game, player_idx: int, infoset_idx: int,
                                 action_idx: int, payoff: typing.Union[str, float],
                                 rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    av = profile.action_value(game.players[player_idx].infosets[infoset_idx].actions[action_idx])
    assert av == (gbt.Rational(payoff) if rational_flag else payoff)


@pytest.mark.parametrize(
    "game,label,payoff,rational_flag",
    [(games.create_mixed_behav_game(), "U1", 3.0, False),
     (games.create_mixed_behav_game(), "D1", 3.0, False),
     (games.create_mixed_behav_game(), "U2", 3.0, False),
     (games.create_mixed_behav_game(), "D2", 3.0, False),
     (games.create_mixed_behav_game(), "U3", 3.5, False),
     (games.create_mixed_behav_game(), "D3", 3.0, False),
     (games.create_mixed_behav_game(), "U1", "3", True),
     (games.create_mixed_behav_game(), "D1", "3", True),
     (games.create_mixed_behav_game(), "U2", "3", True),
     (games.create_mixed_behav_game(), "D2", "3", True),
     (games.create_mixed_behav_game(), "U3", "7/2", True),
     (games.create_mixed_behav_game(), "D3", "3", True),
     # (games.create_myerson_2_card_poker(), "RAISE", 0.5, False),
     # (games.create_myerson_2_card_poker(), "FOLD", -2, False),
     (games.create_myerson_2_card_poker(), "MEET", 1, False),
     (games.create_myerson_2_card_poker(), "PASS", 0, False),
     # (games.create_myerson_2_card_poker(), "RAISE", "1/2", True), # TODO infoset/action lab
     # (games.create_myerson_2_card_poker(), "FOLD", "-2", True),
     (games.create_myerson_2_card_poker(), "MEET", "1", True),
     (games.create_myerson_2_card_poker(), "PASS", "0", True),
     ]
)
def test_action_value_by_label_reference(game: gbt.Game, label: str,
                                         payoff: typing.Union[str, float], rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    assert profile.action_value(label) == (gbt.Rational(payoff) if rational_flag else payoff)


@pytest.mark.parametrize(
    "game,rational_flag",
    [(games.create_mixed_behav_game(), False),
     (games.create_mixed_behav_game(), True),
     (games.create_myerson_2_card_poker(), False),
     (games.create_myerson_2_card_poker(), True),
     ]
)
def test_regret_consistency(game: gbt.Game, rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    for player in game.players:
        for infoset in player.infosets:
            for action in infoset.actions:
                assert (
                    profile.regret(action) ==
                    max(profile.action_value(a) for a in infoset.actions) -
                    profile.action_value(action)
                )


@pytest.mark.parametrize(
    "game,player_idx,infoset_idx,action_idx,action_probs,rational_flag,tol,value",
    [
     # uniform
     (games.create_mixed_behav_game(), 0, 0, 0, None, False, TOL, 0),
     (games.create_mixed_behav_game(), 0, 0, 1, None, False, TOL, 0),
     (games.create_mixed_behav_game(), 1, 0, 0, None, False, TOL, 0),
     (games.create_mixed_behav_game(), 1, 0, 1, None, False, TOL, 0),
     (games.create_mixed_behav_game(), 2, 0, 0, None, False, TOL, 0),
     (games.create_mixed_behav_game(), 2, 0, 1, None, False, TOL, 0.5),  # 3.5 - 3
     # U1 U2 U3
     (games.create_mixed_behav_game(), 0, 0, 0, [1.0, 0.0, 1.0, 0.0, 1.0, 0.0], False, TOL, 0),
     (games.create_mixed_behav_game(), 0, 0, 0, ["1", "0", "1", "0", "1", "0"], True, ZERO, 0),
     (games.create_mixed_behav_game(), 0, 0, 1, [1.0, 0.0, 1.0, 0.0, 1.0, 0.0], False, TOL, 9),
     (games.create_mixed_behav_game(), 0, 0, 1, ["1", "0", "1", "0", "1", "0"], True, ZERO, 9),
     (games.create_mixed_behav_game(), 1, 0, 0, [1.0, 0.0, 1.0, 0.0, 1.0, 0.0], False, TOL, 0),
     (games.create_mixed_behav_game(), 1, 0, 0, ["1", "0", "1", "0", "1", "0"], True, ZERO, 0),
     (games.create_mixed_behav_game(), 1, 0, 1, [1.0, 0.0, 1.0, 0.0, 1.0, 0.0], False, TOL, 8),
     (games.create_mixed_behav_game(), 1, 0, 1, ["1", "0", "1", "0", "1", "0"], True, ZERO, 8),
     # Mixed Nash equilibrium
     (games.create_mixed_behav_game(), 0, 0, 0, ["2/5", "3/5", "1/2", "1/2", "1/3", "2/3"], True,
      ZERO, 0),
     (games.create_mixed_behav_game(), 0, 0, 1, ["2/5", "3/5", "1/2", "1/2", "1/3", "2/3"], True,
      ZERO, 0),
     (games.create_mixed_behav_game(), 1, 0, 0, ["2/5", "3/5", "1/2", "1/2", "1/3", "2/3"], True,
      ZERO, 0),
     (games.create_mixed_behav_game(), 1, 0, 1, ["2/5", "3/5", "1/2", "1/2", "1/3", "2/3"], True,
      ZERO, 0),
     (games.create_mixed_behav_game(), 2, 0, 0, ["2/5", "3/5", "1/2", "1/2", "1/3", "2/3"], True,
      ZERO, 0),
     (games.create_mixed_behav_game(), 2, 0, 1, ["2/5", "3/5", "1/2", "1/2", "1/3", "2/3"], True,
      ZERO, 0),
     # uniform
     (games.create_myerson_2_card_poker(), 0, 0, 0, None, False, TOL, 0),
     (games.create_myerson_2_card_poker(), 0, 0, 1, None, False, TOL, 2.5),  # 1.5 - (-1)
     (games.create_myerson_2_card_poker(), 0, 1, 0, None, False, TOL, 0),
     (games.create_myerson_2_card_poker(), 0, 1, 1, None, False, TOL, 0.5),  # -0.5 - (-1)
     (games.create_myerson_2_card_poker(), 1, 0, 0, None, False, TOL, 0),
     (games.create_myerson_2_card_poker(), 1, 0, 1, None, False, TOL, 1),  # -0 - (-1)
     # mixed Nash equilibrium
     (games.create_myerson_2_card_poker(), 0, 0, 0, ["1", "0", "1/3", "2/3", "2/3", "1/3"],
      True, ZERO, 0),
     (games.create_myerson_2_card_poker(), 0, 0, 1, ["1", "0", "1/3", "2/3", "2/3", "1/3"],
      True, ZERO, "8/3"),  # (2/3*2 + 1/3*1) - (-1)
     ]
)
def test_regret_reference(game: gbt.Game, player_idx: int, infoset_idx: int, action_idx: int,
                          action_probs: typing.Union[None, list], rational_flag: bool,
                          tol: typing.Union[gbt.Rational, float], value: typing.Union[str, float]):
    action = game.players[player_idx].infosets[infoset_idx].actions[action_idx]
    profile = game.mixed_behavior_profile(rational=rational_flag)
    if action_probs:
        _set_action_probs(profile, action_probs, rational_flag)
    regret = profile.regret(action)
    value = gbt.Rational(value) if rational_flag else value
    assert abs(regret - value) <= tol


@pytest.mark.parametrize(
    "game,rational_flag",
    [(games.create_mixed_behav_game(), False),
     (games.create_mixed_behav_game(), True),
     (games.create_myerson_2_card_poker(), False),
     (games.create_myerson_2_card_poker(), True),
     ]
)
def test_martingale_property_of_node_value(game: gbt.Game, rational_flag: bool):
    """Loops over all nodes and for non-chance, non-terminal nodes, this checks that the node
    value is equal to the expected value of the node values of its children, using the normalized
    realization probabilities of those children
    """
    profile = game.mixed_behavior_profile(rational=rational_flag)
    for node in game.nodes():
        if node.is_terminal or node.player.is_chance:
            continue
        expected_val = 0
        node_prob = profile.realiz_prob(node)
        for child in node.children:
            prob = profile.realiz_prob(child) / node_prob
            expected_val += prob * profile.node_value(node.player, child)
        assert profile.node_value(node.player, node) == expected_val


@pytest.mark.parametrize(
    "game,rational_flag",
    [(games.create_mixed_behav_game(), False),
     (games.create_mixed_behav_game(), True),
     (games.create_myerson_2_card_poker(), False),
     (games.create_myerson_2_card_poker(), True)]
)
def test_node_value_consistency(game: gbt.Game, rational_flag: bool):
    """Test that the profile's node value at the root for each player matches the profile's payoff
    for the respective player"""
    profile = game.mixed_behavior_profile(rational=rational_flag)
    for player in game.players:
        assert profile.node_value(player, game.root) == profile.payoff(player)


@pytest.mark.parametrize(
    "game,action_probs,rational_flag,expected_value",
    [
     # uniform (non-Nash):
     (games.create_mixed_behav_game(), None, True, "1/16"),
     (games.create_mixed_behav_game(), None, False, 0.0625),
     # four pure Nash equilibria:
     (games.create_mixed_behav_game(), [1.0, 0.0, 1.0, 0.0, 1.0, 0.0], False, 0),  # U1 U2 U3
     (games.create_mixed_behav_game(), ["1", "0", "1", "0", "1", "0"], True, 0),
     (games.create_mixed_behav_game(), ["1", "0", "0", "1", "0", "1"], True, 0),  # U1 D2 D3
     (games.create_mixed_behav_game(), [1.0, 0.0, 0.0, 1.0, 0, 1.0], False, 0),
     (games.create_mixed_behav_game(), ["0", "1", "1", "0", "0", "1"], True, 0),  # D1 U2 D3
     (games.create_mixed_behav_game(), [0.0, 1.0, 1.0, 0.0, 0, 1.0], False, 0),
     (games.create_mixed_behav_game(), ["0", "1", "0", "1", "1", "0"], True, 0),  # D1 D2 U3
     (games.create_mixed_behav_game(), [0.0, 1.0, 0.0, 1.0, 1.0, 0], False, 0),
     # mixed Nash equilibrium (only rational tested):
     (games.create_mixed_behav_game(), ["2/5", "3/5", "1/2", "1/2", "1/3", "2/3"], True, 0),
     # non-Nash pure profile:
     (games.create_mixed_behav_game(), [0.0, 1.0, 0.0, 1.0, 0.0, 1.0], False, 29.0),  # D1 D2 D3
     (games.create_mixed_behav_game(), ["0", "1", "0", "1", "0", "1"], True, "29"),
     # uniform (non-Nash):
     (games.create_myerson_2_card_poker(), None, True, "15/8"),
     (games.create_myerson_2_card_poker(), None, False, 1.875),
     # mixed Nash equilibrium (only rational tested):
     (games.create_myerson_2_card_poker(), ["1", "0", "1/3", "2/3", "2/3", "1/3"], True, 0),
     # non-Nash pure profile:
     # Raise at 1:1, Raise at 1:2, Meet at 2:1
     (games.create_myerson_2_card_poker(), ["1", "0", "1", "0", "1", "0"], True, 1),
     (games.create_myerson_2_card_poker(), [1.0, 0.0, 1.0, 0.0, 1.0, 0.0], False, 1.0),
     ]
)
def test_liap_value_reference(game: gbt.Game, action_probs: typing.Union[None, list],
                              rational_flag: bool, expected_value: typing.Union[str, float]):
    """Tests liap_value under profile given by action_probs
    (which will be uniform if action_probs is None)
    """
    profile = game.mixed_behavior_profile(rational=rational_flag)
    if action_probs:
        _set_action_probs(profile, action_probs, rational_flag)
    assert (
        profile.liap_value() == (gbt.Rational(expected_value) if rational_flag else expected_value)
    )


@pytest.mark.parametrize(
    "game,tol,probs,infoset_idx,member_idx,value,rational_flag",
    [(games.create_mixed_behav_game(), TOL, [0.8, 0.2, 0.4, 0.6, 0.0, 1.0], 0, 0, 1.0, False),
     (games.create_mixed_behav_game(), TOL, [0.8, 0.2, 0.4, 0.6, 0.0, 1.0], 1, 0, 0.8, False),
     (games.create_mixed_behav_game(), TOL, [0.8, 0.2, 0.4, 0.6, 0.0, 1.0], 1, 1, 0.2, False),
     (games.create_mixed_behav_game(), TOL, [0.8, 0.2, 0.4, 0.6, 0.0, 1.0], 2, 0, 0.32, False),
     (games.create_mixed_behav_game(), TOL, [0.8, 0.2, 0.4, 0.6, 0.0, 1.0], 2, 1, 0.48, False),
     (games.create_mixed_behav_game(), ZERO, ["4/5", "1/5", "2/5", "3/5", "0", "1"], 0, 0, "1",
      True),
     (games.create_mixed_behav_game(), ZERO, ["4/5", "1/5", "2/5", "3/5", "0", "1"], 1, 0, "4/5",
      True),
     (games.create_mixed_behav_game(), ZERO, ["4/5", "1/5", "2/5", "3/5", "0", "1"], 1, 1, "1/5",
      True),
     (games.create_mixed_behav_game(), ZERO, ["4/5", "1/5", "2/5", "3/5", "0", "1"], 2, 0, "8/25",
      True),
     (games.create_mixed_behav_game(), ZERO, ["4/5", "1/5", "2/5", "3/5", "0", "1"], 2, 1, "12/25",
      True),
     (games.create_myerson_2_card_poker(), ZERO, ["4/5", "1/5", "2/5", "3/5", "0", "1"],
      0, 0, "1", True),
     (games.create_myerson_2_card_poker(), ZERO, ["4/5", "1/5", "2/5", "3/5", "0", "1"],
      1, 0, "1", True),
     (games.create_myerson_2_card_poker(), ZERO, ["4/5", "1/5", "2/5", "3/5", "0", "1"],
      2, 0, "2/3", True),
     (games.create_myerson_2_card_poker(), ZERO, ["4/5", "1/5", "2/5", "3/5", "0", "1"],
      2, 1, "1/3", True),
     (games.create_myerson_2_card_poker(), ZERO, ["1", "0", "2/5", "3/5", "0", "1"],
      2, 0, "5/7", True),
     (games.create_myerson_2_card_poker(), ZERO, ["1", "0", "2/5", "3/5", "0", "1"],
      2, 1, "2/7", True),
     ]
    )
def test_node_belief_reference(game: gbt.Game, tol: typing.Union[gbt.Rational, float],
                               probs: list, infoset_idx: int, member_idx: int,
                               value: typing.Union[str, float], rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    _set_action_probs(profile, probs, rational_flag)
    node = game.infosets[infoset_idx].members[member_idx]
    value = gbt.Rational(value) if rational_flag else value
    assert abs(profile.belief(node) - value) <= tol


@pytest.mark.parametrize(
    "game,rational_flag",
    [(games.create_myerson_2_card_poker(), True),
     (games.create_myerson_2_card_poker(), False),
     ]
)
def test_payoff_with_chance_player(game: gbt.Game, rational_flag: bool):
    """Ensure a value error is thrown when we call payoff for a chance player"""
    chance_player = game.players.chance
    with pytest.raises(ValueError):
        game.mixed_behavior_profile(rational=rational_flag).payoff(chance_player)


@pytest.mark.parametrize(
    "game,rational_flag",
    [(games.create_myerson_2_card_poker(), True),
     (games.create_myerson_2_card_poker(), False),
     ]
)
def test_payoff_with_chance_player_infoset(game: gbt.Game, rational_flag: bool):
    """Ensure a value error is raised when we call action value for a chance action"""
    chance_infoset = game.players.chance.infosets[0]
    with pytest.raises(ValueError):
        game.mixed_behavior_profile(rational=rational_flag).infoset_value(chance_infoset)


@pytest.mark.parametrize(
    "game,rational_flag",
    [(games.create_myerson_2_card_poker(), True),
     (games.create_myerson_2_card_poker(), False),
     ]
)
def test_action_value_with_chance_player_action(game: gbt.Game, rational_flag: bool):
    """Ensure a value error is raised when we call action value for a chance action"""
    chance_action = game.players.chance.infosets[0].actions[0]
    with pytest.raises(ValueError):
        game.mixed_behavior_profile(rational=rational_flag).action_value(chance_action)


def _get_answers_one_order(game: gbt.Game, action_probs_1st: tuple, action_probs_2nd: tuple,
                           rational_flag: bool, func_to_test: typing.Callable,
                           object_to_test_on: typing.Any):
    """helper function for the 'profile_order' caching tests"""
    ret = dict()
    profile = game.mixed_behavior_profile(rational=rational_flag)
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


PROBS_1A_doub = (0.2, 0.8, 0.1, 0.9, 0.5, 0.5)
PROBS_2A_doub = (1.0, 0.0, 1.0, 0.0, 1.0, 0.0)
PROBS_1A_rat = ("2/10", "8/10", "1/10", "9/10", "1/2", "1/2")
PROBS_2A_rat = ("1", "0", "1", "0", "1", "0")
PROBS_1B_doub = (0.5, 0.5, 0.5, 0.5, 0.5, 0.5)
PROBS_2B_doub = (1.0, 0.0, 1.0, 0.0, 1.0, 0.0)
PROBS_1B_rat = ("1/2", "1/2", "1/2", "1/2", "1/2", "1/2")
PROBS_2B_rat = ("1", "0", "1", "0", "1", "0")


@pytest.mark.parametrize(
    "game,action_probs1,action_probs2,rational_flag,func_to_test,objects_to_test",
    [
     ######################################################################################
     # belief (at nodes)
     (games.create_mixed_behav_game(), PROBS_1A_doub, PROBS_2A_doub, False,
      lambda x, y: x.belief(y), lambda x: x.nodes()),
     (games.create_mixed_behav_game(), PROBS_1A_rat, PROBS_2A_rat, True,
      lambda x, y: x.belief(y), lambda x: x.nodes()),
     (games.create_myerson_2_card_poker(), PROBS_1B_doub, PROBS_2B_doub, False,
      lambda x, y: x.belief(y), lambda x: x.nodes()),
     (games.create_myerson_2_card_poker(), PROBS_1A_rat, PROBS_2A_rat, True,
      lambda x, y: x.belief(y), lambda x: x.nodes()),
     ######################################################################################
     # realiz_prob (at nodes)
     (games.create_mixed_behav_game(), PROBS_1A_doub, PROBS_2A_doub, False,
      lambda x, y: x.realiz_prob(y), lambda x: x.nodes()),
     (games.create_mixed_behav_game(), PROBS_1A_rat, PROBS_2A_rat, True,
      lambda x, y: x.realiz_prob(y), lambda x: x.nodes()),
     (games.create_myerson_2_card_poker(), PROBS_1B_doub, PROBS_2B_doub, False,
      lambda x, y: x.realiz_prob(y), lambda x: x.nodes()),
     (games.create_myerson_2_card_poker(), PROBS_1A_rat, PROBS_2A_rat, True,
      lambda x, y: x.realiz_prob(y), lambda x: x.nodes()),
     ######################################################################################
     # infoset_prob
     (games.create_mixed_behav_game(), PROBS_1A_doub, PROBS_2A_doub, False,
      lambda x, y: x.infoset_prob(y), lambda x: x.infosets),
     (games.create_mixed_behav_game(), PROBS_1A_rat, PROBS_2A_rat, True,
      lambda x, y: x.infoset_prob(y), lambda x: x.infosets),
     (games.create_myerson_2_card_poker(), PROBS_1B_doub, PROBS_2B_doub, False,
      lambda x, y: x.infoset_prob(y), lambda x: x.infosets),
     (games.create_myerson_2_card_poker(), PROBS_1A_rat, PROBS_2A_rat, True,
      lambda x, y: x.infoset_prob(y), lambda x: x.infosets),
     ######################################################################################
     # infoset_value
     (games.create_mixed_behav_game(), PROBS_1A_doub, PROBS_2A_doub, False,
      lambda x, y: x.infoset_value(y), lambda x: x.infosets),
     (games.create_mixed_behav_game(), PROBS_1A_rat, PROBS_2A_rat, True,
      lambda x, y: x.infoset_value(y), lambda x: x.infosets),
     (games.create_myerson_2_card_poker(), PROBS_1B_doub, PROBS_2B_doub, False,
      lambda x, y: x.infoset_value(y), lambda x: x.infosets),
     (games.create_myerson_2_card_poker(), PROBS_1A_rat, PROBS_2A_rat, True,
      lambda x, y: x.infoset_value(y), lambda x: x.infosets),
     ######################################################################################
     # action_value
     (games.create_mixed_behav_game(), PROBS_1A_doub, PROBS_2A_doub, False,
      lambda x, y: x.action_value(y), lambda x: x.actions),
     (games.create_mixed_behav_game(), PROBS_1A_rat, PROBS_2A_rat, True,
      lambda x, y: x.action_value(y), lambda x: x.actions),
     (games.create_myerson_2_card_poker(), PROBS_1B_doub, PROBS_2B_doub, False,
      lambda x, y: x.action_value(y), lambda x: x.actions),
     (games.create_myerson_2_card_poker(), PROBS_1A_rat, PROBS_2A_rat, True,
      lambda x, y: x.action_value(y), lambda x: x.actions),
     ######################################################################################
     # regret (for actions)
     (games.create_mixed_behav_game(), PROBS_1A_doub, PROBS_2A_doub, False,
      lambda x, y: x.regret(y), lambda x: x.actions),
     (games.create_mixed_behav_game(), PROBS_1A_rat, PROBS_2A_rat, True,
      lambda x, y: x.regret(y), lambda x: x.actions),
     (games.create_myerson_2_card_poker(), PROBS_1B_doub, PROBS_2B_doub, False,
      lambda x, y: x.regret(y), lambda x: x.actions),
     (games.create_myerson_2_card_poker(), PROBS_1A_rat, PROBS_2A_rat, True,
      lambda x, y: x.regret(y), lambda x: x.actions),
     ######################################################################################
     # node_value
     (games.create_mixed_behav_game(), PROBS_1A_doub, PROBS_2A_doub, False,
      lambda x, y: x.node_value(player=y[0], node=y[1]),
      lambda x: list(product(x.players, x.nodes()))),
     (games.create_mixed_behav_game(), PROBS_1A_rat, PROBS_2A_rat, True,
      lambda x, y: x.node_value(player=y[0], node=y[1]),
      lambda x: list(product(x.players, x.nodes()))),
     (games.create_myerson_2_card_poker(), PROBS_1B_doub, PROBS_2B_doub, False,
      lambda x, y: x.node_value(player=y[0], node=y[1]),
      lambda x: list(product(x.players, x.nodes()))),
     (games.create_myerson_2_card_poker(), PROBS_1A_rat, PROBS_2A_rat, True,
      lambda x, y: x.node_value(player=y[0], node=y[1]),
      lambda x: list(product(x.players, x.nodes()))),
     ######################################################################################
     # liap_value (of profile, hence [1] for objects_to_test, any singleton collection would do)
     (games.create_mixed_behav_game(), PROBS_1A_doub, PROBS_2A_doub, False,
      lambda x, y: x.liap_value(), lambda x: [1]),
     (games.create_mixed_behav_game(), PROBS_1A_rat, PROBS_2A_rat, True,
      lambda x, y: x.liap_value(), lambda x: [1]),
     (games.create_myerson_2_card_poker(), PROBS_1B_doub, PROBS_2B_doub, False,
      lambda x, y: x.liap_value(), lambda x: [1]),
     (games.create_myerson_2_card_poker(), PROBS_1A_rat, PROBS_2A_rat, True,
      lambda x, y: x.liap_value(), lambda x: [1]),
     ]
)
def test_profile_order_consistency(game: gbt.Game,
                                   action_probs1: tuple,
                                   action_probs2: tuple, rational_flag: bool,
                                   func_to_test: typing.Callable,
                                   objects_to_test: typing.Callable):
    _get_and_check_answers(game, action_probs1, action_probs2, rational_flag,
                           func_to_test, objects_to_test(game))
