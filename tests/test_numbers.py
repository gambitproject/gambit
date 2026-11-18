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


SCIENTIFIC_PAYOFFS = [
    ("1.5e-3", fractions.Fraction(3, 2000)),
    ("2E+4", fractions.Fraction(20000)),
    ("3e2", fractions.Fraction(300)),
    ("-1.25e-1", fractions.Fraction(-1, 8)),
    ("1e0", fractions.Fraction(1)),
    ("1.5e3", fractions.Fraction(1500)),
]


def _scientific_nfg(entries):
    # 2x2 game with player 1's payoffs in the first row of entries; the rest are zero.
    values = entries + ["0"] * (8 - len(entries))
    return 'NFG 1 R "scientific" { "1" "2" } { 2 2 }\n\n' + " ".join(values) + "\n"


def _scientific_efg(entries):
    payoffs = " ".join(entries)
    return (
        'EFG 2 R "scientific" { "1" "2" }\n""\n\n'
        'c "" 1 "" { "a" 1/2 "b" 1/2 } 0\n'
        f't "" 1 "" {{ {payoffs} }}\n'
        't "" 2 "" { 0, 0 }\n'
    )


@pytest.mark.parametrize(("text", "expected"), SCIENTIFIC_PAYOFFS)
def test_read_nfg_scientific_notation_payoff(text, expected):
    game = gbt.read_nfg(io.StringIO(_scientific_nfg([text, "0"])))
    value = game.get_outcome_payoffs("1")[list(game.players)[0]]
    assert type(value) is gbt.Rational
    assert value == expected
    assert str(value) == text


@pytest.mark.parametrize(("text", "expected"), SCIENTIFIC_PAYOFFS)
def test_read_efg_scientific_notation_payoff(text, expected):
    game = gbt.read_efg(io.StringIO(_scientific_efg([text, "0"])))
    label = game.get_outcomes()[0]
    value = game.get_outcome_payoffs(label)[list(game.players)[0]]
    assert type(value) is gbt.Rational
    assert value == expected
    assert str(value) == text


@pytest.mark.parametrize(
    ("first", "second", "expected"),
    [
        ("1.5e-3", "0.9985", fractions.Fraction(3, 2000)),
        ("5E-1", "5e-1", fractions.Fraction(1, 2)),
        ("1e0", "0", fractions.Fraction(1)),
    ],
)
def test_read_efg_scientific_notation_chance_probability(first, second, expected):
    efg = (
        'EFG 2 R "scientific" { "1" }\n""\n\n'
        f'c "" 1 "" {{ "a" {first} "b" {second} }} 0\n'
        't "" 1 "" { 0 }\n'
        't "" 2 "" { 0 }\n'
    )
    game = gbt.read_efg(io.StringIO(efg))
    probs = game.get_action_probs(gbt.H.path())
    assert probs["a"] == expected
    assert str(probs["a"]) == first
