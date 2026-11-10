"""Validation of the weights given for a mixed strategy or mixed action.

The helper is tested directly, and each of the three profile setters is tested for the
messages it must give, so that all three stay in step.
"""

import pytest

import pygambit as gbt
from pygambit import _distributions

from . import games

R = gbt.Rational
HELPER = _distributions.validate_distribution


def _validate(distribution, sparse=True, labels=("a", "b"), to_prob=_distributions.to_float_prob):
    return HELPER(
        labels,
        distribution,
        sparse,
        to_prob,
        "mixed strategy",
        _distributions.PLAYER_STRATEGIES,
    )


@pytest.mark.parametrize(
    "value, expected", [(3, 3.0), (0.1, 0.1), ("1/4", 0.25), (R(1, 8), 0.125)]
)
def test_to_float_prob(value, expected):
    result = _distributions.to_float_prob(value)
    assert result == expected
    assert type(result) is float


@pytest.mark.parametrize("value, expected", [(3, R(3)), ("0.5", R(1, 2)), ("2/6", R(1, 3))])
def test_to_rational_prob(value, expected):
    result = _distributions.to_rational_prob(value)
    assert result == expected
    assert isinstance(result, gbt.Rational)


@pytest.mark.parametrize(
    "to_prob", [_distributions.to_float_prob, _distributions.to_rational_prob]
)
@pytest.mark.parametrize("value", ["abc", "a/b", None])
def test_to_prob_rejects_non_numbers(to_prob, value):
    with pytest.raises(ValueError, match="Cannot convert"):
        to_prob(value)


def test_omitted_labels_get_weight_zero_and_every_label_is_returned():
    assert _validate({"b": 2}) == {"a": 0.0, "b": 2.0}


def test_weights_are_not_normalised():
    assert _validate({"a": 1, "b": 3}) == {"a": 1.0, "b": 3.0}


def test_rational_conversion_is_exact():
    values = _validate({"a": "1/3", "b": "2/3"}, to_prob=_distributions.to_rational_prob)
    assert values == {"a": R(1, 3), "b": R(2, 3)}


def test_result_follows_the_order_of_labels():
    values = _validate({"c": 1}, labels=["c", "a", "b"])
    assert list(values) == ["c", "a", "b"]


@pytest.mark.parametrize("distribution", [[1, 2], None, 3, "ab"])
def test_not_a_mapping_is_a_type_error(distribution):
    with pytest.raises(TypeError) as excinfo:
        _validate(distribution)
    assert str(excinfo.value) == (
        "a mixed strategy must be set from a Mapping from strategy label to weight, "
        f"not {type(distribution).__name__}"
    )


def test_unknown_labels_are_listed_in_order():
    with pytest.raises(ValueError) as excinfo:
        _validate({"z": 1, "a": 1, "y": 1})
    assert str(excinfo.value) == "not a strategy label for this player: y, z"


def test_unknown_labels_are_reported_before_missing_ones():
    with pytest.raises(ValueError, match="not a strategy label"):
        _validate({"z": 1}, sparse=False)


def test_non_sparse_requires_every_label():
    with pytest.raises(ValueError) as excinfo:
        _validate({"a": 1}, sparse=False)
    assert str(excinfo.value) == (
        "a distribution must specify exactly one weight for each of the player's "
        "strategies, by label, unless sparse=True"
    )
    assert _validate({"a": 1, "b": 0}, sparse=False) == {"a": 1.0, "b": 0.0}


def test_negative_weight():
    with pytest.raises(ValueError) as excinfo:
        _validate({"a": -1, "b": 2})
    assert str(excinfo.value) == "a mixed strategy's weights must be non-negative"


@pytest.mark.parametrize("distribution", [{}, {"a": 0, "b": 0}, {"a": "0/5"}])
def test_all_zero(distribution):
    with pytest.raises(ValueError) as excinfo:
        _validate(distribution)
    assert str(excinfo.value) == "a mixed strategy's weights must not all be zero"


def test_the_noun_and_owner_text_appear_in_messages():
    def action_helper(distribution, sparse=True):
        return HELPER(
            ["x", "y"],
            distribution,
            sparse,
            _distributions.to_float_prob,
            "mixed action",
            _distributions.INFOSET_ACTIONS,
        )

    with pytest.raises(
        TypeError, match="a mixed action must be set from a Mapping from action label"
    ):
        action_helper(5)
    with pytest.raises(ValueError, match="not an action label at this information set: q"):
        action_helper({"q": 1})
    with pytest.raises(ValueError, match="for each action at the information set, unless"):
        action_helper({"x": 1}, sparse=False)
    with pytest.raises(ValueError, match="a mixed action's weights must be non-negative"):
        action_helper({"x": -1})
    with pytest.raises(ValueError, match="a mixed action's weights must not all be zero"):
        action_helper({})


def _strategic_setters(rational):
    game = games.read_from_file("coordination_4x4_payoff.nfg")
    profile = game.mixed_strategy_profile(rational=rational)
    return (
        "mixed strategy",
        game.get_strategies("Player 1"),
        [
            lambda d: profile.__setitem__("Player 1", d),
            lambda d: profile.set_mixed_strategy("Player 1", d, sparse=True),
        ],
        lambda: profile["Player 1"],
    )


def _behavior_setters(rational):
    game = games.read_from_file("mixed_behavior_game.efg")
    profile = game.mixed_behavior_profile(rational=rational)
    selector = gbt.H.path(*games.player_infosets(game, "Player 1")[0].actions)
    labels = [label for label, _ in profile[selector]]
    return (
        "mixed action",
        labels,
        [
            lambda d: profile.__setitem__(selector, d),
            lambda d: profile.set_mixed_action(selector, d, sparse=True),
        ],
        lambda: profile[selector],
    )


def _bagg_setters(rational):
    game = games.read_from_file("bayesian_2p2t_shared_node.bagg")
    profile = game.mixed_behavior_profile(rational=rational)
    labels = [label for label, _ in profile["1", "1"]]
    return (
        "mixed action",
        labels,
        [
            lambda d: profile.__setitem__(("1", "1"), d),
        ],
        lambda: profile["1", "1"],
    )


SETTERS = [_strategic_setters, _behavior_setters, _bagg_setters]


@pytest.mark.parametrize("make", SETTERS, ids=lambda f: f.__name__)
@pytest.mark.parametrize("rational", [False, True])
def test_profile_setters_report_the_same_errors(make, rational):
    noun, labels, setters, _ = make(rational)
    first = labels[0]
    for setter in setters:
        with pytest.raises(TypeError) as excinfo:
            setter([1, 2])
        assert str(excinfo.value).startswith(f"a {noun} must be set from a Mapping from ")
        with pytest.raises(ValueError, match="not an? (strategy|action) label"):
            setter({"no-such-label": 1})
        with pytest.raises(ValueError) as excinfo:
            setter({first: -1})
        assert str(excinfo.value) == f"a {noun}'s weights must be non-negative"
        with pytest.raises(ValueError) as excinfo:
            setter({first: 0})
        assert str(excinfo.value) == f"a {noun}'s weights must not all be zero"
        with pytest.raises(ValueError, match="Cannot convert 'abc' to a number"):
            setter({first: "abc"})


@pytest.mark.parametrize("make", SETTERS, ids=lambda f: f.__name__)
@pytest.mark.parametrize("rational", [False, True])
def test_a_rejected_distribution_leaves_the_profile_unchanged(make, rational):
    _, labels, setters, get = make(rational)
    before = dict(get())
    for setter in setters:
        with pytest.raises(ValueError):
            setter({labels[0]: 1, "no-such-label": 1})
        with pytest.raises(ValueError):
            setter({labels[0]: -1})
    assert dict(get()) == before


@pytest.mark.parametrize("make", SETTERS, ids=lambda f: f.__name__)
def test_profile_setters_store_weights_in_the_profiles_number_type(make):
    for rational, kind in [(False, float), (True, gbt.Rational)]:
        _, labels, setters, get = make(rational)
        setters[0]({labels[0]: "1/4", labels[1]: "3/4"})
        stored = dict(get())
        assert all(type(v) is kind for v in stored.values())
        assert stored[labels[0]] == (0.25 if not rational else R(1, 4))
