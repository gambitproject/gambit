"""Tests of `Rational`, which keeps the text a number was entered as."""

import copy
import fractions
import io
import pickle

import pytest

import pygambit as gbt

from . import games


def test_rational_from_text_keeps_text():
    value = gbt.Rational("1.5")
    assert value == fractions.Fraction(3, 2)
    assert str(value) == "1.5"
    assert repr(value) == "Rational('1.5')"
    assert hash(value) == hash(fractions.Fraction(3, 2))


def test_rational_without_text_prints_as_fraction():
    assert str(gbt.Rational(3, 2)) == "3/2"
    assert repr(gbt.Rational(3, 2)) == "Rational(3, 2)"


def test_rational_arithmetic_returns_fraction():
    total = gbt.Rational("1.5") + gbt.Rational("1/3")
    assert type(total) is fractions.Fraction
    assert total == fractions.Fraction(11, 6)


@pytest.mark.parametrize("text", ["1.5", "1/3", "2"])
def test_rational_roundtrips_pickle_and_copy(text):
    value = gbt.Rational(text)
    for other in (pickle.loads(pickle.dumps(value)), copy.copy(value), copy.deepcopy(value)):
        assert type(other) is gbt.Rational
        assert other == value
        assert str(other) == text


def test_rational_pickle_without_text():
    other = pickle.loads(pickle.dumps(gbt.Rational(3, 2)))
    assert repr(other) == "Rational(3, 2)"


def test_outcome_payoffs_keep_entered_text_and_can_be_summed():
    game = gbt.StrategicGame.from_arrays([[1.5, "1/3"], [0, 1]], [[0, 1], [1, 0]])
    first = game.get_outcome_payoffs("1")
    second = game.get_outcome_payoffs("3")
    player = list(game.players)[0]
    assert type(first[player]) is gbt.Rational
    assert first[player] == fractions.Fraction(3, 2)
    assert str(first[player]) == "1.5"
    assert str(second[player]) == "1/3"
    assert first[player] + second[player] == fractions.Fraction(11, 6)


def test_computed_payoffs_carry_no_text():
    game = gbt.StrategicGame.from_arrays([[1.5, 0], [0, 1]], [[0, 1.5], [1, 0]])
    contingency = {p: game.get_strategies(p)[0] for p in game.players}
    value = game.get_payoffs(contingency)[list(game.players)[0]]
    assert type(value) is gbt.Rational
    assert repr(value) == "Rational(3, 2)"


def test_chance_probabilities_keep_entered_text():
    game = games.read_from_file("sample_extensive_game.efg")
    game.append_event(gbt.H.path("1", "1"), {"a": gbt.Decimal("0.5"), "b": gbt.Rational(1, 2)})
    probs = game.get_action_probs(gbt.H.path("1", "1"))
    assert all(type(p) is gbt.Rational for p in probs.values())
    assert str(probs["a"]) == "0.5"
    assert probs["a"] == probs["b"] == fractions.Fraction(1, 2)


def test_nfg_roundtrip_text_unchanged():
    game = gbt.StrategicGame.from_arrays([[1.5, 0], [0, 1]], [[0, 1.5], [1, 0]])
    text = game.to_nfg()
    assert gbt.read_nfg(io.StringIO(text)).to_nfg() == text
