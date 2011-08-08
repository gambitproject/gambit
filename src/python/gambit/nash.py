"""
A set of utilities for computing Nash equilibria
"""

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
                             close_fds=True)
        child_stdin, child_stdout = p.stdin, p.stdout
        if game.is_tree:
            child_stdin.write(game.write(strategic=False))
        else:
            child_stdin.write(game.write(strategic=True))
        # Need to close, or at least flush, stdin of the child, or else
        # processing won't begin...
        child_stdin.close()
        return child_stdout

    def _parse_output(self, stream, game, rational):
        profiles = [ ]
        for line in stream:
            entries = line.strip().split(",")
            if entries[0] != "NE":  continue
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
    def solve(self, game):
        command_line = "gambit-enumpure"
        return self._parse_output(self.launch(command_line, game),
                                  game, rational=True)

class ExternalLPSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-lp solver
    for computing equilibria in two-player games using linear programming.
    """
    def solve(self, game, rational=False):
        if rational:
            command_line = "gambit-lp"
        else:
            command_line = "gambit-lp -d 10"
        return self._parse_output(self.launch(command_line, game),
                                  game, rational)

class ExternalLCPSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-lcp solver
    for computing equilibria in two-player games using linear complementarity
    programming.
    """
    def solve(self, game, rational=False):
        if rational:
            command_line = "gambit-lcp"
        else:
            command_line = "gambit-lcp -d 10"
        return self._parse_output(self.launch(command_line, game),
                                  game, rational)

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
                                  game, rational)

class ExternalEnumPolySolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-enumpoly solver
    for computing equilibria in N-player games systems of polynomial equations.
    """
    def solve(self, game):
        command_line = "gambit-enumpoly -d 10"
        return self._parse_output(self.launch(command_line, game),
                                  game, rational)

class ExternalLyapunovSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-liap solver
    for computing equilibria in N-player games using Lyapunov function minimization.
    """
    def solve(self, game):
        command_line = "gambit-liap -d 10"
        return self._parse_output(self.launch(command_line, game),
                                  game, rational)

class ExternalIteratedPolymatrixSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-ipa solver
    for computing equilibria in N-player games using iterated polymatrix approximation.
    """
    def solve(self, game):
        command_line = "gambit-ipa -d 10"
        return self._parse_output(self.launch(command_line, game),
                                  game, rational)

class ExternalLogitSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-logit solver
    for computing equilibria in N-player games using quantal response equilibrium.
    """
    def solve(self, game):
        profiles = [ ]
        command_line = "gambit-logit -d 20 -e"
        return self._parse_output(self.launch(command_line, game),
                                  game, rational)
