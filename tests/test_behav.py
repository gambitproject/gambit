import pytest
import typing

import pygambit as gbt

from . import games


# tolerance for floating point assertions
TOL = 1e-13



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
     (games.create_mixed_behav_game(), 2, "13/4", True)
     ]
)
def test_payoff(game: gbt.Game, player_idx: int, payoff: typing.Union[str, float],
                rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    payoff = gbt.Rational(payoff) if rational_flag else payoff
    assert profile.payoff(game.players[player_idx]) == payoff


@pytest.mark.parametrize(
    "game,label,payoff,rational_flag",
    [(games.create_mixed_behav_game(), "Player 1", 3.0, False),
     (games.create_mixed_behav_game(), "Player 2", 3.0, False),
     (games.create_mixed_behav_game(), "Player 3", 3.25, False),
     (games.create_mixed_behav_game(), "Player 1", 3.0, False),
     (games.create_mixed_behav_game(), "Player 2", 3.0, False),
     (games.create_mixed_behav_game(), "Player 3", 3.25, False)
     ]
)
def test_payoff_by_label(game: gbt.Game, label: str, payoff: typing.Union[str, float],
                         rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    payoff = gbt.Rational(payoff) if rational_flag else payoff
    assert profile.payoff(label) == payoff


@pytest.mark.parametrize(
    "game,player_idx,infoset_idx,rational_flag",
    [(games.create_mixed_behav_game(), 0, 0, False),
     (games.create_mixed_behav_game(), 1, 0, False),
     (games.create_mixed_behav_game(), 2, 0, False),
     (games.create_mixed_behav_game(), 0, 0, True),
     (games.create_mixed_behav_game(), 1, 0, True),
     (games.create_mixed_behav_game(), 2, 0, True),
     ]
)
def test_is_defined_at(game: gbt.Game, player_idx: int, infoset_idx: int, rational_flag: bool):
    """Test to check if an infoset is defined"""
    infoset = game.players[player_idx].infosets[infoset_idx]
    profile = game.mixed_behavior_profile(rational=rational_flag)
    assert profile.is_defined_at(infoset)


@pytest.mark.parametrize(
    "game,label,rational_flag",
    [(games.create_mixed_behav_game(), "Infoset 1:1", False),
     (games.create_mixed_behav_game(), "Infoset 2:1", False),
     (games.create_mixed_behav_game(), "Infoset 3:1", False),
     (games.create_mixed_behav_game(), "Infoset 1:1", True),
     (games.create_mixed_behav_game(), "Infoset 2:1", True),
     (games.create_mixed_behav_game(), "Infoset 3:1", True)
     ]
)
def test_is_defined_at_by_label(game: gbt.Game, label: str, rational_flag: bool):
    """Test to check if an infoset is defined by string labels"""
    profile = game.mixed_behavior_profile(rational=rational_flag)
    assert profile.is_defined_at(label)


@pytest.mark.parametrize(
    "game,player,infoset,action,prob,rational_flag",
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
     (games.create_mixed_behav_game(), 2, 0, 1, "1/2", True)]
)
def test_get_probabilities_action(game: gbt.Game, player: int, infoset: int, action: int,
                                  prob: typing.Union[str, float], rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    action = game.players[player].infosets[infoset].actions[action]
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
     (games.create_mixed_behav_game(), "D3", "1/2", True)]
)
def test_get_probabilities_action_by_label(game: gbt.Game, label: str,
                                           prob: typing.Union[str, float], rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    prob = gbt.Rational(prob) if rational_flag else prob
    assert profile[label] == prob


@pytest.mark.parametrize(
    "game,player_idx,infoset_idx,probs,rational_flag",
    [(games.create_mixed_behav_game(), 0, 0, [0.5, 0.5], False),
     (games.create_mixed_behav_game(), 1, 0, [0.5, 0.5], False),
     (games.create_mixed_behav_game(), 2, 0, [0.5, 0.5], False),
     (games.create_mixed_behav_game(), 0, 0, ["1/2", "1/2"], True),
     (games.create_mixed_behav_game(), 1, 0, ["1/2", "1/2"], True),
     (games.create_mixed_behav_game(), 2, 0, ["1/2", "1/2"], True)]
)
def test_get_probabilities_infoset(game: gbt.Game, player_idx: int, infoset_idx: int, probs: list,
                                   rational_flag: bool):
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
     (games.create_mixed_behav_game(), 2, "Infoset 3:1", ["1/2", "1/2"], True)]
)
def test_get_probabilities_infoset_by_label(game: gbt.Game, player_idx: int, infoset_label: str,
                                            probs: list, rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    player = game.players[player_idx]
    probs = [gbt.Rational(prob) for prob in probs] if rational_flag else probs
    assert profile[player][infoset_label] == probs
    assert profile[infoset_label] == probs


@pytest.mark.parametrize(
    "game,player_idx,probs,rational_flag",
    [(games.create_mixed_behav_game(), 0, [0.5, 0.5], False),
     (games.create_mixed_behav_game(), 1, [0.5, 0.5], False),
     (games.create_mixed_behav_game(), 2, [0.5, 0.5], False),
     (games.create_mixed_behav_game(), 0, ["1/2", "1/2"], True),
     (games.create_mixed_behav_game(), 1, ["1/2", "1/2"], True),
     (games.create_mixed_behav_game(), 2, ["1/2", "1/2"], True)]
)
def test_get_probabilities_player(game: gbt.Game, player_idx: int, probs: list,
                                  rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    player = game.players[player_idx]
    probs = [gbt.Rational(prob) for prob in probs] if rational_flag else probs
    assert profile[player] == [probs]


@pytest.mark.parametrize(
    "game,player_label,probs,rational_flag",
    [(games.create_mixed_behav_game(), "Player 1", [0.5, 0.5], False),
     (games.create_mixed_behav_game(), "Player 2", [0.5, 0.5], False),
     (games.create_mixed_behav_game(), "Player 3", [0.5, 0.5], False),
     (games.create_mixed_behav_game(), "Player 1", ["1/2", "1/2"], True),
     (games.create_mixed_behav_game(), "Player 2", ["1/2", "1/2"], True),
     (games.create_mixed_behav_game(), "Player 3", ["1/2", "1/2"], True)]
)
def test_get_probabilities_player_by_label(game: gbt.Game, player_label: str, probs: list,
                                           rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    probs = [gbt.Rational(prob) for prob in probs] if rational_flag else probs
    assert profile[player_label] == [probs]


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
     (games.create_mixed_behav_game(), 5, "97/98", True)]
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
     (games.create_mixed_behav_game(), "D3", "97/98", True)]
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
    "game,player_idx,probs,rational_flag",
    [(games.create_mixed_behav_game(), 0, [[0.72, 0.28]], False),
     (games.create_mixed_behav_game(), 1, [[0.42, 0.58]], False),
     (games.create_mixed_behav_game(), 2, [[0.02, 0.98]], False),
     (games.create_mixed_behav_game(), 0, [["7/9", "2/9"]], True),
     (games.create_mixed_behav_game(), 1, [["4/13", "9/13"]], True),
     (games.create_mixed_behav_game(), 2, [["1/98", "97/98"]], True),
     ]
)
def test_set_probabilities_player(game: gbt.Game, player_idx: int, probs: list,
                                  rational_flag: bool):
    player = game.players[player_idx]
    profile = game.mixed_behavior_profile(rational=rational_flag)
    if rational_flag:
        probs = [[gbt.Rational(p) for p in probs[0]]]
    profile[player] = probs
    assert profile[player] == probs


@pytest.mark.parametrize(
    "game,player_label,probs,rational_flag",
    [(games.create_mixed_behav_game(), "Player 1", [[0.72, 0.28]], False),
     (games.create_mixed_behav_game(), "Player 2", [[0.42, 0.58]], False),
     (games.create_mixed_behav_game(), "Player 3", [[0.02, 0.98]], False),
     (games.create_mixed_behav_game(), "Player 1", [["7/9", "2/9"]], True),
     (games.create_mixed_behav_game(), "Player 2", [["4/13", "9/13"]], True),
     (games.create_mixed_behav_game(), "Player 3", [["1/98", "97/98"]], True),
     ]
)
def test_set_probabilities_player_by_label(game: gbt.Game, player_label: str, probs: list,
                                           rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    if rational_flag:
        probs = [[gbt.Rational(p) for p in probs[0]]]
    profile[player_label] = probs
    assert profile[player_label] == probs


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
     (games.create_complicated_extensive_game(), 0, "1", True),
     (games.create_complicated_extensive_game(), 1, "1/2", True),
     (games.create_complicated_extensive_game(), 2, "1/4", True),
     (games.create_complicated_extensive_game(), 3, "1/8", True),
     (games.create_complicated_extensive_game(), 4, "1/8", True),
     (games.create_complicated_extensive_game(), 5, "1/4", True),
     (games.create_complicated_extensive_game(), 6, "1/2", True),
     (games.create_complicated_extensive_game(), 7, "1/4", True),
     (games.create_complicated_extensive_game(), 8, "1/8", True),
     (games.create_complicated_extensive_game(), 9, "1/8", True),
     (games.create_complicated_extensive_game(), 10, "1/4", True),
     (games.create_complicated_extensive_game(), 0, 1.0, False),
     (games.create_complicated_extensive_game(), 1, 0.5, False),
     (games.create_complicated_extensive_game(), 2, 0.25, False),
     (games.create_complicated_extensive_game(), 3, 0.125, False),
     (games.create_complicated_extensive_game(), 4, 0.125, False),
     (games.create_complicated_extensive_game(), 5, 0.25, False),
     (games.create_complicated_extensive_game(), 6, 0.5, False),
     (games.create_complicated_extensive_game(), 7, 0.25, False),
     (games.create_complicated_extensive_game(), 8, 0.125, False),
     (games.create_complicated_extensive_game(), 9, 0.125, False),
     (games.create_complicated_extensive_game(), 10, 0.25, False)]
)
def test_realiz_prob_nodes(game: gbt.Game, node_idx: int, realiz_prob: typing.Union[str, float],
                           rational_flag: bool):
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
     (games.create_mixed_behav_game(), 2, 0, "1", True)]
)
def test_infoset_prob(game: gbt.Game, player_idx: int, infoset_idx: int,
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
     (games.create_mixed_behav_game(), "Infoset 3:1", "1", True)]
)
def test_infoset_prob_by_label(game: gbt.Game, label: str, prob: typing.Union[str, float],
                               rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    assert profile.infoset_prob(label) == (gbt.Rational(prob) if rational_flag else prob)


@pytest.mark.parametrize(
    "game,player_idx,infoset_idx,payoff,rational_flag",
    [(games.create_mixed_behav_game(), 0, 0, 3.0, False),
     (games.create_mixed_behav_game(), 1, 0, 3.0, False),
     (games.create_mixed_behav_game(), 2, 0, 3.25, False),
     (games.create_mixed_behav_game(), 0, 0, "3", True),
     (games.create_mixed_behav_game(), 1, 0, "3", True),
     (games.create_mixed_behav_game(), 2, 0, "13/4", True)]
)
def test_infoset_payoff(game: gbt.Game, player_idx: int, infoset_idx: int,
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
     (games.create_mixed_behav_game(), "Infoset 3:1", "13/4", True)]
)
def test_infoset_payoff_by_label(game: gbt.Game, label: str, payoff: typing.Union[str, float],
                                 rational_flag: bool):
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
     ]
)
def test_action_payoff(game: gbt.Game, player_idx: int, infoset_idx: int, action_idx: int,
                       payoff: typing.Union[str, float], rational_flag: bool):
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
     (games.create_mixed_behav_game(), "D3", "3", True)]
)
def test_action_value_by_label(game: gbt.Game, label: str, payoff: typing.Union[str, float],
                               rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    assert profile.action_value(label) == (gbt.Rational(payoff) if rational_flag else payoff)


@pytest.mark.parametrize(
    "game,rational_flag",
    [(games.create_mixed_behav_game(), False),
     (games.create_mixed_behav_game(), True),
     ]
)
def test_regret(game: gbt.Game, rational_flag: bool):
    """Consistency check for the regret of a profile"""
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
    "game,infoset_idx,member_idx,rational_flag",
    [(games.create_complicated_extensive_game(), 0, 0, False),
     (games.create_complicated_extensive_game(), 0, 0, True),
     (games.create_complicated_extensive_game(), 1, 0, False),
     (games.create_complicated_extensive_game(), 1, 0, True)]
)
def test_martingale_property_of_node_value(game: gbt.Game, infoset_idx: int, member_idx: int,
                                           rational_flag: bool):
    """Given a node, this checks that the node value is equal to the expected value of the node
    values of its children, using the normalized realization probabilities of those children
    """
    profile = game.mixed_behavior_profile(rational=rational_flag)
    parent = game.infosets[infoset_idx].members[member_idx]
    expected_val = 0
    parent_prob = profile.realiz_prob(parent)
    for child in parent.children:
        prob = profile.realiz_prob(child) / parent_prob
        expected_val += prob * profile.node_value(parent.player, child)
    assert profile.node_value(parent.player, parent) == expected_val


@pytest.mark.parametrize(
    "game,rational_flag",
    [(games.create_mixed_behav_game(), False),
     (games.create_mixed_behav_game(), True)]
)
def test_node_value(game: gbt.Game, rational_flag: bool):
    """Test that the profile's node value at the root for each player matches the profile's payoff
    for the respective player"""
    profile = game.mixed_behavior_profile(rational=rational_flag)
    for player in game.players:
        assert profile.node_value(player, game.root) == profile.payoff(player)


@pytest.mark.parametrize(
    "game,rational_flag,expected_value",
    [(games.create_mixed_behav_game(), True, "1/16"),
     (games.create_mixed_behav_game(), True, 0.0625),
     ]
)
def test_liap_value(game: gbt.Game, rational_flag: bool, expected_value: typing.Union[str, float]):
    """Tests liap_value under the assumption of a default uniform behavior profile"""
    profile = game.mixed_behavior_profile(rational=rational_flag)
    assert (
        profile.liap_value() == (gbt.Rational(expected_value) if rational_flag else expected_value)
    )


@pytest.mark.parametrize(
    "game,rational_flag",
    [(games.create_mixed_behav_game(), True),
     (games.create_mixed_behav_game(), False)]
)
def test_as_strategy(game: gbt.Game, rational_flag: bool):
    behav_profile = game.mixed_behavior_profile(rational=rational_flag)
    mixed_profile = behav_profile.as_strategy()
    assert (
        [mixed_profile[strategy] for strategy in game.strategies] ==
        [behav_profile[action] for action in game.actions]
    )


@pytest.mark.parametrize(
    "game,tol,values,infoset_idx,member_idx,value,rational_flag",
    [(games.create_mixed_behav_game(), TOL, [0.8, 0.2, 0.4, 0.6, 0.0, 1.0], 0, 0, 1.0, False),
     (games.create_mixed_behav_game(), TOL, [0.8, 0.2, 0.4, 0.6, 0.0, 1.0], 1, 0, 0.8, False),
     (games.create_mixed_behav_game(), TOL, [0.8, 0.2, 0.4, 0.6, 0.0, 1.0], 1, 1, 0.2, False),
     (games.create_mixed_behav_game(), TOL, [0.8, 0.2, 0.4, 0.6, 0.0, 1.0], 2, 0, 0.32, False),
     (games.create_mixed_behav_game(), TOL, [0.8, 0.2, 0.4, 0.6, 0.0, 1.0], 2, 1, 0.48, False),
     (games.create_mixed_behav_game(), TOL, ["4/5", "1/5", "2/5", "3/5", "0", "1"], 0, 0, "1", True),
     (games.create_mixed_behav_game(), TOL, ["4/5", "1/5", "2/5", "3/5", "0", "1"], 1, 0, "4/5", True),
     (games.create_mixed_behav_game(), TOL, ["4/5", "1/5", "2/5", "3/5", "0", "1"], 1, 1, "1/5", True),
     (games.create_mixed_behav_game(), TOL, ["4/5", "1/5", "2/5", "3/5", "0", "1"], 2, 0, "8/25", True),
     (games.create_mixed_behav_game(), TOL, ["4/5", "1/5", "2/5", "3/5", "0", "1"], 2, 1, "12/25", True)
     ]
    )
def test_node_belief(game: gbt.Game, tol: float, values: list, infoset_idx: int,
                     member_idx: int, value: typing.Union[str, float], rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    _set_action_probs(profile, values, rational_flag)
    node = game.infosets[infoset_idx].members[member_idx]
    if rational_flag:
        assert profile.belief(node) == gbt.Rational(value)
    else:
        assert abs(profile.belief(node) - value) < tol


@pytest.mark.parametrize(
    "game,rational_flag",
    [(games.create_complicated_extensive_game(), True),
     (games.create_complicated_extensive_game(), False)]
)
def test_payoff_with_chance_player(game: gbt.Game, rational_flag: bool):
    """Ensure a value error is thrown when we call payoff for a chance player"""
    chance_player = game.players.chance
    with pytest.raises(ValueError):
        game.mixed_behavior_profile(rational=rational_flag).payoff(chance_player)


@pytest.mark.parametrize(
    "game,rational_flag",
    [(games.create_complicated_extensive_game(), True),
     (games.create_complicated_extensive_game(), False)]
)
def test_payoff_with_chance_player_infoset(game: gbt.Game, rational_flag: bool):
    """Ensure a value error is raised when we call action value for a chance action"""
    chance_infoset = game.players.chance.infosets[0]
    with pytest.raises(ValueError):
        game.mixed_behavior_profile(rational=rational_flag).infoset_value(chance_infoset)


@pytest.mark.parametrize(
    "game,rational_flag",
    [(games.create_complicated_extensive_game(), True),
     (games.create_complicated_extensive_game(), False)]
)
def test_action_value_with_chance_player_action(game: gbt.Game, rational_flag: bool):
    """Ensure a value error is raised when we call action value for a chance action"""
    chance_action = game.players.chance.infosets[0].actions[0]
    with pytest.raises(ValueError):
        game.mixed_behavior_profile(rational=rational_flag).action_value(chance_action)
