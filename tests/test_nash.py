"""Test of calls to Nash equilibrium solvers.

For many solvers the tests simply ensure that calling the solver works, and then for some cases,
checking the expected results on a very simple game.

There is better test coverage for
lp_solve, lcp_solve, and enumpoly_solve, all in mixed behaviors.
"""

import dataclasses
import functools
import typing

import pytest

import pygambit as gbt
from pygambit import Rational as Q

from . import games

TOL = 1e-13  # tolerance for floating point assertions
TOL_LARGE = 1e-3  # larger tolerance for floating point assertions
TOL_HUGE = 1e-2  # huge tolerance for floating point assertions


def d(*probs) -> tuple:
    """Helper function to let us write d() to be suggestive of
    "probability distribution on simplex" ("Delta")
    """
    return tuple(probs)


@dataclasses.dataclass
class EquilibriumTestCase:
    """Summarising the data relevant for a test fixture of a call to an equilibrium solver."""

    factory: typing.Callable[[], gbt.Game]
    solver: typing.Callable[[gbt.Game], gbt.qre.LogitQREMixedStrategyFitResult]
    expected: list
    regret_tol: float | gbt.Rational = Q(0)
    prob_tol: float | gbt.Rational = Q(0)


@dataclasses.dataclass
class QREquilibriumTestCase:
    """Summarising the data relevant for a test fixture of a call to an QRE solver."""

    factory: typing.Callable[[], gbt.Game]
    solver: typing.Callable[[gbt.Game], gbt.nash.NashComputationResult]
    expected: list
    prob_tol: float
    lam_tol: float


##################################################################################################
# NASH SOLVER IN PURE/MIXED STRATEGIES (as opposed to pure/mixed behaviors)
##################################################################################################

ENUMPURE_CASES = [
    # Zero-sum games
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "two_player_perfect_info_win_lose.efg"
            ),
            solver=functools.partial(gbt.nash.enumpure_solve),
            expected=[
                [d(0, 0, 1, 0), d(1, 0, 0)],
                [d(0, 0, 1, 0), d(0, 1, 0)],
                [d(0, 0, 1, 0), d(0, 0, 1)],
            ],
        ),
        marks=pytest.mark.nash_enumpure_strategy,
        id="test_enumpure_1",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=games.create_stripped_down_poker_efg,
            solver=functools.partial(gbt.nash.enumpure_solve),
            expected=[],
        ),
        marks=pytest.mark.nash_enumpure_strategy,
        id="test_enumpure_2",
    ),
    # Non-zero-sum 2-player games
    pytest.param(
        EquilibriumTestCase(
            factory=games.create_one_shot_trust_efg,
            solver=functools.partial(gbt.nash.enumpure_solve),
            expected=[[d(0, 1), d(0, 1)]],
        ),
        marks=pytest.mark.nash_enumpure_strategy,
        id="test_enumpure_3",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_one_shot_trust_efg, unique_NE_variant=True),
            solver=functools.partial(gbt.nash.enumpure_solve),
            expected=[[d(1, 0), d(0, 1)]],
        ),
        marks=pytest.mark.nash_enumpure_strategy,
        id="test_enumpure_4",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_EFG_for_nxn_bimatrix_coordination_game, n=3),
            solver=functools.partial(gbt.nash.enumpure_solve),
            expected=[
                [d(1, 0, 0), d(1, 0, 0)],
                [d(0, 1, 0), d(0, 1, 0)],
                [d(0, 0, 1), d(0, 0, 1)],
            ],
        ),
        marks=pytest.mark.nash_enumpure_strategy,
        id="test_enumpure_5",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=games.create_EFG_for_6x6_bimatrix_with_long_LH_paths_and_unique_eq,
            solver=functools.partial(gbt.nash.enumpure_solve),
            expected=[],
        ),
        marks=pytest.mark.nash_enumpure_strategy,
        id="test_enumpure_6",
    ),
    # 3-player game
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "mixed_behavior_game.efg"),
            solver=functools.partial(gbt.nash.enumpure_solve),
            expected=[
                [d(1, 0), d(1, 0), d(1, 0)],
                [d(0, 1), d(0, 1), d(1, 0)],
                [d(0, 1), d(1, 0), d(0, 1)],
                [d(1, 0), d(0, 1), d(0, 1)],
            ],
        ),
        marks=pytest.mark.nash_enumpure_strategy,
        id="test_enumpure_7",
    ),
    # 2x2x2 strategic form game based on local max cut -- 2 pure
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"
            ),
            solver=functools.partial(gbt.nash.enumpure_solve),
            expected=[
                [d(1, 0), d(0, 1), d(1, 0)],
                [d(0, 1), d(1, 0), d(0, 1)],
            ],
        ),
        marks=pytest.mark.nash_enumpure_strategy,
        id="test_enumpure_8",
    ),
]


ENUMMIXED_RATIONAL_CASES = [
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "two_player_perfect_info_win_lose.efg"
            ),
            solver=functools.partial(gbt.nash.enummixed_solve, rational=True),
            expected=[
                [d(0, 0, 1, 0), d(1, 0, 0)],
                [d(0, 0, 1, 0), d(0, 1, 0)],
                [d(0, 0, 1, 0), d(0, 0, 1)],
            ],
        ),
        marks=pytest.mark.nash_enummixed_strategy,
        id="test_enumixed_rational_1",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=games.create_stripped_down_poker_efg,
            solver=functools.partial(gbt.nash.enummixed_solve, rational=True),
            expected=[
                [d(Q("1/3"), Q("2/3"), 0, 0), d(Q("2/3"), Q("1/3"))],
            ],
        ),
        marks=pytest.mark.nash_enummixed_strategy,
        id="test_enumixed_rational_2",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=games.create_one_shot_trust_efg,
            solver=functools.partial(gbt.nash.enummixed_solve, rational=True),
            expected=[
                [d(0, 1), d(Q("1/2"), Q("1/2"))],
                [d(0, 1), d(0, 1)],
            ],
        ),
        marks=pytest.mark.nash_enummixed_strategy,
        id="test_enumixed_rational_3",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_EFG_for_nxn_bimatrix_coordination_game, n=3),
            solver=functools.partial(gbt.nash.enummixed_solve, rational=True),
            expected=[
                [d(1, 0, 0), d(1, 0, 0)],
                [d(Q("1/2"), Q("1/2"), 0), d(Q("1/2"), Q("1/2"), 0)],
                [d(Q("1/3"), Q("1/3"), Q("1/3")), d(Q("1/3"), Q("1/3"), Q("1/3"))],
                [d(Q("1/2"), 0, Q("1/2")), d(Q("1/2"), 0, Q("1/2"))],
                [d(0, 1, 0), d(0, 1, 0)],
                [d(0, Q("1/2"), Q("1/2")), d(0, Q("1/2"), Q("1/2"))],
                [d(0, 0, 1), d(0, 0, 1)],
            ],
        ),
        marks=pytest.mark.nash_enummixed_strategy,
        id="test_enumixed_rational_4",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=games.create_EFG_for_6x6_bimatrix_with_long_LH_paths_and_unique_eq,
            solver=functools.partial(gbt.nash.enummixed_solve, rational=True),
            expected=[
                [
                    d(Q("1/30"), Q("1/6"), Q("3/10"), Q("3/10"), Q("1/6"), Q("1/30")),
                    d(Q("1/6"), Q("1/30"), Q("3/10"), Q("3/10"), Q("1/30"), Q("1/6")),
                ],
            ],
        ),
        marks=pytest.mark.nash_enummixed_strategy,
        id="test_enumixed_rational_5",
    ),
]


ENUMMIXED_DOUBLE_CASES = [
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "two_player_perfect_info_win_lose.efg"
            ),
            solver=functools.partial(gbt.nash.enummixed_solve, rational=False),
            expected=[
                [d(0, 0, 1, 0), d(1, 0, 0)],
                [d(0, 0, 1, 0), d(0, 1, 0)],
                [d(0, 0, 1, 0), d(0, 0, 1)],
            ],
        ),
        marks=pytest.mark.nash_enummixed_strategy,
        id="test_enumixed_double_1",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=games.create_stripped_down_poker_efg,
            solver=functools.partial(gbt.nash.enummixed_solve, rational=False),
            expected=[
                [d(Q("1/3"), Q("2/3"), 0, 0), d(Q("2/3"), Q("1/3"))],
            ],
            prob_tol=TOL,
            regret_tol=TOL,
        ),
        marks=pytest.mark.nash_enummixed_strategy,
        id="test_enumixed_double_2",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=games.create_one_shot_trust_efg,
            solver=functools.partial(gbt.nash.enummixed_solve, rational=False),
            expected=[
                [d(0, 1), d(Q("1/2"), Q("1/2"))],
                [d(0, 1), d(0, 1)],
            ],
        ),
        marks=pytest.mark.nash_enummixed_strategy,
        id="test_enumixed_double_3",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_EFG_for_nxn_bimatrix_coordination_game, n=3),
            solver=functools.partial(gbt.nash.enummixed_solve, rational=False),
            expected=[
                [d(1, 0, 0), d(1, 0, 0)],
                [d(Q("1/2"), Q("1/2"), 0), d(Q("1/2"), Q("1/2"), 0)],
                [d(Q("1/3"), Q("1/3"), Q("1/3")), d(Q("1/3"), Q("1/3"), Q("1/3"))],
                [d(Q("1/2"), 0, Q("1/2")), d(Q("1/2"), 0, Q("1/2"))],
                [d(0, 1, 0), d(0, 1, 0)],
                [d(0, Q("1/2"), Q("1/2")), d(0, Q("1/2"), Q("1/2"))],
                [d(0, 0, 1), d(0, 0, 1)],
            ],
            prob_tol=TOL,
            regret_tol=TOL,
        ),
        marks=pytest.mark.nash_enummixed_strategy,
        id="test_enumixed_double_4",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=games.create_EFG_for_6x6_bimatrix_with_long_LH_paths_and_unique_eq,
            solver=functools.partial(gbt.nash.enummixed_solve, rational=False),
            expected=[
                [
                    d(Q("1/30"), Q("1/6"), Q("3/10"), Q("3/10"), Q("1/6"), Q("1/30")),
                    d(Q("1/6"), Q("1/30"), Q("3/10"), Q("3/10"), Q("1/30"), Q("1/6")),
                ],
            ],
            prob_tol=TOL,
            regret_tol=TOL,
        ),
        marks=pytest.mark.nash_enummixed_strategy,
        id="test_enumixed_double_5",
    ),
]


LP_STRATEGY_RATIONAL_CASES = [
    pytest.param(
        EquilibriumTestCase(
            factory=games.create_2x2_zero_sum_efg,
            solver=functools.partial(gbt.nash.lp_solve, rational=True, use_strategic=True),
            expected=[[d("1/2", "1/2"), d("1/2", "1/2")]],
        ),
        marks=pytest.mark.nash_lp_strategy,
        id="test_lp_strategy_rational_1",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.create_2x2_zero_sum_efg, variant="missing term outcome"
            ),
            solver=functools.partial(gbt.nash.lp_solve, rational=True, use_strategic=True),
            expected=[[d("1/2", "1/2"), d("1/2", "1/2")]],
        ),
        marks=pytest.mark.nash_lp_strategy,
        id="test_lp_strategy_rational_2",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.create_stripped_down_poker_efg, nonterm_outcomes=False
            ),
            solver=functools.partial(gbt.nash.lp_solve, rational=True, use_strategic=True),
            expected=[[d("1/3", "2/3", 0, 0), d("2/3", "1/3")]],
        ),
        marks=pytest.mark.nash_lp_strategy,
        id="test_lp_strategy_rational_3",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_stripped_down_poker_efg, nonterm_outcomes=True),
            solver=functools.partial(gbt.nash.lp_solve, rational=True, use_strategic=True),
            expected=[[d("1/3", "2/3", 0, 0), d("2/3", "1/3")]],
        ),
        marks=pytest.mark.nash_lp_strategy,
        id="test_lp_strategy_rational_4",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_kuhn_poker_efg, nonterm_outcomes=False),
            solver=functools.partial(gbt.nash.lp_solve, rational=True, use_strategic=True),
            expected=[games.kuhn_poker_lp_mixed_strategy_prof()],
        ),
        marks=pytest.mark.nash_lp_strategy,
        id="test_lp_strategy_rational_5",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_kuhn_poker_efg, nonterm_outcomes=True),
            solver=functools.partial(gbt.nash.lp_solve, rational=True, use_strategic=True),
            expected=[games.kuhn_poker_lp_mixed_strategy_prof()],
        ),
        marks=pytest.mark.nash_lp_strategy,
        id="test_lp_strategy_rational_6",
    ),
]


LP_STRATEGY_DOUBLE_CASES = [
    pytest.param(
        EquilibriumTestCase(
            factory=games.create_2x2_zero_sum_efg,
            solver=functools.partial(gbt.nash.lp_solve, rational=False, use_strategic=True),
            expected=[[d("1/2", "1/2"), d("1/2", "1/2")]],
        ),
        marks=pytest.mark.nash_lp_strategy,
        id="test_lp_strategy_double_1",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.create_2x2_zero_sum_efg, variant="missing term outcome"
            ),
            solver=functools.partial(gbt.nash.lp_solve, rational=False, use_strategic=True),
            expected=[[d("1/2", "1/2"), d("1/2", "1/2")]],
        ),
        marks=pytest.mark.nash_lp_strategy,
        id="test_lp_strategy_double_2",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.create_stripped_down_poker_efg, nonterm_outcomes=False
            ),
            solver=functools.partial(gbt.nash.lp_solve, rational=False, use_strategic=True),
            expected=[[d("1/3", "2/3", 0, 0), d("2/3", "1/3")]],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lp_strategy,
        id="test_lp_strategy_double_3",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_stripped_down_poker_efg, nonterm_outcomes=True),
            solver=functools.partial(gbt.nash.lp_solve, rational=False, use_strategic=True),
            expected=[[d("1/3", "2/3", 0, 0), d("2/3", "1/3")]],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lp_strategy,
        id="test_lp_strategy_double_4",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_kuhn_poker_efg, nonterm_outcomes=False),
            solver=functools.partial(gbt.nash.lp_solve, rational=False, use_strategic=True),
            expected=[games.kuhn_poker_lp_mixed_strategy_prof()],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lp_strategy,
        id="test_lp_strategy_double_5",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_kuhn_poker_efg, nonterm_outcomes=True),
            solver=functools.partial(gbt.nash.lp_solve, rational=False, use_strategic=True),
            expected=[games.kuhn_poker_lp_mixed_strategy_prof()],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lp_strategy,
        id="test_lp_strategy_double_6",
    ),
]


LCP_STRATEGY_RATIONAL_CASES = [
    # Zero-sum games
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_2x2_zero_sum_efg, variant=None),
            solver=functools.partial(
                gbt.nash.lcp_solve, rational=True, use_strategic=True, stop_after=None
            ),
            expected=[[d("1/2", "1/2"), d("1/2", "1/2")]],
        ),
        marks=pytest.mark.nash_lcp_strategy,
        id="test_lcp_strategy_rational_01",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.create_2x2_zero_sum_efg, variant="with neutral outcome"
            ),
            solver=functools.partial(
                gbt.nash.lcp_solve, rational=True, use_strategic=True, stop_after=None
            ),
            expected=[[d("1/2", "1/2"), d("1/2", "1/2")]],
        ),
        marks=pytest.mark.nash_lcp_strategy,
        id="test_lcp_strategy_rational_2",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.create_2x2_zero_sum_efg, variant="missing term outcome"
            ),
            solver=functools.partial(
                gbt.nash.lcp_solve, rational=True, use_strategic=True, stop_after=None
            ),
            expected=[[d("1/2", "1/2"), d("1/2", "1/2")]],
        ),
        marks=pytest.mark.nash_lcp_strategy,
        id="test_lcp_strategy_rational_3",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.create_stripped_down_poker_efg, nonterm_outcomes=False
            ),
            solver=functools.partial(
                gbt.nash.lcp_solve, rational=True, use_strategic=True, stop_after=None
            ),
            expected=[[d("1/3", "2/3", 0, 0), d("2/3", "1/3")]],
        ),
        marks=pytest.mark.nash_lcp_strategy,
        id="test_lcp_strategy_rational_4",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_stripped_down_poker_efg, nonterm_outcomes=True),
            solver=functools.partial(
                gbt.nash.lcp_solve, rational=True, use_strategic=True, stop_after=None
            ),
            expected=[[d("1/3", "2/3", 0, 0), d("2/3", "1/3")]],
        ),
        marks=pytest.mark.nash_lcp_strategy,
        id="test_lcp_strategy_rational_5",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_kuhn_poker_efg, nonterm_outcomes=False),
            solver=functools.partial(
                gbt.nash.lcp_solve, rational=True, use_strategic=True, stop_after=1
            ),
            expected=[games.kuhn_poker_lcp_first_mixed_strategy_prof()],
        ),
        marks=pytest.mark.nash_lcp_strategy,
        id="test_lcp_strategy_rational_6",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_kuhn_poker_efg, nonterm_outcomes=True),
            solver=functools.partial(
                gbt.nash.lcp_solve, rational=True, use_strategic=True, stop_after=1
            ),
            expected=[games.kuhn_poker_lcp_first_mixed_strategy_prof()],
        ),
        marks=pytest.mark.nash_lcp_strategy,
        id="test_lcp_strategy_rational_7",
    ),
    # Non-zero-sum games
    pytest.param(
        EquilibriumTestCase(
            factory=games.create_one_shot_trust_efg,
            solver=functools.partial(
                gbt.nash.lcp_solve, rational=True, use_strategic=True, stop_after=None
            ),
            expected=[[d(0, 1), d("1/2", "1/2")]],
        ),
        marks=pytest.mark.nash_lcp_strategy,
        id="test_lcp_strategy_rational_8",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_EFG_for_nxn_bimatrix_coordination_game, n=3),
            solver=functools.partial(
                gbt.nash.lcp_solve, rational=True, use_strategic=True, stop_after=None
            ),
            expected=[
                [d(1, 0, 0), d(1, 0, 0)],
                [d("1/2", "1/2", 0), d("1/2", "1/2", 0)],
                [d(0, 1, 0), d(0, 1, 0)],
                [d(0, "1/2", "1/2"), d(0, "1/2", "1/2")],
                [d("1/3", "1/3", "1/3"), d("1/3", "1/3", "1/3")],
                [d("1/2", 0, "1/2"), d("1/2", 0, "1/2")],
                [d(0, 0, 1), d(0, 0, 1)],
            ],
        ),
        marks=pytest.mark.nash_lcp_strategy,
        id="test_lcp_strategy_rational_9",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_EFG_for_nxn_bimatrix_coordination_game, n=4),
            solver=functools.partial(
                gbt.nash.lcp_solve, rational=True, use_strategic=True, stop_after=1
            ),
            expected=[[d(1, 0, 0, 0), d(1, 0, 0, 0)]],
        ),
        marks=pytest.mark.nash_lcp_strategy,
        id="test_lcp_strategy_rational_10",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=games.create_EFG_for_6x6_bimatrix_with_long_LH_paths_and_unique_eq,
            solver=functools.partial(
                gbt.nash.lcp_solve, rational=True, use_strategic=True, stop_after=None
            ),
            expected=[
                [
                    d("1/30", "1/6", "3/10", "3/10", "1/6", "1/30"),
                    d("1/6", "1/30", "3/10", "3/10", "1/30", "1/6"),
                ]
            ],
        ),
        marks=pytest.mark.nash_lcp_strategy,
        id="test_lcp_strategy_rational_11",
    ),
]


LCP_STRATEGY_DOUBLE_CASES = [
    # Zero-sum games
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_2x2_zero_sum_efg, variant=None),
            solver=functools.partial(
                gbt.nash.lcp_solve, rational=False, use_strategic=True, stop_after=None
            ),
            expected=[[d("1/2", "1/2"), d("1/2", "1/2")]],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lcp_strategy,
        id="test_lcp_strategy_double_01",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.create_2x2_zero_sum_efg, variant="with neutral outcome"
            ),
            solver=functools.partial(
                gbt.nash.lcp_solve, rational=False, use_strategic=True, stop_after=None
            ),
            expected=[[d("1/2", "1/2"), d("1/2", "1/2")]],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lcp_strategy,
        id="test_lcp_strategy_double_2",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.create_2x2_zero_sum_efg, variant="missing term outcome"
            ),
            solver=functools.partial(
                gbt.nash.lcp_solve, rational=False, use_strategic=True, stop_after=None
            ),
            expected=[[d("1/2", "1/2"), d("1/2", "1/2")]],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lcp_strategy,
        id="test_lcp_strategy_double_3",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.create_stripped_down_poker_efg, nonterm_outcomes=False
            ),
            solver=functools.partial(
                gbt.nash.lcp_solve, rational=False, use_strategic=True, stop_after=None
            ),
            expected=[[d("1/3", "2/3", 0, 0), d("2/3", "1/3")]],
        ),
        marks=pytest.mark.nash_lcp_strategy,
        id="test_lcp_strategy_double_4",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_stripped_down_poker_efg, nonterm_outcomes=True),
            solver=functools.partial(
                gbt.nash.lcp_solve, rational=False, use_strategic=True, stop_after=None
            ),
            expected=[[d("1/3", "2/3", 0, 0), d("2/3", "1/3")]],
        ),
        marks=pytest.mark.nash_lcp_strategy,
        id="test_lcp_strategy_double_5",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_kuhn_poker_efg, nonterm_outcomes=False),
            solver=functools.partial(
                gbt.nash.lcp_solve, rational=False, use_strategic=True, stop_after=1
            ),
            expected=[games.kuhn_poker_lcp_first_mixed_strategy_prof()],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lcp_strategy,
        id="test_lcp_strategy_double_6",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_kuhn_poker_efg, nonterm_outcomes=True),
            solver=functools.partial(
                gbt.nash.lcp_solve, rational=False, use_strategic=True, stop_after=1
            ),
            expected=[games.kuhn_poker_lcp_first_mixed_strategy_prof()],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lcp_strategy,
        id="test_lcp_strategy_double_7",
    ),
    # Non-zero-sum games
    pytest.param(
        EquilibriumTestCase(
            factory=games.create_one_shot_trust_efg,
            solver=functools.partial(
                gbt.nash.lcp_solve, rational=False, use_strategic=True, stop_after=None
            ),
            expected=[[d(0, 1), d("1/2", "1/2")]],
        ),
        marks=pytest.mark.nash_lcp_strategy,
        id="test_lcp_strategy_double_8",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_EFG_for_nxn_bimatrix_coordination_game, n=3),
            solver=functools.partial(
                gbt.nash.lcp_solve, rational=False, use_strategic=True, stop_after=None
            ),
            expected=[
                [d(1, 0, 0), d(1, 0, 0)],
                [d("1/2", "1/2", 0), d("1/2", "1/2", 0)],
                [d(0, 1, 0), d(0, 1, 0)],
                [d(0, "1/2", "1/2"), d(0, "1/2", "1/2")],
                [d("1/3", "1/3", "1/3"), d("1/3", "1/3", "1/3")],
                [d("1/2", 0, "1/2"), d("1/2", 0, "1/2")],
                [d(0, 0, 1), d(0, 0, 1)],
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lcp_strategy,
        id="test_lcp_strategy_double_9",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_EFG_for_nxn_bimatrix_coordination_game, n=4),
            solver=functools.partial(
                gbt.nash.lcp_solve, rational=False, use_strategic=True, stop_after=1
            ),
            expected=[[d(1, 0, 0, 0), d(1, 0, 0, 0)]],
        ),
        marks=pytest.mark.nash_lcp_strategy,
        id="test_lcp_strategy_double_10",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=games.create_EFG_for_6x6_bimatrix_with_long_LH_paths_and_unique_eq,
            solver=functools.partial(
                gbt.nash.lcp_solve, rational=False, use_strategic=True, stop_after=None
            ),
            expected=[
                [
                    d("1/30", "1/6", "3/10", "3/10", "1/6", "1/30"),
                    d("1/6", "1/30", "3/10", "3/10", "1/30", "1/6"),
                ]
            ],
            regret_tol=TOL,
            prob_tol=TOL_LARGE,
        ),
        marks=pytest.mark.nash_lcp_strategy,
        id="test_lcp_strategy_double_11",
    ),
]


LOGIT_STRATEGY_CASES = [
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "stripped_down_poker.efg"),
            solver=functools.partial(gbt.nash.logit_solve, use_strategic=True),
            expected=[[d("0.334", "0.667", 0, 0), d("0.667", "0.3324")]],
            # expected=[[d("1/3", "2/3", 0, 0), d("2/3", "1/3")]],
            # [[[0.3342446335455467, 0.6657553666093431,
            # 1.2005988475699076e-296, 2.3913775890307135e-296],
            # [0.6675673092925399, 0.33243269085235666]]]
            prob_tol=TOL_HUGE,
            regret_tol=TOL_LARGE,
        ),
        marks=pytest.mark.nash_logit_strategy,
        id="test_logic_strategy_1",
    ),
]


IPA_STRATEGY_CASES = [
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "stripped_down_poker.efg"),
            solver=gbt.nash.ipa_solve,
            expected=[[d("1/3", "2/3", 0, 0), d("2/3", "1/3")]],
        ),
        marks=pytest.mark.nash_ipa_strategy,
        id="test_ipa_1",
    ),
]


GNM_STRATEGY_CASES = [
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "stripped_down_poker.efg"),
            solver=gbt.nash.gnm_solve,
            expected=[[d("1/3", "2/3", 0, 0), d("2/3", "1/3")]],
            prob_tol=TOL_LARGE,
            regret_tol=TOL_LARGE,
        ),
        marks=pytest.mark.nash_gnm_strategy,
        id="test_gnm_1",
    ),
]


CASES = []
CASES += ENUMPURE_CASES
CASES += ENUMMIXED_RATIONAL_CASES
CASES += ENUMMIXED_DOUBLE_CASES
CASES += LP_STRATEGY_RATIONAL_CASES
CASES += LP_STRATEGY_DOUBLE_CASES
CASES += LCP_STRATEGY_RATIONAL_CASES
CASES += LCP_STRATEGY_DOUBLE_CASES
CASES += LOGIT_STRATEGY_CASES
CASES += IPA_STRATEGY_CASES
CASES += GNM_STRATEGY_CASES


@pytest.mark.nash
@pytest.mark.parametrize("test_case", CASES, ids=lambda c: c.label)
def test_nash_strategy_solver(test_case: EquilibriumTestCase, subtests) -> None:
    """Test calls of Nash solvers.

    Subtests:
    - Max regret no more than `test_case.regret_tol`
    - Equilibria are output in the expected order.  Equilibria are deemed to match if the maximum
      difference in probabilities is no more than `test_case.prob_tol`
    """
    game = test_case.factory()
    result = test_case.solver(game)
    with subtests.test("number of equilibria found"):
        assert len(result.equilibria) == len(test_case.expected)
    for i, (eq, exp) in enumerate(zip(result.equilibria, test_case.expected, strict=True)):
        with subtests.test(eq=i, check="max_regret"):
            assert eq.max_regret() <= test_case.regret_tol
        with subtests.test(eq=i, check="strategy_profile"):
            expected = game.mixed_strategy_profile(rational=True, data=exp)
            for player in game.players:
                for strategy in player.strategies:
                    assert abs(eq[strategy] - expected[strategy]) <= test_case.prob_tol


##################################################################################################
# NASH SOLVER IN MIXED BEHAVIORS
##################################################################################################


LP_BEHAVIOR_RATIONAL_CASES = [
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "two_player_perfect_info_win_lose.efg"
            ),
            solver=gbt.nash.lp_solve,
            expected=[[[d(0, 1), d(1, 0)], [d(1, 0), d(1, 0)]]],
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_rational_01",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "two_player_perfect_info_win_lose_with_nonterm_outcomes.efg"
            ),
            solver=gbt.nash.lp_solve,
            expected=[[[d(0, 1), d(1, 0)], [d(1, 0), d(1, 0)]]],
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_rational_2",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.create_2x2_zero_sum_efg, variant="missing term outcome"
            ),
            solver=gbt.nash.lp_solve,
            expected=[[[d("1/2", "1/2")], [d("1/2", "1/2")]]],
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_rational_3",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.create_2x2_zero_sum_efg, variant="with neutral outcome"
            ),
            solver=gbt.nash.lp_solve,
            expected=[[[d("1/2", "1/2")], [d("1/2", "1/2")]]],
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_rational_4",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_2x2_zero_sum_efg, variant=None),
            solver=gbt.nash.lp_solve,
            expected=[[[d("1/2", "1/2")], [d("1/2", "1/2")]]],
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_rational_5",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.create_stripped_down_poker_efg, nonterm_outcomes=False
            ),
            solver=gbt.nash.lp_solve,
            expected=[
                [
                    [d(1, 0), d("1/3", "2/3")],
                    [d("2/3", "1/3")],
                ]
            ],
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_rational_6",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_stripped_down_poker_efg, nonterm_outcomes=True),
            solver=gbt.nash.lp_solve,
            expected=[
                [
                    [d(1, 0), d("1/3", "2/3")],
                    [d("2/3", "1/3")],
                ]
            ],
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_rational_7",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_kuhn_poker_efg, nonterm_outcomes=False),
            solver=gbt.nash.lp_solve,
            expected=[
                [
                    [d(1, 0), d(1, 0), d(1, 0), d("2/3", "1/3"), d(1, 0), d(0, 1)],
                    [d(1, 0), d("2/3", "1/3"), d(0, 1), d(0, 1), d("2/3", "1/3"), d(1, 0)],
                ]
            ],
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_rational_8",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_kuhn_poker_efg, nonterm_outcomes=True),
            solver=gbt.nash.lp_solve,
            expected=[
                [
                    [d(1, 0), d(1, 0), d(1, 0), d("2/3", "1/3"), d(1, 0), d(0, 1)],
                    [d(1, 0), d("2/3", "1/3"), d(0, 1), d(0, 1), d("2/3", "1/3"), d(1, 0)],
                ]
            ],
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_rational_9",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "zerosum_efg_from_sequence_form_STOC94_paper.efg"
            ),
            solver=gbt.nash.lp_solve,
            expected=[
                [
                    [d(0, 1), d("2/3", "1/3"), d("1/3", "2/3")],
                    [d("5/6", "1/6"), d("5/9", "4/9")],
                ]
            ],
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_rational_10",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "perfect_info_with_chance.efg"),
            solver=gbt.nash.lp_solve,
            expected=[
                [
                    [d(0, 1)],
                    [d(1, 0), d(1, 0)],
                ]
            ],
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_rational_11",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "2_player_chance.efg"),
            solver=gbt.nash.lp_solve,
            expected=[
                [
                    [d("1/3", 0, "2/3"), d("2/3", 0, "1/3")],
                    [d("2/3", "1/3"), d("2/3", "1/3"), d("1/3", "2/3")],
                ]
            ],
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_rational_12",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file,
                "2_player_chance_nonterm_outcomes_and_missing_term_outcomes.efg",
            ),
            solver=gbt.nash.lp_solve,
            expected=[
                [
                    [d("1/3", 0, "2/3"), d("2/3", 0, "1/3")],
                    [d("2/3", "1/3"), d("2/3", "1/3"), d("1/3", "2/3")],
                ]
            ],
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_rational_13",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "large_payoff_game.efg"),
            solver=gbt.nash.lp_solve,
            expected=[
                [
                    [d(1, 0), d(1, 0)],
                    [
                        d(0, 1),
                        d("9999999999999999999/10000000000000000000", "1/10000000000000000000"),
                    ],
                ]
            ],
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_rational_14",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "chance_in_middle.efg"),
            solver=gbt.nash.lp_solve,
            expected=[
                [
                    [d("3/11", "8/11"), d(1, 0), d(1, 0), d(1, 0), d(1, 0)],
                    [d(1, 0), d("6/11", "5/11")],
                ]
            ],
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_rational_15",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "chance_in_middle_with_nonterm_outcomes.efg"
            ),
            solver=gbt.nash.lp_solve,
            expected=[
                [
                    [d("3/11", "8/11"), d(1, 0), d(1, 0), d(1, 0), d(1, 0)],
                    [d(1, 0), d("6/11", "5/11")],
                ]
            ],
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_rational_16",
    ),
]


LP_BEHAVIOR_DOUBLE_CASES = [
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "two_player_perfect_info_win_lose.efg"
            ),
            solver=functools.partial(gbt.nash.lp_solve, rational=False),
            expected=[[[d(0, 1), d(1, 0)], [d(1, 0), d(1, 0)]]],
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_double_01",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "two_player_perfect_info_win_lose_with_nonterm_outcomes.efg"
            ),
            solver=functools.partial(gbt.nash.lp_solve, rational=False),
            expected=[[[d(0, 1), d(1, 0)], [d(1, 0), d(1, 0)]]],
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_double_2",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.create_2x2_zero_sum_efg, variant="missing term outcome"
            ),
            solver=functools.partial(gbt.nash.lp_solve, rational=False),
            expected=[[[d("1/2", "1/2")], [d("1/2", "1/2")]]],
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_double_3",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.create_2x2_zero_sum_efg, variant="with neutral outcome"
            ),
            solver=functools.partial(gbt.nash.lp_solve, rational=False),
            expected=[[[d("1/2", "1/2")], [d("1/2", "1/2")]]],
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_double_4",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_2x2_zero_sum_efg, variant=None),
            solver=functools.partial(gbt.nash.lp_solve, rational=False),
            expected=[[[d("1/2", "1/2")], [d("1/2", "1/2")]]],
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_double_5",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.create_stripped_down_poker_efg, nonterm_outcomes=False
            ),
            solver=functools.partial(gbt.nash.lp_solve, rational=False),
            expected=[
                [
                    [d(1, 0), d("1/3", "2/3")],
                    [d("2/3", "1/3")],
                ]
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_double_6",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_stripped_down_poker_efg, nonterm_outcomes=True),
            solver=functools.partial(gbt.nash.lp_solve, rational=False),
            expected=[
                [
                    [d(1, 0), d("1/3", "2/3")],
                    [d("2/3", "1/3")],
                ]
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_double_7",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_kuhn_poker_efg, nonterm_outcomes=False),
            solver=functools.partial(gbt.nash.lp_solve, rational=False),
            expected=[
                [
                    [d("2/3", "1/3"), d(1, 0), d(1, 0), d("1/3", "2/3"), d(0, 1), d("1/2", "1/2")],
                    [d(1, 0), d("2/3", "1/3"), d(0, 1), d(0, 1), d("2/3", "1/3"), d(1, 0)],
                ]
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_double_8",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_kuhn_poker_efg, nonterm_outcomes=True),
            solver=functools.partial(gbt.nash.lp_solve, rational=False),
            expected=[
                [
                    [d("2/3", "1/3"), d(1, 0), d(1, 0), d("1/3", "2/3"), d(0, 1), d("1/2", "1/2")],
                    [d(1, 0), d("2/3", "1/3"), d(0, 1), d(0, 1), d("2/3", "1/3"), d(1, 0)],
                ]
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_double_9",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "zerosum_efg_from_sequence_form_STOC94_paper.efg"
            ),
            solver=functools.partial(gbt.nash.lp_solve, rational=False),
            expected=[
                [
                    [d(0, 1), d("2/3", "1/3"), d("1/3", "2/3")],
                    [d("5/6", "1/6"), d("5/9", "4/9")],
                ]
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_double_10",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "perfect_info_with_chance.efg"),
            solver=functools.partial(gbt.nash.lp_solve, rational=False),
            expected=[
                [
                    [d(0, 1)],
                    [d(1, 0), d(1, 0)],
                ]
            ],
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_double_11",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "2_player_chance.efg"),
            solver=functools.partial(gbt.nash.lp_solve, rational=False),
            expected=[
                [
                    [d("1/3", 0, "2/3"), d("2/3", 0, "1/3")],
                    [d("2/3", "1/3"), d("2/3", "1/3"), d("1/3", "2/3")],
                ]
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_double_12",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file,
                "2_player_chance_nonterm_outcomes_and_missing_term_outcomes.efg",
            ),
            solver=functools.partial(gbt.nash.lp_solve, rational=False),
            expected=[
                [
                    [d("1/3", 0, "2/3"), d("2/3", 0, "1/3")],
                    [d("2/3", "1/3"), d("2/3", "1/3"), d("1/3", "2/3")],
                ]
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_double_13",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "large_payoff_game.efg"),
            solver=functools.partial(gbt.nash.lp_solve, rational=False),
            expected=[
                [
                    [d(1, 0), d(1, 0)],
                    [
                        d(0, 1),
                        d("9999999999999999999/10000000000000000000", "1/10000000000000000000"),
                    ],
                ]
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=[
            pytest.mark.nash_lp_behavior,
            pytest.mark.xfail(reason="Problem with large payoffs when working in floats"),
        ],
        id="test_lp_behavior_double_14",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "chance_in_middle.efg"),
            solver=functools.partial(gbt.nash.lp_solve, rational=False),
            expected=[
                [
                    [d("3/11", "8/11"), d(1, 0), d(1, 0), d(1, 0), d(1, 0)],
                    [d(1, 0), d("6/11", "5/11")],
                ]
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_double_15",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "chance_in_middle_with_nonterm_outcomes.efg"
            ),
            solver=functools.partial(gbt.nash.lp_solve, rational=False),
            expected=[
                [
                    [d("3/11", "8/11"), d(1, 0), d(1, 0), d(1, 0), d(1, 0)],
                    [d(1, 0), d("6/11", "5/11")],
                ]
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_double_16",
    ),
]


LCP_BEHAVIOR_RATIONAL_CASES = [
    # Zero-sum games (also tested with lp solve)
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_2x2_zero_sum_efg, variant=None),
            solver=gbt.nash.lcp_solve,
            expected=[[[d("1/2", "1/2")], [d("1/2", "1/2")]]],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_rational_01",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.create_2x2_zero_sum_efg, variant="with neutral outcome"
            ),
            solver=gbt.nash.lcp_solve,
            expected=[[[d("1/2", "1/2")], [d("1/2", "1/2")]]],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_rational_02",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.create_2x2_zero_sum_efg, variant="missing term outcome"
            ),
            solver=gbt.nash.lcp_solve,
            expected=[[[d("1/2", "1/2")], [d("1/2", "1/2")]]],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_rational_3",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_stripped_down_poker_efg, nonterm_outcomes=True),
            solver=gbt.nash.lcp_solve,
            expected=[[[d(1, 0), d("1/3", "2/3")], [d("2/3", "1/3")]]],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_rational_4",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_kuhn_poker_efg, nonterm_outcomes=False),
            solver=gbt.nash.lcp_solve,
            expected=[
                [
                    [d("2/3", "1/3"), d(1, 0), d(1, 0), d("1/3", "2/3"), d(0, 1), d("1/2", "1/2")],
                    [d(1, 0), d("2/3", "1/3"), d(0, 1), d(0, 1), d("2/3", "1/3"), d(1, 0)],
                ]
            ],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_rational_5",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_kuhn_poker_efg, nonterm_outcomes=True),
            solver=gbt.nash.lcp_solve,
            expected=[
                [
                    [d("2/3", "1/3"), d(1, 0), d(1, 0), d("1/3", "2/3"), d(0, 1), d("1/2", "1/2")],
                    [d(1, 0), d("2/3", "1/3"), d(0, 1), d(0, 1), d("2/3", "1/3"), d(1, 0)],
                ]
            ],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_rational_6",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "perfect_info_with_chance.efg"),
            solver=gbt.nash.lcp_solve,
            expected=[[[d(0, 1)], [d(0, 1), d(0, 1)]]],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_rational_7",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "two_player_perfect_info_win_lose.efg"
            ),
            solver=gbt.nash.lcp_solve,
            expected=[[[d(0, 1), d(1, 0)], [d(0, 1), d("1/2", "1/2")]]],
            # 1/2-1/2 for l/r is determined by MixedBehaviorProfile.UndefinedToCentroid()
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_rational_8",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "two_player_perfect_info_win_lose_with_nonterm_outcomes.efg"
            ),
            solver=gbt.nash.lcp_solve,
            expected=[[[d(0, 1), d(1, 0)], [d(0, 1), d("1/2", "1/2")]]],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_rational_9",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "2_player_chance.efg"),
            solver=gbt.nash.lcp_solve,
            expected=[
                [
                    [d("1/3", 0, "2/3"), d("2/3", 0, "1/3")],
                    [d("2/3", "1/3"), d("1/3", "2/3"), d("1/3", "2/3")],
                ]
            ],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_rational_10",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file,
                "2_player_chance_nonterm_outcomes_and_missing_term_outcomes.efg",
            ),
            solver=gbt.nash.lcp_solve,
            expected=[
                [
                    [d("1/3", 0, "2/3"), d("2/3", 0, "1/3")],
                    [d("2/3", "1/3"), d("1/3", "2/3"), d("1/3", "2/3")],
                ]
            ],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_rational_11",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "large_payoff_game.efg"),
            solver=gbt.nash.lcp_solve,
            expected=[
                [
                    [d(1, 0), d(1, 0)],
                    [
                        d(0, 1),
                        d("9999999999999999999/10000000000000000000", "1/10000000000000000000"),
                    ],
                ]
            ],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_rational_12",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "chance_in_middle.efg"),
            solver=gbt.nash.lcp_solve,
            expected=[
                [
                    [d("3/11", "8/11"), d(1, 0), d(1, 0), d(1, 0), d(1, 0)],
                    [d(1, 0), d("6/11", "5/11")],
                ]
            ],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_rational_13",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "chance_in_middle_with_nonterm_outcomes.efg"
            ),
            solver=gbt.nash.lcp_solve,
            expected=[
                [
                    [d("3/11", "8/11"), d(1, 0), d(1, 0), d(1, 0), d(1, 0)],
                    [d(1, 0), d("6/11", "5/11")],
                ]
            ],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_rational_14",
    ),
    # Non-zero-sum games
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "reduction_both_players_payoff_ties_GTE_survey.efg"
            ),
            solver=gbt.nash.lcp_solve,
            expected=[[[d(0, 0, 1, 0), d(1, 0)], [d(0, 1), d(0, 1), d(0, 1), d(0, 1)]]],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_rational_15",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=games.create_EFG_for_6x6_bimatrix_with_long_LH_paths_and_unique_eq,
            solver=gbt.nash.lcp_solve,
            expected=[
                [
                    [d("1/30", "1/6", "3/10", "3/10", "1/6", "1/30")],
                    [d("1/6", "1/30", "3/10", "3/10", "1/30", "1/6")],
                ]
            ],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_rational_16",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_EFG_for_nxn_bimatrix_coordination_game, 3),
            solver=gbt.nash.lcp_solve,
            expected=[[[d(0, 0, 1)], [d(0, 0, 1)]]],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_rational_17",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_EFG_for_nxn_bimatrix_coordination_game, 4),
            solver=gbt.nash.lcp_solve,
            expected=[[[d(0, 0, 0, 1)], [d(0, 0, 0, 1)]]],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_rational_18",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "entry_accommodation.efg"),
            solver=gbt.nash.lcp_solve,
            expected=[[[d("2/3", "1/3"), d(1, 0), d(1, 0)], [d("2/3", "1/3")]]],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_rational_19",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "entry_accommodation_with_nonterm_outcomes.efg"
            ),
            solver=gbt.nash.lcp_solve,
            expected=[[[d("2/3", "1/3"), d(1, 0), d(1, 0)], [d("2/3", "1/3")]]],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_rational_20",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "2_player_non_zero_sum.efg"),
            solver=gbt.nash.lcp_solve,
            expected=[[[d("1/3", "2/3")], [d("1/2", "1/2")]]],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_rational_21",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "2_player_non_zero_sum_missing_term_outcome.efg"
            ),
            solver=gbt.nash.lcp_solve,
            expected=[[[d("1/3", "2/3")], [d("1/2", "1/2")]]],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_rational_22",
    ),
]


LCP_BEHAVIOR_DOUBLE_CASES = [
    # Zero-sum games (also tested with lp solve)
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_2x2_zero_sum_efg, variant=None),
            solver=functools.partial(gbt.nash.lcp_solve, rational=False),
            expected=[[[d("1/2", "1/2")], [d("1/2", "1/2")]]],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_double_01",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.create_2x2_zero_sum_efg, variant="with neutral outcome"
            ),
            solver=functools.partial(gbt.nash.lcp_solve, rational=False),
            expected=[[[d("1/2", "1/2")], [d("1/2", "1/2")]]],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_double_02",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.create_2x2_zero_sum_efg, variant="missing term outcome"
            ),
            solver=functools.partial(gbt.nash.lcp_solve, rational=False),
            expected=[[[d("1/2", "1/2")], [d("1/2", "1/2")]]],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_double_3",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_stripped_down_poker_efg, nonterm_outcomes=True),
            solver=functools.partial(gbt.nash.lcp_solve, rational=False),
            expected=[[[d(1, 0), d("1/3", "2/3")], [d("2/3", "1/3")]]],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_double_4",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_kuhn_poker_efg, nonterm_outcomes=False),
            solver=functools.partial(gbt.nash.lcp_solve, rational=False),
            expected=[
                [
                    [d("2/3", "1/3"), d(1, 0), d(1, 0), d("1/3", "2/3"), d(0, 1), d("1/2", "1/2")],
                    [d(1, 0), d("2/3", "1/3"), d(0, 1), d(0, 1), d("2/3", "1/3"), d(1, 0)],
                ]
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_double_5",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_kuhn_poker_efg, nonterm_outcomes=True),
            solver=functools.partial(gbt.nash.lcp_solve, rational=False),
            expected=[
                [
                    [d("2/3", "1/3"), d(1, 0), d(1, 0), d("1/3", "2/3"), d(0, 1), d("1/2", "1/2")],
                    [d(1, 0), d("2/3", "1/3"), d(0, 1), d(0, 1), d("2/3", "1/3"), d(1, 0)],
                ]
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_double_6",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "perfect_info_with_chance.efg"),
            solver=functools.partial(gbt.nash.lcp_solve, rational=False),
            expected=[[[d(0, 1)], [d(0, 1), d(0, 1)]]],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_double_7",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "two_player_perfect_info_win_lose.efg"
            ),
            solver=functools.partial(gbt.nash.lcp_solve, rational=False),
            expected=[[[d(0, 1), d(1, 0)], [d(0, 1), d("1/2", "1/2")]]],
            # 1/2-1/2 for l/r is determined by MixedBehaviorProfile.UndefinedToCentroid()
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_double_8",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "two_player_perfect_info_win_lose_with_nonterm_outcomes.efg"
            ),
            solver=functools.partial(gbt.nash.lcp_solve, rational=False),
            expected=[[[d(0, 1), d(1, 0)], [d(0, 1), d("1/2", "1/2")]]],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_double_9",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "2_player_chance.efg"),
            solver=functools.partial(gbt.nash.lcp_solve, rational=False),
            expected=[
                [
                    [d("1/3", 0, "2/3"), d("2/3", 0, "1/3")],
                    [d("2/3", "1/3"), d("1/3", "2/3"), d("1/3", "2/3")],
                ]
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_double_10",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file,
                "2_player_chance_nonterm_outcomes_and_missing_term_outcomes.efg",
            ),
            solver=functools.partial(gbt.nash.lcp_solve, rational=False),
            expected=[
                [
                    [d("1/3", 0, "2/3"), d("2/3", 0, "1/3")],
                    [d("2/3", "1/3"), d("1/3", "2/3"), d("1/3", "2/3")],
                ]
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_double_11",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "large_payoff_game.efg"),
            solver=functools.partial(gbt.nash.lcp_solve, rational=False),
            expected=[
                [
                    [d(1, 0), d(1, 0)],
                    [
                        d(0, 1),
                        d("9999999999999999999/10000000000000000000", "1/10000000000000000000"),
                    ],
                ]
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=[
            pytest.mark.nash_lcp_behavior,
            pytest.mark.xfail(reason="Problem with large payoffs when working in floats"),
        ],
        id="test_lcp_behavior_double_12",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "chance_in_middle.efg"),
            solver=functools.partial(gbt.nash.lcp_solve, rational=False),
            expected=[
                [
                    [d("3/11", "8/11"), d(1, 0), d(1, 0), d(1, 0), d(1, 0)],
                    [d(1, 0), d("6/11", "5/11")],
                ]
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_double_13",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "chance_in_middle_with_nonterm_outcomes.efg"
            ),
            solver=functools.partial(gbt.nash.lcp_solve, rational=False),
            expected=[
                [
                    [d("3/11", "8/11"), d(1, 0), d(1, 0), d(1, 0), d(1, 0)],
                    [d(1, 0), d("6/11", "5/11")],
                ]
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_double_14",
    ),
    # Non-zero-sum games
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "reduction_both_players_payoff_ties_GTE_survey.efg"
            ),
            solver=functools.partial(gbt.nash.lcp_solve, rational=False),
            expected=[[[d(0, 0, 1, 0), d(1, 0)], [d(0, 1), d(0, 1), d(0, 1), d(0, 1)]]],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_double_15",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=games.create_EFG_for_6x6_bimatrix_with_long_LH_paths_and_unique_eq,
            solver=functools.partial(gbt.nash.lcp_solve, rational=False),
            expected=[
                [
                    [d("1/30", "1/6", "3/10", "3/10", "1/6", "1/30")],
                    [d("1/6", "1/30", "3/10", "3/10", "1/30", "1/6")],
                ]
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_double_16",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_EFG_for_nxn_bimatrix_coordination_game, 3),
            solver=functools.partial(gbt.nash.lcp_solve, rational=False),
            expected=[[[d(0, 0, 1)], [d(0, 0, 1)]]],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_double_17",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_EFG_for_nxn_bimatrix_coordination_game, 4),
            solver=functools.partial(gbt.nash.lcp_solve, rational=False),
            expected=[[[d(0, 0, 0, 1)], [d(0, 0, 0, 1)]]],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_double_18",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "entry_accommodation.efg"),
            solver=functools.partial(gbt.nash.lcp_solve, rational=False),
            expected=[[[d("2/3", "1/3"), d(1, 0), d(1, 0)], [d("2/3", "1/3")]]],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_double_19",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "entry_accommodation_with_nonterm_outcomes.efg"
            ),
            solver=functools.partial(gbt.nash.lcp_solve, rational=False),
            expected=[[[d("2/3", "1/3"), d(1, 0), d(1, 0)], [d("2/3", "1/3")]]],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_double_20",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "2_player_non_zero_sum.efg"),
            solver=functools.partial(gbt.nash.lcp_solve, rational=False),
            expected=[[[d("1/3", "2/3")], [d("1/2", "1/2")]]],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_double_21",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "2_player_non_zero_sum_missing_term_outcome.efg"
            ),
            solver=functools.partial(gbt.nash.lcp_solve, rational=False),
            expected=[[[d("1/3", "2/3")], [d("1/2", "1/2")]]],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_double_22",
    ),
]


ENUMPOLY_BEHAVIOR_CASES = [
    # #############################################################
    # Examples where Nash pure behaviors and agent-form pure equillibrium behaviors coincide
    # #############################################################
    # 2-player zero-sum games
    pytest.param(
        EquilibriumTestCase(
            factory=games.create_stripped_down_poker_efg,
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=None),
            expected=[
                [[d(1, 0), d("1/3", "2/3")], [d("2/3", "1/3")]],
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_enumpoly_behavior,
        id="test_enumpoly_behavior_01",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "2_player_non_zero_sum.efg"),
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=1),
            expected=[
                [[d("1/3", "2/3")], [d("1/2", "1/2")]],
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_enumpoly_behavior,
        id="test_enumpoly_behavior_2",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "2_player_non_zero_sum_missing_term_outcome.efg"
            ),
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=1),
            expected=[
                [[d("1/3", "2/3")], [d("1/2", "1/2")]],
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_enumpoly_behavior,
        id="test_enumpoly_behavior_3",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "chance_in_middle.efg"),
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=1),
            expected=[
                [
                    [d("3/11", "8/11"), d(1, 0), d(1, 0), d(1, 0), d(1, 0)],
                    [d(1, 0), d("6/11", "5/11")],
                ],
                # [[[1, 0], [1, 0], [1, 0], [0, 0], [0, 0]], [[0, 1], [1, 0]]],
                # [[[0, 1], [0, 0], [0, 0], [1, 0], [1, 0]], [[1, 0], [0, 1]]],
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_enumpoly_behavior,
        id="test_enumpoly_behavior_4",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "chance_in_middle_with_nonterm_outcomes.efg"
            ),
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=1),
            expected=[
                [
                    [d("3/11", "8/11"), d(1, 0), d(1, 0), d(1, 0), d(1, 0)],
                    [d(1, 0), d("6/11", "5/11")],
                ],
                # [[[1, 0], [1, 0], [1, 0], [0, 0], [0, 0]], [[0, 1], [1, 0]]],
                # [[[0, 1], [0, 0], [0, 0], [1, 0], [1, 0]], [[1, 0], [0, 1]]],
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_enumpoly_behavior,
        id="test_enumpoly_behavior_5",
    ),
    # 3-player games
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_EFG_for_nxn_bimatrix_coordination_game, n=3),
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=None),
            expected=[
                [[d("1/3", "1/3", "1/3")], [d("1/3", "1/3", "1/3")]],
                [[d("1/2", "1/2", 0)], [d("1/2", "1/2", 0)]],
                [[d("1/2", 0, "1/2")], [d("1/2", 0, "1/2")]],
                [[d(1, 0, 0)], [d(1, 0, 0)]],
                [[d(0, "1/2", "1/2")], [d(0, "1/2", "1/2")]],
                [[d(0, 1, 0)], [d(0, 1, 0)]],
                [[d(0, 0, 1)], [d(0, 0, 1)]],
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_enumpoly_behavior,
        id="test_enumpoly_behavior_6",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "3_player.efg"),
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=2),
            expected=[
                [[d(1, 0), d(1, 0)], [d(1, 0), d("1/2", "1/2")], [d(1, 0), d(0, 1)]],
                [[d(1, 0), d(1, 0)], [d(1, 0), d(0, 1)], [d(1, 0), d("1/3", "2/3")]],
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_enumpoly_behavior,
        id="test_enumpoly_behavior_7",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "3_player_with_nonterm_outcomes.efg"),
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=2),
            expected=[
                [[d(1, 0), d(1, 0)], [d(1, 0), d("1/2", "1/2")], [d(1, 0), d(0, 1)]],
                [[d(1, 0), d(1, 0)], [d(1, 0), d(0, 1)], [d(1, 0), d("1/3", "2/3")]],
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_enumpoly_behavior,
        id="test_enumpoly_behavior_8",
    ),
    # 4-player game
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_EFG_for_nxn_bimatrix_coordination_game, n=4),
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=1),
            expected=[
                [[d("1/4", "1/4", "1/4", "1/4")], [d("1/4", "1/4", "1/4", "1/4")]],
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_enumpoly_behavior,
        id="test_enumpoly_behavior_9",
    ),
]
# ##############################################################################
# 3-player game
# (
# games.read_from_file("mixed_behavior_game.efg"),
# [
# [[["1/2", "1/2"]], [["2/5", "3/5"]], [["1/4", "3/4"]]],
# [[["2/5", "3/5"]], [["1/2", "1/2"]], [["1/3", "2/3"]]],
# ],
# 2,  # 9 in total found by enumpoly (see unordered test)
# ),
# ##############################################################################


LOGIT_BEHAVIOR_CASES = [
    pytest.param(
        EquilibriumTestCase(
            factory=games.create_stripped_down_poker_efg,
            solver=gbt.nash.logit_solve,
            expected=[
                [[d(1, 0), d("1/3", "2/3")], [d("2/3", "1/3")]],
            ],
            regret_tol=TOL_LARGE,
            prob_tol=TOL_LARGE,
        ),
        marks=pytest.mark.nash_logit_behavior,
        id="test_logit_behavior_01",
    ),
]


CASES = []
CASES += LP_BEHAVIOR_RATIONAL_CASES
CASES += LP_BEHAVIOR_DOUBLE_CASES
CASES += LCP_BEHAVIOR_RATIONAL_CASES
CASES += LCP_BEHAVIOR_DOUBLE_CASES
CASES += ENUMPOLY_BEHAVIOR_CASES
CASES += LOGIT_BEHAVIOR_CASES


@pytest.mark.nash
@pytest.mark.parametrize("test_case", CASES, ids=lambda c: c.label)
def test_nash_behavior_solver(test_case: EquilibriumTestCase, subtests) -> None:
    """Test calls of Nash solvers in EFGs in mixed behaviors

    Subtests:
    - Max regret no more than `test_case.regret_tol`
    - Agent max regret no more than max regret (+ `test_case.regret_tol`)
    - Equilibria are output in the expected order.  Equilibria are deemed to match if the maximum
      difference in probabilities is no more than `test_case.prob_tol`
    """
    game = test_case.factory()
    result = test_case.solver(game)
    with subtests.test("number of equilibria found"):
        assert len(result.equilibria) == len(test_case.expected)
    for i, (eq, exp) in enumerate(zip(result.equilibria, test_case.expected, strict=True)):
        with subtests.test(eq=i, check="max_regret"):
            assert eq.max_regret() <= test_case.regret_tol
        with subtests.test(eq=i, check="max_regret"):
            assert eq.agent_max_regret() <= eq.max_regret() + test_case.regret_tol
        with subtests.test(eq=i, check="strategy_profile"):
            expected = game.mixed_behavior_profile(rational=True, data=exp)
            for player in game.players:
                for action in player.actions:
                    assert abs(eq[action] - expected[action]) <= test_case.prob_tol


##################################################################################################
# BEHAVIOR SOLVER WITHOUT SUBTESTS -- TEMP FOR ISSUE 660
##################################################################################################

ENUMPOLY_ISSUE_660_CASES = [
    # 2-player non-zero-sum games
    pytest.param(
        EquilibriumTestCase(
            factory=games.create_one_shot_trust_efg,
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=None),
            expected=[
                [[d(0, 1)], [d("1/2", "1/2")]],
                [[d(0, 1)], [d(0, 1)]],
                # second entry assumes we extend to Nash using only pure behaviors
                # currently we get [[0, 1]], [[0, 0]]] as a second eq
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=[
            pytest.mark.nash_enumpoly_behavior,
            pytest.mark.xfail(reason="Problem with enumpoly, as per issue #660"),
        ],
        id="enumpoly_one_shot_trust_issue_660",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_one_shot_trust_efg, unique_NE_variant=True),
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=None),
            expected=[
                [[[d(1, 0)], [d(0, 1)]]],
                # currently we get [d(0, 1)], [d(0, 0)]] as a second eq
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=[
            pytest.mark.nash_enumpoly_behavior,
            pytest.mark.xfail(reason="Problem with enumpoly, as per issue #660"),
        ],
        id="enumpoly_one_shot_trust_unique_NE_issue_660",
    ),
]


@pytest.mark.nash
@pytest.mark.parametrize("test_case", ENUMPOLY_ISSUE_660_CASES, ids=lambda c: c.label)
def test_nash_behavior_solver_no_subtests_only_profile(test_case: EquilibriumTestCase) -> None:
    """TEMP: to be included with test_nash_behavior_solver when 660 is resolved."""
    game = test_case.factory()
    result = test_case.solver(game)
    assert len(result.equilibria) == len(test_case.expected)
    for eq, exp in zip(result.equilibria, test_case.expected, strict=True):
        expected = game.mixed_behavior_profile(rational=True, data=exp)
        for player in game.players:
            for action in player.actions:
                assert abs(eq[action] - expected[action]) <= test_case.prob_tol


##################################################################################################
# BEHVAIOR SOLVER -- UNORDERED
##################################################################################################

ENUMPOLY_BEHAVIOR_UNORDERED_CASES = [
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "mixed_behavior_game.efg"),
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=9),
            expected=[
                [[["2/5", "3/5"]], [["1/2", "1/2"]], [["1/3", "2/3"]]],
                [[["1/2", "1/2"]], [["2/5", "3/5"]], [["1/4", "3/4"]]],
                [[["1/2", "1/2"]], [["1/2", "1/2"]], [[1, 0]]],
                [[["1/3", "2/3"]], [[1, 0]], [["1/4", "3/4"]]],
                [[[1, 0]], [[1, 0]], [[1, 0]]],
                [[[1, 0]], [[0, 1]], [[0, 1]]],
                [[[0, 1]], [["1/4", "3/4"]], [["1/3", "2/3"]]],
                [[[0, 1]], [[1, 0]], [[0, 1]]],
                [[[0, 1]], [[0, 1]], [[1, 0]]],
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_enumpoly_behavior,
        id="test_enumpoly_behavior_unordered_1",
    ),
]


@pytest.mark.nash
@pytest.mark.parametrize("test_case", ENUMPOLY_BEHAVIOR_UNORDERED_CASES, ids=lambda c: c.label)
def test_nash_behavior_solver_unordered(test_case: EquilibriumTestCase, subtests) -> None:
    """Test calls of Nash solvers in EFGs in mixed behaviors -- UNORDERED

    Subtests:
    - Agent max regret no more than `test_case.regret_tol`
    - Agent max regret no more than max regret (+ `test_case.regret_tol`)
    - Equilibria that are output are distinct and all appear in the expected set
      Equilibria are deemed to match if the maximum difference in probabilities is no more
      than `test_case.prob_tol`
    """

    def are_the_same(game, found, candidate):
        for p in game.players:
            for a in p.actions:
                if not abs(found[a] - candidate[a]) <= TOL:
                    return False
        return True

    game = test_case.factory()
    result = test_case.solver(game)
    with subtests.test("number of equilibria found"):
        assert len(result.equilibria) == len(test_case.expected)
    for i, eq in enumerate(result.equilibria):
        with subtests.test(eq=i, check="agent_max_regret"):
            assert eq.max_regret() <= test_case.regret_tol
        with subtests.test(eq=i, check="max_regret"):
            assert eq.agent_max_regret() <= eq.max_regret() + test_case.regret_tol
        with subtests.test(eq=i, check="strategy_profile"):
            found = False
            for exp in test_case.expected[:]:
                expected = game.mixed_behavior_profile(rational=True, data=exp)
                if are_the_same(game, eq, expected):
                    test_case.expected.remove(exp)
                    found = True
                    break
            assert found


##################################################################################################
# AGENTS NASH SOLVERS (IN MIXED BEHAVIORS)
##################################################################################################


ENUMPURE_AGENT_CASES = [
    # #############################################################
    # Examples where Nash pure behaviors and agent-form pure equillibrium behaviors coincide
    # #############################################################
    # Zero-sum games
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "two_player_perfect_info_win_lose.efg"
            ),
            solver=functools.partial(gbt.nash.enumpure_agent_solve),
            expected=[
                [[d(1, 0), d(1, 0)], [d(0, 1), d(1, 0)]],
                [[d(0, 1), d(1, 0)], [d(1, 0), d(1, 0)]],
                [[d(0, 1), d(1, 0)], [d(1, 0), d(0, 1)]],
                [[d(0, 1), d(1, 0)], [d(0, 1), d(1, 0)]],
                [[d(0, 1), d(1, 0)], [d(0, 1), d(0, 1)]],
            ],
        ),
        marks=pytest.mark.nash_enumpure_strategy,
        id="test_enumpure_agent_1",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=games.create_stripped_down_poker_efg,
            solver=functools.partial(gbt.nash.enumpure_agent_solve),
            expected=[],
        ),
        marks=pytest.mark.nash_enumpure_strategy,
        id="test_enumpure_agent_2",
    ),
    # Non-zero-sum 2-player games
    pytest.param(
        EquilibriumTestCase(
            factory=games.create_one_shot_trust_efg,
            solver=functools.partial(gbt.nash.enumpure_agent_solve),
            expected=[
                [[d(0, 1)], [d(0, 1)]],
            ],
        ),
        marks=pytest.mark.nash_enumpure_strategy,
        id="test_enumpure_agent_3",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_one_shot_trust_efg, unique_NE_variant=True),
            solver=functools.partial(gbt.nash.enumpure_agent_solve),
            expected=[
                [[d(1, 0)], [d(0, 1)]],
            ],
        ),
        marks=pytest.mark.nash_enumpure_strategy,
        id="test_enumpure_agent_4",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_EFG_for_nxn_bimatrix_coordination_game, n=3),
            solver=functools.partial(gbt.nash.enumpure_agent_solve),
            expected=[
                [[d(1, 0, 0)], [d(1, 0, 0)]],
                [[d(0, 1, 0)], [d(0, 1, 0)]],
                [[d(0, 0, 1)], [d(0, 0, 1)]],
            ],
        ),
        marks=pytest.mark.nash_enumpure_strategy,
        id="test_enumpure_agent_5",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.create_EFG_for_6x6_bimatrix_with_long_LH_paths_and_unique_eq
            ),
            solver=functools.partial(gbt.nash.enumpure_agent_solve),
            expected=[],
        ),
        marks=pytest.mark.nash_enumpure_strategy,
        id="test_enumpure_agent_6",
    ),
    # 3-player games
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "mixed_behavior_game.efg"),
            solver=functools.partial(gbt.nash.enumpure_agent_solve),
            expected=[
                [[d(1, 0)], [d(1, 0)], [d(1, 0)]],
                [[d(1, 0)], [d(0, 1)], [d(0, 1)]],
                [[d(0, 1)], [d(1, 0)], [d(0, 1)]],
                [[d(0, 1)], [d(0, 1)], [d(1, 0)]],
            ],
        ),
        marks=pytest.mark.nash_enumpure_strategy,
        id="test_enumpure_agent_7",
    ),
    #############################################################
    # Examples where the are agent-form pure equillibrium behaviors that are not Nash eq
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "myerson_fig_4_2.efg"),
            solver=functools.partial(gbt.nash.enumpure_agent_solve),
            expected=[[[d(1, 0), d(0, 1)], [d(0, 1)]], [[d(0, 1), d(0, 1)], [d(1, 0)]]],
        ),
        marks=pytest.mark.nash_enumpure_strategy,
        id="test_enumpure_agent_8",
    ),
]


# LIAP_AGENT_CASES = [
# pytest.param(
# EquilibriumTestCase(
# factory=functools.partial(
# games.read_from_file, "two_player_perfect_info_win_lose.efg"
# ),
# solver=functools.partial(gbt.nash.liap_agent_solve),  # Need to pass the start arg
# expected=[[[[1, 0], ["1/3", "2/3"]], [["2/3", "1/3"]]]],
# ),
# marks=pytest.mark.nash_enumpure_strategy,
# id="test_liap_agent_1",
# ),
# ]

AGENT_CASES = []
AGENT_CASES += ENUMPURE_AGENT_CASES
# TO ADD: pygambit.nash.liap_agent_solve


@pytest.mark.nash
@pytest.mark.parametrize("test_case", AGENT_CASES, ids=lambda c: c.label)
def test_nash_agent_solver(test_case: EquilibriumTestCase, subtests) -> None:
    """Test calls of Nash solvers in EFGs using "agent" versions.

    Subtests:
    - Agent max regret no more than `test_case.regret_tol`
    - Agent max regret no more than max regret (+ `test_case.regret_tol`)
    - Equilibria are output in the expected order.  Equilibria are deemed to match if the maximum
      difference in probabilities is no more than `test_case.prob_tol`
    """
    game = test_case.factory()
    result = test_case.solver(game)
    with subtests.test("number of equilibria found"):
        assert len(result.equilibria) == len(test_case.expected)
    for i, (eq, exp) in enumerate(zip(result.equilibria, test_case.expected, strict=True)):
        with subtests.test(eq=i, check="agent_max_regret"):
            assert eq.agent_max_regret() <= test_case.regret_tol
        with subtests.test(eq=i, check="max_regret"):
            assert eq.agent_max_regret() <= eq.max_regret() + test_case.regret_tol
        with subtests.test(eq=i, check="strategy_profile"):
            expected = game.mixed_behavior_profile(rational=True, data=exp)
            for player in game.players:
                for action in player.actions:
                    assert abs(eq[action] - expected[action]) <= test_case.prob_tol


##################################################################################################
# TODO:
# The below all take a start argument that depends on the game, which doesn't immediately
# work with our current implementation of EquilibriumTestClass
##################################################################################################


def test_liap_agent():
    """Test calls of agent liap for mixed behavior equilibria."""

    game = games.read_from_file("stripped_down_poker.efg")
    result = gbt.nash.liap_agent_solve(game.mixed_behavior_profile())
    assert len(result.equilibria) == 1
    eq = result.equilibria[0]

    exp = [[[1, 0], ["1/3", "2/3"]], [["2/3", "1/3"]]]
    exp = game.mixed_behavior_profile(exp, rational=True)

    for player in game.players:
        for action in player.actions:
            assert abs(eq[action] - exp[action]) <= TOL_LARGE


def test_liap_strategy():
    """Test calls of liap for mixed strategy equilibria."""
    game = games.read_from_file("stripped_down_poker.efg")
    _ = gbt.nash.liap_solve(game.mixed_strategy_profile())

    # NashComputationResult(method='liap', rational=False, use_strategic=True, equilibria=[],
    # parameters={'start': [[0.25, 0.25, 0.25, 0.25], [0.5, 0.5]],
    # 'maxregret': 0.0001, 'maxiter': 1000})
    # Nothing found!


def test_simpdiv_strategy():
    """Test calls of simplicial subdivision for mixed strategy equilibria."""
    game = games.read_from_file("stripped_down_poker.efg")
    result = gbt.nash.simpdiv_solve(game.mixed_strategy_profile(rational=True))
    assert len(result.equilibria) == 1

    # [[[Rational(174763, 524288), Rational(349525, 524288), Rational(0, 1), Rational(0, 1)],
    # [Rational(699051, 1048576), Rational(349525, 1048576)]]]


##################################################################################################
# QRE solvers
##################################################################################################


LOGIT_BRANCH_CASES = [
    pytest.param(
        QREquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "const_sum_game.nfg"),
            solver=functools.partial(
                gbt.qre.logit_solve_branch, maxregret=0.2, first_step=0.2, max_accel=1
            ),
            expected=[{"idx": 0, "lam": 0, "profile": [d(0.5, 0.5), d(0.5, 0.5)]}],
            prob_tol=TOL_LARGE,
            lam_tol=TOL_LARGE,
        ),
        marks=pytest.mark.qre_logit,
        id="test_logit_branch_1",
    ),
]


LOGIT_LAMBDA_CASES = [
    pytest.param(
        QREquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "const_sum_game.nfg"),
            solver=functools.partial(
                gbt.qre.logit_solve_lambda, lam=[1, 2, 3], first_step=0.2, max_accel=1
            ),
            expected=[
                {"idx": 0, "lam": 1, "profile": [d(0.643, 0.357), d(0.5883, 0.41168)]},
                {"idx": 1, "lam": 2, "profile": [d(0.7727, 0.2273), d(0.6117, 0.3883)]},
                {"idx": 2, "lam": 3, "profile": [d(0.8595, 0.1405), d(0.6038, 0.39618)]},
            ],
            prob_tol=TOL_LARGE,
            lam_tol=TOL_LARGE,
        ),
        marks=pytest.mark.qre_logit,
        id="test_logit_lambda_1",
    ),
]


CASES = []
CASES += LOGIT_BRANCH_CASES
CASES += LOGIT_LAMBDA_CASES


@pytest.mark.nash
@pytest.mark.parametrize("test_case", CASES, ids=lambda c: c.label)
def test_qre_solver(test_case: QREquilibriumTestCase, subtests) -> None:
    """Test calls of QRE solvers.

    Subtests:
    - Expected value of lambda for given idx,
        difference in lambda not more than `test_case.lam_tol`
    - Expected profile for given idx and lambda,
        difference in probabilities is no more than `test_case.prob_tol`
    """
    game = test_case.factory()
    result = test_case.solver(game)

    for i, exp in enumerate(test_case.expected):
        found = result[exp["idx"]]
        with subtests.test(eq=i, check="lambda"):
            assert abs(exp["lam"] - found.lam) <= test_case.lam_tol
        with subtests.test(eq=i, check="strategy_profile"):
            exp_profile = game.mixed_strategy_profile(rational=True, data=exp["profile"])
            for player in game.players:
                for s in player.strategies:
                    assert abs(found.profile[s] - exp_profile[s]) <= test_case.prob_tol


##################################################################################################
# The remaining tests check for raising errors
##################################################################################################


def test_logit_solve_branch_error_with_invalid_maxregret():
    game = games.read_from_file("const_sum_game.nfg")
    with pytest.raises(ValueError, match="must be positive"):
        gbt.qre.logit_solve_branch(game=game, maxregret=0)
    with pytest.raises(ValueError, match="must be positive"):
        gbt.qre.logit_solve_branch(game=game, maxregret=-0.3)


def test_logit_solve_branch_error_with_invalid_first_step():
    game = games.read_from_file("const_sum_game.nfg")
    with pytest.raises(ValueError, match="must be positive"):
        gbt.qre.logit_solve_branch(game=game, first_step=0)
    with pytest.raises(ValueError, match="must be positive"):
        gbt.qre.logit_solve_branch(game=game, first_step=-0.3)


def test_logit_solve_branch_error_with_invalid_max_accel():
    game = games.read_from_file("const_sum_game.nfg")
    with pytest.raises(ValueError, match="at least 1.0"):
        gbt.qre.logit_solve_branch(game=game, max_accel=0)
    with pytest.raises(ValueError, match="at least 1.0"):
        gbt.qre.logit_solve_branch(game=game, max_accel=0.1)


def test_logit_solve_lambda_error_with_invalid_first_step():
    game = games.read_from_file("const_sum_game.nfg")
    with pytest.raises(ValueError, match="must be positive"):
        gbt.qre.logit_solve_lambda(game=game, lam=[1, 2, 3], first_step=0)
    with pytest.raises(ValueError, match="must be positive"):
        gbt.qre.logit_solve_lambda(game=game, lam=[1, 2, 3], first_step=-1)


def test_logit_solve_lambda_error_with_invalid_max_accel():
    game = games.read_from_file("const_sum_game.nfg")
    with pytest.raises(ValueError, match="at least 1.0"):
        gbt.qre.logit_solve_lambda(game=game, lam=[1, 2, 3], max_accel=0)
    with pytest.raises(ValueError, match="at least 1.0"):
        gbt.qre.logit_solve_lambda(game=game, lam=[1, 2, 3], max_accel=0.1)
