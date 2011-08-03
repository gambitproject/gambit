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

