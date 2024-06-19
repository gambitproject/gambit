import pygambit as gbt
import pytest

from . import games


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
