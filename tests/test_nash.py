"""Test of calls to Nash equilibrium and QRE solvers."""

import dataclasses
import functools
import typing

import pytest

import pygambit as gbt
from pygambit import Rational as Q

from . import games

TOL = 1e-13  # standard tol for floating point assertions; not used only when it is too small
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


@dataclasses.dataclass
class EquilibriumTestCaseWithStart:
    """Summarising the data relevant for a test fixture of a call to an equilibrium solver
    that needs a starting profile."""

    factory: typing.Callable[[], gbt.Game]
    solver: typing.Callable[[gbt.Game], gbt.nash.NashComputationResult]
    start_data: None | list
    expected: list
    regret_tol: float | gbt.Rational = Q(0)
    prob_tol: float | gbt.Rational = Q(0)


@dataclasses.dataclass
class QREquilibriumTestCase:
    """Summarising the data relevant for a test fixture of a call to an QRE solver."""

    factory: typing.Callable[[], gbt.Game]
    solver: typing.Callable[[gbt.Game], gbt.qre.LogitQREMixedStrategyFitResult]
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
    # Action graph games
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "2x2.agg"),
            solver=gbt.nash.enumpure_solve,
            expected=[
                [d(1, 0), d(1, 0)],
                [d(0, 1), d(0, 1)],
            ],
        ),
        marks=pytest.mark.nash_enumpure_strategy,
        id="test_enumpure_9",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "2x2_small_payoffs.agg"),
            solver=gbt.nash.enumpure_solve,
            expected=[
                [d(1, 0), d(1, 0)],
                [d(0, 1), d(0, 1)],
            ],
        ),
        marks=pytest.mark.nash_enumpure_strategy,
        id="test_enumpure_10",
    ),
    # Bayesian Action graph games
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "Bayesian-Coffee-3-2-2-3.bagg"),
            solver=gbt.nash.enumpure_solve,
            expected=[
                [
                    [0, 1, 0, 0, 0, 0, 0],
                    [1, 0, 0, 0, 0, 0, 0],
                    [1, 0, 0, 0, 0, 0, 0],
                    [1, 0, 0, 0, 0, 0, 0],
                    [1, 0, 0, 0, 0, 0, 0],
                    [1, 0, 0, 0, 0, 0, 0],
                ],
                [
                    [1, 0, 0, 0, 0, 0, 0],
                    [1, 0, 0, 0, 0, 0, 0],
                    [0, 1, 0, 0, 0, 0, 0],
                    [1, 0, 0, 0, 0, 0, 0],
                    [1, 0, 0, 0, 0, 0, 0],
                    [1, 0, 0, 0, 0, 0, 0],
                ],
                [
                    [1, 0, 0, 0, 0, 0, 0],
                    [1, 0, 0, 0, 0, 0, 0],
                    [1, 0, 0, 0, 0, 0, 0],
                    [1, 0, 0, 0, 0, 0, 0],
                    [0, 1, 0, 0, 0, 0, 0],
                    [1, 0, 0, 0, 0, 0, 0],
                ],
            ],
        ),
        marks=pytest.mark.nash_enumpure_strategy,
        id="test_enumpure_11",
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
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                gbt.catalog.load, "journals/dcg/vonstengel1999/6x6_game_with_75_eq"
            ),
            solver=functools.partial(gbt.nash.enummixed_solve, rational=True),
            expected=[
                [d("20/23", "3/23", 0, 0, 0, 0), d("4/19", "15/19", 0, 0, 0, 0)],
                [d("45/52", "7/52", 0, 0, 0, 0), d(0, "14/17", "3/17", 0, 0, 0)],
                [d("25/29", "4/29", 0, 0, 0, 0), d(0, 0, "5/9", "4/9", 0, 0)],
                [d("55/64", "9/64", 0, 0, 0, 0), d(0, 0, 0, "3/58", "55/58", 0)],
                [d("6/7", "1/7", 0, 0, 0, 0), d(0, 0, 0, 0, "6/7", "1/7")],
                [
                    d("300/443", "108/443", "30/443", "5/443", 0, 0),
                    d("30/97", "25/97", "15/97", "27/97", 0, 0),
                ],
                [
                    d("1540/2297", "567/2297", "162/2297", "28/2297", 0, 0),
                    d("70/347", "70/347", 0, "42/347", "165/347", 0),
                ],
                [
                    d("1232/1853", "462/1853", "135/1853", "24/1853", 0, 0),
                    d("7/199", "21/398", 0, 0, "297/398", "33/199"),
                ],
                [
                    d("1584/2425", "616/2425", "189/2425", "36/2425", 0, 0),
                    d(0, "7/841", "42/841", 0, "594/841", "198/841"),
                ],
                [
                    d("275/427", "110/427", "5/61", "1/61", 0, 0),
                    d(0, 0, "5/61", "1/61", "275/427", "110/427"),
                ],
                [
                    d("5775/9871", "2640/9871", "1050/9871", "336/9871", "70/9871", 0),
                    d("70/9871", 0, "1050/9871", "336/9871", "5775/9871", "2640/9871"),
                ],
                [
                    d("132/245", "66/245", "6/49", "12/245", "4/245", "1/245"),
                    d("4/245", "1/245", "6/49", "12/245", "132/245", "66/245"),
                ],
                [
                    d("420/571", "126/571", "70/1713", 0, 0, "5/1713"),
                    d("112/367", "84/367", "63/367", "108/367", 0, 0),
                ],
                [
                    d("154/211", "189/844", "9/211", 0, 0, "3/844"),
                    d("35/188", "63/376", 0, "45/376", "99/188", 0),
                ],
                [
                    d("1232/1699", "385/1699", "75/1699", 0, 0, "7/1699"),
                    d("140/4663", "189/4663", 0, 0, "3564/4663", "770/4663"),
                ],
                [
                    d("4752/6649", "1540/6649", "315/6649", 0, 0, "42/6649"),
                    d(0, "42/6649", "315/6649", 0, "4752/6649", "1540/6649"),
                ],
                [
                    d("594/841", "198/841", "42/841", 0, 0, "7/841"),
                    d(0, 0, "189/2425", "36/2425", "1584/2425", "616/2425"),
                ],
                [
                    d("525/626", "48/313", 0, 0, "5/626", 0),
                    d("56/227", 0, "75/227", "96/227", 0, 0),
                ],
                [
                    d("1925/2306", "180/1153", 0, 0, "21/2306", 0),
                    d("42/475", 0, 0, "48/475", "77/95", 0),
                ],
                [
                    d("385/463", "220/1389", 0, 0, "14/1389", 0),
                    d("14/1389", 0, 0, 0, "385/463", "220/1389"),
                ],
                [
                    d("105/131", "21/131", 0, 0, "5/262", "5/262"),
                    d("112/377", "70/377", "75/377", "120/377", 0, 0),
                ],
                [
                    d("770/971", "315/1942", 0, 0, "21/971", "45/1942"),
                    d("84/515", "63/515", 0, "12/103", "308/515", 0),
                ],
                [
                    d("11/14", "55/336", 0, 0, "1/42", "3/112"),
                    d("1/42", "3/112", 0, 0, "11/14", "55/336"),
                ],
                [
                    d("3564/4663", "770/4663", 0, 0, "140/4663", "189/4663"),
                    d(0, "7/1699", "75/1699", 0, "1232/1699", "385/1699"),
                ],
                [
                    d("297/398", "33/199", 0, 0, "7/199", "21/398"),
                    d(0, 0, "135/1853", "24/1853", "1232/1853", "462/1853"),
                ],
                [d("28/29", 0, 0, "1/29", 0, 0), d("7/32", "25/32", 0, 0, 0, 0)],
                [d("24/25", 0, 0, "1/25", 0, 0), d(0, "25/31", "6/31", 0, 0, 0)],
                [d("150/157", 0, 0, "7/157", 0, 0), d(0, 0, "9/16", "7/16", 0, 0)],
                [d("275/289", 0, 0, "14/289", 0, 0), d(0, 0, 0, "14/289", "275/289", 0)],
                [d("55/58", 0, 0, "3/58", 0, 0), d(0, 0, 0, 0, "55/64", "9/64")],
                [
                    d("525/626", 0, 0, "28/313", "45/626", 0),
                    d("63/265", 0, "18/53", "112/265", 0, 0),
                ],
                [
                    d("275/334", 0, 0, "16/167", "27/334", 0),
                    d("27/334", 0, 0, "16/167", "275/334", 0),
                ],
                [
                    d("77/95", 0, 0, "48/475", "42/475", 0),
                    d("21/2306", 0, 0, 0, "1925/2306", "180/1153"),
                ],
                [
                    d("21/32", 0, 0, "7/64", "9/64", "3/32"),
                    d("126/431", "75/431", "90/431", "140/431", 0, 0),
                ],
                [
                    d("385/617", 0, 0, "70/617", "189/1234", "135/1234"),
                    d("189/1234", "135/1234", 0, "70/617", "385/617", 0),
                ],
                [
                    d("308/515", 0, 0, "12/103", "84/515", "63/515"),
                    d("21/971", "45/1942", 0, 0, "770/971", "315/1942"),
                ],
                [
                    d("99/188", 0, 0, "45/376", "35/188", "63/376"),
                    d(0, "3/844", "9/211", 0, "154/211", "189/844"),
                ],
                [
                    d("165/347", 0, 0, "42/347", "70/347", "70/347"),
                    d(0, 0, "162/2297", "28/2297", "1540/2297", "567/2297"),
                ],
                [d(0, 1, 0, 0, 0, 0), d(0, 0, 0, 0, 0, 1)],
                [d(0, 0, "3/5", "2/5", 0, 0), d("3/13", "10/13", 0, 0, 0, 0)],
                [d(0, 0, "7/12", "5/12", 0, 0), d(0, "25/32", "7/32", 0, 0, 0)],
                [d(0, 0, "4/7", "3/7", 0, 0), d(0, 0, "4/7", "3/7", 0, 0)],
                [d(0, 0, "9/16", "7/16", 0, 0), d(0, 0, 0, "7/157", "150/157", 0)],
                [d(0, 0, "5/9", "4/9", 0, 0), d(0, 0, 0, 0, "25/29", "4/29")],
                [
                    d(0, "792/1541", "560/1541", "189/1541", 0, 0),
                    d(0, 0, "560/1541", "189/1541", 0, "792/1541"),
                ],
                [
                    d(0, "616/1171", "420/1171", "135/1171", 0, 0),
                    d(0, "175/1049", "280/1049", 0, 0, "594/1049"),
                ],
                [
                    d(0, "77/142", "25/71", "15/142", 0, 0),
                    d("80/321", "175/321", 0, 0, 0, "22/107"),
                ],
                [
                    d(0, 0, "70/199", "84/199", "45/199", 0),
                    d("45/199", 0, "70/199", "84/199", 0, 0),
                ],
                [
                    d(0, 0, "18/53", "112/265", "63/265", 0),
                    d("45/626", 0, 0, "28/313", "525/626", 0),
                ],
                [
                    d(0, 0, "75/227", "96/227", "56/227", 0),
                    d("5/626", 0, 0, 0, "525/626", "48/313"),
                ],
                [
                    d(0, "396/995", "70/199", "189/995", "12/199", 0),
                    d("12/199", 0, "70/199", "189/995", 0, "396/995"),
                ],
                [d(0, 0, "2/9", "1/3", "2/7", "10/63"), d("2/7", "10/63", "2/9", "1/3", 0, 0)],
                [
                    d(0, 0, "90/431", "140/431", "126/431", "75/431"),
                    d("9/64", "3/32", 0, "7/64", "21/32", 0),
                ],
                [
                    d(0, 0, "75/377", "120/377", "112/377", "70/377"),
                    d("5/262", "5/262", 0, 0, "105/131", "21/131"),
                ],
                [
                    d(0, 0, "63/367", "108/367", "112/367", "84/367"),
                    d(0, "5/1713", "70/1713", 0, "420/571", "126/571"),
                ],
                [
                    d(0, 0, "15/97", "27/97", "30/97", "25/97"),
                    d(0, 0, "30/443", "5/443", "300/443", "108/443"),
                ],
                [
                    d(0, "231/731", "700/2193", "315/1462", "80/731", "175/4386"),
                    d("80/731", "175/4386", "700/2193", "315/1462", 0, "231/731"),
                ],
                [d(0, 0, "1/3", 0, 0, "2/3"), d("1/4", "3/4", 0, 0, 0, 0)],
                [d(0, 0, "7/27", 0, 0, "20/27"), d(0, "20/27", "7/27", 0, 0, 0)],
                [d(0, 0, "7/32", 0, 0, "25/32"), d(0, 0, "7/12", "5/12", 0, 0)],
                [d(0, 0, "6/31", 0, 0, "25/31"), d(0, 0, 0, "1/25", "24/25", 0)],
                [d(0, 0, "3/17", 0, 0, "14/17"), d(0, 0, 0, 0, "45/52", "7/52")],
                [
                    d(0, "594/1049", "280/1049", 0, 0, "175/1049"),
                    d(0, 0, "420/1171", "135/1171", 0, "616/1171"),
                ],
                [d(0, "22/37", "10/37", 0, 0, "5/37"), d(0, "5/37", "10/37", 0, 0, "22/37")],
                [
                    d(0, "77/122", "50/183", 0, 0, "35/366"),
                    d("160/629", "315/629", 0, 0, 0, "154/629"),
                ],
                [d(0, 0, 0, 0, 1, 0), d(1, 0, 0, 0, 0, 0)],
                [d(0, "11/15", 0, 0, "4/15", 0), d("4/15", 0, 0, 0, 0, "11/15")],
                [d(0, 0, 0, 0, "2/7", "5/7"), d("2/7", "5/7", 0, 0, 0, 0)],
                [d(0, 0, 0, 0, "1/4", "3/4"), d(0, "2/3", "1/3", 0, 0, 0)],
                [d(0, 0, 0, 0, "3/13", "10/13"), d(0, 0, "3/5", "2/5", 0, 0)],
                [d(0, 0, 0, 0, "7/32", "25/32"), d(0, 0, 0, "1/29", "28/29", 0)],
                [d(0, 0, 0, 0, "4/19", "15/19"), d(0, 0, 0, 0, "20/23", "3/23")],
                [
                    d(0, "22/107", 0, 0, "80/321", "175/321"),
                    d(0, 0, "25/71", "15/142", 0, "77/142"),
                ],
                [
                    d(0, "154/629", 0, 0, "160/629", "315/629"),
                    d(0, "35/366", "50/183", 0, 0, "77/122"),
                ],
                [d(0, "77/246", 0, 0, "32/123", "35/82"), d("32/123", "35/82", 0, 0, 0, "77/246")],
            ],
        ),
        marks=pytest.mark.nash_enummixed_strategy,
        id="test_enumixed_rational_6",
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
    # Action graph games
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "2x2.agg"),
            solver=functools.partial(gbt.nash.enummixed_solve, rational=False),
            expected=[
                [d(1, 0), d(1, 0)],
                [d(0, 1), d(0, 1)],
                [
                    d("10/11", "1/11"),
                    d("10/11", "1/11"),
                ],
            ],
            prob_tol=TOL,
            regret_tol=TOL,
        ),
        marks=pytest.mark.nash_enummixed_strategy,
        id="test_enummixed_double_7",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "2x2_small_payoffs.agg"),
            solver=functools.partial(gbt.nash.enummixed_solve, rational=False),
            expected=[
                [d(1, 0), d(1, 0)],
                [d(0, 1), d(0, 1)],
                [
                    d("1/2", "1/2"),
                    d("1/2", "1/2"),
                ],
            ],
            prob_tol=TOL,
            regret_tol=TOL,
        ),
        marks=pytest.mark.nash_enummixed_strategy,
        id="test_enummixed_double_8",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                gbt.catalog.load, "journals/dcg/vonstengel1999/6x6_game_with_75_eq"
            ),
            solver=functools.partial(gbt.nash.enummixed_solve, rational=False),
            expected=[
                [d("20/23", "3/23", 0, 0, 0, 0), d("4/19", "15/19", 0, 0, 0, 0)],
                [d("45/52", "7/52", 0, 0, 0, 0), d(0, "14/17", "3/17", 0, 0, 0)],
                [d("25/29", "4/29", 0, 0, 0, 0), d(0, 0, "5/9", "4/9", 0, 0)],
                [d("55/64", "9/64", 0, 0, 0, 0), d(0, 0, 0, "3/58", "55/58", 0)],
                [d("6/7", "1/7", 0, 0, 0, 0), d(0, 0, 0, 0, "6/7", "1/7")],
                [
                    d("300/443", "108/443", "30/443", "5/443", 0, 0),
                    d("30/97", "25/97", "15/97", "27/97", 0, 0),
                ],
                [
                    d("1540/2297", "567/2297", "162/2297", "28/2297", 0, 0),
                    d("70/347", "70/347", 0, "42/347", "165/347", 0),
                ],
                [
                    d("1232/1853", "462/1853", "135/1853", "24/1853", 0, 0),
                    d("7/199", "21/398", 0, 0, "297/398", "33/199"),
                ],
                [
                    d("1584/2425", "616/2425", "189/2425", "36/2425", 0, 0),
                    d(0, "7/841", "42/841", 0, "594/841", "198/841"),
                ],
                [
                    d("275/427", "110/427", "5/61", "1/61", 0, 0),
                    d(0, 0, "5/61", "1/61", "275/427", "110/427"),
                ],
                [
                    d("5775/9871", "2640/9871", "1050/9871", "336/9871", "70/9871", 0),
                    d("70/9871", 0, "1050/9871", "336/9871", "5775/9871", "2640/9871"),
                ],
                [
                    d("132/245", "66/245", "6/49", "12/245", "4/245", "1/245"),
                    d("4/245", "1/245", "6/49", "12/245", "132/245", "66/245"),
                ],
                [
                    d("420/571", "126/571", "70/1713", 0, 0, "5/1713"),
                    d("112/367", "84/367", "63/367", "108/367", 0, 0),
                ],
                [
                    d("154/211", "189/844", "9/211", 0, 0, "3/844"),
                    d("35/188", "63/376", 0, "45/376", "99/188", 0),
                ],
                [
                    d("1232/1699", "385/1699", "75/1699", 0, 0, "7/1699"),
                    d("140/4663", "189/4663", 0, 0, "3564/4663", "770/4663"),
                ],
                [
                    d("4752/6649", "1540/6649", "315/6649", 0, 0, "42/6649"),
                    d(0, "42/6649", "315/6649", 0, "4752/6649", "1540/6649"),
                ],
                [
                    d("594/841", "198/841", "42/841", 0, 0, "7/841"),
                    d(0, 0, "189/2425", "36/2425", "1584/2425", "616/2425"),
                ],
                [
                    d("525/626", "48/313", 0, 0, "5/626", 0),
                    d("56/227", 0, "75/227", "96/227", 0, 0),
                ],
                [
                    d("1925/2306", "180/1153", 0, 0, "21/2306", 0),
                    d("42/475", 0, 0, "48/475", "77/95", 0),
                ],
                [
                    d("385/463", "220/1389", 0, 0, "14/1389", 0),
                    d("14/1389", 0, 0, 0, "385/463", "220/1389"),
                ],
                [
                    d("105/131", "21/131", 0, 0, "5/262", "5/262"),
                    d("112/377", "70/377", "75/377", "120/377", 0, 0),
                ],
                [
                    d("770/971", "315/1942", 0, 0, "21/971", "45/1942"),
                    d("84/515", "63/515", 0, "12/103", "308/515", 0),
                ],
                [
                    d("11/14", "55/336", 0, 0, "1/42", "3/112"),
                    d("1/42", "3/112", 0, 0, "11/14", "55/336"),
                ],
                [
                    d("3564/4663", "770/4663", 0, 0, "140/4663", "189/4663"),
                    d(0, "7/1699", "75/1699", 0, "1232/1699", "385/1699"),
                ],
                [
                    d("297/398", "33/199", 0, 0, "7/199", "21/398"),
                    d(0, 0, "135/1853", "24/1853", "1232/1853", "462/1853"),
                ],
                [d("28/29", 0, 0, "1/29", 0, 0), d("7/32", "25/32", 0, 0, 0, 0)],
                [d("24/25", 0, 0, "1/25", 0, 0), d(0, "25/31", "6/31", 0, 0, 0)],
                [d("150/157", 0, 0, "7/157", 0, 0), d(0, 0, "9/16", "7/16", 0, 0)],
                [d("275/289", 0, 0, "14/289", 0, 0), d(0, 0, 0, "14/289", "275/289", 0)],
                [d("55/58", 0, 0, "3/58", 0, 0), d(0, 0, 0, 0, "55/64", "9/64")],
                [
                    d("525/626", 0, 0, "28/313", "45/626", 0),
                    d("63/265", 0, "18/53", "112/265", 0, 0),
                ],
                [
                    d("275/334", 0, 0, "16/167", "27/334", 0),
                    d("27/334", 0, 0, "16/167", "275/334", 0),
                ],
                [
                    d("77/95", 0, 0, "48/475", "42/475", 0),
                    d("21/2306", 0, 0, 0, "1925/2306", "180/1153"),
                ],
                [
                    d("21/32", 0, 0, "7/64", "9/64", "3/32"),
                    d("126/431", "75/431", "90/431", "140/431", 0, 0),
                ],
                [
                    d("385/617", 0, 0, "70/617", "189/1234", "135/1234"),
                    d("189/1234", "135/1234", 0, "70/617", "385/617", 0),
                ],
                [
                    d("308/515", 0, 0, "12/103", "84/515", "63/515"),
                    d("21/971", "45/1942", 0, 0, "770/971", "315/1942"),
                ],
                [
                    d("99/188", 0, 0, "45/376", "35/188", "63/376"),
                    d(0, "3/844", "9/211", 0, "154/211", "189/844"),
                ],
                [
                    d("165/347", 0, 0, "42/347", "70/347", "70/347"),
                    d(0, 0, "162/2297", "28/2297", "1540/2297", "567/2297"),
                ],
                [d(0, 1, 0, 0, 0, 0), d(0, 0, 0, 0, 0, 1)],
                [d(0, 0, "3/5", "2/5", 0, 0), d("3/13", "10/13", 0, 0, 0, 0)],
                [d(0, 0, "7/12", "5/12", 0, 0), d(0, "25/32", "7/32", 0, 0, 0)],
                [d(0, 0, "4/7", "3/7", 0, 0), d(0, 0, "4/7", "3/7", 0, 0)],
                [d(0, 0, "9/16", "7/16", 0, 0), d(0, 0, 0, "7/157", "150/157", 0)],
                [d(0, 0, "5/9", "4/9", 0, 0), d(0, 0, 0, 0, "25/29", "4/29")],
                [
                    d(0, "792/1541", "560/1541", "189/1541", 0, 0),
                    d(0, 0, "560/1541", "189/1541", 0, "792/1541"),
                ],
                [
                    d(0, "616/1171", "420/1171", "135/1171", 0, 0),
                    d(0, "175/1049", "280/1049", 0, 0, "594/1049"),
                ],
                [
                    d(0, "77/142", "25/71", "15/142", 0, 0),
                    d("80/321", "175/321", 0, 0, 0, "22/107"),
                ],
                [
                    d(0, 0, "70/199", "84/199", "45/199", 0),
                    d("45/199", 0, "70/199", "84/199", 0, 0),
                ],
                [
                    d(0, 0, "18/53", "112/265", "63/265", 0),
                    d("45/626", 0, 0, "28/313", "525/626", 0),
                ],
                [
                    d(0, 0, "75/227", "96/227", "56/227", 0),
                    d("5/626", 0, 0, 0, "525/626", "48/313"),
                ],
                [
                    d(0, "396/995", "70/199", "189/995", "12/199", 0),
                    d("12/199", 0, "70/199", "189/995", 0, "396/995"),
                ],
                [d(0, 0, "2/9", "1/3", "2/7", "10/63"), d("2/7", "10/63", "2/9", "1/3", 0, 0)],
                [
                    d(0, 0, "90/431", "140/431", "126/431", "75/431"),
                    d("9/64", "3/32", 0, "7/64", "21/32", 0),
                ],
                [
                    d(0, 0, "75/377", "120/377", "112/377", "70/377"),
                    d("5/262", "5/262", 0, 0, "105/131", "21/131"),
                ],
                [
                    d(0, 0, "63/367", "108/367", "112/367", "84/367"),
                    d(0, "5/1713", "70/1713", 0, "420/571", "126/571"),
                ],
                [
                    d(0, 0, "15/97", "27/97", "30/97", "25/97"),
                    d(0, 0, "30/443", "5/443", "300/443", "108/443"),
                ],
                [
                    d(0, "231/731", "700/2193", "315/1462", "80/731", "175/4386"),
                    d("80/731", "175/4386", "700/2193", "315/1462", 0, "231/731"),
                ],
                [d(0, 0, "1/3", 0, 0, "2/3"), d("1/4", "3/4", 0, 0, 0, 0)],
                [d(0, 0, "7/27", 0, 0, "20/27"), d(0, "20/27", "7/27", 0, 0, 0)],
                [d(0, 0, "7/32", 0, 0, "25/32"), d(0, 0, "7/12", "5/12", 0, 0)],
                [d(0, 0, "6/31", 0, 0, "25/31"), d(0, 0, 0, "1/25", "24/25", 0)],
                [d(0, 0, "3/17", 0, 0, "14/17"), d(0, 0, 0, 0, "45/52", "7/52")],
                [
                    d(0, "594/1049", "280/1049", 0, 0, "175/1049"),
                    d(0, 0, "420/1171", "135/1171", 0, "616/1171"),
                ],
                [d(0, "22/37", "10/37", 0, 0, "5/37"), d(0, "5/37", "10/37", 0, 0, "22/37")],
                [
                    d(0, "77/122", "50/183", 0, 0, "35/366"),
                    d("160/629", "315/629", 0, 0, 0, "154/629"),
                ],
                [d(0, 0, 0, 0, 1, 0), d(1, 0, 0, 0, 0, 0)],
                [d(0, "11/15", 0, 0, "4/15", 0), d("4/15", 0, 0, 0, 0, "11/15")],
                [d(0, 0, 0, 0, "2/7", "5/7"), d("2/7", "5/7", 0, 0, 0, 0)],
                [d(0, 0, 0, 0, "1/4", "3/4"), d(0, "2/3", "1/3", 0, 0, 0)],
                [d(0, 0, 0, 0, "3/13", "10/13"), d(0, 0, "3/5", "2/5", 0, 0)],
                [d(0, 0, 0, 0, "7/32", "25/32"), d(0, 0, 0, "1/29", "28/29", 0)],
                [d(0, 0, 0, 0, "4/19", "15/19"), d(0, 0, 0, 0, "20/23", "3/23")],
                [
                    d(0, "22/107", 0, 0, "80/321", "175/321"),
                    d(0, 0, "25/71", "15/142", 0, "77/142"),
                ],
                [
                    d(0, "154/629", 0, 0, "160/629", "315/629"),
                    d(0, "35/366", "50/183", 0, 0, "77/122"),
                ],
                [d(0, "77/246", 0, 0, "32/123", "35/82"), d("32/123", "35/82", 0, 0, 0, "77/246")],
            ],
            prob_tol=TOL_LARGE,
            regret_tol=TOL_LARGE,
        ),
        marks=pytest.mark.nash_enummixed_strategy,
        id="test_enumixed_double_9",
    ),
]


ENUMPOLY_STRATEGY_CASES = [
    # 2-player games
    # 3x3 coordination game with 3 pure and 4 mixed equilibria
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_EFG_for_nxn_bimatrix_coordination_game, n=3),
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=None, use_strategic=True),
            expected=[
                [d(1, 0, 0), d(1, 0, 0)],
                [d(0, 1, 0), d(0, 1, 0)],
                [d(0, 0, 1), d(0, 0, 1)],
                [d("1/2", "1/2", 0), d("1/2", "1/2", 0)],
                [d("1/2", 0, "1/2"), d("1/2", 0, "1/2")],
                [d(0, "1/2", "1/2"), d(0, "1/2", "1/2")],
                [d("1/3", "1/3", "1/3"), d("1/3", "1/3", "1/3")],
            ],
            prob_tol=TOL,
            regret_tol=TOL,
        ),
        marks=pytest.mark.nash_enumpoly_strategy,
        id="test_enumpoly_strategy_1",
    ),
    # 4x4 coordination game
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_EFG_for_nxn_bimatrix_coordination_game, n=4),
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=None, use_strategic=True),
            expected=[
                [d(1, 0, 0, 0), d(1, 0, 0, 0)],
                [d(0, 1, 0, 0), d(0, 1, 0, 0)],
                [d(0, 0, 1, 0), d(0, 0, 1, 0)],
                [d(0, 0, 0, 1), d(0, 0, 0, 1)],
                [d("1/2", "1/2", 0, 0), d("1/2", "1/2", 0, 0)],
                [d("1/2", 0, "1/2", 0), d("1/2", 0, "1/2", 0)],
                [d("1/2", 0, 0, "1/2"), d("1/2", 0, 0, "1/2")],
                [d(0, "1/2", "1/2", 0), d(0, "1/2", "1/2", 0)],
                [d(0, "1/2", 0, "1/2"), d(0, "1/2", 0, "1/2")],
                [d(0, 0, "1/2", "1/2"), d(0, 0, "1/2", "1/2")],
                [d("1/3", "1/3", "1/3", 0), d("1/3", "1/3", "1/3", 0)],
                [d("1/3", "1/3", 0, "1/3"), d("1/3", "1/3", 0, "1/3")],
                [d("1/3", 0, "1/3", "1/3"), d("1/3", 0, "1/3", "1/3")],
                [d(0, "1/3", "1/3", "1/3"), d(0, "1/3", "1/3", "1/3")],
                [d("1/4", "1/4", "1/4", "1/4"), d("1/4", "1/4", "1/4", "1/4")],
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_enumpoly_strategy,
        id="test_enumpoly_strategy_2",
    ),
    # Figure 10.5 efg from Game Theory Basics
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(gbt.catalog.load, "books/vonstengel2022/fig10.5"),
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=None, use_strategic=True),
            expected=[
                [d(1, 0), d(1, 0, 0, 0)],
                [d("1/2", "1/2"), d(0, "2/5", "3/5", 0)],
                [d("1/4", "3/4"), d(0, 0, "1/3", "2/3")],
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_enumpoly_strategy,
        id="test_enumpoly_strategy_3",
    ),
    # 6x6 bimatrix game with 75 equilibria
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                gbt.catalog.load, "journals/dcg/vonstengel1999/6x6_game_with_75_eq"
            ),
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=10),
            expected=[
                [d(0, 1, 0, 0, 0, 0), d(0, 0, 0, 0, 0, 1)],
                [d(0, 0, 0, 0, 1, 0), d(1, 0, 0, 0, 0, 0)],
                [d("20/23", "3/23", 0, 0, 0, 0), d("4/19", "15/19", 0, 0, 0, 0)],
                [d("45/52", "7/52", 0, 0, 0, 0), d(0, "14/17", "3/17", 0, 0, 0)],
                [d("25/29", "4/29", 0, 0, 0, 0), d(0, 0, "5/9", "4/9", 0, 0)],
                [d("55/64", "9/64", 0, 0, 0, 0), d(0, 0, 0, "3/58", "55/58", 0)],
                [d("6/7", "1/7", 0, 0, 0, 0), d(0, 0, 0, 0, "6/7", "1/7")],
                [d("28/29", 0, 0, "1/29", 0, 0), d("7/32", "25/32", 0, 0, 0, 0)],
                [d("24/25", 0, 0, "1/25", 0, 0), d(0, "25/31", "6/31", 0, 0, 0)],
                [d("150/157", 0, 0, "7/157", 0, 0), d(0, 0, "9/16", "7/16", 0, 0)],
            ],
            regret_tol=TOL_LARGE,
            prob_tol=TOL_LARGE,
        ),
        marks=pytest.mark.nash_enumpoly_strategy,
        id="test_enumpoly_strategy_4",
    ),
    #################################
    # 3-player games
    #################################
    # 2x2x2 strategic form game based on local max cut -- 2 pure and 1 mixed
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "2x2x2_nfg_from_local_max_cut_2_pure_1_mixed_eq.nfg"
            ),
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=None),
            expected=[
                [d(1, 0), d(0, 1), d(1, 0)],
                [d(0, 1), d(1, 0), d(0, 1)],
                [d("1/2", "1/2"), d("1/2", "1/2"), d("1/2", "1/2")],
            ],
            prob_tol=TOL,
            regret_tol=TOL,
        ),
        marks=pytest.mark.nash_enumpoly_strategy,
        id="test_enumpoly_strategy_5",
    ),
    # A three-player game with a unique Nash equilibrium in irrational mixed strategies
    # (nau2004 sec4 catalog game)
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(gbt.catalog.load, "journals/ijgt/nau2004/sec4"),
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=None),
            expected=[
                [
                    d(0.6192325794725537, 0.3807674205274463),
                    d(0.4798042226776053, 0.5201957773223946),
                    d(0.3788253360656313, 0.6211746639343687),
                ],
            ],
            prob_tol=TOL,
            regret_tol=TOL,
        ),
        marks=pytest.mark.nash_enumpoly_strategy,
        id="test_enumpoly_strategy_6",
    ),
    # A three-player 2x2x2 game with 3 pure, 2 incompletely mixed, and a
    # continuum of completely mixed Nash equilibria (nau2004 sec5 catalog game)
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(gbt.catalog.load, "journals/ijgt/nau2004/sec5"),
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=None),
            expected=[
                [d(1, 0), d(0, 1), d(1, 0)],
                [d(0, 1), d(1, 0), d(1, 0)],
                [d(0, 1), d(0, 1), d(0, 1)],
            ],
            prob_tol=TOL,
            regret_tol=TOL,
        ),
        marks=pytest.mark.nash_enumpoly_strategy,
        id="test_enumpoly_strategy_7",
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
    # 4x4 coordination game
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
            regret_tol=TOL,
            prob_tol=TOL,
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
            regret_tol=TOL,
            prob_tol=TOL,
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
    # 4x4 coordination game
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
    # Action graph game, solved exactly (Rational arithmetic throughout).
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "2x2.agg"),
            solver=functools.partial(
                gbt.nash.lcp_solve, rational=True, use_strategic=True, stop_after=None
            ),
            expected=[
                [d(1, 0), d(1, 0)],
                [
                    d("10/11", "1/11"),
                    d("10/11", "1/11"),
                ],
                [d(0, 1), d(0, 1)],
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lcp_strategy,
        id="test_lcp_strategy_double_12",
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
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "Bayesian-Coffee-3-2-2-3.bagg"),
            solver=gbt.nash.logit_solve,
            expected=[
                [
                    [0.9124962637548039, 0.08750373624519617, 0.0, 0.0, 0.0, 0.0, 0.0],
                    [1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
                    [0.9124962637547669, 0.08750373624523317, 0.0, 0.0, 0.0, 0.0, 0.0],
                    [1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
                    [0.9124962637547208, 0.08750373624527921, 0.0, 0.0, 0.0, 0.0, 0.0],
                    [1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
                ]
            ],
            prob_tol=TOL_LARGE,
            regret_tol=TOL_LARGE,
        ),
        marks=pytest.mark.nash_logit_strategy,
        id="test_logit_strategy_2",
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
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "2x2.agg"),
            solver=gbt.nash.gnm_solve,
            expected=[[d(1, 0), d(1, 0)]],
            prob_tol=TOL_LARGE,
            regret_tol=TOL_LARGE,
        ),
        marks=pytest.mark.nash_gnm_strategy,
        id="test_gnm_2",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "2x2.bagg"),
            solver=gbt.nash.gnm_solve,
            expected=[[d(1, 0), d(1, 0)]],
            prob_tol=TOL_LARGE,
            regret_tol=TOL_LARGE,
        ),
        marks=pytest.mark.nash_gnm_strategy,
        id="test_gnm_3",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "Bayesian-Coffee-3-2-2-3.bagg"),
            solver=gbt.nash.gnm_solve,
            expected=[
                [
                    d(1, 0, 0, 0, 0, 0, 0),
                    d(1, 0, 0, 0, 0, 0, 0),
                    d(1, 0, 0, 0, 0, 0, 0),
                    d(1, 0, 0, 0, 0, 0, 0),
                    d(0, 1, 0, 0, 0, 0, 0),
                    d(1, 0, 0, 0, 0, 0, 0),
                ]
            ],
            prob_tol=TOL_LARGE,
            regret_tol=TOL_LARGE,
        ),
        marks=pytest.mark.nash_gnm_strategy,
        id="test_gnm_4",
    ),
]


CASES = []
CASES += ENUMPURE_CASES
CASES += ENUMMIXED_RATIONAL_CASES
CASES += ENUMMIXED_DOUBLE_CASES
CASES += ENUMPOLY_STRATEGY_CASES
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
# NASH SOLVERS WITH START PROFILES
##################################################################################################


LIAP_STRATEGY_CASES = [
    pytest.param(
        EquilibriumTestCaseWithStart(
            factory=functools.partial(games.read_from_file, "stripped_down_poker.efg"),
            solver=gbt.nash.liap_solve,
            start_data=dict(data=None, rational=False),
            expected=[],
            regret_tol=TOL_LARGE,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_liap_strategy,
        id="test_liap_strategy_1",
    ),
]


SIMPDIV_CASES = [
    pytest.param(
        EquilibriumTestCaseWithStart(
            factory=functools.partial(games.read_from_file, "stripped_down_poker.efg"),
            solver=functools.partial(gbt.nash.simpdiv_solve),
            start_data=dict(data=None, rational=True),
            expected=[
                [
                    d(Q(174763, 524288), Q(349525, 524288), 0, 0),
                    d(Q(699051, 1048576), Q(349525, 1048576)),
                ]
            ],
            regret_tol=TOL_LARGE,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_simpdiv,
        id="test_simpdiv_1",
    ),
]

CASES = []
CASES += LIAP_STRATEGY_CASES
CASES += SIMPDIV_CASES


@pytest.mark.nash
@pytest.mark.parametrize("test_case", CASES, ids=lambda c: c.label)
def test_nash_strategy_solver_w_start(test_case: EquilibriumTestCaseWithStart, subtests) -> None:
    """Test calls of Nash solvers that start a starting profile.

    Subtests:
    - Max regret no more than `test_case.regret_tol`
    - Equilibria are output in the expected order.  Equilibria are deemed to match if the maximum
      difference in probabilities is no more than `test_case.prob_tol`
    """
    game = test_case.factory()
    start = game.mixed_strategy_profile(**test_case.start_data)
    result = test_case.solver(start)
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
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file,
                "2_player_PI_2_dev_off_eq_path_const_sum.efg",
            ),
            solver=gbt.nash.lp_solve,
            expected=[
                [[d(0, 1), d("1/5", "1/5", "1/5", "1/5", "1/5")], [d(0, 1)]],
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_lp_behavior,
        id="test_lp_behavior_rational_17",
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
    # pytest.param(
    # EquilibriumTestCase(
    # factory=functools.partial(games.read_from_file, "large_payoff_game.efg"),
    # solver=functools.partial(gbt.nash.lp_solve, rational=False),
    # expected=[
    # [
    # [d(1, 0), d(1, 0)],
    # [
    # d(0, 1),
    # d("9999999999999999999/10000000000000000000", "1/10000000000000000000"),
    # ],
    # ]
    # ],
    # regret_tol=TOL,
    # prob_tol=TOL,
    # ),
    # marks=[
    # pytest.mark.nash_lp_behavior,
    # pytest.mark.xfail(reason="Problem with large payoffs when working in floats"),
    # ],
    # id="test_lp_behavior_double_14",
    # ),
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
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file,
                "2_player_PI_2_dev_off_eq_path_const_sum.efg",
            ),
            solver=gbt.nash.lcp_solve,
            expected=[
                [[d(0, 1), d("1/5", "1/5", "1/5", "1/5", "1/5")], [d(0, 1)]],
            ],
        ),
        marks=pytest.mark.nash_lcp_behavior,
        id="test_lcp_behavior_rational_23",
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
    # pytest.param(
    # EquilibriumTestCase(
    # factory=functools.partial(games.read_from_file, "large_payoff_game.efg"),
    # solver=functools.partial(gbt.nash.lcp_solve, rational=False),
    # expected=[
    # [
    # [d(1, 0), d(1, 0)],
    # [
    # d(0, 1),
    # d("9999999999999999999/10000000000000000000", "1/10000000000000000000"),
    # ],
    # ]
    # ],
    # regret_tol=TOL,
    # prob_tol=TOL,
    # ),
    # marks=[
    # pytest.mark.nash_lcp_behavior,
    # pytest.mark.xfail(reason="Problem with large payoffs when working in floats"),
    # ],
    # id="test_lcp_behavior_double_12",
    # ),
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
        id="test_enumpoly_behavior_1",
    ),
    # 2-player non-zero-sum games
    pytest.param(
        EquilibriumTestCase(
            factory=games.create_one_shot_trust_efg,
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=None),
            expected=[
                [[d(0, 1)], [d(0, 1)]],
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_enumpoly_behavior,
        id="test_enumpoly_behavior_2",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_one_shot_trust_efg, unique_NE_variant=True),
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=None),
            expected=[
                [[d(1, 0)], [d(0, 1)]],
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_enumpoly_behavior,
        id="test_enumpoly_behavior_3",
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
        id="test_enumpoly_behavior_4",
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
        id="test_enumpoly_behavior_5",
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
        id="test_enumpoly_behavior_6",
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
        id="test_enumpoly_behavior_7",
    ),
    # 3x3 coordination game
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
        id="test_enumpoly_behavior_8",
    ),
    # 4x4 coordination game
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.create_EFG_for_nxn_bimatrix_coordination_game, n=4),
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=None),
            expected=[
                [[d("1/4", "1/4", "1/4", "1/4")], [d("1/4", "1/4", "1/4", "1/4")]],
                [[d("1/3", "1/3", "1/3", 0)], [d("1/3", "1/3", "1/3", 0)]],
                [[d("1/3", "1/3", 0, "1/3")], [d("1/3", "1/3", 0, "1/3")]],
                [[d("1/2", "1/2", 0, 0)], [d("1/2", "1/2", 0, 0)]],
                [[d("1/3", 0, "1/3", "1/3")], [d("1/3", 0, "1/3", "1/3")]],
                [[d("1/2", 0, "1/2", 0)], [d("1/2", 0, "1/2", 0)]],
                [[d("1/2", 0, 0, "1/2")], [d("1/2", 0, 0, "1/2")]],
                [[d(1, 0, 0, 0)], [d(1, 0, 0, 0)]],
                [[d(0, "1/3", "1/3", "1/3")], [d(0, "1/3", "1/3", "1/3")]],
                [[d(0, "1/2", "1/2", 0)], [d(0, "1/2", "1/2", 0)]],
                [[d(0, "1/2", 0, "1/2")], [d(0, "1/2", 0, "1/2")]],
                [[d(0, 1, 0, 0)], [d(0, 1, 0, 0)]],
                [[d(0, 0, "1/2", "1/2")], [d(0, 0, "1/2", "1/2")]],
                [[d(0, 0, 1, 0)], [d(0, 0, 1, 0)]],
                [[d(0, 0, 0, 1)], [d(0, 0, 0, 1)]],
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_enumpoly_behavior,
        id="test_enumpoly_behavior_9",
    ),
    # Figure 10.5 efg from Game Theory Basics
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(gbt.catalog.load, "books/vonstengel2022/fig10.5"),
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=None),
            expected=[
                [[d("1/2", "1/2")], [d("2/5", "3/5"), d(0, 1), d(1, 0)]],
                [[d("1/4", "3/4")], [d(0, 1), d(1, 0), d("1/3", "2/3")]],
                [[d(1, 0)], [d(1, 0), d(1, 0), d(1, 0)]],
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_enumpoly_behavior,
        id="test_enumpoly_behavior_10",
    ),
    # 6x6 bimatrix game with 75 equilibria -- runs slow so only do first 2 equilibria
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.create_efg_corresponding_to_bimatrix_game,
                gbt.catalog.load("journals/dcg/vonstengel1999/6x6_game_with_75_eq"),
            ),
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=2),
            expected=[
                [
                    [d("132/245", "66/245", "6/49", "12/245", "4/245", "1/245")],
                    [d("4/245", "1/245", "6/49", "12/245", "132/245", "66/245")],
                ],
                [
                    [d("5775/9871", "2640/9871", "1050/9871", "336/9871", "70/9871", 0)],
                    [d("70/9871", 0, "1050/9871", "336/9871", "5775/9871", "2640/9871")],
                ],
            ],
            regret_tol=TOL_LARGE,
            prob_tol=TOL_LARGE,
        ),
        marks=pytest.mark.nash_enumpoly_behavior,
        id="test_enumpoly_behavior_11",
    ),
    ########################################################################
    # 3-player games
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "3_player.efg"),
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=None),
            expected=[
                [[d(1, 0), d(1, 0)], [d(1, 0), d(1, 0)], [d(1, 0), d(1, 0)]],
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_enumpoly_behavior,
        id="test_enumpoly_behavior_12",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "3_player_with_nonterm_outcomes.efg"),
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=None),
            expected=[
                [[d(1, 0), d(1, 0)], [d(1, 0), d(1, 0)], [d(1, 0), d(1, 0)]],
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_enumpoly_behavior,
        id="test_enumpoly_behavior_13",
    ),
    # 3-player perfect info game to test behavior two off equilibrium path
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "3_player_PI_2_dev_off_eq_path.efg"),
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=None),
            expected=[
                # candidate,10,10,1000,10000
                [[d(1, 0)], [d(1, 0), d(1, 0, 0, 0)], [d(1, 0, 0, 0, 0)]],
                # candidate,01,00,0000,00000
                [[d(0, 1)], [d(1, 0), d(1, 0, 0, 0)], [d(1, 0, 0, 0, 0)]],
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_enumpoly_behavior,
        id="test_enumpoly_behavior_14",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "3_player_PI_2_dev_off_eq_path.efg"),
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=None),
            expected=[
                [[d(1, 0)], [d(1, 0), d(1, 0, 0, 0)], [d(1, 0, 0, 0, 0)]],
                [
                    [d(0, 1)],
                    [d(1, 0), d(1, 0, 0, 0)],
                    [d(1, 0, 0, 0, 0)],
                ],
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_enumpoly_behavior,
        id="test_enumpoly_behavior_15",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file,
                "3_player_PI_2_dev_off_eq_path_strict_dom_for_p1.efg",
            ),
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=None),
            expected=[
                [
                    [d(0, 1)],
                    [d(1, 0), d(1, 0, 0, 0)],
                    [d(1, 0, 0, 0, 0)],
                ],
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_enumpoly_behavior,
        id="test_enumpoly_behavior_16",
    ),
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file,
                "2_player_PI_2_dev_off_eq_path_const_sum.efg",
            ),
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=None),
            expected=[
                [[d(0, 1), d(1, 0, 0, 0, 0)], [d(1, 0)]],
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_enumpoly_behavior,
        id="test_enumpoly_behavior_17",
    ),
    # 3-player game equivalent to a simultaneous-move 2x2x2 game (nine equilibria total,
    # including two totally-mixed ones); this previously required an unordered check due
    # to run-to-run variation in the order equilibria were found (see #589)
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(games.read_from_file, "mixed_behavior_game.efg"),
            solver=functools.partial(gbt.nash.enumpoly_solve, stop_after=None),
            expected=[
                [[d("1/2", "1/2")], [d("2/5", "3/5")], [d("1/4", "3/4")]],
                [[d("2/5", "3/5")], [d("1/2", "1/2")], [d("1/3", "2/3")]],
                [[d("1/2", "1/2")], [d("1/2", "1/2")], [d(1, 0)]],
                [[d("1/3", "2/3")], [d(1, 0)], [d("1/4", "3/4")]],
                [[d(1, 0)], [d(1, 0)], [d(1, 0)]],
                [[d(1, 0)], [d(0, 1)], [d(0, 1)]],
                [[d(0, 1)], [d("1/4", "3/4")], [d("1/3", "2/3")]],
                [[d(0, 1)], [d(1, 0)], [d(0, 1)]],
                [[d(0, 1)], [d(0, 1)], [d(1, 0)]],
            ],
            regret_tol=TOL,
            prob_tol=TOL,
        ),
        marks=pytest.mark.nash_enumpoly_behavior,
        id="test_enumpoly_behavior_18",
    ),
]


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
    pytest.param(
        EquilibriumTestCase(
            factory=functools.partial(
                games.read_from_file, "chance_root_5_moves_no_nonterm_player_nodes.efg"
            ),
            solver=gbt.nash.logit_solve,
            expected=[
                [[]]  # Zero-dimension edge case (two players)
            ],
            regret_tol=TOL_LARGE,
            prob_tol=TOL_LARGE,
        ),
        marks=pytest.mark.nash_logit_behavior,
        id="test_logit_behavior_degenerate",
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
            factory=functools.partial(gbt.catalog.load, "books/myerson1991/fig4_2"),
            solver=functools.partial(gbt.nash.enumpure_agent_solve),
            expected=[[[d(1, 0), d(0, 1)], [d(0, 1)]], [[d(0, 1), d(0, 1)], [d(1, 0)]]],
        ),
        marks=pytest.mark.nash_enumpure_strategy,
        id="test_enumpure_agent_8",
    ),
]


AGENT_CASES = []
AGENT_CASES += ENUMPURE_AGENT_CASES


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
# AGENTS NASH SOLVERS WITH START PROFILE
##################################################################################################


LIAP_AGENT_CASES = [
    pytest.param(
        EquilibriumTestCaseWithStart(
            factory=functools.partial(games.read_from_file, "stripped_down_poker.efg"),
            solver=gbt.nash.liap_agent_solve,
            start_data=dict(data=None, rational=False),
            expected=[[[d(1, 0), d("1/3", "2/3")], [d("2/3", "1/3")]]],
            regret_tol=TOL_LARGE,
            prob_tol=TOL_LARGE,
        ),
        marks=pytest.mark.nash_liap_agent,
        id="test_liap_agent_1",
    ),
]

AGENT_WITH_START_CASES = []
AGENT_WITH_START_CASES += LIAP_AGENT_CASES


@pytest.mark.nash
@pytest.mark.parametrize("test_case", AGENT_WITH_START_CASES, ids=lambda c: c.label)
def test_nash_agent_w_start_solver(test_case: EquilibriumTestCase, subtests) -> None:
    """Test calls of Nash solvers with starting profile in EFGs using "agent" versions.

    Subtests:
    - Agent max regret no more than `test_case.regret_tol`
    - Agent max regret no more than max regret (+ `test_case.regret_tol`)
    - Equilibria are output in the expected order.  Equilibria are deemed to match if the maximum
      difference in probabilities is no more than `test_case.prob_tol`
    """
    game = test_case.factory()
    start = game.mixed_behavior_profile(**test_case.start_data)
    result = test_case.solver(start)
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


def test_enumpoly_solve_error_with_invalid_stop_after():
    game = games.read_from_file("const_sum_game.nfg")
    with pytest.raises(ValueError, match="must be a positive integer"):
        gbt.nash.enumpoly_solve(game, stop_after=0)
    with pytest.raises(ValueError, match="must be a positive integer"):
        gbt.nash.enumpoly_solve(game, stop_after=-1)


def test_lcp_solve_error_with_invalid_stop_after():
    game = games.read_from_file("const_sum_game.nfg")
    with pytest.raises(ValueError, match="must be a positive integer"):
        gbt.nash.lcp_solve(game, stop_after=0)
    with pytest.raises(ValueError, match="must be a positive integer"):
        gbt.nash.lcp_solve(game, stop_after=-1)


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
