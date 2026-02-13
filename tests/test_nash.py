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
    solver: typing.Callable[[gbt.Game], gbt.nash.NashComputationResult]
    expected: list
    regret_tol: float | gbt.Rational = Q(0)
    prob_tol: float | gbt.Rational = Q(0)


##################################################################################################
# NASH SOLVER IN MIXED STRATEGIES
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
        id="test1_TODO",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=games.create_stripped_down_poker_efg,
            solver=functools.partial(gbt.nash.enumpure_solve),
            expected=[],
        ),
        marks=pytest.mark.nash_enumpure_strategy,
        id="test2_TODO",
    ),
    # Non-zero-sum 2-player games
    pytest.param(
        EquilibriumTestCase(
            factory=games.create_one_shot_trust_efg,
            solver=functools.partial(gbt.nash.enumpure_solve),
            expected=[[d(0, 1), d(0, 1)]],
        ),
        marks=pytest.mark.nash_enumpure_strategy,
        id="test3",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_one_shot_trust_efg, unique_NE_variant=True),
            solver=functools.partial(gbt.nash.enumpure_solve),
            expected=[[d(1, 0), d(0, 1)]],
        ),
        marks=pytest.mark.nash_enumpure_strategy,
        id="test3b",
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
        id="test4",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=games.create_EFG_for_6x6_bimatrix_with_long_LH_paths_and_unique_eq,
            solver=functools.partial(gbt.nash.enumpure_solve),
            expected=[],
        ),
        marks=pytest.mark.nash_enumpure_strategy,
        id="test4",
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
        id="test5",
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
        id="test6",
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
        id="test1_TODO",
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
        id="test1",
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
        id="test2",
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
        id="test3",
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
        id="test4",
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
        id="test1_TODO",
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
        id="test1_TODO",
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
        id="test1_TODO",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_stripped_down_poker_efg, nonterm_outcomes=True),
            solver=functools.partial(gbt.nash.lp_solve, rational=True, use_strategic=True),
            expected=[[d("1/3", "2/3", 0, 0), d("2/3", "1/3")]],
        ),
        marks=pytest.mark.nash_lp_strategy,
        id="test1_TODO",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_kuhn_poker_efg, nonterm_outcomes=False),
            solver=functools.partial(gbt.nash.lp_solve, rational=True, use_strategic=True),
            expected=[games.kuhn_poker_lp_mixed_strategy_prof()],
        ),
        marks=pytest.mark.nash_lp_strategy,
        id="test1_TODO",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_kuhn_poker_efg, nonterm_outcomes=True),
            solver=functools.partial(gbt.nash.lp_solve, rational=True, use_strategic=True),
            expected=[games.kuhn_poker_lp_mixed_strategy_prof()],
        ),
        marks=pytest.mark.nash_lp_strategy,
        id="test1_TODO",
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
        id="test_lcp_strategy_rational_1",
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
        id="test_lcp_strategy_double_1",
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
        id="test1_TODO",
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
        id="test1_TODO",
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
        id="test1_TODO",
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
# CASES += LOGIT_STRATEGY_CASES
# CASES += IPA_STRATEGY_CASES
# CASES += GNM_STRATEGY_CASES


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


def test_lp_strategy_double():
    """Test calls of LP for mixed strategy equilibria, floating-point."""
    game = games.read_from_file("stripped_down_poker.efg")
    result = gbt.nash.lp_solve(game, use_strategic=True, rational=False)
    assert len(result.equilibria) == 1
    # For floating-point results are not exact, so we skip testing exact values for now


def test_lcp_strategy_double():
    """Test calls of LCP for mixed strategy equilibria, floating-point."""
    game = games.read_from_file("stripped_down_poker.efg")
    result = gbt.nash.lcp_solve(game, use_strategic=True, rational=False)
    assert len(result.equilibria) == 1
    # For floating-point results are not exact, so we skip testing exact values for now


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
        id="test1_TODO",
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
        id="test1_TODO",
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
        id="test1_TODO",
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
        id="test1_TODO",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_2x2_zero_sum_efg, variant=None),
            solver=gbt.nash.lp_solve,
            expected=[[[d("1/2", "1/2")], [d("1/2", "1/2")]]],
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test1_TODO",
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
        id="test1_TODO",
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
        id="test1_TODO",
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
        id="test1_TODO",
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
        id="test1_TODO",
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
        id="test1_TODO",
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
        id="test1_TODO",
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
        id="test1_TODO",
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
        id="test1_TODO",
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
        id="test1_TODO",
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
        id="test1_TODO",
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
        id="test1_TODO",
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
        id="test1_TODO",
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
        id="test1_TODO",
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
        id="test1_TODO",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_stripped_down_poker_efg, nonterm_outcomes=True),
            solver=gbt.nash.lcp_solve,
            expected=[[[d(1, 0), d("1/3", "2/3")], [d("2/3", "1/3")]]],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test1_TODO",
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
        id="test1_TODO",
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
        id="test1_TODO",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "perfect_info_with_chance.efg"),
            solver=gbt.nash.lcp_solve,
            expected=[[[d(0, 1)], [d(0, 1), d(0, 1)]]],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test1_TODO",
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
        id="test1_TODO",
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
        id="test1_TODO",
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
        id="test1_TODO",
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
        id="test1_TODO",
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
        id="test1_TODO",
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
        id="test1_TODO",
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
        id="test1_TODO",
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
        id="test1_TODO",
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
        id="test1_TODO",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_EFG_for_nxn_bimatrix_coordination_game, 3),
            solver=gbt.nash.lcp_solve,
            expected=[[[d(0, 0, 1)], [d(0, 0, 1)]]],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test1_TODO",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_EFG_for_nxn_bimatrix_coordination_game, 4),
            solver=gbt.nash.lcp_solve,
            expected=[[[d(0, 0, 0, 1)], [d(0, 0, 0, 1)]]],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test1_TODO",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "entry_accommodation.efg"),
            solver=gbt.nash.lcp_solve,
            expected=[[[d("2/3", "1/3"), d(1, 0), d(1, 0)], [d("2/3", "1/3")]]],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test1_TODO",
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
        id="test1_TODO",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "2_player_non_zero_sum.efg"),
            solver=gbt.nash.lcp_solve,
            expected=[[[d("1/3", "2/3")], [d("1/2", "1/2")]]],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test1_TODO",
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
        id="test1_TODO",
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
        id="test1_TODO",
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
        id="test8_TODO",
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
        id="test9_TODO",
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
        id="test9_TODO",
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
        id="test9_TODO",
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
        id="test4_TODO",
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
        id="test6_TODO",
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
        id="test7_TODO",
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
        id="test5_TODO",
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

CASES = []
CASES += LP_BEHAVIOR_RATIONAL_CASES
CASES += LCP_BEHAVIOR_RATIONAL_CASES
CASES += ENUMPOLY_BEHAVIOR_CASES


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


############################################################
# CREATE AUTO VARIANTS OF THE RATIONAL TESTS FOR DOUBLES?
############################################################


def test_lp_behavior_double():
    """Test calls of LP for mixed behavior equilibria, floating-point."""
    game = games.read_from_file("stripped_down_poker.efg")
    result = gbt.nash.lp_solve(game, use_strategic=False, rational=False)
    assert len(result.equilibria) == 1
    # For floating-point results are not exact, so we skip testing exact values for now


def test_lcp_behavior_double():
    """Test calls of LCP for mixed behavior equilibria, floating-point."""
    game = games.read_from_file("stripped_down_poker.efg")
    result = gbt.nash.lcp_solve(game, use_strategic=False, rational=False)
    assert len(result.equilibria) == 1
    # For floating-point results are not exact, so we skip testing exact values for now


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
        id="test5_TODO",
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
        id="test1_TODO",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=games.create_stripped_down_poker_efg,
            solver=functools.partial(gbt.nash.enumpure_agent_solve),
            expected=[],
        ),
        marks=pytest.mark.nash_enumpure_strategy,
        id="test2_TODO",
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
        id="test3_TODO",
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
        id="test3b",
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
        id="test4",
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
        id="test4",
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
        id="test5",
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
        id="test6",
    ),
]

LIAP_AGENT_CASES = [
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "two_player_perfect_info_win_lose.efg"
            ),
            solver=functools.partial(gbt.nash.liap_agent_solve),  # Need to pass the start arg
            expected=[[[[1, 0], ["1/3", "2/3"]], [["2/3", "1/3"]]]],
        ),
        marks=pytest.mark.nash_enumpure_strategy,
        id="test1_TODO",
    ),
]

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
##################################################################################################


def test_logit_behavior():
    """Test calls of logit for behavior equilibria."""
    game = games.read_from_file("stripped_down_poker.efg")
    result = gbt.nash.logit_solve(game, use_strategic=False)
    assert len(result.equilibria) == 1

    # [[[[1.0, 0.0], [0.33333338649882943, 0.6666666135011706]],
    # [[0.6666667065407631, 0.3333332934592369]]]]


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


# Needs a new solver tester
def test_logit_solve_lambda():
    game = games.read_from_file("const_sum_game.nfg")
    assert (
        len(gbt.qre.logit_solve_lambda(game=game, lam=[1, 2, 3], first_step=0.2, max_accel=1)) > 0
    )
    # [LogitQREMixedStrategyProfile(lam=1.000000,profile=[[0.6429793593274791, 0.3570206406725209],
    # [0.588319024552166, 0.41168097544783405]]),
    # LogitQREMixedStrategyProfile(lam=2.000000,profile=[[0.7726766071376159, 0.2273233928623842],
    # [0.6117434791999494, 0.38825652080005063]]),
    # LogitQREMixedStrategyProfile(lam=3.000000,profile=[[0.859536709259968, 0.14046329074003203],
    # [0.6038157860344706, 0.39618421396552944]])]


def test_logit_solve_branch():
    game = games.read_from_file("const_sum_game.nfg")
    assert (
        len(gbt.qre.logit_solve_branch(game=game, maxregret=0.2, first_step=0.2, max_accel=1)) > 0
    )

    # [LogitQREMixedStrategyProfile(lam=0.000000,profile=[[0.5, 0.5], [0.5, 0.5]])]


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
