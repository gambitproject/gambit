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

import cdd  # continuous double description - includes exact LP solver
import numpy as np

import pygambit as gbt


# Convert a game to numpy array
# Possibly the best location would be in the definition of
# gambit.Game, however, the function would need to be limited
# to strategic games.
def game_to_np(game):
    m = len(game.players[0].strategies)
    n = len(game.players[1].strategies)
    A = [[game[i, j][0] for j in range(n)] for i in range(m)]
    B = [[game[i, j][1] for j in range(n)] for i in range(m)]
    A = np.array(A)
    B = np.array(B)
    return A, B


# Calculates epsilon for a given strategy profile
# If best_response is true then return the epsilon corresponding
# to that of WSNE otherwise treat as an epsilon-NE
# Not entirely sure where this function should be.
def compute_epsilon(M, r, c, best_response=False):
    r = np.mat(r)
    c = np.mat(c)
    M = np.mat(M)
    payoffs = M*c.transpose()
    # print "payoffs", payoffs
    if best_response:
        n = payoffs[r.transpose() > 0].min()
    else:
        n = (r * payoffs)[0, 0]
    # print "n", n
    x = payoffs.max()
    # print "x", x
    return x-n


class ApproximateSolution:
    """A representation of an approximate Nash solution."""
    def __init__(self, profile):
        self._profile = profile
        self._setinfo()

    def __repr__(self):
        return (
            "<ApproximateProfile for '%s': %s is %s-NE, %s-WSNE>" %
            (self._profile.game.title, self._profile, self.epsNE, self.epsWSNE)
        )

    # Computes and stores the epsilon-NE and epsilon-WSNE for
    # a given strategy profile
    def _setinfo(self):
        A, B = game_to_np(self._profile.game)
        x = np.array(self._profile[self._profile.game.players[0]])
        y = np.array(self._profile[self._profile.game.players[1]])
        self.epsNE = max(compute_epsilon(A, x, y), compute_epsilon(B.transpose(), y, x))
        self.epsWSNE = max(compute_epsilon(A, x, y, True), compute_epsilon(B.transpose(), y, x, True))


class KontogiannisSpirakisSolver:
    """Compute a 2/3 well-supported approximate Nash equilibrium of a bimatrix game.

    Implements the algorithm presented in
    Spyros C. Kontogiannis, Paul G. Spirakis (2010). Well Supported
    Approximate Equilibria in Bimatrix Games. Algorithmica 57(4): 653-667.
    """
    def _solve(self, M):
        """Find II's minmax strategy for zero-sum game M."""
        m, n = M.shape

        M = np.column_stack([[0]*m, -M, [1]*m])
        nn = np.column_stack([[0]*n, np.eye(n), [0]*n])
        # non-negativity constraints
        n1 = [1] + ([-1] * n) + [0]
        n2 = [-1] + ([1] * n) + [0]

        d = np.vstack([M, nn, n1, n2])

        mat = cdd.Matrix(d.tolist(), number_type='fraction')
        mat.obj_type = cdd.LPObjType.MIN
        mat.obj_func = ([0] * (n+1)) + [1]

        lp = cdd.LinProg(mat)
        lp.solve()
        assert lp.status == cdd.LPStatusType.OPTIMAL
        # print mat

        # print(lp.obj_value) # value of game = player I's equilibrium payoff

        # primal solution is players II's equilibrium (minmax) strategy
        return lp.primal_solution

    # Converts numpy array A, B to the matrix D according to the KS algorithm
    def _ksGame(self, A, B):
        return Fraction(1, 2) * (A - B)

    def solve(self, game: gbt.Game):
        """Computes the well-supported approximate-Nash equilibrium."""
        A, B = game_to_np(game)
        D = self._ksGame(A, B)
        y = self._solve(D)
        y = y[0:A.shape[1]]
        x = self._solve(-D.transpose())
        x = x[0:A.shape[0]]

        p = game.mixed_strategy_profile(rational=True)
        p[game.players[0]] = x
        p[game.players[1]] = y

        return [ApproximateSolution(p)]
