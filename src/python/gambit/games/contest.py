"""
Implementation of contest games.
"""

import meanstat

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
            p_win = 1.0 * own / (own+others)
            return p_win * (self.omega - self.cost(own) + self.prize) + \
                   (1.0-p_win) * (self.omega - self.cost(own))
        except ZeroDivisionError:
            return self.omega

