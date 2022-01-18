"""
Implementation of mean statistic games for Gambit
"""

import csv
import gambit

def cartesian(L, *lists):
    """
    Generates the Cartesian product of an arbitrary number of lists.
    """
    if not lists:
        for x in L:
            yield (x,)
    else:
        for x in L:
            for y in cartesian(lists[0], *lists[1:]):
                yield (x,)+y


class MSGame:
    """
    A general mean statistic game: a symmetric game in which the
    player's payoff depends on his choice and the sum of the choices
    of the other players.

    This class is abstract in that it depends upon, but does not define,
    the method Payoff(own, others), which provides the payoff for
    any given vector of own choice and sum of others' choices.
    """
    def __init__(self, N, contribs):
        self.N = N
        self.contribs = contribs
        self.statistics = xrange((self.N-1)*min(self.contribs),
                                 (self.N-1)*max(self.contribs)+1)
    def AsTable(self):
        return [ [ own, others, self.Payoff(own, others) ]
                 for (own, others) in cartesian(self.contribs,
                                                self.statistics) ]
    
    def AsMatrix(self):
        return [ [ "" ] + [ other for other in self.statistics ] ] + \
               [ [ own ] + [ self.Payoff(own, other)
                             for other in self.statistics ]
                 for own in self.statistics ]
            
    def AsNfg(self):
        game = gambit.NewTable([ len(self.contribs) for i in xrange(self.N) ])

        # Label each strategy by its contribution
        for (pl, player) in enumerate(game.Players()):
            for (st, strategy) in enumerate(player.Strategies()):
                strategy.SetLabel(str(self.contribs[st]))


        for cont in gambit.StrategyIterator(gambit.StrategySupport(game)):
            # Create and attach an outcome to this contingency
            outcome = game.NewOutcome()
            cont.SetOutcome(outcome)
            # This is the vector of choices made in this contingency
            choices = [ int(cont.GetStrategy(pl+1).GetLabel())
                        for pl in xrange(self.N) ]
            for pl in xrange(self.N):
                outcome.SetPayoff(pl+1,
                                  self.Payoff(choices[pl],
                                              sum(choices)-choices[pl]))

        return game

    def __getitem__(self, key):
        if len(key) != 2:
            raise KeyError
        return self.Payoff(key[0], key[1])

    def __setitem__(self, key, value):
        raise NotImplementedError

class MSTableGame(MSGame):
    """
    A mean statistic game implemented as a table, indexed by
    (own choice, sum of others' choices).
    """
    def __init__(self, N, contribs, table=None):
        MSGame.__init__(self, N, contribs)
        self.payoffs = { }
        if table is not None:
            for row in table:
                self.payoffs[(row[0], row[1])] = row[2]

    def __setitem__(self, key, value):
        if len(key) != 2 or \
               key[0] not in self.contribs or key[1] not in self.statistics:
            raise IndexError
        self.payoffs[(key[0], key[1])] = value

    def Payoff(self, own, others):
        if own not in self.contribs or others not in self.statistics:
            raise KeyError

        try:
            return self.payoffs[(own, others)]
        except KeyError:
            return 0.0
        
            


class CobbDouglasPGGame(MSGame):
    """
    A symmetric public goods game with a Cobb-Douglas payoff specification,
    following, e.g., Andreoni (1993)
    """
    def __init__(self, N, contribs, omega, alpha, tax=0):
        MSGame.__init__(self, N, contribs)
        self.omega = omega
        self.alpha = alpha
        self.tax = tax

    def Payoff(self, own, others):
        return (self.omega-own-self.tax)**self.alpha * \
               (self.N*self.tax+own+others)**(1.0-self.alpha)
        
class QuadraticPGGame(MSGame):
    """
    A symmetric public goods game with a quadratic payoff specification.
    """
    def __init__(self, N, contribs, omega, m, c, tax=0):
        MSGame.__init__(self, N, contribs)
        self.omega = omega
        self.m = m
        self.c = c
        self.tax = tax

    def Payoff(self, own, others):
        return self.omega-own-self.tax \
               + self.m*(self.N*self.tax+own+others) \
               - self.c*(self.N*self.tax+own+others)**2
        

class LotteryChoiceGame(MSGame):
    """
    A lottery entry game, where the probability of winning a prize is equal to
    a player's choice 'x' divided by the sum of all choices.
    """
    def __init__(self, N, choices, prize, omega):
        MSGame.__init__(self, N, choices)
        self.omega = omega

    def Payoff(own, others):
        try:
            return self.omega - own + self.price * own / (own+others)
        except ZeroDivisionError:
            return self.omega

