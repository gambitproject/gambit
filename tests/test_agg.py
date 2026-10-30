"""Tests of generic Game operations on action graph game (AGG/BAGG) representations.
"""

import itertools

import pytest

import pygambit as gbt

from . import games

AGG_BAGG_GAME_PATHS = [
    "2x2.agg",
    "2x2_small_payoffs.agg",
    "2x2.bagg",
    "Bayesian-Coffee-3-2-2-3.bagg",
]


@pytest.mark.parametrize("game_path", ["2x2.agg", "2x2_small_payoffs.agg"])
def test_agg_is_const_sum(game_path):
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


@pytest.mark.parametrize("game_path", ["2x2.agg", "2x2_small_payoffs.agg"])
def test_agg_to_nfg(game_path):
    game = games.read_from_file(game_path)
    serialized_game = game.to_nfg()
    assert serialized_game[:3] == "NFG"


ABSENT_ATTRS = ["set_players", "relabel_strategies", "set_strategies", "behavior_support_profile"]

BAGG_STRATEGY_STUBS = [
    ("get_strategies", lambda g: g.get_strategies("1")),
    ("contingencies", lambda g: g.contingencies),
    ("get_payoffs", lambda g: g.get_payoffs({})),
    ("mixed_strategy_profile", lambda g: g.mixed_strategy_profile()),
    ("random_strategy_profile", lambda g: g.random_strategy_profile()),
    ("strategy_support_profile", lambda g: g.strategy_support_profile()),
    ("to_arrays", lambda g: g.to_arrays()),
    ("to_nfg", lambda g: g.to_nfg()),
    ("to_html", lambda g: g.to_html()),
    ("to_latex", lambda g: g.to_latex()),
    ("to_efg", lambda g: g.to_efg()),
    ("relabel_players", lambda g: g.relabel_players({"1": "a"})),
    ("is_const_sum", lambda g: g.is_const_sum),
    ("get_min_payoff", lambda g: g.get_min_payoff("1")),
    ("get_max_payoff", lambda g: g.get_max_payoff("1")),
    ("has_perfect_recall", lambda g: g.has_perfect_recall("1")),
]


@pytest.mark.parametrize("game_path", AGG_BAGG_GAME_PATHS)
@pytest.mark.parametrize("attr", ABSENT_ATTRS)
def test_agg_bagg_missing_extensive_and_strategic_only_attrs(game_path, attr):
    """`set_players`, `relabel_strategies`, `set_strategies`, and
    `behavior_support_profile` are each implemented (in C++) only for the tree and/or
    strategic representations, not for AGG/BAGG -- they must not exist as attributes
    on an `ActionGraphGame`/`BayesianActionGraphGame` at all, rather than existing and
    raising when called.  The strategy-shaped members that a BAGG refuses (see
    `BAGG_STRATEGY_STUBS`) do exist, and each raises the one dedicated error."""
    game = games.read_from_file(game_path)
    assert not hasattr(game, attr)


@pytest.mark.parametrize("game_path", ["2x2.bagg", "Bayesian-Coffee-3-2-2-3.bagg"])
@pytest.mark.parametrize(
    "name,call", BAGG_STRATEGY_STUBS, ids=[n for n, _ in BAGG_STRATEGY_STUBS]
)
def test_bagg_strategy_shaped_members_raise(game_path, name, call):
    game = games.read_from_file(game_path)
    assert hasattr(type(game), name)
    with pytest.raises(gbt.UnsupportedForBayesianGameError, match="not supported for Bayesian"):
        call(game)


def test_bagg_error_is_dedicated_error_type():
    assert issubclass(gbt.UnsupportedForBayesianGameError, gbt.UndefinedOperationError)


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
        assert game.get_max_payoff(player) == gbt.Rational(1, 3)
        assert game.get_min_payoff(player) == -10

    # pure-strategy payoff lookup (AGGPureStrategyProfileRep::GetPayoff) also reports the
    # exact value, not a double-rounded approximation -- for both the fraction and the long
    # decimal payoff
    p0, p1 = game.players
    s0 = game.get_strategies(p0)
    s1 = game.get_strategies(p1)

    profile = game.mixed_strategy_profile(rational=True)
    profile[p0] = {s0[0]: gbt.Rational(1), s0[1]: gbt.Rational(0)}
    profile[p1] = {s1[0]: gbt.Rational(0), s1[1]: gbt.Rational(1)}
    assert profile.payoffs[p0] == gbt.Rational(1, 3)

    profile = game.mixed_strategy_profile(rational=True)
    profile[p0] = {s0[0]: gbt.Rational(0), s0[1]: gbt.Rational(1)}
    profile[p1] = {s1[0]: gbt.Rational(0), s1[1]: gbt.Rational(1)}
    assert profile.payoffs[p0] == gbt.Rational("0.123456789012345")
    assert profile.payoffs[p1] == gbt.Rational("0.123456789012345")


def test_bagg_fraction_type_distribution_parsed_exactly():
    """BAGG type-distribution probabilities written as a fraction ("1/3") are parsed exactly
    (agg::BAGG::exactIndepTypeDist) and the exact weighted-sum payoff computation over them
    (agg::BAGG::getMixedPayoff<Rational>) agrees with the double engine on the same profile.

    2x2_fraction_types.bagg: player 1 has two types (weights 1/3, 2/3), player 2 has one.
    """
    game = games.read_from_file("2x2_fraction_types.bagg")
    assert game.mixed_behavior_profile(rational=True).infoset_probs["1"] == {
        "1": gbt.Rational(1, 3), "2": gbt.Rational(2, 3)
    }
    exact = game.mixed_behavior_profile(rational=True)
    dbl = game.mixed_behavior_profile(rational=False)
    for profile, one, zero in [(exact, gbt.Rational(1), gbt.Rational(0)), (dbl, 1.0, 0.0)]:
        profile["1", "1"] = {"1": one, "2": zero}
        profile["1", "2"] = {"1": zero, "2": one}
        profile["2", "1"] = {"1": one, "2": zero}
    assert exact.payoffs["2"] == gbt.Rational(22)
    assert float(exact.payoffs["2"]) == dbl.payoffs["2"]


@pytest.mark.parametrize(
    "game_path",
    [
        "2x2.agg",
        "2x2.bagg",
    ],
)
def test_agg_bagg_mixed_strategy_profile_rational_exact_payoff(game_path):
    """AGG/BAGG mixed-strategy payoffs support exact (rational) computation: the convolution
    algorithm (agg::AGG::getMixedPayoff et al.) is generic in its numeric type, so it also runs
    with Rational arithmetic throughout (agg::AGG::getMixedPayoff<Rational>), not just double.

    2x2.agg has a known mixed equilibrium at (10/11, 1/11) for both players, with payoff
    exactly -5/11 to each -- computed here directly (not via a solver), confirming the exact
    engine reproduces it with zero regret.
    """
    game = games.read_from_file(game_path)
    if isinstance(game, gbt.BayesianActionGraphGame):
        profile = game.mixed_behavior_profile(rational=True)
        for player in game.players:
            profile[player, "1"] = {"1": gbt.Rational(10, 11), "2": gbt.Rational(1, 11)}
    else:
        profile = game.mixed_strategy_profile(rational=True)
        for player in game.players:
            strategies = game.get_strategies(player)
            profile[player] = {
                strategies[0]: gbt.Rational(10, 11), strategies[1]: gbt.Rational(1, 11)
            }
    for player in game.players:
        assert profile.payoffs[player] == gbt.Rational(-5, 11)
    assert profile.max_regret() == 0


def test_agg_rational_algorithms_find_exact_mixed_equilibrium():
    """lcp_solve(rational=True) and enummixed_solve(rational=True) find the (10/11, 1/11)
    mixed equilibrium of 2x2.agg exactly.  A BAGG is rejected by both; see
    test_bagg_solvers.py. See test_nash.py::test_lcp_strategy_double_12 for the
    equivalent full-precision check via the standard solver-test harness.
    """
    game = games.read_from_file("2x2.agg")
    results = [
        gbt.nash.lcp_solve(game, rational=True, use_strategic=True),
        gbt.nash.enummixed_solve(game, rational=True),
    ]
    for result in results:
        mixed = [
            eq for eq in result.equilibria
            if any(
                0 < eq[p][s] < 1 for p in game.players for s in game.get_strategies(p)
            )
        ]
        assert len(mixed) == 1
        for player in game.players:
            for strategy in game.get_strategies(player):
                assert mixed[0][player][strategy] in (
                    gbt.Rational(10, 11), gbt.Rational(1, 11)
                )
        assert mixed[0].max_regret() == 0


def _pure_behavior_profile(game, rational, contingency):
    profile = game.mixed_behavior_profile(rational=rational)
    one, zero = (gbt.Rational(1), gbt.Rational(0)) if rational else (1.0, 0.0)
    for (player, infoset), action_index in contingency.items():
        profile[player, infoset] = {
            action: one if i == action_index else zero
            for i, (action, _) in enumerate(profile[player, infoset])
        }
    return profile


def _assert_exact_pure_payoffs_match_double(game, contingencies):
    for contingency in contingencies:
        exact = _pure_behavior_profile(game, True, contingency)
        dbl = _pure_behavior_profile(game, False, contingency)
        for player in game.players:
            assert float(exact.payoffs[player]) == pytest.approx(dbl.payoffs[player])


@pytest.mark.parametrize("game_path", ["2x2.bagg", "2x2_fraction_types.bagg"])
def test_bagg_pure_strategy_payoff_matches_degenerate_mixed_profile(game_path):
    """The exact payoff of every pure behavior profile (agg::BAGG::getMixedPayoff<Rational>
    at a degenerate profile) agrees with the double engine on the same profile, and the
    ex-ante payoff of a pure profile is its type-weighted average of conditional payoffs.
    """
    game = games.read_from_file(game_path)
    slots = [(p, t) for p in game.players for t in game.get_infosets(p)]
    sizes = [len(game.mixed_behavior_profile()[p, t]) for p, t in slots]
    contingencies = [
        dict(zip(slots, choice, strict=True))
        for choice in itertools.product(*(range(size) for size in sizes))
    ]
    _assert_exact_pure_payoffs_match_double(game, contingencies)


def test_bagg_pure_strategy_payoff_with_multiple_players_and_types():
    """Same cross-check as above, sampled (full enumeration is 7**6 contingencies) on a BAGG
    with several players each holding several types, to exercise the exact payoff's
    handling of more than one other player's type distribution at once.
    """
    game = games.read_from_file("Bayesian-Coffee-3-2-2-3.bagg")
    slots = [(p, t) for p in game.players for t in game.get_infosets(p)]
    sizes = [len(game.mixed_behavior_profile()[p, t]) for p, t in slots]
    choices = [
        tuple(0 for _ in sizes),
        tuple(size - 1 for size in sizes),
        tuple(i % size for i, size in enumerate(sizes)),
    ]
    _assert_exact_pure_payoffs_match_double(
        game, [dict(zip(slots, choice, strict=True)) for choice in choices]
    )
