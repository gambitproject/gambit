"""Test of calls to Nash equilibrium solvers.

For many solvers the tests simply ensure that calling the solver works, and then for some cases,
checking the expected results on a very simple game.

There is better test coverage for
lp_solve, lcp_solve, and enumpoly_solve, all in mixed behaviors.
"""

import pytest

import pygambit as gbt

from . import games

TOL = 1e-13  # tolerance for floating point assertions


def test_enumpure_strategy():
    """Test calls of enumeration of pure strategies."""
    game = games.read_from_file("stripped_down_poker.efg")
    assert len(gbt.nash.enumpure_solve(game).equilibria) == 0


def test_enumpure_agent():
    """Test calls of enumeration of pure agent strategies."""
    game = games.read_from_file("stripped_down_poker.efg")
    assert len(gbt.nash.enumpure_agent_solve(game).equilibria) == 0


def test_enummixed_double():
    """Test calls of enumeration of mixed strategy equilibria for 2-player games, floating-point.
    """
    game = games.read_from_file("stripped_down_poker.efg")
    result = gbt.nash.enummixed_solve(game, rational=False)
    assert len(result.equilibria) == 1
    # For floating-point results are not exact, so we skip testing exact values for now


@pytest.mark.nash
@pytest.mark.nash_enummixed_strategy
@pytest.mark.parametrize(
    "game,mixed_strategy_prof_data",
    [
        # Zero-sum games
        (games.create_stripped_down_poker_efg(), [[["1/3", "2/3", 0, 0], ["2/3", "1/3"]]]),
        # Non-zero-sum games
        (games.create_one_shot_trust_efg(), [[[0, 1], ["1/2", "1/2"]],
                                             [[0, 1], [0, 1]]]),
        (
                games.create_EFG_for_nxn_bimatrix_coordination_game(3),
                [
                    [[1, 0, 0], [1, 0, 0]],
                    [["1/2", "1/2", 0], ["1/2", "1/2", 0]],
                    [["1/3", "1/3", "1/3"], ["1/3", "1/3", "1/3"]],
                    [["1/2", 0, "1/2"], ["1/2", 0, "1/2"]],
                    [[0, 1, 0], [0, 1, 0]],
                    [[0, "1/2", "1/2"], [0, "1/2", "1/2"]],
                    [[0, 0, 1], [0, 0, 1]],
                ],
        ),
        (
                games.create_EFG_for_6x6_bimatrix_with_long_LH_paths_and_unique_eq(),
                [
                    [["1/30", "1/6", "3/10", "3/10", "1/6", "1/30"],
                     ["1/6", "1/30", "3/10", "3/10", "1/30", "1/6"]],
                ],
        ),
    ]
)
def test_enummixed_rational(game: gbt.Game, mixed_strategy_prof_data: list):
    """Test calls of enumeration of extreme mixed strategy equilibria, rational precision

       Tests max regret being zero (internal consistency) and compares the computed sequence of
       extreme equilibria to a previosuly computed sequence (regression test)
    """
    result = gbt.nash.enummixed_solve(game, rational=True)
    assert len(result.equilibria) == len(mixed_strategy_prof_data)
    for eq, exp in zip(result.equilibria, mixed_strategy_prof_data, strict=True):
        assert eq.max_regret() == 0
        expected = game.mixed_strategy_profile(rational=True, data=exp)
        assert eq == expected


@pytest.mark.nash
@pytest.mark.nash_enumpoly_behavior
@pytest.mark.parametrize(
    "game,mixed_behav_prof_data,stop_after",
    [
        # 2-player zero-sum games
        (
                games.create_stripped_down_poker_efg(),
                [[[[1, 0], ["1/3", "2/3"]], [["2/3", "1/3"]]]],
                None,
        ),
        # 2-player non-zero-sum games
        pytest.param(
            games.create_one_shot_trust_efg(),
            [[[[0, 1]], [["1/2", "1/2"]]], [[[0, 1]], [[0, 1]]]],
            # second entry assumes we extend to Nash using only pure behaviors
            # currently we get [[0, 1]], [[0, 0]]] as a second eq
            None,
            marks=pytest.mark.xfail(reason="Problem with enumpoly, as per issue #660")
        ),
        pytest.param(
            games.create_one_shot_trust_efg(unique_NE_variant=True),
            [[[[1, 0]], [[0, 1]]]],  # currently we get [[0, 1]], [[0, 0]]] as a second eq
            None,
            marks=pytest.mark.xfail(reason="Problem with enumpoly, as per issue #660")
        ),
        (
                games.create_EFG_for_nxn_bimatrix_coordination_game(3),
                [
                    [[["1/3", "1/3", "1/3"]], [["1/3", "1/3", "1/3"]]],
                    [[["1/2", "1/2", 0]], [["1/2", "1/2", 0]]],
                    [[["1/2", 0, "1/2"]], [["1/2", 0, "1/2"]]],
                    [[[1, 0, 0]], [[1, 0, 0]]],
                    [[[0, "1/2", "1/2"]], [[0, "1/2", "1/2"]]],
                    [[[0, 1, 0]], [[0, 1, 0]]],
                    [[[0, 0, 1]], [[0, 0, 1]]],
                ],
                None,
        ),
        (
                games.create_EFG_for_nxn_bimatrix_coordination_game(4),
                [[[["1/4", "1/4", "1/4", "1/4"]], [["1/4", "1/4", "1/4", "1/4"]]]],
                1,
        ),
        # 3-player game
        # (
        # games.create_mixed_behav_game_efg(),
        # [
        # [[["1/2", "1/2"]], [["2/5", "3/5"]], [["1/4", "3/4"]]],
        # [[["2/5", "3/5"]], [["1/2", "1/2"]], [["1/3", "2/3"]]],
        # ],
        # 2,  # 9 in total found by enumpoly (see unordered test)
        # ),
        ##############################################################################
        ##############################################################################
        (
            games.create_3_player_with_internal_outcomes_efg(),
            [
                [[[1, 0], [1, 0]], [[1, 0], ["1/2", "1/2"]], [[1, 0], [0, 1]]],
                [[[1, 0], [1, 0]], [[1, 0], [0, 1]],
                    [[1, 0], ["1/3", "2/3"]]]],
            2,
        ),
        (
            games.create_3_player_with_internal_outcomes_efg(nonterm_outcomes=True),
            [
                [[[1, 0], [1, 0]], [[1, 0], ["1/2", "1/2"]], [[1, 0], [0, 1]]],
                [[[1, 0], [1, 0]], [[1, 0], [0, 1]],
                 [[1, 0], ["1/3", "2/3"]]]],
            2,
        ),
        ##############################################################################
        ##############################################################################
        (
            games.create_non_zero_sum_lacking_outcome_efg(),
            [[[["1/3", "2/3"]], [["1/2", "1/2"]]]],
            1,
        ),
        (
            games.create_non_zero_sum_lacking_outcome_efg(missing_term_outcome=True),
            [[[["1/3", "2/3"]], [["1/2", "1/2"]]]],
            1,
        ),
        ##############################################################################
        ##############################################################################
        (
                games.create_chance_in_middle_efg(),
                [[[["3/11", "8/11"], [1, 0], [1, 0], [1, 0], [1, 0]], [[1, 0], ["6/11", "5/11"]]],
                 ],  # [[[1, 0], [1, 0], [1, 0], [0, 0], [0, 0]], [[0, 1], [1, 0]]],
                     # [[[0, 1], [0, 0], [0, 0], [1, 0], [1, 0]], [[1, 0], [0, 1]]],
                1,  # subsequent eqs have undefined infosets; include after #issue 660
        ),
        (
                games.create_chance_in_middle_efg(nonterm_outcomes=True),
                [[[["3/11", "8/11"], [1, 0], [1, 0], [1, 0], [1, 0]], [[1, 0], ["6/11", "5/11"]]],
                 ],  # [[[1, 0], [1, 0], [1, 0], [0, 0], [0, 0]], [[0, 1], [1, 0]]],
                     # [[[0, 1], [0, 0], [0, 0], [1, 0], [1, 0]], [[1, 0], [0, 1]]],
                1,
        ),
    ],
)
def test_enumpoly_ordered_behavior(
        game: gbt.Game, mixed_behav_prof_data: list, stop_after: None | int
):
    """Test calls of enumpoly for mixed behavior equilibria,
    using max_regret and agent_max_regret (internal consistency); and
    comparison to a set of previously computed equilibria with this function (regression test).
    This set will be the full set of all computed equilibria if stop_after is None,
    else the first stop_after-many equilibria.

    This is the "ordered" version where we test for the outputs coming in a specific
    order; there is also an "unordered" version.  The game 2x2x2.nfg, for example,
    has a point at which the Jacobian is singular.  As a result, the order in which it
    returns the two totally-mixed equilbria is system-dependent due, essentially,
    to inherent numerical instability near that point.
    """
    if stop_after:
        result = gbt.nash.enumpoly_solve(
            game, use_strategic=False, stop_after=stop_after, maxregret=0.00001
        )
        assert len(result.equilibria) == stop_after
    else:
        # compute all
        result = gbt.nash.enumpoly_solve(game, use_strategic=False)
    assert len(result.equilibria) == len(mixed_behav_prof_data)
    for eq, exp in zip(result.equilibria, mixed_behav_prof_data, strict=True):
        assert abs(eq.max_regret()) <= TOL
        assert abs(eq.agent_max_regret()) <= TOL
        expected = game.mixed_behavior_profile(rational=True, data=exp)
        for p in game.players:
            for i in p.infosets:
                for a in i.actions:
                    assert abs(eq[p][i][a] - expected[p][i][a]) <= TOL


@pytest.mark.nash
@pytest.mark.nash_enumpoly_behavior
@pytest.mark.parametrize(
    "game,mixed_behav_prof_data,stop_after",
    [
        ##############################################################################
        ##############################################################################
        (
            games.create_3_player_with_internal_outcomes_efg(),
            [
                [[[1, 0], [1, 0]], [[1, 0], ["1/2", "1/2"]], [[1, 0], [0, 1]]],
                [[[1, 0], [1, 0]], [[1, 0], [0, 1]], [[1, 0], ["1/3", "2/3"]]],
            ],
            2,
        ),
        (
            games.create_3_player_with_internal_outcomes_efg(nonterm_outcomes=True),
            [
                [[[1, 0], [1, 0]], [[1, 0], ["1/2", "1/2"]], [[1, 0], [0, 1]]],
                [[[1, 0], [1, 0]], [[1, 0], [0, 1]], [[1, 0], ["1/3", "2/3"]]]],
            2,
        ),
        ##############################################################################
        ##############################################################################
    ],
)
def test_enumpoly_ordered_behavior_PROBLEM_CASE(
        game: gbt.Game, mixed_behav_prof_data: list, stop_after: None | int
):
    """Test calls of enumpoly for mixed behavior equilibria,
    using max_regret and agent_max_regret (internal consistency); and
    comparison to a set of previously computed equilibria with this function (regression test).
    This set will be the full set of all computed equilibria if stop_after is None,
    else the first stop_after-many equilibria.

    This is the "ordered" version where we test for the outputs coming in a specific
    order; there is also an "unordered" version.  The game 2x2x2.nfg, for example,
    has a point at which the Jacobian is singular.  As a result, the order in which it
    returns the two totally-mixed equilbria is system-dependent due, essentially,
    to inherent numerical instability near that point.
    """
    if stop_after:
        result = gbt.nash.enumpoly_solve(
            game, use_strategic=False, stop_after=stop_after, maxregret=0.00001
        )
        assert len(result.equilibria) == stop_after
    else:
        # compute all
        result = gbt.nash.enumpoly_solve(game, use_strategic=False)
    assert len(result.equilibria) == len(mixed_behav_prof_data)
    for eq, exp in zip(result.equilibria, mixed_behav_prof_data, strict=True):
        assert abs(eq.max_regret()) <= TOL
        assert abs(eq.agent_max_regret()) <= TOL
        expected = game.mixed_behavior_profile(rational=True, data=exp)
        for p in game.players:
            for i in p.infosets:
                for a in i.actions:
                    assert abs(eq[p][i][a] - expected[p][i][a]) <= TOL


@pytest.mark.nash
@pytest.mark.nash_enumpoly_behavior
@pytest.mark.parametrize(
    "game,mixed_behav_prof_data,stop_after",
    [
        # 3-player game
        (
                games.create_mixed_behav_game_efg(),
                [
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
                9,
        ),
    ],
)
def test_enumpoly_unordered_behavior(
        game: gbt.Game, mixed_behav_prof_data: list, stop_after: None | int
):
    """Test calls of enumpoly for mixed behavior equilibria,
    using max_regret and agent_max_regret (internal consistency); and
    comparison to a set of previously computed equilibria using this function (regression test).

    This set will be the full set of all computed equilibria if stop_after is None,
    else the first stop_after-many equilibria.

    This is the "unordered" version where we test for the outputs belong to a set
    of expected output; there is also an "unordered" that expects the outputs in a specific order.

    In this unordered version, once something from the expected set is found it is removed,
    so we are checking for no duplicate outputs.
    """
    if stop_after:
        result = gbt.nash.enumpoly_solve(
            game, use_strategic=False, stop_after=stop_after, maxregret=0.00001
        )
        assert len(result.equilibria) == stop_after
    else:
        # compute all
        result = gbt.nash.enumpoly_solve(game, use_strategic=False)

    assert len(result.equilibria) == len(mixed_behav_prof_data)

    def are_the_same(game, found, candidate):
        for p in game.players:
            for i in p.infosets:
                for a in i.actions:
                    if not abs(found[p][i][a] - candidate[p][i][a]) <= TOL:
                        return False
        return True

    for eq in result.equilibria:
        assert abs(eq.max_regret()) <= TOL
        assert abs(eq.agent_max_regret()) <= TOL
        found = False
        for exp in mixed_behav_prof_data[:]:
            expected = game.mixed_behavior_profile(rational=True, data=exp)
            if are_the_same(game, eq, expected):
                mixed_behav_prof_data.remove(exp)
                found = True
                break
        assert found


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
        (
                games.create_2x2_zero_sum_efg(),
                [[["1/2", "1/2"], ["1/2", "1/2"]]],
                None
        ),
        (
                games.create_2x2_zero_sum_efg(missing_term_outcome=True),
                [[["1/2", "1/2"], ["1/2", "1/2"]]],
                None
        ),
        (games.create_stripped_down_poker_efg(), [[["1/3", "2/3", 0, 0], ["2/3", "1/3"]]], None),
        (
                games.create_stripped_down_poker_efg(nonterm_outcomes=True),
                [[["1/3", "2/3", 0, 0], ["2/3", "1/3"]]],
                None
        ),
        (games.create_kuhn_poker_efg(), [games.kuhn_poker_lcp_first_mixed_strategy_prof()], 1),
        (
                games.create_kuhn_poker_efg(nonterm_outcomes=True),
                [games.kuhn_poker_lcp_first_mixed_strategy_prof()],
                1
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
                    [["1/30", "1/6", "3/10", "3/10", "1/6", "1/30"],
                     ["1/6", "1/30", "3/10", "3/10", "1/30", "1/6"]],
                ],
                None
        ),
    ]
)
def test_lcp_strategy_rational(game: gbt.Game, mixed_strategy_prof_data: list,
                               stop_after: None | int):
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
        (
                games.create_2x2_zero_sum_efg(),
                [[["1/2", "1/2"]], [["1/2", "1/2"]]]
        ),
        (
            games.create_2x2_zero_sum_efg(missing_term_outcome=True),
            [[["1/2", "1/2"]], [["1/2", "1/2"]]],
        ),
        (games.create_matching_pennies_efg(),
         [[["1/2", "1/2"]], [["1/2", "1/2"]]]),
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
                games.create_perfect_info_with_chance_efg(),
                [[[0, 1]], [[0, 1], [0, 1]]],
        ),
        (
                games.create_two_player_perfect_info_win_lose_efg(),
                [[[0, 1], [1, 0]], [[0, 1], ["1/2", "1/2"]]],
        ),
        (
            games.create_two_player_perfect_info_win_lose_efg(nonterm_outcomes=True),
            [[[0, 1], [1, 0]], [[0, 1], ["1/2", "1/2"]]],
        ),
        (
            games.create_three_action_internal_outcomes_efg(),
            [
                [["1/3", 0, "2/3"], ["2/3", 0, "1/3"]],
                [["2/3", "1/3"], ["1/3", "2/3"], ["1/3", "2/3"]],
            ]
        ),
        (
            games.create_three_action_internal_outcomes_efg(nonterm_outcomes=True),
            [
                [["1/3", 0, "2/3"], ["2/3", 0, "1/3"]],
                [["2/3", "1/3"], ["1/3", "2/3"], ["1/3", "2/3"]],
            ],
        ),
        (
                games.create_large_payoff_game_efg(),
                [
                    [[1, 0], [1, 0]],
                    [[0, 1], ["9999999999999999999/10000000000000000000",
                              "1/10000000000000000000"]],
                ],
        ),
        (
            games.create_chance_in_middle_efg(),
            [
                [["3/11", "8/11"], [1, 0], [1, 0], [1, 0], [1, 0]],
                [[1, 0], ["6/11", "5/11"]]
            ]
        ),
        (
            games.create_chance_in_middle_efg(nonterm_outcomes=True),
            [
                [["3/11", "8/11"], [1, 0], [1, 0], [1, 0], [1, 0]],
                [[1, 0], ["6/11", "5/11"]]
            ],
        ),
        # Non-zero-sum games
        (
                games.create_reduction_both_players_payoff_ties_efg(),
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
            games.create_entry_accomodation_efg(),
            [[["2/3", "1/3"], [1, 0], [1, 0]], [["2/3", "1/3"]]]
        ),
        (
            games.create_entry_accomodation_efg(nonterm_outcomes=True),
            [[["2/3", "1/3"], [1, 0], [1, 0]], [["2/3", "1/3"]]],
        ),
        (
            games.create_non_zero_sum_lacking_outcome_efg(),
            [[["1/3", "2/3"]], [["1/2", "1/2"]]]
        ),
        (
            games.create_non_zero_sum_lacking_outcome_efg(missing_term_outcome=True),
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
                [["1/3", "2/3", 0, 0], ["2/3", "1/3"]]
        ),
        (games.create_kuhn_poker_efg(), games.kuhn_poker_lp_mixed_strategy_prof()),
        (
                games.create_kuhn_poker_efg(nonterm_outcomes=True),
                games.kuhn_poker_lp_mixed_strategy_prof()
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


@pytest.mark.nash
@pytest.mark.nash_lp_behavior
@pytest.mark.parametrize(
    "game,mixed_behav_prof_data",
    [
        (
                games.create_two_player_perfect_info_win_lose_efg(),
                [[[0, 1], [1, 0]], [[1, 0], [1, 0]]],
        ),
        (
                games.create_two_player_perfect_info_win_lose_efg(nonterm_outcomes=True),
                [[[0, 1], [1, 0]], [[1, 0], [1, 0]]],
        ),
        (
                games.create_2x2_zero_sum_efg(missing_term_outcome=False),
                [[["1/2", "1/2"]], [["1/2", "1/2"]]]
        ),
        (
            games.create_2x2_zero_sum_efg(missing_term_outcome=True),
            [[["1/2", "1/2"]], [["1/2", "1/2"]]],
        ),
        (games.create_matching_pennies_efg(with_neutral_outcome=False),
         [[["1/2", "1/2"]], [["1/2", "1/2"]]]),
        (
            games.create_matching_pennies_efg(with_neutral_outcome=True),
            [[["1/2", "1/2"]], [["1/2", "1/2"]]],
        ),
        (
                games.create_stripped_down_poker_efg(),
                [[[1, 0], ["1/3", "2/3"]], [["2/3", "1/3"]]],
        ),
        (
            games.create_stripped_down_poker_efg(nonterm_outcomes=True),
            [[[1, 0], ["1/3", "2/3"]], [["2/3", "1/3"]]],
        ),
        (
                games.create_kuhn_poker_efg(),
                [
                    [[1, 0], [1, 0], [1, 0], ["2/3", "1/3"], [1, 0], [0, 1]],
                    [[1, 0], ["2/3", "1/3"], [0, 1], [0, 1], ["2/3", "1/3"], [1, 0]],
                ],
        ),
        (
            games.create_kuhn_poker_efg(nonterm_outcomes=True),
            [
                [
                    [1, 0],
                    [1, 0],
                    [1, 0],
                    ["2/3", "1/3"],
                    [1, 0],
                    [0, 1],
                ],
                [[1, 0], ["2/3", "1/3"], [0, 1], [0, 1], ["2/3", "1/3"], [1, 0]],
            ],
        ),
        (
                games.create_seq_form_STOC_paper_zero_sum_2_player_efg(),
                [
                    [[0, 1], ["2/3", "1/3"], ["1/3", "2/3"]],
                    [["5/6", "1/6"], ["5/9", "4/9"]],
                ],
        ),
        (
                games.create_perfect_info_with_chance_efg(),
                [[[0, 1]], [[1, 0], [1, 0]]],
        ),
        (
            games.create_three_action_internal_outcomes_efg(),
            [
                [["1/3", 0, "2/3"], ["2/3", 0, "1/3"]],
                [["2/3", "1/3"], ["2/3", "1/3"], ["1/3", "2/3"]],
            ]
        ),
        (
            games.create_three_action_internal_outcomes_efg(nonterm_outcomes=True),
            [
                [["1/3", 0, "2/3"], ["2/3", 0, "1/3"]],
                [["2/3", "1/3"], ["2/3", "1/3"], ["1/3", "2/3"]],
            ],
        ),
        (
                games.create_large_payoff_game_efg(),
                [
                    [[1, 0], [1, 0]],
                    [[0, 1], ["9999999999999999999/10000000000000000000",
                              "1/10000000000000000000"]],
                ],
        ),
        (
            games.create_chance_in_middle_efg(),
            [
                [["3/11", "8/11"], [1, 0], [1, 0], [1, 0], [1, 0]],
                [[1, 0], ["6/11", "5/11"]]
            ],
        ),
        (
            games.create_chance_in_middle_efg(nonterm_outcomes=True),
            [
                [["3/11", "8/11"], [1, 0], [1, 0], [1, 0], [1, 0]],
                [[1, 0], ["6/11", "5/11"]]
            ],
        ),
    ],
)
def test_lp_behavior_rational(game: gbt.Game, mixed_behav_prof_data: list):
    """Test calls of LP for mixed behavior equilibria, rational precision,
    using max_regret and agent_max_regret (internal consistency); and
    comparison to a previously computed equilibrium using this function (regression test).
    """
    result = gbt.nash.lp_solve(game, use_strategic=False, rational=True)
    assert len(result.equilibria) == 1
    eq = result.equilibria[0]
    assert eq.max_regret() == 0
    assert eq.agent_max_regret() == 0
    expected = game.mixed_behavior_profile(rational=True, data=mixed_behav_prof_data)
    assert eq == expected


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
    assert len(gbt.qre.logit_solve_branch(
        game=game, maxregret=0.2, first_step=0.2, max_accel=1)) > 0


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
    assert len(gbt.qre.logit_solve_lambda(
        game=game, lam=[1, 2, 3], first_step=0.2, max_accel=1)) > 0
