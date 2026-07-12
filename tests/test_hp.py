"""Test of calls to the Herings & Peeters (2001) homotopy solver."""

import dataclasses
import typing

import numpy as np
import pytest

import pygambit as gbt

TOL = 1e-6


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

    # El prior que descubriste accidentalmente
    prior[list(p1.strategies)[0]] = 2.0 / 3.0
    prior[list(p1.strategies)[1]] = 1.0 / 3.0
    prior[list(p2.strategies)[0]] = 1.0 / 3.0
    prior[list(p2.strategies)[1]] = 2.0 / 3.0

    return prior


HP_CASES = [
    pytest.param(
        HPSolverTestCase(
            factory=create_hp_paper_example,
            expected=[d(0.8, 0.2), d(1.0 / 3.0, 2.0 / 3.0)],
        ),
        marks=pytest.mark.xfail(reason="Mathematical curve tracking not yet fully implemented"),
        id="test_hp_herings_peeters_example",
    ),
    pytest.param(
        HPSolverTestCase(
            factory=create_hs_example_1,
            expected=[d(0.0, 1.0), d(0.0, 1.0)],
        ),
        marks=pytest.mark.xfail(reason="Mathematical curve tracking not yet fully implemented"),
        id="test_hp_hs_example_1",
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
