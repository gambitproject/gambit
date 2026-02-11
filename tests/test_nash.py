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
        marks=pytest.mark.nash_enumpure_strategy,
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

# def test_enummixed_double():
# """Test calls of enumeration of mixed strategy equilibria for 2-player games, floating-point.
# """
# game = games.read_from_file("stripped_down_poker.efg")
# result = gbt.nash.enummixed_solve(game, rational=False)
# assert len(result.equilibria) == 1
# # For floating-point results are not exact, so we skip testing exact values for now


CASES = []
CASES += ENUMPURE_CASES
CASES += ENUMMIXED_RATIONAL_CASES


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

# games.read_from_file("two_player_perfect_info_win_lose.efg"),
# [[[0, 1], [1, 0]], [[1, 0], [1, 0]]],

# games.read_from_file("two_player_perfect_info_win_lose_with_nonterm_outcomes.efg"),
# [[[0, 1], [1, 0]], [[1, 0], [1, 0]]],

# games.create_2x2_zero_sum_efg(missing_term_outcome=False),
# [[["1/2", "1/2"]], [["1/2", "1/2"]]],

# games.create_2x2_zero_sum_efg(missing_term_outcome=True),
# [[["1/2", "1/2"]], [["1/2", "1/2"]]],

# games.create_matching_pennies_efg(with_neutral_outcome=False),
# [[["1/2", "1/2"]], [["1/2", "1/2"]]],

# games.create_matching_pennies_efg(with_neutral_outcome=True),
# [[["1/2", "1/2"]], [["1/2", "1/2"]]],

# games.create_stripped_down_poker_efg(),
# [[[1, 0], ["1/3", "2/3"]], [["2/3", "1/3"]]],

# games.create_stripped_down_poker_efg(nonterm_outcomes=True),
# [[[1, 0], ["1/3", "2/3"]], [["2/3", "1/3"]]],

# games.create_kuhn_poker_efg(),
# [
    # [[1, 0], [1, 0], [1, 0], ["2/3", "1/3"], [1, 0], [0, 1]],
    # [[1, 0], ["2/3", "1/3"], [0, 1], [0, 1], ["2/3", "1/3"], [1, 0]],
# ],

# games.create_kuhn_poker_efg(nonterm_outcomes=True),
# [
    # [
        # [1, 0],
        # [1, 0],
        # [1, 0],
        # ["2/3", "1/3"],
        # [1, 0],
        # [0, 1],
    # ],
    # [[1, 0], ["2/3", "1/3"], [0, 1], [0, 1], ["2/3", "1/3"], [1, 0]],
# ],

# games.read_from_file("zerosum_efg_from_sequence_form_STOC94_paper.efg"),
# [
    # [[0, 1], ["2/3", "1/3"], ["1/3", "2/3"]],
    # [["5/6", "1/6"], ["5/9", "4/9"]],
# ],

# games.read_from_file("perfect_info_with_chance.efg"),
# [[[0, 1]], [[1, 0], [1, 0]]],

# games.read_from_file("2_player_chance.efg"),
# [
    # [["1/3", 0, "2/3"], ["2/3", 0, "1/3"]],
    # [["2/3", "1/3"], ["2/3", "1/3"], ["1/3", "2/3"]],
# ],

# games.read_from_file("2_player_chance_nonterm_outcomes_and_missing_term_outcomes.efg"),
# [
    # [["1/3", 0, "2/3"], ["2/3", 0, "1/3"]],
    # [["2/3", "1/3"], ["2/3", "1/3"], ["1/3", "2/3"]],
# ],

# games.read_from_file("large_payoff_game.efg"),
# [
    # [[1, 0], [1, 0]],
    # [[0, 1], ["9999999999999999999/10000000000000000000", "1/10000000000000000000"]],
# ],

# games.read_from_file("chance_in_middle.efg"),
# [[["3/11", "8/11"], [1, 0], [1, 0], [1, 0], [1, 0]], [[1, 0], ["6/11", "5/11"]]],

# games.read_from_file("chance_in_middle_with_nonterm_outcomes.efg"),
# [[["3/11", "8/11"], [1, 0], [1, 0], [1, 0], [1, 0]], [[1, 0], ["6/11", "5/11"]]],


LP_BEHAVIOR_RATIONAL_CASES = [
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "chance_in_middle_with_nonterm_outcomes.efg"
            ),
            solver=gbt.nash.lp_solve,
            expected=[
                [
                 [d("3/11", "8/11"), d(1, 0), d(1, 0), d(1, 0), d(1, 0)],
                 [d(1, 0), d("6/11", "5/11")]
                ]
            ],
        ),
        marks=pytest.mark.nash_enumpure_strategy,
        id="test1_TODO",
    ),
    ]

CASES = []
CASES += LP_BEHAVIOR_RATIONAL_CASES


@pytest.mark.nash
@pytest.mark.parametrize("test_case", CASES, ids=lambda c: c.label)
def test_nash_behavior_solver(test_case: EquilibriumTestCase, subtests) -> None:
    """Test calls of Nash solvers in mixed behaviors

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

# def test_lp_behavior_rational(game: gbt.Game, mixed_behav_prof_data: list):
    # """Test calls of LP for mixed behavior equilibria, rational precision,
    # using max_regret and agent_max_regret (internal consistency); and
    # comparison to a previously computed equilibrium using this function (regression test).
    # """
    # result = gbt.nash.lp_solve(game, use_strategic=False, rational=True)
    # assert len(result.equilibria) == 1
    # eq = result.equilibria[0]
    # assert eq.max_regret() == 0
    # assert eq.agent_max_regret() == 0
    # expected = game.mixed_behavior_profile(rational=True, data=mixed_behav_prof_data)
    # assert eq == expected

##################################################################################################
# AGENTS NASH SOLVERS (IN MIXED BEHAVIORS
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


ENUMPOLY_AGENT_CASES = [
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


AGENT_CASES = []
AGENT_CASES += ENUMPURE_AGENT_CASES
AGENT_CASES += ENUMPOLY_AGENT_CASES


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
# TEMP FOR ISSUE 660
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
def test_nash_agent_solver_no_subtests_only_profile(test_case: EquilibriumTestCase) -> None:
    """Test calls of Nash solvers in EFGs using "agent" versions.

    Checks for expected number of equilibria, and that the equilibria are output
    in the expected order.  Equilibria are deemed to match if the maximum
    difference in probabilities is no more than `test_case.prob_tol`
    """
    game = test_case.factory()
    result = test_case.solver(game)
    assert len(result.equilibria) == len(test_case.expected)
    for eq, exp in zip(result.equilibria, test_case.expected, strict=True):
        expected = game.mixed_behavior_profile(rational=True, data=exp)
        for player in game.players:
            for action in player.actions:
                assert abs(eq[action] - expected[action]) <= test_case.prob_tol


##################################################################################################
# AGENT UNORDERED
##################################################################################################

ENUMPOLY_AGENT_UNORDERED_CASES = [
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
@pytest.mark.parametrize("test_case", ENUMPOLY_AGENT_UNORDERED_CASES, ids=lambda c: c.label)
def test_nash_agent_solver_unordered(test_case: EquilibriumTestCase, subtests) -> None:
    """Test calls of Nash solvers in EFGs using "agent" versions.

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
            assert eq.agent_max_regret() <= test_case.regret_tol
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
# STILL TODO........
##################################################################################################


def test_lcp_strategy_double():
    """Test calls of LCP for mixed strategy equilibria, floating-point."""
    game = games.read_from_file("stripped_down_poker.efg")
    result = gbt.nash.lcp_solve(game, use_strategic=True, rational=False)
    assert len(result.equilibria) == 1
    # For floating-point results are not exact, so we skip testing exact values for now


@pytest.mark.nash
@pytest.mark.nash_lcp_strategy
@pytest.mark.parametrize(
    "game,mixed_strategy_prof_data,stop_after",
    [
        # Zero-sum games
        (games.create_2x2_zero_sum_efg(), [[["1/2", "1/2"], ["1/2", "1/2"]]], None),
        (
            games.create_2x2_zero_sum_efg(missing_term_outcome=True),
            [[["1/2", "1/2"], ["1/2", "1/2"]]],
            None,
        ),
        (games.create_stripped_down_poker_efg(), [[["1/3", "2/3", 0, 0], ["2/3", "1/3"]]], None),
        (
            games.create_stripped_down_poker_efg(nonterm_outcomes=True),
            [[["1/3", "2/3", 0, 0], ["2/3", "1/3"]]],
            None,
        ),
        (games.create_kuhn_poker_efg(), [games.kuhn_poker_lcp_first_mixed_strategy_prof()], 1),
        (
            games.create_kuhn_poker_efg(nonterm_outcomes=True),
            [games.kuhn_poker_lcp_first_mixed_strategy_prof()],
            1,
        ),
        # Non-zero-sum games
        (games.create_one_shot_trust_efg(), [[[0, 1], ["1/2", "1/2"]]], None),
        (
            games.create_EFG_for_nxn_bimatrix_coordination_game(3),
            [
                [[1, 0, 0], [1, 0, 0]],
                [["1/2", "1/2", 0], ["1/2", "1/2", 0]],
                [[0, 1, 0], [0, 1, 0]],
                [[0, "1/2", "1/2"], [0, "1/2", "1/2"]],
                [["1/3", "1/3", "1/3"], ["1/3", "1/3", "1/3"]],
                [["1/2", 0, "1/2"], ["1/2", 0, "1/2"]],
                [[0, 0, 1], [0, 0, 1]],
            ],
            None,
        ),
        (
            games.create_EFG_for_nxn_bimatrix_coordination_game(4),
            [[[1, 0, 0, 0], [1, 0, 0, 0]]],
            1,
        ),
        (
            games.create_EFG_for_6x6_bimatrix_with_long_LH_paths_and_unique_eq(),
            [
                [
                    ["1/30", "1/6", "3/10", "3/10", "1/6", "1/30"],
                    ["1/6", "1/30", "3/10", "3/10", "1/30", "1/6"],
                ],
            ],
            None,
        ),
    ],
)
def test_lcp_strategy_rational(
    game: gbt.Game, mixed_strategy_prof_data: list, stop_after: None | int
):
    """Test calls of LCP for mixed strategy equilibria, rational precision
    using max_regret (internal consistency); and comparison to a sequence of previously
    computed equilibria using this function (regression test).

    This sequence will correspond to the full set of all computed equilibria if stop_after
    is None, else the first stop_after-many equilibria.
    """
    result = gbt.nash.lcp_solve(game, use_strategic=True, rational=True, stop_after=stop_after)

    if stop_after:
        result = gbt.nash.lcp_solve(game, use_strategic=True, stop_after=stop_after)
        assert len(result.equilibria) == stop_after
    else:
        # compute all
        result = gbt.nash.lcp_solve(game, use_strategic=True)
    assert len(result.equilibria) == len(mixed_strategy_prof_data)
    for eq, exp in zip(result.equilibria, mixed_strategy_prof_data, strict=True):
        assert eq.max_regret() == 0
        expected = game.mixed_strategy_profile(rational=True, data=exp)
        assert eq == expected


def test_lcp_behavior_double():
    """Test calls of LCP for mixed behavior equilibria, floating-point."""
    game = games.read_from_file("stripped_down_poker.efg")
    result = gbt.nash.lcp_solve(game, use_strategic=False, rational=False)
    assert len(result.equilibria) == 1
    # For floating-point results are not exact, so we skip testing exact values for now


@pytest.mark.nash
@pytest.mark.nash_lcp_behavior
@pytest.mark.parametrize(
    "game,mixed_behav_prof_data",
    [
        # Zero-sum games (also tested with lp solve)
        (games.create_2x2_zero_sum_efg(), [[["1/2", "1/2"]], [["1/2", "1/2"]]]),
        (
            games.create_2x2_zero_sum_efg(missing_term_outcome=True),
            [[["1/2", "1/2"]], [["1/2", "1/2"]]],
        ),
        (games.create_matching_pennies_efg(), [[["1/2", "1/2"]], [["1/2", "1/2"]]]),
        (
            games.create_matching_pennies_efg(with_neutral_outcome=True),
            [[["1/2", "1/2"]], [["1/2", "1/2"]]],
        ),
        (games.create_stripped_down_poker_efg(), [[[1, 0], ["1/3", "2/3"]], [["2/3", "1/3"]]]),
        (
            games.create_stripped_down_poker_efg(nonterm_outcomes=True),
            [[[1, 0], ["1/3", "2/3"]], [["2/3", "1/3"]]],
        ),
        (
            games.create_kuhn_poker_efg(),
            [
                [
                    ["2/3", "1/3"],
                    [1, 0],
                    [1, 0],
                    ["1/3", "2/3"],
                    [0, 1],
                    ["1/2", "1/2"],
                ],
                [[1, 0], ["2/3", "1/3"], [0, 1], [0, 1], ["2/3", "1/3"], [1, 0]],
            ],
        ),
        (
            games.create_kuhn_poker_efg(nonterm_outcomes=True),
            [
                [
                    ["2/3", "1/3"],
                    [1, 0],
                    [1, 0],
                    ["1/3", "2/3"],
                    [0, 1],
                    ["1/2", "1/2"],
                ],
                [[1, 0], ["2/3", "1/3"], [0, 1], [0, 1], ["2/3", "1/3"], [1, 0]],
            ],
        ),
        # In the next test case:
        # 1/2-1/2 for l/r is determined by MixedBehaviorProfile.UndefinedToCentroid()
        (
            games.read_from_file("perfect_info_with_chance.efg"),
            [[[0, 1]], [[0, 1], [0, 1]]],
        ),
        (
            games.read_from_file("two_player_perfect_info_win_lose.efg"),
            [[[0, 1], [1, 0]], [[0, 1], ["1/2", "1/2"]]],
        ),
        (
            games.read_from_file("two_player_perfect_info_win_lose_with_nonterm_outcomes.efg"),
            [[[0, 1], [1, 0]], [[0, 1], ["1/2", "1/2"]]],
        ),
        (
            games.read_from_file("2_player_chance.efg"),
            [
                [["1/3", 0, "2/3"], ["2/3", 0, "1/3"]],
                [["2/3", "1/3"], ["1/3", "2/3"], ["1/3", "2/3"]],
            ],
        ),
        (
            games.read_from_file("2_player_chance_nonterm_outcomes_and_missing_term_outcomes.efg"),
            [
                [["1/3", 0, "2/3"], ["2/3", 0, "1/3"]],
                [["2/3", "1/3"], ["1/3", "2/3"], ["1/3", "2/3"]],
            ],
        ),
        (
            games.read_from_file("large_payoff_game.efg"),
            [
                [[1, 0], [1, 0]],
                [[0, 1], ["9999999999999999999/10000000000000000000", "1/10000000000000000000"]],
            ],
        ),
        (
            games.read_from_file("chance_in_middle.efg"),
            [[["3/11", "8/11"], [1, 0], [1, 0], [1, 0], [1, 0]], [[1, 0], ["6/11", "5/11"]]],
        ),
        (
            games.read_from_file("chance_in_middle_with_nonterm_outcomes.efg"),
            [[["3/11", "8/11"], [1, 0], [1, 0], [1, 0], [1, 0]], [[1, 0], ["6/11", "5/11"]]],
        ),
        # Non-zero-sum games
        (
            games.read_from_file("reduction_both_players_payoff_ties_GTE_survey.efg"),
            [[[0, 0, 1, 0], [1, 0]], [[0, 1], [0, 1], [0, 1], [0, 1]]],
        ),
        (
            games.create_EFG_for_6x6_bimatrix_with_long_LH_paths_and_unique_eq(),
            [
                [["1/30", "1/6", "3/10", "3/10", "1/6", "1/30"]],
                [["1/6", "1/30", "3/10", "3/10", "1/30", "1/6"]],
            ],
        ),
        (games.create_EFG_for_nxn_bimatrix_coordination_game(3), [[[0, 0, 1]], [[0, 0, 1]]]),
        (
            games.create_EFG_for_nxn_bimatrix_coordination_game(4),
            [[[0, 0, 0, 1]], [[0, 0, 0, 1]]],
        ),
        (
            games.read_from_file("entry_accommodation.efg"),
            [[["2/3", "1/3"], [1, 0], [1, 0]], [["2/3", "1/3"]]],
        ),
        (
            games.read_from_file("entry_accommodation_with_nonterm_outcomes.efg"),
            [[["2/3", "1/3"], [1, 0], [1, 0]], [["2/3", "1/3"]]],
        ),
        (games.read_from_file("2_player_non_zero_sum.efg"), [[["1/3", "2/3"]], [["1/2", "1/2"]]]),
        (
            games.read_from_file("2_player_non_zero_sum_missing_term_outcome.efg"),
            [[["1/3", "2/3"]], [["1/2", "1/2"]]],
        ),
    ],
)
def test_lcp_behavior_rational(game: gbt.Game, mixed_behav_prof_data: list):
    """Test calls of LCP for mixed behavior equilibria, rational precision.

    using max_regret and agent_max_regret (internal consistency); and
    comparison to a previously computed equilibrium using this function (regression test).
    """
    result = gbt.nash.lcp_solve(game, use_strategic=False, rational=True)
    assert len(result.equilibria) == 1
    eq = result.equilibria[0]
    assert eq.max_regret() == 0
    assert eq.agent_max_regret() == 0
    expected = game.mixed_behavior_profile(rational=True, data=mixed_behav_prof_data)
    assert eq == expected


def test_lp_strategy_double():
    """Test calls of LP for mixed strategy equilibria, floating-point."""
    game = games.read_from_file("stripped_down_poker.efg")
    result = gbt.nash.lp_solve(game, use_strategic=True, rational=False)
    assert len(result.equilibria) == 1
    # For floating-point results are not exact, so we skip testing exact values for now


@pytest.mark.nash
@pytest.mark.nash_lp_strategy
@pytest.mark.parametrize(
    "game,mixed_strategy_prof_data",
    [
        (
            games.create_2x2_zero_sum_efg(),
            [["1/2", "1/2"], ["1/2", "1/2"]],
        ),
        (
            games.create_2x2_zero_sum_efg(missing_term_outcome=True),
            [["1/2", "1/2"], ["1/2", "1/2"]],
        ),
        (games.create_stripped_down_poker_efg(), [["1/3", "2/3", 0, 0], ["2/3", "1/3"]]),
        (
            games.create_stripped_down_poker_efg(nonterm_outcomes=True),
            [["1/3", "2/3", 0, 0], ["2/3", "1/3"]],
        ),
        (games.create_kuhn_poker_efg(), games.kuhn_poker_lp_mixed_strategy_prof()),
        (
            games.create_kuhn_poker_efg(nonterm_outcomes=True),
            games.kuhn_poker_lp_mixed_strategy_prof(),
        ),
    ],
)
def test_lp_strategy_rational(game: gbt.Game, mixed_strategy_prof_data: list):
    """Test calls of LP for mixed strategy equilibria, rational precision."""
    result = gbt.nash.lp_solve(game, use_strategic=True, rational=True)
    assert len(result.equilibria) == 1
    eq = result.equilibria[0]
    assert eq.max_regret() == 0
    expected = game.mixed_strategy_profile(rational=True, data=mixed_strategy_prof_data)
    assert eq == expected


def test_lp_behavior_double():
    """Test calls of LP for mixed behavior equilibria, floating-point."""
    game = games.read_from_file("stripped_down_poker.efg")
    result = gbt.nash.lp_solve(game, use_strategic=False, rational=False)
    assert len(result.equilibria) == 1
    # For floating-point results are not exact, so we skip testing exact values for now


def test_liap_strategy():
    """Test calls of liap for mixed strategy equilibria."""
    game = games.read_from_file("stripped_down_poker.efg")
    _ = gbt.nash.liap_solve(game.mixed_strategy_profile())


def test_liap_agent():
    """Test calls of agent liap for mixed behavior equilibria."""
    game = games.read_from_file("stripped_down_poker.efg")
    _ = gbt.nash.liap_agent_solve(game.mixed_behavior_profile())


def test_simpdiv_strategy():
    """Test calls of simplicial subdivision for mixed strategy equilibria."""
    game = games.read_from_file("stripped_down_poker.efg")
    result = gbt.nash.simpdiv_solve(game.mixed_strategy_profile(rational=True))
    assert len(result.equilibria) == 1


def test_ipa_strategy():
    """Test calls of IPA for mixed strategy equilibria."""
    game = games.read_from_file("stripped_down_poker.efg")
    result = gbt.nash.ipa_solve(game)
    assert len(result.equilibria) == 1


def test_gnm_strategy():
    """Test calls of GNM for mixed strategy equilibria."""
    game = games.read_from_file("stripped_down_poker.efg")
    result = gbt.nash.gnm_solve(game)
    assert len(result.equilibria) == 1


def test_logit_strategy():
    """Test calls of logit for mixed strategy equilibria."""
    game = games.read_from_file("stripped_down_poker.efg")
    result = gbt.nash.logit_solve(game, use_strategic=True)
    assert len(result.equilibria) == 1


def test_logit_behavior():
    """Test calls of logit for behavior equilibria."""
    game = games.read_from_file("stripped_down_poker.efg")
    result = gbt.nash.logit_solve(game, use_strategic=False)
    assert len(result.equilibria) == 1


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


def test_logit_solve_branch():
    game = games.read_from_file("const_sum_game.nfg")
    assert (
        len(gbt.qre.logit_solve_branch(game=game, maxregret=0.2, first_step=0.2, max_accel=1)) > 0
    )


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


def test_logit_solve_lambda():
    game = games.read_from_file("const_sum_game.nfg")
    assert (
        len(gbt.qre.logit_solve_lambda(game=game, lam=[1, 2, 3], first_step=0.2, max_accel=1)) > 0
    )
