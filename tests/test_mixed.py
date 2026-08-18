import typing

import pytest

import pygambit as gbt

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
    # assumes rationals given as strings
    convert = (lambda p: gbt.Rational(p)) if rational_flag else (lambda p: p)
    if len(probs) != len(profile.game.strategies):
        raise ValueError("probs must have one entry per strategy in the game")
    offset = 0
    for player in profile.game.players:
        k = len(player.strategies)
        profile[player.label] = {
            s.label: convert(p)
            for s, p in zip(player.strategies, probs[offset:offset + k], strict=True)
        }
        offset += k


@pytest.mark.parametrize(
    "game,profile_data,rational_flag",
    [
        ###############################################################################
        # 4x4 coordination nfg
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [[0, 0, 0, 0], ["1/3", "1/3", "1/3", 0]],
            True,
        ),
        (games.read_from_file("coordination_4x4_payoff.nfg"), [[1, 0, 0, 0], [0, 0, 0, 0]], True),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [[0, 0, 0, 0], [1.0, 1.0, 1.0, 1.0]],
            False,
        ),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [[1.0, 1.0, 1.0, 1.0], [0, 0, 0, 0]],
            False,
        ),
        ###############################################################################
        # centipede with chance efg
        (games.read_from_file("cent3.efg"), [[0, 0, 0, 0], [1, 0, 0, 0]], True),
        (games.read_from_file("cent3.efg"), [[1, 0, 0, 0], [0, 0, 0, 0]], True),
        (games.read_from_file("cent3.efg"), [[0, 0, 0, 0], [1, 0, 0, 0]], False),
        (games.read_from_file("cent3.efg"), [[1, 0, 0, 0], [0, 0, 0, 0]], False),
    ],
)
def test_normalize_zero_value_error(game, profile_data, rational_flag):
    """A profile with an all-zero distribution for some player can no longer even be
    constructed: assignment now validates this, so normalize() no longer needs to.
    """
    with pytest.raises(ValueError, match="zero"):
        game.mixed_strategy_profile(rational=rational_flag, data=profile_data)


@pytest.mark.parametrize(
    "game,profile_data,rational_flag",
    [
        ###############################################################################
        # 4x4 coordination nfg
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [[1, 1, 0, -1], ["1/3", "1/3", "1/3", 0]],
            True,
        ),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [[0, 0, 0, -1.0], [1.0, 1.0, 1.0, 1.0]],
            False,
        ),
        ###############################################################################
        # zero matrix nfg
        (games.read_from_file("2x2_bimatrix_all_zero_payoffs.nfg"), [[1, 0], [0, -1]], True),
        (
            games.read_from_file("2x2_bimatrix_all_zero_payoffs.nfg"),
            [[1.0, 1.0], [0, -1.0]],
            False,
        ),
        ###############################################################################
        # centipede with chance efg
        (games.read_from_file("cent3.efg"), [[-1, 0, 0, 0], [1, 0, 0, 0]], True),
        (games.read_from_file("cent3.efg"), [[1, 0, 0, 0], [-1, 0, 0, 0]], True),
        (games.read_from_file("cent3.efg"), [[-1, 0, 0, 0], [1, 0, 0, 0]], False),
        (games.read_from_file("cent3.efg"), [[1, 0, 0, 0], [-1, 0, 0, 0]], False),
    ],
)
def test_normalize_neg_entry_value_error(game, profile_data, rational_flag):
    """A profile with a negative weight for some player can no longer even be constructed:
    assignment now validates this, so normalize() no longer needs to.
    """
    with pytest.raises(ValueError, match="negative"):
        game.mixed_strategy_profile(rational=rational_flag, data=profile_data)


@pytest.mark.parametrize(
    "game,profile_data,expected_data,rational_flag",
    [
        ###############################################################################
        # 4x4 coordination nfg
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [[1, 2, 3, 14], [1, 1, 1, 1]],
            [["1/20", "2/20", "3/20", "14/20"], ["1/4", "1/4", "1/4", "1/4"]],
            True,
        ),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [[1.0, 2.0, 3.0, 14.0], [1, 1, 1, 1]],
            [[1 / 20, 2 / 20, 3 / 20, 14 / 20], [0.25, 0.25, 0.25, 0.25]],
            False,
        ),
        ###############################################################################
        # centipede with chance efg
        (
            games.read_from_file("cent3.efg"),
            [[1, 2, 3, 14], [1, 1, 1, 1]],
            [["1/20", "2/20", "3/20", "14/20"], ["1/4", "1/4", "1/4", "1/4"]],
            True,
        ),
        (
            games.read_from_file("cent3.efg"),
            [[1.0, 2.0, 3.0, 14.0], [1, 1, 1, 1]],
            [[1 / 20, 2 / 20, 3 / 20, 14 / 20], [0.25, 0.25, 0.25, 0.25]],
            False,
        ),
    ],
)
def test_normalize(game, profile_data, expected_data, rational_flag):
    assert game.mixed_strategy_profile(
        data=profile_data, rational=rational_flag
    ).normalize() == game.mixed_strategy_profile(data=expected_data, rational=rational_flag)


@pytest.mark.parametrize(
    "game,strategy_label,rational_flag,prob",
    [
        ##############################################################################
        # zero matrix nfg
        (games.read_from_file("2x2_bimatrix_all_zero_payoffs.nfg"), "cooperate", False, 0.72),
        (games.read_from_file("2x2_bimatrix_all_zero_payoffs.nfg"), "cooperate", True, "7/9"),
        ###############################################################################
        # coordination 4x4 nfg outcome version with strategy labels
        (games.read_from_file("coordination_4x4_outcome.nfg"), "1-1", False, 0.25),
        (games.read_from_file("coordination_4x4_outcome.nfg"), "1-1", True, "1/4"),
    ],
)
def test_set_and_get_probability_by_strategy_label(
    game: gbt.Game, strategy_label: str, rational_flag: bool, prob: float | str
):
    """A single strategy's probability can be set and read via a whole-player
    distribution, keyed by strategy label.
    """
    prob = gbt.Rational(prob) if rational_flag else prob
    profile = game.mixed_strategy_profile(rational=rational_flag)
    player = game.strategies[strategy_label].player
    profile[player.label] = {
        s.label: (prob if s.label == strategy_label else 0) for s in player.strategies
    }
    assert profile[player.label][strategy_label] == prob


@pytest.mark.parametrize(
    "game,player_label,rational_flag,profile_data",
    [
        ##############################################################################
        # zero matrix nfg
        (games.read_from_file("2x2_bimatrix_all_zero_payoffs.nfg"), "Joe", False, [0.72, 0.28]),
        (games.read_from_file("2x2_bimatrix_all_zero_payoffs.nfg"), "Joe", True, ["7/9", "2/9"]),
        ##############################################################################
        # coordination 4x4 nfg outcome version with strategy labels
        (games.read_from_file("coordination_4x4_payoff.nfg"), P1, False, [0.25, 0, 0, 0.75]),
        (games.read_from_file("coordination_4x4_payoff.nfg"), P1, True, ["1/4", 0, 0, "3/4"]),
        ##############################################################################
        # stripped-down poker efg
        (games.create_stripped_down_poker_efg(), "Alice", False, [0.25, 0.75, 0, 0]),
        (games.create_stripped_down_poker_efg(), "Bob", False, [1, 0]),
        (games.create_stripped_down_poker_efg(), "Alice", True, ["1/4", "3/4", 0, 0]),
        (games.create_stripped_down_poker_efg(), "Bob", True, [1, 0]),
    ],
)
def test_set_and_get_probabilities_by_player_label(
    game: gbt.Game, player_label: str, rational_flag: bool, profile_data: list
):
    profile_data = [gbt.Rational(p) for p in profile_data] if rational_flag else profile_data
    profile = game.mixed_strategy_profile(rational=rational_flag)
    player = game.players[player_label]
    expected = dict(zip((s.label for s in player.strategies), profile_data, strict=True))
    profile[player_label] = expected
    assert profile[player_label] == expected


def test_setitem_allows_sparse_distribution():
    game = games.read_from_file("coordination_4x4_payoff.nfg")
    profile = game.mixed_strategy_profile()
    profile[P1] = {"1": 1}
    assert profile[P1] == {"1": 1, "2": 0, "3": 0, "4": 0}


def test_set_strategy_sparse_matches_setitem():
    game = games.read_from_file("coordination_4x4_payoff.nfg")
    sparse_profile = game.mixed_strategy_profile()
    sparse_profile.set_strategy(P1, {"1": 1}, sparse=True)
    setitem_profile = game.mixed_strategy_profile()
    setitem_profile[P1] = {"1": 1}
    assert sparse_profile[P1] == setitem_profile[P1]


def test_set_strategy_defaults_to_requiring_every_label():
    game = games.read_from_file("coordination_4x4_payoff.nfg")
    profile = game.mixed_strategy_profile()
    with pytest.raises(ValueError, match="exactly one weight"):
        profile.set_strategy(P1, {"1": 1})


@pytest.mark.parametrize("sparse", [False, True])
def test_setitem_and_set_strategy_reject_unknown_strategy_label(sparse: bool):
    game = games.read_from_file("coordination_4x4_payoff.nfg")
    profile = game.mixed_strategy_profile()
    with pytest.raises(ValueError, match="not a strategy label"):
        profile.set_strategy(P1, {"not-a-strategy": 1}, sparse=sparse)
    with pytest.raises(ValueError, match="not a strategy label"):
        profile[P1] = {"not-a-strategy": 1}


def test_setitem_empty_distribution_is_all_zero_error():
    game = games.read_from_file("coordination_4x4_payoff.nfg")
    profile = game.mixed_strategy_profile()
    with pytest.raises(ValueError, match="zero"):
        profile[P1] = {}


@pytest.mark.parametrize("sparse", [False, True])
def test_setitem_and_set_strategy_reject_non_mapping(sparse: bool):
    game = games.read_from_file("coordination_4x4_payoff.nfg")
    profile = game.mixed_strategy_profile()
    with pytest.raises(TypeError, match="Mapping"):
        profile.set_strategy(P1, [1, 0, 0, 0], sparse=sparse)
    with pytest.raises(TypeError, match="Mapping"):
        profile[P1] = [1, 0, 0, 0]


@pytest.mark.parametrize("sparse", [False, True])
def test_setitem_and_set_strategy_reject_uncoercible_weight(sparse: bool):
    game = games.read_from_file("coordination_4x4_payoff.nfg")
    profile = game.mixed_strategy_profile()
    full_distribution = {"1": "abc", "2": 0, "3": 0, "4": 0}
    with pytest.raises(ValueError, match="convert"):
        profile.set_strategy(P1, full_distribution, sparse=sparse)
    with pytest.raises(ValueError, match="convert"):
        profile[P1] = full_distribution


def test_setitem_sparse_rejects_negative_weight():
    """Negativity is checked even for weights given under a sparse distribution."""
    game = games.read_from_file("coordination_4x4_payoff.nfg")
    profile = game.mixed_strategy_profile()
    with pytest.raises(ValueError, match="negative"):
        profile[P1] = {"1": -1}


@pytest.mark.parametrize("sparse", [False, True])
def test_indexing_rejects_player_object(sparse: bool):
    """Unlike Game._resolve_player, MixedStrategyProfile's indexing is str-label only."""
    game = games.read_from_file("coordination_4x4_payoff.nfg")
    profile = game.mixed_strategy_profile()
    player = game.players[P1]
    with pytest.raises(TypeError):
        profile[player]
    with pytest.raises(TypeError):
        profile[player] = {"1": 1}
    with pytest.raises(TypeError):
        profile.set_strategy(player, {"1": 1}, sparse=sparse)


@pytest.mark.parametrize("rational_flag", [False, True])
def test_copy_mutating_copy_does_not_affect_original(rational_flag: bool):
    game = games.read_from_file("coordination_4x4_payoff.nfg")
    original = game.mixed_strategy_profile(rational=rational_flag)
    original_before = dict(original[P1])
    copy = original.copy()
    copy[P1] = {"1": 1}
    assert dict(original[P1]) == original_before
    assert dict(copy[P1]) == {"1": 1, "2": 0, "3": 0, "4": 0}


@pytest.mark.parametrize("rational_flag", [False, True])
def test_copy_mutating_original_does_not_affect_copy(rational_flag: bool):
    game = games.read_from_file("coordination_4x4_payoff.nfg")
    original = game.mixed_strategy_profile(rational=rational_flag)
    copy = original.copy()
    copy_before = dict(copy[P1])
    original[P1] = {"1": 1}
    assert dict(copy[P1]) == copy_before
    assert dict(original[P1]) == {"1": 1, "2": 0, "3": 0, "4": 0}


@pytest.mark.parametrize(
    "game,player_label,strategy_label,prob,rational_flag",
    [
        ##############################################################################
        # stripped-down poker efg
        # Player 1
        (games.create_stripped_down_poker_efg(), "Alice", "1", 0.25, False),
        (games.create_stripped_down_poker_efg(), "Alice", "2", 0.25, False),
        (games.create_stripped_down_poker_efg(), "Alice", "3", 0.25, False),
        (games.create_stripped_down_poker_efg(), "Alice", "4", 0.25, False),
        (games.create_stripped_down_poker_efg(), "Alice", "1", "1/4", True),
        (games.create_stripped_down_poker_efg(), "Alice", "2", "1/4", True),
        (games.create_stripped_down_poker_efg(), "Alice", "3", "1/4", True),
        (games.create_stripped_down_poker_efg(), "Alice", "4", "1/4", True),
        # Player 2
        (games.create_stripped_down_poker_efg(), "Bob", "1", 0.5, False),
        (games.create_stripped_down_poker_efg(), "Bob", "2", 0.5, False),
        (games.create_stripped_down_poker_efg(), "Bob", "1", "1/2", True),
        (games.create_stripped_down_poker_efg(), "Bob", "2", "1/2", True),
        ##############################################################################
        # coordination 4x4 nfg outcome version with strategy labels
        (games.read_from_file("coordination_4x4_outcome.nfg"), P1, "1-1", "1/4", True),
        (games.read_from_file("coordination_4x4_outcome.nfg"), P2, "2-1", "1/4", True),
    ],
)
def test_profile_indexing_by_player_and_strategy_label_reference(
    game: gbt.Game, player_label: str, strategy_label: str, prob: str | float, rational_flag: bool
):
    profile = game.mixed_strategy_profile(rational=rational_flag)
    prob = gbt.Rational(prob) if rational_flag else prob
    assert profile[player_label][strategy_label] == prob


@pytest.mark.parametrize(
    "game,player_label,strategy_label,rational_flag",
    [
        ##############################################################################
        # stripped-down poker efg
        (games.create_stripped_down_poker_efg(), "Bob", "11", True),
        (games.create_stripped_down_poker_efg(), "Bob", "11", False),
        (games.create_stripped_down_poker_efg(), "Alice", "99", True),
        (games.create_stripped_down_poker_efg(), "Alice", "99", False),
        (games.create_stripped_down_poker_efg(), "Bob", "99", True),
        (games.create_stripped_down_poker_efg(), "Bob", "99", False),
        ##############################################################################
        # coordination 4x4 nfg outcome version with strategy labels
        (games.read_from_file("coordination_4x4_outcome.nfg"), P1, "2-1", True),
        (games.read_from_file("coordination_4x4_outcome.nfg"), P2, "1-1", True),
    ],
)
def test_profile_indexing_by_player_and_invalid_strategy_label(
    game: gbt.Game, player_label: str, strategy_label: str, rational_flag: bool
):
    """Test that we get a KeyError and that "player" appears in the error message"""
    with pytest.raises(KeyError, match="for player"):
        game.mixed_strategy_profile(rational=rational_flag)[player_label][strategy_label]


@pytest.mark.parametrize(
    "game,label,rational_flag",
    [
        ##############################################################################
        # stripped-down poker efg: not a label of anything in the game
        (games.create_stripped_down_poker_efg(), "13", True),
        (games.create_stripped_down_poker_efg(), "13", False),
        ##############################################################################
        # coordination 4x4 nfg outcome version: a strategy label, not a player label
        (games.read_from_file("coordination_4x4_outcome.nfg"), "1-1", True),
        (games.read_from_file("coordination_4x4_outcome.nfg"), "1-1", False),
    ],
)
def test_profile_indexing_by_invalid_player_label(
    game: gbt.Game, label: str, rational_flag: bool
):
    """MixedStrategyProfile.__getitem__ only resolves player labels; anything else, including a
    strategy label, raises KeyError.
    """
    with pytest.raises(KeyError):
        game.mixed_strategy_profile(rational=rational_flag)[label]


@pytest.mark.parametrize(
    "game,player_label,strategy_data,rational_flag",
    [
        ############################################################################
        # mixed behav efg
        (games.read_from_file("mixed_behavior_game.efg"), P1, [0.5, 0.5], False),
        (games.read_from_file("mixed_behavior_game.efg"), P2, [0.5, 0.5], False),
        (games.read_from_file("mixed_behavior_game.efg"), P3, [0.5, 0.5], False),
        (games.read_from_file("mixed_behavior_game.efg"), P1, ["1/2", "1/2"], True),
        (games.read_from_file("mixed_behavior_game.efg"), P2, ["1/2", "1/2"], True),
        (games.read_from_file("mixed_behavior_game.efg"), P3, ["1/2", "1/2"], True),
        ############################################################################
        # stripped-down poker efg
        (games.create_stripped_down_poker_efg(), "Alice", [0.25, 0.25, 0.25, 0.25], False),
        (games.create_stripped_down_poker_efg(), "Bob", [0.5, 0.5], False),
        (games.create_stripped_down_poker_efg(), "Alice", ["1/4", "1/4", "1/4", "1/4"], True),
        (games.create_stripped_down_poker_efg(), "Bob", ["1/2", "1/2"], True),
        ############################################################################
        # coordination 4x4 nfg
        (games.read_from_file("coordination_4x4_payoff.nfg"), P1, [0.25, 0.25, 0.25, 0.25], False),
        (games.read_from_file("coordination_4x4_payoff.nfg"), P2, [0.25, 0.25, 0.25, 0.25], False),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            P1,
            ["1/4", "1/4", "1/4", "1/4"],
            True,
        ),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            P2,
            ["1/4", "1/4", "1/4", "1/4"],
            True,
        ),
    ],
)
def test_profile_indexing_by_player_label_reference(
    game: gbt.Game, player_label: str, strategy_data: list, rational_flag: bool
):
    profile = game.mixed_strategy_profile(rational=rational_flag)
    if rational_flag:
        strategy_data = [gbt.Rational(prob) for prob in strategy_data]
    player = game.players[player_label]
    expected = dict(zip((s.label for s in player.strategies), strategy_data, strict=True))
    assert profile[player_label] == expected


@pytest.mark.parametrize(
    "game,rational_flag",
    [
        (games.read_from_file("mixed_behavior_game.efg"), False),
        (games.read_from_file("mixed_behavior_game.efg"), True),
        (games.read_from_file("cent3.efg"), False),
        (games.read_from_file("cent3.efg"), True),
    ],
)
def test_as_behavior_roundtrip(game: gbt.Game, rational_flag: bool):
    assert game.mixed_strategy_profile(
        rational=rational_flag
    ).as_behavior().as_strategy() == game.mixed_strategy_profile(rational=rational_flag)


@pytest.mark.parametrize(
    "game,rational_flag",
    [
        (games.read_from_file("2x2_bimatrix_all_zero_payoffs.nfg"), False),
        (games.read_from_file("2x2_bimatrix_all_zero_payoffs.nfg"), True),
        (games.read_from_file("coordination_4x4_payoff.nfg"), False),
        (games.read_from_file("coordination_4x4_payoff.nfg"), True),
    ],
)
def test_as_behavior_error(game: gbt.Game, rational_flag: bool):
    with pytest.raises(gbt.UndefinedOperationError):
        _ = game.mixed_strategy_profile(rational=rational_flag).as_behavior()


@pytest.mark.parametrize(
    "game,profile_data,rational_flag,payoffs",
    [
        ###############################################################################
        # zero matrix nfg
        (games.read_from_file("2x2_bimatrix_all_zero_payoffs.nfg"), None, False, (0, 0)),
        (games.read_from_file("2x2_bimatrix_all_zero_payoffs.nfg"), None, True, (0, 0)),
        ###############################################################################
        # 4x4 coordination nfg
        (games.read_from_file("coordination_4x4_payoff.nfg"), None, False, (0.25, 0.25)),
        (games.read_from_file("coordination_4x4_payoff.nfg"), None, True, ("1/4", "1/4")),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [["1/3", "1/3", "1/3", 0], ["1/3", "1/3", "1/3", 0]],
            True,
            ("1/3", "1/3"),
        ),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [["1/3", "1/3", 0, "1/3"], ["1/3", "1/3", "1/3", 0]],
            True,
            ("2/9", "2/9"),
        ),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [[1, 0, 0, 0], [1, 0, 0, 0]],
            False,
            (1, 1),
        ),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [[1, 0, 0, 0], [1, 0, 0, 0]],
            True,
            (1, 1),
        ),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [[1, 0, 0, 0], [0, 1, 0, 0]],
            False,
            (0, 0),
        ),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [[1, 0, 0, 0], [0, 1, 0, 0]],
            True,
            (0, 0),
        ),
        ###############################################################################
        # 2x2x2 nfg
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            None,
            True,
            ("4/8", "16/8", "4/8"),
        ),
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [[1, 0], [1, 0], [1, 0]],
            True,
            (0, 0, 0),
        ),
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [[0, 1], [1, 0], [1, 0]],
            True,
            (1, 2, -1),
        ),
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [["1/2", "1/2"], [1, 0], [1, 0]],
            True,
            ("1/2", 1, "-1/2"),
        ),
        ###############################################################################
        # stripped-down poker efg
        (games.create_stripped_down_poker_efg(), None, False, (-0.25, 0.25)),
        (games.create_stripped_down_poker_efg(), None, True, ("-1/4", "1/4")),
        # Bet/Call
        (games.create_stripped_down_poker_efg(), [[1, 0, 0, 0], [1, 0]], False, (0, 0)),
        (games.create_stripped_down_poker_efg(), [[1, 0, 0, 0], [1, 0]], True, (0, 0)),
        # Fold/Fold for player 1 (player 2's strategy is payoff-irrelevant)
        (games.create_stripped_down_poker_efg(), [[0, 0, 0, 1], [1, 0]], False, (-1, 1)),
        (games.create_stripped_down_poker_efg(), [[0, 0, 0, 1], [1, 0]], True, (-1, 1)),
        (games.create_stripped_down_poker_efg(), [[0, 0, 0, 1], [0.5, 0.5]], False, (-1, 1)),
        (
            games.create_stripped_down_poker_efg(),
            [[0, 0, 0, 1], ["1/2", "1/2"]],
            True,
            (-1, 1),
        ),
        ###############################################################################
        # mixed behavior efg (3 players)
        (games.read_from_file("mixed_behavior_game.efg"), None, False, (3.0, 3.0, 3.25)),
        (games.read_from_file("mixed_behavior_game.efg"), None, True, (3, 3, "13/4")),
    ],
)
def test_payoffs_reference(
    game: gbt.Game, profile_data: list, rational_flag: bool, payoffs: tuple
):
    profile = game.mixed_strategy_profile(rational=rational_flag, data=profile_data)
    for payoff, player in zip(payoffs, profile.game.players, strict=True):
        payoff = gbt.Rational(payoff) if rational_flag else payoff
        assert profile.payoffs[player.label] == payoff


@pytest.mark.parametrize(
    "game,profile_data,rational_flag,strategy_values",
    [
        ###############################################################################
        # zero matrix nfg
        (games.read_from_file("2x2_bimatrix_all_zero_payoffs.nfg"), None, False, ([0, 0], [0, 0])),
        (games.read_from_file("2x2_bimatrix_all_zero_payoffs.nfg"), None, True, ([0, 0], [0, 0])),
        ###############################################################################
        # 4x4 coordination nfg
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            None,
            False,
            ([0.25, 0.25, 0.25, 0.25], [0.25, 0.25, 0.25, 0.25]),
        ),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            None,
            True,
            ([0.25, 0.25, 0.25, 0.25], [0.25, 0.25, 0.25, 0.25]),
        ),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [["1", "0", "0", "0"], ["1", "0", "0", "0"]],
            True,
            (["1", "0", "0", "0"], ["1", "0", "0", "0"]),
        ),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [["3/7", "0", "0", "4/7"], ["1/3", "1/3", "1/3", "0"]],
            True,
            (["1/3", "1/3", "1/3", "0"], ["3/7", "0", "0", "4/7"]),
        ),
        ###############################################################################
        # 2x2x2 nfg
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            None,
            True,
            (["1/2", "1/2"], [2, 2], ["1/2", "1/2"]),
        ),
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [[1, 0], [1, 0], [1, 0]],
            True,
            ([0, 1], [0, 4], [0, 1]),
        ),
        ###############################################################################
        # coordination 4x4 nfg, outcome-mapped version
        (
            games.read_from_file("coordination_4x4_outcome.nfg"),
            None,
            False,
            ([0.25, 0.25, 0.25, 0.25], [0.25, 0.25, 0.25, 0.25]),
        ),
        (
            games.read_from_file("coordination_4x4_outcome.nfg"),
            None,
            True,
            (["1/4", "1/4", "1/4", "1/4"], ["1/4", "1/4", "1/4", "1/4"]),
        ),
        ###############################################################################
        # stripped-down poker efg
        (games.create_stripped_down_poker_efg(), None, False, ((0.5, 0.25, -0.75, -1), (0.5, 0))),
        (
            games.create_stripped_down_poker_efg(),
            None,
            True,
            (("1/2", "1/4", "-3/4", -1), ("1/2", 0)),
        ),
    ],
)
def test_strategy_value_reference(
    game: gbt.Game, profile_data: list, rational_flag: bool, strategy_values: list
):
    profile = game.mixed_strategy_profile(rational=rational_flag, data=profile_data)
    for strategy_values_for_player, player in zip(
        strategy_values, profile.game.players, strict=True
    ):
        for i, s in enumerate(player.strategies):
            sv = strategy_values_for_player[i]
            sv = gbt.Rational(sv) if rational_flag else sv
            assert profile.strategy_values[player.label][s.label] == sv


@pytest.mark.parametrize(
    "game,profile_data,liap_exp,tol,rational_flag",
    [
        ##############################################################################
        # Zero matrix nfg, all liap_values are zero
        (
            games.read_from_file("2x2_bimatrix_all_zero_payoffs.nfg"),
            [["3/4", "1/4"], ["2/5", "3/5"]],
            0,
            ZERO,
            True,
        ),
        (
            games.read_from_file("2x2_bimatrix_all_zero_payoffs.nfg"),
            [["1/2", "1/2"], ["1/2", "1/2"]],
            0,
            ZERO,
            True,
        ),
        (
            games.read_from_file("2x2_bimatrix_all_zero_payoffs.nfg"),
            [[1, 0], [1, 0]],
            0,
            ZERO,
            True,
        ),
        (
            games.read_from_file("2x2_bimatrix_all_zero_payoffs.nfg"),
            [[1 / 4, 3 / 4], [2 / 5, 3 / 5]],
            0,
            TOL,
            False,
        ),
        ##############################################################################
        # 4x4 coordination nfg
        (games.read_from_file("coordination_4x4_payoff.nfg"), None, 0, ZERO, True),
        (games.read_from_file("coordination_4x4_payoff.nfg"), None, 0, TOL, False),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [[1, 0, 0, 0], [1, 0, 0, 0]],
            0,
            ZERO,
            True,
        ),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [[1, 0, 0, 0], [1, 0, 0, 0]],
            0,
            TOL,
            False,
        ),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [["1/3", "1/2", "1/12", "1/12"], ["3/8", "1/8", "1/4", "1/4"]],
            "245/2304",
            ZERO,
            True,
        ),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [[1 / 3, 1 / 2, 1 / 12, 1 / 12], [3 / 8, 1 / 8, 1 / 4, 1 / 4]],
            245 / 2304,
            TOL,
            False,
        ),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [["1/3", 0, 0, "2/3"], [1, 0, 0, 0]],
            "5/9",
            ZERO,
            True,
        ),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [[1 / 3, 0, 0, 2 / 3], [1, 0, 0, 0]],
            5 / 9,
            TOL,
            False,
        ),
        ##############################################################################
        # El Farol bar game efg
        (
            games.read_from_file("el_farol_bar.efg"),
            [["1/2", "1/2"], ["1/2", "1/2"], ["1/2", "1/2"], ["1/2", "1/2"], ["1/2", "1/2"]],
            0,
            ZERO,
            True,
        ),
        (
            games.read_from_file("el_farol_bar.efg"),
            [[1, 0], [1, 0], [0, 1], [0, 1], [0, 1]],
            0,
            ZERO,
            True,
        ),
        ##############################################################################
        # # 2x2x2 nfg with 2 pure and 1 mixed eq
        # Pure non-Nash eq:
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [[1, 0], [1, 0], [1, 0]],
            18,
            ZERO,
            True,
        ),  # 4^2+1+1
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [[0, 1], [0, 1], [0, 1]],
            18,
            ZERO,
            True,
        ),  # 4^2+1+1
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [[1, 0], [0, 1], [0, 1]],
            9,
            ZERO,
            True,
        ),  # 3^2
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [[0, 1], [1, 0], [1, 0]],
            9,
            ZERO,
            True,
        ),  # 3^2
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [[1, 0], [0, 1], [0, 1]],
            9,
            ZERO,
            True,
        ),  # 3^2
        # Non-pure non-Nash eq:
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [["1/2", "1/2"], [1, 0], [1, 0]],
            "33/4",
            ZERO,
            True,
        ),
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [[1, 0], ["1/2", "1/2"], [1, 0]],
            4,
            ZERO,
            True,
        ),
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [[1, 0], [1, 0], ["1/2", "1/2"]],
            "33/4",
            ZERO,
            True,
        ),
        # Nash eq:
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [[1, 0], [0, 1], [1, 0]],
            0,
            ZERO,
            True,
        ),
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [[0, 1], [1, 0], [0, 1]],
            0,
            ZERO,
            True,
        ),
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            None,
            0,
            ZERO,
            True,
        ),  # uniform is Nash
    ],
)
def test_liap_value_reference(
    game: gbt.Game,
    profile_data: list,
    liap_exp: float | str,
    tol: float | gbt.Rational | int,
    rational_flag: bool,
):
    profile = game.mixed_strategy_profile(rational=rational_flag, data=profile_data)
    liap_exp = gbt.Rational(liap_exp) if rational_flag else liap_exp
    assert abs(profile.liap_value() - liap_exp) <= tol


@pytest.mark.parametrize(
    "game,profile_data,player_regrets_exp,tol,rational_flag",
    [
        ##############################################################################
        # Zero matrix nfg, all liap_values are zero
        (
            games.read_from_file("2x2_bimatrix_all_zero_payoffs.nfg"),
            [["3/4", "1/4"], ["2/5", "3/5"]],
            [0] * 2,
            ZERO,
            True,
        ),
        (
            games.read_from_file("2x2_bimatrix_all_zero_payoffs.nfg"),
            [["1/2", "1/2"], ["1/2", "1/2"]],
            [0] * 2,
            ZERO,
            True,
        ),
        (
            games.read_from_file("2x2_bimatrix_all_zero_payoffs.nfg"),
            [[1, 0], [1, 0]],
            [0] * 2,
            ZERO,
            True,
        ),
        (
            games.read_from_file("2x2_bimatrix_all_zero_payoffs.nfg"),
            [[1 / 4, 3 / 4], [2 / 5, 3 / 5]],
            [0] * 2,
            TOL,
            False,
        ),
        ##############################################################################
        # 4x4 coordination nfg
        (games.read_from_file("coordination_4x4_payoff.nfg"), None, [0] * 2, ZERO, True),
        (games.read_from_file("coordination_4x4_payoff.nfg"), None, [0] * 2, TOL, False),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [[1, 0, 0, 0], [1, 0, 0, 0]],
            [0] * 2,
            ZERO,
            True,
        ),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [[1, 0, 0, 0], [1, 0, 0, 0]],
            [0] * 2,
            TOL,
            False,
        ),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [["1/3", "1/2", "1/12", "1/12"], ["3/8", "1/8", "1/4", "1/4"]],
            ["7/48", "13/48"],
            ZERO,
            True,
        ),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [[1 / 3, 1 / 2, 1 / 12, 1 / 12], [3 / 8, 1 / 8, 1 / 4, 1 / 4]],
            [7 / 48, 13 / 48],
            TOL,
            False,
        ),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [["1/3", 0, 0, "2/3"], [1, 0, 0, 0]],
            ["2/3", "1/3"],
            ZERO,
            True,
        ),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [[1 / 3, 0, 0, 2 / 3], [1, 0, 0, 0]],
            [2 / 3, 1 / 3],
            TOL,
            False,
        ),
        ##############################################################################
        # El Farol bar game efg
        (
            games.read_from_file("el_farol_bar.efg"),
            [["1/2", "1/2"], ["1/2", "1/2"], ["1/2", "1/2"], ["1/2", "1/2"], ["1/2", "1/2"]],
            [0] * 5,
            ZERO,
            True,
        ),
        (
            games.read_from_file("el_farol_bar.efg"),
            [[1, 0], [1, 0], [0, 1], [0, 1], [0, 1]],
            [0] * 5,
            ZERO,
            True,
        ),
        ##############################################################################
        # 2x2x2 nfg with 2 pure and 1 mixed eq
        # Pure non-Nash
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [[1, 0], [1, 0], [1, 0]],
            [1, 4, 1],
            ZERO,
            True,
        ),  # 111
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [[0, 1], [0, 1], [0, 1]],
            [1, 4, 1],
            ZERO,
            True,
        ),  # 000
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [[1, 0], [0, 1], [0, 1]],
            [0, 0, 3],
            ZERO,
            True,
        ),  # 100
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [[0, 1], [1, 0], [1, 0]],
            [0, 0, 3],
            ZERO,
            True,
        ),  # 011
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [[0, 1], [0, 1], [1, 0]],
            [3, 0, 0],
            ZERO,
            True,
        ),  # 001
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [[1, 0], [1, 0], [0, 1]],
            [3, 0, 0],
            ZERO,
            True,
        ),  # 110
        # Mixed non-Nash
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [["1/2", "1/2"], [1, 0], [1, 0]],
            ["1/2", 2, 2],
            ZERO,
            True,
        ),
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [[1, 0], ["1/2", "1/2"], [1, 0]],
            [0, 2, 0],
            ZERO,
            True,
        ),
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [[1, 0], [1, 0], ["1/2", "1/2"]],
            [2, 2, "1/2"],
            ZERO,
            True,
        ),
        # Nash eq:
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [[1, 0], [0, 1], [1, 0]],
            [0] * 3,
            ZERO,
            True,
        ),  # 101
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [[0, 1], [1, 0], [0, 1]],
            [0] * 3,
            ZERO,
            True,
        ),  # 010
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            None,
            [0] * 3,
            ZERO,
            True,
        ),  # uniform is Nash
    ],
)
def test_player_regret_max_regret_reference(
    game: gbt.Game,
    profile_data: list,
    player_regrets_exp: list,
    tol: float | gbt.Rational | int,
    rational_flag: bool,
):
    profile = game.mixed_strategy_profile(rational=rational_flag, data=profile_data)
    if rational_flag:
        player_regrets_exp = [gbt.Rational(r) for r in player_regrets_exp]
    player_regrets = profile.player_regrets
    for p, r in zip(game.players, player_regrets_exp, strict=True):
        assert abs(player_regrets[p.label] - r) <= tol
    assert abs(profile.max_regret() - max(player_regrets_exp)) <= tol


@pytest.mark.parametrize(
    "game,rational_flag",
    [
        #################################################################################
        # 4x4 coordination nfg
        (games.read_from_file("coordination_4x4_payoff.nfg"), False),
        (games.read_from_file("coordination_4x4_payoff.nfg"), True),
        #################################################################################
        # Zero matrix nfg
        (games.read_from_file("2x2_bimatrix_all_zero_payoffs.nfg"), False),
        (games.read_from_file("2x2_bimatrix_all_zero_payoffs.nfg"), True),
        #################################################################################
        # El Farol bar game efg
        (games.read_from_file("el_farol_bar.efg"), False),
        (games.read_from_file("el_farol_bar.efg"), True),
        #################################################################################
        # Centipede with chance efg
        (games.read_from_file("cent3.efg"), False),
        (games.read_from_file("cent3.efg"), True),
        #################################################################################
        # 2x2x2 nfg
        (games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"), False),
        (games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"), True),
    ],
)
def test_strategy_regret_consistency(game: gbt.Game, rational_flag: bool):
    profile = game.mixed_strategy_profile(rational=rational_flag)
    strategy_values = profile.strategy_values
    strategy_regrets = profile.strategy_regrets
    for player in game.players:
        player_strategy_values = strategy_values[player.label]
        for strategy in player.strategies:
            assert strategy_regrets[player.label][strategy.label] == (
                max(player_strategy_values[s.label] for s in player.strategies)
                - player_strategy_values[strategy.label]
            )


@pytest.mark.parametrize(
    "game,profile_data,tol,rational_flag",
    [
        #################################################################################
        # 4x4 coordination nfg
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [["1/5", "2/5", "0/5", "2/5"], ["3/8", "1/4", "3/8", "0/4"]],
            ZERO,
            True,
        ),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [[1 / 3, 1 / 3, 0 / 3, 1 / 3], [1 / 4, 1 / 4, 3 / 8, 1 / 8]],
            TOL,
            False,
        ),
        #################################################################################
        # Centipede with chance efg
        (
            games.read_from_file("cent3.efg"),
            [["1/3", "1/3", "1/3", "0/1"], ["1/10", "3/5", "3/10", 0]],
            ZERO,
            True,
        ),
        (
            games.read_from_file("cent3.efg"),
            [[1 / 3, 1 / 3, 1 / 3, 0], [0.10, 3 / 5, 0.3, 0]],
            TOL,
            False,
        ),
        #################################################################################
        # El Farol bar game efg
        (
            games.read_from_file("el_farol_bar.efg"),
            [[1, 0], ["1/2", "1/2"], ["1/3", "2/3"], ["1/5", "4/5"], ["1/8", "7/8"]],
            ZERO,
            True,
        ),
        (
            games.read_from_file("el_farol_bar.efg"),
            [[1, 0], [1 / 2, 1 / 2], [1 / 3, 2 / 3], [1 / 5, 4 / 5], [1 / 8, 7 / 8]],
            TOL,
            False,
        ),
        #################################################################################
        # 2x2x2 nfg with 2 pure and 1 mixed eq
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            None,
            ZERO,
            True,
        ),
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [[1, 0], [1, 0], [1, 0]],
            ZERO,
            True,
        ),
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            None,
            TOL,
            False,
        ),
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [[1, 0], [1, 0], [1, 0]],
            TOL,
            False,
        ),
    ],
)
def test_liap_value_consistency(
    game: gbt.Game, profile_data: list, tol: float | gbt.Rational, rational_flag: bool
):
    profile = game.mixed_strategy_profile(rational=rational_flag, data=profile_data)
    strategy_values = profile.strategy_values
    payoffs = profile.payoffs

    assert (
        abs(
            profile.liap_value()
            - sum(
                [
                    max(strategy_values[player.label][strategy.label] - payoffs[player.label], 0)
                    ** 2
                    for player in game.players
                    for strategy in player.strategies
                ]
            )
        )
        <= tol
    )


@pytest.mark.parametrize(
    "game,profile_data,tol,rational_flag",
    [
        #################################################################################
        # 4x4 coordination nfg
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [["1/5", "2/5", "0/5", "2/5"], ["3/8", "1/4", "3/8", "0/4"]],
            ZERO,
            True,
        ),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [[1 / 3, 1 / 3, 0 / 3, 1 / 3], [1 / 4, 1 / 4, 3 / 8, 1 / 8]],
            TOL,
            False,
        ),
        #################################################################################
        # Centipede with chance efg
        (
            games.read_from_file("cent3.efg"),
            [["1/3", "1/3", "1/3", "0/1"], ["1/10", "3/5", "3/10", 0]],
            ZERO,
            True,
        ),
        (
            games.read_from_file("cent3.efg"),
            [[1 / 3, 1 / 3, 1 / 3, 0], [0.10, 3 / 5, 0.3, 0]],
            TOL,
            False,
        ),
        #################################################################################
        # El Farol bar game efg
        (
            games.read_from_file("el_farol_bar.efg"),
            [[1, 0], ["1/2", "1/2"], ["1/3", "2/3"], ["1/5", "4/5"], ["1/8", "7/8"]],
            ZERO,
            True,
        ),
        (
            games.read_from_file("el_farol_bar.efg"),
            [[1, 0], [1 / 2, 1 / 2], [1 / 3, 2 / 3], [1 / 5, 4 / 5], [1 / 8, 7 / 8]],
            TOL,
            False,
        ),
        #################################################################################
        # 2x2x2 nfg with 2 pure and 1 mixed eq
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            None,
            ZERO,
            True,
        ),
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [[1, 0], [1, 0], [1, 0]],
            ZERO,
            True,
        ),
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            None,
            TOL,
            False,
        ),
        (
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            [[1, 0], [1, 0], [1, 0]],
            TOL,
            False,
        ),
    ],
)
def test_player_regret_max_regret_consistency(
    game: gbt.Game, profile_data: list, tol: float | gbt.Rational, rational_flag: bool
):
    profile = game.mixed_strategy_profile(rational=rational_flag, data=profile_data)
    strategy_values = profile.strategy_values
    payoffs = profile.payoffs
    player_regrets = []
    for p in game.players:
        p_regret = max(
            [
                max(strategy_values[p.label][strategy.label] - payoffs[p.label], 0)
                for strategy in p.strategies
            ]
        )
        player_regrets.append(p_regret)
        assert abs(profile.player_regrets[p.label] - p_regret) <= tol
    assert abs(profile.max_regret() - max(player_regrets)) <= tol


@pytest.mark.parametrize(
    "game,profile1,profile2,alpha,tol,rational_flag",
    [
        #################################################################################
        # 4x4 coordination nfg
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [["1/5", "2/5", "0/5", "2/5"], ["3/8", "1/4", "3/8", "0/4"]],
            [["1/5", "2/5", "0/5", "2/5"], ["1/4", "3/8", "0/4", "3/8"]],
            gbt.Rational("3/5"),
            ZERO,
            True,
        ),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [[1 / 5, 2 / 5, 0 / 5, 2 / 5], [3 / 8, 1 / 4, 3 / 8, 0 / 4]],
            [[1 / 5, 2 / 5, 0 / 5, 2 / 5], [1 / 4, 3 / 8, 0 / 4, 3 / 8]],
            3 / 5,
            TOL,
            False,
        ),
        #################################################################################
        # Zero matrix nfg
        (
            games.read_from_file("2x2_bimatrix_all_zero_payoffs.nfg"),
            [["1/4", "3/4"], ["3/5", "2/5"]],
            [["1/2", "1/2"], ["3/5", "2/5"]],
            gbt.Rational("5/6"),
            ZERO,
            True,
        ),
        #################################################################################
        # Centipede game with chance
        (
            games.read_from_file("cent3.efg"),
            [["1/3", "1/3", "1/3", "0/1"], ["1/10", "3/5", "3/10", "0/1"]],
            [["1/3", "1/3", "1/3", "0/1"], ["1/5", "2/5", "1/5", "1/5"]],
            gbt.Rational("1/12"),
            ZERO,
            True,
        ),
        (
            games.read_from_file("cent3.efg"),
            [[1 / 3, 1 / 3, 1 / 3, 0 / 1], [1 / 10, 3 / 5, 3 / 10, 0 / 1]],
            [[1 / 3, 1 / 3, 1 / 3, 0 / 1], [1 / 5, 2 / 5, 1 / 5, 1 / 5]],
            1 / 12,
            TOL,
            False,
        ),
        #################################################################################
        # Selten's horse
        (
            gbt.catalog.load("journals/ijgt/selten1975/fig1"),
            [["4/9", "5/9"], ["1/11", "10/11"], ["8/9", "1/9"]],
            [["4/9", "5/9"], ["10/11", "1/11"], ["8/9", "1/9"]],
            gbt.Rational("4/9"),
            ZERO,
            True,
        ),
        #################################################################################
        # El Farol bar game
        (
            games.read_from_file("el_farol_bar.efg"),
            [["4/9", "5/9"], ["1/3", "2/3"], ["1/2", "1/2"], ["11/12", "1/12"], ["1/2", "1/2"]],
            [["4/9", "5/9"], ["1/3", "2/3"], ["1/2", "1/2"], ["1/12", "11/12"], ["1/2", "1/2"]],
            gbt.Rational("1/2"),
            ZERO,
            True,
        ),
    ],
)
def test_linearity_payoff_property(
    game: gbt.Game,
    profile1: list,
    profile2: list,
    alpha: float | gbt.Rational,
    tol: float | gbt.Rational,
    rational_flag: bool,
):
    profile1 = game.mixed_strategy_profile(rational=rational_flag, data=profile1)
    profile2 = game.mixed_strategy_profile(rational=rational_flag, data=profile2)

    profile_data = [
        [
            alpha * profile1[player.label][strategy.label]
            + (1 - alpha) * profile2[player.label][strategy.label]
            for strategy in player.strategies
        ]
        for player in game.players
    ]
    profile3 = game.mixed_strategy_profile(rational=rational_flag, data=profile_data)

    payoffs1 = profile1.payoffs
    payoffs2 = profile2.payoffs
    payoffs3 = profile3.payoffs
    for player in game.players:
        assert (
            abs(
                alpha * payoffs1[player.label]
                + (1 - alpha) * payoffs2[player.label]
                - payoffs3[player.label]
            )
            <= tol
        )


@pytest.mark.parametrize(
    "game,profile_data,tol,rational_flag",
    [
        #################################################################################
        # 4x4 coordination nfg
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [["1/5", "2/5", "0/5", "2/5"], ["1/4", "3/8", "0/4", "3/8"]],
            ZERO,
            True,
        ),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [[0.2, 0.4, 0, 0.4], [1 / 4, 3 / 8, 0, 3 / 8]],
            TOL,
            False,
        ),
        (
            gbt.Game.from_arrays([[1, 2], [-3, 4]], [[-4, 3], [2, 1]]),
            [[1 / 2, 1 / 2], [3 / 5, 2 / 5]],
            TOL,
            False,
        ),
        #################################################################################
        # Zero matrix nfg
        (
            games.read_from_file("2x2_bimatrix_all_zero_payoffs.nfg"),
            [["4/5", "1/5"], ["4/7", "3/7"]],
            ZERO,
            True,
        ),
        #################################################################################
        # Centipede game with chance
        (
            games.read_from_file("cent3.efg"),
            [["1/5", "2/5", "1/5", "1/5"], ["1/10", "3/5", "3/10", "0/1"]],
            ZERO,
            True,
        ),
        (
            games.read_from_file("cent3.efg"),
            [[1 / 3, 1 / 3, 1 / 3, 0 / 1], [1 / 10, 3 / 5, 3 / 10, 0 / 1]],
            TOL,
            False,
        ),
        #################################################################################
        # Selten's horse
        (
            gbt.catalog.load("journals/ijgt/selten1975/fig1"),
            [["4/9", "5/9"], ["6/11", "5/11"], ["4/7", "3/7"]],
            ZERO,
            True,
        ),
        (
            gbt.catalog.load("journals/ijgt/selten1975/fig1"),
            [[4 / 9, 5 / 9], [6 / 11, 5 / 11], [4 / 7, 3 / 7]],
            TOL,
            False,
        ),
        #################################################################################
        # El Farol bar game
        (
            games.read_from_file("el_farol_bar.efg"),
            [["4/9", "5/9"], ["1/3", "2/3"], ["0/1", "1/1"], ["11/12", "1/12"], ["1/3", "2/3"]],
            ZERO,
            True,
        ),
    ],
)
def test_payoff_and_strategy_value_consistency(
    game: gbt.Game, profile_data: list, tol: float | gbt.Rational, rational_flag: bool
):
    profile = game.mixed_strategy_profile(rational=rational_flag, data=profile_data)
    strategy_values = profile.strategy_values
    payoffs = profile.payoffs
    for player in game.players:
        player_strategy_values = strategy_values[player.label]
        assert (
            abs(
                sum(
                    [
                        profile[player.label][strategy.label]
                        * player_strategy_values[strategy.label]
                        for strategy in player.strategies
                    ]
                )
                - payoffs[player.label]
            )
            <= tol
        )


@pytest.mark.parametrize(
    "game,rational_flag",
    [
        (games.read_from_file("coordination_4x4_payoff.nfg"), False),
        (games.read_from_file("coordination_4x4_payoff.nfg"), True),
        (games.create_stripped_down_poker_efg(), False),
        (games.create_stripped_down_poker_efg(), True),
    ],
)
def test_len_matches_iter_count(game: gbt.Game, rational_flag: bool):
    """len() is the number of players (not the total strategy count across players),
    and likewise for each player's MixedStrategy against their own strategies.
    """
    profile = game.mixed_strategy_profile(rational=rational_flag)
    assert len(profile) == len(game.players)
    assert len(profile) == len(list(profile))
    for player in game.players:
        mixed_strategy = profile[player.label]
        assert len(mixed_strategy) == len(player.strategies)
        assert len(mixed_strategy) == len(list(mixed_strategy))


def test_mixed_strategy_equality():
    # both players are uniform over 4 strategies: numerically identical, but distinct players
    game = games.read_from_file("coordination_4x4_payoff.nfg")
    profile = game.mixed_strategy_profile()
    p1_strategy = profile[P1]
    p2_strategy = profile[P2]

    assert dict(p1_strategy) == dict(p2_strategy)
    assert p1_strategy == {"1": 0.25, "2": 0.25, "3": 0.25, "4": 0.25}
    assert p1_strategy == profile[P1]  # a fresh snapshot for the same player
    assert p1_strategy != p2_strategy
    assert p1_strategy != 42


@pytest.mark.parametrize(
    "game,profile_data,rational_flag",
    [
        (games.read_from_file("coordination_4x4_payoff.nfg"), None, False),
        (games.read_from_file("coordination_4x4_payoff.nfg"), None, True),
        (games.create_stripped_down_poker_efg(), None, False),
        (games.create_stripped_down_poker_efg(), None, True),
    ],
)
def test_vectorized_quantities_consistency(game: gbt.Game, profile_data, rational_flag: bool):
    """The vectorized payoffs/player_regrets/strategy_values/strategy_regrets properties
    are mathematically consistent with each other (regret is the gap to the best
    response), and carry their own type identity.
    """
    profile = game.mixed_strategy_profile(rational=rational_flag, data=profile_data)

    payoffs = profile.payoffs
    player_regrets = profile.player_regrets
    strategy_values = profile.strategy_values
    strategy_regrets = profile.strategy_regrets

    assert isinstance(payoffs, gbt.PayoffVector)
    assert isinstance(payoffs, gbt.PlayerIndexedVector)
    assert isinstance(player_regrets, gbt.PlayerRegretVector)
    assert isinstance(strategy_values, gbt.StrategyValuesVector)
    assert isinstance(strategy_regrets, gbt.StrategyRegretsVector)

    for player in game.players:
        player_strategy_values = strategy_values[player.label]
        player_strategy_regrets = strategy_regrets[player.label]
        assert isinstance(player_strategy_values, gbt.StrategyValueVector)
        assert isinstance(player_strategy_values, gbt.StrategyIndexedVector)
        assert isinstance(player_strategy_regrets, gbt.StrategyRegretVector)

        best_response_value = max(player_strategy_values[s.label] for s in player.strategies)
        assert player_regrets[player.label] == best_response_value - payoffs[player.label]
        for strategy in player.strategies:
            assert (
                player_strategy_regrets[strategy.label]
                == best_response_value - player_strategy_values[strategy.label]
            )

    # equal to an equivalent plain dict or same-type vector, but never to a vector of a
    # different quantity, even where the underlying numbers happen to coincide
    expected = {p.label: payoffs[p.label] for p in game.players}
    assert payoffs == expected
    assert payoffs == gbt.PayoffVector(expected)
    assert payoffs != player_regrets
    assert payoffs != gbt.PlayerRegretVector(expected)

    with pytest.raises(KeyError):
        payoffs["not a player label"]


def test_repr_and_repr_latex_smoke():
    """Smoke test against silent formatting regressions, not a public API guarantee."""
    game = games.read_from_file("coordination_4x4_payoff.nfg")
    profile = game.mixed_strategy_profile(rational=True)

    assert repr(profile) == (
        "{'Player 1': {'1': Rational(1, 4), '2': Rational(1, 4), '3': Rational(1, 4), "
        "'4': Rational(1, 4)}, 'Player 2': {'1': Rational(1, 4), '2': Rational(1, 4), "
        "'3': Rational(1, 4), '4': Rational(1, 4)}}"
    )
    assert profile._repr_latex_() == (
        r"$\left\{\text{Player 1}:\left\{\text{1}:\frac{1}{4},\text{2}:\frac{1}{4},"
        r"\text{3}:\frac{1}{4},\text{4}:\frac{1}{4}\right\},\text{Player 2}:"
        r"\left\{\text{1}:\frac{1}{4},\text{2}:\frac{1}{4},\text{3}:\frac{1}{4},"
        r"\text{4}:\frac{1}{4}\right\}\right\}$"
    )

    mixed_strategy = profile[P1]
    assert repr(mixed_strategy) == (
        "{'1': Rational(1, 4), '2': Rational(1, 4), '3': Rational(1, 4), '4': Rational(1, 4)}"
    )
    assert mixed_strategy._repr_latex_() == (
        r"$\left\{\text{1}:\frac{1}{4},\text{2}:\frac{1}{4},"
        r"\text{3}:\frac{1}{4},\text{4}:\frac{1}{4}\right\}$"
    )

    # a flat vector (one value per player, no nesting)
    payoffs = profile.payoffs
    assert repr(payoffs) == "{'Player 1': Rational(1, 4), 'Player 2': Rational(1, 4)}"
    assert payoffs._repr_latex_() == (
        r"$\left\{\text{Player 1}:\frac{1}{4},\text{Player 2}:\frac{1}{4}\right\}$"
    )

    # a float-valued profile's _repr_latex_ falls back to repr(), since plain floats
    # (unlike Rational) don't know how to render themselves as LaTeX
    float_strategy = game.mixed_strategy_profile(rational=False)[P1]
    assert float_strategy._repr_latex_() == repr(float_strategy)


@pytest.mark.parametrize(
    "game,profile1,profile2,alpha,rational_flag,tol",
    [
        #################################################################################
        # 4x4 coordination nfg
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [["1/1111", "10/1111", "100/1111", "1000/1111"], ["1/4", "1/8", "3/8", "1/4"]],
            [["1/1111", "10/1111", "99/1111", "1001/1111"], ["1/4", "1/8", "3/8", "1/4"]],
            "1/2",
            True,
            ZERO,
        ),
        (
            games.read_from_file("coordination_4x4_payoff.nfg"),
            [[1 / 1111, 10 / 1111, 100 / 1111, 1000 / 1111], [1 / 4, 1 / 8, 3 / 8, 1 / 4]],
            [[1 / 1111, 10 / 1111, 99 / 1111, 1001 / 1111], [1 / 4, 1 / 8, 3 / 8, 1 / 4]],
            1 / 2,
            False,
            TOL,
        ),
        #################################################################################
        # centipede game with chance
        (
            games.read_from_file("cent3.efg"),
            [["1/3", "1/3", "1/3", "0"], ["1/10", "3/5", "3/10", "0"]],
            [["1/3", "1/3", "1/3", "0"], ["1/10", "3/5", "3/10", "0"]],
            "82943/62500",
            True,
            ZERO,
        ),
        (
            games.read_from_file("cent3.efg"),
            [[1 / 3, 1 / 3, 1 / 3, 0], [1 / 10, 3 / 5, 3 / 10, 0]],
            [[1 / 3, 1 / 3, 1 / 3, 0], [1 / 10, 3 / 5, 3 / 10, 0]],
            82943 / 62500,
            False,
            TOL,
        ),
    ],
)
def test_property_linearity_strategy_value(
    game: gbt.Game,
    profile1: list,
    profile2: list,
    alpha: float | str,
    rational_flag: bool,
    tol: float | gbt.Rational,
):
    alpha = gbt.Rational(alpha) if rational_flag else alpha

    profile1 = game.mixed_strategy_profile(rational=rational_flag, data=profile1)
    profile2 = game.mixed_strategy_profile(rational=rational_flag, data=profile2)

    profile_data = [
        [
            alpha * profile1[player.label][strategy.label]
            + (1 - alpha) * profile2[player.label][strategy.label]
            for strategy in player.strategies
        ]
        for player in game.players
    ]
    profile3 = game.mixed_strategy_profile(rational=rational_flag, data=profile_data)

    strategy_values1 = profile1.strategy_values
    strategy_values2 = profile2.strategy_values
    strategy_values3 = profile3.strategy_values
    for player in game.players:
        for strategy in player.strategies:
            convex_comb = (
                alpha * strategy_values1[player.label][strategy.label]
                + (1 - alpha) * strategy_values2[player.label][strategy.label]
            )
            assert abs(strategy_values3[player.label][strategy.label] - convex_comb) <= tol


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
    profile = game.mixed_strategy_profile(rational=rational_flag)
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


# For 4x4 coord nfg:
PROBS_1A_doub = (0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25)
PROBS_2A_doub = (0.5, 0, 0.5, 0, 0.5, 0, 0.5, 0)
PROBS_1A_rat = ("1/4", "1/4", "1/4", "1/4", "1/4", "1/4", "1/4", "1/4")
PROBS_2A_rat = ("1/2", "0", "1/2", "0", "1/2", "0", "1/2", "0")
# For 2x2x2 nfg and stripped_down_poker efg (both have 6 strategies in total):
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
        pytest.param(
            games.read_from_file("coordination_4x4_payoff.nfg"),
            PROBS_1A_doub,
            PROBS_2A_doub,
            False,
            lambda profile, player: profile.payoffs[player.label],
            lambda game: game.players,
            id="payoffs_coord_doub",
        ),
        pytest.param(
            games.read_from_file("coordination_4x4_payoff.nfg"),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda profile, player: profile.payoffs[player.label],
            lambda game: game.players,
            id="payoffs_coord_rat",
        ),
        # 2x2x2 nfg
        pytest.param(
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            PROBS_1B_doub,
            PROBS_2B_doub,
            False,
            lambda profile, player: profile.payoffs[player.label],
            lambda game: game.players,
            id="payoffs_2x2x2_doub",
        ),
        pytest.param(
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            PROBS_1B_rat,
            PROBS_2B_rat,
            True,
            lambda profile, player: profile.payoffs[player.label],
            lambda game: game.players,
            id="payoffs_2x2x2_rat",
        ),
        # stripped-down poker
        pytest.param(
            games.create_stripped_down_poker_efg(),
            PROBS_1B_doub,
            PROBS_2B_doub,
            False,
            lambda profile, player: profile.payoffs[player.label],
            lambda game: game.players,
            id="payoffs_poker_doub",
        ),
        pytest.param(
            games.create_stripped_down_poker_efg(),
            PROBS_1B_rat,
            PROBS_2B_rat,
            True,
            lambda profile, player: profile.payoffs[player.label],
            lambda game: game.players,
            id="payoffs_poker_rat",
        ),
        #################################################################################
        # regret (for strategies)
        # 4x4 coordination nfg
        pytest.param(
            games.read_from_file("coordination_4x4_payoff.nfg"),
            PROBS_1A_doub,
            PROBS_2A_doub,
            False,
            lambda profile, strategy: profile.strategy_regrets[strategy.player.label][
                strategy.label
            ],
            lambda game: game.strategies,
            id="regret_coord_doub",
        ),
        pytest.param(
            games.read_from_file("coordination_4x4_payoff.nfg"),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda profile, strategy: profile.strategy_regrets[strategy.player.label][
                strategy.label
            ],
            lambda game: game.strategies,
            id="regret_coord_rat",
        ),
        # 2x2x2 nfg
        pytest.param(
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            PROBS_1B_doub,
            PROBS_2B_doub,
            False,
            lambda profile, strategy: profile.strategy_regrets[strategy.player.label][
                strategy.label
            ],
            lambda game: game.strategies,
            id="regret_2x2x2_doub",
        ),
        pytest.param(
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            PROBS_1B_rat,
            PROBS_2B_rat,
            True,
            lambda profile, strategy: profile.strategy_regrets[strategy.player.label][
                strategy.label
            ],
            lambda game: game.strategies,
            id="regret_2x2x2_rat",
        ),
        # stripped-down poker
        pytest.param(
            games.create_stripped_down_poker_efg(),
            PROBS_1B_doub,
            PROBS_2B_doub,
            False,
            lambda profile, strategy: profile.strategy_regrets[strategy.player.label][
                strategy.label
            ],
            lambda game: game.strategies,
            id="regret_poker_doub",
        ),
        pytest.param(
            games.create_stripped_down_poker_efg(),
            PROBS_1B_rat,
            PROBS_2B_rat,
            True,
            lambda profile, strategy: profile.strategy_regrets[strategy.player.label][
                strategy.label
            ],
            lambda game: game.strategies,
            id="regret_poker_rat",
        ),
        #################################################################################
        # strategy_value (for strategies)
        # 4x4 coordination nfg
        pytest.param(
            games.read_from_file("coordination_4x4_payoff.nfg"),
            PROBS_1A_doub,
            PROBS_2A_doub,
            False,
            lambda profile, strategy: profile.strategy_values[strategy.player.label][
                strategy.label
            ],
            lambda game: game.strategies,
            id="strat_value_coord_doub",
        ),
        pytest.param(
            games.read_from_file("coordination_4x4_payoff.nfg"),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda profile, strategy: profile.strategy_values[strategy.player.label][
                strategy.label
            ],
            lambda game: game.strategies,
            id="strat_value_coord_rat",
        ),
        # 2x2x2 nfg
        pytest.param(
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            PROBS_1B_doub,
            PROBS_2B_doub,
            False,
            lambda profile, strategy: profile.strategy_values[strategy.player.label][
                strategy.label
            ],
            lambda game: game.strategies,
            id="strat_value_2x2x2_doub",
        ),
        pytest.param(
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            PROBS_1B_rat,
            PROBS_2B_rat,
            True,
            lambda profile, strategy: profile.strategy_values[strategy.player.label][
                strategy.label
            ],
            lambda game: game.strategies,
            id="strat_value_2x2x2_rat",
        ),
        # stripped-down poker
        pytest.param(
            games.create_stripped_down_poker_efg(),
            PROBS_1B_doub,
            PROBS_2B_doub,
            False,
            lambda profile, strategy: profile.strategy_values[strategy.player.label][
                strategy.label
            ],
            lambda game: game.strategies,
            id="strat_value_poker_doub",
        ),
        pytest.param(
            games.create_stripped_down_poker_efg(),
            PROBS_1B_rat,
            PROBS_2B_rat,
            True,
            lambda profile, strategy: profile.strategy_values[strategy.player.label][
                strategy.label
            ],
            lambda game: game.strategies,
            id="strat_value_poker_rat",
        ),
        #################################################################################
        # liap_value (of profile, hence [1] for objects_to_test, any singleton collection would do)
        # 4x4 coordination nfg
        pytest.param(
            games.read_from_file("coordination_4x4_payoff.nfg"),
            PROBS_1A_doub,
            PROBS_2A_doub,
            False,
            lambda profile, y: profile.liap_value(),
            lambda x: [1],
            id="liap_value_coord_doub",
        ),
        pytest.param(
            games.read_from_file("coordination_4x4_payoff.nfg"),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda profile, y: profile.liap_value(),
            lambda x: [1],
            id="liap_value_coord_rat",
        ),
        # 2x2x2 nfg
        pytest.param(
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            PROBS_1B_doub,
            PROBS_2B_doub,
            False,
            lambda profile, y: profile.liap_value(),
            lambda x: [1],
            id="liap_value_2x2x2_doub",
        ),
        pytest.param(
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            PROBS_1B_rat,
            PROBS_2B_rat,
            True,
            lambda profile, y: profile.liap_value(),
            lambda x: [1],
            id="liap_value_2x2x2_rat",
        ),
        # stripped-down poker
        pytest.param(
            games.create_stripped_down_poker_efg(),
            PROBS_1B_doub,
            PROBS_2B_doub,
            False,
            lambda profile, y: profile.liap_value(),
            lambda x: [1],
            id="liap_value_poker_doub",
        ),
        pytest.param(
            games.create_stripped_down_poker_efg(),
            PROBS_1B_rat,
            PROBS_2B_rat,
            True,
            lambda profile, y: profile.liap_value(),
            lambda x: [1],
            id="liap_value_poker_rat",
        ),
        #################################################################################
        # max_regret (of profile, hence [1] for objects_to_test, any singleton collection would do)
        # 4x4 coordination nfg
        pytest.param(
            games.read_from_file("coordination_4x4_payoff.nfg"),
            PROBS_1A_doub,
            PROBS_2A_doub,
            False,
            lambda profile, y: profile.max_regret(),
            lambda x: [1],
            id="max_regret_coord_doub",
        ),
        pytest.param(
            games.read_from_file("coordination_4x4_payoff.nfg"),
            PROBS_1A_rat,
            PROBS_2A_rat,
            True,
            lambda profile, y: profile.max_regret(),
            lambda x: [1],
            id="max_regret_coord_rat",
        ),
        # 2x2x2 nfg
        pytest.param(
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            PROBS_1B_doub,
            PROBS_2B_doub,
            False,
            lambda profile, y: profile.max_regret(),
            lambda x: [1],
            id="max_regret_2x2x2_doub",
        ),
        pytest.param(
            games.read_from_file("2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"),
            PROBS_1B_rat,
            PROBS_2B_rat,
            True,
            lambda profile, y: profile.max_regret(),
            lambda x: [1],
            id="max_regret_2x2x2_rat",
        ),
        # stripped-down poker
        pytest.param(
            games.create_stripped_down_poker_efg(),
            PROBS_1B_doub,
            PROBS_2B_doub,
            False,
            lambda profile, y: profile.max_regret(),
            lambda x: [1],
            id="max_regret_poker_doub",
        ),
        pytest.param(
            games.create_stripped_down_poker_efg(),
            PROBS_1B_rat,
            PROBS_2B_rat,
            True,
            lambda profile, y: profile.max_regret(),
            lambda x: [1],
            id="max_regret_poker_rat",
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
