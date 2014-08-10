# This script builds a serial cost pricing game
# (See Chen, Razzolini, and Turocy, _Economic Theory_)

import sys
import gambit

# Number of players
N = 4

alphas = [ 48, 54, 58, 60 ]
omegas = [ 60, 20, 0, 0 ]

# Strategy space
strats = range(0, 21)

# Cost function
def Cost(q):  return q*q

def Qsuper(q, i):
    if i == 0:
        return 0
    elif i == 1:
        return N * q[0]
    elif i == 2:
        return q[0] + (N-1)*q[1]
    elif i == 3:
        return q[0] + q[1] + (N-2)*q[2]
    elif i == 4:
        return q[0] + q[1] + q[2] + (N-3)*q[3]
    else:
        raise IndexError
    
# Function mapping own choice and choices of others into payoffs
def Payoff(alpha, omega, own, others):
    # Cost share depends only on lower quantities and one's own
    lower = [ x for x in others if x <= own ]
    lower.sort()
    lower.append(own)

    payoff = gambit.Rational(omega) + gambit.Rational(alpha*own)

    for k in xrange(1, len(lower)+1):
        payoff -= gambit.Rational(Cost(Qsuper(lower, k)) -
                                  Cost(Qsuper(lower, k-1)),
                                  N + 1 - k)
    return payoff

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
