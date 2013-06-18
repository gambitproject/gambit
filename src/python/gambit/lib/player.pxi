#
# This file is part of Gambit
# Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/python/gambit/lib/player.pxi
# Cython wrapper for players
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
from gambit.lib.error import UndefinedOperationError

cdef class Infosets(Collection):
    "Represents a collection of infosets for a player."
    cdef c_GamePlayer player
    def __len__(self):    return self.player.deref().NumInfosets()
    def __getitem__(self, iset):
        if not isinstance(iset, int):  return Collection.__getitem__(self, iset)
        cdef Infoset s
        s = Infoset()
        s.infoset = self.player.deref().GetInfoset(iset+1)
        return s

cdef class Strategies(Collection):
    "Represents a collection of strategies for a player."
    cdef c_GamePlayer player

    def add(self, label=""):
        cdef Game g
        g = Game()
        g.game = self.player.deref().GetGame()
        if g.is_tree:
            raise TypeError, "Adding strategies is only applicable to games in strategic form"
        cdef Strategy s
        s = Strategy()
        s.strategy = self.player.deref().NewStrategy()
        s.label = str(label)
        return s

    def __len__(self):    return self.player.deref().NumStrategies()
    def __getitem__(self, st):
        if not isinstance(st, int):  return Collection.__getitem__(self, st)
        cdef Strategy s
        s = Strategy()
        s.strategy = self.player.deref().GetStrategy(st+1)
        return s

cdef class PlayerSupportStrategies(Collection):
    "Represents a collection of strategies for a player in a restriction"
    cdef Player player
    cdef StrategicRestriction restriction

    def add(self, label=""):
        raise UndefinedOperationError("Adding strategies is only applicable"\
                                      "to players in a game, not in a restriction")

    def __init__(self, Player player not None, StrategicRestriction restriction not None):
        self.restriction = restriction
        self.player = player
    
    def __len__(self):
        return self.restriction.num_strategies_player(self.player.number)
    def __getitem__(self, strat):
        if not isinstance(strat, int):
            return Collection.__getitem__(self, strat)
        cdef Strategy s
        s = Strategy()
        s.strategy = self.restriction.support.GetStrategy(self.player.number+1, strat+1)
        s.restriction = self.restriction
        return s

cdef class Player:
    cdef c_GamePlayer player
    cdef StrategicRestriction restriction

    def __repr__(self):
        if self.is_chance:
            return "<Player [CHANCE] in game '%s'>" % self.game.title
        return "<Player [%d] '%s' in game '%s'>" % (self.number,
                                                    self.label,
                                                    self.game.title)
    
    def __richcmp__(Player self, other, whichop):
        if isinstance(other, Player):
            if whichop == 2:
                return self.player.deref() == ((<Player>other).player).deref()
            elif whichop == 3:
                return self.player.deref() != ((<Player>other).player).deref()
            else:
                raise NotImplementedError
        else:
            if whichop == 2:
                return False
            elif whichop == 3:
                return True
            else:
                raise NotImplementedError

    def __hash__(self):
        return long(<long>self.player.deref())

    property game:
        def __get__(self):
            cdef Game g
            if self.restriction is not None:
                return self.restriction
            g = Game()
            g.game = self.player.deref().GetGame() 
            return g

    property label:
        def __get__(self):
            return self.player.deref().GetLabel().c_str()

        def __set__(self, char *value):
            if self.restriction is not None:
                raise UndefinedOperationError("Changing objects in a restriction is not supported")
            # check to see if the player's name has been used elsewhere
            if value in [ i.label for i in self.game.players ]:
                warnings.warn("Another player with an identical label exists")
            cdef cxx_string s
            s.assign(value)
            self.player.deref().SetLabel(s)

    property number:
        def __get__(self):
            return self.player.deref().GetNumber() - 1

    property is_chance:
        def __get__(self):
            return True if self.player.deref().IsChance() != 0 else False

    property strategies:
        def __get__(self):
            cdef Strategies s
            cdef PlayerSupportStrategies ps
            if self.restriction is not None:
                ps = PlayerSupportStrategies(self, self.restriction)
                return ps
            s = Strategies()
            s.player = self.player
            return s
        
    property infosets:
        def __get__(self):
            cdef Infosets s
            s = Infosets()
            s.player = self.player
            return s

    property min_payoff:
        def __get__(self):
            cdef Game g
            cdef cxx_string s
            g = Game()
            g.game = self.player.deref().GetGame()
            s = rat_str(g.game.deref().GetMinPayoff(self.number + 1))
            return fractions.Fraction(s.c_str())

    property max_payoff:
        def __get__(self):
            cdef Game g
            cdef cxx_string s
            g = Game()
            g.game = self.player.deref().GetGame()
            s = rat_str(g.game.deref().GetMaxPayoff(self.number + 1))
            return fractions.Fraction(s.c_str())

    def unrestrict(self):
        cdef Game g
        g = Game()
        g.game = self.player.deref().GetGame()
        return g.players[self.number]
