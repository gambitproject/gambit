#
# This file is part of Gambit
# Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
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
from gambit.profiles import Solution

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
                profile = game.behav_profile(rational=rational)
            else:
                profile = game.mixed_profile(rational=rational)
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
        command_line = "gambit-simpdiv"
        return self._parse_output(self.launch(command_line, game),
                                  game, rational=True)
    
class ExternalGlobalNewtonSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-gnm solver
    for computing equilibria in N-player games using the global Newton method.
    """
    def solve(self, game):
        command_line = "gambit-gnm -d 10"
        return self._parse_output(self.launch(command_line, game),
                                  game, rational=False)

class ExternalEnumPolySolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-enumpoly solver
    for computing equilibria in N-player games systems of polynomial equations.
    """
    def solve(self, game, use_strategic=False):
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
        command_line = "gambit-ipa -d 10"
        return self._parse_output(self.launch(command_line, game),
                                  game, rational=False)

class ExternalLogitSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-logit solver
    for computing equilibria in N-player games using quantal response equilibrium.
    """
    def solve(self, game, use_strategic=False):
        profiles = [ ]
        command_line = "gambit-logit -d 20 -e"
        if use_strategic and game.is_tree:
            command_line += " -S"
        return self._parse_output(self.launch(command_line, game),
                                  game, rational=False,
                                  extensive=game.is_tree and not use_strategic)
