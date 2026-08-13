"""Tests of generic Game operations on action graph game (AGG/BAGG) representations.
"""

import pytest

import pygambit as gbt

from . import games

AGG_BAGG_GAME_PATHS = [
    "2x2.agg",
    "2x2_small_payoffs.agg",
    "2x2.bagg",
    "Bayesian-Coffee-3-2-2-3.bagg",
]


@pytest.mark.parametrize("game_path", AGG_BAGG_GAME_PATHS)
def test_agg_bagg_is_const_sum(game_path):
    game = games.read_from_file(game_path)
    assert not game.is_const_sum


@pytest.mark.parametrize(
    "game_path,exp_min,exp_max",
    [
        ("2x2.agg", -10, 95),
        ("2x2.bagg", -10, 95),
        ("Bayesian-Coffee-3-2-2-3.bagg", 0, 99),
    ],
)
def test_agg_bagg_get_min_max_payoff(game_path, exp_min, exp_max):
    game = games.read_from_file(game_path)
    assert game.min_payoff == exp_min
    assert game.max_payoff == exp_max


@pytest.mark.parametrize("game_path", AGG_BAGG_GAME_PATHS)
def test_agg_bagg_to_nfg(game_path):
    game = games.read_from_file(game_path)
    serialized_game = game.to_nfg()
    assert serialized_game[:3] == "NFG"


def test_agg_fraction_and_long_decimal_payoffs_parsed_exactly():
    """Payoffs written in an .agg file as a fraction ("1/3") or a decimal needing more than
    double's ~15-17 significant digits of precision to round-trip are parsed into exact
    Rationals (via Number, agg::AGG::exactPayoffs), not silently rounded through a bare
    `istream >> double` read.

    2x2_fraction_payoffs.agg is 2x2.agg with two of its four payoffs replaced: 35 -> 1/3 and
    95 -> 0.123456789012345.
    """
    game = games.read_from_file("2x2_fraction_payoffs.agg")
    assert game.max_payoff == gbt.Rational(1, 3)
    assert game.min_payoff == -10
    for player in game.players:
        assert player.max_payoff == gbt.Rational(1, 3)
        assert player.min_payoff == -10

    # pure-strategy payoff lookup (AGGPureStrategyProfileRep::GetPayoff) also reports the
    # exact value, not a double-rounded approximation
    profile = game.mixed_strategy_profile(rational=True)
    p0, p1 = game.players
    s0 = list(p0.strategies)
    s1 = list(p1.strategies)
    profile[s0[0]] = gbt.Rational(1)
    profile[s0[1]] = gbt.Rational(0)
    profile[s1[0]] = gbt.Rational(0)
    profile[s1[1]] = gbt.Rational(1)
    assert profile.payoff(p0) == gbt.Rational(1, 3)


def test_bagg_fraction_type_distribution_parsed_exactly():
    """BAGG type-distribution probabilities written as a fraction ("1/3") are parsed exactly
    (agg::BAGG::exactIndepTypeDist) and the exact weighted-sum payoff computation over them
    (agg::BAGG::getExactMixedPayoff) agrees with the double engine on the same profile.

    2x2_fraction_types.bagg: player 1 has two types (weights 1/3, 2/3), player 2 has one.
    """
    game = games.read_from_file("2x2_fraction_types.bagg")
    p1t0, p1t1, p2 = game.players
    exact = game.mixed_strategy_profile(rational=True)
    dbl = game.mixed_strategy_profile(rational=False)
    for profile, one, zero in [(exact, gbt.Rational(1), gbt.Rational(0)), (dbl, 1.0, 0.0)]:
        s0, s1, s2 = list(p1t0.strategies), list(p1t1.strategies), list(p2.strategies)
        profile[s0[0]], profile[s0[1]] = one, zero
        profile[s1[0]], profile[s1[1]] = zero, one
        profile[s2[0]], profile[s2[1]] = one, zero
    assert exact.payoff(p2) == gbt.Rational(22)
    assert float(exact.payoff(p2)) == dbl.payoff(p2)


@pytest.mark.parametrize("game_path", ["2x2.agg", "2x2.bagg"])
def test_agg_bagg_mixed_strategy_profile_rational_exact_payoff(game_path):
    """AGG/BAGG mixed-strategy payoffs support exact (rational) computation: the convolution
    algorithm (agg::AGG::getMixedPayoff et al.) is generic in its numeric type, so it also runs
    with Rational arithmetic throughout (agg::AGG::getExactMixedPayoff), not just double.

    2x2.agg has a known mixed equilibrium at (10/11, 1/11) for both players, with payoff
    exactly -5/11 to each -- computed here directly (not via a solver), confirming the exact
    engine reproduces it with zero regret.
    """
    game = games.read_from_file(game_path)
    profile = game.mixed_strategy_profile(rational=True)
    for player in game.players:
        strategies = list(player.strategies)
        profile[strategies[0]] = gbt.Rational(10, 11)
        profile[strategies[1]] = gbt.Rational(1, 11)
    for player in game.players:
        assert profile.payoff(player) == gbt.Rational(-5, 11)
    assert profile.max_regret() == 0


@pytest.mark.parametrize("game_path", ["2x2.agg", "2x2.bagg"])
def test_agg_bagg_rational_algorithms_find_exact_mixed_equilibrium(game_path):
    """lcp_solve(rational=True) and enummixed_solve(rational=True) find the (10/11, 1/11)
    mixed equilibrium of 2x2.agg exactly. See test_nash.py::test_lcp_strategy_double_12 for the
    equivalent full-precision check via the standard solver-test harness.
    """
    game = games.read_from_file(game_path)
    results = [
        gbt.nash.lcp_solve(game, rational=True, use_strategic=True),
        gbt.nash.enummixed_solve(game, rational=True),
    ]
    for result in results:
        mixed = [eq for eq in result.equilibria if any(0 < eq[s] < 1 for s in game.strategies)]
        assert len(mixed) == 1
        for player in game.players:
            for strategy in player.strategies:
                assert mixed[0][strategy] in (gbt.Rational(10, 11), gbt.Rational(1, 11))
        assert mixed[0].max_regret() == 0
