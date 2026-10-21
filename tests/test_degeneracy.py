from fractions import Fraction

import numpy as np
import pytest

import pygambit as gbt


def _assert_valid_witness(A, B, player, strategy):
    """Check exactly that the opponent has more pure best responses to
    ``strategy`` than its support size."""
    responder = B if player == 0 else [list(col) for col in zip(*A, strict=True)]
    strategy = [Fraction(p) for p in strategy]
    assert len(strategy) == len(responder)
    assert all(p >= 0 for p in strategy)
    assert sum(strategy) == 1
    payoffs = [sum(p * Fraction(row[j]) for p, row in zip(strategy, responder, strict=True))
               for j in range(len(responder[0]))]
    best_responses = sum(v == max(payoffs) for v in payoffs)
    support_size = sum(p > 0 for p in strategy)
    assert best_responses > support_size


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
    g = gbt.Game.new_table([2, 2, 2])
    with pytest.raises(NotImplementedError, match="two-player"):
        g.is_degenerate()


# --- certificate=True, nondegenerate ---
def test_certificate_false_case():
    g = gbt.Game.from_arrays([[1, -1], [-1, 1]], [[-1, 1], [1, -1]])
    result, cert = g.is_degenerate(certificate=True)
    assert result is False
    assert cert is None


# --- certificate validity: player 0 witness (#cert, #2) ---
def test_certificate_witnesses_degeneracy_player0():
    A = [[2, 1], [1, 1]]
    B = [[1, 1], [1, 2]]
    g = gbt.Game.from_arrays(A, B)
    is_deg, (player, strategy) = g.is_degenerate(certificate=True)
    assert is_deg is True
    _assert_valid_witness(A, B, player, strategy)


# --- certificate validity: player 1 witness (#5, catches the index bug) ---
def test_certificate_witnesses_degeneracy_player1():
    A = [[3, 0], [0, 3], [Fraction(3, 2), Fraction(3, 2)]]
    B = [[4, 3], [2, 6], [3, 1]]
    g = gbt.Game.from_arrays(A, B)
    is_deg, (player, strategy) = g.is_degenerate(certificate=True)
    assert is_deg is True
    assert player == 1
    _assert_valid_witness(A, B, player, strategy)


# --- 4x5 mixed-degeneracy witness validity ---
def test_4x5_mixed_witness_valid():
    A = [[7, 1, 2, 1, 3], [1, 7, 1, 2, 1], [2, 2, 7, 1, 2], [1, 3, 1, 7, 6]]
    B = [[6, 0, 3, 1, 2], [0, 6, 3, 1, 0], [1, 2, 5, 0, 1], [2, 1, 0, 4, 3]]
    g = gbt.Game.from_arrays(A, B)
    is_deg, (player, strategy) = g.is_degenerate(certificate=True)
    assert is_deg is True
    _assert_valid_witness(A, B, player, strategy)


# --- Degenerate only through a mixed strategy (no pure ties) ---
def test_degenerate_only_via_mixed_strategy():
    A = [[1, 2, 3], [6, 5, 4]]
    B = [[2, 0, 1], [0, 2, 1]]
    g = gbt.Game.from_arrays(A, B)
    is_deg, (player, strategy) = g.is_degenerate(certificate=True)
    assert is_deg is True
    assert player == 0
    assert list(strategy) == [Fraction(1, 2), Fraction(1, 2)]
    _assert_valid_witness(A, B, player, strategy)


# --- Witness is a numpy array of exact rationals ---
def test_certificate_is_exact_rational_array():
    g = gbt.Game.from_arrays([[2, 1], [1, 1]], [[1, 1], [1, 2]])
    _, (_, strategy) = g.is_degenerate(certificate=True)
    assert isinstance(strategy, np.ndarray)
    assert all(isinstance(p, gbt.Rational) for p in strategy)


# --- Payoff ties that floating point would blur must still count exactly ---
def test_near_tie_is_not_degenerate():
    eps = Fraction(1, 10**12)
    g = gbt.Game.from_arrays([[1, 0], [0, 1]], [[1, 1 + eps], [1 + eps, 1]])
    assert g.is_degenerate() is False
