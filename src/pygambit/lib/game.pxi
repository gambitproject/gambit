#
# This file is part of Gambit
# Copyright (c) 1994-2022, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/python/gambit/lib/game.pxi
# Cython wrapper for games
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
import itertools
from libcpp cimport bool

import numpy as np

from pygambit.lib.error import UndefinedOperationError
import pygambit.gte
import pygambit.gameiter


cdef class Outcomes(Collection):
    "Represents a collection of outcomes in a game."
    cdef c_Game game
    def __len__(self):    return self.game.deref().NumOutcomes()
    def __getitem__(self, outc):
        if not isinstance(outc, int):  return Collection.__getitem__(self, outc)
        cdef Outcome c
        c = Outcome()
        c.outcome = self.game.deref().GetOutcome(outc+1)
        return c

    def add(self, label=""):
        cdef Outcome c
        c = Outcome()
        c.outcome = self.game.deref().NewOutcome()
        if label != "": c.label = str(label)
        return c

cdef class Players(Collection):
    "Represents a collection of players in a game."
    cdef c_Game game
    cdef StrategicRestriction restriction
    def __len__(self):       return self.game.deref().NumPlayers()
    def __getitem__(self, pl):
        if not isinstance(pl, int):  return Collection.__getitem__(self, pl)
        cdef Player p
        p = Player()
        p.player = self.game.deref().GetPlayer(pl+1)
        if self.restriction is not None:
            p.restriction = self.restriction
        return p

    def add(self, label=""):
        cdef Player p
        if self.restriction is not None:
            raise UndefinedOperationError("Changing objects in a restriction is not supported")
        p = Player()
        p.player = self.game.deref().NewPlayer()
        if label != "": p.label = str(label)
        return p

    property chance:
        def __get__(self):
            cdef Player p
            p = Player()
            p.player = self.game.deref().GetChance()
            p.restriction = self.restriction
            return p

cdef class GameActions(Collection):
    "Represents a collection of actions in a game."
    cdef c_Game game
    def __len__(self):
        return self.game.deref().BehavProfileLength()
    def __getitem__(self, action):
        if not isinstance(action, int):
            return Collection.__getitem__(self, action)
        cdef Action a
        a = Action()
        a.action = self.game.deref().GetAction(action+1)
        return a

cdef class GameInfosets(Collection):
    "Represents a collection of infosets in a game."
    cdef c_Game game
    def __len__(self):
        cdef Array[int] num_infosets
        num_infosets = self.game.deref().NumInfosets()
        size = num_infosets.Length()
        n = 0
        for i in range(1,size+1):
            n += num_infosets.getitem(i)
        return n
    def __getitem__(self, infoset):
        if not isinstance(infoset, int):
            return Collection.__getitem__(self, infoset)
        cdef Infoset i
        i = Infoset()
        i.infoset = self.game.deref().GetInfoset(infoset+1)
        return i

cdef class GameStrategies(Collection):
    "Represents a collection of strategies in a game."
    cdef c_Game game
    def __len__(self):
        return self.game.deref().MixedProfileLength()
    def __getitem__(self, st):
        if not isinstance(st, int):
            return Collection.__getitem__(self, st)
        cdef Strategy s
        s = Strategy()
        s.strategy = self.game.deref().GetStrategy(st+1)
        return s

cdef class Game(object):
    cdef c_Game game

    @classmethod
    def new_tree(cls, title=None):
        cdef Game g
        g = cls()
        g.game = NewTree()
        if title is not None:
            g.title = title
        else:
            g.title = "Untitled extensive game"
        return g

    @classmethod
    def new_table(cls, dim, title=None):
        cdef Game g
        cdef Array[int] *d
        d = new Array[int](len(dim))
        for i in range(1, len(dim)+1):
            setitem_array_int(d, i, dim[i-1])
        g = cls()
        g.game = NewTable(d)
        del d
        if title is not None:
            g.title = title
        else:
            g.title = "Untitled strategic game"
        return g

    @classmethod
    def from_arrays(cls, *arrays, title=None):
        cdef Game g
        arrays = [np.array(a) for a in arrays]
        if len(set(a.shape for a in arrays)) > 1:
            raise ValueError("All specified arrays must have the same shape")
        g = Game.new_table(arrays[0].shape)
        for profile in itertools.product(
                *(range(arrays[0].shape[i]) for i in range(len(g.players)))
        ):
            for pl in range(len(g.players)):
                g[profile][pl] = arrays[pl][profile]
        if title is not None:
            g.title = title
        else:
            g.title = "Untitled strategic game"
        return g

    @classmethod
    def from_dict(cls, payoffs, title=None):
        cdef Game g
        payoffs = {k: np.array(v) for k, v in payoffs.items()}
        if len(set(a.shape for a in payoffs.values())) > 1:
            raise ValueError("All specified arrays must have the same shape")
        arrays = list(payoffs.values())
        shape = arrays[0].shape
        g = Game.new_table(shape)
        for (player, label) in zip(g.players, payoffs):
            player.label = label
        for profile in itertools.product(
                *(range(shape[i]) for i in range(len(g.players)))
        ):
            for (pl, _) in enumerate(arrays):
                g[profile][pl] = arrays[pl][profile]
        if title is not None:
            g.title = title
        else:
            g.title = "Untitled strategic game"
        return g

    @classmethod
    def read_game(cls, fn):
        cdef Game g
        g = cls()
        with open(fn, "rb") as f:
            data = f.read()
        try:
            g.game = ParseGame(data)
        except Exception as exc:
            raise ValueError(f"Parse error in game file: {exc}") from None
        return g

    @classmethod
    def parse_game(cls, s):
        cdef Game g
        g = cls()
        try:
            g.game = ParseGame(s.encode('ascii'))
        except Exception as exc:
            raise ValueError(f"Parse error in game file: {exc}") from None
        return g        

    def __str__(self):
        return f"<Game '{self.title}'>"

    def __repr__(self):
        return self.write()

    def _repr_html_(self):
        if self.is_tree:
            return self.write()
        else:
            return self.write('html')

    def __richcmp__(Game self, other, whichop):
        if isinstance(other, Game):
            if whichop == 2:
                return self.game.deref() == ((<Game>other).game).deref()
            elif whichop == 3:
                return self.game.deref() != ((<Game>other).game).deref()
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
        return long(<long>self.game.deref())

    property is_tree:
        def __get__(self):
            return True if self.game.deref().IsTree() != 0 else False

    property title:
        def __get__(self):
            return self.game.deref().GetTitle().decode('ascii')
        def __set__(self, str value):
            self.game.deref().SetTitle(value.encode('ascii'))

    property comment:
        def __get__(self):
            return self.game.deref().GetComment().decode('ascii')
        def __set__(self, str value):
            self.game.deref().SetComment(value.encode('ascii'))

    property actions:
        def __get__(self):
            cdef GameActions a
            if self.is_tree:
                a = GameActions()
                a.game = self.game
                return a
            raise UndefinedOperationError("Operation only defined for "\
                                           "games with a tree representation")

    property infosets:
        def __get__(self):
            cdef GameInfosets i
            if self.is_tree:
                i = GameInfosets()
                i.game = self.game
                return i
            raise UndefinedOperationError("Operation only defined for "\
                                           "games with a tree representation")

    property players:
        def __get__(self):
            cdef Players p
            p = Players()
            p.game = self.game
            return p

    property strategies:
        def __get__(self):
            cdef GameStrategies s
            s = GameStrategies()
            s.game = self.game
            return s

    property outcomes:
        def __get__(self):
            cdef Outcomes c
            c = Outcomes()
            c.game = self.game
            return c

    property contingencies:
        def __get__(self):
            return pygambit.gameiter.Contingencies(self)

    property root:
        def __get__(self):
            cdef Node n
            if self.is_tree:
                n = Node()
                n.node = self.game.deref().GetRoot()
                return n
            raise UndefinedOperationError("Operation only defined for "\
                                           "games with a tree representation")
                         
    property is_const_sum:
        def __get__(self):
            return self.game.deref().IsConstSum()

    property is_perfect_recall:
        def __get__(self):
            return self.game.deref().IsPerfectRecall()

    property min_payoff:
        def __get__(self):
            return rat_to_py(self.game.deref().GetMinPayoff(0))

    property max_payoff:
        def __get__(self):
            return rat_to_py(self.game.deref().GetMaxPayoff(0))

    def _get_contingency(self, *args):
        cdef c_PureStrategyProfile *psp
        cdef Outcome outcome
        cdef TreeGameOutcome tree_outcome
        psp = new c_PureStrategyProfile(self.game.deref().NewPureStrategyProfile())
        
        
        for (pl, st) in enumerate(args):
            psp.deref().SetStrategy(self.game.deref().GetPlayer(pl+1).deref().GetStrategy(st+1))

        if self.is_tree:
            tree_outcome = TreeGameOutcome()
            tree_outcome.psp = psp
            tree_outcome.c_game = self.game
            return tree_outcome
        else:
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


    def mixed_strategy_profile(self, data=None, rational=False):
        cdef MixedStrategyProfileDouble mspd
        cdef MixedStrategyProfileRational mspr
        cdef c_Rational dummy_rat
        if not self.is_perfect_recall:
            raise UndefinedOperationError(
                "Mixed strategies not supported for games with "
                "imperfect recall."
            )
        if not rational:
            mspd = MixedStrategyProfileDouble()
            mspd.profile = new c_MixedStrategyProfileDouble(
                self.game.deref().NewMixedStrategyProfile(0.0)
            )
            if data is None:
                return mspd
            if len(data) != len(self.players):
                raise ValueError(
                    "Number of elements does not match number of players"
                )
            for (p, d) in zip(self.players, data):
                if len(p.strategies) != len(d):
                    raise ValueError(
                        f"Number of elements does not match number of "
                        f"strategies for {p}"
                    )
                for (s, v) in zip(p.strategies, d):
                    mspd[s] = float(v)
            return mspd
        else:
            mspr = MixedStrategyProfileRational()
            mspr.profile = new c_MixedStrategyProfileRational(
                self.game.deref().NewMixedStrategyProfile(dummy_rat)
            )
            if data is None:
                return mspr
            if len(data) != len(self.players):
                raise ValueError(
                    "Number of elements does not match number of players"
                )
            for (p, d) in zip(self.players, data):
                if len(p.strategies) != len(d):
                    raise ValueError(
                        f"Number of elements does not match number of "
                        f"strategies for {p}"
                    )
                for (s, v) in zip(p.strategies, d):
                    mspr[s] = Rational(v)
            return mspr

    def mixed_behavior_profile(self, rational=False):
        cdef MixedBehaviorProfileDouble mbpd
        cdef MixedBehaviorProfileRational mbpr
        if self.is_tree:
            if not rational:
                mbpd = MixedBehaviorProfileDouble()
                mbpd.profile = new c_MixedBehaviorProfileDouble(self.game)
                return mbpd
            else:
                mbpr = MixedBehaviorProfileRational()
                mbpr.profile = new c_MixedBehaviorProfileRational(self.game)
                return mbpr
        else:
            raise UndefinedOperationError("Game must have a tree representation"\
                                      " to create a mixed behavior profile")
 
    def support_profile(self):
        return StrategySupportProfile(list(self.strategies), self)

    def num_nodes(self):
        if self.is_tree:
            return self.game.deref().NumNodes()
        return 0

    def unrestrict(self):
        return self

    def write(self, format='native'):
        if format == 'gte':
            return pygambit.gte.write_game(self)
        else:
            return WriteGame(self.game, format.encode('ascii')).decode('ascii')
