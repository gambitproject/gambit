#
# This file is part of Gambit
# Copyright (c) 1994-2022, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/python/gambit/lib/stratspt.pxi
# Cython wrapper for strategy supports
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

import functools
from cython.operator cimport dereference as deref

cdef class StrategySupportProfile(Collection):
    """A set-like object representing a subset of the strategies in game.
    A StrategySupportProfile always contains at least one strategy for each player
    in the game.
    """
    cdef c_StrategySupportProfile *support

    def __init__(self, strategies, Game game not None):
        if len(set([strat.player.number for strat in strategies])) != len(game.players):
            raise ValueError(
               "A StrategySupportProfile must have at least one strategy for each player"
            )
        # There's at least one strategy for each player, so this forms a valid support profile
        self.support = new c_StrategySupportProfile((<Game>game).game)
        for strategy in game.strategies:
            if strategy not in strategies:
                self.support.RemoveStrategy((<Strategy>strategy).strategy)

    def __dealloc__(self):
        del self.support

    @property
    def game(self) -> Game:
        """The `Game` on which the support profile is defined."""
        cdef Game g
        g = Game()
        g.game = self.support.GetGame()
        return g

    def __len__(self) -> int:
        """Returns the total number of strategies in the support profile."""
        return self.support.MixedProfileLength()

    def __richcmp__(self, other: typing.Any, whichop: int) -> bool:
        if isinstance(other, StrategySupportProfile):
            if whichop == 1:
                return self.issubset(other)
            elif whichop == 2:
                return deref(self.support) == deref((<StrategySupportProfile>other).support)
            elif whichop == 3:
                return deref(self.support) != deref((<StrategySupportProfile>other).support)
            elif whichop == 5:
                return self.issuperset(other)
            else:
                raise NotImplementedError
        else:
            if whichop == 2:
                return False
            elif whichop == 3:
                return True
            else:
                raise NotImplementedError

    def __getitem__(self, index: int) -> Strategy:
        cdef Strategy s
        for pl in range(len(self.game.players)):
            if index < self.support.NumStrategiesPlayer(pl+1):
                s = Strategy()
                s.strategy = self.support.GetStrategy(pl+1, index+1)
                return s
            index = index - self.support.NumStrategiesPlayer(pl+1)
        raise IndexError("StrategySupportProfile index out of range")

    def __iter__(self) -> typing.Generator[Strategy, None, None]:
        cdef Strategy s
        for pl in range(len(self.game.players)):
            for st in range(self.support.NumStrategiesPlayer(pl+1)):
                s = Strategy()
                s.strategy = self.support.GetStrategy(pl+1, st+1)
                yield s

    def __and__(self, other: StrategySupportProfile) -> StrategySupportProfile:
        return self.intersection(other)

    def __or__(self, other: StrategySupportProfile) -> StrategySupportProfile:
        return self.union(other)

    def __sub__(self, other: StrategySupportProfile) -> StrategySupportProfile:
        return self.difference(other)

    def remove(self, strategy: Strategy) -> StrategySupportProfile:
        """Creates a new support profile without the given strategy.

        Parameters
        ----------
        strategy : Strategy
            The strategy to remove from the profile.

        Raises
        ------
        UndefinedOperationError
            If `strategy` is the only strategy in the support for its player.
        """
        if len(list(filter(lambda x: x.player == strategy.player, self))) > 1:
            strategies = list(self)[:]
            strategies.remove(strategy)
            return StrategySupportProfile(strategies, self.game)
        else:
            raise UndefinedOperationError("remove(): cannot remove last strategy of a player")

    def difference(self, other: StrategySupportProfile) -> StrategySupportProfile:
        """Create a support profile which contains all strategies in this profile that
        are not in `other`.

        Parameters
        ----------
        other : StrategySupportProfile
            The support profile to subtract from this one.

        Returns
        -------
        StrategySupportProfile
            The support profile resulting from the operation.
        """
        return StrategySupportProfile(set(self) - set(other), self.game)

    def intersection(self, other: StrategySupportProfile) -> StrategySupportProfile:
        """Create a support profile which contains all strategies that are in both this and
        another profile.

        Parameters
        ----------
        other : StrategySupportProfile
            The support profile to intersect with this one.

        Returns
        -------
        StrategySupportProfile
            The support profile resulting from the operation.
        """
        return StrategySupportProfile(set(self) & set(other), self.game)

    def union(self, other: StrategySupportProfile) -> StrategySupportProfile:
        """Create a support profile which contains all strategies that are in either this or
        another profile.

        Parameters
        ----------
        other : StrategySupportProfile
            The other support profile to add to this one.

        Returns
        -------
        StrategySupportProfile
            The support profile resulting from the operation.
        """
        return StrategySupportProfile(set(self) | set(other), self.game)

    def issubset(self, other: StrategySupportProfile) -> bool:
        """Test for whether this support is contained in another.

        Parameters
        ----------
        other : StrategySupportProfile
            The other support profile to compare to.

        Returns
        -------
        bool
            `True` if every strategy in the profile is also in `other`.
        """
        return functools.reduce(lambda acc,st: acc & (st in other), self, True)

    def issuperset(self, other: StrategySupportProfile) -> bool:
        """Test for whether annother support is contained in this one.

        Parameters
        ----------
        other : StrategySupportProfile
            The other support profile to compare to.

        Returns
        -------
        bool
            `True` if every strategy in `other` is in this profile.
        """

    def restrict(self) -> StrategicRestriction:
        """Creates a `StrategicRestriction`, which is a restriction of the game
        in which only the strategies in this profile are present.
        """
        cdef StrategicRestriction restriction
        restriction = StrategicRestriction()
        restriction.support = new c_StrategySupportProfile(deref(self.support))
        return restriction

    def undominated(self, strict=False, external=False):
        cdef StrategicRestriction restriction
        restriction = StrategicRestriction()
        restriction.support = new c_StrategySupportProfile(self.support.Undominated(strict, external))
        new_profile = StrategySupportProfile(restriction.strategies, self.game)
        return new_profile


cdef class RestrictionOutcomes(Collection):
    """Represents a collection of outcomes in a restriction."""
    cdef StrategicRestriction restriction

    def __init__(self, StrategicRestriction restriction not None):
        self.restriction = restriction
    def __len__(self):    return (<Game>self.restriction.unrestrict()).game.deref().NumOutcomes()
    def __getitem__(self, outc):
        if not isinstance(outc, int):  return Collection.__getitem__(self, outc)
        cdef Outcome c
        c = Outcome()
        c.outcome = (<Game>self.restriction.unrestrict()).game.deref().GetOutcome(outc+1)
        c.restriction = self.restriction
        return c

    def add(self, label=""):
        raise UndefinedOperationError("Changing objects in a restriction is not supported")

cdef class RestrictionStrategies(Collection):
    """Represents a collection of strategies in a restriction."""
    cdef StrategicRestriction restriction

    def __init__(self, StrategicRestriction restriction not None):
        self.restriction = restriction
    def __len__(self):    return self.restriction.support.MixedProfileLength()
    def __getitem__(self, strat):
        if not isinstance(strat, int):
            return Collection.__getitem__(self, strat)
        cdef Array[int] num_strategies
        cdef Strategy s
        num_strategies = self.restriction.support.NumStrategies()
        for i in range(1,num_strategies.Length()+1):
            if strat - num_strategies.getitem(i) < 0:
                s = Strategy()
                s.strategy = self.restriction.support.GetStrategy(i, strat+1)  
                s.restriction = self.restriction
                return s
            strat = strat - num_strategies.getitem(i)
        raise IndexError("Index out of range")


cdef class StrategicRestriction:
    """
    A StrategicRestriction is a read-only view on a game, defined by a
    subset of the strategies on the original game.
    """
    cdef c_StrategySupportProfile *support

    def __init__(self):
        self.support = (<c_StrategySupportProfile *>0)
    def __dealloc__(self):
        if self.support != (<c_StrategySupportProfile *>0):
            del self.support
    def __repr__(self):
        return self.write()

    def __richcmp__(StrategicRestriction self, other, whichop):
        if isinstance(other, StrategicRestriction):
            if whichop == 2:
                return deref(self.support) == deref((<StrategicRestriction>other).support)
            elif whichop == 3:
                return deref(self.support) != deref((<StrategicRestriction>other).support)
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
        return long(<long>&self.support)

    property title:
        def __get__(self):
            return "Restriction from Game '%s'" % self.unrestrict().title

    property players:
        def __get__(self):
            cdef Players p
            p = Players()
            p.game = (<Game>self.unrestrict()).game
            p.restriction = self
            return p

    property strategies:
        def __get__(self):
            cdef RestrictionStrategies s
            s = RestrictionStrategies(self)
            return s

    property outcomes:
        def __get__(self):
            cdef RestrictionOutcomes o
            o = RestrictionOutcomes(self)
            return o

    property is_tree:
        def __get__(self):
            # Any strategic restriction is automatically not a tree
            # representation, even if the parent game does have one.
            return False

    property is_const_sum:
        def __get__(self):
            return self.unrestrict().is_const_sum

    property is_perfect_recall:
        def __get__(self):
            return self.unrestrict().is_perfect_recall

    property min_payoff:
        def __get__(self):
            return self.unrestrict().min_payoff

    property max_payoff:
        def __get__(self):
            return self.unrestrict().max_payoff

    def write(self, format='native'):
        if format != 'native' and format != 'nfg':
            raise NotImplementedError
        else:
            return WriteGame(deref(self.support)).c_str()

    def undominated(self, strict=False):
        cdef StrategicRestriction new_restriction
        new_restriction = StrategicRestriction()
        new_restriction.support = new c_StrategySupportProfile(self.support.Undominated(strict, False))
        return new_restriction

    def num_strategies_player(self, pl):
        return self.support.NumStrategiesPlayer(pl+1)

    def support_profile(self):
        return StrategySupportProfile(list(self.strategies), self.unrestrict())

    def unrestrict(self):
        cdef Game g
        g = Game()
        g.game = self.support.GetGame()
        return g

    ###
    # Methods below here have been borrowed from the basic Game
    # implementation, and may not be fully correct in handling
    # all cases
    ###

    def _get_contingency(self, *args):
        cdef c_PureStrategyProfile *psp
        cdef Outcome outcome
        psp = new c_PureStrategyProfile(deref(self.support).GetGame().deref().NewPureStrategyProfile())
        
        
        for (pl, st) in enumerate(args):
            psp.deref().SetStrategy(deref(self.support).GetStrategy(pl+1, st+1))

        outcome = Outcome()
        outcome.outcome = psp.deref().GetOutcome()
        del psp
        return outcome

    # As of Cython 0.11.2, cython does not support the * notation for the argument
    # to __getitem__, which is required for multidimensional slicing to work. 
    # We work around this by providing a shim.
    def __getitem__(self, i):
        try:
            if len(i) != len(self.players):
                raise KeyError, "Number of strategies is not equal to the number of players"
        except TypeError:
            raise TypeError, "contingency must be a tuple-like object"
        cont = [ 0 ] * len(self.players)
        for (pl, st) in enumerate(i):
            if isinstance(st, int):
                if st < 0 or st >= len(self.players[pl].strategies):
                    raise IndexError, "Provided strategy index %d out of range for player %d" % (st, pl)
                cont[pl] = st
            elif isinstance(st, str):
                try:
                    cont[pl] = [ s.label for s in self.players[pl].strategies ].index(st)
                except ValueError:
                    raise IndexError, "Provided strategy label '%s' not defined" % st
            elif isinstance(st, Strategy):
                try:
                    cont[pl] = list(self.players[pl].strategies).index(st)
                except ValueError:
                    raise IndexError, "Provided strategy '%s' not available to player" % st
            else:
                raise TypeError("Must use a tuple of ints, strategy labels, or strategies")
        return self._get_contingency(*tuple(cont))

    def mixed_strategy_profile(self, rational=False):
        cdef MixedStrategyProfileDouble mspd
        cdef MixedStrategyProfileRational mspr
        cdef c_Rational dummy_rat
        if not rational:
            mspd = MixedStrategyProfileDouble()
            mspd.profile = new c_MixedStrategyProfileDouble(deref(self.support).NewMixedStrategyProfileDouble())
            return mspd
        else:
            mspr = MixedStrategyProfileRational()
            mspr.profile = new c_MixedStrategyProfileRational(deref(self.support).NewMixedStrategyProfileRational())
            return mspr
