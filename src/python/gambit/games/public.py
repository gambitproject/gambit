"""
Voluntary public-goods contribution games.
"""

import meanstat

class CobbDouglasVCGame(meanstat.MeanStatisticGame):
    """
    A symmetric public goods game with a Cobb-Douglas payoff specification,
    following, e.g., Andreoni (1993)
    """
    def __init__(self, N, choices, omega, alpha, tax=0):
        meanstat.MeanStatisticGame.__init__(self, N, choices)
        self.omega = omega
        self.alpha = alpha
        self.tax = tax

    @property
    def title(self):
        return "Cobb-Douglas public goods game with N=%d, alpha=%s" % \
               (self.N, self.alpha)

    def payoff(self, own, others):
        return (self.omega-own-self.tax)**self.alpha * \
               (self.N*self.tax+own+others)**(1.0-self.alpha)
        
class QuadraticVCGame(meanstat.MeanStatisticGame):
    """
    A symmetric public goods game with a quadratic payoff specification.
    """
    def __init__(self, N, choices, omega, m, c, tax=0):
        meanstat.MeanStatisticGame.__init__(self, N, choices)
        self.omega = omega
        self.m = m
        self.c = c
        self.tax = tax

    @property
    def title(self):
        return "Quadratic public goods game with N=%d, m=%s, c=%s" % \
               (self.N, self.m, self.c)

    def payoff(self, own, others):
        return self.omega-own-self.tax \
               + self.m*(self.N*self.tax+own+others) \
               - self.c*(self.N*self.tax+own+others)**2
        

