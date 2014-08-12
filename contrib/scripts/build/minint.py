#
# A Python script to test the Gambit Python wrapper by building
# the minimum-unique-integer game
#


import gambit, sys

K = int(sys.argv[2])
N = int(sys.argv[1])
nfg = gambit.NewTable([K for i in range(N)])

# Pre-create outcomes.  Outcome 'i' is the outcome where player 'i' wins.

for pl in xrange(1, nfg.NumPlayers() + 1):
    outcome = nfg.NewOutcome()
    outcome.SetLabel('%d wins' % pl)
    outcome.SetPayoff(pl, "1")

for profile in gambit.StrategyIterator(gambit.StrategySupportProfile(nfg)):
    choices = [ profile.GetStrategy(pl).GetNumber()
                for pl in range(1, nfg.NumPlayers() + 1) ]
    for ch in range(1, K+1):
        if len([x for x in choices if x == ch]) == 1:
            # We have a winner
            profile.SetOutcome(nfg.GetOutcome(choices.index(ch) + 1))
            break
    # If we don't have a winner, leave outcome null, payoffs zero

    
print nfg.AsNfgFile()
