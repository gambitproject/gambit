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
ctypedef c_Game (*GameParser)(const string &) except +IOError


@cython.cfunc
def read_game(filepath_or_buffer: str | pathlib.Path | io.IOBase,
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
        g = Game.wrap(parser(data))
    except Exception as exc:
        raise ValueError(f"Parse error in game file: {exc}") from None
    return g


def read_gbt(filepath_or_buffer: str | pathlib.Path | io.IOBase) -> Game:
    """Construct a game from its serialised representation in a GBT file.

    Parameters
    ----------
    filepath_or_buffer : str, pathlib.Path or io.IOBase
        The path to the file containing the game representation or file-like object

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
    read_efg, read_nfg, read_agg, read_bagg
    """
    return read_game(filepath_or_buffer, parser=ParseGbtGame)


def read_efg(filepath_or_buffer: str | pathlib.Path | io.IOBase) -> Game:
    """Construct a game from its serialised representation in an EFG file.

    Parameters
    ----------
    filepath_or_buffer : str, pathlib.Path or io.IOBase
        The path to the file containing the game representation or file-like object

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
    read_gbt, read_nfg, read_agg, read_bagg
    """
    return read_game(filepath_or_buffer, parser=ParseEfgGame)


def read_nfg(filepath_or_buffer: str | pathlib.Path | io.IOBase) -> Game:
    """Construct a game from its serialised representation in a NFG file.

    Parameters
    ----------
    filepath_or_buffer : str, pathlib.Path or io.IOBase
        The path to the file containing the game representation or file-like object

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
    read_gbt, read_efg, read_agg, read_bagg
    """
    return read_game(filepath_or_buffer, parser=ParseNfgGame)


def read_agg(filepath_or_buffer: str | pathlib.Path | io.IOBase) -> Game:
    """Construct a game from its serialised representation in an AGG file.

    Parameters
    ----------
    filepath_or_buffer : str, pathlib.Path or io.IOBase
        The path to the file containing the game representation or file-like object

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
    read_gbt, read_efg, read_nfg, read_bagg
    """
    return read_game(filepath_or_buffer, parser=ParseAggGame)


def read_bagg(filepath_or_buffer: str | pathlib.Path | io.IOBase) -> Game:
    """Construct a game from its serialised representation in a BAGG file.

    Parameters
    ----------
    filepath_or_buffer : str, pathlib.Path or io.IOBase
        The path to the file containing the game representation or file-like object

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
    read_gbt, read_efg, read_nfg, read_agg
    """
    return read_game(filepath_or_buffer, parser=ParseBaggGame)


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
class GameSubgames:
    """Represents the set of subgames in a game."""
    game = cython.declare(c_Game)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create GameSubgames outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(game: c_Game) -> GameSubgames:
        obj: GameSubgames = GameSubgames.__new__(GameSubgames)
        obj.game = game
        return obj

    def __repr__(self) -> str:
        return f"GameSubgames(game={Game.wrap(self.game)})"

    def __len__(self) -> int:
        """The number of subgames in the game."""
        if not self.game.deref().IsTree():
            return 0
        return self.game.deref().GetSubgames().size()

    def __iter__(self) -> typing.Iterator[Subgame]:
        """Iterate over the game subgames in postorder."""
        if not self.game.deref().IsTree():
            return
        for subgame in self.game.deref().GetSubgames():
            yield Subgame.wrap(subgame)


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

    def __getitem__(self, label: str) -> Outcome:
        """Returns the outcome with text label `label`.

        Parameters
        ----------
        label : str
            The text label of the outcome to return.  Lookup is by exact match;
            leading/trailing whitespace is stripped from `label`.

        Raises
        ------
        KeyError
            If no outcome in the game has label `label`.
        ValueError
            If `label` is empty or all whitespace, or if more than one outcome has label `label`.
        TypeError
            If `label` is not a string.

        .. versionchanged:: 16.7.0
            Integer indexing is no longer supported; reference an outcome by its label, or iterate
            over the collection.  String lookup now requires an exact match of the label;
            previously, leading/trailing whitespace was stripped from `label` before comparison.
        """
        return _resolve_by_label(self, label, "Game", "outcome", "outcomes")


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

    def __getitem__(self, label: str) -> Player:
        """Returns the player with text label `label`.

        Parameters
        ----------
        label : str
            The text label of the player to return.  Lookup is by exact match;
            leading/trailing whitespace is stripped from `label`.

        Raises
        ------
        KeyError
            If no player in the game has label `label`.
        ValueError
            If `label` is empty or all whitespace, or if more than one player has label `label`.
        TypeError
            If `label` is not a string.

        .. versionchanged:: 16.7.0
            Integer indexing is no longer supported; reference a player by its label, or iterate
            over the collection.  String lookup now requires an exact match of the label;
            previously, leading/trailing whitespace was stripped from `label` before comparison.
        """
        return _resolve_by_label(self, label, "Game", "player", "players")

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

    def __getitem__(self, label: str) -> Action:
        """Returns the action with text label `label`.

        Parameters
        ----------
        label : str
            The text label of the action to return.  Lookup is by exact match;
            leading/trailing whitespace is stripped from `label`.

        Raises
        ------
        KeyError
            If no action in the game has label `label`.
        ValueError
            If `label` is empty or all whitespace, or if more than one action has label `label`.
        TypeError
            If `label` is not a string.

        .. versionchanged:: 16.7.0
            Integer indexing is no longer supported; reference an action by its label, or iterate
            over the collection.  String lookup now requires an exact match of the label;
            previously, leading/trailing whitespace was stripped from `label` before comparison.
        """
        return _resolve_by_label(self, label, "Game", "action", "actions")


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

    def __getitem__(self, label: str) -> Infoset:
        """Returns the information set with text label `label`.

        Parameters
        ----------
        label : str
            The text label of the infoset to return.  Lookup is by exact match;
            leading/trailing whitespace is stripped from `label`.

        Raises
        ------
        KeyError
            If no information set in the game has label `label`.
        ValueError
            If `label` is empty or all whitespace, or if more than one information set has
            label `label`.
        TypeError
            If `label` is not a string.

        .. versionchanged:: 16.7.0
            Integer indexing is no longer supported; reference an information set by its label,
            or iterate over the collection.  String lookup now requires an exact match of the
            label; previously, leading/trailing whitespace was stripped from `label` before
            comparison.
        """
        return _resolve_by_label(self, label, "Game", "infoset", "infosets")


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

    def __getitem__(self, label: str) -> Strategy:
        """Returns the strategy with text label `label`.

        Parameters
        ----------
        label : str
            The text label of the strategy to return.  Lookup is by exact match;
            leading/trailing whitespace is stripped from `label`.

        Raises
        ------
        KeyError
            If no strategy in the game has label `label`.
        ValueError
            If `label` is empty or all whitespace, or if more than one strategy has label `label`.
        TypeError
            If `label` is not a string.

        .. versionchanged:: 16.7.0
            Integer indexing is no longer supported; reference a strategy by its label, or iterate
            over the collection.  String lookup now requires an exact match of the label;
            previously, leading/trailing whitespace was stripped from `label` before comparison.
        """
        return _resolve_by_label(self, label, "Game", "strategy", "strategies")


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
            g.game.deref().NewPlayer(str(player).encode("utf-8"))
        return g

    @classmethod
    def new_table(cls, dim, title: str = "Untitled strategic game") -> Game:
        """Create a new ``Game`` with a strategic representation.

        Players are labeled ``"1"``, ``"2"``, and so on;
        each player's strategies are likewise labeled ``"1"``, ``"2"``, and so on.

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

        Players are labeled ``"1"``, ``"2"``, and so on;
        each player's strategies are likewise labeled ``"1"``, ``"2"``, and so on.

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

        The players are labeled with the keys of `payoffs`, and therefore
        must be valid player labels.  Each player's strategies are labeled
        ``"1"``, ``"2"``, and so on.

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
        to be short.  Unlike object labels, a title has no printable-character or
        spacing restriction; it need only be well-formed UTF-8 text.

        .. versionchanged:: 17.0.0
            Must be well-formed UTF-8 text; an invalid value now raises ``ValueError``.
        """
        return self.game.deref().GetTitle().decode("utf-8")

    @title.setter
    def title(self, value: str) -> None:
        self.game.deref().SetTitle(value.encode("utf-8"))

    @property
    def description(self) -> str:
        """Get or set the description of the game.

        A game's description is an arbitrary string, and may be more discursive
        than a title.  Unlike object labels, a description has no printable-character
        or spacing restriction; it need only be well-formed UTF-8 text.

        .. versionchanged:: 16.6.0
           Renamed ``Game.comment`` to ``Game.description``.

        .. versionchanged:: 17.0.0
            Must be well-formed UTF-8 text; an invalid value now raises ``ValueError``.
        """
        return self.game.deref().GetDescription().decode("utf-8")

    @description.setter
    def description(self, value: str) -> None:
        self.game.deref().SetDescription(value.encode("utf-8"))

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

        See Also
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

        See Also
        --------
        Game.min_payoff
        Player.max_payoff
        """
        return rat_to_py(self.game.deref().GetMaxPayoff())

    @property
    def subgames(self) -> GameSubgames:
        """The set of subgames in the game.

        Iteration over this property yields the subgames in postorder
        (children before parents).

        .. versionadded:: 16.7.0

        Raises
        ------
        UndefinedOperationError
            If the game does not have a tree representation.
        """
        if not self.is_tree:
            raise UndefinedOperationError(
                "Operation only defined for games with a tree representation"
            )
        return GameSubgames.wrap(self.game)

    def minimal_subgame(self, infoset: typing.Union[Infoset, str]) -> Subgame:
        """Returns the smallest subgame containing `infoset`.

        Parameters
        ----------
        infoset : Infoset or str
            The information set to query.

        Returns
        -------
        Subgame
            The smallest subgame containing `infoset`.

        .. versionadded:: 16.7.0

        Raises
        ------
        UndefinedOperationError
            If the game does not have a tree representation.
        MismatchError
            If `infoset` is from a different game.
        """
        if not self.is_tree:
            raise UndefinedOperationError(
                "Operation only defined for games with a tree representation"
            )
        resolved_infoset = self._resolve_infoset(infoset, "minimal_subgame")
        return Subgame.wrap(
            self.game.deref().GetMinimalSubgame(cython.cast(Infoset, resolved_infoset).infoset)
        )

    def get_behavior(self,
                     player: Player | str,
                     strategy: Strategy | str) -> StrategyBehavior:
        """Return the mapping from information sets to actions prescribed by a strategy.

        .. versionadded:: 17.0.0

        Parameters
        ----------
        player : Player or str
            The player whose strategy to view.
        strategy : Strategy or str
            The strategy to view.

        Returns
        -------
        StrategyBehavior

        Raises
        ------
        UndefinedOperationError
            If the game does not have a tree representation.
        MismatchError
            If `player` is from a different game, or `strategy` belongs to a different player.
        KeyError
            If `strategy` is a string and `player` has no strategy with that label.

        See Also
        --------
        Strategy.action : The action prescribed at a single information set.
        """
        if not self.is_tree:
            raise UndefinedOperationError(
                "get_behavior(): only defined for games with a tree representation"
            )
        resolved_player = cython.cast(Player, self._resolve_player(player, "get_behavior"))
        if isinstance(strategy, Strategy):
            if strategy.player != resolved_player:
                raise MismatchError(
                    f"get_behavior(): strategy must belong to player "
                    f"'{resolved_player.label}'"
                )
            resolved_strategy = strategy
        elif isinstance(strategy, str):
            if not strategy.strip():
                raise ValueError(
                    "get_behavior(): strategy cannot be an empty string or all spaces"
                )
            resolved_strategy = resolved_player.strategies[strategy]
        else:
            raise TypeError(
                f"get_behavior(): strategy must be Strategy or str, "
                f"not {strategy.__class__.__name__}"
            )
        return StrategyBehavior.wrap(resolved_player, resolved_strategy)

    def _get_contingency(self, *args):
        psp: shared_ptr[c_PureStrategyProfile] = make_shared[c_PureStrategyProfile](
            self.game.deref().NewPureStrategyProfile()
        )

        for (pl, st) in enumerate(args):
            deref(deref(psp).deref()).SetStrategy(
                self.game.deref().GetPlayer(pl+1).deref().GetStrategy(st+1)
            )

        if self.is_tree or self.game.deref().IsAgg():
            return DerivedGameOutcome.wrap(self.game, psp)
        else:
            outcome = Outcome.wrap(deref(deref(psp).deref()).GetOutcome())
            if outcome.outcome != cython.cast(c_GameOutcome, NULL):
                return outcome
            else:
                return None

    def __getitem__(self, contingency):
        """Returns the `Outcome` associated with a profile of pure strategies.

        Each strategy in the profile may be given as a ``Strategy``, its text label,
        or its integer index within the corresponding player's strategies.

        Raises
        ------
        TypeError
            If `contingency` is not a tuple-like object, or contains an element
            that is not an ``int``, ``str``, or ``Strategy``.
        KeyError
            If the number of elements in `contingency` does not equal the
            number of players.
        IndexError
            If an integer index is out of range for the corresponding player,
            or a label or ``Strategy`` does not belong to that player.

        .. note::
            Unlike the game's object collections, strategies within a contingency can be referenced
            by integer index, as a contingency is a coordinate in the players' strategy spaces;
            labels and ``Strategy`` objects are also accepted.
        """
        players = list(self.players)
        try:
            if len(contingency) != len(players):
                raise KeyError("Number of strategies is not equal to the number of players")
        except TypeError:
            raise TypeError("contingency must be a tuple-like object") from None
        cont = [0 for _ in players]
        for (pl, st) in enumerate(contingency):
            player = players[pl]
            if isinstance(st, int):
                if st < 0 or st >= len(player.strategies):
                    raise IndexError(f"Provided strategy index {st} out of range for player {pl}")
                cont[pl] = st
            elif isinstance(st, str):
                try:
                    cont[pl] = [s.label for s in player.strategies].index(st)
                except ValueError:
                    raise IndexError(f"Provided strategy label '{st}' not defined")
            elif isinstance(st, Strategy):
                try:
                    cont[pl] = list(player.strategies).index(st)
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
        ...
        TypeError
            If `player` is not a `Player`, `NodePlayer`, or a `str`
        ValueError
            If `player` is an empty `str` or is a `NodePlayer` that resolves to no player
            (terminal node).
        """
        if isinstance(player, NodePlayer):
            resolved = cython.cast(NodePlayer, player)._resolve()
            if resolved is None:
                raise ValueError(
                    f"{funcname}(): {argname} resolves to no player "
                    f"(the node is terminal)"
                )
            player = resolved
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
            If `outcome` is not an `Outcome`, `NodeOutcome`, or a `str`
        ValueError
            If `outcome` is an empty `str` or all spaces, or is a `NodeOutcome` that
            resolves to no outcome (no outcome is attached to its node).
        """
        if isinstance(outcome, NodeOutcome):
            resolved = cython.cast(NodeOutcome, outcome)._resolve()
            if resolved is None:
                raise ValueError(
                    f"{funcname}(): {argname} resolves to no outcome "
                    f"(no outcome is attached to the node)"
                )
            outcome = resolved
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
                raise KeyError(f"{funcname}(): no outcome with label '{outcome}'")
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
            If `infoset` is not an `Infoset`, `NodeInfoset`, or a `str`
        ValueError
            If `infoset` is an empty `str` or all spaces, or is a `NodeInfoset` that
            resolves to no information set (its node is terminal).
        """
        if isinstance(infoset, NodeInfoset):
            resolved = cython.cast(NodeInfoset, infoset)._resolve()
            if resolved is None:
                raise ValueError(
                    f"{funcname}(): {argname} resolves to no information set "
                    f"(the node is terminal)"
                )
            infoset = resolved
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

    def _resolve_probs(self,
                       probs: typing.Sequence | typing.Mapping,
                       action_labels: list[str],
                       funcname: str) -> list:
        """Resolve a probability specification against an ordered list of action labels.

        `probs` may be a sequence (positional; must have exactly one entry per action)
        or a mapping from action labels to values (may be sparse; omitted labels are
        assigned zero).  Returns a dense list of values in action order.
        """
        if isinstance(probs, typing.Mapping):
            unknown = [k for k in probs if k not in action_labels]
            if unknown:
                raise KeyError(f"{funcname}(): no action with label '{unknown[0]}'")
            return [probs.get(label, 0) for label in action_labels]
        probs = list(probs)
        if len(probs) != len(action_labels):
            raise IndexError(f"{funcname}(): must specify exactly one probability per action")
        return probs

    def append_move(self, nodes: Node | NodeReferenceSet,
                    player: Player | str,
                    actions: list[str]) -> None:
        """Add a move for `player` at terminal `nodes`.  All elements of `nodes` become part of
        a new information set, with actions labeled according to `actions`.

        `player` must be a personal player; use `append_event` to add a chance move.

        Raises
        ------
        UndefinedOperationError
            If `nodes` are not all terminal, `actions` is empty, or `player` is the
            chance player.
        MismatchError
            If an element from `nodes` is a `Node` from a different game,
            or `player` is a `Player` from a different game.
        ValueError
            If `nodes` has duplicated elements, or is empty; or if `actions` contains
            an empty or a duplicated label.
        """
        resolved_player = cython.cast(Player, self._resolve_player(player, "append_move"))
        if resolved_player.is_chance:
            raise UndefinedOperationError(
                "append_move(): `player` must be a personal player; "
                "use append_event() to add a chance move"
            )
        if not actions:
            raise UndefinedOperationError("append_move(): `actions` must be a nonempty list")
        if any(not label for label in actions):
            raise ValueError("append_move(): action labels must not be empty")
        if len(set(actions)) != len(actions):
            raise ValueError("append_move(): action labels must be unique")
        resolved_nodes = self._resolve_nodes(nodes, "append_move", "nodes")
        if any(len(n.children) > 0 for n in resolved_nodes):
            raise UndefinedOperationError("append_move(): `nodes` must be terminal nodes")

        resolved_node = cython.cast(Node, resolved_nodes[0])
        c_actions = stdvector[string]()
        for label in actions:
            c_actions.push_back(label.encode("utf-8"))
        self.game.deref().AppendMove(resolved_node.node, resolved_player.player, c_actions)
        resolved_infoset = cython.cast(NodeInfoset, resolved_node.infoset)._resolve()
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

    def append_event(self, nodes: Node | NodeReferenceSet,
                     actions: list[str],
                     probs: typing.Sequence | typing.Mapping) -> None:
        """Add a chance move at terminal `nodes`, with distribution `probs`.  All elements
        of `nodes` become part of a new event, with actions labeled according to `actions`.

        .. versionadded:: 17.0.0

        Parameters
        ----------
        nodes : Node or NodeReferenceSet
            The nonempty set of terminal nodes at which to add the move.
        actions : list of str
            The labels of the actions of the new event.  Nonempty, with no empty or
            duplicated label.
        probs : sequence or mapping
            The probability distribution over `actions`.  A sequence must specify one
            probability per action, in the order given in `actions`.  A mapping from
            action labels to probabilities may be sparse; omitted actions are assigned
            probability zero.  Probabilities are non-negative and sum to exactly one.

        Raises
        ------
        UndefinedOperationError
            If `nodes` are not all terminal, or `actions` is empty.
        MismatchError
            If an element from `nodes` is a `Node` from a different game.
        KeyError
            If a key of `probs` matches no label in `actions`.
        IndexError
            If a sequence `probs` does not have exactly one entry per action.
        ValueError
            If `nodes` has duplicated elements, or is empty; if `actions` contains
            an empty or a duplicated label; or if `probs` are not non-negative numbers
            summing to exactly one.
        """
        if not actions:
            raise UndefinedOperationError("append_event(): `actions` must be a nonempty list")
        if any(not label for label in actions):
            raise ValueError("append_event(): action labels must not be empty")
        if len(set(actions)) != len(actions):
            raise ValueError("append_event(): action labels must be unique")
        resolved_nodes = self._resolve_nodes(nodes, "append_event", "nodes")
        if any(len(n.children) > 0 for n in resolved_nodes):
            raise UndefinedOperationError("append_event(): `nodes` must be terminal nodes")
        resolved_probs = self._resolve_probs(probs, actions, "append_event")

        resolved_node = cython.cast(Node, resolved_nodes[0])
        c_actions = stdvector[string]()
        for label in actions:
            c_actions.push_back(label.encode("utf-8"))
        c_probs = stdvector[c_Number]()
        for p in resolved_probs:
            c_probs.push_back(_to_number(p))
        self.game.deref().AppendEvent(resolved_node.node, c_actions, c_probs)
        resolved_infoset = cython.cast(NodeInfoset, resolved_node.infoset)._resolve()
        for n in resolved_nodes[1:]:
            self.game.deref().AppendMove(cython.cast(Node, n).node, resolved_infoset.infoset)

    def insert_move(self, node: Node | str,
                    player: Player | str, actions: list[str]) -> None:
        """Insert a move for `player` prior to the node `node`, with actions labeled
        according to `actions`.  `node` becomes the first child of the newly-inserted node.

        `player` must be a personal player; use `insert_event` to insert a chance move.

        Raises
        ------
        UndefinedOperationError
            If `actions` is empty, or `player` is the chance player.
        MismatchError
            If `node` is a `Node` from a different game, or `player` is a `Player` from a
            different game.
        ValueError
            If `actions` contains an empty or a duplicated label.
        """
        resolved_node = cython.cast(Node, self._resolve_node(node, "insert_move"))
        resolved_player = cython.cast(Player, self._resolve_player(player, "insert_move"))
        if resolved_player.is_chance:
            raise UndefinedOperationError(
                "insert_move(): `player` must be a personal player; "
                "use insert_event() to insert a chance move"
            )
        if not actions:
            raise UndefinedOperationError("insert_move(): `actions` must be a nonempty list")
        if any(not label for label in actions):
            raise ValueError("insert_move(): action labels must not be empty")
        if len(set(actions)) != len(actions):
            raise ValueError("insert_move(): action labels must be unique")
        c_actions = stdvector[string]()
        for label in actions:
            c_actions.push_back(label.encode("utf-8"))
        self.game.deref().InsertMove(resolved_node.node, resolved_player.player, c_actions)

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

    def insert_event(self, node: Node | str,
                     actions: list[str],
                     probs: typing.Sequence | typing.Mapping) -> None:
        """Insert a chance move prior to the node `node`, with actions labeled according
        to `actions` and distribution `probs`.  `node` becomes the first child of the
        newly-inserted node.

        .. versionadded:: 17.0.0

        Parameters
        ----------
        node : Node or str
            The node before which to insert the move.
        actions : list of str
            The labels of the actions of the new event.  Nonempty, with no empty or
            duplicated label.
        probs : sequence or mapping
            The probability distribution over `actions`.  A sequence must specify one
            probability per action, in the order given in `actions`.  A mapping from
            action labels to probabilities may be sparse; omitted actions are assigned
            probability zero.  Probabilities are non-negative and sum to exactly one.

        Raises
        ------
        UndefinedOperationError
            If `actions` is empty.
        MismatchError
            If `node` is a `Node` from a different game.
        KeyError
            If a key of `probs` matches no label in `actions`.
        IndexError
            If a sequence `probs` does not have exactly one entry per action.
        ValueError
            If `actions` contains an empty or a duplicated label, or if `probs` are not
            non-negative numbers summing to exactly one.
        """
        resolved_node = cython.cast(Node, self._resolve_node(node, "insert_event"))
        if not actions:
            raise UndefinedOperationError("insert_event(): `actions` must be a nonempty list")
        if any(not label for label in actions):
            raise ValueError("insert_event(): action labels must not be empty")
        if len(set(actions)) != len(actions):
            raise ValueError("insert_event(): action labels must be unique")
        resolved_probs = self._resolve_probs(probs, actions, "insert_event")
        c_actions = stdvector[string]()
        for label in actions:
            c_actions.push_back(label.encode("utf-8"))
        c_probs = stdvector[c_Number]()
        for p in resolved_probs:
            c_probs.push_back(_to_number(p))
        self.game.deref().InsertEvent(resolved_node.node, c_actions, c_probs)

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
        """Add an action at the information set `infoset`, with an automatically generated
        numeric label unique among the actions at `infoset`.  If `before` is not null, the
        new action is inserted before `before`.

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
            c_action = self.game.deref().InsertAction(resolved_infoset.infoset,
                                                      cython.cast(c_GameAction, NULL))
        else:
            resolved_action = cython.cast(
                Action, self._resolve_action(before, "add_action", "before")
            )
            if resolved_infoset != resolved_action.infoset:
                raise MismatchError("add_action(): must specify an action from the same infoset")
            c_action = self.game.deref().InsertAction(resolved_infoset.infoset,
                                                      resolved_action.action)

        current = {action.label for action in resolved_infoset.actions}
        number = c_action.deref().GetNumber()
        while str(number) in current:
            number += 1
        c_labels = stdmap[string, string]()
        c_labels[c_action.deref().GetLabel()] = str(number).encode("utf-8")
        self.game.deref().RelabelActions(resolved_infoset.infoset, c_labels)

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

    def make_event(self,
                   nodes: Node | NodeReferenceSet,
                   probs: typing.Sequence | typing.Mapping,
                   label: str | None = None) -> None:
        """Form `nodes` into a single event with distribution `probs`.

        `nodes` must all be nonterminal nodes of this game with the same actions, with the same
        labels in the same order.  They need not be chance nodes; personal nodes are
        converted, and the move is thereafter resolved by chance.  Nodes are removed from
        whatever information sets or events they currently belong to; any of those which
        retain members survive, keeping their labels, and those left with no members are deleted.
        Any ``Infoset`` object, and any of its ``Action`` objects, referring to a deleted one
        becomes invalid, and subsequent use raises ``RuntimeError``.
        The resulting event is accessible as ``node.infoset`` for any node in `nodes`.

        The first node in `nodes` determines the action order of the event,
        and is the frame against which mapping keys in `probs` are resolved.

        .. versionadded:: 17.0.0

        Parameters
        ----------
        nodes : Node or NodeReferenceSet
            The nonempty set of nonterminal nodes to place in the event.
        probs : sequence or mapping
            The probability distribution over the actions of the event.  A sequence must specify
            one probability per action, in action order.  A mapping from action labels
            to probabilities may be sparse; omitted actions are assigned probability zero.
            Probabilities are non-negative and sum to exactly one.
        label : str, optional
            The label of the new event.  If specified, must be unique among the events
            of the game after the operation.  A label currently held by another event
            may be reused only if all members of that event are among `nodes`.

        Raises
        ------
        MismatchError
            If any of `nodes` is from a different game.
        KeyError
            If a node reference matches no node, or a key of `probs` matches no
            action label of the event.
        IndexError
            If a sequence `probs` does not have exactly one entry per action.
        UndefinedOperationError
            If any of `nodes` is a terminal node, or the game is not a tree.
        ValueError
            If `nodes` is empty or contains a repeated node; if the nodes do not
            all have the same actions in the same order; if `probs` are not
            non-negative numbers summing to exactly one; or if `label` is not
            unique among the game's events after the operation.
        """
        if not self.is_tree:
            raise UndefinedOperationError(
                "make_event(): operation only defined for games with a tree representation"
            )
        resolved_nodes = self._resolve_nodes(nodes, "make_event")
        if any(n.is_terminal for n in resolved_nodes):
            raise UndefinedOperationError(
                "make_event(): all nodes must be nonterminal"
            )
        resolved_node = cython.cast(Node, resolved_nodes[0])
        action_labels = [a.label for a in resolved_node.infoset.actions]
        if any([a.label for a in n.infoset.actions] != action_labels
               for n in resolved_nodes[1:]):
            raise ValueError(
                "make_event(): all nodes must have the same actions, "
                "with the same labels in the same order"
            )
        resolved_probs = self._resolve_probs(probs, action_labels, "make_event")
        c_nodes = stdvector[c_GameNode]()
        for n in resolved_nodes:
            c_nodes.push_back(cython.cast(Node, n).node)
        c_probs = stdvector[c_Number]()
        for p in resolved_probs:
            c_probs.push_back(_to_number(p))
        self.game.deref().MakeEvent(c_nodes, c_probs, (label or "").encode("utf-8"))

    def relabel_actions(self,
                        infoset: Infoset | str,
                        labels: typing.Mapping[str, str],
                        strict: bool = True) -> None:
        """Simultaneously reassign the labels of actions at `infoset`.

        `labels` maps current action labels to their replacements.  The reassignment
        is simultaneous, so labels can be swapped directly, e.g. ``{"a": "b", "b": "a"}``.
        Actions are not re-ordered: each relabelled action keeps its position and, at an event,
        its probability.  After the operation, the labels must be nonempty and unique.

        .. versionadded:: 17.0.0

        Parameters
        ----------
        infoset : Infoset or str
            The information set at which to relabel actions.  If a string is passed,
            the information set is determined by finding the personal-player
            information set with that label, if any.
        labels : Mapping[str, str]
            A mapping from current action labels to replacement labels.  Entries
            whose key equals their value are ignored.
        strict : bool, default True
            If `True`, every key of `labels` must be the label of an action at
            `infoset`, and unknown keys raise ``KeyError``.  If `False`, unknown
            keys are ignored.

        Raises
        ------
        MismatchError
            If `infoset` is an `Infoset` from a different game.
        KeyError
            If `infoset` is a string matching no information set; or, when `strict`
            is `True`, if a key of `labels` matches no action at `infoset`.
        TypeError
            If `labels` is not a mapping, or any key or value is not a string.
        ValueError
            If a key of `labels` matches more than one action at `infoset` (possible
            in games read from files predating unique-label enforcement); or if any
            replacement label is empty, is not a valid label, or would result in a
            duplicate label at the information set.
        """
        resolved_infoset = cython.cast(Infoset, self._resolve_infoset(infoset, "relabel_actions"))
        if not hasattr(labels, "items"):
            raise TypeError(
                f"relabel_actions(): labels must be a mapping, "
                f"not {labels.__class__.__name__}"
            )
        current = [action.label for action in resolved_infoset.actions]
        c_labels = stdmap[string, string]()
        for old, new in labels.items():
            if not isinstance(old, str) or not isinstance(new, str):
                raise TypeError("relabel_actions(): labels must map str to str")
            matches = current.count(old)
            if matches > 1:
                raise ValueError(
                    f"relabel_actions(): label '{old}' is ambiguous at this information set"
                )
            if matches == 0:
                if strict:
                    raise KeyError(f"relabel_actions(): no action with label '{old}'")
                continue
            if new == old:
                continue
            c_labels[old.encode("utf-8")] = new.encode("utf-8")
        if c_labels.empty():
            return
        self.game.deref().RelabelActions(resolved_infoset.infoset, c_labels)

    def make_infoset(self,
                     nodes: Node | NodeReferenceSet,
                     player: str,
                     label: str | None = None) -> None:
        """Form `nodes` into a single information set belonging to `player`.

        The nodes must all: (i) be nonterminal nodes of this game, (ii) have the same
        actions, with the same labels in the same order.  They need not currently be
        personal decision nodes; nodes belonging to a chance event are converted, discarding
        their probabilities.  Nodes are removed from whatever information sets or events
        they currently belong to; any of those which retain members after removal survive,
        keeping their labels.  Infosets left with no members are deleted.

        The structure of the tree is unchanged: no nodes are created or removed.
        This operation may introduce imperfect recall or absent-mindedness.

        .. versionadded:: 17.0.0

        Parameters
        ----------
        nodes : Node or NodeReferenceSet
            The nodes to place in the information set.  Nonempty; each
            node may be referenced only once.
        player : str
            The label of the player to whom the information set belongs.
        label : str, optional
            The label of the new information set.  If specified, must be unique
            among the information sets of `player` after the operation.  A label
            currently held by another of `player`'s information sets may be reused
            only if all members of that set are among `nodes`.

        Raises
        ------
        MismatchError
            If any of `nodes` is from a different game.
        KeyError
            If any of `nodes`, or `player`, is a label matching no such object in the game.
        TypeError
            If any of `nodes`, or `player`, is not of an accepted type.
        UndefinedOperationError
            If any of `nodes` is a terminal node, or if the game is not a tree.
        ValueError
            If `nodes` is empty or contains a repeated node; if the nodes do not all
            have the same actions in the same order; or if `label` is not unique among
            `player`'s information sets after the operation.
        """
        if not self.is_tree:
            raise UndefinedOperationError(
                "make_infoset(): operation only defined for games with a tree representation"
            )
        resolved_nodes = self._resolve_nodes(nodes, "make_infoset")
        resolved_player = cython.cast(Player, self._resolve_player(player, "make_infoset"))
        for n in resolved_nodes:
            if n.is_terminal:
                raise UndefinedOperationError(
                    "make_infoset(): all nodes must be decision nodes"
                )
        c_nodes = stdvector[c_GameNode]()
        for n in resolved_nodes:
            c_nodes.push_back(cython.cast(Node, n).node)
        self.game.deref().MakeInfoset(c_nodes, resolved_player.player,
                                      (label or "").encode())

    def reveal(self,
               infoset: Infoset | str,
               player: Player | str) -> None:
        """Reveals the move made at `infoset` to `player`.

        Revealing the move modifies all subsequent information sets for `player` such
        that any two nodes which are successors of two different actions at this
        information set are placed in different information sets for `player`.

        Revelation is a one-shot operation; it is not enforced with respect to any
        revisions made to the game tree subsequently.

        .. versionchanged:: 17.0.0
            Revealing the move at an absent-minded information set is not permitted.

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
        UndefinedOperationError
            If `infoset` is absent-minded, or if `player` is the chance player.
        """
        resolved_infoset = cython.cast(Infoset, self._resolve_infoset(infoset, "reveal"))
        resolved_player = cython.cast(Player, self._resolve_player(player, "reveal"))
        if resolved_player.is_chance:
            raise UndefinedOperationError(
                "reveal(): `player` must be a personal player"
            )
        if resolved_infoset.is_absent_minded:
            raise UndefinedOperationError(
                "reveal(): revealing the move at an absent-minded information set "
                "is not well-defined"
            )
        self.game.deref().Reveal(resolved_infoset.infoset, resolved_player.player)

    def add_player(self, label: str) -> Player:
        """Add a new player to the game.

        .. versionchanged:: 16.7.0
            A label is now required and must be nonempty and unique among the game's players.
            In extensive games, the label cannot be ``"Chance"``, which is reserved for the
            chance player.

        .. versionchanged:: 17.0.0
            In a game with a strategic representation, the new player's sole strategy is
            labeled ``"1"``.

        Parameters
        ----------
        label : str
            The label for the new player.  Must be nonempty and not the same as the label
            of an existing player in the game.

        Returns
        -------
        Player
            A reference to the newly-created player.

        Raises
        ------
        ValueError
            If `label` is empty, is already the label of another player, or (in an
            extensive game) is ``"Chance"``, the reserved label of the chance player.
        """
        return Player.wrap(self.game.deref().NewPlayer(label.encode("utf-8")))

    def add_outcome(self,
                    label: str,
                    payoffs: list | None = None) -> Outcome:
        """Add a new outcome to the game.

        .. versionchanged:: 16.7.0
            A label is now required and must be nonempty and unique among the
            game's outcomes.

        Parameters
        ----------
        label : str
            The label for the outcome.  Must be nonempty and not already in use
            by another outcome in the game.
        payoffs : list, optional
            The payoffs of the outcome to each player.

        Raises
        ------
        ValueError
            If `payoffs` is specified but is not the same length as the number of players
            in the game, or if `label` is empty or already in use by another outcome.

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
        c = Outcome.wrap(self.game.deref().NewOutcome(label.encode("utf-8")))
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

    def relabel_strategies(self,
                           player: Player | str,
                           labels: typing.Mapping[str, str],
                           strict: bool = True) -> None:
        """Simultaneously reassign the labels of `player`'s strategies.

        `labels` maps current strategy labels to their replacements.  The reassignment
        is simultaneous, so labels can be swapped directly, e.g. ``{"1": "2", "2": "1"}``.
        Strategies are not re-ordered: each relabelled strategy keeps its position.
        After the operation, the player's strategy labels must be nonempty and unique.

        .. versionadded:: 17.0.0

        Parameters
        ----------
        player : Player or str
            The player whose strategies to relabel.  If a string is passed, the player
            is determined by finding the player with that label, if any.
        labels : Mapping[str, str]
            A mapping from current strategy labels to replacement labels.  Entries
            whose key equals their value are ignored.
        strict : bool, default True
            If `True`, every key of `labels` must be the label of a strategy of
            `player`, and unknown keys raise ``KeyError``.  If `False`, unknown keys
            are ignored.

        Raises
        ------
        MismatchError
            If `player` is a `Player` from a different game.
        KeyError
            If `player` is a string matching no player; or, when `strict` is `True`,
            if a key of `labels` matches no strategy of `player`.
        TypeError
            If `labels` is not a mapping, or any key or value is not a string.
        UndefinedOperationError
            If the game has a tree representation, where strategies are derived from
            the tree.
        ValueError
            If a key of `labels` matches more than one strategy of `player`; or if any
            replacement label is empty, is not a valid label, or would result in a
            duplicate label for the player.

        See Also
        --------
        relabel_actions : Change the labels of actions at an information set.
        """
        if self.is_tree:
            raise UndefinedOperationError(
                "Relabelling strategies is only applicable to games in strategic form"
            )
        resolved_player = cython.cast(Player, self._resolve_player(player, "relabel_strategies"))
        if not hasattr(labels, "items"):
            raise TypeError(
                f"relabel_strategies(): labels must be a mapping, "
                f"not {labels.__class__.__name__}"
            )
        current = [strategy.label for strategy in resolved_player.strategies]
        c_labels = stdmap[string, string]()
        for old, new in labels.items():
            if not isinstance(old, str) or not isinstance(new, str):
                raise TypeError("relabel_strategies(): labels must map str to str")
            matches = current.count(old)
            if matches > 1:
                raise ValueError(
                    f"relabel_strategies(): label '{old}' is ambiguous for this player"
                )
            if matches == 0:
                if strict:
                    raise KeyError(f"relabel_strategies(): no strategy with label '{old}'")
                continue
            if new == old:
                continue
            c_labels[old.encode("utf-8")] = new.encode("utf-8")
        if c_labels.empty():
            return
        self.game.deref().RelabelStrategies(resolved_player.player, c_labels)

    def set_strategies(self,
                       player: Player | str,
                       strategies: list[str],
                       drop: bool = False,
                       add: bool = True) -> None:
        """Set the strategies of `player` to be `strategies`, matching by label.

        - An entry of `strategies` matching the label of a current strategy refers to
        that strategy, keeping the outcomes at its contingencies;
        - An entry matching no current strategy creates a new strategy there,
        with no outcome at any of its contingencies;
        - A current strategy whose label is not in `strategies` is deleted,
        along with the outcomes at its contingencies.
        - Listing the current labels in a new order reorders the strategies,
        permuting the payoff table to match.

        The defaults permit creation and forbid deletion.

        .. versionadded:: 17.0.0
            Subsumes and replaces `Game.add_strategy` and `Game.delete_strategy`.

        Parameters
        ----------
        player : Player or str
            The player whose strategies to set.
        strategies : list of str
            The labels of the strategies the player is to have, in order.  Must be
            nonempty and without duplicates; each label must be a valid, nonempty label.
        drop : bool, default False
            Deleting strategies is destructive, so it must be explicitly confirmed:
            if any current strategy is missing from `strategies` and `drop` is
            `False`, the operation raises without modifying the game.
        add : bool, default True
            If `False`, entries of `strategies` matching no current strategy raise.

        Raises
        ------
        MismatchError
            If `player` is a `Player` from a different game.
        KeyError
            If `player` is a string matching no player.
        TypeError
            If `strategies` is a string, or not an iterable of strings.
        UndefinedOperationError
            If the game has a tree representation, where the strategies are derived
            from the tree; or if `strategies` is empty.
        ValueError
            If a label in `strategies` is repeated, empty, or invalid.

        See Also
        --------
        relabel_strategies : Change the labels of strategies, keeping the table unchanged.
        set_actions : The analogous operation on the actions of an information set.
        """
        if self.is_tree:
            raise UndefinedOperationError(
                "Setting strategies is only applicable to games in strategic form"
            )
        resolved_player = cython.cast(Player, self._resolve_player(player, "set_strategies"))
        if isinstance(strategies, str) or not hasattr(strategies, "__iter__"):
            raise TypeError("set_strategies(): strategies must be an iterable of str")
        labels = list(strategies)
        for label in labels:
            if not isinstance(label, str):
                raise TypeError("set_strategies(): strategies must be an iterable of str")
        if not labels:
            raise UndefinedOperationError("set_strategies(): `strategies` must be a nonempty list")
        current = [strategy.label for strategy in resolved_player.strategies]
        if len(set(current)) != len(current):
            raise ValueError(
                "set_strategies(): the player has duplicate strategy labels, "
                "so matching by label is not well-defined"
            )
        added = [label for label in labels if label not in current]
        if added and not add:
            raise ValueError(f"set_strategies(): would create new strategies {added}")
        missing = [label for label in current if label not in labels]
        if missing and not drop:
            raise ValueError(
                f"set_strategies(): would delete strategies {missing} and the outcomes "
                f"at their contingencies; pass drop=True to confirm"
            )
        c_labels = stdvector[string]()
        for label in labels:
            c_labels.push_back(label.encode("utf-8"))
        self.game.deref().SetStrategies(resolved_player.player, c_labels)


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
