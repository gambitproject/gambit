#
# This file is part of Gambit
# Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
#                          Tobenna Igwe (T.Igwe@liverpool.ac.uk)
#
# FILE: src/python/gambit/approx.py
# Implementation of Kontogiannis-Spirakis algorithm for well-supported
# approximate Nash equilibrium.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#

from fractions import Fraction

import numpy as np
import cdd

import pygambit as gbt


class KontogiannisSpirakisResult:
    """Represents the result of the Kontogiannis-Spirakis algorithm.

    Attributes
    ----------
    profile : MixedStrategyProfile
        The computed mixed-strategy profile.

    epsilon_ne : Rational
        The value of $\varepsilon$ for which the profile is an $\varepsilon$-Nash equilibrium.

    epsilon_wsne : Rational
        The value of $\varepsilon$ for which the profile is an $\varepsilon$-well-supported
        approximate Nash equilibrium.
    """
    def __init__(self, profile: gbt.MixedStrategyProfileRational, matrices):
        self._profile = profile
        self._compute_attributes(matrices)

    @property
    def profile(self) -> gbt.MixedStrategyProfileRational:
        return self._profile

    @property
    def epsilon_ne(self) -> gbt.Rational:
        return self._epsilon_ne

    @property
    def epsilon_wsne(self) -> gbt.Rational:
        return self._epsilon_wsne

    def __str__(self) -> str:
        return (
            "<KontogiannisSpirakisResult for '%s': %s is %s-NE, %s-WSNE>" %
            (self.profile.game.title, self.profile, self.epsilon_ne, self.epsilon_wsne)
        )

    @staticmethod
    def _compute_epsilon_br(M: np.array, r: np.array, c: np.array) -> gbt.Rational:
        """Calculates epsilon for strategy profile for NE."""
        payoffs = np.asmatrix(M) * np.asmatrix(c).transpose()
        return payoffs.max() - payoffs[np.asmatrix(r).transpose() > 0].min()

    @staticmethod
    def _compute_epsilon_wsne(M: np.array, r: np.array, c: np.array) -> gbt.Rational:
        """Calculates epsilon for strategy profile for WSNE."""
        payoffs = np.asmatrix(M) * np.asmatrix(c).transpose()
        return payoffs.max() - (np.asmatrix(r) * payoffs)[0, 0]

    def _compute_attributes(self, matrices):
        """Computes the values of epsilon for the profile."""
        A, B = matrices
        x = np.array(self._profile[self._profile.game.players[0]])
        y = np.array(self._profile[self._profile.game.players[1]])
        self._epsilon_ne = max(
            self._compute_epsilon_wsne(A, x, y),
            self._compute_epsilon_wsne(B.transpose(), y, x)
        )
        self._epsilon_wsne = max(
            self._compute_epsilon_br(A, x, y),
            self._compute_epsilon_br(B.transpose(), y, x)
        )


class KontogiannisSpirakisSolver:
    """Compute a 2/3 well-supported approximate Nash equilibrium of a bimatrix game.
    using the algorithm of Kontogiannis and Spirakis.
    """
    @staticmethod
    def _solve(M: np.array) -> tuple:
        """Find II's minmax strategy for zero-sum game M."""
        m, n = M.shape

        M = np.column_stack([[0]*m, -M, [1]*m])
        nn = np.column_stack([[0]*n, np.eye(n), [0]*n])
        # non-negativity constraints
        n1 = [1] + ([-1] * n) + [0]
        n2 = [-1] + ([1] * n) + [0]
        mat = cdd.Matrix(np.vstack([M, nn, n1, n2]).tolist(), number_type='fraction')
        mat.obj_type = cdd.LPObjType.MIN
        mat.obj_func = ([0] * (n+1)) + [1]

        lp = cdd.LinProg(mat)
        lp.solve()
        if lp.status != cdd.LPStatusType.OPTIMAL:
            raise RuntimeError("LP solver failed to find optimal solution")
        # The primal solution is player 2's equilibrium (minmax) strategy
        return lp.primal_solution

    @staticmethod
    def _extract_payoff_table(game: gbt.Game, player: gbt.Player) -> np.array:
        """Extract the payoff table of 'player' from 'game' as a numpy array."""
        return (
            np.array([[game[i, j][player] for j in game.players[1].strategies]
                      for i in game.players[0].strategies])
        )

    @staticmethod
    def _construct_ks_game(A: np.array, B: np.array) -> np.array:
        """Constructs the payoff matrix of the zero-sum auxiliary game
        used by the K-S algorithm."""
        return Fraction(1, 2) * (A - B)

    def solve(self, game: gbt.Game) -> KontogiannisSpirakisResult:
        """Computes the well-supported approximate-Nash equilibrium."""
        A = self._extract_payoff_table(game, game.players[0])
        B = self._extract_payoff_table(game, game.players[1])
        D = self._construct_ks_game(A, B)
        p = game.mixed_strategy_profile(
            rational=True,
            data=[self._solve(-D.transpose())[:A.shape[0]],
                  self._solve(D)[:A.shape[1]]]
        )
        return KontogiannisSpirakisResult(p, (A, B))


def solve_wellsupported(game: gbt.Game) -> KontogiannisSpirakisResult:
    """Compute a 2/3 well-supported approximate Nash equilibrium of a bimatrix game.
    using the algorithm of [1]_.

    .. versionadded:: 16.1.0

    Parameters
    ----------
    game : gbt.Game
        The game to compute an approximate Nash equilibrium for.

    Returns
    -------
    KontogiannisSpirakisResult
        The result of the computation represented as a `KontogiannisSpirakisResult`.

    .. [1] Spyros C. Kontogiannis and Paul G. Spirakis, "Well supported
       approximate equilibria in bimatrix games,"  Algorithmica 57(4): 653-667, 2010.
    """
    return KontogiannisSpirakisSolver().solve(game)
