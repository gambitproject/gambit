"""
A set of utilities for computing Nash equilibria
"""

import subprocess

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

class ExternalEnumPureSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-enumpure solver
    for computing pure-strategy equilibria.
    """
    def solve(self, game, rational=False):
        profiles = [ ]
        command_line = "gambit-enumpure"
        for line in self.launch(command_line, game):
            entries = line.strip().split(",")
            if entries[0] != "NE":  continue
            profile = game.mixed_profile()
            for (i, p) in enumerate(entries[1:]):
                profile[i] = float(p)
            profiles.append(profile)
        return profiles

class ExternalLPSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-lp solver
    for computing equilibria in two-player games using linear programming.
    """
    def solve(self, game, rational=False):
        profiles = [ ]
        command_line = "gambit-lp -d 10"
        for line in self.launch(command_line, game):
            entries = line.strip().split(",")
            if entries[0] != "NE":  continue
            profile = game.mixed_profile()
            for (i, p) in enumerate(entries[1:]):
                profile[i] = float(p)
            profiles.append(profile)
        return profiles

class ExternalLCPSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-lcp solver
    for computing equilibria in two-player games using linear complementarity
    programming.
    """
    def solve(self, game, rational=False):
        profiles = [ ]
        command_line = "gambit-lcp -d 10"
        for line in self.launch(command_line, game):
            entries = line.strip().split(",")
            if entries[0] != "NE":  continue
            profile = game.mixed_profile()
            for (i, p) in enumerate(entries[1:]):
                profile[i] = float(p)
            profiles.append(profile)
        return profiles

class ExternalEnumMixedSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-enummixed solver
    for computing equilibria in two-player games using enumeration of extreme points.
    """
    def solve(self, game, rational=False):
        profiles = [ ]
        command_line = "gambit-enummixed -d 10"
        for line in self.launch(command_line, game):
            entries = line.strip().split(",")
            if entries[0] != "NE":  continue
            profile = game.mixed_profile()
            for (i, p) in enumerate(entries[1:]):
                profile[i] = float(p)
            profiles.append(profile)
        return profiles

class ExternalSimpdivSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-simpdiv solver
    for computing equilibria in N-player games using simpicial subdivision.
    """
    def solve(self, game):
        profiles = [ ]
        command_line = "gambit-simpdiv -d 10"
        for line in self.launch(command_line, game):
            entries = line.strip().split(",")
            if entries[0] != "NE":  continue
            profile = game.mixed_profile()
            for (i, p) in enumerate(entries[1:]):
                profile[i] = float(p)
            profiles.append(profile)
        return profiles

class ExternalGlobalNewtonSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-gnm solver
    for computing equilibria in N-player games using the global Newton method.
    """
    def solve(self, game):
        profiles = [ ]
        command_line = "gambit-gnm -d 10"
        for line in self.launch(command_line, game):
            entries = line.strip().split(",")
            if entries[0] != "NE":  continue
            profile = game.mixed_profile()
            for (i, p) in enumerate(entries[1:]):
                profile[i] = float(p)
            profiles.append(profile)
        return profiles

class ExternalEnumPolySolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-enumpoly solver
    for computing equilibria in N-player games systems of polynomial equations.
    """
    def solve(self, game):
        profiles = [ ]
        command_line = "gambit-enumpoly -d 10"
        for line in self.launch(command_line, game):
            entries = line.strip().split(",")
            if entries[0] != "NE":  continue
            profile = game.mixed_profile()
            for (i, p) in enumerate(entries[1:]):
                profile[i] = float(p)
            profiles.append(profile)
        return profiles

class ExternalLyapunovSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-liap solver
    for computing equilibria in N-player games using Lyapunov function minimization.
    """
    def solve(self, game):
        profiles = [ ]
        command_line = "gambit-liap -d 10"
        for line in self.launch(command_line, game):
            entries = line.strip().split(",")
            if entries[0] != "NE":  continue
            profile = game.mixed_profile()
            for (i, p) in enumerate(entries[1:]):
                profile[i] = float(p)
            profiles.append(profile)
        return profiles

class ExternalIteratedPolymatrixSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-ipa solver
    for computing equilibria in N-player games using iterated polymatrix approximation.
    """
    def solve(self, game):
        profiles = [ ]
        command_line = "gambit-ipa -d 10"
        for line in self.launch(command_line, game):
            entries = line.strip().split(",")
            if entries[0] != "NE":  continue
            profile = game.mixed_profile()
            for (i, p) in enumerate(entries[1:]):
                profile[i] = float(p)
            profiles.append(profile)
        return profiles


class ExternalLogitSolver(ExternalSolver):
    """
    Algorithm class to manage calls to external gambit-logit solver
    for computing equilibria in N-player games using quantal response equilibrium.
    """
    def solve(self, game):
        profiles = [ ]
        command_line = "gambit-logit -d 20 -e"
        for line in self.launch(command_line, game):
            entries = line.strip().split(",")
            if entries[0] != "NE":  continue
            profile = game.mixed_profile()
            for (i, p) in enumerate(entries[1:]):
                profile[i] = float(p)
            profiles.append(profile)
        return profiles

