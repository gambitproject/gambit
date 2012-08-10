import itertools
from cython.operator cimport dereference as deref
from gambit.lib.error import UndefinedOperationError

cdef class SupportSet(Collection):
    "Represents a collection of strategies of a game."
    cdef list strategies
    cdef int num_players

    def __cinit__(self):
        self.strategies = []
    def __init__(self, strategies, num_players):
        if self.is_valid(strategies, num_players):
            self.strategies = list(strategies)
            self.num_players = num_players
        else:
            raise ValueError("invalid set of strategies")
    def __len__(self):    return len(self.strategies)
    def __richcmp__(self, other, whichop):
        if isinstance(other, SupportSet):
            if whichop == 1:
                return self.issubset(other)
            elif whichop == 2:
                return (self >= other) & (self <= other)
            elif whichop == 3:
                return not (self >= other) & (self <= other)
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
    def __getitem__(self, strat):
        if not isinstance(strat, int):
            return Collection.__getitem__(self, strat)
        return self.strategies[strat]
    # Set-like methods
    def __and__(self, other):
        if isinstance(other, SupportSet):
            return self.intersection(other)
        raise NotImplementedError
    def __or__(self, other):
        if isinstance(other, SupportSet):
            return self.union(other)
        raise NotImplementedError
    def __sub__(self, other):
        if isinstance(other, SupportSet):
            return self.difference(other)
        raise NotImplementedError

    def remove(self, strategy):
        if isinstance(strategy, Strategy):
            if len(filter(lambda x: x.player == strategy.player, self.strategies)) > 1:
                strategies = list(self.strategies)[:]
                strategies.remove(strategy)
                return SupportSet(strategies, self.num_players)
            else:
                raise UndefinedOperationError("cannot remove last strategy"\
                                              " of a player")
        raise TypeError("delete requires a Strategy object")

    def difference(self, SupportSet other):
        return SupportSet(filter(lambda x: x not in other, self), self.num_players)

    def intersection(self, SupportSet other):
        result = SupportSet(filter(lambda x: x in other, self), self.num_players)

    def is_valid(self, strategies, num_players):
        if len(set([strat.player.number for strat in strategies])) == num_players \
            & num_players >= 1:
            return True
        return False

    def issubset(self, SupportSet other):
        return reduce(lambda acc,st: acc & (st in other), self, True)

    def issuperset(self, SupportSet other):
        return other.issubset(self)

    def union(self, SupportSet other):
        return SupportSet(self.unique(list(self) + list(other)))

    def unique(self, lst):
        uniq = []
        [uniq.append(i) for i in lst if not uniq.count(i)]
        return uniq

cdef class SupportOutcomes(Collection):
    "Represents a collection of outcomes in a support."
    cdef StrategySupport support

    def __init__(self, StrategySupport support not None):
        self.support = support
    def __len__(self):    return (<Game>self.support.unrestrict()).game.deref().NumOutcomes()
    def __getitem__(self, outc):
        if not isinstance(outc, int):  return Collection.__getitem__(self, outc)
        cdef Outcome c
        c = Outcome()
        c.outcome = (<Game>self.support.unrestrict()).game.deref().GetOutcome(outc+1)
        c.support = self.support
        return c

    def add(self, label=""):
        raise UndefinedOperationError("Changing objects in a support is not supported")

cdef class SupportStrategies(Collection):
    "Represents a collection of strategies in a support."
    cdef StrategySupport support

    def __init__(self, StrategySupport support not None):
        self.support = support
    def __len__(self):    return self.support.support.MixedProfileLength()
    def __getitem__(self, strat):
        if not isinstance(strat, int):
            return Collection.__getitem__(self, strat)
        cdef c_ArrayInt num_strategies
        cdef Strategy s
        num_strategies = self.support.support.NumStrategies()
        for i in range(1,num_strategies.Length()+1):
            if strat - num_strategies.getitem(i) < 0:
                s = Strategy()
                s.strategy = self.support.support.GetStrategy(i, strat+1)  
                s.support = self.support
                return s
            strat = strat - num_strategies.getitem(i)
        raise IndexError("Index out of range")

cdef class StrategySupport(BaseGame):
    cdef c_StrategySupport support

    def __repr__(self):
        return "<Support from Game '%s'>" % self.title

    def __richcmp__(StrategySupport self, other, whichop):
        if isinstance(other, StrategySupport):
            if whichop == 2:
                return self.support == (<StrategySupport>other).support
            elif whichop == 3:
                return self.support != (<StrategySupport>other).support
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
            return "Support from Game '%s'" % self.unrestrict().title

    property players:
        def __get__(self):
            cdef Players p
            p = Players()
            p.game = (<Game>self.unrestrict()).game
            p.support = self
            return p

    property strategies:
        def __get__(self):
            cdef SupportStrategies s
            s = SupportStrategies(self)
            return s

    property outcomes:
        def __get__(self):
            cdef SupportOutcomes o
            o = SupportOutcomes(self)
            return o

    property is_const_sum:
        def __get__(self):
            return self.unrestrict().is_const_sum

    property is_perfect_recall:
        def __get__(self):
            return self.unrestrict().is_perfect_recall

    def delete(self, strat):
        cdef StrategySupport new_support
        if isinstance(strat, Strategy):
            new_support = StrategySupport()
            new_support.support = self.support
            new_support.support.RemoveStrategy((<Strategy>strat).strategy)
            return new_support
        raise TypeError("delete requires a Strategy object")

    def undominated(self, strict=False, external=False):
        cdef StrategySupport new_support
        new_support = StrategySupport()
        new_support.support = self.support.Undominated(strict, external)
        return new_support

    def issubset(self, spt):
        if isinstance(spt, StrategySupport):
            return self.support.IsSubsetOf((<StrategySupport>spt).support)
        raise TypeError("issubset requires a StrategySupport object")

    def num_strategies_player(self, pl):
        return self.support.NumStrategiesPlayer(pl+1)

    def support_set(self):
        return SupportSet(list(self.strategies), len(self.players))

    def restrict(self, SupportSet sp):
        cdef StrategySupport new_support
        support = self.support_set()
        if support >= sp:
            difference = support - sp
            new_support = StrategySupport()
            new_support.support = self.support
            for strategy in difference:
                new_support.support.RemoveStrategy((<Strategy>strategy).strategy)
            return new_support
        else:
            raise UndefinedOperationError("cannot restrict game to a non-subset of it")

    def unrestrict(self):
        cdef Game g
        g = Game()
        g.game = self.support.GetGame()
        return g
