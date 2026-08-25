import numpy as np
import pytest

import pygambit as gbt


def _best_response_count(responder_payoff, strategizer_mix, tol=1e-7):
    payoffs = np.asarray(strategizer_mix, float) @ responder_payoff
    return int(np.sum(payoffs >= payoffs.max() - tol))


# --- Nondegenerate (#6) ---
def test_matching_pennies_not_degenerate():
    g = gbt.Game.from_arrays([[1, -1], [-1, 1]], [[-1, 1], [1, -1]])
    assert g.is_degenerate() is False


# --- Degenerate, pure, player 1 side (#2) — literature examples ---
def test_degenerate_DGRB2010():
    g = gbt.Game.from_arrays([[3, 3], [2, 5], [0, 6]], [[3, 3], [2, 6], [3, 1]])
    assert g.is_degenerate() is True


def test_degenerate_NN2007():
    g = gbt.Game.from_arrays([[0, 6], [2, 5], [3, 3]], [[1, 0], [0, 2], [4, 4]])
    assert g.is_degenerate() is True


# --- Degenerate, pure, player 2 side (#3) ---
def test_degenerate_player2_pure():
    g = gbt.Game.from_arrays([[5, 0], [5, 2]], [[1, 2], [3, 4]])
    assert g.is_degenerate() is True


# --- Degenerate via LP, player 1 mixing (#4) ---
def test_degenerate_LP_player1_mixing():
    g = gbt.Game.from_arrays([[1, 2, 3], [4, 5, 6], [7, 8, 9]],
                             [[6, 0, 3], [0, 6, 3], [1, 2, 5]])
    assert g.is_degenerate() is True


# --- Not two players (#1) ---
def test_rejects_three_player():
    g = gbt.Game.new_table([2, 2, 2])       # <-- verify constructor name!
    with pytest.raises(ValueError):
        g.is_degenerate()


# --- certificate=True, nondegenerate ---
def test_certificate_false_case():
    g = gbt.Game.from_arrays([[1, -1], [-1, 1]], [[-1, 1], [1, -1]])
    result, cert = g.is_degenerate(certificate=True)
    assert result is False
    assert cert is None


# --- certificate validity: player 0 witness (#cert, #2) ---
def test_certificate_witnesses_degeneracy_player0():
    A = np.array([[2, 1], [1, 1]], float)
    B = np.array([[1, 1], [1, 2]], float)
    g = gbt.Game.from_arrays(A, B)
    is_deg, (player, strategy) = g.is_degenerate(certificate=True)
    assert is_deg is True
    strategy = np.asarray(strategy, float)
    mat = B if player == 0 else A.T
    n_br = _best_response_count(mat, strategy)
    assert n_br > int(np.sum(strategy > 1e-7))


# --- certificate validity: player 1 witness (#5, catches the index bug) ---
def test_certificate_witnesses_degeneracy_player1():
    A = np.array([[3, 0], [0, 3], [1.5, 1.5]], float)
    B = np.array([[4, 3], [2, 6], [3, 1]], float)
    g = gbt.Game.from_arrays(A, B)
    is_deg, (player, strategy) = g.is_degenerate(certificate=True)
    assert is_deg is True
    assert player == 1
    strategy = np.asarray(strategy, float)
    n_br = _best_response_count(A.T, strategy)
    assert n_br > int(np.sum(strategy > 1e-7))


# --- 4x5 mixed-degeneracy witness validity ---
def test_4x5_mixed_witness_valid():
    A = [[7, 1, 2, 1, 3], [1, 7, 1, 2, 1], [2, 2, 7, 1, 2], [1, 3, 1, 7, 6]]
    B = [[6, 0, 3, 1, 2], [0, 6, 3, 1, 0], [1, 2, 5, 0, 1], [2, 1, 0, 4, 3]]
    g = gbt.Game.from_arrays(A, B)
    is_deg, (player, strategy) = g.is_degenerate(certificate=True)
    assert is_deg is True
    strategy = np.asarray(strategy, float)
    mat = np.array(B, float) if player == 0 else np.array(A, float).T
    n_br = _best_response_count(mat, strategy)
    assert n_br > int(np.sum(strategy > 1e-7))
