import typing
from itertools import product

import pytest

import pygambit as gbt

from . import games

TOL = 1e-13  # tolerance for floating point assertions
ZERO = gbt.Rational(0)  # tolerance for rational assertions


def _set_action_probs(profile: gbt.MixedBehaviorProfile, probs: list, rational_flag: bool):
    """Set the action probabilities in a behavior profile called ```profile``` according to a
    list with probabilities in the order of ```profile.game.actions``` (grouped by information
    set, matching how ```game.actions``` itself is ordered).
    """
    convert = (lambda p: gbt.Rational(p)) if rational_flag else (lambda p: p)
    probs_iter = iter(probs)
    for infoset in profile.game.infosets:
        node = next(iter(infoset.members))
        profile[node] = {a.label: convert(next(probs_iter)) for a in infoset.actions}


@pytest.mark.parametrize(
    "game,label,payoff,rational_flag",
    [
        (games.read_from_file("mixed_behavior_game.efg"), "Player 1", 3.0, False),
        (games.read_from_file("mixed_behavior_game.efg"), "Player 2", 3.0, False),
        (games.read_from_file("mixed_behavior_game.efg"), "Player 3", 3.25, False),
        (games.read_from_file("mixed_behavior_game.efg"), "Player 1", "3", True),
        (games.read_from_file("mixed_behavior_game.efg"), "Player 2", "3", True),
        (games.read_from_file("mixed_behavior_game.efg"), "Player 3", "13/4", True),
        (games.create_stripped_down_poker_efg(), "Alice", -0.25, False),
        (games.create_stripped_down_poker_efg(), "Bob", 0.25, False),
        (games.create_stripped_down_poker_efg(), "Alice", "-1/4", True),
        (games.create_stripped_down_poker_efg(), "Bob", "1/4", True),
    ],
)
def test_payoff_by_label_reference(
    game: gbt.Game, label: str, payoff: str | float, rational_flag: bool
):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    payoff = gbt.Rational(payoff) if rational_flag else payoff
    assert profile.payoff(label) == payoff


@pytest.mark.parametrize(
    "game,rational_flag",
    [
        (games.read_from_file("mixed_behavior_game.efg"), False),
        (games.read_from_file("mixed_behavior_game.efg"), True),
        (games.create_stripped_down_poker_efg(), False),
        (games.create_stripped_down_poker_efg(), True),
    ],
)
def test_is_defined_at(game: gbt.Game, rational_flag: bool):
    """Test to check if infoset are all defined"""
    profile = game.mixed_behavior_profile(rational=rational_flag)
    for infoset in game.infosets:
        assert profile.is_defined_at(infoset)


@pytest.mark.parametrize(
    "game,label,rational_flag",
    [
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 1:1", False),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 2:1", False),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 3:1", False),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 1:1", True),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 2:1", True),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 3:1", True),
        (games.create_stripped_down_poker_efg(), "Alice has King", False),
        (games.create_stripped_down_poker_efg(), "Alice has Queen", False),
        (games.create_stripped_down_poker_efg(), "Bob's response", False),
        (games.create_stripped_down_poker_efg(), "Alice has King", True),
        (games.create_stripped_down_poker_efg(), "Alice has Queen", True),
        (games.create_stripped_down_poker_efg(), "Bob's response", True),
    ],
)
def test_is_defined_at_by_label(game: gbt.Game, label: str, rational_flag: bool):
    """Test to check if an infoset is defined by string labels"""
    profile = game.mixed_behavior_profile(rational=rational_flag)
    assert profile.is_defined_at(label)


@pytest.mark.parametrize(
    "game,player_label,infoset_label,action_label,prob,rational_flag",
    [
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 1",
            "Infoset 1:1",
            "U1",
            0.5,
            False,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 1",
            "Infoset 1:1",
            "D1",
            0.5,
            False,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 2",
            "Infoset 2:1",
            "U2",
            0.5,
            False,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 2",
            "Infoset 2:1",
            "D2",
            0.5,
            False,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 3",
            "Infoset 3:1",
            "U3",
            0.5,
            False,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 3",
            "Infoset 3:1",
            "D3",
            0.5,
            False,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 1",
            "Infoset 1:1",
            "U1",
            "1/2",
            True,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 1",
            "Infoset 1:1",
            "D1",
            "1/2",
            True,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 2",
            "Infoset 2:1",
            "U2",
            "1/2",
            True,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 2",
            "Infoset 2:1",
            "D2",
            "1/2",
            True,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 3",
            "Infoset 3:1",
            "U3",
            "1/2",
            True,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 3",
            "Infoset 3:1",
            "D3",
            "1/2",
            True,
        ),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has King", "Bet", 0.5, False),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has King", "Fold", 0.5, False),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has Queen", "Bet", 0.5, False),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has Queen", "Fold", 0.5, False),
        (games.create_stripped_down_poker_efg(), "Bob", "Bob's response", "Call", 0.5, False),
        (games.create_stripped_down_poker_efg(), "Bob", "Bob's response", "Fold", 0.5, False),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has King", "Bet", "1/2", True),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has King", "Fold", "1/2", True),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has Queen", "Bet", "1/2", True),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has Queen", "Fold", "1/2", True),
        (games.create_stripped_down_poker_efg(), "Bob", "Bob's response", "Call", "1/2", True),
        (games.create_stripped_down_poker_efg(), "Bob", "Bob's response", "Fold", "1/2", True),
    ],
)
def test_profile_indexing_by_player_infoset_action_reference(
    game: gbt.Game,
    player_label: str,
    infoset_label: str,
    action_label: str,
    prob: str | float,
    rational_flag: bool,
):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    infoset = game.players[player_label].infosets[infoset_label]
    node = next(iter(infoset.members))
    prob = gbt.Rational(prob) if rational_flag else prob
    assert profile[node][action_label] == prob


@pytest.mark.parametrize(
    "game,player_label,infoset_label,probs,rational_flag",
    [
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 1",
            "Infoset 1:1",
            [0.5, 0.5],
            False,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 2",
            "Infoset 2:1",
            [0.5, 0.5],
            False,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 3",
            "Infoset 3:1",
            [0.5, 0.5],
            False,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 1",
            "Infoset 1:1",
            ["1/2", "1/2"],
            True,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 2",
            "Infoset 2:1",
            ["1/2", "1/2"],
            True,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 3",
            "Infoset 3:1",
            ["1/2", "1/2"],
            True,
        ),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has King", [0.5, 0.5], False),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has Queen", [0.5, 0.5], False),
        (games.create_stripped_down_poker_efg(), "Bob", "Bob's response", [0.5, 0.5], False),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has King", ["1/2", "1/2"], True),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has Queen", ["1/2", "1/2"], True),
        (games.create_stripped_down_poker_efg(), "Bob", "Bob's response", ["1/2", "1/2"], True),
    ],
)
def test_profile_indexing_by_player_and_infoset_reference(
    game: gbt.Game, player_label: str, infoset_label: str, probs: list, rational_flag: bool
):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    infoset = game.players[player_label].infosets[infoset_label]
    probs = [gbt.Rational(prob) for prob in probs] if rational_flag else probs
    expected = dict(zip((a.label for a in infoset.actions), probs, strict=True))
    assert profile[next(iter(infoset.members))] == expected


@pytest.mark.parametrize(
    "game,player_label,infoset_label,probs,rational_flag",
    [
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 1",
            "Infoset 1:1",
            [0.5, 0.5],
            False,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 2",
            "Infoset 2:1",
            [0.5, 0.5],
            False,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 3",
            "Infoset 3:1",
            [0.5, 0.5],
            False,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 1",
            "Infoset 1:1",
            ["1/2", "1/2"],
            True,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 2",
            "Infoset 2:1",
            ["1/2", "1/2"],
            True,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 3",
            "Infoset 3:1",
            ["1/2", "1/2"],
            True,
        ),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has King", [0.5, 0.5], False),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has Queen", [0.5, 0.5], False),
        (games.create_stripped_down_poker_efg(), "Bob", "Bob's response", [0.5, 0.5], False),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has King", ["1/2", "1/2"], True),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has Queen", ["1/2", "1/2"], True),
        (games.create_stripped_down_poker_efg(), "Bob", "Bob's response", ["1/2", "1/2"], True),
    ],
)
def test_profile_indexing_by_player_and_infoset_label_reference(
    game: gbt.Game, player_label: str, infoset_label: str, probs: list, rational_flag: bool
):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    player = game.players[player_label]
    infoset = player.infosets[infoset_label]
    node = next(iter(infoset.members))
    probs = [gbt.Rational(prob) for prob in probs] if rational_flag else probs
    expected = dict(zip((a.label for a in infoset.actions), probs, strict=True))
    assert profile[player_label][node] == expected
    assert profile[node] == expected


@pytest.mark.parametrize(
    "game,player_label,other_player_label",
    [
        (games.read_from_file("mixed_behavior_game.efg"), "Player 1", "Player 2"),
        (games.create_stripped_down_poker_efg(), "Alice", "Bob"),
    ],
)
def test_behavior_indexing_rejects_node_from_different_player(
    game: gbt.Game, player_label: str, other_player_label: str
):
    """MixedBehavior/MixedBehaviorProfile reject a Node whose information set belongs to a
    different player than the one being indexed.
    """
    profile = game.mixed_behavior_profile()
    other_infoset = next(iter(game.players[other_player_label].infosets))
    other_node = next(iter(other_infoset.members))
    with pytest.raises(gbt.MismatchError):
        profile[player_label][other_node]


@pytest.mark.parametrize(
    "game,player_label,behav_data,rational_flag",
    [
        (games.read_from_file("mixed_behavior_game.efg"), "Player 1", [[0.5, 0.5]], False),
        (games.read_from_file("mixed_behavior_game.efg"), "Player 2", [[0.5, 0.5]], False),
        (games.read_from_file("mixed_behavior_game.efg"), "Player 3", [[0.5, 0.5]], False),
        (games.read_from_file("mixed_behavior_game.efg"), "Player 1", [["1/2", "1/2"]], True),
        (games.read_from_file("mixed_behavior_game.efg"), "Player 2", [["1/2", "1/2"]], True),
        (games.read_from_file("mixed_behavior_game.efg"), "Player 3", [["1/2", "1/2"]], True),
        (games.create_stripped_down_poker_efg(), "Alice", [[0.5, 0.5], [0.5, 0.5]], False),
        (games.create_stripped_down_poker_efg(), "Bob", [[0.5, 0.5]], False),
        (games.create_stripped_down_poker_efg(), "Alice", [["1/2", "1/2"], ["1/2", "1/2"]], True),
        (games.create_stripped_down_poker_efg(), "Bob", [["1/2", "1/2"]], True),
    ],
)
def test_profile_indexing_by_player_label_reference(
    game: gbt.Game, player_label: str, behav_data: list, rational_flag: bool
):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    if rational_flag:
        behav_data = [[gbt.Rational(prob) for prob in probs] for probs in behav_data]
    player = game.players[player_label]
    expected = [
        dict(zip((a.label for a in infoset.actions), probs, strict=True))
        for infoset, probs in zip(player.infosets, behav_data, strict=True)
    ]
    assert profile[player_label] == expected


@pytest.mark.parametrize(
    "game,infoset_label,action_label,prob,rational_flag",
    [
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 1:1", "U1", 0.72, False),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 1:1", "D1", 0.28, False),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 2:1", "U2", 0.42, False),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 2:1", "D2", 0.58, False),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 3:1", "U3", 0.02, False),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 3:1", "D3", 0.98, False),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 1:1", "U1", "2/9", True),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 1:1", "D1", "7/9", True),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 2:1", "U2", "4/13", True),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 2:1", "D2", "9/13", True),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 3:1", "U3", "1/98", True),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 3:1", "D3", "97/98", True),
        (games.create_stripped_down_poker_efg(), "Alice has King", "Bet", 0.1, False),
        (games.create_stripped_down_poker_efg(), "Alice has King", "Fold", 0.2, False),
        (games.create_stripped_down_poker_efg(), "Alice has Queen", "Bet", 0.3, False),
        (games.create_stripped_down_poker_efg(), "Alice has Queen", "Fold", 0.4, False),
        (games.create_stripped_down_poker_efg(), "Bob's response", "Call", 0.5, False),
        (games.create_stripped_down_poker_efg(), "Bob's response", "Fold", 0.6, False),
        (games.create_stripped_down_poker_efg(), "Alice has King", "Bet", "1/10", True),
        (games.create_stripped_down_poker_efg(), "Alice has King", "Fold", "2/10", True),
        (games.create_stripped_down_poker_efg(), "Alice has Queen", "Bet", "3/10", True),
        (games.create_stripped_down_poker_efg(), "Alice has Queen", "Fold", "4/10", True),
        (games.create_stripped_down_poker_efg(), "Bob's response", "Call", "5/10", True),
        (games.create_stripped_down_poker_efg(), "Bob's response", "Fold", "6/10", True),
    ],
)
def test_set_probabilities_action(
    game: gbt.Game, infoset_label: str, action_label: str, prob: str | float, rational_flag: bool
):
    """Test to set probabilities of actions by infoset and action label"""
    profile = game.mixed_behavior_profile(rational=rational_flag)
    prob = gbt.Rational(prob) if rational_flag else prob
    node = next(iter(game.infosets[infoset_label].members))
    profile[node] = {action_label: prob}
    assert profile[node][action_label] == prob


@pytest.mark.parametrize(
    "game,player_label,infoset_label,probs,rational_flag",
    [
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 1",
            "Infoset 1:1",
            [0.72, 0.28],
            False,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 2",
            "Infoset 2:1",
            [0.42, 0.58],
            False,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 3",
            "Infoset 3:1",
            [0.02, 0.98],
            False,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 1",
            "Infoset 1:1",
            ["7/9", "2/9"],
            True,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 2",
            "Infoset 2:1",
            ["4/13", "9/13"],
            True,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 3",
            "Infoset 3:1",
            ["1/98", "97/98"],
            True,
        ),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has King", [0.1, 0.9], False),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has Queen", [0.2, 0.8], False),
        (games.create_stripped_down_poker_efg(), "Bob", "Bob's response", [0.3, 0.7], False),
        (
            games.create_stripped_down_poker_efg(),
            "Alice",
            "Alice has King",
            ["1/10", "9/10"],
            True,
        ),
        (
            games.create_stripped_down_poker_efg(),
            "Alice",
            "Alice has Queen",
            ["2/10", "8/10"],
            True,
        ),
        (games.create_stripped_down_poker_efg(), "Bob", "Bob's response", ["3/10", "7/10"], True),
    ],
)
def test_set_probabilities_infoset(
    game: gbt.Game, player_label: str, infoset_label: str, probs: list, rational_flag: bool
):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    if rational_flag:
        probs = [gbt.Rational(p) for p in probs]
    infoset = game.players[player_label].infosets[infoset_label]
    node = next(iter(infoset.members))
    expected = dict(zip((a.label for a in infoset.actions), probs, strict=True))
    profile[node] = expected
    assert profile[node] == expected


@pytest.mark.parametrize(
    "game,infoset_label,probs,rational_flag",
    [
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 1:1", [0.72, 0.28], False),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 2:1", [0.42, 0.58], False),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 3:1", [0.02, 0.98], False),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 1:1", ["7/9", "2/9"], True),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 2:1", ["4/13", "9/13"], True),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 3:1", ["1/98", "97/98"], True),
        (games.create_stripped_down_poker_efg(), "Alice has King", [0.1, 0.9], False),
        (games.create_stripped_down_poker_efg(), "Alice has Queen", [0.2, 0.8], False),
        (games.create_stripped_down_poker_efg(), "Bob's response", [0.3, 0.7], False),
        (games.create_stripped_down_poker_efg(), "Alice has King", ["1/10", "9/10"], True),
        (games.create_stripped_down_poker_efg(), "Alice has Queen", ["2/10", "8/10"], True),
        (games.create_stripped_down_poker_efg(), "Bob's response", ["3/10", "7/10"], True),
    ],
)
def test_set_probabilities_infoset_by_label(
    game: gbt.Game, infoset_label: str, probs: list, rational_flag: bool
):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    if rational_flag:
        probs = [gbt.Rational(p) for p in probs]
    infoset = game.infosets[infoset_label]
    node = next(iter(infoset.members))
    expected = dict(zip((a.label for a in infoset.actions), probs, strict=True))
    profile[node] = expected
    assert profile[node] == expected


@pytest.mark.parametrize(
    "game,player_label,behav_data,rational_flag",
    [
        (games.read_from_file("mixed_behavior_game.efg"), "Player 1", [[0.72, 0.28]], False),
        (games.read_from_file("mixed_behavior_game.efg"), "Player 2", [[0.42, 0.58]], False),
        (games.read_from_file("mixed_behavior_game.efg"), "Player 3", [[0.02, 0.98]], False),
        (games.read_from_file("mixed_behavior_game.efg"), "Player 1", [["7/9", "2/9"]], True),
        (games.read_from_file("mixed_behavior_game.efg"), "Player 2", [["4/13", "9/13"]], True),
        (games.read_from_file("mixed_behavior_game.efg"), "Player 3", [["1/98", "97/98"]], True),
        (games.create_stripped_down_poker_efg(), "Alice", [[0.1, 0.9], [0.5, 0.5]], False),
        (games.create_stripped_down_poker_efg(), "Bob", [[0.6, 0.4]], False),
        (games.create_stripped_down_poker_efg(), "Alice", [["1/3", "2/3"], ["1/2", "1/2"]], True),
        (games.create_stripped_down_poker_efg(), "Bob", [["2/3", "1/3"]], True),
    ],
)
def test_set_probabilities_player_by_label(
    game: gbt.Game, player_label: str, behav_data: list, rational_flag: bool
):
    """A whole player's behavior is set information-set by information set: there is no
    single-call whole-player setter, since (unlike a whole player's mixed strategy) there
    is no meaningful atomic unit larger than one information set's distribution.
    """
    profile = game.mixed_behavior_profile(rational=rational_flag)
    if rational_flag:
        behav_data = [[gbt.Rational(prob) for prob in probs] for probs in behav_data]
    player = game.players[player_label]
    expected = [
        dict(zip((a.label for a in infoset.actions), probs, strict=True))
        for infoset, probs in zip(player.infosets, behav_data, strict=True)
    ]
    for infoset, distribution in zip(player.infosets, expected, strict=True):
        profile[next(iter(infoset.members))] = distribution
    assert profile[player_label] == expected


def _p1_node(game: gbt.Game):
    return next(iter(next(iter(game.players["Player 1"].infosets)).members))


def test_behavior_setitem_allows_sparse_distribution():
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.mixed_behavior_profile()
    node = _p1_node(game)
    profile[node] = {"U1": 1}
    assert profile[node] == {"U1": 1, "D1": 0}


def test_set_mixed_action_sparse_matches_setitem():
    game = games.read_from_file("mixed_behavior_game.efg")
    node = _p1_node(game)
    sparse_profile = game.mixed_behavior_profile()
    sparse_profile.set_mixed_action(node, {"U1": 1}, sparse=True)
    setitem_profile = game.mixed_behavior_profile()
    setitem_profile[node] = {"U1": 1}
    assert sparse_profile[node] == setitem_profile[node]


def test_set_mixed_action_defaults_to_requiring_every_label():
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.mixed_behavior_profile()
    node = _p1_node(game)
    with pytest.raises(ValueError, match="exactly one weight"):
        profile.set_mixed_action(node, {"U1": 1})


@pytest.mark.parametrize("sparse", [False, True])
def test_setitem_and_set_mixed_action_reject_unknown_action_label(sparse: bool):
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.mixed_behavior_profile()
    node = _p1_node(game)
    with pytest.raises(ValueError, match="not an action label"):
        profile.set_mixed_action(node, {"not-an-action": 1}, sparse=sparse)
    with pytest.raises(ValueError, match="not an action label"):
        profile[node] = {"not-an-action": 1}


def test_behavior_setitem_empty_distribution_is_all_zero_error():
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.mixed_behavior_profile()
    node = _p1_node(game)
    with pytest.raises(ValueError, match="zero"):
        profile[node] = {}


@pytest.mark.parametrize("sparse", [False, True])
def test_setitem_and_set_mixed_action_reject_non_mapping(sparse: bool):
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.mixed_behavior_profile()
    node = _p1_node(game)
    with pytest.raises(TypeError, match="Mapping"):
        profile.set_mixed_action(node, [1, 0], sparse=sparse)
    with pytest.raises(TypeError, match="Mapping"):
        profile[node] = [1, 0]


@pytest.mark.parametrize("sparse", [False, True])
def test_setitem_and_set_mixed_action_reject_uncoercible_weight(sparse: bool):
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.mixed_behavior_profile()
    node = _p1_node(game)
    full_distribution = {"U1": "abc", "D1": 0}
    with pytest.raises(ValueError, match="convert"):
        profile.set_mixed_action(node, full_distribution, sparse=sparse)
    with pytest.raises(ValueError, match="convert"):
        profile[node] = full_distribution


def test_behavior_setitem_sparse_rejects_negative_weight():
    """Negativity is checked even for weights given under a sparse distribution."""
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.mixed_behavior_profile()
    node = _p1_node(game)
    with pytest.raises(ValueError, match="negative"):
        profile[node] = {"U1": -1}


@pytest.mark.parametrize("sparse", [False, True])
def test_behavior_indexing_rejects_infoset_object(sparse: bool):
    """MixedBehaviorProfile's indexing is Node-only; an Infoset object is rejected."""
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.mixed_behavior_profile()
    infoset = next(iter(game.players["Player 1"].infosets))
    with pytest.raises(TypeError):
        profile[infoset]
    with pytest.raises(TypeError):
        profile[infoset] = {"U1": 1}
    with pytest.raises(TypeError):
        profile.set_mixed_action(infoset, {"U1": 1}, sparse=sparse)


@pytest.mark.parametrize("rational_flag", [False, True])
def test_mixed_action_and_behavior_are_frozen_snapshots(rational_flag: bool):
    """MixedAction/MixedBehavior are snapshots taken at retrieval time: they do not
    reflect later mutations to the profile they came from.
    """
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.mixed_behavior_profile(rational=rational_flag)
    node = _p1_node(game)
    action_before = profile[node]
    behavior_before = profile["Player 1"]
    profile[node] = {"U1": 1, "D1": 0}
    assert dict(action_before) == {"U1": 0.5, "D1": 0.5}
    assert dict(behavior_before[node]) == {"U1": 0.5, "D1": 0.5}
    assert dict(profile[node]) == {"U1": 1, "D1": 0}


@pytest.mark.parametrize("rational_flag", [False, True])
def test_behavior_copy_mutating_copy_does_not_affect_original(rational_flag: bool):
    game = games.read_from_file("mixed_behavior_game.efg")
    original = game.mixed_behavior_profile(rational=rational_flag)
    node = _p1_node(game)
    original_before = dict(original[node])
    copy = original.copy()
    copy[node] = {"U1": 1, "D1": 0}
    assert dict(original[node]) == original_before
    assert dict(copy[node]) == {"U1": 1, "D1": 0}


@pytest.mark.parametrize("rational_flag", [False, True])
def test_behavior_copy_mutating_original_does_not_affect_copy(rational_flag: bool):
    game = games.read_from_file("mixed_behavior_game.efg")
    original = game.mixed_behavior_profile(rational=rational_flag)
    node = _p1_node(game)
    copy = original.copy()
    copy_before = dict(copy[node])
    original[node] = {"U1": 1, "D1": 0}
    assert dict(copy[node]) == copy_before
    assert dict(original[node]) == {"U1": 1, "D1": 0}


@pytest.mark.parametrize(
    "game,path,realiz_prob,rational_flag",
    [
        (games.read_from_file("mixed_behavior_game.efg"), [], "1", True),
        (games.read_from_file("mixed_behavior_game.efg"), ["U1"], "1/2", True),
        (games.read_from_file("mixed_behavior_game.efg"), ["U1", "U2"], "1/4", True),
        (games.read_from_file("mixed_behavior_game.efg"), ["U1", "U2", "U3"], "1/8", True),
        (games.read_from_file("mixed_behavior_game.efg"), ["U1", "U2", "D3"], "1/8", True),
        (games.read_from_file("mixed_behavior_game.efg"), ["U1", "D2"], "1/4", True),
        (games.read_from_file("mixed_behavior_game.efg"), ["U1", "D2", "U3"], "1/8", True),
        (games.read_from_file("mixed_behavior_game.efg"), ["U1", "D2", "D3"], "1/8", True),
        (games.read_from_file("mixed_behavior_game.efg"), ["D1"], "1/2", True),
        (games.read_from_file("mixed_behavior_game.efg"), ["D1", "U2"], "1/4", True),
        (games.read_from_file("mixed_behavior_game.efg"), ["D1", "U2", "U3"], "1/8", True),
        (games.read_from_file("mixed_behavior_game.efg"), ["D1", "U2", "D3"], "1/8", True),
        (games.read_from_file("mixed_behavior_game.efg"), ["D1", "D2"], "1/4", True),
        (games.read_from_file("mixed_behavior_game.efg"), ["D1", "D2", "U3"], "1/8", True),
        (games.read_from_file("mixed_behavior_game.efg"), ["D1", "D2", "D3"], "1/8", True),
        (games.read_from_file("mixed_behavior_game.efg"), [], 1.0, False),
        (games.read_from_file("mixed_behavior_game.efg"), ["U1"], 0.5, False),
        (games.read_from_file("mixed_behavior_game.efg"), ["U1", "U2"], 0.25, False),
        (games.read_from_file("mixed_behavior_game.efg"), ["U1", "U2", "U3"], 0.125, False),
        (games.read_from_file("mixed_behavior_game.efg"), ["U1", "U2", "D3"], 0.125, False),
        (games.read_from_file("mixed_behavior_game.efg"), ["U1", "D2"], 0.25, False),
        (games.read_from_file("mixed_behavior_game.efg"), ["U1", "D2", "U3"], 0.125, False),
        (games.read_from_file("mixed_behavior_game.efg"), ["U1", "D2", "D3"], 0.125, False),
        (games.read_from_file("mixed_behavior_game.efg"), ["D1"], 0.5, False),
        (games.read_from_file("mixed_behavior_game.efg"), ["D1", "U2"], 0.25, False),
        (games.read_from_file("mixed_behavior_game.efg"), ["D1", "U2", "U3"], 0.125, False),
        (games.read_from_file("mixed_behavior_game.efg"), ["D1", "U2", "D3"], 0.125, False),
        (games.read_from_file("mixed_behavior_game.efg"), ["D1", "D2"], 0.25, False),
        (games.read_from_file("mixed_behavior_game.efg"), ["D1", "D2", "U3"], 0.125, False),
        (games.read_from_file("mixed_behavior_game.efg"), ["D1", "D2", "D3"], 0.125, False),
        (games.create_stripped_down_poker_efg(), [], "1", True),
        (games.create_stripped_down_poker_efg(), ["King"], "1/2", True),
        (games.create_stripped_down_poker_efg(), ["King", "Bet"], "1/4", True),
        (games.create_stripped_down_poker_efg(), ["King", "Bet", "Call"], "1/8", True),
        (games.create_stripped_down_poker_efg(), ["King", "Bet", "Fold"], "1/8", True),
        (games.create_stripped_down_poker_efg(), ["King", "Fold"], "1/4", True),
        (games.create_stripped_down_poker_efg(), ["Queen"], "1/2", True),
        (games.create_stripped_down_poker_efg(), ["Queen", "Bet"], "1/4", True),
        (games.create_stripped_down_poker_efg(), ["Queen", "Bet", "Call"], "1/8", True),
        (games.create_stripped_down_poker_efg(), ["Queen", "Bet", "Fold"], "1/8", True),
        (games.create_stripped_down_poker_efg(), ["Queen", "Fold"], "1/4", True),
        (games.create_stripped_down_poker_efg(), [], 1.0, False),
        (games.create_stripped_down_poker_efg(), ["King"], 0.5, False),
        (games.create_stripped_down_poker_efg(), ["King", "Bet"], 0.25, False),
        (games.create_stripped_down_poker_efg(), ["King", "Bet", "Call"], 0.125, False),
        (games.create_stripped_down_poker_efg(), ["King", "Bet", "Fold"], 0.125, False),
        (games.create_stripped_down_poker_efg(), ["King", "Fold"], 0.25, False),
        (games.create_stripped_down_poker_efg(), ["Queen"], 0.5, False),
        (games.create_stripped_down_poker_efg(), ["Queen", "Bet"], 0.25, False),
        (games.create_stripped_down_poker_efg(), ["Queen", "Bet", "Call"], 0.125, False),
        (games.create_stripped_down_poker_efg(), ["Queen", "Bet", "Fold"], 0.125, False),
        (games.create_stripped_down_poker_efg(), ["Queen", "Fold"], 0.25, False),
    ],
)
def test_realiz_prob_nodes_reference(
    game: gbt.Game, path: list[str], realiz_prob: str | float, rational_flag: bool
):
    # nodes have no labels, so each node is reached by walking the action-label
    # path from the root (an empty path is the root itself)
    profile = game.mixed_behavior_profile(rational=rational_flag)
    realiz_prob = gbt.Rational(realiz_prob) if rational_flag else realiz_prob
    node = game.root
    for action_label in path:
        node = node.children[action_label]
    assert profile.realiz_prob(node) == realiz_prob


@pytest.mark.parametrize(
    "game,player_label,infoset_label,prob,rational_flag",
    [
        (games.read_from_file("mixed_behavior_game.efg"), "Player 1", "Infoset 1:1", 1.0, False),
        (games.read_from_file("mixed_behavior_game.efg"), "Player 2", "Infoset 2:1", 1.0, False),
        (games.read_from_file("mixed_behavior_game.efg"), "Player 3", "Infoset 3:1", 1.0, False),
        (games.read_from_file("mixed_behavior_game.efg"), "Player 1", "Infoset 1:1", "1", True),
        (games.read_from_file("mixed_behavior_game.efg"), "Player 2", "Infoset 2:1", "1", True),
        (games.read_from_file("mixed_behavior_game.efg"), "Player 3", "Infoset 3:1", "1", True),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has King", 0.5, False),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has Queen", 0.5, False),
        (games.create_stripped_down_poker_efg(), "Bob", "Bob's response", 0.5, False),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has King", "1/2", True),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has Queen", "1/2", True),
        (games.create_stripped_down_poker_efg(), "Bob", "Bob's response", "1/2", True),
    ],
)
def test_infoset_prob_reference(
    game: gbt.Game, player_label: str, infoset_label: str, prob: str | float, rational_flag: bool
):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    ip = profile.infoset_prob(game.players[player_label].infosets[infoset_label])
    assert ip == (gbt.Rational(prob) if rational_flag else prob)


@pytest.mark.parametrize(
    "game,label,prob,rational_flag",
    [
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 1:1", 1.0, False),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 2:1", 1.0, False),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 3:1", 1.0, False),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 1:1", "1", True),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 2:1", "1", True),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 3:1", "1", True),
        (games.create_stripped_down_poker_efg(), "Alice has King", 0.5, False),
        (games.create_stripped_down_poker_efg(), "Alice has Queen", 0.5, False),
        (games.create_stripped_down_poker_efg(), "Bob's response", 0.5, False),
        (games.create_stripped_down_poker_efg(), "Alice has King", "1/2", True),
        (games.create_stripped_down_poker_efg(), "Alice has Queen", "1/2", True),
        (games.create_stripped_down_poker_efg(), "Bob's response", "1/2", True),
    ],
)
def test_infoset_prob_by_label_reference(
    game: gbt.Game, label: str, prob: str | float, rational_flag: bool
):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    assert profile.infoset_prob(label) == (gbt.Rational(prob) if rational_flag else prob)


@pytest.mark.parametrize(
    "game,infoset_label,prob,rational_flag",
    [
        # P1 infoset 1 is absent-minded (root + one reentry)
        (games.read_from_file("noPR-AM-driver-one-player.efg"), "Absent-minded", 1.0, False),
        (games.read_from_file("noPR-AM-driver-one-player.efg"), "Second", 0.5, False),
        (games.read_from_file("noPR-AM-driver-one-player.efg"), "Third", 0.125, False),
        (games.read_from_file("noPR-AM-driver-one-player.efg"), "Absent-minded", "1", True),
        (games.read_from_file("noPR-AM-driver-one-player.efg"), "Second", "1/2", True),
        (games.read_from_file("noPR-AM-driver-one-player.efg"), "Third", "1/8", True),
        # P1 infoset 1 has 3 members (root + both children are reentries)
        (games.read_from_file("noPR-action-AM.efg"), "Absent-minded", 1.0, False),
        (games.read_from_file("noPR-action-AM.efg"), "Response 1", 0.25, False),
        (games.read_from_file("noPR-action-AM.efg"), "Response 2", 0.25, False),
        (games.read_from_file("noPR-action-AM.efg"), "Response 3", 0.25, False),
        (games.read_from_file("noPR-action-AM.efg"), "Response 4", 0.25, False),
        (games.read_from_file("noPR-action-AM.efg"), "Absent-minded", "1", True),
        (games.read_from_file("noPR-action-AM.efg"), "Response 1", "1/4", True),
        (games.read_from_file("noPR-action-AM.efg"), "Response 2", "1/4", True),
        (games.read_from_file("noPR-action-AM.efg"), "Response 3", "1/4", True),
        (games.read_from_file("noPR-action-AM.efg"), "Response 4", "1/4", True),
        # # P1 infoset 1 has 3 members (3-node chain with the last member being
        # # behavioral-strategy-reachable, but not pure-strategy-reachable)
        (games.read_from_file("noPR-action-AM-three-chain.efg"), "Absent-minded", 1.0, False),
        (games.read_from_file("noPR-action-AM-three-chain.efg"), "Second", 0.5, False),
        (games.read_from_file("noPR-action-AM-three-chain.efg"), "Player 2", 0.0625, False),
        (games.read_from_file("noPR-action-AM-three-chain.efg"), "Absent-minded", "1", True),
        (games.read_from_file("noPR-action-AM-three-chain.efg"), "Second", "1/2", True),
        (games.read_from_file("noPR-action-AM-three-chain.efg"), "Player 2", "1/16", True),
    ],
)
def test_absent_minded_infoset_prob(
    game: gbt.Game, infoset_label: str, prob: str | float, rational_flag: bool
):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    ip = profile.infoset_prob(game.infosets[infoset_label])
    assert ip == (gbt.Rational(prob) if rational_flag else prob)


@pytest.mark.parametrize("rational_flag", [False, True])
def test_nature_rooted_game_root_reached_with_certainty(rational_flag: bool):
    """The chance root infoset is reached with probability one."""
    game = gbt.catalog.load("journals/geb/gilboa1997/fig2")
    profile = game.mixed_behavior_profile(rational=rational_flag)
    one = gbt.Rational(1) if rational_flag else 1.0
    assert profile.realiz_prob(game.root) == one
    assert profile.infoset_prob(game.root.infoset) == one


@pytest.mark.parametrize(
    "game,player_label,infoset_label,payoff,rational_flag",
    [
        (games.read_from_file("mixed_behavior_game.efg"), "Player 1", "Infoset 1:1", 3.0, False),
        (games.read_from_file("mixed_behavior_game.efg"), "Player 2", "Infoset 2:1", 3.0, False),
        (games.read_from_file("mixed_behavior_game.efg"), "Player 3", "Infoset 3:1", 3.25, False),
        (games.read_from_file("mixed_behavior_game.efg"), "Player 1", "Infoset 1:1", "3", True),
        (games.read_from_file("mixed_behavior_game.efg"), "Player 2", "Infoset 2:1", "3", True),
        (games.read_from_file("mixed_behavior_game.efg"), "Player 3", "Infoset 3:1", "13/4", True),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has King", 0.25, False),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has Queen", -0.75, False),
        (games.create_stripped_down_poker_efg(), "Bob", "Bob's response", -0.5, False),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has King", "1/4", True),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has Queen", "-3/4", True),
        (games.create_stripped_down_poker_efg(), "Bob", "Bob's response", "-1/2", True),
    ],
)
def test_infoset_payoff_reference(
    game: gbt.Game, player_label: str, infoset_label: str, payoff: str | float, rational_flag: bool
):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    iv = profile.infoset_value(game.players[player_label].infosets[infoset_label])
    assert iv == (gbt.Rational(payoff) if rational_flag else payoff)


@pytest.mark.parametrize(
    "game,label,payoff,rational_flag",
    [
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 1:1", 3.0, False),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 2:1", 3.0, False),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 3:1", 3.25, False),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 1:1", "3", True),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 2:1", "3", True),
        (games.read_from_file("mixed_behavior_game.efg"), "Infoset 3:1", "13/4", True),
        (games.create_stripped_down_poker_efg(), "Alice has King", 0.25, False),
        (games.create_stripped_down_poker_efg(), "Alice has Queen", -0.75, False),
        (games.create_stripped_down_poker_efg(), "Bob's response", -0.5, False),
        (games.create_stripped_down_poker_efg(), "Alice has King", "1/4", True),
        (games.create_stripped_down_poker_efg(), "Alice has Queen", "-3/4", True),
        (games.create_stripped_down_poker_efg(), "Bob's response", "-1/2", True),
    ],
)
def test_infoset_payoff_by_label_reference(
    game: gbt.Game, label: str, payoff: str | float, rational_flag: bool
):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    assert profile.infoset_value(label) == (gbt.Rational(payoff) if rational_flag else payoff)


@pytest.mark.parametrize(
    "game,player_label,infoset_label,action_label,payoff,rational_flag",
    [
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 1",
            "Infoset 1:1",
            "U1",
            3.0,
            False,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 1",
            "Infoset 1:1",
            "D1",
            3.0,
            False,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 2",
            "Infoset 2:1",
            "U2",
            3.0,
            False,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 2",
            "Infoset 2:1",
            "D2",
            3.0,
            False,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 3",
            "Infoset 3:1",
            "U3",
            3.5,
            False,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 3",
            "Infoset 3:1",
            "D3",
            3.0,
            False,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 3",
            "Infoset 3:1",
            "D3",
            3.0,
            False,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 1",
            "Infoset 1:1",
            "U1",
            "3/1",
            True,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 1",
            "Infoset 1:1",
            "D1",
            "3/1",
            True,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 2",
            "Infoset 2:1",
            "U2",
            "3/1",
            True,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 2",
            "Infoset 2:1",
            "D2",
            "3/1",
            True,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 3",
            "Infoset 3:1",
            "U3",
            "7/2",
            True,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 3",
            "Infoset 3:1",
            "D3",
            "3/1",
            True,
        ),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has King", "Bet", 1.5, False),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has King", "Fold", -1, False),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has Queen", "Bet", -0.5, False),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has Queen", "Fold", -1, False),
        (games.create_stripped_down_poker_efg(), "Bob", "Bob's response", "Call", 0, False),
        (games.create_stripped_down_poker_efg(), "Bob", "Bob's response", "Fold", -1, False),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has King", "Bet", "3/2", True),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has King", "Fold", -1, True),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has Queen", "Bet", "-1/2", True),
        (games.create_stripped_down_poker_efg(), "Alice", "Alice has Queen", "Fold", -1, True),
        (games.create_stripped_down_poker_efg(), "Bob", "Bob's response", "Call", 0, True),
        (games.create_stripped_down_poker_efg(), "Bob", "Bob's response", "Fold", -1, True),
    ],
)
def test_action_payoff_reference(
    game: gbt.Game,
    player_label: str,
    infoset_label: str,
    action_label: str,
    payoff: str | float,
    rational_flag: bool,
):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    av = profile.action_value(
        game.players[player_label].infosets[infoset_label].actions[action_label]
    )
    assert av == (gbt.Rational(payoff) if rational_flag else payoff)


@pytest.mark.parametrize(
    "game,label,payoff,rational_flag",
    [
        (games.read_from_file("mixed_behavior_game.efg"), "U1", 3.0, False),
        (games.read_from_file("mixed_behavior_game.efg"), "D1", 3.0, False),
        (games.read_from_file("mixed_behavior_game.efg"), "U2", 3.0, False),
        (games.read_from_file("mixed_behavior_game.efg"), "D2", 3.0, False),
        (games.read_from_file("mixed_behavior_game.efg"), "U3", 3.5, False),
        (games.read_from_file("mixed_behavior_game.efg"), "D3", 3.0, False),
        (games.read_from_file("mixed_behavior_game.efg"), "U1", "3", True),
        (games.read_from_file("mixed_behavior_game.efg"), "D1", "3", True),
        (games.read_from_file("mixed_behavior_game.efg"), "U2", "3", True),
        (games.read_from_file("mixed_behavior_game.efg"), "D2", "3", True),
        (games.read_from_file("mixed_behavior_game.efg"), "U3", "7/2", True),
        (games.read_from_file("mixed_behavior_game.efg"), "D3", "3", True),
        (games.create_stripped_down_poker_efg(), "Call", 0, False),
        (games.create_stripped_down_poker_efg(), "Call", "0", True),
    ],
)
def test_action_value_by_label_reference(
    game: gbt.Game, label: str, payoff: str | float, rational_flag: bool
):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    assert profile.action_value(label) == (gbt.Rational(payoff) if rational_flag else payoff)


@pytest.mark.parametrize(
    "game,rational_flag",
    [
        (games.read_from_file("mixed_behavior_game.efg"), False),
        (games.read_from_file("mixed_behavior_game.efg"), True),
        (games.create_stripped_down_poker_efg(), False),
        (games.create_stripped_down_poker_efg(), True),
        (games.create_kuhn_poker_efg(), False),
        (games.create_kuhn_poker_efg(), True),
    ],
)
def test_action_regret_consistency(game: gbt.Game, rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    for player in game.players:
        for infoset in player.infosets:
            for action in infoset.actions:
                assert profile.action_regret(action) == max(
                    profile.action_value(a) for a in infoset.actions
                ) - profile.action_value(action)


@pytest.mark.parametrize(
    "game,rational_flag",
    [
        (games.read_from_file("mixed_behavior_game.efg"), False),
        (games.read_from_file("mixed_behavior_game.efg"), True),
        (games.create_stripped_down_poker_efg(), False),
        (games.create_stripped_down_poker_efg(), True),
        (games.create_kuhn_poker_efg(), False),
        (games.create_kuhn_poker_efg(), True),
    ],
)
def test_infoset_regret_consistency(game: gbt.Game, rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    for player in game.players:
        for infoset in player.infosets:
            assert profile.infoset_regret(infoset) == max(
                profile.action_value(a) for a in infoset.actions
            ) - profile.infoset_value(infoset)


@pytest.mark.parametrize(
    "game,rational_flag",
    [
        (games.read_from_file("mixed_behavior_game.efg"), False),
        (games.read_from_file("mixed_behavior_game.efg"), True),
        (games.create_stripped_down_poker_efg(), False),
        (games.create_stripped_down_poker_efg(), True),
        (games.create_kuhn_poker_efg(), False),
        (games.create_kuhn_poker_efg(), True),
        (games.read_from_file("3_player.efg"), False),
        (games.read_from_file("3_player.efg"), True),
    ],
)
def test_max_regret_consistency(game: gbt.Game, rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    assert profile.max_regret() == profile.as_strategy().max_regret()


@pytest.mark.parametrize(
    "game,rational_flag",
    [
        (games.read_from_file("mixed_behavior_game.efg"), False),
        (games.read_from_file("mixed_behavior_game.efg"), True),
        (games.create_stripped_down_poker_efg(), False),
        (games.create_stripped_down_poker_efg(), True),
        (games.create_kuhn_poker_efg(), False),
        (games.create_kuhn_poker_efg(), True),
    ],
)
def test_agent_max_regret_consistency(game: gbt.Game, rational_flag: bool):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    assert profile.agent_max_regret() == max(
        [profile.infoset_regret(infoset) for infoset in game.infosets]
    )


@pytest.mark.parametrize(
    "game,player_label,infoset_label,action_label,action_probs,rational_flag,tol,value",
    [
        # uniform
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 1",
            "Infoset 1:1",
            "U1",
            None,
            False,
            TOL,
            0,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 1",
            "Infoset 1:1",
            "D1",
            None,
            False,
            TOL,
            0,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 2",
            "Infoset 2:1",
            "U2",
            None,
            False,
            TOL,
            0,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 2",
            "Infoset 2:1",
            "D2",
            None,
            False,
            TOL,
            0,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 3",
            "Infoset 3:1",
            "U3",
            None,
            False,
            TOL,
            0,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 3",
            "Infoset 3:1",
            "D3",
            None,
            False,
            TOL,
            0.5,
        ),  # 3.5 - 3
        # U1 U2 U3
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 1",
            "Infoset 1:1",
            "U1",
            [1, 0, 1, 0, 1, 0],
            False,
            TOL,
            0,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 1",
            "Infoset 1:1",
            "U1",
            [1, 0, 1, 0, 1, 0],
            True,
            ZERO,
            0,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 1",
            "Infoset 1:1",
            "D1",
            [1, 0, 1, 0, 1, 0],
            False,
            TOL,
            9,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 1",
            "Infoset 1:1",
            "D1",
            [1, 0, 1, 0, 1, 0],
            True,
            ZERO,
            9,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 2",
            "Infoset 2:1",
            "U2",
            [1, 0, 1, 0, 1, 0],
            False,
            TOL,
            0,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 2",
            "Infoset 2:1",
            "U2",
            [1, 0, 1, 0, 1, 0],
            True,
            ZERO,
            0,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 2",
            "Infoset 2:1",
            "D2",
            [1, 0, 1, 0, 1, 0],
            False,
            TOL,
            8,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 2",
            "Infoset 2:1",
            "D2",
            [1, 0, 1, 0, 1, 0],
            True,
            ZERO,
            8,
        ),
        # Mixed Nash equilibrium
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 1",
            "Infoset 1:1",
            "U1",
            ["2/5", "3/5", "1/2", "1/2", "1/3", "2/3"],
            True,
            ZERO,
            0,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 1",
            "Infoset 1:1",
            "D1",
            ["2/5", "3/5", "1/2", "1/2", "1/3", "2/3"],
            True,
            ZERO,
            0,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 2",
            "Infoset 2:1",
            "U2",
            ["2/5", "3/5", "1/2", "1/2", "1/3", "2/3"],
            True,
            ZERO,
            0,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 2",
            "Infoset 2:1",
            "D2",
            ["2/5", "3/5", "1/2", "1/2", "1/3", "2/3"],
            True,
            ZERO,
            0,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 3",
            "Infoset 3:1",
            "U3",
            ["2/5", "3/5", "1/2", "1/2", "1/3", "2/3"],
            True,
            ZERO,
            0,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            "Player 3",
            "Infoset 3:1",
            "D3",
            ["2/5", "3/5", "1/2", "1/2", "1/3", "2/3"],
            True,
            ZERO,
            0,
        ),
        # uniform
        (
            games.create_stripped_down_poker_efg(),
            "Alice",
            "Alice has King",
            "Bet",
            None,
            False,
            TOL,
            0,
        ),
        (
            games.create_stripped_down_poker_efg(),
            "Alice",
            "Alice has King",
            "Fold",
            None,
            False,
            TOL,
            2.5,
        ),  # 1.5 - (-1)
        (
            games.create_stripped_down_poker_efg(),
            "Alice",
            "Alice has Queen",
            "Bet",
            None,
            False,
            TOL,
            0,
        ),
        (
            games.create_stripped_down_poker_efg(),
            "Alice",
            "Alice has Queen",
            "Fold",
            None,
            False,
            TOL,
            0.5,
        ),  # -0.5 - (-1)
        (
            games.create_stripped_down_poker_efg(),
            "Bob",
            "Bob's response",
            "Call",
            None,
            False,
            TOL,
            0,
        ),
        (
            games.create_stripped_down_poker_efg(),
            "Bob",
            "Bob's response",
            "Fold",
            None,
            False,
            TOL,
            1,
        ),  # -0 - (-1)
        # mixed Nash equilibrium
        (
            games.create_stripped_down_poker_efg(),
            "Alice",
            "Alice has King",
            "Bet",
            ["1", "0", "1/3", "2/3", "2/3", "1/3"],
            True,
            ZERO,
            0,
        ),
        (
            games.create_stripped_down_poker_efg(),
            "Alice",
            "Alice has King",
            "Fold",
            ["1", "0", "1/3", "2/3", "2/3", "1/3"],
            True,
            ZERO,
            "8/3",
        ),  # (2/3*2 + 1/3*1) - (-1)
    ],
)
def test_action_regret_reference(
    game: gbt.Game,
    player_label: str,
    infoset_label: str,
    action_label: str,
    action_probs: None | list,
    rational_flag: bool,
    tol: gbt.Rational | float,
    value: str | float,
):
    action = game.players[player_label].infosets[infoset_label].actions[action_label]
    profile = game.mixed_behavior_profile(rational=rational_flag)
    if action_probs:
        _set_action_probs(profile, action_probs, rational_flag)
    regret = profile.action_regret(action)
    value = gbt.Rational(value) if rational_flag else value
    assert abs(regret - value) <= tol


@pytest.mark.parametrize(
    "game,rational_flag",
    [
        (games.read_from_file("mixed_behavior_game.efg"), False),
        (games.read_from_file("mixed_behavior_game.efg"), True),
        (games.create_stripped_down_poker_efg(), False),
        (games.create_stripped_down_poker_efg(), True),
    ],
)
def test_martingale_property_of_node_value(game: gbt.Game, rational_flag: bool):
    """Loops over all nodes and for non-chance, non-terminal nodes, this checks that the node
    value is equal to the expected value of the node values of its children, using the normalized
    realization probabilities of those children
    """
    profile = game.mixed_behavior_profile(rational=rational_flag)
    for node in game.nodes:
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
    [
        (games.read_from_file("mixed_behavior_game.efg"), False),
        (games.read_from_file("mixed_behavior_game.efg"), True),
        (games.create_stripped_down_poker_efg(), False),
        (games.create_stripped_down_poker_efg(), True),
    ],
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
        (games.read_from_file("mixed_behavior_game.efg"), None, True, "1/16"),
        (games.read_from_file("mixed_behavior_game.efg"), None, False, 0.0625),
        # four pure Nash equilibria:
        (
            games.read_from_file("mixed_behavior_game.efg"),
            [1.0, 0.0, 1.0, 0.0, 1.0, 0.0],
            False,
            0,
        ),  # U1 U2 U3
        (games.read_from_file("mixed_behavior_game.efg"), ["1", "0", "1", "0", "1", "0"], True, 0),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            ["1", "0", "0", "1", "0", "1"],
            True,
            0,
        ),  # U1 D2 D3
        (games.read_from_file("mixed_behavior_game.efg"), [1.0, 0.0, 0.0, 1.0, 0, 1.0], False, 0),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            ["0", "1", "1", "0", "0", "1"],
            True,
            0,
        ),  # D1 U2 D3
        (games.read_from_file("mixed_behavior_game.efg"), [0.0, 1.0, 1.0, 0.0, 0, 1.0], False, 0),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            ["0", "1", "0", "1", "1", "0"],
            True,
            0,
        ),  # D1 D2 U3
        (games.read_from_file("mixed_behavior_game.efg"), [0.0, 1.0, 0.0, 1.0, 1.0, 0], False, 0),
        # mixed Nash equilibrium (only rational tested):
        (
            games.read_from_file("mixed_behavior_game.efg"),
            ["2/5", "3/5", "1/2", "1/2", "1/3", "2/3"],
            True,
            0,
        ),
        # non-Nash pure profile: D1 D2 D3
        (
            games.read_from_file("mixed_behavior_game.efg"),
            [0.0, 1.0, 0.0, 1.0, 0.0, 1.0],
            False,
            29.0,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            ["0", "1", "0", "1", "0", "1"],
            True,
            "29",
        ),
        # uniform (non-Nash):
        (games.create_stripped_down_poker_efg(), None, True, "15/8"),
        (games.create_stripped_down_poker_efg(), None, False, 1.875),
        # mixed Nash equilibrium (only rational tested):
        (games.create_stripped_down_poker_efg(), ["1", "0", "1/3", "2/3", "2/3", "1/3"], True, 0),
        # non-Nash pure profile:
        # Raise at 1:1, Raise at 1:2, Meet at 2:1
        (games.create_stripped_down_poker_efg(), ["1", "0", "1", "0", "1", "0"], True, 1),
        (games.create_stripped_down_poker_efg(), [1.0, 0.0, 1.0, 0.0, 1.0, 0.0], False, 1.0),
    ],
)
def test_agent_liap_value_reference(
    game: gbt.Game, action_probs: None | list, rational_flag: bool, expected_value: str | float
):
    """Tests agent_liap_value under profile given by action_probs
    (which will be uniform if action_probs is None)
    """
    profile = game.mixed_behavior_profile(rational=rational_flag)
    if action_probs:
        _set_action_probs(profile, action_probs, rational_flag)
    assert profile.agent_liap_value() == (
        gbt.Rational(expected_value) if rational_flag else expected_value
    )


@pytest.mark.parametrize(
    "game,action_probs,rational_flag,max_regret,agent_max_regret,liap_value,agent_liap_value",
    [
        # uniform (non-Nash):
        (
            games.read_from_file("mixed_behavior_game.efg"),
            None,
            True,
            "1/4",
            "1/4",
            "1/16",
            "1/16",
        ),
        (games.read_from_file("mixed_behavior_game.efg"), None, False, 0.25, 0.25, 0.0625, 0.0625),
        (gbt.catalog.load("books/myerson1991/fig4_2"), [0, 1, 0, 1, 1, 0], True, 1, 0, 1, 0),
    ],
)
def test_agent_max_regret_versus_non_agent(
    game: gbt.Game,
    action_probs: None | list,
    rational_flag: bool,
    max_regret: str | float,
    agent_max_regret: str | float,
    agent_liap_value: str | float,
    liap_value: str | float,
):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    if action_probs:
        _set_action_probs(profile, action_probs, rational_flag)
    assert profile.max_regret() == (gbt.Rational(max_regret) if rational_flag else max_regret)
    assert profile.agent_max_regret() == (
        gbt.Rational(agent_max_regret) if rational_flag else agent_max_regret
    )
    assert profile.liap_value() == (gbt.Rational(liap_value) if rational_flag else liap_value)
    assert profile.agent_liap_value() == (
        gbt.Rational(agent_liap_value) if rational_flag else agent_liap_value
    )


@pytest.mark.parametrize(
    "game,tol,probs,path,value,rational_flag",
    [
        (
            games.read_from_file("mixed_behavior_game.efg"),
            TOL,
            [0.8, 0.2, 0.4, 0.6, 0.0, 1.0],
            [],
            1.0,
            False,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            TOL,
            [0.8, 0.2, 0.4, 0.6, 0.0, 1.0],
            ["U1"],
            0.8,
            False,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            TOL,
            [0.8, 0.2, 0.4, 0.6, 0.0, 1.0],
            ["D1"],
            0.2,
            False,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            TOL,
            [0.8, 0.2, 0.4, 0.6, 0.0, 1.0],
            ["U1", "U2"],
            0.32,
            False,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            TOL,
            [0.8, 0.2, 0.4, 0.6, 0.0, 1.0],
            ["U1", "D2"],
            0.48,
            False,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            ZERO,
            ["4/5", "1/5", "2/5", "3/5", "0", "1"],
            [],
            "1",
            True,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            ZERO,
            ["4/5", "1/5", "2/5", "3/5", "0", "1"],
            ["U1"],
            "4/5",
            True,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            ZERO,
            ["4/5", "1/5", "2/5", "3/5", "0", "1"],
            ["D1"],
            "1/5",
            True,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            ZERO,
            ["4/5", "1/5", "2/5", "3/5", "0", "1"],
            ["U1", "U2"],
            "8/25",
            True,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            ZERO,
            ["4/5", "1/5", "2/5", "3/5", "0", "1"],
            ["U1", "D2"],
            "12/25",
            True,
        ),
        (
            games.create_stripped_down_poker_efg(),
            ZERO,
            ["4/5", "1/5", "2/5", "3/5", "0", "1"],
            ["King"],
            "1",
            True,
        ),
        (
            games.create_stripped_down_poker_efg(),
            ZERO,
            ["4/5", "1/5", "2/5", "3/5", "0", "1"],
            ["Queen"],
            "1",
            True,
        ),
        (
            games.create_stripped_down_poker_efg(),
            ZERO,
            ["4/5", "1/5", "2/5", "3/5", "0", "1"],
            ["King", "Bet"],
            "2/3",
            True,
        ),
        (
            games.create_stripped_down_poker_efg(),
            ZERO,
            ["4/5", "1/5", "2/5", "3/5", "0", "1"],
            ["Queen", "Bet"],
            "1/3",
            True,
        ),
        (
            games.create_stripped_down_poker_efg(),
            ZERO,
            ["1", "0", "2/5", "3/5", "0", "1"],
            ["King", "Bet"],
            "5/7",
            True,
        ),
        (
            games.create_stripped_down_poker_efg(),
            ZERO,
            ["1", "0", "2/5", "3/5", "0", "1"],
            ["Queen", "Bet"],
            "2/7",
            True,
        ),
        # Information set I1 = {root, reentry_node}; reentry_node is reached by ["1", "1"].
        # The upper frontier of I1 is {root}, so the conditioning event has probability
        # realiz(root) = 1 for every profile, and beliefs are normalised by it:
        # belief(root) = 1, belief(reentry_node) = realiz(reentry_node).
        (
            games.read_from_file("noPR-AM-driver-one-player.efg"),
            ZERO, ["1/2", "1/2", "1/2", "1/2", "1/2", "1/2"], [], "1", True,
        ),
        (
            games.read_from_file("noPR-AM-driver-one-player.efg"),
            ZERO, ["1/2", "1/2", "1/2", "1/2", "1/2", "1/2"], ["1", "1"], "1/4", True,
        ),
        (
            games.read_from_file("noPR-AM-driver-one-player.efg"),
            TOL, [0.5, 0.5, 0.5, 0.5, 0.5, 0.5], [], 1.0, False,
        ),
        (
            games.read_from_file("noPR-AM-driver-one-player.efg"),
            TOL, [0.5, 0.5, 0.5, 0.5, 0.5, 0.5], ["1", "1"], 0.25, False,
        ),
        # asymmetric: p(I1,1)=2/3, p(I2,1)=3/4; realiz(reentry_node)=1/2
        (
            games.read_from_file("noPR-AM-driver-one-player.efg"),
            ZERO, ["2/3", "1/3", "3/4", "1/4", "1/2", "1/2"], [], "1", True,
        ),
        (
            games.read_from_file("noPR-AM-driver-one-player.efg"),
            ZERO, ["2/3", "1/3", "3/4", "1/4", "1/2", "1/2"], ["1", "1"], "1/2", True,
        ),
        # reentry reached with prob 1
        (
            games.read_from_file("noPR-AM-driver-one-player.efg"),
            ZERO, ["1", "0", "1", "0", "1", "0"], ["1", "1"], "1", True,
        ),
    ],
)
def test_node_belief_reference(
    game: gbt.Game,
    tol: gbt.Rational | float,
    probs: list,
    path: list[str],
    value: str | float,
    rational_flag: bool,
):
    # nodes have no labels, so each belief node is reached by walking the
    # action-label path from the root (an empty path is the root itself)
    profile = game.mixed_behavior_profile(rational=rational_flag)
    _set_action_probs(profile, probs, rational_flag)
    node = game.root
    for action_label in path:
        node = node.children[action_label]
    value = gbt.Rational(value) if rational_flag else value
    assert abs(profile.belief(node) - value) <= tol


@pytest.mark.parametrize(
    "game,rational_flag",
    [
        (games.create_stripped_down_poker_efg(), True),
        (games.create_stripped_down_poker_efg(), False),
    ],
)
def test_payoff_value_error_with_chance_player(game: gbt.Game, rational_flag: bool):
    """Ensure a value error is thrown when we call payoff for a chance player"""
    chance_player = game.players.chance
    with pytest.raises(ValueError):
        game.mixed_behavior_profile(rational=rational_flag).payoff(chance_player)


@pytest.mark.parametrize(
    "game,rational_flag",
    [
        (games.create_stripped_down_poker_efg(), True),
        (games.create_stripped_down_poker_efg(), False),
    ],
)
def test_infoset_value_error_with_chance_player_infoset(game: gbt.Game, rational_flag: bool):
    """Ensure a value error is raised when we call action value for a chance action"""
    chance_infoset = next(iter(game.players.chance.infosets))
    with pytest.raises(ValueError):
        game.mixed_behavior_profile(rational=rational_flag).infoset_value(chance_infoset)


@pytest.mark.parametrize(
    "game,rational_flag",
    [
        (games.create_stripped_down_poker_efg(), True),
        (games.create_stripped_down_poker_efg(), False),
    ],
)
def test_action_value_error_with_chance_player_action(game: gbt.Game, rational_flag: bool):
    """Ensure a value error is raised when we call action value for a chance action"""
    chance_action = next(iter(next(iter(game.players.chance.infosets)).actions))
    with pytest.raises(ValueError):
        game.mixed_behavior_profile(rational=rational_flag).action_value(chance_action)


def _get_answers_one_order(
    game: gbt.Game,
    action_probs_1st: tuple,
    action_probs_2nd: tuple,
    rational_flag: bool,
    func_to_test: typing.Callable,
    object_to_test_on: typing.Any,
):
    """helper function for the 'profile_order' caching tests"""
    ret = dict()
    profile = game.mixed_behavior_profile(rational=rational_flag)
    _set_action_probs(profile, action_probs_1st, rational_flag)
    ret[action_probs_1st] = func_to_test(profile, object_to_test_on)
    _set_action_probs(profile, action_probs_2nd, rational_flag)  # change the profile in place
    ret[action_probs_2nd] = func_to_test(profile, object_to_test_on)
    return ret


def _get_and_check_answers(
    game: gbt.Game,
    action_probs1: tuple,
    action_probs2: tuple,
    rational_flag: bool,
    func_to_test: typing.Callable,
    objects_to_test_on: typing.Collection,
):
    """helper function for the 'profile_order' caching tests"""
    order1_answers = {
        o: _get_answers_one_order(
            game, action_probs1, action_probs2, rational_flag, func_to_test, o
        )
        for o in objects_to_test_on
    }
    order2_answers = {
        o: _get_answers_one_order(
            game, action_probs2, action_probs1, rational_flag, func_to_test, o
        )
        for o in objects_to_test_on
    }
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
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_doub,
            PROBS_2A_doub,
            False,
            lambda x, y: x.belief(y),
            lambda x: x.nodes,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.belief(y),
            lambda x: x.nodes,
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1B_doub,
            PROBS_2B_doub,
            False,
            lambda x, y: x.belief(y),
            lambda x: x.nodes,
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.belief(y),
            lambda x: x.nodes,
        ),
        ######################################################################################
        # realiz_prob (at nodes)
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_doub,
            PROBS_2A_doub,
            False,
            lambda x, y: x.realiz_prob(y),
            lambda x: x.nodes,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.realiz_prob(y),
            lambda x: x.nodes,
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1B_doub,
            PROBS_2B_doub,
            False,
            lambda x, y: x.realiz_prob(y),
            lambda x: x.nodes,
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.realiz_prob(y),
            lambda x: x.nodes,
        ),
        ######################################################################################
        # infoset_prob
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_doub,
            PROBS_2A_doub,
            False,
            lambda x, y: x.infoset_prob(y),
            lambda x: x.infosets,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.infoset_prob(y),
            lambda x: x.infosets,
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1B_doub,
            PROBS_2B_doub,
            False,
            lambda x, y: x.infoset_prob(y),
            lambda x: x.infosets,
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.infoset_prob(y),
            lambda x: x.infosets,
        ),
        ######################################################################################
        # infoset_value
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_doub,
            PROBS_2A_doub,
            False,
            lambda x, y: x.infoset_value(y),
            lambda x: x.infosets,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.infoset_value(y),
            lambda x: x.infosets,
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1B_doub,
            PROBS_2B_doub,
            False,
            lambda x, y: x.infoset_value(y),
            lambda x: x.infosets,
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.infoset_value(y),
            lambda x: x.infosets,
        ),
        ######################################################################################
        # action_value
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_doub,
            PROBS_2A_doub,
            False,
            lambda x, y: x.action_value(y),
            lambda x: x.actions,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.action_value(y),
            lambda x: x.actions,
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1B_doub,
            PROBS_2B_doub,
            False,
            lambda x, y: x.action_value(y),
            lambda x: x.actions,
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.action_value(y),
            lambda x: x.actions,
        ),
        ######################################################################################
        # regret (for actions)
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_doub,
            PROBS_2A_doub,
            False,
            lambda x, y: x.action_regret(y),
            lambda x: x.actions,
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.action_regret(y),
            lambda x: x.actions,
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1B_doub,
            PROBS_2B_doub,
            False,
            lambda x, y: x.action_regret(y),
            lambda x: x.actions,
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.action_regret(y),
            lambda x: x.actions,
        ),
        ######################################################################################
        # node_value
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_doub,
            PROBS_2A_doub,
            False,
            lambda x, y: x.node_value(player=y[0], node=y[1]),
            lambda x: list(product(x.players, x.nodes)),
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.node_value(player=y[0], node=y[1]),
            lambda x: list(product(x.players, x.nodes)),
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1B_doub,
            PROBS_2B_doub,
            False,
            lambda x, y: x.node_value(player=y[0], node=y[1]),
            lambda x: list(product(x.players, x.nodes)),
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.node_value(player=y[0], node=y[1]),
            lambda x: list(product(x.players, x.nodes)),
        ),
        ######################################################################################
        # agent_liap_value (of profile, hence [1] for objects_to_test,
        # any singleton collection would do)
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_doub,
            PROBS_2A_doub,
            False,
            lambda x, y: x.agent_liap_value(),
            lambda x: [1],
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.agent_liap_value(),
            lambda x: [1],
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1B_doub,
            PROBS_2B_doub,
            False,
            lambda x, y: x.agent_liap_value(),
            lambda x: [1],
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.agent_liap_value(),
            lambda x: [1],
        ),
        ######################################################################################
        # liap_value (of profile, hence [1] for objects_to_test,
        # any singleton collection would do)
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_doub,
            PROBS_2A_doub,
            False,
            lambda x, y: x.liap_value(),
            lambda x: [1],
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.liap_value(),
            lambda x: [1],
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1B_doub,
            PROBS_2B_doub,
            False,
            lambda x, y: x.liap_value(),
            lambda x: [1],
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.liap_value(),
            lambda x: [1],
        ),
        ######################################################################################
        # agent_max_regret (of profile, hence [1] for objects_to_test,
        # any singleton collection would do)
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_doub,
            PROBS_2A_doub,
            False,
            lambda x, y: x.agent_max_regret(),
            lambda x: [1],
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.agent_max_regret(),
            lambda x: [1],
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1B_doub,
            PROBS_2B_doub,
            False,
            lambda x, y: x.agent_max_regret(),
            lambda x: [1],
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.agent_max_regret(),
            lambda x: [1],
        ),
        ######################################################################################
        # max_regret (of profile, hence [1] for objects_to_test,
        # any singleton collection would do)
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_doub,
            PROBS_2A_doub,
            False,
            lambda x, y: x.max_regret(),
            lambda x: [1],
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.max_regret(),
            lambda x: [1],
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1B_doub,
            PROBS_2B_doub,
            False,
            lambda x, y: x.max_regret(),
            lambda x: [1],
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.max_regret(),
            lambda x: [1],
        ),
    ],
)
def test_profile_order_consistency(
    game: gbt.Game,
    action_probs1: tuple,
    action_probs2: tuple,
    rational_flag: bool,
    func_to_test: typing.Callable,
    objects_to_test: typing.Callable,
):
    _get_and_check_answers(
        game, action_probs1, action_probs2, rational_flag, func_to_test, objects_to_test(game)
    )


@pytest.mark.parametrize(
    "game,rational_flag,data",
    [
        (games.read_from_file("mixed_behavior_game.efg"), True, [[[0, 1]], [[0, 1]], [[1, 0]]]),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            True,
            [[["1/5", "4/5"]], [["1/4", "3/4"]], [[1, 0]]],
        ),
        (
            games.create_stripped_down_poker_efg(),
            True,
            [[[1 / 5, 4 / 5], [3 / 5, 2 / 5]], [[1 / 4, 3 / 4]]],
        ),
        (games.read_from_file("mixed_behavior_game.efg"), False, [[[0, 1]], [[1, 0]], [[1, 0]]]),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            False,
            [[[1 / 5, 4 / 5]], [[1 / 4, 3 / 4]], [[1, 0]]],
        ),
        (
            games.create_stripped_down_poker_efg(),
            False,
            [[[1 / 5, 4 / 5], [3 / 5, 2 / 5]], [[1 / 4, 3 / 4]]],
        ),
    ],
)
def test_specific_profile(game: gbt.Game, rational_flag: bool, data: list):
    """Test that the mixed behavior profile is initialized from a specific distribution
    for each player over his actions.
    """
    profile = game.mixed_behavior_profile(rational=rational_flag, data=data)
    flattened = iter([k for i in data for j in i for k in j])
    for infoset in game.infosets:
        node = next(iter(infoset.members))
        for action in infoset.actions:
            prob = next(flattened)
            assert profile[node][action.label] == (gbt.Rational(prob) if rational_flag else prob)


@pytest.mark.parametrize(
    "game,rational_flag,data",
    [
        (
            games.read_from_file("mixed_behavior_game.efg"),
            True,
            [[[0, 1, 0]], [[1, 0]], [["1/2", "1/2"]]],
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            True,
            [[[0, 1]], [[1, 0]], [[1, 0]], [[0, 1]]],
        ),
        (
            games.create_stripped_down_poker_efg(),
            True,
            [[["1/5", "4/5"], ["3/5", "2/5"]], [["1/4", "3/4"], ["1/4", "3/4"]]],
        ),
        (
            games.read_from_file("el_farol_bar.efg"),
            True,
            [[4 / 9, 5 / 9], [0], [1 / 2, 1 / 2], [11 / 12, 1 / 12], [1 / 2, 1 / 2]],
        ),
        (games.read_from_file("el_farol_bar.efg"), True, [[1 / 2, 1 / 2]]),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            False,
            [[[0, 1, 0]], [[1, 0]], [[1, 0]]],
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            False,
            [[[0, 1]], [[1, 0]], [[1, 0]], [[0, 1]]],
        ),
        (
            games.create_stripped_down_poker_efg(),
            False,
            [[[1 / 5, 4 / 5], [3 / 5, 2 / 5]], [[1 / 4, 3 / 4], [1 / 4, 3 / 4]]],
        ),
        (
            games.read_from_file("el_farol_bar.efg"),
            False,
            [[4 / 9, 5 / 9], [0], [1 / 2, 1 / 2], [11 / 12, 1 / 12], [1 / 2, 1 / 2]],
        ),
        (games.read_from_file("el_farol_bar.efg"), False, [[1 / 2, 1 / 2]]),
    ],
)
def test_profile_data_error(game: gbt.Game, rational_flag: bool, data: list):
    """Test to ensure a pygambit.ValueError is raised when the data do not
    match with the number of players, the number of the infosets, and the
    number of actions per infoset.
    """
    with pytest.raises(ValueError):
        game.mixed_behavior_profile(rational=rational_flag, data=data)


@pytest.mark.parametrize(
    "game,rational_flag,data",
    [
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            True,
            [["1/5", "2/5", 0, "2/5"], ["1/4", "3/8", "1/4", "3/8"]],
        ),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            False,
            [[1 / 5, 2 / 5, 0 / 5, 2 / 5], [1 / 4, 3 / 8, 1 / 4, 3 / 8]],
        ),
    ],
)
def test_tree_representation_error(game: gbt.Game, rational_flag: bool, data: list):
    """Test to ensure a pygambit.UndefinedOperationError is raised when the game
    to create a mixed behavior profile does not have a tree representation.
    """
    with pytest.raises(gbt.UndefinedOperationError):
        game.mixed_behavior_profile(rational=rational_flag, data=data)


def test_undefined_action_value():
    """Test that undefined action values return `None`."""
    game = gbt.catalog.load("journals/ijgt/selten1975/fig1")
    *_, p3 = game.players
    action = next(iter(next(iter(p3.infosets)).actions))
    for rat in [False, True]:
        profile = game.mixed_behavior_profile([[[1, 0]], [[1, 0]], [[1, 0]]], rational=rat)
        assert profile.action_value(action) is None


def test_undefined_belief():
    """Test that undefined beliefs return `None`."""
    game = gbt.catalog.load("journals/ijgt/selten1975/fig1")
    *_, p3 = game.players
    node = next(iter(next(iter(p3.infosets)).members))
    for rat in [False, True]:
        profile = game.mixed_behavior_profile([[[1, 0]], [[1, 0]], [[1, 0]]], rational=rat)
        assert profile.belief(node) is None


def test_undefined_infoset_value():
    """Test that undefined infoset values return `None`."""
    game = gbt.catalog.load("journals/ijgt/selten1975/fig1")
    *_, p3 = game.players
    infoset = next(iter(p3.infosets))
    for rat in [False, True]:
        profile = game.mixed_behavior_profile([[[1, 0]], [[1, 0]], [[1, 0]]], rational=rat)
        assert profile.infoset_value(infoset) is None
