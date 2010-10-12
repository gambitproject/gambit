"""
Implementation of contest games.
"""

import meanstat

class TullockGame(meanstat.MeanStatisticGame):
    """
    A Tullock contest game.
    """
    def __init__(self, N, choices, prize, omega):
        meanstat.MeanStatisticGame.__init__(self, N, choices)
        self.prize = prize
        self.omega = omega

    @property
    def title(self):
        return "Lottery choice game with N=%d, prize=%s, endowment=%s" % \
               (self.N, self.prize, self.omega)

    def payoff(self, own, others):
        try:
            p_win = 1.0 * own / (own+others)
            return p_win * (self.omega - own + self.prize) + \
                   (1.0-p_win) * (self.omega - own)
        except ZeroDivisionError:
            return self.omega

