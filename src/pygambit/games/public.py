#
# This file is part of Gambit
# Copyright (c) 1994-2016, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/python/gambit/games/public.py
# Custom implementation of voluntary public goods contribution games
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#

"""
Voluntary public-goods contribution games.
"""

import meanstat

class CobbDouglasVCGame(meanstat.MeanStatisticGame):
    """
    A symmetric public goods game with a Cobb-Douglas payoff specification,
    following, e.g., Andreoni (1993)
    """
    def __init__(self, N, min_choice, max_choice, step_choice, omega, alpha, tax=0):
        meanstat.MeanStatisticGame.__init__(self, N,
                                            min_choice, max_choice, step_choice)
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
    def __init__(self, N, min_choice, max_choice, step_choice, omega, m, c, tax=0):
        meanstat.MeanStatisticGame.__init__(self, N,
                                            min_choice, max_choice, step_choice)
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
        

