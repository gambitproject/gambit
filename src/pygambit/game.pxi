#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/game.pxi
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
import dataclasses
import io
import itertools
import pathlib

import cython
import numpy as np
import scipy.stats

import pygambit.gameiter

ctypedef string (*GameWriter)(const c_Game &) except +IOError
ctypedef c_Game (*GameParser)(const string &, bool) except +IOError


@cython.cfunc
def read_game(filepath_or_buffer: str | pathlib.Path | io.IOBase,
              normalize_labels: bool,
              parser: GameParser):

    g = cython.declare(Game)
    if isinstance(filepath_or_buffer, io.TextIOBase):
        data = filepath_or_buffer.read().encode("utf-8")
    elif isinstance(filepath_or_buffer, io.IOBase):
        data = filepath_or_buffer.read()
    else:
        with open(filepath_or_buffer, "rb") as f:
            data = f.read()
    try:
        g = Game.wrap(parser(data, normalize_labels))
    except Exception as exc:
        raise ValueError(f"Parse error in game file: {exc}") from None
    return g


def read_gbt(filepath_or_buffer: str | pathlib.Path | io.IOBase,
             normalize_labels: bool = False) -> Game:
    """Construct a game from its serialised representation in a GBT file.

    Parameters
    ----------
    filepath_or_buffer : str, pathlib.Path or io.IOBase
        The path to the file containing the game representation or file-like object
    normalize_labels : bool (default False)
        Ensure all labels are nonempty and unique within their scopes.
        This will be enforced in a future version of Gambit.

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
    read_efg, read_nfg, read_agg
    """
    return read_game(filepath_or_buffer, normalize_labels, parser=ParseGbtGame)


def read_efg(filepath_or_buffer: str | pathlib.Path | io.IOBase,
             normalize_labels: bool = False) -> Game:
    """Construct a game from its serialised representation in an EFG file.

    Parameters
    ----------
    filepath_or_buffer : str, pathlib.Path or io.IOBase
        The path to the file containing the game representation or file-like object
    normalize_labels : bool (default False)
        Ensure all labels are nonempty and unique within their scopes.
        This will be enforced in a future version of Gambit.

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
    read_gbt, read_nfg, read_agg
    """
    return read_game(filepath_or_buffer, normalize_labels, parser=ParseEfgGame)


def read_nfg(filepath_or_buffer: str | pathlib.Path | io.IOBase,
             normalize_labels: bool = False) -> Game:
    """Construct a game from its serialised representation in a NFG file.

    Parameters
    ----------
    filepath_or_buffer : str, pathlib.Path or io.IOBase
        The path to the file containing the game representation or file-like object
    normalize_labels : bool (default False)
        Ensure all labels are nonempty and unique within their scopes.
        This will be enforced in a future version of Gambit.

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
    read_gbt, read_efg, read_agg
    """
    return read_game(filepath_or_buffer, normalize_labels, parser=ParseNfgGame)


def read_agg(filepath_or_buffer: str | pathlib.Path | io.IOBase,
             normalize_labels: bool = False) -> Game:
    """Construct a game from its serialised representation in an AGG file.

    Parameters
    ----------
    filepath_or_buffer : str, pathlib.Path or io.IOBase
        The path to the file containing the game representation or file-like object
    normalize_labels : bool (default False)
        Ensure all labels are nonempty and unique within their scopes.
        This will be enforced in a future version of Gambit.

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
    read_gbt, read_efg, read_nfg
    """
    return read_game(filepath_or_buffer, normalize_labels, parser=ParseAggGame)


@cython.cclass
class GameNodes:
    """Represents the set of nodes in a game."""
    game = cython.declare(c_Game)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create GameNodes outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(game: c_Game) -> GameNodes:
        obj: GameNodes = GameNodes.__new__(GameNodes)
        obj.game = game
        return obj

    def __repr__(self) -> str:
        return f"GameNodes(game={Game.wrap(self.game)})"

    def __len__(self) -> int:
        """The number of nodes in the game."""
        if not self.game.deref().IsTree():
            return 0
        return self.game.deref().NumNodes()

    def __iter__(self) -> typing.Iterator[Node]:
        """Iterate over the game nodes in the depth-first traversal order."""
        if not self.game.deref().IsTree():
            return

        for node in self.game.deref().GetNodes():
            yield Node.wrap(node)


@cython.cclass
class GameOutcomes:
    """Represents the set of outcomes in a game."""
    game = cython.declare(c_Game)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create GameOutcomes outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(game: c_Game) -> GameOutcomes:
        obj: GameOutcomes = GameOutcomes.__new__(GameOutcomes)
        obj.game = game
        return obj

    def __repr__(self) -> str:
        return f"GameOutcomes(game={Game.wrap(self.game)})"

    def __len__(self) -> int:
        """The number of outcomes in the game."""
        return self.game.deref().GetOutcomes().size()

    def __iter__(self) -> typing.Iterator[Outcome]:
        for outcome in self.game.deref().GetOutcomes():
            yield Outcome.wrap(outcome)

    def __getitem__(self, index: int | str) -> Outcome:
        if isinstance(index, str):
            if not index.strip():
                raise ValueError("Outcome label cannot be empty or all whitespace")
            matches = [x for x in self if x.label == index.strip()]
            if not matches:
                raise KeyError(f"Game has no outcome with label '{index}'")
            if len(matches) > 1:
                raise ValueError(f"Game has multiple outcomes with label '{index}'")
            return matches[0]
        if isinstance(index, int):
            return Outcome.wrap(self.game.deref().GetOutcome(index + 1))
        raise TypeError(f"Outcome index must be int or str, not {index.__class__.__name__}")


@cython.cclass
class GamePlayers:
    """Represents a collection of players in a game."""
    game = cython.declare(c_Game)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create GamePlayers outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(game: c_Game) -> GamePlayers:
        obj: GamePlayers = GamePlayers.__new__(GamePlayers)
        obj.game = game
        return obj

    def __repr__(self) -> str:
        return f"GamePlayers(game={Game.wrap(self.game)})"

    def __len__(self) -> int:
        """Returns the number of players in the game."""
        return self.game.deref().NumPlayers()

    def __iter__(self) -> typing.Iterator[Player]:
        for player in self.game.deref().GetPlayers():
            yield Player.wrap(player)

    def __getitem__(self, index: int | str) -> Player:
        if isinstance(index, str):
            if not index.strip():
                raise ValueError("Player label cannot be empty or all whitespace")
            matches = [x for x in self if x.label == index.strip()]
            if not matches:
                raise KeyError(f"Game has no player with label '{index}'")
            if len(matches) > 1:
                raise ValueError(f"Game has multiple players with label '{index}'")
            return matches[0]
        if isinstance(index, int):
            try:
                return Player.wrap(self.game.deref().GetPlayer(index + 1))
            except IndexError:
                raise IndexError("Index out of range") from None
        raise TypeError(f"Player index must be int or str, not {index.__class__.__name__}")

    @property
    def chance(self) -> Player:
        """Returns the chance player associated with the game."""
        return Player.wrap(self.game.deref().GetChance())


@cython.cclass
class GameActions:
    """Represents the set of all actions in a game."""
    game = cython.declare(Game)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create GameActions outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(game: Game) -> GameActions:
        obj: GameActions = GameActions.__new__(GameActions)
        obj.game = game
        return obj

    def __repr__(self) -> str:
        return f"GameActions(game={self.game})"

    def __len__(self) -> int:
        return sum(len(s.actions) for s in self.game.infosets)

    def __iter__(self) -> typing.Iterator[Action]:
        for infoset in self.game.infosets:
            yield from infoset.actions

    def __getitem__(self, index: int | str) -> Action:
        if isinstance(index, str):
            if not index.strip():
                raise ValueError("Action label cannot be empty or all whitespace")
            matches = [x for x in self if x.label == index.strip()]
            if not matches:
                raise KeyError(f"Game has no action with label '{index}'")
            if len(matches) > 1:
                raise ValueError(f"Game has multiple actions with label '{index}'")
            return matches[0]
        if isinstance(index, int):
            for i, action in enumerate(self):
                if i == index:
                    return action
            else:
                raise IndexError("Index out of range")
        raise TypeError(f"Action index must be int or str, not {index.__class__.__name__}")


@cython.cclass
class GameInfosets:
    """Represents the set of all infosets in a game."""
    game = cython.declare(Game)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create GameInfosets outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(game: Game) -> GameInfosets:
        obj: GameInfosets = GameInfosets.__new__(GameInfosets)
        obj.game = game
        return obj

    def __repr__(self) -> str:
        return f"GameInfosets(game={self.game})"

    def __len__(self) -> int:
        return sum(len(p.infosets) for p in self.game.players)

    def __iter__(self) -> typing.Iterator[Infoset]:
        for player in self.game.players:
            yield from player.infosets

    def __getitem__(self, index: int | str) -> Infoset:
        if isinstance(index, str):
            if not index.strip():
                raise ValueError("Infoset label cannot be empty or all whitespace")
            matches = [x for x in self if x.label == index.strip()]
            if not matches:
                raise KeyError(f"Game has no infoset with label '{index}'")
            if len(matches) > 1:
                raise ValueError(f"Game has multiple infosets with label '{index}'")
            return matches[0]
        if isinstance(index, int):
            for i, infoset in enumerate(self):
                if i == index:
                    return infoset
            else:
                raise IndexError("Index out of range")
        raise TypeError(f"Infoset index must be int or str, not {index.__class__.__name__}")


@cython.cclass
class GameStrategies:
    """Represents the set of all strategies in the game."""
    game = cython.declare(Game)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create GameStrategies outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(game: Game) -> GameStrategies:
        obj: GameStrategies = GameStrategies.__new__(GameStrategies)
        obj.game = game
        return obj

    def __repr__(self) -> str:
        return f"GameStrategies(game={self.game})"

    def __len__(self) -> int:
        return sum(len(p.strategies) for p in self.game.players)

    def __iter__(self) -> typing.Iterator[Strategy]:
        for player in self.game.players:
            yield from player.strategies

    def __getitem__(self, index: int | str) -> Strategy:
        if isinstance(index, str):
            if not index.strip():
                raise ValueError("Strategy label cannot be empty or all whitespace")
            matches = [x for x in self if x.label == index.strip()]
            if not matches:
                raise KeyError(f"Game has no strategy with label '{index}'")
            if len(matches) > 1:
                raise ValueError(f"Game has multiple strategies with label '{index}'")
            return matches[0]
        if isinstance(index, int):
            for i, strat in enumerate(self):
                if i == index:
                    return strat
            else:
                raise IndexError("Index out of range")
        raise TypeError(f"Strategy index must be int or str, not {index.__class__.__name__}")


@cython.cclass
class Game:
    """A game, the fundamental unit of analysis in game theory.

    Games may be represented in extensive or strategic form.
    """
    game = cython.declare(c_Game)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Use Game.new_tree() or Game.new_table() to create a new game")

    @staticmethod
    @cython.cfunc
    def wrap(game: c_Game) -> Game:
        obj: Game = Game.__new__(Game)
        obj.game = game
        return obj

    @classmethod
    def new_tree(cls,
                 players: list[str] | None = None,
                 title: str = "Untitled extensive game") -> Game:
        """Create a new ``Game`` consisting of a trivial game tree,
        with one node, which is both root and terminal.

        .. versionchanged:: 16.1.0
            Added the `players` and `title` parameters

        Parameters
        ----------
        players : list of str, optional
            A list of labels for the (strategic) players of the game.  If `players`
            is not specified, the game initially has no players defined other than
            the chance player.
        title : str, optional
            The title of the game.  If no title is specified, "Untitled extensive game"
            is used.

        Returns
        -------
        Game
            The newly-created extensive game.
        """
        g = Game.wrap(NewTree())
        g.title = title
        for player in (players or []):
            Player.wrap(g.game.deref().NewPlayer()).label = str(player)
        return g

    @classmethod
    def new_table(cls, dim, title: str = "Untitled strategic game") -> Game:
        """Create a new ``Game`` with a strategic representation.

        .. versionchanged:: 16.1.0
            Added the `title` parameter.

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
        g = Game.wrap(NewTable(list(dim)))
        g.title = title
        return g

    @classmethod
    def from_arrays(cls, *arrays, title: str = "Untitled strategic game") -> Game:
        """Create a new ``Game`` with a strategic representation.

        Each entry in `arrays` gives the payoff matrix for the
        corresponding player.  The arrays must all have the same shape,
        and have the same number of dimensions as the total number of
        players.

        .. versionchanged:: 16.1.0
            Added the `title` parameter.

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

        See Also
        --------
        from_dict : Create strategic game and set player labels
        to_array: Generate the payoff tables for players represented as numpy arrays
        """
        arrays = [np.array(a) for a in arrays]
        if len(set(a.shape for a in arrays)) > 1:
            raise ValueError("All specified arrays must have the same shape")
        shape = arrays[0].shape
        g = Game.new_table(shape)
        for profile in itertools.product(*(range(s) for s in shape)):
            for array, player in zip(arrays, g.players, strict=True):
                g[profile][player] = array[profile]
        g.title = title
        return g

    def to_arrays(self, dtype: typing.Type = Rational) -> list[np.array]:
        """Generate the payoff tables for players represented as numpy arrays.

        Parameters
        ----------
        dtype : type
            The type to which payoff values will be converted and
            the resulting arrays will be of that dtype

        Returns
        -------
        list of np.array

        See Also
        --------
        from_arrays : Create game from list-like of array-like
        """
        arrays = []

        shape = tuple(len(player.strategies) for player in self.players)
        for player in self.players:
            array = np.zeros(shape=shape, dtype=object)
            for profile in itertools.product(*(range(s) for s in shape)):
                try:
                    array[profile] = dtype(self[profile][player])
                except (ValueError, TypeError, IndexError, KeyError):
                    raise ValueError(
                        f"Payoff '{self[profile][player]}' cannot be "
                        f"converted to requested type '{dtype}'"
                        ) from None
            arrays.append(array)
        return arrays

    @classmethod
    def from_dict(cls, payoffs, title: str = "Untitled strategic game") -> Game:
        """Create a new ``Game`` with a strategic representation.

        Each entry in `payoffs` is a key-value pair
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

        See Also
        --------
        from_arrays : Create game from list-like of array-like
        """
        payoffs = {k: np.array(v) for k, v in payoffs.items()}
        if len(set(a.shape for a in payoffs.values())) > 1:
            raise ValueError("All specified arrays must have the same shape")
        arrays = list(payoffs.values())
        shape = arrays[0].shape
        g = Game.new_table(shape)
        for (player, label) in zip(g.players, payoffs, strict=True):
            player.label = label
        for profile in itertools.product(*(range(s) for s in shape)):
            for array, player in zip(arrays, g.players, strict=True):
                g[profile][player] = array[profile]
        g.title = title
        return g

    def __repr__(self) -> str:
        if self.title:
            return f"Game(title='{self.title}')"
        else:
            return f"Game(id={hash(self)}"

    def _repr_html_(self):
        if self.is_tree:
            return repr(self)
        else:
            return self.to_html()

    def __eq__(self, other: typing.Any) -> bool:
        return (
            isinstance(other, Game) and
            self.game.deref() == cython.cast(Game, other).game.deref()
        )

    def __hash__(self) -> int:
        return cython.cast(cython.long, self.game.deref())

    @property
    def is_tree(self) -> bool:
        """Return whether a game has a tree-based representation."""
        return self.game.deref().IsTree()

    @property
    def title(self) -> str:
        """Get or set the title of the game.

        The title of the game is an arbitrary string, generally intended
        to be short.
        """
        return self.game.deref().GetTitle().decode("ascii")

    @title.setter
    def title(self, value: str) -> None:
        self.game.deref().SetTitle(value.encode("ascii"))

    @property
    def comment(self) -> str:
        """Get or set the comment of the game.

        A game's comment is an arbitrary string, and may be more discursive
        than a title.
        """
        return self.game.deref().GetComment().decode("ascii")

    @comment.setter
    def comment(self, value: str) -> None:
        self.game.deref().SetComment(value.encode("ascii"))

    @property
    def actions(self) -> GameActions:
        """The set of actions available in the game.

        Raises
        ------
        UndefinedOperationError
            If the game does not have a tree representation.
        """
        if not self.is_tree:
            raise UndefinedOperationError(
                "Operation only defined for games with a tree representation"
            )
        return GameActions.wrap(self)

    @property
    def infosets(self) -> GameInfosets:
        """The set of information sets in the game.

        Raises
        ------
        UndefinedOperationError
            If the game does not have a tree representation.
        """
        if not self.is_tree:
            raise UndefinedOperationError(
                "Operation only defined for games with a tree representation"
            )
        return GameInfosets.wrap(self)

    @property
    def players(self) -> GamePlayers:
        """The set of players in the game."""
        return GamePlayers.wrap(self.game)

    @property
    def strategies(self) -> GameStrategies:
        """The set of strategies in the game."""
        return GameStrategies.wrap(self)

    @property
    def outcomes(self) -> GameOutcomes:
        """The set of outcomes in the game."""
        return GameOutcomes.wrap(self.game)

    @property
    def nodes(self) -> GameNodes:
        """The set of nodes in the game.

        Iteration over this property yields the nodes in the order of depth-first search.

        .. versionchanged:: 16.4
           Changed from a method ``nodes()`` to a property.

        Raises
        ------
        UndefinedOperationError
            If the game does not have a tree representation.
        """
        if not self.is_tree:
            raise UndefinedOperationError(
                "Operation only defined for games with a tree representation"
            )

        return GameNodes.wrap(self.game)

    @property
    def contingencies(self) -> pygambit.gameiter.Contingencies:
        """An iterator over the contingencies in the game."""
        return pygambit.gameiter.Contingencies(self)

    @property
    def root(self) -> Node:
        """The root node of the game.

        Raises
        ------
        UndefinedOperationError
            If the game does not hae a tree representation.
        """
        if not self.is_tree:
            raise UndefinedOperationError(
                "root: only games with a tree representation have a root node"
            )
        return Node.wrap(self.game.deref().GetRoot())

    @property
    def is_const_sum(self) -> bool:
        """Whether the game is constant sum."""
        return self.game.deref().IsConstSum()

    @property
    def is_perfect_recall(self) -> bool:
        """Whether the game is perfect recall.

        By convention, games with a strategic representation have perfect recall as they
        are treated as simultaneous-move games.
        """
        return self.game.deref().IsPerfectRecall()

    @property
    def min_payoff(self) -> decimal.Decimal | Rational:
        """The minimum payoff to any player in any play of the game.

        .. versionchanged:: 16.5.0
           Changed from reporting minimum payoff in any (non-null) outcome to the minimum
           payoff in any play of the game.

        See also
        --------
        Game.max_payoff
        Player.min_payoff
        """
        return rat_to_py(self.game.deref().GetMinPayoff())

    @property
    def max_payoff(self) -> decimal.Decimal | Rational:
        """The maximum payoff to any player in any play of the game.

        .. versionchanged:: 16.5.0
           Changed from reporting maximum payoff in any (non-null) outcome to the maximum
           payoff in any play of the game.

        See also
        --------
        Game.min_payoff
        Player.max_payoff
        """
        return rat_to_py(self.game.deref().GetMaxPayoff())

    def set_chance_probs(self, infoset: Infoset | str, probs: typing.Sequence):
        """Set the action probabilities at chance information set `infoset`.

        Parameters
        ----------
        infoset : Infoset or str
            The chance information set at which to set the action probabilities.
            If a string is passed, the information set is determined by finding the chance
            information set with that label, if any.
        probs : array-like
            The action probabilities to set

        Raises
        ------
        MismatchError
            If `infoset` is not an information set in this game
        UndefinedOperationError
            If `infoset` is not an information set of the chance player
        IndexError
            If the length of `probs` is not the same as the number of actions at the
            information set
        ValueError
            If any of the elements of `probs` are not interpretable as numbers, or the values of
            `probs` are not non-negative numbers that sum to exactly one.
        """
        infoset = self._resolve_infoset(infoset, "set_chance_probs")
        if not infoset.is_chance:
            raise UndefinedOperationError(
                "set_chance_probs() first argument must be a chance infoset"
            )
        if len(infoset.actions) != len(probs):
            raise IndexError("set_chance_probs(): must specify exactly one probability per action")
        numbers = Array[c_Number](len(probs))
        for i in range(1, len(probs)+1):
            setitem_array_number(numbers, i, _to_number(probs[i-1]))
        try:
            self.game.deref().SetChanceProbs(cython.cast(Infoset, infoset).infoset, numbers)
        except RuntimeError:
            raise ValueError(
                "set_chance_probs(): must specify non-negative probabilities that sum to one"
            ) from None

    def _get_contingency(self, *args):
        psp: shared_ptr[c_PureStrategyProfile] = make_shared[c_PureStrategyProfile](
            self.game.deref().NewPureStrategyProfile()
        )

        for (pl, st) in enumerate(args):
            deref(deref(psp).deref()).SetStrategy(
                self.game.deref().GetPlayer(pl+1).deref().GetStrategy(st+1)
            )

        if self.is_tree:
            return TreeGameOutcome.wrap(self.game, psp)
        else:
            outcome = Outcome.wrap(deref(deref(psp).deref()).GetOutcome())
            if outcome.outcome != cython.cast(c_GameOutcome, NULL):
                return outcome
            else:
                return None

    def __getitem__(self, i):
        """Returns the `Outcome` associated with a profile of pure strategies.
        """
        try:
            if len(i) != len(self.players):
                raise KeyError("Number of strategies is not equal to the number of players")
        except TypeError:
            raise TypeError("contingency must be a tuple-like object")
        cont = [0 for _ in self.players]
        for (pl, st) in enumerate(i):
            if isinstance(st, int):
                if st < 0 or st >= len(self.players[pl].strategies):
                    raise IndexError(f"Provided strategy index {st} out of range for player {pl}")
                cont[pl] = st
            elif isinstance(st, str):
                try:
                    cont[pl] = [s.label for s in self.players[pl].strategies].index(st)
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

    def _fill_strategy_profile(self,
                               profile: MixedStrategyProfile,
                               data: list | None,
                               typefunc: typing.Callable) -> MixedStrategyProfile:
        """Utility function to fill a `MixedStrategyProfile` with the data from a nested list."""
        if data is None:
            return profile
        if len(data) != len(self.players):
            raise ValueError("Number of elements does not match number of players")
        for (p, d) in zip(self.players, data, strict=True):
            if len(p.strategies) != len(d):
                raise ValueError(
                    f"Number of elements does not match number of strategies for {p}"
                )
            for (s, v) in zip(p.strategies, d, strict=True):
                profile[s] = typefunc(v)
        return profile

    def mixed_strategy_profile(self, data=None, rational=False) -> MixedStrategyProfile:
        """Create a mixed strategy profile over the game.

        If `data` is not specified, the mixed
        strategy profile is initialized to uniform randomization for each
        player over their strategies.  If the game has a tree
        representation, the mixed strategy profile is defined over the
        reduced strategic form representation.

        Parameters
        ----------
        data
            A nested list (or compatible type) with the
            same dimension as the strategy set of the game,
            specifying the probabilities of the strategies.

        rational
            If True, probabilities are represented using rational numbers;
            otherwise floating point numbers are used.

        See Also
        --------
        random_strategy_profile :
            Create a `MixedStrategyProfile` with randomly-drawn probabilities.
        """
        if not self.is_perfect_recall:
            raise UndefinedOperationError(
                "Mixed strategies not supported for games with imperfect recall."
            )
        if rational:
            mspr = MixedStrategyProfileRational.wrap(
                make_shared[c_MixedStrategyProfile[c_Rational]](
                    self.game.deref().NewMixedStrategyProfile(c_Rational())
                )
            )
            return self._fill_strategy_profile(mspr, data, Rational)
        else:
            mspd = MixedStrategyProfileDouble.wrap(make_shared[c_MixedStrategyProfile[double]](
                self.game.deref().NewMixedStrategyProfile(0.0)
            ))
            return self._fill_strategy_profile(mspd, data, float)

    def random_strategy_profile(
            self,
            denom: int = None,
            gen: np.random.Generator | None = None
    ) -> MixedStrategyProfile:
        """Create a `MixedStrategy` on the game, with probabilities drawn
        from the uniform distribution over the set of mixed strategy profiles.

        Parameters
        ----------
        denom : int, optional
            If specified, the probabilities are generated on a grid with denominator
            `denom`, and the resulting profile will be a `MixedStrategyProfileRational`.
            If not specified, the probabilities will be floating point numbers, and
            the resulting profile will be a `MixedStrategyProfileRational`.

        gen : np.random.Generator, optional
            If specified, uses the `numpy` random number generator `gen` to generate
            uniform random samples.  Otherwise, uses the default generation method
            in `numpy`.

        .. versionadded:: 16.2.0
           Replaces the functionality of `MixedStrategyProfile.randomize()`.

        See Also
        --------
        mixed_strategy_profile : Create a `MixedStrategyProfile` with specified probabilities.
        """
        if denom is None:
            profile = self.mixed_strategy_profile()
            for player in self.players:
                for strategy, prob in zip(
                        player.strategies,
                        scipy.stats.dirichlet(
                            alpha=[1 for strategy in player.strategies],
                            seed=gen
                        ).rvs(size=1)[0],
                        strict=True
                        ):
                    profile[strategy] = prob
            return profile
        elif denom < 1:
            raise ValueError("random_strategy_profile(): denom must be positive")
        else:
            profile = self.mixed_strategy_profile(rational=True)
            for player in self.players:
                k = len(player.strategies)
                sample = (
                    [0] +
                    sorted(
                        (gen or np.random).choice(np.arange(1, denom+k), size=k-1, replace=False)
                    ) +
                    [denom + k]
                )
                for strategy, (hi, lo) in zip(
                    player.strategies,
                    zip(
                        sample[1:],
                        sample[:-1],
                        strict=True
                    ),
                    strict=True
                ):
                    profile[strategy] = Rational(hi - lo - 1, denom)
            return profile

    def _fill_behavior_profile(self,
                               profile: MixedBehaviorProfile,
                               data: list | None,
                               typefunc: typing.Callable) -> MixedBehaviorProfile:
        """Utility function to fill a `MixedBehaviorProfile` with the data from a nested list."""
        if data is None:
            return profile
        if len(data) != len(self.players):
            raise ValueError("Number of elements does not match number of players")
        for (p, d) in zip(self.players, data):
            if len(p.infosets) != len(d):
                raise ValueError(f"Number of elements does not match number of infosets for {p}")
            for (i, v) in zip(p.infosets, d, strict=True):
                if len(i.actions) != len(v):
                    raise ValueError(
                        f"Number of elements does not match number of "
                        f"actions for infoset {i} for {p}"
                    )
                for (a, u) in zip(i.actions, v, strict=True):
                    profile[a] = typefunc(u)
        return profile

    def mixed_behavior_profile(self, data=None, rational=False) -> MixedBehaviorProfile:
        """Create a mixed behavior profile over the game.

        If `data` is not specified, the profile is initialized to uniform randomization
        at each information set.

        Parameters
        ----------
        data : array_like of array_like of array_like, optional
            A nested list (or compatible type) with the same dimension as the action set of the
            game, specifying the probabilities of the actions.

        rational : bool, optional
            If True, probabilities are represented using rational numbers;
            otherwise floating point numbers are used.

        Raises
        ------
        UndefinedOperationError
            If the game does not have a tree representation.

        See Also
        --------
        random_behavior_profile :
           Create a `MixedBehaviorProfile` with randomly-drawn probabilities.
        """
        if not self.is_tree:
            raise UndefinedOperationError(
                "Game must have a tree representation to create a mixed behavior profile"
            )
        if rational:
            mbpr = MixedBehaviorProfileRational.wrap(
                make_shared[c_MixedBehaviorProfile[c_Rational]](self.game)
            )
            return self._fill_behavior_profile(mbpr, data, Rational)
        else:
            mbpd = MixedBehaviorProfileDouble.wrap(
                make_shared[c_MixedBehaviorProfile[double]](self.game)
            )
            return self._fill_behavior_profile(mbpd, data, float)

    def random_behavior_profile(
            self,
            denom: int = None,
            gen: np.random.Generator | None = None
    ) -> MixedBehaviorProfile:
        """Create a `MixedBehaviorProfile` on the game, with probabilities drawn
        from the uniform distribution over the set of mixed behavior profiles.

        Parameters
        ----------
        denom : int, optional
            If specified, the probabilities are generated on a grid with denominator
            `denom`, and the resulting profile will be a `MixedBehaviorProfileRational`.
            If not specified, the probabilities will be floating point numbers, and
            the resulting profile will be a `MixedBehaviorProfileRational`.

        gen : np.random.Generator, optional
            If specified, uses the `numpy` random number generator `gen` to generate
            uniform random samples.  Otherwise, uses the default generation method
            in `numpy`.

        .. versionadded:: 16.2.0
           Replaces the functionality of `MixedBehaviorProfile.randomize()`.

        See Also
        --------
        mixed_behavior_profile : Create a `MixedBehaviorProfile` with specified probabilities.
        """
        if not self.is_tree:
            raise UndefinedOperationError(
                "Game must have a tree representation to create a mixed behavior profile"
            )
        if denom is None:
            profile = self.mixed_behavior_profile()
            for infoset in self.infosets:
                for action, prob in zip(
                        infoset.actions,
                        scipy.stats.dirichlet(alpha=[1 for action in infoset.actions],
                                              seed=gen).rvs(size=1)[0],
                        strict=True
                ):
                    profile[action] = prob
            return profile
        elif denom < 1:
            raise ValueError("random_behavior_profile(): denom must be positive")
        else:
            profile = self.mixed_behavior_profile(rational=True)
            for infoset in self.infosets:
                k = len(infoset.actions)
                sample = (
                    [0] +
                    sorted(
                        (gen or np.random).choice(np.arange(1, denom+k), size=k-1, replace=False)
                    ) +
                    [denom + k]
                )
                for action, (hi, lo) in zip(
                    infoset.actions,
                    zip(
                        sample[1:], sample[:-1],
                        strict=True
                    ),
                    strict=True
                ):
                    profile[action] = Rational(hi - lo - 1, denom)
            return profile

    def strategy_support_profile(
            self, strategies: typing.Callable | None = None
    ) -> StrategySupportProfile:
        """Create a new `StrategySupportProfile` on the game.

        Parameters
        ----------
        strategies : function, optional
            By default the support profile contains all strategies for all players.
            If specified, only strategies for which the supplied function returns `True`
            are included.

        Returns
        -------
        StrategySupportProfile
        """
        profile = StrategySupportProfile.wrap(make_shared[c_StrategySupportProfile](self.game))
        if strategies is not None:
            for strategy in self.strategies:
                if not strategies(strategy):
                    if not (deref(profile.profile)
                            .RemoveStrategy(cython.cast(Strategy, strategy).strategy)):
                        raise ValueError("attempted to remove the last strategy for player")
        return profile

    @cython.cfunc
    def _to_format(
        self,
        writer: GameWriter,
        filepath_or_buffer: str | pathlib.Path | io.IOBase | None = None
    ):
        serialized_game = writer(self.game)
        if filepath_or_buffer is None:
            return serialized_game.decode()
        if isinstance(filepath_or_buffer, io.TextIOBase):
            filepath_or_buffer.write(serialized_game.decode())
        elif isinstance(filepath_or_buffer, io.IOBase):
            filepath_or_buffer.write(serialized_game)
        else:
            with open(filepath_or_buffer, "w") as f:
                f.write(serialized_game.decode())

    def to_efg(
        self,
        filepath_or_buffer: str | pathlib.Path | io.IOBase | None = None
    ) -> str | None:
        """Save the game to an .efg file or return its serialized representation

        Parameters
        ----------
        filepath_or_buffer : str or Path or io.IOBase or None, default None
            String, path object, or file-like object implementing a write() function.
            If None, the result is returned as a string.

        Return
        ------
        String representation of the game or None if the game is saved to a file

        See Also
        --------
        to_nfg, to_html, to_latex
        """
        return self._to_format(WriteEfgFile, filepath_or_buffer)

    def to_nfg(
        self,
        filepath_or_buffer: str | pathlib.Path | io.IOBase | None = None
    ) -> str | None:
        """Save the game to a .nfg file or return its serialized representation

        Parameters
        ----------
        filepath_or_buffer : str or Path or BufferedWriter or None, default None
            String, path object, or file-like object implementing a write() function.
            If None, the result is returned as a string.

        Return
        ------
        String representation of the game or None if the game is saved to a file

        See Also
        --------
        to_efg, to_html, to_latex
        """
        return self._to_format(WriteNfgFile, filepath_or_buffer)

    def to_html(
        self,
        filepath_or_buffer: str | pathlib.Path | io.IOBase | None = None
    ) -> str | None:
        """Export the game to HTML format.

        Generates a rendering of the strategic form of the game as a
        collection of HTML tables.  The first player is the row
        chooser; the second player the column chooser.  For games with
        more than two players, a collection of tables is generated,
        one for each possible strategy combination of players 3 and higher.

        Parameters
        ----------
        filepath_or_buffer : str or Path or BufferedWriter or None, default None
            String, path object, or file-like object implementing a write() function.
            If None, the result is returned as a string.

        Return
        ------
        String representation of the game or None if the game is exported to a file

        See Also
        --------
        to_efg, to_nfg, to_latex
        """
        return self._to_format(WriteHTMLFile, filepath_or_buffer)

    def to_latex(
        self,
        filepath_or_buffer: str | pathlib.Path | io.IOBase | None = None
    ) -> str | None:
        """Export the game to LaTeX format.

        Generates a rendering of the strategic form of the game in
        LaTeX, suitable for use with `Martin Osborne's sgame style
        <https://www.economics.utoronto.ca/osborne/latex/>`_.
        The first player is the row
        chooser; the second player the column chooser.  For games with
        more than two players, a collection of tables is generated,
        one for each possible strategy combination of players 3 and higher.

        Parameters
        ----------
        filepath_or_buffer : str or Path or BufferedWriter or None, default None
            String, path object, or file-like object implementing a write() function.
            If None, the result is returned as a string.

        Return
        ------
        String representation of the game or None if the game is exported to a file

        See Also
        --------
        to_efg, to_nfg, to_html
        """
        return self._to_format(WriteLaTeXFile, filepath_or_buffer)

    def _resolve_player(self,
                        player: typing.Any, funcname: str, argname: str = "player") -> Player:
        """Resolve an attempt to reference a player of the game.

        Parameters
        ----------
        player : Any
            An object to resolve as a reference to a player.
        funcname : str
            The name of the function to raise any exception on behalf of.
        argname : str, default 'player'
            The name of the argument being checked

        Raises
        ------
        MismatchError
            If `player` is a `Player` from a different game.
        KeyError
            If `player` is a string and no player in the game has that label.
        TypeError
            If `player` is not a `Player` or a `str`
        ValueError
            If `player` is an empty `str` or all spaces
        """
        if isinstance(player, Player):
            if player.game != self:
                raise MismatchError(f"{funcname}(): {argname} must be part of the same game")
            return player
        elif isinstance(player, str):
            if not player.strip():
                raise ValueError(
                    f"{funcname}(): {argname} cannot be an empty string or all spaces"
                )
            try:
                return self.players[player]
            except KeyError:
                raise KeyError(f"{funcname}(): no player with label '{player}'")
        raise TypeError(
            f"{funcname}(): {argname} must be Player or str, not {player.__class__.__name__}"
        )

    def _resolve_outcome(self,
                         outcome: typing.Any, funcname: str, argname: str = "outcome") -> Outcome:
        """Resolve an attempt to reference an outcome of the game.

        Parameters
        ----------
        outcome : Any
            An object to resolve as a reference to an outcome.
        funcname : str
            The name of the function to raise any exception on behalf of.
        argname : str, default 'outcome'
            The name of the argument being checked

        Raises
        ------
        MismatchError
            If `outcome` is an `Outcome` from a different game.
        KeyError
            If `outcome` is a string and no outcome in the game has that label.
        TypeError
            If `outcome` is not an `Outcome` or a `str`
        ValueError
            If `outcome` is an empty `str` or all spaces
        """
        if isinstance(outcome, Outcome):
            if outcome.game != self:
                raise MismatchError(f"{funcname}(): {argname} must be part of the same game")
            return outcome
        elif isinstance(outcome, str):
            if not outcome.strip():
                raise ValueError(
                    f"{funcname}(): {argname} cannot be an empty string or all spaces"
                )
            try:
                return self.outcomes[outcome]
            except KeyError:
                raise KeyError(f"{funcname}(): no node with label '{outcome}'")
        raise TypeError(
            f"{funcname}(): {argname} must be Outcome or str, not {outcome.__class__.__name__}"
        )

    def _resolve_strategy(self,
                          strategy: typing.Any,
                          funcname: str,
                          argname: str = "strategy") -> Strategy:
        """Resolve an attempt to reference a strategy of the game.

        Parameters
        ----------
        strategy : Any
            An object to resolve as a reference to a strategy.
        funcname : str
            The name of the function to raise any exception on behalf of.
        argname : str, default 'strategy'
            The name of the argument being checked

        Raises
        ------
        MismatchError
            If `strategy` is a `Strategy` from a different game.
        KeyError
            If `strategy` is a string and no strategy in the game has that label.
        TypeError
            If `strategy` is not a `Strategy` or a `str`
        ValueError
            If `strategy` is an empty `str` or all spaces
        """
        if isinstance(strategy, Strategy):
            if strategy.game != self:
                raise MismatchError(f"{funcname}(): {argname} must be part of the same game")
            return strategy
        elif isinstance(strategy, str):
            if not strategy.strip():
                raise ValueError(
                    f"{funcname}(): {argname} cannot be an empty string or all spaces"
                )
            try:
                return self.strategies[strategy]
            except KeyError:
                raise KeyError(f"{funcname}(): no strategy with label '{strategy}'")
        raise TypeError(
            f"{funcname}(): {argname} must be Strategy or str, not {strategy.__class__.__name__}"
        )

    def _resolve_node(self, node: typing.Any, funcname: str, argname: str = "node") -> Node:
        """Resolve an attempt to reference a node of the game.

        Parameters
        ----------
        node : Any
            An object to resolve as a reference to a node.
        funcname : str
            The name of the function to raise any exception on behalf of.
        argname : str, default 'node'
            The name of the argument being checked

        Raises
        ------
        MismatchError
            If `node` is a `Node` from a different game.
        KeyError
            If `node` is a string and no node in the game has that label.
        TypeError
            If `node` is not a `Node` or a `str`
        ValueError
            If `node` is an empty `str` or all spaces
        """
        if isinstance(node, Node):
            if node.game != self:
                raise MismatchError(f"{funcname}(): {argname} must be part of the same game")
            return node
        elif isinstance(node, str):
            if not node.strip():
                raise ValueError(
                    f"{funcname}(): {argname} cannot be an empty string or all spaces"
                )
            for n in self.nodes:
                if n.label == node:
                    return n
            raise KeyError(f"{funcname}(): no node with label '{node}'")
        raise TypeError(
            f"{funcname}(): {argname} must be Node or str, not {node.__class__.__name__}"
        )

    def _resolve_nodes(self,
                       nodes: typing.Any,
                       funcname: str,
                       argname: str = "nodes") -> list[Node]:
        """Resolve an attempt to reference a subset of the nodes of the game of the game.

        See `_resolve_node` for details on functionality.
        """
        resolved_nodes = [
            self._resolve_node(n, funcname, argname)
            for n in (nodes if hasattr(nodes, "__iter__") and not isinstance(nodes, str)
                      else [nodes])
        ]
        if not resolved_nodes:
            raise ValueError(f"{funcname}(): `{argname}` must not be empty")
        if len(resolved_nodes) != len(set(resolved_nodes)):
            raise ValueError(f"{funcname}(): Each node must be referenced only once")
        return resolved_nodes

    def _resolve_infoset(self,
                         infoset: typing.Any, funcname: str, argname: str = "infoset") -> Infoset:
        """Resolve an attempt to reference an information set of the game.

        Parameters
        ----------
        infoset : Any
            An object to resolve as a reference to an information set.
        funcname : str
            The name of the function to raise any exception on behalf of.
        argname : str, default 'infoset'
            The name of the argument being checked

        Raises
        ------
        MismatchError
            If `infoset` is an `Infoset` from a different game.
        KeyError
            If `infoset` is a string and no information set in the game has that label.
        TypeError
            If `infoset` is not an `Infoset` or a `str`
        ValueError
            If `infoset` is an empty `str` or all spaces
        """
        if isinstance(infoset, Infoset):
            if infoset.game != self:
                raise MismatchError(f"{funcname}(): {argname} must be part of the same game")
            return infoset
        elif isinstance(infoset, str):
            if not infoset.strip():
                raise ValueError(
                    f"{funcname}(): {argname} cannot be an empty string or all spaces"
                )
            try:
                return self.infosets[infoset]
            except KeyError:
                raise KeyError(f"{funcname}(): no information set with label '{infoset}'")
        raise TypeError(
            f"{funcname}(): {argname} must be Infoset or str, not {infoset.__class__.__name__}"
        )

    def _resolve_action(self,
                        action: typing.Any, funcname: str, argname: str = "action") -> Action:
        """Resolve an attempt to reference an action of the game.

        Parameters
        ----------
        action : Any
            An object to resolve as a reference to an action.
        funcname : str
            The name of the function to raise any exception on behalf of.
        argname : str, default 'action'
            The name of the argument being checked

        Raises
        ------
        MismatchError
            If `action` is an `Action` from a different game.
        KeyError
            If `action` is a string and no action in the game has that label.
        TypeError
            If `action` is not an `Action` or a `str`
        ValueError
            If `action` is an empty `str` or all spaces
        """
        if isinstance(action, Action):
            if action.infoset.game != self:
                raise MismatchError(f"{funcname}(): {argname} must be part of the same game")
            return action
        elif isinstance(action, str):
            if not action.strip():
                raise ValueError(
                    f"{funcname}(): {argname} cannot be an empty string or all spaces"
                )
            try:
                return self.actions[action]
            except KeyError:
                raise KeyError(f"{funcname}(): no action with label '{action}'")
        raise TypeError(
            f"{funcname}(): {argname} must be Action or str, not {action.__class__.__name__}"
        )

    def append_move(self, nodes: Node | NodeReferenceSet,
                    player: Player | str,
                    actions: list[str]) -> None:
        """Add a move for `player` at terminal `nodes`.  All elements of `nodes` become part of
        a new information set, with actions labeled according to `actions`.

        Raises
        ------
        UndefinedOperationError
            If `nodes` are not all terminal, or `actions` is not a positive number.
        MismatchError
            If an element from `nodes` is a `Node` from a different game,
            or `player` is a `Player` from a different game.
        ValueError
            If `nodes` has duplicated elements, or is empty.
        """
        resolved_player = cython.cast(Player, self._resolve_player(player, "append_move"))
        if not actions:
            raise UndefinedOperationError("append_move(): `actions` must be a nonempty list")
        resolved_nodes = self._resolve_nodes(nodes, "append_move", "nodes")
        if any(len(n.children) > 0 for n in resolved_nodes):
            raise UndefinedOperationError("append_move(): `nodes` must be terminal nodes")

        resolved_node = cython.cast(Node, resolved_nodes[0])
        self.game.deref().AppendMove(resolved_node.node, resolved_player.player, len(actions))
        for label, action in zip(actions, resolved_node.infoset.actions, strict=True):
            action.label = label
        resolved_infoset = cython.cast(Infoset, resolved_node.infoset)
        for n in resolved_nodes[1:]:
            self.game.deref().AppendMove(cython.cast(Node, n).node, resolved_infoset.infoset)

    def append_infoset(self, nodes: Node | NodeReferenceSet,
                       infoset: Infoset | str) -> None:
        """Add a move in information set `infoset` at terminal `nodes`.

        Raises
        ------
        UndefinedOperationError
            If any element in `nodes` is not a terminal node.
        MismatchError
            If an element in `nodes` is a `Node` from a different game,
            or `infoset` is an `Infoset` from a different game.
        ValueError
            If `nodes` has duplicated elements, or is empty.
        """
        resolved_infoset = cython.cast(Infoset, self._resolve_infoset(infoset, "append_infoset"))
        resolved_nodes = self._resolve_nodes(nodes, "append_infoset", "nodes")
        if any(len(n.children) > 0 for n in resolved_nodes):
            raise UndefinedOperationError("append_infoset(): `nodes` must be terminal nodes")
        for n in resolved_nodes:
            self.game.deref().AppendMove(cython.cast(Node, n).node, resolved_infoset.infoset)

    def insert_move(self, node: Node | str,
                    player: Player | str, actions: int) -> None:
        """Insert a move for `player` prior to the node `node`, with `actions` actions.
        `node` becomes the first child of the newly-inserted node.

        Raises
        ------
        UndefinedOperationError
            If `actions` is not a positive number.
        MismatchError
            If `node` is a `Node` from a different game, or `player` is a `Player` from a
            different game.
        """
        resolved_node = cython.cast(Node, self._resolve_node(node, "insert_move"))
        resolved_player = cython.cast(Player, self._resolve_player(player, "insert_move"))
        if actions < 1:
            raise UndefinedOperationError("insert_move(): `actions` must be a positive number")
        self.game.deref().InsertMove(resolved_node.node, resolved_player.player, actions)

    def insert_infoset(self, node: Node | str,
                       infoset: Infoset | str) -> None:
        """Insert a move in information set `infoset` prior to the node `node`.
        `node` becomes the first child of the newly-inserted node.

        Raises
        ------
        MismatchError
            If `node` is a `Node` from a different game, or `infoset` is an `Infoset` from a
            different game.
        """
        resolved_node = cython.cast(Node, self._resolve_node(node, "insert_infoset"))
        resolved_infoset = cython.cast(Infoset, self._resolve_infoset(infoset, "insert_infoset"))
        self.game.deref().InsertMove(resolved_node.node, resolved_infoset.infoset)

    def copy_tree(self, src: Node | str, dest: Node | str) -> None:
        """Copy the subtree rooted at the node `src` to the node `dest`.

        Each node in the subtree copied to follow `dest` is placed in the same information set
        as the corresponding node in the original subtree under `src`.

        It is permitted for `dest` to be a descendant of `src`.
        The operation uses the subtree rooted at `src` as it is at the time the function is called,
        so no infinite recursion is triggered.

        The outcome associated with `dest` is not changed by this operation.

        Parameters
        ----------
        src : Node or str
            The root of the source subtree to copy
        dest : Node or str
            The destination subtree to copy to.  `dest` must be a terminal node.

        Raises
        ------
        MismatchError
            If `src` or `dest` is not a member of the same game as this node.
        UndefinedOperationError
            If `dest` is not a terminal node.
        """
        resolved_src = cython.cast(Node, self._resolve_node(src, "copy_tree", "src"))
        resolved_dest = cython.cast(Node, self._resolve_node(dest, "copy_tree", "dest"))
        if not resolved_dest.is_terminal:
            raise UndefinedOperationError("copy_tree(): `dest` must be a terminal node.")
        self.game.deref().CopyTree(resolved_dest.node, resolved_src.node)

    def move_tree(self, src: Node | str, dest: Node | str) -> None:
        """Move the subtree rooted at 'src' to 'dest'.

        Parameters
        ----------
        src : Node or str
            The root of the source subtree to move
        dest : Node or str
            The destination subtree to move to.  `dest` must be a terminal node.

        Raises
        ------
        MismatchError
            If `src` or `dest` is not a member of the same game as this node.
        UndefinedOperationError
            If `dest` is not a terminal node, or `dest` is a successor of `src`.
        """
        resolved_src = cython.cast(Node, self._resolve_node(src, "move_tree", "src"))
        resolved_dest = cython.cast(Node, self._resolve_node(dest, "move_tree", "dest"))
        if not resolved_dest.is_terminal:
            raise UndefinedOperationError("move_tree(): `dest` must be a terminal node.")
        if resolved_dest.is_successor_of(resolved_src):
            raise UndefinedOperationError("move_tree(): `dest` cannot be a successor of `src`.")
        self.game.deref().MoveTree(resolved_dest.node, resolved_src.node)

    def delete_parent(self, node: Node | str) -> None:
        """Delete the parent node of `node`.  `node` replaces its parent in the tree.  All other
        subtrees rooted at `node`'s parent are deleted.

        Parameters
        ----------
        node : Node or str
            The node to retain after deleting its parent.
            If a string is passed, the node is determined by finding the node with that label,
            if any.

        Raises
        ------
        MismatchError
            If `node` is a `Node` from a different game.
        """
        resolved_node = cython.cast(Node, self._resolve_node(node, "delete_parent"))
        self.game.deref().DeleteParent(resolved_node.node)

    def delete_tree(self, node: Node | str) -> None:
        """Truncate the game tree at `node`, deleting the subtree beneath it.

        Parameters
        ----------
        node : Node or str
            The node to truncate the game at.  If a string is passed, the node is determined by
            finding the node with that label, if any.

        Raises
        ------
        MismatchError
            If `node` is a `Node` from a different game.
        """
        resolved_node = cython.cast(Node, self._resolve_node(node, "delete_tree"))
        self.game.deref().DeleteTree(resolved_node.node)

    def add_action(self,
                   infoset: Infoset | str,
                   before: Action | str | None = None) -> None:
        """Add an action at the information set `infoset`.   If `before` is not null, the new
        action is inserted before `before`.

        Parameters
        ----------
        infoset : Infoset or str
            The information set at which to add an action
        before : Action or str, optional
            The action before which to add the new action.  If `before` is not specified,
            the new action is the first at the information set

        Raises
        ------
        MismatchError
            If `infoset` is an `Infoset` from a different game, `before` is an `Action`
            from a different game, or `before` is not an action at `infoset`.
        """
        resolved_infoset = cython.cast(Infoset, self._resolve_infoset(infoset, "add_action"))
        if before is None:
            self.game.deref().InsertAction(resolved_infoset.infoset,
                                           cython.cast(c_GameAction, NULL))
        else:
            resolved_action = cython.cast(
                Action, self._resolve_action(before, "add_action", "before")
            )
            if resolved_infoset != resolved_action.infoset:
                raise MismatchError("add_action(): must specify an action from the same infoset")
            self.game.deref().InsertAction(resolved_infoset.infoset, resolved_action.action)

    def delete_action(self, action: Action | str) -> None:
        """Deletes `action` from its information set.  The subtrees which
        are rooted at nodes that follow the deleted action are also deleted.
        If the action is at a chance node then the probabilities of any remaining actions
        are normalized to sum to one; if all remaining actions previously had probability zero
        then this normalization gives those remaining actions all equal probability.

        Raises
        ------
        UndefinedOperationError
            If `action` is the only action at its information set.
        MismatchError
            If `action` is an `Action` from a different game.
        """
        resolved_action = cython.cast(Action, self._resolve_action(action, "delete_action"))
        if len(resolved_action.infoset.actions) == 1:
            raise UndefinedOperationError(
                "delete_action(): cannot delete the only action at an information set"
            )
        self.game.deref().DeleteAction(resolved_action.action)

    def leave_infoset(self, node: Node | str):
        """Remove this node from its information set. If this node is the only node
        in its information set, this operation has no effect.

        Parameters
        ----------
        node : Node or str
            The node to move to a new singleton information set.
        """
        resolved_node = cython.cast(Node, self._resolve_node(node, "leave_infoset"))
        self.game.deref().LeaveInfoset(resolved_node.node)

    def set_infoset(self,
                    node: Node | str,
                    infoset: Infoset | str) -> None:
        """Place `node` in the information set `infoset`.  `node` must have the same
        number of descendants as `infoset` has actions.

        Parameters
        ----------
        node : Node or str
            The node to set the information set
        infoset : Infoset or str
            The information set to join

        Raises
        ------
        MismatchError
            If `node` is a `Node` from a different game, or `infoset` is an `Infoset` from
            a different game.
        """
        resolved_node = cython.cast(Node, self._resolve_node(node, "set_infoset"))
        resolved_infoset = cython.cast(Infoset, self._resolve_infoset(infoset, "set_infoset"))
        if len(resolved_node.children) != len(resolved_infoset.actions):
            raise ValueError(
                "set_infoset(): `infoset` must have same number of actions as `node` has children."
            )
        self.game.deref().SetInfoset(resolved_node.node, resolved_infoset.infoset)

    def reveal(self,
               infoset: Infoset | str,
               player: Player | str) -> None:
        """Reveals the move made at `infoset` to `player`.

        Revealing the move modifies all subsequent information sets for `player` such
        that any two nodes which are successors of two different actions at this
        information set are placed in different information sets for `player`.

        Revelation is a one-shot operation; it is not enforced with respect to any
        revisions made to the game tree subsequently.

        Parameters
        ----------
        infoset : Infoset or str
            The information set of the move to reveal to the player
        player : Player or str
            The player to which to reveal the move at this information set.

        Raises
        ------
        MismatchError
            If `infoset` is an `Infoset` from a different game, or
            `player` is a `Player` from a different game.
        """
        resolved_infoset = cython.cast(Infoset, self._resolve_infoset(infoset, "reveal"))
        resolved_player = cython.cast(Player, self._resolve_player(player, "reveal"))
        self.game.deref().Reveal(resolved_infoset.infoset, resolved_player.player)

    def sort_infosets(self) -> None:
        """Sort information sets into a standard order.

        .. deprecated:: 16.5.0
           This operation is deprecated as efficient management of the iteration orders of
           information sets and their members is now handled by the representation objects.

        Raises
        ------
        UndefinedOperationError
            If the game does not have a tree representation.
        """
        warnings.warn(
            "sort_infosets() is deprecated; This operation is now done automatically when"
            " required. "
            "This function will be removed in a future release.",
            FutureWarning
        )
        if not self.is_tree:
            raise UndefinedOperationError(
                "Operation only defined for games with a tree representation"
            )

    def add_player(self, label: str = "") -> Player:
        """Add a new player to the game.

        Parameters
        ----------
        label : str, default ""
            The label for the player.

        Returns
        -------
        Player
            A reference to the newly-created player.
        """
        p = Player.wrap(self.game.deref().NewPlayer())
        if str(label) != "":
            p.label = str(label)
        return p

    def set_player(self, infoset: Infoset | str,
                   player: Player | str) -> None:
        """Set the player at an information set.

        Parameters
        ----------
        infoset : Infoset or str
            The information set to assign to the player
        player : Player or str
            The player to have the move at the information set

        Raises
        ------
        MismatchError
            If `infoset` is an `Infoset` from another game, or `player` is a
            `Player` from another game.
        """
        resolved_player = cython.cast(Player, self._resolve_player(player, "set_player"))
        resolved_infoset = cython.cast(Infoset, self._resolve_infoset(infoset, "set_player"))
        self.game.deref().SetPlayer(resolved_infoset.infoset, resolved_player.player)

    def add_outcome(self,
                    payoffs: list | None = None,
                    label: str = "") -> Outcome:
        """Add a new outcome to the game.

        Parameters
        ----------
        payoffs : list, optional
            The payoffs of the outcome to each player.
        label : str, default ""
            The label for the outcome

        Raises
        ------
        ValueError
            If `payoffs` is specified but is not the same length as the number of players
            in the game.

        Returns
        -------
        Outcome
            A reference to the newly-created outcome.
        """
        if payoffs is not None:
            if len(payoffs) != len(self.players):
                raise ValueError("add_outcome(): number of payoffs must equal number of players")
        else:
            payoffs = [0 for _ in self.players]
        c = Outcome.wrap(self.game.deref().NewOutcome())
        if str(label) != "":
            c.label = str(label)
        for player, payoff in zip(self.players, payoffs, strict=True):
            c[player] = payoff
        return c

    def delete_outcome(self, outcome: Outcome | str) -> None:
        """Delete an outcome from the game.

        If this game is an extensive game, any
        node at which this outcome is attached has its outcome reset to null.  If this game
        is a strategic game, any contingency at which this outcome is attached as its outcome
        reset to null.

        Parameters
        ----------
        outcome : Outcome or str
            The outcome to delete from the game

        Raises
        ------
        MismatchError
            If `outcome` is an `Outcome` from another game.
        """
        resolved_outcome = cython.cast(Outcome, self._resolve_outcome(outcome, "delete_outcome"))
        self.game.deref().DeleteOutcome(resolved_outcome.outcome)

    def set_outcome(self, node: Node | str,
                    outcome: Outcome | str | None) -> None:
        """Set `outcome` to be the outcome at `node`.  If `outcome` is None, the
        outcome at `node` is unset.

        Parameters
        ----------
        node : Node or str
            The node to set the outcome at
        outcome : Outcome or str or None
            The outcome to assign to the node

        Raises
        ------
        MismatchError
            If `node` is a `Node` from a different game, or `outcome` is an
            `Outcome` from a different game.
        """
        resolved_node = cython.cast(Node, self._resolve_node(node, "set_outcome"))
        if outcome is None:
            self.game.deref().SetOutcome(resolved_node.node, cython.cast(c_GameOutcome, NULL))
            return
        resolved_outcome = cython.cast(Outcome, self._resolve_outcome(outcome, "set_outcome"))
        self.game.deref().SetOutcome(resolved_node.node, resolved_outcome.outcome)

    def add_strategy(self, player: Player | str, label: str = None) -> Strategy:
        """Add a new strategy to the set of strategies for `player`.

        Parameters
        ----------
        player : Player or str
            The player to create the new strategy for
        label : str, optional
            The label to assign to the new strategy

        Returns
        -------
        Strategy
            The newly-created strategy

        Raises
        ------
        MismatchError
            If `player` is a `Player` from a different game.
        UndefinedOperationError
            If called on a game which has an extensive representation.
        """
        if self.is_tree:
            raise UndefinedOperationError(
                "Adding strategies is only applicable to games in strategic form"
            )
        resolved_player = cython.cast(Player,
                                      self._resolve_player(player, "add_strategy"))
        return Strategy.wrap(
            self.game.deref().NewStrategy(resolved_player.player,
                                          (str(label) if label is not None else "").encode())
        )

    def delete_strategy(self, strategy: Strategy | str) -> None:
        """Delete `strategy` from the game.

        Parameters
        ----------
        strategy : Strategy or str
            The strategy to delete

        Raises
        ------
        MismatchError
            If `strategy` is a `strategy` from a different game.
        UndefinedOperationError
            If called on a game which has an extensive representation, or if `strategy` is the
            only strategy for its player.
        """
        if self.is_tree:
            raise UndefinedOperationError(
                "Deleting strategies is only applicable to games in strategic form"
            )
        resolved_strategy = cython.cast(
            Strategy, self._resolve_strategy(strategy, "delete_strategy")
        )
        if len(resolved_strategy.player.strategies) == 1:
            raise UndefinedOperationError("Cannot delete the only strategy for a player")
        self.game.deref().DeleteStrategy(resolved_strategy.strategy)


@dataclasses.dataclass
class NodeCoordinates:
    level: int
    sublevel: int
    offset: float


@cython.cfunc
def _layout_tree(game: Game) -> dict[GameNode, NodeCoordinates]:
    layout = CreateLayout(game.game)
    data = {}
    for node in game.nodes:
        data[node] = NodeCoordinates(deref(layout).GetNodeLevel(cython.cast(Node, node).node),
                                     deref(layout).GetNodeSublevel(cython.cast(Node, node).node),
                                     deref(layout).GetNodeOffset(cython.cast(Node, node).node))
    return data


def layout_tree(game: Game) -> dict[GameNode, dict]:
    return _layout_tree(game)
