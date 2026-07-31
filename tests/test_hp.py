"""Test of calls to the Herings & Peeters (2001) homotopy solver."""

import dataclasses
import functools
import typing

import numpy as np
import pytest

import pygambit as gbt

from . import games

TOL = 1e-8


def d(*probs) -> tuple:
    """Helper function to let us write d() to be suggestive of
    "probability distribution on simplex" ("Delta")
    """
    return tuple(probs)


@dataclasses.dataclass
class HPSolverTestCase:
    """Summarising the data relevant for a test fixture of a call to the HP solver."""
    factory: typing.Callable[[], gbt.MixedStrategyProfileDouble]
    expected: list
    prob_tol: float = TOL


def create_prior_from_catalog(game_path: str) -> gbt.MixedStrategyProfile:
    """Loads a game from the catalog and returns a prior that is uniform over all strategies."""
    game = gbt.catalog.load(game_path)
    return game.mixed_strategy_profile()


def load_game_from_file(file_name: str) -> gbt.MixedStrategyProfile:
    """Loads a game from a file in tests/test_games"""
    return games.read_from_file(file_name)


def check_equilibrium(result, subtests) -> None:
    """Checks that the result of the HP solver is a valid Nash equilibrium."""
    with subtests.test("number of equilibria found"):
        assert len(result.equilibria) == 1

    with subtests.test("valid Nash equilibrium (max_regret <= TOL)"):
        assert result.equilibria[0].max_regret() <= TOL


def create_hs_base_game() -> gbt.Game:
    """Creates the base 2x2 game used in all examples from Harsanyi & Selten (1988) Section 4.11
    and also featured in Herings & Peeters (2001).
    """
    p1_payoffs = np.array([[2, 0], [0, 1]])
    p2_payoffs = np.array([[1, 0], [0, 4]])
    return gbt.Game.from_arrays(p1_payoffs, p2_payoffs, title="HS 1988 Base Game")


def create_hp_paper_example() -> gbt.MixedStrategyProfileDouble:
    """Creates the example from Herings & Peeters (2001) Figure 1.
    Also used in Harsanyi & Selten (1988) Section 4.11. -Second Example."""
    game = create_hs_base_game()
    prior = game.mixed_strategy_profile()
    p1, p2 = list(game.players)

    prior[list(p1.strategies)[0]] = 0.5
    prior[list(p1.strategies)[1]] = 0.5
    prior[list(p2.strategies)[0]] = 2.0 / 3.0
    prior[list(p2.strategies)[1]] = 1.0 / 3.0

    return prior


def create_hs_example_1() -> gbt.MixedStrategyProfileDouble:
    """Harsanyi & Selten (1988) Section 4.11 - First Example."""
    game = create_hs_base_game()
    prior = game.mixed_strategy_profile()
    p1, p2 = list(game.players)

    prior[list(p1.strategies)[0]] = 1.0 / 3.0
    prior[list(p1.strategies)[1]] = 2.0 / 3.0
    prior[list(p2.strategies)[0]] = 1.0 / 6.0
    prior[list(p2.strategies)[1]] = 5.0 / 6.0

    return prior


def create_t0_degenerate_example() -> gbt.MixedStrategyProfileDouble:
    """A prior that causes multiple best responses exactly at t=0."""
    game = create_hs_base_game()
    prior = game.mixed_strategy_profile()
    p1, p2 = list(game.players)

    prior[list(p1.strategies)[0]] = 2.0 / 3.0
    prior[list(p1.strategies)[1]] = 1.0 / 3.0
    prior[list(p2.strategies)[0]] = 1.0 / 3.0
    prior[list(p2.strategies)[1]] = 2.0 / 3.0

    return prior


def create_test_hp_2x2x2_nfg_eq1() -> gbt.MixedStrategyProfileDouble:
    """Creates a prior for the 2x2x2.nfg game that is exactly the first equilibrium."""
    game = load_game_from_file("../../contrib/games/2x2x2.nfg")
    return game.mixed_strategy_profile(data=[(1.0, 0.0), (1.0, 0.0), (1.0, 0.0)])


def create_test_hp_2x2x2_nfg_eq1_perturbed() -> gbt.MixedStrategyProfileDouble:
    """Creates a prior for the 2x2x2.nfg game that is the first equilibrium perturbed."""
    game = load_game_from_file("../../contrib/games/2x2x2.nfg")
    return game.mixed_strategy_profile(data=[(0.9, 0.1), (0.9, 0.1), (0.9, 0.1)])


def create_test_hp_2x2x2_nfg_eq2() -> gbt.MixedStrategyProfileDouble:
    """Creates a prior for the 2x2x2.nfg game that is exactly the second pure equilibrium."""
    game = load_game_from_file("../../contrib/games/2x2x2.nfg")
    return game.mixed_strategy_profile(data=[(1.0, 0.0), (0.0, 1.0), (0.0, 1.0)])


def create_test_hp_2x2x2_nfg_eq2_perturbed() -> gbt.MixedStrategyProfileDouble:
    """Creates a prior for the 2x2x2.nfg game that is the second equilibrium perturbed."""
    game = load_game_from_file("../../contrib/games/2x2x2.nfg")
    return game.mixed_strategy_profile(data=[(0.9, 0.1), (0.1, 0.9), (0.1, 0.9)])


def create_test_hp_2x2x2_nfg_eq3() -> gbt.MixedStrategyProfileDouble:
    """Creates a prior for the 2x2x2.nfg game that is exactly the third pure equilibrium."""
    game = load_game_from_file("../../contrib/games/2x2x2.nfg")
    return game.mixed_strategy_profile(data=[(0.0, 1.0), (1.0, 0.0), (0.0, 1.0)])


def create_test_hp_2x2x2_nfg_eq3_perturbed() -> gbt.MixedStrategyProfileDouble:
    """Creates a prior for the 2x2x2.nfg game that is the third equilibrium perturbed."""
    game = load_game_from_file("../../contrib/games/2x2x2.nfg")
    return game.mixed_strategy_profile(data=[(0.1, 0.9), (0.9, 0.1), (0.1, 0.9)])


def create_test_hp_2x2x2_nfg_eq4() -> gbt.MixedStrategyProfileDouble:
    """Creates a prior for the 2x2x2.nfg game that is exactly the fourth pure equilibrium."""
    game = load_game_from_file("../../contrib/games/2x2x2.nfg")
    return game.mixed_strategy_profile(data=[(0.0, 1.0), (0.0, 1.0), (1.0, 0.0)])


def create_test_hp_2x2x2_nfg_eq4_perturbed() -> gbt.MixedStrategyProfileDouble:
    """Creates a prior for the 2x2x2.nfg game that is the fourth equilibrium perturbed."""
    game = load_game_from_file("../../contrib/games/2x2x2.nfg")
    return game.mixed_strategy_profile(data=[(0.1, 0.9), (0.1, 0.9), (0.9, 0.1)])


def create_test_hp_2x2x2_nfg_eq5_perturbed() -> gbt.MixedStrategyProfileDouble:
    """Creates a perturbed prior near the fifth equilibrium (partially mixed)."""
    game = load_game_from_file("../../contrib/games/2x2x2.nfg")
    return game.mixed_strategy_profile(data=[(0.51, 0.49), (0.49, 0.51), (0.9, 0.1)])


def create_test_hp_2x2x2_nfg_eq8_perturbed() -> gbt.MixedStrategyProfileDouble:
    """Creates a perturbed prior near the eighth equilibrium (totally mixed)."""
    game = load_game_from_file("../../contrib/games/2x2x2.nfg")
    return game.mixed_strategy_profile(data=[(0.51, 0.49), (0.41, 0.59), (0.26, 0.74)])


def create_test_hp_2x2x2_nfg_eq9_perturbed() -> gbt.MixedStrategyProfileDouble:
    """Creates a perturbed prior near the ninth equilibrium (totally mixed)."""
    game = load_game_from_file("../../contrib/games/2x2x2.nfg")
    return game.mixed_strategy_profile(data=[(0.41, 0.59), (0.51, 0.49), (0.34, 0.66)])


HP_CASES = [
    pytest.param(
        HPSolverTestCase(
            factory=create_hp_paper_example,
            expected=[d(0.0, 1.0), d(0.0, 1.0)],
        ),
        id="test_hp_herings_peeters_example",
    ),
    pytest.param(
        HPSolverTestCase(
            factory=create_hs_example_1,
            expected=[d(0.0, 1.0), d(0.0, 1.0)],
        ),
        id="test_hp_hs_example_1",
    ),
    pytest.param(
        HPSolverTestCase(
            factory=functools.partial(create_prior_from_catalog, "books/myerson1991/fig2_1"),
            expected=[d(1.0 / 3.0, 2.0 / 3.0, 0.0, 0.0), d(2.0 / 3.0, 1.0 / 3.0)],
        ),
        id="test_hp_myerson1991_fig2_1",
    ),
    pytest.param(
        HPSolverTestCase(
            factory=functools.partial(create_prior_from_catalog, "books/vonstengel2022/fig10.1"),
            expected=[d(0.0, 0.5, 0, 0.5), d(1.0 / 4.0, 3.0 / 4.0)],
        ),
        id="test_hp_vonstengel2022_fig10.1",
    ),
    pytest.param(
        HPSolverTestCase(
            factory=functools.partial(create_prior_from_catalog, "journals/ijgt/nau2004/sec4"),
            expected=[
                d(0.6192325794725538, 0.38076742052744617),
                d(0.4798042226776052, 0.5201957773223949),
                d(0.37882533606563146, 0.6211746639343685)
            ],
        ),
        id="test_hp_nau2004_sec4",
    ),
    pytest.param(
        HPSolverTestCase(
            factory=functools.partial(create_prior_from_catalog, "journals/other/reiley2008/fig1"),
            expected=[d(1.0 / 3.0, 2.0 / 3.0, 0.0, 0.0), d(2.0 / 3.0, 1.0 / 3.0)],
        ),
        id="test_hp_reiley2008_fig1",
    ),
    pytest.param(
        HPSolverTestCase(
            factory=create_test_hp_2x2x2_nfg_eq1,
            expected=[d(1.0, 0.0), d(1.0, 0.0), d(1.0, 0.0)],
        ),
        id="test_hp_2x2x2_nfg_eq1",
    ),
    pytest.param(
        HPSolverTestCase(
            factory=create_test_hp_2x2x2_nfg_eq1_perturbed,
            expected=[d(1.0, 0.0), d(1.0, 0.0), d(1.0, 0.0)],
        ),
        id="test_hp_2x2x2_nfg_eq1_perturbed",
    ),
    pytest.param(
        HPSolverTestCase(
            factory=create_test_hp_2x2x2_nfg_eq2,
            expected=[d(1.0, 0.0), d(0.0, 1.0), d(0.0, 1.0)],
        ),
        id="test_hp_2x2x2_nfg_eq2",
    ),
    pytest.param(
        HPSolverTestCase(
            factory=create_test_hp_2x2x2_nfg_eq2_perturbed,
            expected=[d(1.0, 0.0), d(0.0, 1.0), d(0.0, 1.0)],
        ),
        id="test_hp_2x2x2_nfg_eq2_perturbed",
    ),
    pytest.param(
        HPSolverTestCase(
            factory=create_test_hp_2x2x2_nfg_eq3,
            expected=[d(0.0, 1.0), d(1.0, 0.0), d(0.0, 1.0)],
        ),
        id="test_hp_2x2x2_nfg_eq3",
    ),
    pytest.param(
        HPSolverTestCase(
            factory=create_test_hp_2x2x2_nfg_eq3_perturbed,
            expected=[d(0.0, 1.0), d(1.0, 0.0), d(0.0, 1.0)],
        ),
        id="test_hp_2x2x2_nfg_eq3_perturbed",
    ),
    pytest.param(
        HPSolverTestCase(
            factory=create_test_hp_2x2x2_nfg_eq4,
            expected=[d(0.0, 1.0), d(0.0, 1.0), d(1.0, 0.0)],
        ),
        id="test_hp_2x2x2_nfg_eq4",
    ),
    pytest.param(
        HPSolverTestCase(
            factory=create_test_hp_2x2x2_nfg_eq4_perturbed,
            expected=[d(0.0, 1.0), d(0.0, 1.0), d(1.0, 0.0)],
        ),
        id="test_hp_2x2x2_nfg_eq4_perturbed",
    ),
]


@pytest.mark.nash
@pytest.mark.parametrize("test_case", HP_CASES)
def test_hp_strategy_solver(test_case: HPSolverTestCase, subtests) -> None:
    """Test calls of the HP solver with starting priors.

    Subtests:
    - Number of equilibria found is exactly 1.
    - Equilibrium profile matches the expected theoretical result.
    """
    prior = test_case.factory()
    game = prior.game

    result = gbt.nash.hp_solve(prior=prior)

    with subtests.test("number of equilibria found"):
        # The HP method uniquely selects exactly 1 equilibrium.
        assert len(result.equilibria) == 1

    eq = result.equilibria[0]
    expected = game.mixed_strategy_profile(rational=False, data=test_case.expected)

    with subtests.test("strategy_profile matches expected"):
        for player in game.players:
            for strategy in player.strategies:
                assert abs(eq[strategy] - expected[strategy]) <= test_case.prob_tol


@pytest.mark.nash
def test_hp_degenerate_t0_prior_raises_error() -> None:
    """Test that the HP solver correctly identifies when given a degenerate prior."""
    prior = create_t0_degenerate_example()
    with pytest.raises(RuntimeError, match="Multiple best responses found for player 1. "
                       "Only one best response is allowed."):
        gbt.nash.hp_solve(prior=prior)


@pytest.mark.nash
def test_hp_degenerate_t0_prior_selten1975_fig2() -> None:
    game = gbt.catalog.load("journals/ijgt/selten1975/fig2")
    prior = game.mixed_strategy_profile()
    with pytest.raises(RuntimeError, match="Multiple best responses found for player 1. "
                       "Only one best response is allowed."):
        gbt.nash.hp_solve(prior=prior)


@pytest.mark.nash
def test_hp_2x2x2_nfg_eq5_perturbed(subtests) -> None:
    """Test convergence from a perturbed prior near eq 5 using max regret."""
    prior = create_test_hp_2x2x2_nfg_eq5_perturbed()
    result = gbt.nash.hp_solve(prior=prior)
    check_equilibrium(result, subtests)


@pytest.mark.nash
def test_hp_2x2x2_nfg_eq8_perturbed(subtests) -> None:
    """Test convergence from a perturbed prior near eq 8 using max regret."""
    prior = create_test_hp_2x2x2_nfg_eq8_perturbed()
    result = gbt.nash.hp_solve(prior=prior)
    check_equilibrium(result, subtests)


@pytest.mark.nash
def test_hp_2x2x2_nfg_eq9_perturbed(subtests) -> None:
    """Test convergence from a perturbed prior near eq 9 using max regret."""
    prior = create_test_hp_2x2x2_nfg_eq9_perturbed()
    result = gbt.nash.hp_solve(prior=prior)
    check_equilibrium(result, subtests)


CATALOG_GAMES_TO_TEST = [
    pytest.param(
        "books/myerson1991/fig2_1",
        id="catalog games that converge to a NE - books/myerson1991/fig2_1"
    ),
    pytest.param(
        "books/shohamleytonbrown2008/fig5_10",
        id="catalog games that converge to a NE - books/shohamleytonbrown2008/fig5_10"
    ),
    pytest.param(
        "books/shohamleytonbrown2008/fig5_11",
        id="catalog games that converge to a NE - books/shohamleytonbrown2008/fig5_11"
    ),
    pytest.param(
        "books/shohamleytonbrown2008/fig5_15",
        id="catalog games that converge to a NE - books/shohamleytonbrown2008/fig5_15"
    ),
    pytest.param(
        "books/shohamleytonbrown2008/fig6_2",
        id="catalog games that converge to a NE - books/shohamleytonbrown2008/fig6_2"
    ),
    pytest.param(
        "books/shohamleytonbrown2008/fig6_8",
        id="catalog games that converge to a NE - books/shohamleytonbrown2008/fig6_8"
    ),
    pytest.param(
        "books/vonstengel2022/fig10.1",
        id="catalog games that converge to a NE - books/vonstengel2022/fig10.1"
    ),
    pytest.param(
        "books/vonstengel2022/fig10.12",
        id="catalog games that converge to a NE - books/vonstengel2022/fig10.12"
    ),
    pytest.param(
        "books/watson2013/fig29_1",
        id="catalog games that converge to a NE - books/watson2013/fig29_1"
    ),
    pytest.param(
        "conf/itcs/jakobsen2016/fig1c",
        id="catalog games that converge to a NE - conf/itcs/jakobsen2016/fig1c"
    ),
    pytest.param(
        "conf/itcs/jakobsen2016/fig3",
        id="catalog games that converge to a NE - conf/itcs/jakobsen2016/fig3"
    ),
    pytest.param(
        "journals/geb/bagwell1995",
        id="catalog games that converge to a NE - journals/geb/bagwell1995"
    ),
    pytest.param(
        "journals/ijgt/nau2004/sec3",
        id="catalog games that converge to a NE - journals/ijgt/nau2004/sec3"
    ),
    pytest.param(
        "journals/ijgt/nau2004/sec4",
        id="catalog games that converge to a NE - journals/ijgt/nau2004/sec4"
    ),
    pytest.param(
        "journals/ijgt/nau2004/sec5",
        id="catalog games that converge to a NE - journals/ijgt/nau2004/sec5"
    ),
    pytest.param(
        "journals/other/reiley2008/fig1",
        id="catalog games that converge to a NE - journals/other/reiley2008/fig1"
    ),
]


@pytest.mark.nash
@pytest.mark.parametrize("game_id", CATALOG_GAMES_TO_TEST)
def test_hp_catalog_games_max_regret(game_id, subtests) -> None:
    """Some catalog games that converge to a Nash equilibrium with a 1e-8 tolerance."""
    game = gbt.catalog.load(game_id)
    prior = game.mixed_strategy_profile()
    result = gbt.nash.hp_solve(prior=prior)
    check_equilibrium(result, subtests)


LOCAL_GAMES_TO_TEST = [
    pytest.param(
        "8x8.nfg",
        id="contrib games that converge to a NE - 8x8.nfg"
    ),
    pytest.param(
        "5x4x3.nfg",
        id="contrib games that converge to a NE - 5x4x3.nfg"
    ),
    pytest.param(
        "3x3x3.nfg",
        marks=pytest.mark.xfail(reason="t goes to negative values"),
        id="contrib games that converge to a NE - 3x3x3.nfg"
    ),
    pytest.param(
        "2x2x2x2x2.nfg",
        id="contrib games that converge to a NE - 2x2x2x2x2.nfg"
    ),
    pytest.param(
        "8x2x2.nfg",
        id="contrib games that converge to a NE - 8x2x2.nfg"
    ),
    pytest.param(
        "pd.nfg",
        id="contrib games that converge to a NE - pd.nfg"
    ),
    pytest.param(
        "2x2x2x2.nfg",
        id="contrib games that converge to a NE - 2x2x2x2.nfg"
    ),
]


@pytest.mark.nash
@pytest.mark.parametrize("filename", LOCAL_GAMES_TO_TEST)
def test_hp_large_local_games_max_regret(filename, subtests) -> None:
    """Some contrib/games that converge to a Nash equilibrium with a 1e-8 tolerance."""
    game = load_game_from_file(f"../../contrib/games/{filename}")
    prior = game.mixed_strategy_profile()
    result = gbt.nash.hp_solve(prior=prior)
    check_equilibrium(result, subtests)
