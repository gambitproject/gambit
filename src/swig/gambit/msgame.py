"""
Implementation of mean statistic games for Gambit
"""

import csv
import libgambit

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


class MeanStatisticGame(object):
    """
    A general mean statistic game: a symmetric game in which the
    player's payoff depends on his choice and the sum of the choices
    of the other players.

    This class is abstract in that it depends upon, but does not define,
    the method payoff(own, others), which provides the payoff for
    any given vector of own choice and sum of others' choices.
    """
    def __init__(self, N, choices):
        self.N = N
        self.choices = choices
        self.statistics = xrange((self.N-1)*min(self.choices),
                                 (self.N-1)*max(self.choices)+1)
    def table(self):
        return [ [ own, others, self.payoff(own, others) ]
                 for (own, others) in cartesian(self.choices,
                                                self.statistics) ]
    
    def matrix(self):
        return [ [ "" ] + [ other for other in self.statistics ] ] + \
               [ [ own ] + [ self.payoff(own, other)
                             for other in self.statistics ]
                 for own in self.choices ]

    def nfg(self):
        game = libgambit.NewTable([ len(self.choices) for i in xrange(self.N) ])

        # Label each strategy by its contribution
        for (pl, player) in enumerate(game.Players()):
            for (st, strategy) in enumerate(player.Strategies()):
                strategy.SetLabel(str(self.choices[st]))


        for cont in game.contingencies():
            # Create and attach an outcome to this contingency
            outcome = game.NewOutcome()
            cont.SetOutcome(outcome)
            # This is the vector of choices made in this contingency
            choices = [ int(cont.GetStrategy(pl+1).GetLabel())
                        for pl in xrange(self.N) ]
            for pl in xrange(self.N):
                outcome.SetPayoff(pl+1,
                                  self.payoff(choices[pl],
                                              sum(choices)-choices[pl]))

        return game

    def __getitem__(self, key):
        if len(key) != 2:
            raise KeyError
        return self.payoff(key[0], key[1])

    def __setitem__(self, key, value):
        raise NotImplementedError

    def mixed_strategy(self, point=None):
        return MSMixedProfile(self, point)
            

class MeanStatisticTableGame(MeanStatisticGame):
    """
    A mean statistic game implemented as a table, indexed by
    (own choice, sum of others' choices).
    """
    def __init__(self, N, choices, table=None, matrix=None):
        MeanStatisticGame.__init__(self, N, choices)
        self.payoffs = { }
        if table is not None:
            for row in table:
                self.payoffs[(int(row[0]), int(row[1]))] = float(row[2])
        elif matrix is not None:
            for row in matrix[1:]:
                for (col, payoff) in zip(matrix[0][1:], row[1:]):
                    self.payoffs[(int(row[0]), int(col))] = float(payoff)

    def __setitem__(self, key, value):
        if len(key) != 2 or \
               key[0] not in self.choices or key[1] not in self.statistics:
            raise IndexError
        self.payoffs[(key[0], key[1])] = value

    def title(self):
        return "Mean statistic table game"

    def payoff(self, own, others):
        if own not in self.choices or others not in self.statistics:
            raise KeyError

        try:
            return self.payoffs[(own, others)]
        except KeyError:
            return 0.0
        
            


class CobbDouglasPGGame(MeanStatisticGame):
    """
    A symmetric public goods game with a Cobb-Douglas payoff specification,
    following, e.g., Andreoni (1993)
    """
    def __init__(self, N, choices, omega, alpha, tax=0):
        MeanStatisticGame.__init__(self, N, choices)
        self.omega = omega
        self.alpha = alpha
        self.tax = tax

    def title(self):
        return "Cobb-Douglas public goods game with N=%d, alpha=%s" % \
               (self.N, self.alpha)

    def payoff(self, own, others):
        return (self.omega-own-self.tax)**self.alpha * \
               (self.N*self.tax+own+others)**(1.0-self.alpha)
        
class QuadraticPGGame(MeanStatisticGame):
    """
    A symmetric public goods game with a quadratic payoff specification.
    """
    def __init__(self, N, choices, omega, m, c, tax=0):
        MeanStatisticGame.__init__(self, N, choices)
        self.omega = omega
        self.m = m
        self.c = c
        self.tax = tax

    def title(self):
        return "Quadratic public goods game with N=%d, m=%s, c=%s" % \
               (self.N, self.m, self.c)

    def payoff(self, own, others):
        return self.omega-own-self.tax \
               + self.m*(self.N*self.tax+own+others) \
               - self.c*(self.N*self.tax+own+others)**2
        

class LotteryChoiceGame(MeanStatisticGame):
    """
    A lottery entry game, where the probability of winning a prize is equal to
    a player's choice 'x' divided by the sum of all choices.
    """
    def __init__(self, N, choices, prize, omega):
        MeanStatisticGame.__init__(self, N, choices)
        self.prize = prize
        self.omega = omega

    def title(self):
        return "Lottery choice game with N=%d, prize=%s, endowment=%s" % \
               (self.N, self.prize, self.omega)

    def payoff(self, own, others):
        try:
            return self.omega - own + 1.0 * self.prize * own / (own+others)
        except ZeroDivisionError:
            return self.omega



class MSMixedProfile:
    """
    A (symmetric) mixed strategy profile on a mean statistic game
    """
    def __init__(self, game, profile=None):
        self.game = game
        if profile is None:
            self.profile = [ 1.0 / len(game.choices) for i in game.choices ]
        else:
            self.profile = profile[:]
    
    def __len__(self):   return len(self.profile)
    def __getitem__(self, i):      return self.profile[i]
    def __setitem__(self, i, value):      self.profile[i] = value
    def __str__(self):  return repr(self)
    def __repr__(self):
        return "Mixed strategy profile on '%s': [%s]" % \
               (self.game.title(),
                ", ".join([ str(self[i]) for i in xrange(len(self)) ]))

    
    def strategy_value(self, st):
        return sum([ prob * self.game.payoff(st,
                                             i+(self.game.N-1)*min(self.game.choices))
                     for (i, prob) in
                     enumerate(self.sum_dist(self.game.N-1,
                                             self.profile)) ])

    def strategy_value_deriv(self, st, stOpp):
        return sum([ prob * self.game.payoff(st,
                                             i+stOpp+
                                             (self.game.N-2)*min(self.game.choices))
                     for (i, prob) in
                     enumerate(self.sum_dist(self.game.N-2,
                                             self.profile)) ])

    def sum_dist(self, K, prob):
        """
        Computes the distribution of the sum of K independent choices from
        probability distribution prob
        """
        if K == 0:
            return [ 1.0 ]
        elif K == 1:
            return prob
        elif K == 2:
            v = [ 0.0 for i in xrange(2*len(prob)-1) ]
            for (i, p) in enumerate(prob):
                for (j, q) in enumerate(prob):
                    v[i+j] += p*q
            return v
        else:
            K1 = K / 2
            K2 = K / 2 + K % 2
            v1 = self.sum_dist(K1, prob)
            v2 = self.sum_dist(K2, prob)
            v = [ 0.0 for i in xrange(len(v1)+len(v2)-1) ]
            for (i, p) in enumerate(v1):
                for (j, q) in enumerate(v2):
                    v[i+j] += p*q
            return v


