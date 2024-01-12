#
# FILE: randomnfg.py -- Create random normal form games in Gambit
#
# DESCRIPTION:
# This Python script creates random normal form games of a given dimension
# for Gambit.
#
# When run on its own, the script is invoked with three parameters:
# randomnfg.py ngames dim1 dim2 ... dimN
#
# The script will generate 'ngames' games of the specified dimension, with
# filenames of the form 'prefix-number.nfg'
#
# Payoffs are independent and distributed according to the standard
# normal distribution.  To change the distribution, replace the
# call to random.normalvariate() in line 68.
#

import random
import gambit

def CrossProduct(args):
   ans = [[]]
   for arg in args:
      ans = [x+[y] for x in ans for y in arg]
   return ans

def CreateNfg(dim):
    """
    Create a new normal form game with the given dimensions
    and assign outcomes at all contingencies
    """
    nfg = gambit.NewTable(dim)
    profile = gambit.PureStrategyProfile(nfg)

    for cont in CrossProduct([range(1, nfg.GetPlayer(pl).NumStrategies() + 1)
                              for pl in range(1, nfg.NumPlayers() + 1)]):
        map(lambda pl:
            profile.SetStrategy(nfg.GetPlayer(pl).GetStrategy(cont[pl-1])),
            range(1, nfg.NumPlayers() + 1))
        profile.SetOutcome(nfg.NewOutcome())
    return nfg


def CorrelatedNormal(rho):
    z1 = random.normalvariate(0, 1)
    z2 = random.normalvariate(0, 1)
    return z1, z1*rho + z2*sqrt(1.0-rho*rho)

def RandomizeGame(game, func):
    """
    Randomize the payoffs at the outcomes of a game.
    func is a pointer to a function that produces a (pseudo)random
    number with the desired distribution.
    """

    for outc in xrange(1, game.NumOutcomes() + 1):
        for pl in xrange(1, game.NumPlayers() + 1):
           game.GetOutcome(outc).SetPayoff(pl, func())


if __name__ == '__main__':
    import sys
    nfg = CreateNfg([eval(sys.argv[i]) for i in range(2, len(sys.argv))])

    for iter in xrange(eval(sys.argv[1])):
        RandomizeGame(nfg, lambda: random.normalvariate(0, 1))
        print nfg.AsNfgFile()
