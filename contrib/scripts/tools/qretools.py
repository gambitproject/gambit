"""
A set of utilities for computing and analyzing quantal response equilbria
"""
from __future__ import division

from past.utils import old_div
from builtins import object
import os, sys
import gambit

def LaunchExternal(prog, game):
    """
    Helper function for launching calls to external progams
    """
    child_stdin, child_stdout = os.popen2("%s -q" % prog)
    if game.IsTree():
        child_stdin.write(game.AsEfgFile())
    else:
        child_stdin.write(game.AsNfgFile())
    # Need to close, or at least flush, stdin of the child, or else
    # processing won't begin...
    child_stdin.close()

    return child_stdout

class QRE(object):
    """
    Container class representing a quantal response equilibrium
    """
    def __init__(self, lam, profile):
        self.lam = lam
        self.profile = profile
        
    def ExpectedChoice(self):
        """
        If the game's strategies have labels which are numeric, this
        returns the expected value of each player's choice under the profile
        """
        return [ sum([self.profile[strategy] * float(strategy.GetLabel())
                      for strategy in player.Strategies()])
                 for player in self.profile.GetGame().Players() ]

def ComputeQRE(game):
    solutions = [ ]
    game.BuildComputedValues()
    for line in LaunchExternal("gambit-logit", game).readlines():
        if "Bifurcation" in line: continue

        entries = line.split(",")
        
        soln = game.NewMixedStrategyDouble()
        for (i, p) in enumerate(entries[1:]):
            soln[i+1] = float(p)
        solutions.append(QRE(float(entries[0]), soln))

    return solutions


def PlotAverages(profiles):
    try:
        import pylab, matplotlib, matplotlib.ticker
    except:
        return

    ax = pylab.subplot(111)
    p = pylab.plot([ old_div(qre.lam,(1.0+qre.lam)) for qre in profiles ],
                   [ qre.ExpectedChoice()[0] for qre in profiles ])
    ax.xaxis.set_major_formatter(matplotlib.ticker.FuncFormatter(lambda x, pos: "%.2f" % (old_div(x,(1.0-x)))))
    
    pylab.show()
    
