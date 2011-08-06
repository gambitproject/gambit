"""
Implementation of contest games.
"""

import meanstat
import fractions

class TullockGame(meanstat.MeanStatisticGame):
    """
    A Tullock contest game.
    """
    def __init__(self, N, min_choice, max_choice, step_choice,
                 prize, omega, cost=lambda e: e):
        meanstat.MeanStatisticGame.__init__(self, N,
                                            min_choice, max_choice, step_choice)
        self.prize = prize
        self.omega = omega
        self.cost = cost

    @property
    def title(self):
        return "Lottery choice game with N=%d, prize=%s, endowment=%s" % \
               (self.N, self.prize, self.omega)

    def payoff(self, own, others):
        try:
            p_win = fractions.Fraction(own, own+others)
            p_lose = fractions.Fraction(others, own+others)
            return p_win * (self.omega - self.cost(own) + self.prize) + \
                   p_lose * (self.omega - self.cost(own))
        except ZeroDivisionError:
            return self.omega


import math

class GeneralTullockGame(meanstat.MeanStatisticGame):
    """A contest game with general 'r' for two players."""
    def __init__(self, min_choice, max_choice, step_choice, prize, omega, r=1.0,
                 cost=lambda e: e):
        # Note that this is only a mean-statistic game (trivially!) for two players.
        meanstat.MeanStatisticGame.__init__(self, 2,
                                            min_choice, max_choice, step_choice)
        self.prize = prize
        self.omega = omega
        self.r = r
        self.cost = cost

    @property
    def title(self):
        return "GeneralTullockGame with N=2, prize=%s, endowment=%s, r=%s" % \
               (self.prize, self.omega, self.r)

    def payoff(self, own, other):
        if self.r == 0.0:
            if own > other:
                p_own = 1.0
                p_other = 0.0
            elif own < other or own == 0:
                p_own = 0.0
                p_other = 1.0
            else:
                p_own = 0.5
                p_other = 0.5
        else:
            p_own = math.pow(own, self.r)
            p_other = math.pow(other, self.r)
        try:
            p_win = p_own / (p_own + p_other)
            return p_win * (self.omega - self.cost(own) + self.prize) + \
                   (1.0-p_win) * (self.omega - self.cost(own))
        except ZeroDivisionError:
            return self.omega

