import dataclasses
import types
import typing
from fractions import Fraction

import cdd
import cdd.gmp
import numpy as np
import pytest

import pygambit as gbt

from . import games

# Payoff difference far below double precision: float(1 + EPS) == 1.0, so an
# implementation using floating point would see a tie where there is none.
EPS = Fraction(1, 10**20)


def _assert_valid_witness(game: gbt.Game, player: int, strategy):
    """Check, in exact arithmetic, that the opponent has more pure best responses
    to `strategy` than the size of its support."""
    A, B = game.to_arrays()
    responder = B if player == 0 else A.T
    assert isinstance(strategy, np.ndarray)
    assert all(isinstance(p, gbt.Rational) for p in strategy)
    assert len(strategy) == responder.shape[0]
    assert all(p >= 0 for p in strategy)
    assert sum(strategy) == 1
    payoffs = strategy @ responder
    best_responses = np.count_nonzero(payoffs == np.max(payoffs))
    support_size = np.count_nonzero(strategy > 0)
    assert best_responses > support_size


@pytest.mark.parametrize("game,expected_result", [
    # Nondegenerate games
    # - 2x2: no support of size 2 can have 3 best responses, so no LP is solved
    (gbt.StrategicGame.from_arrays([[1, -1], [-1, 1]], [[-1, 1], [1, -1]]),
     False),  # matching pennies; SageMath
    (gbt.StrategicGame.from_arrays([[2, 5], [0, 4]], [[2, 0], [5, 4]]),
     False),  # prisoner's dilemma; SageMath
    # - larger games, where every candidate LP is infeasible
    (gbt.StrategicGame.from_arrays([[0, -1, 1], [1, 0, -1], [-1, 1, 0]],
                                   [[0, 1, -1], [-1, 0, 1], [1, -1, 0]]),
     False),  # rock-paper-scissors; SageMath
    (gbt.catalog.load("journals/other/shapley1974/fig2"), False),
    (gbt.catalog.load("journals/other/shapley1974/fig3"), False),
    (gbt.catalog.load("journals/dcg/vonstengel1999/6x6_game_with_75_eq"),
     False),  # constructed to be nondegenerate with many equilibria
    (games.read_from_file("coordination_4x4_payoff.nfg"), False),
    # - one player has a single strategy, so only pure strategies are checked
    (gbt.StrategicGame.from_arrays([[1, 2, 3]], [[1, 2, 3]]), False),
    # - near-ties which are not ties in exact arithmetic
    (gbt.StrategicGame.from_arrays([[1, 0], [0, 1]], [[1, 1 + EPS], [1 + EPS, 1]]),
     False),  # pure strategies
    (gbt.StrategicGame.from_arrays([[1, 2, 3], [6, 5, 4]], [[2, 0, 1 - EPS], [0, 2, 1 - EPS]]),
     False),  # mixing (1/2, 1/2) leaves column 3 just short of a best response

    # Degenerate games
    # - a pure strategy has more than one pure best response
    (gbt.StrategicGame.from_arrays([[3, 3], [2, 5], [0, 6]], [[3, 3], [2, 6], [3, 1]]),
     True),  # Avis, Rosenberg, Savani and von Stengel (2010), via SageMath
    (gbt.StrategicGame.from_arrays([[0, 6], [2, 5], [3, 3]], [[1, 0], [0, 2], [4, 4]]),
     True),  # Nisan, Roughgarden, Tardos and Vazirani (2007), via SageMath
    (gbt.StrategicGame.from_arrays([[2, 1], [1, 1]], [[1, 1], [1, 2]]), True),  # SageMath
    (gbt.StrategicGame.from_arrays([[0, -1, 1, 1, -1], [1, 0, -1, -1, 1],
                                    [-1, 1, 0, 1, -1], [-1, 1, -1, 0, 1],
                                    [1, -1, 1, -1, 0]],
                                   [[0, 1, -1, -1, 1], [-1, 0, 1, 1, -1],
                                    [1, -1, 0, -1, 1], [1, -1, 1, 0, -1],
                                    [-1, 1, -1, 1, 0]]),
     True),  # rock-paper-scissors-lizard-Spock; SageMath
    (gbt.StrategicGame.from_arrays([[0, 0, 0]] * 3, [[0, 0, 0]] * 3), True),  # SageMath
    (gbt.StrategicGame.from_arrays([[-75, 18, 45, 33], [42, -8, -77, -18],
                                    [83, 18, 11, 40], [-10, -38, 76, -9]],
                                   [[62, 64, 87, 51], [-41, -27, -69, 52],
                                    [-17, 25, -97, -82], [30, 31, -1, 50]]),
     True),  # SageMath; only the column player's pure strategies have ties
    (gbt.StrategicGame.from_arrays([[1, 2, 3]], [[3, 3, 1]]), True),
    (gbt.catalog.load("journals/ijgt/selten1975/fig2"),
     True),  # extensive game, via its reduced strategic form
    # - only a mixed strategy has more best responses than its support size
    (gbt.StrategicGame.from_arrays([[1, 2, 3], [6, 5, 4]], [[2, 0, 1], [0, 2, 1]]),
     True),  # row player mixes
    (gbt.StrategicGame.from_arrays([[3, 0], [0, 3], [Fraction(3, 2), Fraction(3, 2)]],
                                   [[4, 3], [2, 6], [3, 1]]),
     True),  # column player mixes; SageMath
    (gbt.StrategicGame.from_arrays([[1, 2, 3], [4, 6, 5], [9, 8, 7]],
                                   [[5, 0, 1], [2, 0, 1], [0, 2, 1]]),
     True),  # found only after the LPs for rows {1, 2} and {1, 3} are infeasible
])
def test_is_degenerate(game: gbt.Game, expected_result: bool):
    """
    Verify is_degenerate against degenerate and nondegenerate games, with and
    without a certificate, checking any witness in exact arithmetic.
    """
    assert game.is_degenerate() is expected_result
    result, witness = game.is_degenerate(certificate=True)
    assert result is expected_result
    if expected_result:
        _assert_valid_witness(game, *witness)
    else:
        assert witness is None


@dataclasses.dataclass
class CertificateTestCase:
    """TestCase for a game whose degeneracy has exactly one witness."""
    factory: typing.Callable[[], gbt.Game]
    expected_player: int
    expected_strategy: list


CERTIFICATE_CASES = [
    pytest.param(
        CertificateTestCase(
            # Only the row player's single strategy has tied best responses, (3, 3).
            factory=lambda: gbt.StrategicGame.from_arrays([[1, 2, 3]], [[3, 3, 1]]),
            expected_player=0,
            expected_strategy=[1],
        ),
        id="pure_row_player"
    ),
    pytest.param(
        CertificateTestCase(
            # Only the first column gives the row player tied payoffs, (5, 5).
            factory=lambda: gbt.StrategicGame.from_arrays([[5, 0], [5, 2]], [[1, 2], [3, 4]]),
            expected_player=1,
            expected_strategy=[1, 0],
        ),
        id="pure_column_player"
    ),
    pytest.param(
        CertificateTestCase(
            # Mixing (x, 1 - x) gives the column player (2x, 2 - 2x, 1): all equal only
            # at x = 1/2.  With two rows, the column player cannot have a witness.
            factory=lambda: gbt.StrategicGame.from_arrays([[1, 2, 3], [6, 5, 4]],
                                                          [[2, 0, 1], [0, 2, 1]]),
            expected_player=0,
            expected_strategy=[Fraction(1, 2), Fraction(1, 2)],
        ),
        id="mixed_row_player"
    ),
    pytest.param(
        CertificateTestCase(
            # Mixing (y, 1 - y) gives the row player (3y, 3 - 3y, 3/2): all equal only
            # at y = 1/2.  With two columns, the row player cannot have a witness.
            factory=lambda: gbt.StrategicGame.from_arrays(
                [[3, 0], [0, 3], [Fraction(3, 2), Fraction(3, 2)]], [[4, 3], [2, 6], [3, 1]]
            ),
            expected_player=1,
            expected_strategy=[Fraction(1, 2), Fraction(1, 2)],
        ),
        id="mixed_column_player"
    ),
    pytest.param(
        CertificateTestCase(
            # Rows {1, 2}: columns 2 and 3 always pay 0 and 1, never tied.
            # Rows {1, 3}: (5x, 2 - 2x, 1) are never all equal.
            # Rows {2, 3}: (2x, 2 - 2x, 1) are all equal at x = 1/2.
            factory=lambda: gbt.StrategicGame.from_arrays(
                [[1, 2, 3], [4, 6, 5], [9, 8, 7]], [[5, 0, 1], [2, 0, 1], [0, 2, 1]]
            ),
            expected_player=0,
            expected_strategy=[0, Fraction(1, 2), Fraction(1, 2)],
        ),
        id="mixed_after_infeasible_supports"
    ),
]


@pytest.mark.parametrize("test_case", CERTIFICATE_CASES)
def test_is_degenerate_certificate(test_case: CertificateTestCase):
    """
    Verify the witness returned by is_degenerate(certificate=True) for games
    in which it is unique.
    """
    game = test_case.factory()
    result, (player, strategy) = game.is_degenerate(certificate=True)
    assert result is True
    assert player == test_case.expected_player
    assert list(strategy) == test_case.expected_strategy
    _assert_valid_witness(game, player, strategy)


@pytest.mark.parametrize("strategies", [[3], [2, 2, 2]])
def test_is_degenerate_rejects_non_two_player(strategies: list):
    """is_degenerate is only defined for two-player games."""
    game = gbt.StrategicGame(strategies)
    with pytest.raises(NotImplementedError, match="two-player"):
        game.is_degenerate()


def _fake_lp_status(monkeypatch, status):
    """Make every LP solved by cdd report `status`."""
    monkeypatch.setattr(cdd.gmp, "linprog_from_matrix",
                        lambda mat: types.SimpleNamespace(status=status))
    monkeypatch.setattr(cdd.gmp, "linprog_solve", lambda lp: None)


def _rock_paper_scissors() -> gbt.Game:
    return gbt.StrategicGame.from_arrays([[0, -1, 1], [1, 0, -1], [-1, 1, 0]],
                                         [[0, 1, -1], [-1, 0, 1], [1, -1, 0]])


def test_is_degenerate_struc_inconsistent_lp(monkeypatch):
    """An LP that cdd reports as structurally inconsistent has no witness."""
    _fake_lp_status(monkeypatch, cdd.LPStatusType.STRUC_INCONSISTENT)
    assert _rock_paper_scissors().is_degenerate(certificate=True) == (False, None)


def test_is_degenerate_unexpected_lp_status(monkeypatch):
    """An LP status other than optimal or infeasible is an error."""
    _fake_lp_status(monkeypatch, cdd.LPStatusType.UNBOUNDED)
    with pytest.raises(RuntimeError, match="unexpected LP status"):
        _rock_paper_scissors().is_degenerate()
