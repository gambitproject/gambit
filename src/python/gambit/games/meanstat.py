#
# This file is part of Gambit
# Copyright (c) 1994-2016, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/python/gambit/games/meanstat.py
# Custom implementation of mean-statistic games
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
Representation of mean-statistic games.
"""

import gambit

class Strategy(object):
    def __init__(self, player, st):
        self.player = player
        self.st = st
    def __repr__(self):
        return "<Strategy [%d] '%s' for player [%d] '%s' in game '%s'>" % \
            (self.st, self.label, self.player.pl, self.player.label,
             self.player.game.title)
    def __eq__(self, other):
        if not isinstance(other, Strategy):  return False
        return self.player == other.player and self.st == other.st
    def __ne__(self, other):
        if not isinstance(other, Strategy):  return True
        return self.player != other.player or self.st != other.st
    @property
    def label(self):   return self.player.game.labels[self.st]
        
class Strategies(object):
    def __init__(self, player):
        self.player = player
    def __repr__(self):
        return "<Strategies for player [%d] '%s' in game '%s'>" % \
            (self.player.pl, self.player.label, self.player.game.title)
    def __len__(self):    return len(self.player.game.choices)
    def __getitem__(self, i):
        if not isinstance(i, int) or i < 0 or i >= len(self):
            raise IndexError
        return Strategy(self.player, i)
    
class Player(object):
    def __init__(self, game, pl):
        self.game = game
        self.pl = pl
    def __repr__(self):
        return "<Player [%d] '%s' in game '%s'>" % \
            (self.pl, self.label, self.game.title)
    def __eq__(self, other):
        if not isinstance(other, Player):  return False
        return self.game == other.game and self.pl == other.pl
    def __ne__(self, other):
        if not isinstance(other, Player):  return True
        return self.game != other.game or self.pl != other.pl
    @property
    def strategies(self):    return Strategies(self)
    @property
    def label(self):   return str(self.pl)
    
class Players(object):
    def __init__(self, game):    self.game = game
    def __repr__(self):
        return "<Players in game '%s'>" % self.game.title
    def __len__(self):           return self.game.N
    def __getitem__(self, i):
        if not isinstance(i, int) or i < 0 or i >= self.game.N:
            raise IndexError
        return Player(self.game, i)

class Outcome(object):
    def __init__(self, game, index):
        self.game = game
        self.index = index
    def __repr__(self):
        return "<Strategy profile %s in game '%s'>" % (self.index,
                                                       self.game.title)
    def __eq__(self, other):
        if not isinstance(other, Outcome):  return False
        return self.game == other.game and self.index == other.index
    def __ne__(self, other):
        if not isinstance(other, Outcome):  return True
        return self.game != other.game or self.index != other.index
    def __getitem__(self, pl):
        if not isinstance(pl, int) or pl < 0 or pl >= self.game.N:
            raise IndexError
        total = sum([ self.game.choices[c] for c in self.index ])
        own = self.game.choices[self.index[pl]]
        return self.game.payoff(own, total-own)

class MeanStatisticGame(object):
    """
    A general mean statistic game: a symmetric game in which the
    player's payoff depends on his choice and the sum of the choices
    of the other players.

    This class is abstract in that it depends upon, but does not define,
    the method payoff(own, others), which provides the payoff for
    any given vector of own choice and sum of others' choices.
    """
    def __init__(self, N, min_choice, max_choice, step_choice=1,
                 label_prefix="S"):
        self.N = N
        self.choices = xrange(min_choice, max_choice+1, step_choice)
        self.labels = [ label_prefix+str(c) for c in self.choices ]
        self.statistics = xrange((self.N-1)*min(self.choices),
                                 (self.N-1)*max(self.choices)+1,
                                 step_choice)

    @property
    def is_tree(self):   return False
    @property
    def is_symmetric(self):  return True
    @property
    def players(self):    return Players(self)
    
    def __getitem__(self, key):
        key = list(key)
        if len(key) != self.N:
            raise KeyError("number of strategies != number of players")
        for i in xrange(len(key)):
            if isinstance(key[i], int):
                if key[i] < 0 or key[i] >= len(self.choices):
                    raise IndexError("index %d out of range" % i)
            elif isinstance(key[i], str):
                try:
                    key[i] = [ x.label for x in self.players[i].strategies ].index(key[i])
                except ValueError:
                    raise IndexError("index %d not found" % i)
            elif isinstance(key[i], Strategy):
                if key[i].player != self.players[i]:
                    raise IndexError("strategy %d for wrong player" % i)
                key[i] = key[i].st
            else:
                raise TypeError("unknown index type for index %d" % i)
        return Outcome(self, tuple(key))

    def __setitem__(self, key, value):
        raise NotImplementedError

    def mixed_strategy_profile(self, point=None):
        return MixedStrategyProfile(self, point)

    def to_table(self):
        g = gambit.new_table([ len(self.choices) ] * self.N)
        for pl in xrange(self.N):
            for (st, label) in enumerate(self.labels):
                g.players[pl].strategies[st].label = label
        for cont in g.contingencies:
            g_outc = g[cont]
            t_outc = self[cont]
            for pl in xrange(self.N):
                g_outc[pl] = t_outc[pl]
        return g


class MixedStrategyProfile(object):
    """A (symmetric) mixed strategy profile on a mean statistic game.
    """
    def __init__(self, game, profile=None):
        self.game = game
        if profile is None:
            self.set_centroid()
        else:
            self.profile = profile[:]
    
    def __len__(self):   return len(self.profile)
    def __getitem__(self, i):      return self.profile[i]
    def __setitem__(self, i, value):      self.profile[i] = value
    def __str__(self):  return repr(self)
    def __repr__(self):
        return "Mixed strategy profile on '%s': [%s]" % \
               (self.game.title,
                ", ".join([ str(self[i]) for i in xrange(len(self)) ]))

    def __rmul__(self, fac):
        p = self.game.mixed_strategy_profile()
        for i in xrange(len(self)):
            p[i] = fac * self[i]
        return p

    def __add__(self, other):
        if not hasattr(other, "game") or other.game != self.game:
            raise ValueError("adding a non-MixedProfile to a MixedProfile")
        p = self.game.mixed_strategy_profile()
        for i in xrange(len(self)):
            p[i] = self[i] + other[i]
        return p

    def set_centroid(self):
        self.profile = [ 1.0 / len(self.game.choices) for i in self.game.choices ]

    def normalize(self):
        den = sum(self.profile)
        if den != 0:
            self.profile = [ x/den for x in self.profile ]
                
    def strategy_value(self, st):
        return sum([ prob * self.game.payoff(self.game.choices[st],
                                             self.game.statistics[i])
                     for (i, prob) in
                     enumerate(self.sum_dist(self.game.N-1,
                                             self.profile)) ])

    def strategy_values(self):
        oprob = self.sum_dist(self.game.N-1, self.profile)
        
        return [ sum([ prob * self.game.payoff(st, self.game.statistics[i])
                       for (i, prob) in enumerate(oprob) ])
                 for st in self.game.choices ]

    def strategy_value_deriv(self, st, stOpp):
        return sum([ prob * self.game.payoff(self.game.choices[st],
                                             self.game.statistics[i]+
                                             self.game.choices[stOpp])
                     for (i, prob) in
                     enumerate(self.sum_dist(self.game.N-2,
                                             self.profile)) ])
    
        
        
    def strategy_values_deriv(self):
        oprob = self.sum_dist(self.game.N-2, self.profile)
        
        return [ [ sum([ prob * self.game.payoff(self.game.choices[st],
                                                 self.game.statistics[i]+
                                                 self.game.choices[stOpp])
                         for (i, prob) in enumerate(oprob) ])
                   for stOpp in self.game.choices ]
                 for st in self.game.choices ]

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

