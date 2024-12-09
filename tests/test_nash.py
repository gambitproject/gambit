"""Test of calls to Nash equilibrium solvers.

These tests primarily ensure that calling the solvers works and returns
expected results on a very simple game.  This is not intended to be a
rigorous test suite for the algorithms across all games.
"""

import pytest

import pygambit as gbt

from . import games


def test_enumpure_strategy():
    """Test calls of enumeration of pure strategies."""
    game = games.read_from_file("poker.efg")
    assert len(gbt.nash.enumpure_solve(game, use_strategic=True).equilibria) == 0


def test_enumpure_agent():
    """Test calls of enumeration of pure agent strategies."""
    game = games.read_from_file("poker.efg")
    assert len(gbt.nash.enumpure_solve(game, use_strategic=False).equilibria) == 0


def test_enummixed_strategy_double():
    """Test calls of enumeration of mixed strategy equilibria, floating-point."""
    game = games.read_from_file("poker.efg")
    result = gbt.nash.enummixed_solve(game, rational=False)
    assert len(result.equilibria) == 1
    # For floating-point results are not exact, so we skip testing exact values for now


def test_enummixed_strategy_rational():
    """Test calls of enumeration of mixed strategy equilibria, rational precision."""
    game = games.read_from_file("poker.efg")
    result = gbt.nash.enummixed_solve(game, rational=True)
    assert len(result.equilibria) == 1
    expected = game.mixed_strategy_profile(
        rational=True,
        data=[[gbt.Rational(1, 3), gbt.Rational(2, 3), gbt.Rational(0), gbt.Rational(0)],
              [gbt.Rational(2, 3), gbt.Rational(1, 3)]]
    )
    assert result.equilibria[0] == expected


def test_lcp_strategy_double():
    """Test calls of LCP for mixed strategy equilibria, floating-point."""
    game = games.read_from_file("poker.efg")
    result = gbt.nash.lcp_solve(game, use_strategic=True, rational=False)
    assert len(result.equilibria) == 1
    # For floating-point results are not exact, so we skip testing exact values for now


def test_lcp_strategy_rational():
    """Test calls of LCP for mixed strategy equilibria, rational precision."""
    game = games.read_from_file("poker.efg")
    result = gbt.nash.lcp_solve(game, use_strategic=True, rational=True)
    assert len(result.equilibria) == 1
    expected = game.mixed_strategy_profile(
        rational=True,
        data=[[gbt.Rational(1, 3), gbt.Rational(2, 3), gbt.Rational(0), gbt.Rational(0)],
              [gbt.Rational(2, 3), gbt.Rational(1, 3)]]
    )
    assert result.equilibria[0] == expected


def test_lcp_behavior_double():
    """Test calls of LCP for mixed behavior equilibria, floating-point."""
    game = games.read_from_file("poker.efg")
    result = gbt.nash.lcp_solve(game, use_strategic=False, rational=False)
    assert len(result.equilibria) == 1
    # For floating-point results are not exact, so we skip testing exact values for now


def test_lcp_behavior_rational():
    """Test calls of LCP for mixed behavior equilibria, rational precision."""
    game = games.read_from_file("poker.efg")
    result = gbt.nash.lcp_solve(game, use_strategic=False, rational=True)
    assert len(result.equilibria) == 1
    expected = game.mixed_behavior_profile(rational=True,
                                           data=[[[1, 0],
                                                  [gbt.Rational("1/3"), gbt.Rational("2/3")]],
                                                 [[gbt.Rational("2/3"), gbt.Rational("1/3")]]])
    assert result.equilibria[0] == expected


def test_lp_strategy_double():
    """Test calls of LP for mixed strategy equilibria, floating-point."""
    game = games.read_from_file("poker.efg")
    result = gbt.nash.lp_solve(game, use_strategic=True, rational=False)
    assert len(result.equilibria) == 1
    # For floating-point results are not exact, so we skip testing exact values for now


def test_lp_strategy_rational():
    """Test calls of LP for mixed strategy equilibria, rational precision."""
    game = games.read_from_file("poker.efg")
    result = gbt.nash.lp_solve(game, use_strategic=True, rational=True)
    assert len(result.equilibria) == 1
    expected = game.mixed_strategy_profile(
        rational=True,
        data=[[gbt.Rational(1, 3), gbt.Rational(2, 3), gbt.Rational(0), gbt.Rational(0)],
              [gbt.Rational(2, 3), gbt.Rational(1, 3)]]
    )
    assert result.equilibria[0] == expected


def test_lp_behavior_double():
    """Test calls of LP for mixed behavior equilibria, floating-point."""
    game = games.read_from_file("poker.efg")
    result = gbt.nash.lp_solve(game, use_strategic=False, rational=False)
    assert len(result.equilibria) == 1
    # For floating-point results are not exact, so we skip testing exact values for now


def test_lp_behavior_rational():
    """Test calls of LP for mixed behavior equilibria, rational precision."""
    game = games.read_from_file("poker.efg")
    result = gbt.nash.lp_solve(game, use_strategic=False, rational=True)
    assert len(result.equilibria) == 1
    expected = game.mixed_behavior_profile(rational=True,
                                           data=[[[1, 0],
                                                  [gbt.Rational("1/3"), gbt.Rational("2/3")]],
                                                 [[gbt.Rational("2/3"), gbt.Rational("1/3")]]])
    assert result.equilibria[0] == expected


def test_liap_strategy():
    """Test calls of liap for mixed strategy equilibria."""
    game = games.read_from_file("poker.efg")
    _ = gbt.nash.liap_solve(game.mixed_strategy_profile())


def test_liap_behavior():
    """Test calls of liap for mixed behavior equilibria."""
    game = games.read_from_file("poker.efg")
    _ = gbt.nash.liap_solve(game.mixed_behavior_profile())


def test_simpdiv_strategy():
    """Test calls of simplicial subdivision for mixed strategy equilibria."""
    game = games.read_from_file("poker.efg")
    result = gbt.nash.simpdiv_solve(game.mixed_strategy_profile(rational=True))
    assert len(result.equilibria) == 1


def test_ipa_strategy():
    """Test calls of IPA for mixed strategy equilibria."""
    game = games.read_from_file("poker.efg")
    result = gbt.nash.ipa_solve(game)
    assert len(result.equilibria) == 1


def test_gnm_strategy():
    """Test calls of GNM for mixed strategy equilibria."""
    game = games.read_from_file("poker.efg")
    result = gbt.nash.gnm_solve(game)
    assert len(result.equilibria) == 1


def test_logit_strategy():
    """Test calls of logit for mixed strategy equilibria."""
    game = games.read_from_file("poker.efg")
    result = gbt.nash.logit_solve(game, use_strategic=True)
    assert len(result.equilibria) == 1


def test_logit_behavior():
    """Test calls of logit for mixed behavior equilibria."""
    game = games.read_from_file("poker.efg")
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
