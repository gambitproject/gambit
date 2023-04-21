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
import pathlib

import numpy as np

import pygambit.gte
import pygambit.gameiter


cdef class Outcomes(Collection):
    """Represents a collection of outcomes in a game."""
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
    """Represents a collection of players in a game."""
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
    """Represents a collection of actions in a game."""
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
    """Represents a collection of infosets in a game."""
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
    """Represents a collection of strategies in a game."""
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

cdef class Game:
    """Represents a game, the fundamental concept in game theory.
    Games may be represented in extensive or strategic form.
    """
    cdef c_Game game

    @classmethod
    def new_tree(cls, title: str="Untitled extensive game") -> Game:
        """Creates a new Game consisting of a trivial game tree,
        with one node, which is both root and terminal, and only the chance player.

        .. versionchanged:: 16.1.0
	        Added the ``title`` parameter.

        Parameters
        ----------
        title : str, optional
            The title of the game.  If no title is specified, "Untitled extensive game"
            is used.

        Returns
        -------
        Game
            The newly-created extensive game.
        """
        cdef Game g
        g = cls()
        g.game = NewTree()
        g.title = title
        return g

    @classmethod
    def new_table(cls, dim, title: str="Untitled strategic game") -> Game:
        """Creates a new Game with a strategic representation.

        .. versionchanged:: 16.1.0
            Added the ``title`` parameter.

        Parameters
        ----------
        dim : array-like
            A list specifying the number of strategies for each player.
        title : str, optional
            The title of the game.  If no title is specified, "Untitled strategic game"
            is used.

        Returns
        -------
        Game
            The newly-created strategic game.
        """
        cdef Game g
        cdef Array[int] *d
        d = new Array[int](len(dim))
        for i in range(1, len(dim)+1):
            setitem_array_int(d, i, dim[i-1])
        g = cls()
        g.game = NewTable(d)
        del d
        g.title = title
        return g

    @classmethod
    def from_arrays(cls, *arrays, title: str="Untitled strategic game") -> Game:
        """Creates a new Game with a strategic representation.

        Each entry in ``arrays`` gives the payoff matrix for the
        corresponding player.  The arrays must all have the same shape,
        and have the same number of dimensions as the total number of
        players.

        .. versionchanged:: 16.1.0
            Added the ``title`` parameter.

        Parameters
        ----------
        arrays : array-like of array-like
            The payoff matrices for the players.
        title : str, optional
            The title of the game.  If no title is specified, "Untitled strategic game"
            is used.

        Returns
        -------
        Game
            The newly-created strategic game.
        """
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
        g.title = title
        return g

    @classmethod
    def from_dict(cls, payoffs, title: str="Untitled strategic game") -> Game:
        """Creates a new Game with a strategic representation.

        Each entry in ``payoffs`` is a key-value pair
        giving the label and the payoff matrix for a player.
        The payoff matrices must all have the same shape,
        and have the same number of dimensions as the total number of
        players.

        Parameters
        ----------
        payoffs : dict-like mapping str to array-like
            The names and corresponding payoff matrices for the players.
        title : str, optional
            The title of the game.  If no title is specified, "Untitled strategic game"
            is used.

        Returns
        -------
        Game
            The newly-created strategic game.
        """
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
        g.title = title
        return g

    @classmethod
    def read_game(cls, filepath: typing.Union[str, pathlib.Path]) -> Game:
        """Constructs a game from its serialised representation in a file.

        Parameters
        ----------
        filepath : str or path object
            The path to the file containing the game representation.

        Returns
        -------
        Game
            A game constructed from the representation in the file.

        Raises
        ------
        IOError
            If the file cannot be opened or read
        ValueError
            If the contents of the file are not a valid game representation.

        See Also
        --------
        parse_game : Constructs a game from a text string.
        """
        cdef Game g
        g = cls()
        with open(filepath, "rb") as f:
            data = f.read()
        try:
            g.game = ParseGame(data)
        except Exception as exc:
            raise ValueError(f"Parse error in game file: {exc}") from None
        return g

    @classmethod
    def parse_game(cls, text: str) -> Game:
        """Constructs a game from its serialised representation in a string
        .
        Parameters
        ----------
        text : str
            A string containing the game representation.

        Returns
        -------
        Game
            A game constructed from the representation in the string.

        Raises
        ------
        ValueError
            If the contents of the file are not a valid game representation.

        See Also
        --------
        read_game : Constructs a game from a representation in a file.
        """
        cdef Game g
        g = cls()
        try:
            g.game = ParseGame(text.encode('ascii'))
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
                return self.game.deref() == (<Game>other).game.deref()
            elif whichop == 3:
                return self.game.deref() != (<Game>other).game.deref()
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
        """Returns whether a game has a tree-based representation."""
        def __get__(self) -> bool:
            return self.game.deref().IsTree() != 0

    property title:
        """Gets or sets the title of the game.  The title of the game is
        an arbitrary string, generally intended to be short."""
        def __get__(self) -> str:
            """Gets the title of the game."""
            return self.game.deref().GetTitle().decode('ascii')
        def __set__(self, value: str) -> None:
            """Sets the title of the game."""
            self.game.deref().SetTitle(value.encode('ascii'))

    property comment:
        """Gets or sets the comment of the game.  A game's comment is
        an arbitrary string, and may be more discursive than a title."""
        def __get__(self) -> str:
            """Gets the comment of the game."""
            return self.game.deref().GetComment().decode('ascii')
        def __set__(self, value: str) -> None:
            """Sets the comment of the game."""
            self.game.deref().SetComment(value.encode('ascii'))

    property actions:
        def __get__(self):
            cdef GameActions a
            if self.is_tree:
                a = GameActions()
                a.game = self.game
                return a
            raise UndefinedOperationError(
                "Operation only defined for games with a tree representation"
            )

    property infosets:
        def __get__(self):
            cdef GameInfosets i
            if self.is_tree:
                i = GameInfosets()
                i.game = self.game
                return i
            raise UndefinedOperationError(
                "Operation only defined for games with a tree representation"
            )

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
        """Returns the root node of the game.
        
        Returns
        -------
        Node
            The root node of the game.
        
        Raises
        ------
        UndefinedOperationError
            If the game does not hae a tree representation.
        """
        def __get__(self) -> Node:
            cdef Node n
            if self.is_tree:
                n = Node()
                n.node = self.game.deref().GetRoot()
                return n
            raise UndefinedOperationError(
                "Operation only defined for games with a tree representation"
            )
                         
    property is_const_sum:
        """Returns whether the game is constant sum.

        Returns
        -------
        bool
            `True` if and only if the game is constant sum.
        """
        def __get__(self) -> bool:
            return self.game.deref().IsConstSum()

    property is_perfect_recall:
        """Returns whether the game is perfect recall.

        By convention, games with a strategic representation have perfect recall as they
        are treated as simultaneous-move games.

        Returns
        -------
        bool
            `True` if and only if the game is perfect recall.            
        """
        def __get__(self) -> bool:
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
                raise KeyError("Number of strategies is not equal to the number of players")
        except TypeError:
            raise TypeError("contingency must be a tuple-like object")
        cont = [ 0 ] * len(self.players)
        for (pl, st) in enumerate(i):
            if isinstance(st, int):
                if st < 0 or st >= len(self.players[pl].strategies):
                    raise IndexError(f"Provided strategy index {st} out of range for player {pl}")
                cont[pl] = st
            elif isinstance(st, str):
                try:
                    cont[pl] = [ s.label for s in self.players[pl].strategies ].index(st)
                except ValueError:
                    raise IndexError(f"Provided strategy label '{st}' not defined")
            elif isinstance(st, Strategy):
                try:
                    cont[pl] = list(self.players[pl].strategies).index(st)
                except ValueError:
                    raise IndexError(f"Provided strategy '{st}' not available to player")
            else:
                raise TypeError("Must use a tuple of ints, strategy labels, or strategies")
        return self._get_contingency(*tuple(cont))


    def mixed_strategy_profile(self, data=None, rational=False):
        cdef MixedStrategyProfileDouble mspd
        cdef MixedStrategyProfileRational mspr
        cdef c_Rational dummy_rat
        if not self.is_perfect_recall:
            raise UndefinedOperationError(
                "Mixed strategies not supported for games with imperfect recall."
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
            raise UndefinedOperationError(
                "Game must have a tree representation to create a mixed behavior profile"
            )
 
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
