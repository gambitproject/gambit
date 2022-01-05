#
# This file is part of Gambit
# Copyright (c) 1994-2016, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/python/gambit/games/contest.py
# Custom implementation of contest games
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
Implementation of contest games.
"""

import meanstat
from gambit.lib.libgambit import Rational

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
            if isinstance(own, int):
                p_win = Rational(own, own+others)
                p_lose = Rational(others, own+others)
            else:
                p_win = 1.0 * own / (own+others)
                p_lose = 1.0 * others / (own+others)
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

