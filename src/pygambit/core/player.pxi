#
# This file is part of Gambit
# Copyright (c) 1994-2022, The Gambit Project (http://www.gambit-project.org)
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

cdef class Infosets(Collection):
    """The set of information sets at which a player has the decision."""
    cdef c_GamePlayer player
    def __len__(self):
        """The number of information sets at which the player has the decision."""
        return self.player.deref().NumInfosets()

    def __getitem__(self, iset):
        if not isinstance(iset, int):
            return Collection.__getitem__(self, iset)
        cdef Infoset s
        s = Infoset()
        s.infoset = self.player.deref().GetInfoset(iset+1)
        return s


cdef class Strategies(Collection):
    """The set of strategies available to a player."""
    cdef c_GamePlayer player

    def add(self, label="") -> Strategy:
        """Add a new strategy to the set of the player's strategies.

        Returns
        -------
        The newly-created strategy

        Raises
        ------
        TypeError
            If called on a game which has an extensive representation.
        """
        cdef Game g
        g = Game()
        g.game = self.player.deref().GetGame()
        if g.is_tree:
            raise TypeError("Adding strategies is only applicable to games in strategic form")
        cdef Strategy s
        s = Strategy()
        s.strategy = self.player.deref().NewStrategy()
        s.label = str(label)
        return s

    def __len__(self):
        """The number of strategies for the player in the game."""
        return self.player.deref().NumStrategies()

    def __getitem__(self, st):
        if not isinstance(st, int):
            return Collection.__getitem__(self, st)
        cdef Strategy s
        s = Strategy()
        s.strategy = self.player.deref().GetStrategy(st+1)
        return s


cdef class PlayerSupportStrategies(Collection):
    """Represents a collection of strategies for a player in a restriction"""
    cdef Player player
    cdef StrategicRestriction restriction

    def add(self, label=""):
        raise UndefinedOperationError(
            "Adding strategies is only applicable to players in a game, not in a restriction"
        )

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
    """Represents a player in a :py:class:`Game`."""
    cdef c_GamePlayer player
    cdef StrategicRestriction restriction

    def __repr__(self):
        if self.is_chance:
            return f"<Player [CHANCE] in game '{self.game.title}'>"
        else:
            return (
                f"<Player [{self.number}] '{self.label}' "
                f"in game '{self.game.title}'>"
            )

    def __richcmp__(self, other, whichop):
        if isinstance(other, Player):
            if whichop == 2:
                return self.player.deref() == (<Player>other).player.deref()
            elif whichop == 3:
                return self.player.deref() != (<Player>other).player.deref()
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

    @property
    def game(self) -> Game:
        """Gets the :py:class:`Game` to which the player belongs."""
        cdef Game g
        if self.restriction is not None:
            return self.restriction
        g = Game()
        g.game = self.player.deref().GetGame()
        return g

    @property
    def label(self) -> str:
        """Gets or sets the text label of the player."""
        return self.player.deref().GetLabel().decode('ascii')

    @label.setter
    def label(self, value: str) -> None:
        if self.restriction is not None:
            raise UndefinedOperationError("Changing objects in a restriction is not supported")
        # check to see if the player's name has been used elsewhere
        if value in [i.label for i in self.game.players]:
            warnings.warn("Another player with an identical label exists")
        self.player.deref().SetLabel(value.encode('ascii'))

    @property
    def number(self) -> int:
        """Returns the number of the player in its Game.
        Players are numbered starting with 0.
        """
        return self.player.deref().GetNumber() - 1

    @property
    def is_chance(self) -> bool:
        """Returns `True` if the player is the chance player."""
        return self.player.deref().IsChance() != 0

    @property
    def strategies(self) -> Strategies:
        """Returns the set of strategies belonging to the player."""
        cdef Strategies s
        cdef PlayerSupportStrategies ps
        if self.restriction is not None:
            ps = PlayerSupportStrategies(self, self.restriction)
            return ps
        s = Strategies()
        s.player = self.player
        return s

    @property
    def infosets(self) -> Infosets:
        """Returns the set of information sets at which the player has the decision."""
        cdef Infosets s
        s = Infosets()
        s.player = self.player
        return s

    @property
    def min_payoff(self) -> Rational:
        """Returns the smallest payoff for the player in any outcome of the game."""
        cdef Game g
        g = Game()
        g.game = self.player.deref().GetGame()
        return rat_to_py(g.game.deref().GetMinPayoff(self.number + 1))

    @property
    def max_payoff(self) -> Rational:
        """Returns the largest payoff for the player in any outcome of the game."""
        cdef Game g
        g = Game()
        g.game = self.player.deref().GetGame()
        return rat_to_py(g.game.deref().GetMaxPayoff(self.number + 1))

    def unrestrict(self):
        cdef Game g
        g = Game()
        g.game = self.player.deref().GetGame()
        return g.players[self.number]
