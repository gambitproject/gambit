"""
A set of utilities for computing and analyzing quantal response equilbria
"""

import os, sys
import libgambit

def launch_external_program(prog, game):
    """
    Helper function for launching calls to external programs.
    Calls the specified program 'prog', passing the game to standard
    input in .efg format (if a tree) or .nfg format (if a table).
    Returns the object referencing standard output of the external program.
    """
    child_stdin, child_stdout = os.popen2("%s -q" % prog)
    if game.is_tree():
        child_stdin.write(game.efg_file())
    else:
        child_stdin.write(game.nfg_file())
    # Need to close, or at least flush, stdin of the child, or else
    # processing won't begin...
    child_stdin.close()

    return child_stdout

class MixedQRE(object):
    """
    Container class representing a (strategic) quantal response equilibrium.
    Instrumented to behave as a read-only mixed strategy profile.
    """
    def __init__(self, lam, profile):
        self.profile = profile
        self.lam = lam
        
    def expected_choice(self):
        """
        If the game's strategies have labels which are numeric, this
        returns the expected value of each player's choice under the profile
        """
        return [ sum([self.profile[strategy] * float(strategy.GetLabel())
                      for strategy in player.Strategies()])
                 for player in self.profile.GetGame().Players() ]

    def behavior(self):        return self.profile.behavior()
    def liap_value(self):      return self.profile.GetLiapValue()

    def __len__(self):         return len(self.profile)
    def __getitem__(self, i):  return self.profile[i] 

    def __repr__(self):
        return "QRE with lambda=%f: %s" % (self.lam, repr(self.profile))

    def __str__(self):   return repr(self)
    
class AgentQRE(object):
    """
    Container class representing an (agent) quantal response equilibrium.
    Instrumented to behave as a read-only behavior strategy profile.
    """
    def __init__(self, lam, profile):
        self.profile = profile
        self.lam = lam
        
    def liap_value(self):      return self.profile.GetLiapValue()

    def __len__(self):         return len(self.profile)
    def __getitem__(self, i):  return self.profile[i] 

    def __repr__(self):
        return "QRE with lambda=%f: %s" % (self.lam, repr(self.profile))

    def __str__(self):   return repr(self)
    

def compute_principal_branch(game, strategic=False):
    profiles = [ ]

    if strategic or not game.is_tree():
        game.BuildComputedValues()
        for line in launch_external_program("gambit-logit -S", game):
            if "Bifurcation" in line: continue

            entries = line.split(",")
        
            profile = game.mixed_strategy(rational=False)
            for (i, p) in enumerate(entries[1:]):
                profile[i] = float(p)
            profiles.append(MixedQRE(lam=float(entries[0]), profile=profile))

        return profiles
    else:
        for line in launch_external_program("gambit-logit", game):
            if "Bifurcation" in line: continue

            entries = line.split(",")
        
            profile = game.behavior_strategy(rational=False)
            for (i, p) in enumerate(entries[1:]):
                profile[i] = float(p)
            profiles.append(AgentQRE(lam=float(entries[0]), profile=profile))

        return profiles

def compute_at_lambda(game, lam, strategic=False):
    """
    Compute a good approximation to the QRE on the principal branch
    with parameter lambda.
    """
    if strategic or not game.is_tree():
        game.BuildComputedValues()

        for line in launch_external_program("gambit-logit -S -l %f" % lam,
                                            game):
            if "Bifurcation" in line: continue

            entries = line.split(",")
        
            profile = game.mixed_strategy(rational=False)
            for (i, p) in enumerate(entries[1:]):
                profile[i] = float(p)
            qre = MixedQRE(lam=float(entries[0]), profile=profile)

        return qre
    else:
        for line in launch_external_program("gambit-logit -l %f" % lam, game):
            if "Bifurcation" in line: continue

            entries = line.split(",")
        
            profile = game.behavior_strategy(rational=False)
            for (i, p) in enumerate(entries[1:]):
                profile[i] = float(p)
            qre = AgentQRE(lam=float(entries[0]), profile=profile)

        return qre

    
