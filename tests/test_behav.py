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
    game = profile.game
    for infoset in games.all_infosets(game):
        selector = gbt.H.path(*infoset)
        profile[selector] = {
            a: convert(next(probs_iter)) for a in game.get_actions(selector)
        }


def _infoset_history(game: gbt.Game, label: str) -> tuple:
    """The History of the representative member of the infoset historically
    identified by `label`, matching the removed `Infoset`'s by-label lookup."""
    return games._INFOSET_LABEL_HISTORIES[(game.title, label)]


def _member_selector(game: gbt.Game, label: str) -> gbt.Selector:
    """A `Selector` for some member of the infoset historically identified by
    `label` -- not necessarily the representative one, so profile indexing is
    verified to work from any member, not just the canonical one."""
    history = _infoset_history(game, label)
    member = next(iter(game.get_members(gbt.H.path(*history))))
    return gbt.H.path(*member)


@pytest.mark.parametrize(
    "game,rational_flag,payoffs",
    [
        (games.read_from_file("mixed_behavior_game.efg"), False, (3.0, 3.0, 3.25)),
        (games.read_from_file("mixed_behavior_game.efg"), True, (3, 3, "13/4")),
        (games.create_stripped_down_poker_efg(), False, (-0.25, 0.25)),
        (games.create_stripped_down_poker_efg(), True, ("-1/4", "1/4")),
    ],
)
def test_payoffs_reference(game: gbt.Game, rational_flag: bool, payoffs: tuple):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    for payoff, player in zip(payoffs, game.players, strict=True):
        payoff = gbt.Rational(payoff) if rational_flag else payoff
        assert profile.payoffs[player] == payoff


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
    history = _infoset_history(game, infoset_label)
    assert game.get_player(gbt.H.path(*history)) == player_label
    prob = gbt.Rational(prob) if rational_flag else prob
    assert profile[_member_selector(game, infoset_label)][action_label] == prob


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
def test_profile_indexing_by_selector_reference(
    game: gbt.Game, player_label: str, infoset_label: str, probs: list, rational_flag: bool
):
    """profile[selector] and profile[player_label][selector] resolve to the same
    MixedAction."""
    profile = game.mixed_behavior_profile(rational=rational_flag)
    history = _infoset_history(game, infoset_label)
    assert game.get_player(gbt.H.path(*history)) == player_label
    selector = _member_selector(game, infoset_label)
    probs = [gbt.Rational(prob) for prob in probs] if rational_flag else probs
    expected = dict(zip(game.get_actions(gbt.H.path(*history)), probs, strict=True))
    assert profile[player_label][selector] == expected
    assert profile[selector] == expected


@pytest.mark.parametrize(
    "game,player_label,other_player_label",
    [
        (games.read_from_file("mixed_behavior_game.efg"), "Player 1", "Player 2"),
        (games.create_stripped_down_poker_efg(), "Alice", "Bob"),
    ],
)
def test_behavior_indexing_rejects_selector_from_different_player(
    game: gbt.Game, player_label: str, other_player_label: str
):
    """MixedBehavior rejects a Selector whose information set belongs to a
    different player than the one being indexed.
    """
    profile = game.mixed_behavior_profile()
    other_infoset = games.player_infosets(game, other_player_label)[0]
    other_selector = gbt.H.path(*other_infoset)
    with pytest.raises(gbt.MismatchError):
        profile[player_label][other_selector]


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
    expected = [
        dict(zip(game.get_actions(gbt.H.path(*infoset)), probs, strict=True))
        for infoset, probs in zip(
            games.player_infosets(game, player_label), behav_data, strict=True
        )
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
    """A sparse one-action distribution leaves the infoset's other actions at weight zero."""
    profile = game.mixed_behavior_profile(rational=rational_flag)
    prob = gbt.Rational(prob) if rational_flag else prob
    selector = _member_selector(game, infoset_label)
    profile[selector] = {action_label: prob}
    assert profile[selector][action_label] == prob


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
    history = _infoset_history(game, infoset_label)
    assert game.get_player(gbt.H.path(*history)) == player_label
    selector = _member_selector(game, infoset_label)
    expected = dict(zip(game.get_actions(gbt.H.path(*history)), probs, strict=True))
    profile[selector] = expected
    assert profile[selector] == expected


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
    expected = [
        dict(zip(game.get_actions(gbt.H.path(*infoset)), probs, strict=True))
        for infoset, probs in zip(
            games.player_infosets(game, player_label), behav_data, strict=True
        )
    ]
    for infoset, distribution in zip(
        games.player_infosets(game, player_label), expected, strict=True
    ):
        profile[gbt.H.path(*infoset)] = distribution
    assert profile[player_label] == expected


def _p1_selector(game: gbt.Game) -> gbt.Selector:
    """A Selector for Player 1's first information set."""
    return gbt.H.path(*games.player_infosets(game, "Player 1")[0])


def test_behavior_setitem_allows_sparse_distribution():
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.mixed_behavior_profile()
    selector = _p1_selector(game)
    profile[selector] = {"U1": 1}
    assert profile[selector] == {"U1": 1, "D1": 0}


def test_set_mixed_action_sparse_matches_setitem():
    game = games.read_from_file("mixed_behavior_game.efg")
    selector = _p1_selector(game)
    sparse_profile = game.mixed_behavior_profile()
    sparse_profile.set_mixed_action(selector, {"U1": 1}, sparse=True)
    setitem_profile = game.mixed_behavior_profile()
    setitem_profile[selector] = {"U1": 1}
    assert sparse_profile[selector] == setitem_profile[selector]


def test_set_mixed_action_defaults_to_requiring_every_label():
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.mixed_behavior_profile()
    selector = _p1_selector(game)
    with pytest.raises(ValueError, match="exactly one weight"):
        profile.set_mixed_action(selector, {"U1": 1})


@pytest.mark.parametrize("sparse", [False, True])
def test_setitem_and_set_mixed_action_reject_unknown_action_label(sparse: bool):
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.mixed_behavior_profile()
    selector = _p1_selector(game)
    with pytest.raises(ValueError, match="not an action label"):
        profile.set_mixed_action(selector, {"not-an-action": 1}, sparse=sparse)
    with pytest.raises(ValueError, match="not an action label"):
        profile[selector] = {"not-an-action": 1}


def test_behavior_setitem_empty_distribution_is_all_zero_error():
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.mixed_behavior_profile()
    selector = _p1_selector(game)
    with pytest.raises(ValueError, match="zero"):
        profile[selector] = {}


@pytest.mark.parametrize("sparse", [False, True])
def test_setitem_and_set_mixed_action_reject_non_mapping(sparse: bool):
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.mixed_behavior_profile()
    selector = _p1_selector(game)
    with pytest.raises(TypeError, match="Mapping"):
        profile.set_mixed_action(selector, [1, 0], sparse=sparse)
    with pytest.raises(TypeError, match="Mapping"):
        profile[selector] = [1, 0]


@pytest.mark.parametrize("sparse", [False, True])
def test_setitem_and_set_mixed_action_reject_uncoercible_weight(sparse: bool):
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.mixed_behavior_profile()
    selector = _p1_selector(game)
    full_distribution = {"U1": "abc", "D1": 0}
    with pytest.raises(ValueError, match="convert"):
        profile.set_mixed_action(selector, full_distribution, sparse=sparse)
    with pytest.raises(ValueError, match="convert"):
        profile[selector] = full_distribution


def test_behavior_setitem_sparse_rejects_negative_weight():
    """Negativity is checked even for weights given under a sparse distribution."""
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.mixed_behavior_profile()
    selector = _p1_selector(game)
    with pytest.raises(ValueError, match="negative"):
        profile[selector] = {"U1": -1}


@pytest.mark.parametrize("sparse", [False, True])
def test_behavior_indexing_rejects_node_and_infoset(sparse: bool):
    """MixedBehaviorProfile's indexing is Selector-only; neither a bare History nor a
    bare `Node` object is accepted, following the same pattern as
    `Game.get_minimal_subgame`.
    """
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.mixed_behavior_profile()
    infoset = games.player_infosets(game, "Player 1")[0]
    node = games.all_nodes(game)[0]
    with pytest.raises(TypeError):
        profile[infoset]
    with pytest.raises(TypeError):
        profile[infoset] = {"U1": 1}
    with pytest.raises(TypeError):
        profile.set_mixed_action(infoset, {"U1": 1}, sparse=sparse)
    with pytest.raises(TypeError):
        profile[node]
    with pytest.raises(TypeError):
        profile[node] = {"U1": 1}
    with pytest.raises(TypeError):
        profile.set_mixed_action(node, {"U1": 1}, sparse=sparse)


@pytest.mark.parametrize("rational_flag", [False, True])
def test_mixed_action_and_behavior_are_frozen_snapshots(rational_flag: bool):
    """MixedAction/MixedBehavior are snapshots taken at retrieval time: they do not
    reflect later mutations to the profile they came from.
    """
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.mixed_behavior_profile(rational=rational_flag)
    selector = _p1_selector(game)
    action_before = profile[selector]
    behavior_before = profile["Player 1"]
    profile[selector] = {"U1": 1, "D1": 0}
    assert dict(action_before) == {"U1": 0.5, "D1": 0.5}
    assert dict(behavior_before[selector]) == {"U1": 0.5, "D1": 0.5}
    assert dict(profile[selector]) == {"U1": 1, "D1": 0}


@pytest.mark.parametrize("rational_flag", [False, True])
def test_behavior_copy_mutating_copy_does_not_affect_original(rational_flag: bool):
    game = games.read_from_file("mixed_behavior_game.efg")
    original = game.mixed_behavior_profile(rational=rational_flag)
    selector = _p1_selector(game)
    original_before = dict(original[selector])
    copy = original.copy()
    copy[selector] = {"U1": 1, "D1": 0}
    assert dict(original[selector]) == original_before
    assert dict(copy[selector]) == {"U1": 1, "D1": 0}


@pytest.mark.parametrize("rational_flag", [False, True])
def test_behavior_copy_mutating_original_does_not_affect_copy(rational_flag: bool):
    game = games.read_from_file("mixed_behavior_game.efg")
    original = game.mixed_behavior_profile(rational=rational_flag)
    selector = _p1_selector(game)
    copy = original.copy()
    copy_before = dict(copy[selector])
    original[selector] = {"U1": 1, "D1": 0}
    assert dict(copy[selector]) == copy_before
    assert dict(original[selector]) == {"U1": 1, "D1": 0}


@pytest.mark.parametrize("rational_flag", [False, True])
def test_as_float_returns_double(rational_flag: bool):
    game = games.read_from_file("mixed_behavior_game.efg")
    result = game.mixed_behavior_profile(rational=rational_flag).as_float()
    assert isinstance(result, gbt.MixedBehaviorProfileDouble)


def test_as_float_converts_rational_probabilities():
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.mixed_behavior_profile(rational=True)
    selector = _p1_selector(game)
    profile[selector] = {"U1": "1/3", "D1": "2/3"}
    result = profile.as_float()
    assert dict(result[selector]) == {"U1": pytest.approx(1 / 3), "D1": pytest.approx(2 / 3)}


def test_as_float_is_independent_copy():
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.mixed_behavior_profile(rational=False)
    selector = _p1_selector(game)
    result = profile.as_float()
    assert result == profile
    result[selector] = {"U1": 1.0, "D1": 0.0}
    assert dict(profile[selector]) != dict(result[selector])


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
    # a node's History is the tuple of action labels from the root (empty for the root)
    profile = game.mixed_behavior_profile(rational=rational_flag)
    realiz_prob = gbt.Rational(realiz_prob) if rational_flag else realiz_prob
    assert profile.realiz_probs[tuple(path)] == realiz_prob


@pytest.mark.parametrize(
    "game,rational_flag,infoset_probs",
    [
        (games.read_from_file("mixed_behavior_game.efg"), False, (1.0, 1.0, 1.0)),
        (games.read_from_file("mixed_behavior_game.efg"), True, (1, 1, 1)),
        (games.create_stripped_down_poker_efg(), False, (0.5, 0.5, 0.5)),
        (games.create_stripped_down_poker_efg(), True, ("1/2", "1/2", "1/2")),
    ],
)
def test_infoset_probs_reference(game: gbt.Game, rational_flag: bool, infoset_probs: tuple):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    for prob, infoset in zip(infoset_probs, games.all_infosets(game), strict=True):
        prob = gbt.Rational(prob) if rational_flag else prob
        assert profile.infoset_probs[gbt.H.path(*infoset)] == prob


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
    ip = profile.infoset_probs[_member_selector(game, infoset_label)]
    assert ip == (gbt.Rational(prob) if rational_flag else prob)


@pytest.mark.parametrize("rational_flag", [False, True])
def test_nature_rooted_game_root_reached_with_certainty(rational_flag: bool):
    """The chance root infoset is reached with probability one."""
    game = gbt.catalog.load("journals/geb/gilboa1997/fig2")
    profile = game.mixed_behavior_profile(rational=rational_flag)
    one = gbt.Rational(1) if rational_flag else 1.0
    assert profile.realiz_probs[()] == one
    assert profile.infoset_probs[gbt.H.path()] == one


@pytest.mark.parametrize(
    "game,rational_flag,infoset_values",
    [
        (games.read_from_file("mixed_behavior_game.efg"), False, (3.0, 3.0, 3.25)),
        (games.read_from_file("mixed_behavior_game.efg"), True, (3, 3, "13/4")),
        (games.create_stripped_down_poker_efg(), False, (0.25, -0.75, -0.5)),
        (games.create_stripped_down_poker_efg(), True, ("1/4", "-3/4", "-1/2")),
    ],
)
def test_infoset_values_reference(game: gbt.Game, rational_flag: bool, infoset_values: tuple):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    for payoff, infoset in zip(infoset_values, games.all_infosets(game), strict=True):
        payoff = gbt.Rational(payoff) if rational_flag else payoff
        assert profile.infoset_values[gbt.H.path(*infoset)] == payoff


@pytest.mark.parametrize(
    "game,rational_flag,action_values",
    [
        (
            games.read_from_file("mixed_behavior_game.efg"),
            False,
            ((3.0, 3.0), (3.0, 3.0), (3.5, 3.0)),
        ),
        (games.read_from_file("mixed_behavior_game.efg"), True, ((3, 3), (3, 3), ("7/2", 3))),
        (games.create_stripped_down_poker_efg(), False, ((1.5, -1), (-0.5, -1), (0, -1))),
        (
            games.create_stripped_down_poker_efg(),
            True,
            (("3/2", -1), ("-1/2", -1), (0, -1)),
        ),
    ],
)
def test_action_values_reference(game: gbt.Game, rational_flag: bool, action_values: tuple):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    for values_for_infoset, infoset in zip(action_values, games.all_infosets(game), strict=True):
        selector = gbt.H.path(*infoset)
        infoset_action_values = profile.action_values[selector]
        for value, action in zip(
            values_for_infoset, game.get_actions(selector), strict=True
        ):
            value = gbt.Rational(value) if rational_flag else value
            assert infoset_action_values[action] == value


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
        for infoset in games.player_infosets(game, player):
            selector = gbt.H.path(*infoset)
            actions = game.get_actions(selector)
            for action in actions:
                assert profile.action_regrets[selector][action] == max(
                    profile.action_values[selector][a] for a in actions
                ) - profile.action_values[selector][action]


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
        for infoset in games.player_infosets(game, player):
            selector = gbt.H.path(*infoset)
            assert profile.infoset_regrets[selector] == max(
                profile.action_values[selector][a] for a in game.get_actions(selector)
            ) - profile.infoset_values[selector]


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
    infoset_regrets = profile.infoset_regrets
    assert profile.agent_max_regret() == max(
        infoset_regrets[gbt.H.path(*infoset)] for infoset in games.all_infosets(game)
    )


@pytest.mark.parametrize(
    "game,rational_flag",
    [
        (games.read_from_file("mixed_behavior_game.efg"), False),
        (games.read_from_file("mixed_behavior_game.efg"), True),
        (games.create_stripped_down_poker_efg(), False),
        (games.create_stripped_down_poker_efg(), True),
    ],
)
def test_vectorized_quantities_consistency(game: gbt.Game, rational_flag: bool):
    """The vectorized payoffs/node_values/infoset_values/infoset_regrets/action_values/
    action_regrets/realiz_probs/infoset_probs/beliefs properties are mathematically
    consistent with each other (regret is the gap to the best response), and carry their
    own type identity.
    """
    profile = game.mixed_behavior_profile(rational=rational_flag)

    payoffs = profile.payoffs
    node_values = profile.node_values
    infoset_values = profile.infoset_values
    infoset_regrets = profile.infoset_regrets
    action_values = profile.action_values
    action_regrets = profile.action_regrets
    realiz_probs = profile.realiz_probs
    infoset_probs = profile.infoset_probs
    beliefs = profile.beliefs

    assert isinstance(payoffs, gbt.PayoffVector)
    assert isinstance(payoffs, gbt.PlayerIndexedVector)
    assert isinstance(node_values, gbt.NodeValuesVector)
    assert isinstance(infoset_values, gbt.InfosetValueVector)
    assert isinstance(infoset_regrets, gbt.InfosetRegretVector)
    assert isinstance(action_values, gbt.ActionValuesVector)
    assert isinstance(action_regrets, gbt.ActionRegretsVector)
    assert isinstance(realiz_probs, gbt.RealizProbVector)
    assert isinstance(infoset_probs, gbt.InfosetProbVector)
    assert isinstance(beliefs, gbt.BeliefVector)

    for player in game.players:
        player_node_values = node_values[player]
        assert isinstance(player_node_values, gbt.NodeValueVector)
        assert player_node_values[()] == payoffs[player]

        for infoset in games.player_infosets(game, player):
            selector = gbt.H.path(*infoset)
            actions = game.get_actions(selector)
            infoset_action_values = action_values[selector]
            infoset_action_regrets = action_regrets[selector]
            assert isinstance(infoset_action_values, gbt.ActionValueVector)
            assert isinstance(infoset_action_regrets, gbt.ActionRegretVector)

            best_response_value = max(infoset_action_values[a] for a in actions)
            assert infoset_regrets[selector] == best_response_value - infoset_values[selector]
            for action in actions:
                assert (
                    infoset_action_regrets[action]
                    == best_response_value - infoset_action_values[action]
                )

    for history in game.get_histories(gbt.H.after()):
        if not game.get_actions(gbt.H.path(*history)):
            continue
        if infoset_probs[gbt.H.path(*history)] == 0:
            assert beliefs[history] is None
        else:
            assert beliefs[history] is not None

    # equal to an equivalent plain dict or same-type vector, but never to a vector of a
    # different quantity, even where the underlying numbers happen to coincide
    expected = dict(realiz_probs)
    assert realiz_probs == expected
    assert realiz_probs == gbt.RealizProbVector(expected)
    assert realiz_probs != beliefs
    assert realiz_probs != gbt.BeliefVector(expected)

    with pytest.raises(KeyError):
        payoffs["not a player label"]


@pytest.mark.parametrize(
    "game,action_probs,rational_flag,tol,action_regrets",
    [
        # uniform: only P3 (whose two actions have unequal value, 3.5 vs 3.0) has any regret
        (
            games.read_from_file("mixed_behavior_game.efg"),
            None, False, TOL,
            ((0, 0), (0, 0), (0, 0.5)),
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            None, True, ZERO,
            ((0, 0), (0, 0), (0, "1/2")),
        ),
        # pure profile U1 U2 U3: each player's off-path action has the outside option's
        # regret (9, 8, 12 respectively)
        (
            games.read_from_file("mixed_behavior_game.efg"),
            [1, 0, 1, 0, 1, 0], False, TOL,
            ((0, 9), (0, 8), (0, 12)),
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            ["1", "0", "1", "0", "1", "0"], True, ZERO,
            ((0, 9), (0, 8), (0, 12)),
        ),
        # mixed Nash equilibrium: every action is a best response, so all regrets are zero
        (
            games.read_from_file("mixed_behavior_game.efg"),
            ["2/5", "3/5", "1/2", "1/2", "1/3", "2/3"], True, ZERO,
            ((0, 0), (0, 0), (0, 0)),
        ),
        # uniform
        (
            games.create_stripped_down_poker_efg(),
            None, False, TOL,
            ((0, 2.5), (0, 0.5), (0, 1)),
        ),
        (
            games.create_stripped_down_poker_efg(),
            None, True, ZERO,
            ((0, "5/2"), (0, "1/2"), (0, 1)),
        ),
        # mixed Nash equilibrium: Bob is exactly indifferent (zero regret both ways);
        # Alice has King is the only off-path action left with positive regret
        (
            games.create_stripped_down_poker_efg(),
            ["1", "0", "1/3", "2/3", "2/3", "1/3"], True, ZERO,
            ((0, "8/3"), (0, 0), (0, 0)),
        ),
    ],
)
def test_action_regrets_reference(
    game: gbt.Game,
    action_probs: None | list,
    rational_flag: bool,
    tol: gbt.Rational | float,
    action_regrets: tuple,
):
    profile = game.mixed_behavior_profile(rational=rational_flag)
    if action_probs:
        _set_action_probs(profile, action_probs, rational_flag)
    for regrets_for_infoset, infoset in zip(action_regrets, games.all_infosets(game), strict=True):
        selector = gbt.H.path(*infoset)
        infoset_action_regrets = profile.action_regrets[selector]
        for regret, action in zip(
            regrets_for_infoset, game.get_actions(selector), strict=True
        ):
            regret = gbt.Rational(regret) if rational_flag else regret
            assert abs(infoset_action_regrets[action] - regret) <= tol


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
    realiz_probs = profile.realiz_probs
    node_values = profile.node_values
    for history in game.get_histories(gbt.H.after()):
        player = game.get_player(gbt.H.path(*history))
        if player is None or player == "Chance":
            continue
        expected_val = 0
        node_prob = realiz_probs[history]
        player_node_values = node_values[player]
        for child_history in games.children_histories(game, history):
            prob = realiz_probs[child_history] / node_prob
            expected_val += prob * player_node_values[child_history]
        assert player_node_values[history] == expected_val


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
    node_values = profile.node_values
    payoffs = profile.payoffs
    for player in game.players:
        assert node_values[player][()] == payoffs[player]


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
    # a node's History is the tuple of action labels from the root (empty for the root)
    profile = game.mixed_behavior_profile(rational=rational_flag)
    _set_action_probs(profile, probs, rational_flag)
    value = gbt.Rational(value) if rational_flag else value
    assert abs(profile.beliefs[tuple(path)] - value) <= tol


@pytest.mark.parametrize(
    "game,rational_flag",
    [
        (games.create_stripped_down_poker_efg(), True),
        (games.create_stripped_down_poker_efg(), False),
    ],
)
def test_infoset_value_error_with_chance_player_infoset(game: gbt.Game, rational_flag: bool):
    """The chance player's infosets are excluded from infoset_values, so looking one up
    is a KeyError.
    """
    chance_node = game.get_events()[0]
    with pytest.raises(KeyError):
        game.mixed_behavior_profile(rational=rational_flag).infoset_values[
            gbt.H.path(*chance_node)
        ]


@pytest.mark.parametrize(
    "game,rational_flag",
    [
        (games.create_stripped_down_poker_efg(), True),
        (games.create_stripped_down_poker_efg(), False),
    ],
)
def test_action_value_error_with_chance_player_action(game: gbt.Game, rational_flag: bool):
    """The chance player's infosets are excluded from action_values, so looking up an
    action there is a KeyError.
    """
    chance_node = game.get_events()[0]
    with pytest.raises(KeyError):
        game.mixed_behavior_profile(rational=rational_flag).action_values[
            gbt.H.path(*chance_node)
        ]


def _all_node_actions(game: gbt.Game) -> list[tuple[gbt.Selector, str]]:
    """All (selector, action label) pairs across every personal player's information sets."""
    return [
        (selector, action)
        for player in game.players
        for history in game.get_infosets(player)
        for selector in [gbt.H.path(*history)]
        for action in game.get_actions(selector)
    ]


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
            lambda x, y: x.beliefs[y],
            lambda x: x.get_histories(gbt.H.after()),
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.beliefs[y],
            lambda x: x.get_histories(gbt.H.after()),
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1B_doub,
            PROBS_2B_doub,
            False,
            lambda x, y: x.beliefs[y],
            lambda x: x.get_histories(gbt.H.after()),
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.beliefs[y],
            lambda x: x.get_histories(gbt.H.after()),
        ),
        ######################################################################################
        # realiz_prob (at nodes)
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_doub,
            PROBS_2A_doub,
            False,
            lambda x, y: x.realiz_probs[y],
            lambda x: x.get_histories(gbt.H.after()),
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.realiz_probs[y],
            lambda x: x.get_histories(gbt.H.after()),
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1B_doub,
            PROBS_2B_doub,
            False,
            lambda x, y: x.realiz_probs[y],
            lambda x: x.get_histories(gbt.H.after()),
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.realiz_probs[y],
            lambda x: x.get_histories(gbt.H.after()),
        ),
        ######################################################################################
        # infoset_prob
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_doub,
            PROBS_2A_doub,
            False,
            lambda x, y: x.infoset_probs[gbt.H.path(*y)],
            lambda x: games.all_infosets(x),
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.infoset_probs[gbt.H.path(*y)],
            lambda x: games.all_infosets(x),
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1B_doub,
            PROBS_2B_doub,
            False,
            lambda x, y: x.infoset_probs[gbt.H.path(*y)],
            lambda x: games.all_infosets(x),
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.infoset_probs[gbt.H.path(*y)],
            lambda x: games.all_infosets(x),
        ),
        ######################################################################################
        # infoset_value
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_doub,
            PROBS_2A_doub,
            False,
            lambda x, y: x.infoset_values[gbt.H.path(*y)],
            lambda x: games.all_infosets(x),
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.infoset_values[gbt.H.path(*y)],
            lambda x: games.all_infosets(x),
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1B_doub,
            PROBS_2B_doub,
            False,
            lambda x, y: x.infoset_values[gbt.H.path(*y)],
            lambda x: games.all_infosets(x),
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.infoset_values[gbt.H.path(*y)],
            lambda x: games.all_infosets(x),
        ),
        ######################################################################################
        # action_value
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_doub,
            PROBS_2A_doub,
            False,
            lambda x, ny: x.action_values[ny[0]][ny[1]],
            lambda x: _all_node_actions(x),
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, ny: x.action_values[ny[0]][ny[1]],
            lambda x: _all_node_actions(x),
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1B_doub,
            PROBS_2B_doub,
            False,
            lambda x, ny: x.action_values[ny[0]][ny[1]],
            lambda x: _all_node_actions(x),
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, ny: x.action_values[ny[0]][ny[1]],
            lambda x: _all_node_actions(x),
        ),
        ######################################################################################
        # regret (for actions)
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_doub,
            PROBS_2A_doub,
            False,
            lambda x, ny: x.action_regrets[ny[0]][ny[1]],
            lambda x: _all_node_actions(x),
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, ny: x.action_regrets[ny[0]][ny[1]],
            lambda x: _all_node_actions(x),
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1B_doub,
            PROBS_2B_doub,
            False,
            lambda x, ny: x.action_regrets[ny[0]][ny[1]],
            lambda x: _all_node_actions(x),
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, ny: x.action_regrets[ny[0]][ny[1]],
            lambda x: _all_node_actions(x),
        ),
        ######################################################################################
        # node_value
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_doub,
            PROBS_2A_doub,
            False,
            lambda x, y: x.node_values[y[0]][y[1]],
            lambda x: list(product(x.players, x.get_histories(gbt.H.after()))),
        ),
        (
            games.read_from_file("mixed_behavior_game.efg"),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.node_values[y[0]][y[1]],
            lambda x: list(product(x.players, x.get_histories(gbt.H.after()))),
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1B_doub,
            PROBS_2B_doub,
            False,
            lambda x, y: x.node_values[y[0]][y[1]],
            lambda x: list(product(x.players, x.get_histories(gbt.H.after()))),
        ),
        (
            games.create_stripped_down_poker_efg(),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda x, y: x.node_values[y[0]][y[1]],
            lambda x: list(product(x.players, x.get_histories(gbt.H.after()))),
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
    for each player over their actions.
    """
    profile = game.mixed_behavior_profile(rational=rational_flag, data=data)
    flattened = iter([k for i in data for j in i for k in j])
    for infoset in games.all_infosets(game):
        selector = gbt.H.path(*infoset)
        for action in game.get_actions(selector):
            prob = next(flattened)
            assert profile[selector][action] == (gbt.Rational(prob) if rational_flag else prob)


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
    selector = gbt.H.path(*games.player_infosets(game, p3)[0])
    action = game.get_actions(selector)[0]
    for rat in [False, True]:
        profile = game.mixed_behavior_profile([[[1, 0]], [[1, 0]], [[1, 0]]], rational=rat)
        assert profile.action_values[selector][action] is None


def test_undefined_belief():
    """Test that undefined beliefs return `None`."""
    game = gbt.catalog.load("journals/ijgt/selten1975/fig1")
    *_, p3 = game.players
    history = games.player_infosets(game, p3)[0]
    for rat in [False, True]:
        profile = game.mixed_behavior_profile([[[1, 0]], [[1, 0]], [[1, 0]]], rational=rat)
        assert profile.beliefs[history] is None


def test_undefined_infoset_value():
    """Test that undefined infoset values return `None`."""
    game = gbt.catalog.load("journals/ijgt/selten1975/fig1")
    *_, p3 = game.players
    selector = gbt.H.path(*games.player_infosets(game, p3)[0])
    for rat in [False, True]:
        profile = game.mixed_behavior_profile([[[1, 0]], [[1, 0]], [[1, 0]]], rational=rat)
        assert profile.infoset_values[selector] is None
