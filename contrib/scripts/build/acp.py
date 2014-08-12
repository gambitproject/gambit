# This script builds an average cost pricing game
# (See Chen, Razzolini and Turocy, _Economic Theory_)

import sys
import gambit

# Number of players
N = 4

alphas = [ 48, 54, 58, 60 ]
omegas = [ 180, 102, 0, 0 ]

# Strategy space
strats = range(0, 21)

# Cost function
def Cost(q):  return q*q

# Function mapping own choice and choices of others into payoffs
def Payoff(alpha, omega, own, others):
    total = own + sum(others)

    #print own, others
    if own == 0: return omega

    return (gambit.Rational(omega) + gambit.Rational(alpha*own) -
            gambit.Rational(own, total) * gambit.Rational(Cost(total)))

game = gambit.NewTable([ len(strats) for i in xrange(N) ])

# Label each strategy
for pl in xrange(N):
    player = game.GetPlayer(pl+1)
    for st in xrange(player.NumStrategies()):
        strategy = player.GetStrategy(st+1)
        strategy.SetLabel(str(strats[st]))


for (i, cont) in enumerate(gambit.StrategyIterator(gambit.StrategySupportProfile(game))):
    if i % 1000 == 0: sys.stderr.write("%d\n" % i)
    # Create and attach an outcome to this contingency
    outcome = game.NewOutcome()
    cont.SetOutcome(outcome)
    # This is the vector of choices made in this contingency
    choices = [ int(cont.GetStrategy(pl+1).GetLabel())
                for pl in xrange(N) ]
    for pl in xrange(N):
        pay = Payoff(alphas[pl], omegas[pl], choices[pl],
                     choices[:pl] + choices[pl+1:])
        #print pay
        outcome.SetPayoff(pl+1, pay)

print game.AsNfgFile()
