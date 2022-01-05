#
# This file is part of Gambit
# Copyright (c) 1994-2016, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/python/gambit/nhas.py
# A set of utilities for computing Nash equilibria
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
"""
A set of utilities for computing Nash equilibria
"""

import sys
import subprocess
from fractions import Fraction

import pygambit.lib.libgambit
from .profiles import Solution

class NashSolution(Solution):
    def __init__(self, profile):
        Solution.__init__(self, profile)
    def __repr__(self):
        return "<NashProfile for '%s': %s>" % (self._profile.game.title,
                                               self._profile)
    
    
class ExternalSolver(object):
    """
    Base class for managing calls to external programs.
    """
    def launch(self, prog, game):
        """
        Helper function for launching calls to external programs.
        Calls the specified program 'prog', passing the game to standard
        input in .efg format (if a tree) or .nfg format (if a table).
        Returns the object referencing standard output of the external program.
        """
        p = subprocess.Popen("%s -q" % prog, shell=True,
                             stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                             close_fds=True if sys.platform != "win32" else False)
        child_stdin, child_stdout = p.stdin, p.stdout
        child_stdin.write(game.write(format='native'))
        # Need to close, or at least flush, stdin of the child, or else
        # processing won't begin...
        child_stdin.close()
        return child_stdout

    def _parse_output(self, stream, game, rational, extensive=False):
        profiles = [ ]
        for line in stream:
            entries = line.strip().split(",")
            if entries[0] != "NE":  continue
            if extensive:
                profile = game.mixed_behavior_profile(rational=rational)
            else:
                profile = game.mixed_strategy_profile(rational=rational)
            for (i, p) in enumerate(entries[1:]):
                if rational:
                    profile[i] = Fraction(p)
                else:
                    profile[i] = float(p)
            profiles.append(NashSolution(profile))
        return profiles

class ExternalEnumPureSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-enumpure solver
    for computing pure-strategy equilibria.
    """
    def solve(self, game, use_strategic=False):
        if not game.is_perfect_recall:
            raise RuntimeError("Computing equilibria of games with imperfect recall is not supported.")
        command_line = "gambit-enumpure"
        if use_strategic and game.is_tree:
            command_line += " -S"
        return self._parse_output(self.launch(command_line, game),
                                  game, rational=True,
                                  extensive=game.is_tree and not use_strategic)

class ExternalLPSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-lp solver
    for computing equilibria in two-player games using linear programming.
    """
    def solve(self, game, rational=False, use_strategic=False):
        if len(game.players) != 2:
            raise RuntimeError("Method only valid for two-player games.")
        if not game.is_const_sum:
            raise RuntimeError("Method only valid for constant-sum games.")
        if not game.is_perfect_recall:
            raise RuntimeError("Computing equilibria of games with imperfect recall is not supported.")
        if rational:
            command_line = "gambit-lp"
        else:
            command_line = "gambit-lp -d 10"
        if use_strategic and game.is_tree:
            command_line += " -S"
        return self._parse_output(self.launch(command_line, game),
                                  game, rational,
                                  extensive=game.is_tree and not use_strategic)

class ExternalLCPSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-lcp solver
    for computing equilibria in two-player games using linear complementarity
    programming.
    """
    def solve(self, game, rational=False, use_strategic=False):
        if len(game.players) != 2:
            raise RuntimeError("Method only valid for two-player games.")
        if not game.is_perfect_recall:
            raise RuntimeError("Computing equilibria of games with imperfect recall is not supported.")
        if rational:
            command_line = "gambit-lcp"
        else:
            command_line = "gambit-lcp -d 10"
        if use_strategic and game.is_tree:
            command_line += " -S"
        return self._parse_output(self.launch(command_line, game),
                                  game, rational,
                                  extensive=game.is_tree and not use_strategic)

class ExternalEnumMixedSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-enummixed solver
    for computing equilibria in two-player games using enumeration of extreme points.
    """
    def solve(self, game, rational=False):
        if not game.is_perfect_recall:
            raise RuntimeError("Computing equilibria of games with imperfect recall is not supported.")
        if rational:
            command_line = "gambit-enummixed"
        else:
            command_line = "gambit-enummixed -d 10"
        return self._parse_output(self.launch(command_line, game),
                                  game, rational)

class ExternalSimpdivSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-simpdiv solver
    for computing equilibria in N-player games using simpicial subdivision.
    """
    def solve(self, game):
        if not game.is_perfect_recall:
            raise RuntimeError("Computing equilibria of games with imperfect recall is not supported.")
        command_line = "gambit-simpdiv"
        return self._parse_output(self.launch(command_line, game),
                                  game, rational=True)
    
class ExternalGlobalNewtonSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-gnm solver
    for computing equilibria in N-player games using the global Newton method.
    """
    def solve(self, game):
        if not game.is_perfect_recall:
            raise RuntimeError("Computing equilibria of games with imperfect recall is not supported.")
        command_line = "gambit-gnm -d 10"
        return self._parse_output(self.launch(command_line, game),
                                  game, rational=False)

class ExternalEnumPolySolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-enumpoly solver
    for computing equilibria in N-player games systems of polynomial equations.
    """
    def solve(self, game, use_strategic=False):
        if not game.is_perfect_recall:
            raise RuntimeError("Computing equilibria of games with imperfect recall is not supported.")
        command_line = "gambit-enumpoly -d 10"
        if use_strategic and game.is_tree:
            command_line += " -S"
        return self._parse_output(self.launch(command_line, game),
                                  game, rational=False,
                                  extensive=game.is_tree and not use_strategic)

class ExternalLyapunovSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-liap solver
    for computing equilibria in N-player games using Lyapunov function minimization.
    """
    def solve(self, game, use_strategic=False):
        if not game.is_perfect_recall:
            raise RuntimeError("Computing equilibria of games with imperfect recall is not supported.")
        command_line = "gambit-liap -d 10"
        if use_strategic and game.is_tree:
            command_line += " -S"
        return self._parse_output(self.launch(command_line, game),
                                  game, rational=False,
                                  extensive=game.is_tree and not use_strategic)

class ExternalIteratedPolymatrixSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-ipa solver
    for computing equilibria in N-player games using iterated polymatrix approximation.
    """
    def solve(self, game):
        if not game.is_perfect_recall:
            raise RuntimeError("Computing equilibria of games with imperfect recall is not supported.")
        command_line = "gambit-ipa -d 10"
        return self._parse_output(self.launch(command_line, game),
                                  game, rational=False)

class ExternalLogitSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-logit solver
    for computing equilibria in N-player games using quantal response equilibrium.
    """
    def solve(self, game, use_strategic=False):
        if not game.is_perfect_recall:
            raise RuntimeError("Computing equilibria of games with imperfect recall is not supported.")
        profiles = [ ]
        command_line = "gambit-logit -d 20 -e"
        if use_strategic and game.is_tree:
            command_line += " -S"
        return self._parse_output(self.launch(command_line, game),
                                  game, rational=False,
                                  extensive=game.is_tree and not use_strategic)


def enumpure_solve(game, use_strategic=True, external=False):
    """Convenience function to solve game to find pure-strategy Nash equilibria.
    """
    if external:
        return ExternalEnumPureSolve().solve(game, use_strategic=True)
    if not game.is_tree or use_strategic:
        alg = pygambit.lib.libgambit.EnumPureStrategySolver()
    else:
        alg = pygambit.lib.libgambit.EnumPureAgentSolver()
    return alg.solve(game)

def enummixed_solve(game, rational=True, external=False, use_lrs=False):
    """Convenience function to solve two-player game to find all
    mixed-strategy Nash equilibria.
    """
    if external:
        return ExternalEnumMixedSolver().solve(game, rational=rational)
    if use_lrs:
        alg = pygambit.lib.libgambit.EnumMixedLrsStrategySolver()
    if rational:
        alg = pygambit.lib.libgambit.EnumMixedStrategySolverRational()
    else:
        alg = pygambit.lib.libgambit.EnumMixedStrategySolverDouble()
    return alg.solve(game)

def lcp_solve(game, rational=True, use_strategic=False, external=False,
              stop_after=None, max_depth=None):
    """Convenience function to solve game using an appropriate linear
    complementarity solver.
    """
    if stop_after is None: stop_after = 0
    if max_depth is None:  max_depth = 0
    if external:
        return ExternalLCPSolver().solve(game, rational=rational,
                                         use_strategic=use_strategic)
    if not game.is_tree or use_strategic:
        if rational:
            alg = pygambit.lib.libgambit.LCPStrategySolverRational(stop_after, max_depth)
        else:
            alg = pygambit.lib.libgambit.LCPStrategySolverDouble(stop_after, max_depth)
    else:        
        if rational:
            alg = pygambit.lib.libgambit.LCPBehaviorSolverRational(stop_after, max_depth)
        else:
            alg = pygambit.lib.libgambit.LCPBehaviorSolverDouble(stop_after, max_depth)
    return alg.solve(game)

def lp_solve(game, rational=True, use_strategic=False, external=False):
    """Convenience function to solve game using an appropriate linear
    programming solver.
    """
    if external:
        return ExternalLPSolver().solve(game, rational=rational,
                                         use_strategic=use_strategic)
    if not game.is_tree or use_strategic:
        if rational:
            alg = pygambit.lib.libgambit.LPStrategySolverRational()
        else:
            alg = pygambit.lib.libgambit.LPStrategySolverDouble()
    else:        
        if rational:
            alg = pygambit.lib.libgambit.LPBehaviorSolverRational()
        else:
            alg = pygambit.lib.libgambit.LPBehaviorSolverDouble()
    return alg.solve(game)

def simpdiv_solve(game, external=False):
    """Convenience function to solve game to find a mixed-strategy
    Nash equilibrium using simplicial subdivision.
    """
    if external:
        return ExternalSimpdivSolver().solve(game)
    alg = pygambit.lib.libgambit.SimpdivStrategySolver()
    return alg.solve(game)

def ipa_solve(game, external=False):
    """Convenience function to solve game to find a mixed-strategy
    Nash equilibrium using iterated polymatrix appoximation.
    """
    if external:
        return ExternalIPASolver().solve(game)
    alg = pygambit.lib.libgambit.IPAStrategySolver()
    return alg.solve(game)

def gnm_solve(game, external=False):
    """Convenience function to solve game to find mixed-strategy
    Nash equilibria using the global Newton method.
    """
    if external:
        return ExternalGNMSolver().solve(game)
    alg = pygambit.lib.libgambit.GNMStrategySolver()
    return alg.solve(game)

logit_estimate = pygambit.lib.libgambit.logit_estimate
logit_atlambda = pygambit.lib.libgambit.logit_atlambda
logit_principal_branch = pygambit.lib.libgambit.logit_principal_branch

