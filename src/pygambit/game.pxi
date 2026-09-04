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

import pygambit.gameiter

ctypedef string (*GameWriter)(const c_Game &) except +IOError


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
        c_labels = stdvector[string]()
        for player in (players or []):
            c_labels.push_back(str(player).encode("utf-8"))
        g = Game.wrap(NewTree(c_labels))
        g.title = title
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
        g = Game.wrap(NewTable(list(dim), True))
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
        g = Game.wrap(NewTable(list(shape), False))
        players = list(g.players)
        for profile in itertools.product(*(range(s) for s in shape)):
            contingency = {p: str(i + 1) for p, i in zip(players, profile, strict=True)}
            outcome = g.get_outcome(contingency)
            for array, player in zip(arrays, players, strict=True):
                outcome[player] = array[profile]
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

        players = list(self.players)
        player_strategies = {player: self.get_strategies(player) for player in players}
        shape = tuple(len(player_strategies[player]) for player in players)
        for player in players:
            array = np.zeros(shape=shape, dtype=object)
            for profile in itertools.product(*(range(s) for s in shape)):
                contingency = {
                    p: player_strategies[p][i]
                    for p, i in zip(players, profile, strict=True)
                }
                payoffs = self.get_payoffs(contingency)
                try:
                    array[profile] = dtype(payoffs[player])
                except (ValueError, TypeError, IndexError, KeyError):
                    raise ValueError(
                        f"Payoff '{payoffs[player]}' cannot be "
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
        g = Game.wrap(NewTable(list(shape), False))
        g.relabel_players(
            {player: label for player, label in zip(g.players, payoffs, strict=True)}
        )
        players = list(g.players)
        for profile in itertools.product(*(range(s) for s in shape)):
            contingency = {p: str(i + 1) for p, i in zip(players, profile, strict=True)}
            outcome = g.get_outcome(contingency)
            for array, player in zip(arrays, players, strict=True):
                outcome[player] = array[profile]
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

    def get_infosets(self, player: str) -> list[Node]:
        """Returns a snapshot of the information sets belonging to the personal
        player `player`: the decisions at which that player chooses an action.

        One representative member node is returned per information set, in the order
        the information sets are encountered in the pre-order depth first traversal of
        the game tree. This is a materialized snapshot, not a live view: it reflects
        the game's state at the moment of the call, and does not change if the game is
        subsequently mutated.

        Parameters
        ----------
        player : str
            The label of the personal player whose information sets to return.

        Returns
        -------
        list of Node
            One representative member node per information set belonging to `player`.

        .. versionadded:: 17.0.0

        Raises
        ------
        UndefinedOperationError
            If the game does not have a tree representation.
        KeyError
            If no player in the game has label `player`; the chance player has no
            label reachable this way -- use `get_events` for its events.
        ValueError
            If `player` is an empty string or all whitespace.
        """
        if not self.is_tree:
            raise UndefinedOperationError(
                "Operation only defined for games with a tree representation"
            )
        resolved_player = self._resolve_player(player, "get_infosets")
        return [
            Node.wrap(infoset.deref().GetMember(1))
            for infoset in resolved_player.deref().GetInfosets()
        ]

    def get_events(self) -> list[Node]:
        """Returns a snapshot of the chance player's events: the points of exogenous
        randomness, each with a probability distribution over its actions.

        One representative member node is returned per event, in the order the events
        are encountered in the pre-order depth first traversal of the game tree. This
        is a materialized snapshot, not a live view: it reflects the game's state at
        the moment of the call, and does not change if the game is subsequently
        mutated.

        Returns
        -------
        list of Node
            One representative member node per event.

        .. versionadded:: 17.0.0

        Raises
        ------
        UndefinedOperationError
            If the game does not have a tree representation.
        """
        if not self.is_tree:
            raise UndefinedOperationError(
                "Operation only defined for games with a tree representation"
            )
        return [
            Node.wrap(event.deref().GetMember(1))
            for event in self.game.deref().GetChance().deref().GetInfosets()
        ]

    def get_strategies(self, player: str) -> list[str]:
        """Returns a snapshot of the labels of the strategies belonging to `player`.

        This is a materialized snapshot, not a live view: it reflects the game's
        state at the moment of the call, and does not change if the game is
        subsequently mutated.

        Parameters
        ----------
        player : str
            The label of the player whose strategies to return.

        Returns
        -------
        list of str
            The labels of `player`'s strategies, in order.

        .. versionadded:: 17.0.0

        Raises
        ------
        KeyError
            If no player in the game has label `player`.
        ValueError
            If `player` is an empty string or all whitespace.
        """
        resolved_player = self._resolve_player(player, "get_strategies")
        return [
            s.deref().GetLabel().decode("utf-8") for s in resolved_player.deref().GetStrategies()
        ]

    def get_sequences(self, player: str) -> list[Sequence]:
        """Returns a snapshot of the sequences belonging to `player`.

        This is a materialized snapshot, not a live view: it reflects the game's
        state at the moment of the call, and does not change if the game is
        subsequently mutated.

        Parameters
        ----------
        player : str
            The label of the player whose sequences to return.

        Returns
        -------
        list of Sequence
            `player`'s sequences.

        .. versionadded:: 17.0.0

        Raises
        ------
        KeyError
            If no player in the game has label `player`.
        ValueError
            If `player` is an empty string or all whitespace.
        """
        resolved_player = self._resolve_player(player, "get_sequences")
        return [Sequence.wrap(s) for s in resolved_player.deref().GetSequences()]

    def get_min_payoff(self, player: str) -> Rational:
        """Returns the smallest payoff for `player` in any play of the game.

        .. versionadded:: 17.0.0

        Parameters
        ----------
        player : str
            The label of the player.

        Raises
        ------
        KeyError
            If no player in the game has label `player`.
        ValueError
            If `player` is an empty string or all whitespace.

        See Also
        --------
        Game.get_max_payoff
        Game.min_payoff
        """
        resolved_player = self._resolve_player(player, "get_min_payoff")
        return rat_to_py(self.game.deref().GetPlayerMinPayoff(resolved_player))

    def get_max_payoff(self, player: str) -> Rational:
        """Returns the largest payoff for `player` in any play of the game.

        .. versionadded:: 17.0.0

        Parameters
        ----------
        player : str
            The label of the player.

        Raises
        ------
        KeyError
            If no player in the game has label `player`.
        ValueError
            If `player` is an empty string or all whitespace.

        See Also
        --------
        Game.get_min_payoff
        Game.max_payoff
        """
        resolved_player = self._resolve_player(player, "get_max_payoff")
        return rat_to_py(self.game.deref().GetPlayerMaxPayoff(resolved_player))

    @property
    def players(self) -> GamePlayers:
        """The set of players in the game."""
        return GamePlayers.wrap(self.game)

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

    def _get_nodes(self, selector: Selector) -> list[Node]:
        """Evaluate `selector` (an `H`-built expression) against this game.

        Internal: the `H` selector algebra's evaluator, interpreting the
        selector's ops in order, starting from the root, reusing `Node`'s
        existing navigation (`.children`, `.plays`) rather than walking the
        C++ tree directly. Not part of the public API yet -- used to resolve
        a `Selector`/`GroupedSelector` argument to `append_move`,
        `append_event`, `append_infoset`, and `make_outcome`.
        """
        current: list = None
        for op in selector._ops:
            if isinstance(op, _AfterStep):
                candidates = list(self.nodes) if current is None else current
                current = [n for n in candidates if _matches_suffix(n, op.labels)]
                continue
            if current is None:
                current = [self.root]
            if isinstance(op, _PathStep):
                for step in op.steps:
                    current = (
                        [child for node in current for child in node.children]
                        if step is Ellipsis
                        else [node.children[step] for node in current]
                    )
            elif isinstance(op, _PlaysStep):
                current = [play for node in current for play in node.plays]
            elif isinstance(op, _FilterStep):
                current = [
                    node for node in current
                    if op.predicate(HistoryView._wrap(node, _history_of(node)))
                ]
            else:
                raise TypeError(f"_get_nodes(): unknown selector op {op!r}")
        if current is None:
            current = [self.root]
        return current

    def _get_histories(self, selector: Selector) -> list[tuple]:
        """Evaluate `selector` (an `H`-built expression) against this game,
        materializing each result as a `History` -- a plain tuple of action
        labels from the root, carrying no reference to this game.

        Internal: the History-materializing counterpart to `_get_nodes`, kept
        for use by `_get_groups` and tests. Not part of the public API yet.
        """
        return [_history_of(node) for node in self._get_nodes(selector)]

    def _group_nodes(self, grouped: GroupedSelector) -> dict:
        """Internal: like `_get_groups`, but keeps `Node` objects rather than
        materializing each into a `History` -- used by mutation methods that
        need to resolve straight back to concrete nodes, avoiding a
        Node -> History -> Node round trip.

        Applies `grouped`'s initial partition (`base`/`key`), then its
        `post_ops` in order, each one per-group -- expanding/filtering each
        group's own members independently, leaving the key untouched, except
        that a `.plays` step refines the key by `recall_player`'s last action
        at that point, if `with_recall` set one (see `GroupedSelector`'s
        docstring for why).
        """
        result: dict = {}
        for node in self._get_nodes(grouped.base):
            view: HistoryView = HistoryView._wrap(node, _history_of(node))
            key = grouped.key(view)
            result.setdefault(key, []).append(node)
        for op in grouped.post_ops:
            next_result: dict = {}
            for key, nodes in result.items():
                if isinstance(op, _PlaysStep):
                    expanded = [play for node in nodes for play in node.plays]
                    if grouped.recall_player is None:
                        next_result[key] = expanded
                    else:
                        for play in expanded:
                            refined_key = (key, _last_action(play, grouped.recall_player))
                            next_result.setdefault(refined_key, []).append(play)
                    continue
                if isinstance(op, _AfterStep):
                    next_result[key] = [n for n in nodes if _matches_suffix(n, op.labels)]
                    continue
                raise TypeError(f"_group_nodes(): unknown post-op {op!r}")
            result = next_result
        return result

    def _get_groups(self, grouped: GroupedSelector) -> dict:
        """Evaluate a `.by(callable)`-built `GroupedSelector` against this
        game, returning a dict from each distinct key to the list of
        Histories that produced it.

        Internal: the History-materializing counterpart to `_group_nodes`,
        kept for use by tests. Not part of the public API yet.
        """
        return {
            key: [_history_of(node) for node in nodes]
            for key, nodes in self._group_nodes(grouped).items()
        }

    @property
    def is_const_sum(self) -> bool:
        """Whether the game is constant sum."""
        return self.game.deref().IsConstSum()

    @property
    def is_perfect_recall(self) -> bool:
        """Whether the game is perfect recall.

        By convention, games with a strategic representation have perfect recall as they
        are treated as simultaneous-move games.

        See Also
        --------
        Game.has_perfect_recall
        """
        return self.game.deref().IsPerfectRecall()

    def has_perfect_recall(self, player: str) -> bool:
        """Returns whether `player` has perfect recall.

        A player has perfect recall if, at each of the player's information sets, every
        member node is reached by the same sequence of the player's own prior actions;
        that is, the player never forgets an action they took previously, nor information
        they previously knew.  A game has perfect recall if and only if every player does.

        By convention, in games with a strategic representation every player has perfect
        recall as such games are treated as simultaneous-move games.

        .. versionadded:: 17.0.0

        Parameters
        ----------
        player : str
            The label of the player.

        Raises
        ------
        KeyError
            If no player in the game has label `player`.
        ValueError
            If `player` is an empty string or all whitespace.

        See Also
        --------
        Game.is_perfect_recall
        """
        resolved_player = self._resolve_player(player, "has_perfect_recall")
        return self.game.deref().HasPerfectRecall(resolved_player)

    @property
    def min_payoff(self) -> decimal.Decimal | Rational:
        """The minimum payoff to any player in any play of the game.

        .. versionchanged:: 16.5.0
           Changed from reporting minimum payoff in any (non-null) outcome to the minimum
           payoff in any play of the game.

        See Also
        --------
        Game.max_payoff
        Game.get_min_payoff
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
        Game.get_max_payoff
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

    def get_minimal_subgame(self, node: Selector) -> Subgame:
        """Returns the smallest subgame containing the information set or event that
        the node identified by `node` belongs to.

        `node` is a `Selector` (an `H`-built expression, evaluated against this
        game) that must resolve to exactly one node.

        .. versionadded:: 16.7.0
        .. versionchanged:: 17.0.0
            Renamed from `minimal_subgame`.  `node` (formerly `infoset`) is now a
            `Selector`; a `Node` or `str` is no longer accepted directly -- build
            one with `H`.

        Parameters
        ----------
        node : Selector
            A `Selector` resolving to a single node belonging to the information
            set or event to query.

        Returns
        -------
        Subgame
            The smallest subgame containing the information set or event that
            `node` belongs to.

        Raises
        ------
        TypeError
            If `node` is not a `Selector`.
        UndefinedOperationError
            If the game does not have a tree representation.
        ValueError
            If `node` does not resolve to exactly one node, or belongs to no
            information set or event (it is terminal).
        """
        if not self.is_tree:
            raise UndefinedOperationError(
                "get_minimal_subgame(): operation only defined for games "
                "with a tree representation"
            )
        if not isinstance(node, Selector):
            raise TypeError(
                f"get_minimal_subgame(): node must be a Selector, not {node.__class__.__name__}"
            )
        resolved_node = self._resolve_infoset_or_event(node, "get_minimal_subgame")
        return Subgame.wrap(
            self.game.deref().GetMinimalSubgame(
                cython.cast(Node, resolved_node)._infoset_handle()
            )
        )

    def get_behavior(self,
                     player: str,
                     strategy: str) -> StrategyBehavior:
        """Return the mapping from information sets to actions prescribed by a strategy.

        .. versionadded:: 17.0.0

        Parameters
        ----------
        player : str
            The label of the player whose strategy to view.
        strategy : str
            The label of the strategy to view.

        Returns
        -------
        StrategyBehavior

        Raises
        ------
        UndefinedOperationError
            If the game does not have a tree representation.
        KeyError
            If no player has the label `player`, or `player` has no strategy with
            the label `strategy`.
        """
        if not self.is_tree:
            raise UndefinedOperationError(
                "get_behavior(): only defined for games with a tree representation"
            )
        self._resolve_strategy(player, strategy, "get_behavior")  # validate eagerly
        return StrategyBehavior.wrap(self, player, strategy)

    def _resolve_contingency(self, contingency: typing.Any, funcname: str,
                             argname: str = "contingency") -> dict:
        """Resolve a pure-strategy contingency to a dict from player label to strategy
        label.

        `contingency` must be a complete mapping from the game's players' labels to the
        label of the strategy played by that player.  Each strategy label is validated
        (but not resolved to a handle) eagerly, so the whole mapping is checked before
        any use is made of it.
        """
        if not hasattr(contingency, "items"):
            raise TypeError(f"{funcname}(): {argname} must be a mapping")
        resolved = {}
        for player_label, strategy_label in contingency.items():
            if not isinstance(player_label, str):
                raise TypeError(
                    f"{funcname}(): {argname} keys must be player labels (str), "
                    f"not {player_label.__class__.__name__}"
                )
            if player_label in resolved:
                raise ValueError(f"{funcname}(): each player may appear only once in {argname}")
            self._resolve_strategy(player_label, strategy_label, funcname, argname)
            resolved[player_label] = strategy_label
        if set(resolved) != set(self.players):
            raise ValueError(
                f"{funcname}(): {argname} must specify exactly one strategy "
                f"for each player of the game"
            )
        return resolved

    @cython.cfunc
    def _make_pure_strategy_profile(self, resolved: dict) -> shared_ptr[c_PureStrategyProfile]:
        """Build a C++ pure-strategy profile from a dict mapping player label to
        strategy label."""
        psp: shared_ptr[c_PureStrategyProfile] = make_shared[c_PureStrategyProfile](
            self.game.deref().NewPureStrategyProfile()
        )
        for player_label in self.players:
            handle = self._resolve_strategy(
                player_label, resolved[player_label], "_make_pure_strategy_profile"
            )
            deref(deref(psp).deref()).SetStrategy(handle)
        return psp

    def get_outcome(self, contingency: typing.Mapping) -> Outcome:
        """Returns the `Outcome` attached to a pure-strategy contingency.

        Only defined for games in strategic (table) representation; for extensive-form
        and action-graph games, a pure-strategy contingency has no single stored outcome
        to return (see `get_payoffs`).

        .. versionadded:: 17.0.0

        Parameters
        ----------
        contingency : Mapping
            A complete mapping from the game's players' labels to the label of the
            strategy played by that player.

        Returns
        -------
        Outcome
            The outcome attached to `contingency` (possibly the null outcome).

        Raises
        ------
        UndefinedOperationError
            If the game is not in strategic (table) representation.
        ValueError
            If `contingency` does not specify exactly one strategy for each player
            of the game, or a key is an empty or all-whitespace string.
        KeyError
            If a player label, or a player's strategy label, does not match any
            player, or that player's strategies, in the game.
        TypeError
            If `contingency` is not a mapping, or a key or value is not a `str`.
        """
        if self.is_tree or self.game.deref().IsAgg():
            raise UndefinedOperationError(
                "get_outcome(): operation not defined for games not in "
                "strategic (table) representation"
            )
        resolved = self._resolve_contingency(contingency, "get_outcome")
        psp = self._make_pure_strategy_profile(resolved)
        return Outcome.wrap(deref(deref(psp).deref()).GetOutcome())

    def get_payoffs(self, contingency: typing.Mapping) -> PayoffVector:
        """Returns the payoff to each player at a pure-strategy contingency.

        Works for any game representation.  For extensive-form and action-graph games
        the payoffs are computed, not read from a stored outcome, and are always
        returned as ``Rational`` regardless of the game's own numerical representation.

        .. versionadded:: 17.0.0

        Parameters
        ----------
        contingency : Mapping
            A complete mapping from the game's players' labels to the label of the
            strategy played by that player.

        Returns
        -------
        PayoffVector
            The payoff to each player, keyed by player label.

        Raises
        ------
        ValueError
            If `contingency` does not specify exactly one strategy for each player
            of the game, or a key is an empty or all-whitespace string.
        KeyError
            If a player label, or a player's strategy label, does not match any
            player, or that player's strategies, in the game.
        TypeError
            If `contingency` is not a mapping, or a key or value is not a `str`.
        """
        resolved = self._resolve_contingency(contingency, "get_payoffs")
        psp = self._make_pure_strategy_profile(resolved)
        values = {}
        for player in self.players:
            values[player] = rat_to_py(
                deref(deref(psp).deref()).GetPayoff(self._resolve_player(player, "get_payoffs"))
            )
        return PayoffVector(values)

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
            strategies = self.get_strategies(p)
            if len(strategies) != len(d):
                raise ValueError(
                    f"Number of elements does not match number of strategies for {p}"
                )
            profile[p] = {
                s: typefunc(v) for s, v in zip(strategies, d, strict=True)
            }
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
                profile[player] = _dirichlet_distribution(self.get_strategies(player), gen)
            return profile
        elif denom < 1:
            raise ValueError("random_strategy_profile(): denom must be positive")
        else:
            profile = self.mixed_strategy_profile(rational=True)
            for player in self.players:
                profile[player] = _grid_distribution(self.get_strategies(player), denom, gen)
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
            p_infosets = self.get_infosets(p)
            if len(p_infosets) != len(d):
                raise ValueError(f"Number of elements does not match number of infosets for {p}")
            for (node, v) in zip(p_infosets, d, strict=True):
                if len(node.actions) != len(v):
                    raise ValueError(
                        f"Number of elements does not match number of "
                        f"actions for infoset {node} for {p}"
                    )
                profile._setprob_infoset(
                    node,
                    {a: typefunc(u) for a, u in zip(node.actions, v, strict=True)},
                    sparse=True,
                )
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
            for player in self.players:
                for node in self.get_infosets(player):
                    profile._setprob_infoset(
                        node, _dirichlet_distribution(node.actions, gen), sparse=True
                    )
            return profile
        elif denom < 1:
            raise ValueError("random_behavior_profile(): denom must be positive")
        else:
            profile = self.mixed_behavior_profile(rational=True)
            for player in self.players:
                for node in self.get_infosets(player):
                    profile._setprob_infoset(
                        node, _grid_distribution(node.actions, denom, gen), sparse=True
                    )
            return profile

    def strategy_support_profile(
            self, strategies: typing.Callable | None = None
    ) -> StrategySupportProfile:
        """Create a new `StrategySupportProfile` on the game.

        Parameters
        ----------
        strategies : function, optional
            By default the support profile contains all strategies for all players.
            If specified, called as ``strategies(player, label)`` for each strategy of
            each player; only strategies for which it returns `True` are included.

        Returns
        -------
        StrategySupportProfile
        """
        profile = StrategySupportProfile.wrap(make_shared[c_StrategySupportProfile](self.game))
        if strategies is not None:
            for player in self.players:
                for label in self.get_strategies(player):
                    if not strategies(player, label):
                        handle = self._resolve_strategy(
                            player, label, "strategy_support_profile"
                        )
                        if not deref(profile.profile).RemoveStrategy(handle):
                            raise ValueError("attempted to remove the last strategy for player")
        return profile

    def behavior_support_profile(
            self, actions: typing.Callable | None = None
    ) -> BehaviorSupportProfile:
        """Create a new `BehaviorSupportProfile` on the game.

        Parameters
        ----------
        actions : function, optional
            By default the support profile contains all actions at all information
            sets. If specified, called as ``actions(node, action)`` for each action at
            each information set, where ``node`` is a representative node of the
            information set; only actions for which it returns `True` are included.

        Returns
        -------
        BehaviorSupportProfile
        """
        profile = BehaviorSupportProfile.wrap(make_shared[c_BehaviorSupportProfile](self.game))
        if actions is not None:
            for player in self.players:
                for node in self.get_infosets(player):
                    infoset_handle: c_GameInfoset = cython.cast(Node, node)._infoset_handle()
                    for action in infoset_handle.deref().GetActions():
                        if not actions(node, action.deref().GetLabel().decode("utf-8")):
                            if not deref(profile.profile).RemoveAction(action):
                                raise ValueError(
                                    "attempted to remove the last action at an information set"
                                )
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

    @cython.cfunc
    def _resolve_player(
        self, player: typing.Any, funcname: str, argname: str = "player"
    ) -> c_GamePlayer:
        """Resolve `label` to the C++ handle of one of the game's (personal) players.

        Not part of the public API -- used internally to bridge a player label to
        the underlying C++ object without ever constructing a Python wrapper for it.

        Raises
        ------
        KeyError
            If no player has label `player`.
        TypeError
            If `player` is not a `str`.
        ValueError
            If `player` is an empty string or all spaces.
        """
        if not isinstance(player, str):
            raise TypeError(
                f"{funcname}(): {argname} must be str, not {player.__class__.__name__}"
            )
        if not player.strip():
            raise ValueError(
                f"{funcname}(): {argname} cannot be an empty string or all spaces"
            )
        for p in self.game.deref().GetPlayers():
            if p.deref().GetLabel().decode("utf-8") == player:
                return p
        raise KeyError(f"{funcname}(): no player with label '{player}'")

    @cython.cfunc
    def _resolve_strategy(self, player: str, label, funcname: str,
                          argname: str = "strategy") -> c_GameStrategy:
        """Resolve `label` to the C++ handle of one of `player`'s strategies.

        Not part of the public API -- used internally to bridge a strategy label to
        the underlying C++ object without ever constructing a Python wrapper for it.

        Raises
        ------
        KeyError
            If no player has label `player`, or `player` has no strategy with label `label`.
        TypeError
            If `label` is not a `str`.
        ValueError
            If `label` is an empty string or all spaces.
        """
        if not isinstance(label, str):
            raise TypeError(
                f"{funcname}(): {argname} must be a strategy label (str), "
                f"not {label.__class__.__name__}"
            )
        if not label.strip():
            raise ValueError(f"{funcname}(): {argname} cannot be an empty string or all spaces")
        resolved_player: c_GamePlayer = self._resolve_player(player, funcname, "player")
        for strategy in resolved_player.deref().GetStrategies():
            if strategy.deref().GetLabel().decode("utf-8") == label:
                return strategy
        raise KeyError(
            f"{funcname}(): player '{player}' has no strategy with label '{label}'"
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
        elif isinstance(node, Selector):
            resolved = self._get_nodes(node)
            if len(resolved) != 1:
                raise ValueError(
                    f"{funcname}(): {argname} selector must resolve to exactly one "
                    f"node, resolved to {len(resolved)}"
                )
            return resolved[0]
        elif isinstance(node, tuple):
            # A History -- the manual fallback: root-anchored, every step exact.
            return self._resolve_node(Selector().path(*node), funcname, argname)
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

        `nodes` may also be a `Selector` (an `H`-built expression), evaluated
        against this game via `_get_nodes` before the usual resolution.
        """
        if isinstance(nodes, Selector):
            nodes = self._get_nodes(nodes)
        resolved_nodes = [
            self._resolve_node(n, funcname, argname)
            for n in (nodes if hasattr(nodes, "__iter__") and not isinstance(nodes, (str, tuple))
                      else [nodes])
        ]
        if not resolved_nodes:
            raise ValueError(f"{funcname}(): `{argname}` must not be empty")
        if len(resolved_nodes) != len(set(resolved_nodes)):
            raise ValueError(f"{funcname}(): Each node must be referenced only once")
        return resolved_nodes

    def _resolve_infoset(self,
                         infoset: typing.Any, funcname: str, argname: str = "infoset") -> Node:
        """Resolve an attempt to reference a personal player's information set of the
        game, via a member node or its label.

        Parameters
        ----------
        infoset : Node or str
            A node belonging to the information set, or such a node's label.
        funcname : str
            The name of the function to raise any exception on behalf of.
        argname : str, default 'infoset'
            The name of the argument being checked

        Returns
        -------
        Node
            The resolved node itself, validated as currently belonging to a personal
            player's information set.

        Raises
        ------
        MismatchError
            If `infoset` is a `Node` from a different game.
        KeyError
            If `infoset` is a string and no node in the game has that label.
        TypeError
            If `infoset` is not a `Node` or a `str`
        ValueError
            If `infoset` resolves to a chance event rather than a personal player's
            information set, or to no information set at all (the node is terminal).
        """
        resolved_node = self._resolve_node(infoset, funcname, argname)
        is_personal, is_chance = _node_infoset_kind(resolved_node)
        return _resolve_infoset_or_event_kind(
            resolved_node, is_personal, is_chance,
            "information set", "a personal player's information set", "a chance event",
            funcname, argname
        )

    def _resolve_event(self,
                       event: typing.Any, funcname: str, argname: str = "event") -> Node:
        """Resolve an attempt to reference a chance event of the game, via a member
        node or its label.

        Parameters
        ----------
        event : Node or str
            A node belonging to the event, or such a node's label.
        funcname : str
            The name of the function to raise any exception on behalf of.
        argname : str, default 'event'
            The name of the argument being checked

        Returns
        -------
        Node
            The resolved node itself, validated as currently belonging to a chance
            event.

        Raises
        ------
        MismatchError
            If `event` is a `Node` from a different game.
        KeyError
            If `event` is a string and no node in the game has that label.
        TypeError
            If `event` is not a `Node` or a `str`
        ValueError
            If `event` resolves to a personal player's information set rather than a
            chance event, or to no event at all (the node is terminal).
        """
        resolved_node = self._resolve_node(event, funcname, argname)
        is_personal, is_chance = _node_infoset_kind(resolved_node)
        return _resolve_infoset_or_event_kind(
            resolved_node, is_chance, is_personal,
            "event", "a chance event", "a personal player's information set",
            funcname, argname
        )

    def _resolve_infoset_or_event(self,
                                  infoset: typing.Any,
                                  funcname: str,
                                  argname: str = "infoset") -> Node:
        """Resolve an attempt to reference an information set or event of the game
        (whichever applies), via a member node or its label. For operations that
        apply uniformly to either, such as attaching to an existing one.

        Parameters
        ----------
        infoset : Node or str
            A node belonging to the information set or event, or such a node's label.
        funcname : str
            The name of the function to raise any exception on behalf of.
        argname : str, default 'infoset'
            The name of the argument being checked

        Returns
        -------
        Node
            The resolved node itself, validated as currently belonging to some
            information set or event.

        Raises
        ------
        MismatchError
            If `infoset` is a `Node` from a different game.
        KeyError
            If `infoset` is a string and no node in the game has that label.
        TypeError
            If `infoset` is not a `Node` or a `str`
        ValueError
            If `infoset` resolves to no information set or event (the node is
            terminal).
        """
        resolved_node = self._resolve_node(infoset, funcname, argname)
        is_personal, is_chance = _node_infoset_kind(resolved_node)
        if is_personal or is_chance:
            return resolved_node
        raise ValueError(
            f"{funcname}(): {argname} resolves to no information set "
            f"(the node is terminal)"
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

    def append_move(self, nodes: Selector | GroupedSelector,
                    player: str,
                    actions: list[str]) -> None:
        """Add a move for `player` at terminal `nodes`.  All elements of `nodes` become part of
        a new information set, with actions labeled according to `actions`.

        `player` must be a personal player; use `append_event` to add a chance move.

        `nodes` is a `Selector` (an `H`-built expression, evaluated against this game
        and treated as a flat set of nodes) or a `GroupedSelector` (an `H`-built
        `.by(...)` expression) -- in the latter case, one new information set is
        created per distinct group, rather than one spanning every match.

        .. versionchanged:: 17.0.0
            `nodes` is now a `Selector` or `GroupedSelector`; a `Node` or
            `NodeReferenceSet` is no longer accepted directly -- build one with `H`.

        Raises
        ------
        TypeError
            If `nodes` is not a `Selector` or `GroupedSelector`.
        UndefinedOperationError
            If `nodes` are not all terminal, or `actions` is empty.
        KeyError
            If no player in the game has label `player`.
        ValueError
            If `nodes` has duplicated elements, or is empty; or if `actions` contains
            an empty or a duplicated label.
        """
        if isinstance(nodes, GroupedSelector):
            for group in self._group_nodes(nodes).values():
                if not group:
                    continue
                self._append_move_at(group, player, actions)
            return
        if not isinstance(nodes, Selector):
            raise TypeError(
                f"append_move(): nodes must be a Selector or GroupedSelector, "
                f"not {nodes.__class__.__name__}"
            )
        self._append_move_at(nodes, player, actions)

    def _append_move_at(self, nodes: Selector | list[Node], player: str,
                        actions: list[str]) -> None:
        """Internal: shared body of `append_move`, taking either a `Selector` or an
        already-resolved list of `Node` (the latter used for one group at a time,
        dispatched from a `GroupedSelector`)."""
        resolved_player = self._resolve_player(player, "append_move")
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
        self.game.deref().AppendMove(resolved_node.node, resolved_player, c_actions)
        infoset_handle: c_GameInfoset = resolved_node._infoset_handle()
        for n in resolved_nodes[1:]:
            self.game.deref().AppendMove(cython.cast(Node, n).node, infoset_handle)

    def append_infoset(self, nodes: Selector | GroupedSelector,
                       infoset: Selector) -> None:
        """Add a move at terminal `nodes`, joining the information set that the node
        identified by `infoset` belongs to.

        `nodes` is a `Selector` (an `H`-built expression, evaluated against this game
        and treated as a flat set of nodes) or a `GroupedSelector` (an `H`-built
        `.by(...)` expression, whose groups are pooled together -- every resolved
        node joins the same `infoset` regardless of grouping).

        `infoset` is a `Selector` that must resolve to exactly one node; that node
        must belong to a personal player and must not be terminal -- the information
        set it currently belongs to is the one joined.

        .. versionchanged:: 17.0.0
            `nodes` is now a `Selector` or `GroupedSelector`, and `infoset` is now a
            `Selector` identifying a node by the information set it belongs to,
            rather than a `Node` or `str` reference to an `Infoset`/`Event` directly.
            Joining an existing chance event is no longer supported here.

        Parameters
        ----------
        nodes : Selector or GroupedSelector
            The nonempty set of terminal nodes at which to add the move.
        infoset : Selector
            A `Selector` resolving to a single node of the personal player's
            information set to join.

        Raises
        ------
        TypeError
            If `nodes` is not a `Selector` or `GroupedSelector`, or `infoset` is not
            a `Selector`.
        UndefinedOperationError
            If any element in `nodes` is not a terminal node, or `infoset` resolves
            to a terminal node or to a chance node.
        ValueError
            If `nodes` has duplicated elements, or is empty; or if `infoset` does not
            resolve to exactly one node.
        """
        if isinstance(nodes, GroupedSelector):
            nodes = [n for group in self._group_nodes(nodes).values() for n in group]
        elif not isinstance(nodes, Selector):
            raise TypeError(
                f"append_infoset(): nodes must be a Selector or GroupedSelector, "
                f"not {nodes.__class__.__name__}"
            )
        if not isinstance(infoset, Selector):
            raise TypeError(
                f"append_infoset(): infoset must be a Selector, not {infoset.__class__.__name__}"
            )
        infoset_node = cython.cast(Node, self._resolve_node(infoset, "append_infoset", "infoset"))
        is_personal, _ = _node_infoset_kind(infoset_node)
        if not is_personal:
            raise UndefinedOperationError(
                "append_infoset(): infoset must resolve to a personal player's node"
            )
        infoset_handle: c_GameInfoset = infoset_node._infoset_handle()
        resolved_nodes = self._resolve_nodes(nodes, "append_infoset", "nodes")
        if any(len(n.children) > 0 for n in resolved_nodes):
            raise UndefinedOperationError("append_infoset(): `nodes` must be terminal nodes")
        for n in resolved_nodes:
            self.game.deref().AppendMove(cython.cast(Node, n).node, infoset_handle)

    def append_event(self, nodes: Selector | GroupedSelector,
                     actions: typing.Mapping) -> None:
        """Add a chance move at terminal `nodes`, with actions and their probabilities
        given by `actions`.  All elements of `nodes` become part of a new event.

        `nodes` is a `Selector` (an `H`-built expression, evaluated against this game
        and treated as a flat set of nodes) or a `GroupedSelector` (an `H`-built
        `.by(...)` expression) -- in the latter case, one new event is created per
        distinct group, rather than one spanning every match.

        .. versionadded:: 17.0.0
        .. versionchanged:: 17.0.0
            `nodes` is now a `Selector` or `GroupedSelector`; a `Node` or
            `NodeReferenceSet` is no longer accepted directly -- build one with `H`.
        .. versionchanged:: 17.0.0
            `actions` and `probs` are combined into a single mapping from action
            label to probability, rather than a list of labels plus a separate
            probability sequence or mapping.

        Parameters
        ----------
        nodes : Selector or GroupedSelector
            The nonempty set of terminal nodes at which to add the move.
        actions : Mapping
            A mapping from each new action's label to its probability.  Nonempty,
            with no empty label.  Probabilities are non-negative and sum to exactly
            one.

        Raises
        ------
        TypeError
            If `nodes` is not a `Selector` or `GroupedSelector`.
        UndefinedOperationError
            If `nodes` are not all terminal, or `actions` is empty.
        ValueError
            If `nodes` has duplicated elements, or is empty; if `actions` contains
            an empty label; or if the probabilities are not non-negative numbers
            summing to exactly one.
        """
        if isinstance(nodes, GroupedSelector):
            for group in self._group_nodes(nodes).values():
                if not group:
                    continue
                self._append_event_at(group, actions)
            return
        if not isinstance(nodes, Selector):
            raise TypeError(
                f"append_event(): nodes must be a Selector or GroupedSelector, "
                f"not {nodes.__class__.__name__}"
            )
        self._append_event_at(nodes, actions)

    def _append_event_at(self, nodes: Selector | list[Node], actions: typing.Mapping) -> None:
        """Internal: shared body of `append_event`, taking either a `Selector` or an
        already-resolved list of `Node` (the latter used for one group at a time,
        dispatched from a `GroupedSelector`)."""
        action_labels = list(actions)
        if not action_labels:
            raise UndefinedOperationError("append_event(): `actions` must be a nonempty mapping")
        if any(not label for label in action_labels):
            raise ValueError("append_event(): action labels must not be empty")
        resolved_nodes = self._resolve_nodes(nodes, "append_event", "nodes")
        if any(len(n.children) > 0 for n in resolved_nodes):
            raise UndefinedOperationError("append_event(): `nodes` must be terminal nodes")

        resolved_node = cython.cast(Node, resolved_nodes[0])
        c_actions = stdvector[string]()
        for label in action_labels:
            c_actions.push_back(label.encode("utf-8"))
        c_probs = stdvector[c_Number]()
        for label in action_labels:
            c_probs.push_back(_to_number(actions[label]))
        self.game.deref().AppendEvent(resolved_node.node, c_actions, c_probs)
        event_handle: c_GameInfoset = resolved_node._infoset_handle()
        for n in resolved_nodes[1:]:
            self.game.deref().AppendMove(cython.cast(Node, n).node, event_handle)

    def insert_move(self, node: Selector,
                    player: str, actions: list[str]) -> None:
        """Insert a move for `player` prior to the node identified by `node`, with
        actions labeled according to `actions`.  The node becomes the first child of
        the newly-inserted node.

        `player` must be a personal player; use `insert_event` to insert a chance move.

        `node` is a `Selector` (an `H`-built expression, evaluated against this game)
        that must resolve to exactly one node.

        .. versionchanged:: 17.0.0
            `node` is now a `Selector`; a `Node` or `str` is no longer accepted
            directly -- build one with `H`.

        Raises
        ------
        TypeError
            If `node` is not a `Selector`.
        UndefinedOperationError
            If `actions` is empty.
        KeyError
            If no player in the game has label `player`.
        ValueError
            If `node` does not resolve to exactly one node, or `actions` contains an
            empty or a duplicated label.
        """
        if not isinstance(node, Selector):
            raise TypeError(
                f"insert_move(): node must be a Selector, not {node.__class__.__name__}"
            )
        resolved_node = cython.cast(Node, self._resolve_node(node, "insert_move"))
        resolved_player = self._resolve_player(player, "insert_move")
        if not actions:
            raise UndefinedOperationError("insert_move(): `actions` must be a nonempty list")
        if any(not label for label in actions):
            raise ValueError("insert_move(): action labels must not be empty")
        if len(set(actions)) != len(actions):
            raise ValueError("insert_move(): action labels must be unique")
        c_actions = stdvector[string]()
        for label in actions:
            c_actions.push_back(label.encode("utf-8"))
        self.game.deref().InsertMove(resolved_node.node, resolved_player, c_actions)

    def insert_infoset(self, node: Selector,
                       infoset: Selector) -> None:
        """Insert a move in the information set or event that the node identified by
        `infoset` belongs to, prior to the node identified by `node`.  The node
        becomes the first child of the newly-inserted node.

        `node` and `infoset` are each a `Selector` (an `H`-built expression,
        evaluated against this game) that must resolve to exactly one node.

        .. versionchanged:: 17.0.0
            `node` is now a `Selector`; a `Node` or `str` is no longer accepted
            directly -- build one with `H`.
        .. versionchanged:: 17.0.0
            `infoset` is now a `Selector` identifying a node by the information set
            or event it belongs to, rather than a `Node` or `str` reference to an
            `Infoset`/`Event` directly.

        Raises
        ------
        TypeError
            If `node` or `infoset` is not a `Selector`.
        ValueError
            If `node` or `infoset` does not resolve to exactly one node, or if the
            node identified by `infoset` belongs to no information set or event (it
            is terminal).
        """
        if not isinstance(node, Selector):
            raise TypeError(
                f"insert_infoset(): node must be a Selector, not {node.__class__.__name__}"
            )
        if not isinstance(infoset, Selector):
            raise TypeError(
                f"insert_infoset(): infoset must be a Selector, not {infoset.__class__.__name__}"
            )
        resolved_node = cython.cast(Node, self._resolve_node(node, "insert_infoset"))
        resolved_infoset_node = cython.cast(
            Node, self._resolve_infoset_or_event(infoset, "insert_infoset")
        )
        self.game.deref().InsertMove(resolved_node.node, resolved_infoset_node._infoset_handle())

    def insert_event(self, node: Selector, actions: typing.Mapping) -> None:
        """Insert a chance move prior to the node identified by `node`, with actions
        and their probabilities given by `actions`.  The node becomes the first
        child of the newly-inserted node.

        `node` is a `Selector` (an `H`-built expression, evaluated against this
        game) that must resolve to exactly one node.

        .. versionadded:: 17.0.0
        .. versionchanged:: 17.0.0
            `node` is now a `Selector`; a `Node` or `str` is no longer accepted
            directly -- build one with `H`.
        .. versionchanged:: 17.0.0
            `actions` and `probs` are combined into a single mapping from action
            label to probability, rather than a list of labels plus a separate
            probability sequence or mapping.

        Parameters
        ----------
        node : Selector
            A `Selector` resolving to the single node before which to insert the
            move.
        actions : Mapping
            A mapping from each new action's label to its probability.  Nonempty,
            with no empty label.  Probabilities are non-negative and sum to exactly
            one.

        Raises
        ------
        TypeError
            If `node` is not a `Selector`.
        UndefinedOperationError
            If `actions` is empty.
        ValueError
            If `node` does not resolve to exactly one node; if `actions` contains
            an empty label; or if the probabilities are not non-negative numbers
            summing to exactly one.
        """
        if not isinstance(node, Selector):
            raise TypeError(
                f"insert_event(): node must be a Selector, not {node.__class__.__name__}"
            )
        resolved_node = cython.cast(Node, self._resolve_node(node, "insert_event"))
        action_labels = list(actions)
        if not action_labels:
            raise UndefinedOperationError("insert_event(): `actions` must be a nonempty mapping")
        if any(not label for label in action_labels):
            raise ValueError("insert_event(): action labels must not be empty")
        c_actions = stdvector[string]()
        for label in action_labels:
            c_actions.push_back(label.encode("utf-8"))
        c_probs = stdvector[c_Number]()
        for label in action_labels:
            c_probs.push_back(_to_number(actions[label]))
        self.game.deref().InsertEvent(resolved_node.node, c_actions, c_probs)

    def copy_tree(self, src: Selector, dest: Selector) -> None:
        """Copy the subtree rooted at the node identified by `src` to the node
        identified by `dest`.

        Each node in the subtree copied to follow `dest` is placed in the same information set
        as the corresponding node in the original subtree under `src`.

        It is permitted for `dest` to be a descendant of `src`.
        The operation uses the subtree rooted at `src` as it is at the time the function is called,
        so no infinite recursion is triggered.

        The outcome associated with `dest` is not changed by this operation.

        `src` and `dest` are each a `Selector` (an `H`-built expression, evaluated
        against this game) that must resolve to exactly one node.

        .. versionchanged:: 17.0.0
            `src` and `dest` are now `Selector`s; a `Node` or `str` is no longer
            accepted directly -- build one with `H`.

        Parameters
        ----------
        src : Selector
            A `Selector` resolving to the root of the source subtree to copy.
        dest : Selector
            A `Selector` resolving to the destination subtree to copy to.  Must
            resolve to a terminal node.

        Raises
        ------
        TypeError
            If `src` or `dest` is not a `Selector`.
        UndefinedOperationError
            If `dest` is not a terminal node.
        ValueError
            If `src` or `dest` does not resolve to exactly one node.
        """
        if not isinstance(src, Selector):
            raise TypeError(f"copy_tree(): src must be a Selector, not {src.__class__.__name__}")
        if not isinstance(dest, Selector):
            raise TypeError(
                f"copy_tree(): dest must be a Selector, not {dest.__class__.__name__}"
            )
        resolved_src = cython.cast(Node, self._resolve_node(src, "copy_tree", "src"))
        resolved_dest = cython.cast(Node, self._resolve_node(dest, "copy_tree", "dest"))
        if not resolved_dest.is_terminal:
            raise UndefinedOperationError("copy_tree(): `dest` must be a terminal node.")
        self.game.deref().CopyTree(resolved_dest.node, resolved_src.node)

    def move_tree(self, src: Selector, dest: Selector) -> None:
        """Move the subtree rooted at the node identified by `src` to the node
        identified by `dest`.

        `src` and `dest` are each a `Selector` (an `H`-built expression, evaluated
        against this game) that must resolve to exactly one node.

        .. versionchanged:: 17.0.0
            `src` and `dest` are now `Selector`s; a `Node` or `str` is no longer
            accepted directly -- build one with `H`.

        Parameters
        ----------
        src : Selector
            A `Selector` resolving to the root of the source subtree to move.
        dest : Selector
            A `Selector` resolving to the destination subtree to move to.  Must
            resolve to a terminal node.

        Raises
        ------
        TypeError
            If `src` or `dest` is not a `Selector`.
        UndefinedOperationError
            If `dest` is not a terminal node, or `dest` is a successor of `src`.
        ValueError
            If `src` or `dest` does not resolve to exactly one node.
        """
        if not isinstance(src, Selector):
            raise TypeError(f"move_tree(): src must be a Selector, not {src.__class__.__name__}")
        if not isinstance(dest, Selector):
            raise TypeError(
                f"move_tree(): dest must be a Selector, not {dest.__class__.__name__}"
            )
        resolved_src = cython.cast(Node, self._resolve_node(src, "move_tree", "src"))
        resolved_dest = cython.cast(Node, self._resolve_node(dest, "move_tree", "dest"))
        if not resolved_dest.is_terminal:
            raise UndefinedOperationError("move_tree(): `dest` must be a terminal node.")
        if resolved_dest.is_successor_of(resolved_src):
            raise UndefinedOperationError("move_tree(): `dest` cannot be a successor of `src`.")
        self.game.deref().MoveTree(resolved_dest.node, resolved_src.node)

    def delete_parent(self, node: Selector) -> None:
        """Delete the parent of the node identified by `node`.  That node replaces
        its parent in the tree.  All other subtrees rooted at the parent are deleted.

        `node` is a `Selector` (an `H`-built expression, evaluated against this
        game) that must resolve to exactly one node.

        .. versionchanged:: 17.0.0
            `node` is now a `Selector`; a `Node` or `str` is no longer accepted
            directly -- build one with `H`.

        Parameters
        ----------
        node : Selector
            A `Selector` resolving to the single node to retain after deleting its
            parent.

        Raises
        ------
        TypeError
            If `node` is not a `Selector`.
        ValueError
            If `node` does not resolve to exactly one node.
        """
        if not isinstance(node, Selector):
            raise TypeError(
                f"delete_parent(): node must be a Selector, not {node.__class__.__name__}"
            )
        resolved_node = cython.cast(Node, self._resolve_node(node, "delete_parent"))
        self.game.deref().DeleteParent(resolved_node.node)

    def delete_tree(self, node: Selector) -> None:
        """Truncate the game tree at the node identified by `node`, deleting the
        subtree beneath it.

        `node` is a `Selector` (an `H`-built expression, evaluated against this
        game) that must resolve to exactly one node.

        .. versionchanged:: 17.0.0
            `node` is now a `Selector`; a `Node` or `str` is no longer accepted
            directly -- build one with `H`.

        Parameters
        ----------
        node : Selector
            A `Selector` resolving to the single node to truncate the game at.

        Raises
        ------
        TypeError
            If `node` is not a `Selector`.
        ValueError
            If `node` does not resolve to exactly one node.
        """
        if not isinstance(node, Selector):
            raise TypeError(
                f"delete_tree(): node must be a Selector, not {node.__class__.__name__}"
            )
        resolved_node = cython.cast(Node, self._resolve_node(node, "delete_tree"))
        self.game.deref().DeleteTree(resolved_node.node)

    def set_move_actions(self,
                         infoset: Selector,
                         actions: list[str],
                         drop: bool = False,
                         add: bool = True) -> None:
        """Set the actions at the move that the node identified by `infoset`
        belongs to, to be `actions`, matching by label.

        An entry of `actions` matching the label of a current action refers to that action,
        which keeps its subtrees; an entry matching no current action creates a new action there,
        leading to a new terminal node at every member; a current action whose label is not
        in `actions` is deleted, along with the subtrees its branches lead to.
        Listing the current labels in a new order reorders the actions as well as the children.

        `infoset` is a `Selector` (an `H`-built expression, evaluated against this
        game) that must resolve to exactly one node.

        .. versionadded:: 17.0.0
        .. versionchanged:: 17.0.0
            `infoset` is now a `Selector`; a `Node` or `str` is no longer accepted
            directly -- build one with `H`.

        Parameters
        ----------
        infoset : Selector
            A `Selector` resolving to a single node belonging to the (personal
            player's) move at which to set the actions.
        actions : list of str
            The labels of the actions the move is to have, in order.
            Must be nonempty and without duplicates; each label must be a valid, nonempty label.
        drop : bool, default False
            Deleting actions is destructive, so it must be explicitly confirmed:
            if any current action is missing from `actions` and `drop` is `False`,
            the operation raises without modifying the game.
        add : bool, default True
            If `False`, entries of `actions` matching no current action raise.

        Raises
        ------
        TypeError
            If `infoset` is not a `Selector`; or if `actions` is a string, or not
            an iterable of strings.
        UndefinedOperationError
            If `actions` is empty.
        ValueError
            If `infoset` does not resolve to exactly one node, or resolves to an
            event rather than a personal player's move (use `set_event_actions`
            for an event); or if a label in `actions` is repeated, empty, or
            invalid; or if adding or deleting actions is not confirmed by
            `add`/`drop`.

        See Also
        --------
        set_event_actions : The corresponding operation for the actions of an event.
        relabel_actions : Change the labels of actions, leaving the tree unchanged.
        """
        if not isinstance(infoset, Selector):
            raise TypeError(
                f"set_move_actions(): infoset must be a Selector, not {infoset.__class__.__name__}"
            )
        resolved_infoset = cython.cast(Node, self._resolve_infoset(infoset, "set_move_actions"))
        if isinstance(actions, str) or not hasattr(actions, "__iter__"):
            raise TypeError("set_move_actions(): actions must be an iterable of str")
        labels = list(actions)
        if any(not isinstance(label, str) for label in labels):
            raise TypeError("set_move_actions(): actions must be an iterable of str")
        if not labels:
            raise UndefinedOperationError("set_move_actions(): `actions` must be a nonempty list")
        current = list(resolved_infoset.actions)
        _reconcile_labels(
            current, labels, add, drop, "set_move_actions",
            "information set", "action", "actions", "the subtrees they lead to"
        )
        c_labels = stdvector[string]()
        for label in labels:
            c_labels.push_back(label.encode("utf-8"))
        self.game.deref().SetMoveActions(resolved_infoset._infoset_handle(), c_labels)

    def set_event_actions(self,
                          event: Selector,
                          probs: typing.Mapping,
                          drop: bool = False,
                          add: bool = True) -> None:
        """Set the actions at the event that the node identified by `event`
        belongs to, to be the keys of `probs`, in order, with the given
        probability distribution.

        A key of `probs` matching the label of a current action refers to that action,
        which keeps its subtrees; a key matching no current action creates a new action
        there, leading to a new terminal node at every member; a current action whose label
        is not a key of `probs` is deleted, along with the subtrees its branches lead to.
        Listing the current labels as keys in a new order reorders the actions as well as
        the children.

        Unlike `set_move_actions`, the probability distribution is always declared as part
        of the operation, rather than inferred from the actions which remain: there is no
        way to reorder an event's actions without also restating their probabilities.

        `event` is a `Selector` (an `H`-built expression, evaluated against this
        game) that must resolve to exactly one node.

        .. versionadded:: 17.0.0
        .. versionchanged:: 17.0.0
            `event` is now a `Selector`; a `Node` or `str` is no longer accepted
            directly -- build one with `H`.

        Parameters
        ----------
        event : Selector
            A `Selector` resolving to a single node belonging to the event at
            which to set the actions.
        probs : dict-like
            A mapping from the label of each action the event is to have, in order, to its
            probability.  Must be nonempty, with valid, nonempty keys.  Values must be
            non-negative numbers summing to exactly one.
        drop : bool, default False
            Deleting actions is destructive, so it must be explicitly confirmed:
            if any current action is missing as a key of `probs` and `drop` is `False`,
            the operation raises without modifying the game.
        add : bool, default True
            If `False`, keys of `probs` matching no current action raise.

        Raises
        ------
        TypeError
            If `event` is not a `Selector`; or if `probs` is not a mapping, or a
            key of `probs` is not a string.
        UndefinedOperationError
            If `probs` is empty, or if `event` resolves to a personal player's
            information set rather than an event; use `set_move_actions` for a
            personal player's move.
        ValueError
            If `event` does not resolve to exactly one node; if a key of `probs`
            is empty or invalid; if adding or deleting actions is not confirmed by
            `add`/`drop`; or if the values of `probs` are not non-negative numbers
            summing to exactly one.

        See Also
        --------
        set_move_actions : The corresponding operation for the actions of a personal
            player's move.
        relabel_actions : Change the labels of actions, leaving the tree unchanged.
        """
        if not isinstance(event, Selector):
            raise TypeError(
                f"set_event_actions(): event must be a Selector, not {event.__class__.__name__}"
            )
        resolved_event = cython.cast(Node, self._resolve_event(event, "set_event_actions"))
        if not isinstance(probs, typing.Mapping):
            raise TypeError(
                "set_event_actions(): probs must be a mapping from label to probability"
            )
        labels = list(probs.keys())
        if any(not isinstance(label, str) for label in labels):
            raise TypeError("set_event_actions(): keys of probs must be str")
        if not labels:
            raise UndefinedOperationError(
                "set_event_actions(): `probs` must be a nonempty mapping"
            )
        current = list(resolved_event.actions)
        _reconcile_labels(
            current, labels, add, drop, "set_event_actions",
            "information set", "action", "actions", "the subtrees they lead to"
        )
        c_labels = stdvector[string]()
        c_probs = stdvector[c_Number]()
        for label in labels:
            c_labels.push_back(label.encode("utf-8"))
            c_probs.push_back(_to_number(probs[label]))
        self.game.deref().SetEventActions(resolved_event._infoset_handle(), c_labels, c_probs)

    def make_event(self,
                   nodes: Selector | GroupedSelector,
                   probs: typing.Mapping,
                   label: str | None = None) -> None:
        """Form `nodes` into a single event with distribution `probs`.

        `nodes` must all be nonterminal nodes of this game with the same actions, with the same
        labels in the same order.  They need not be chance nodes; personal nodes are
        converted, and the move is thereafter resolved by chance.  Nodes are removed from
        whatever information sets or events they currently belong to; any of those which
        retain members survive, keeping their labels, and those left with no members are deleted.
        The resulting event's members, actions, and player are accessible via
        ``Node.members``/``Node.actions``/``Node.player`` for any node in `nodes`.

        `nodes` is a `Selector` (an `H`-built expression, evaluated against this game
        and treated as a flat set of nodes) or a `GroupedSelector` (an `H`-built
        `.by(...)` expression, whose groups are pooled together into the one event).

        Which resolved node is treated as "first", determining the action order of
        the event and the frame against which keys of `probs` are resolved, follows
        `nodes`' own resolution order.

        .. versionadded:: 17.0.0
        .. versionchanged:: 17.0.0
            `nodes` is now a `Selector` or `GroupedSelector`; a `Node` or
            `NodeReferenceSet` is no longer accepted directly -- build one with `H`.
        .. versionchanged:: 17.0.0
            `probs` is now always a mapping from action label to probability; a
            positional sequence is no longer accepted.

        Parameters
        ----------
        nodes : Selector or GroupedSelector
            The nonempty set of nonterminal nodes to place in the event.
        probs : Mapping
            The probability distribution over the actions of the event, as a mapping
            from action label to probability.  May be sparse; omitted actions are
            assigned probability zero.  Probabilities are non-negative and sum to
            exactly one.
        label : str, optional
            The label of the new event.  If specified, must be unique among the events
            of the game after the operation.  A label currently held by another event
            may be reused only if all members of that event are among `nodes`.

        Raises
        ------
        TypeError
            If `nodes` is not a `Selector` or `GroupedSelector`, or `probs` is not a
            mapping.
        KeyError
            If a key of `probs` matches no action label of the event.
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
        if isinstance(nodes, GroupedSelector):
            nodes = [n for group in self._group_nodes(nodes).values() for n in group]
        elif not isinstance(nodes, Selector):
            raise TypeError(
                f"make_event(): nodes must be a Selector or GroupedSelector, "
                f"not {nodes.__class__.__name__}"
            )
        if not isinstance(probs, typing.Mapping):
            raise TypeError(
                f"make_event(): probs must be a mapping, not {probs.__class__.__name__}"
            )
        resolved_nodes = self._resolve_nodes(nodes, "make_event")
        if any(n.is_terminal for n in resolved_nodes):
            raise UndefinedOperationError(
                "make_event(): all nodes must be nonterminal"
            )
        resolved_node = cython.cast(Node, resolved_nodes[0])
        action_labels = list(resolved_node.actions)
        if any(list(n.actions) != action_labels
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
                        infoset: Selector,
                        labels: typing.Mapping[str, str],
                        strict: bool = True) -> None:
        """Simultaneously reassign the labels of actions at the information set or
        event that the node identified by `infoset` belongs to.

        `labels` maps current action labels to their replacements.  The reassignment
        is simultaneous, so labels can be swapped directly, e.g. ``{"a": "b", "b": "a"}``.
        Actions are not re-ordered: each relabelled action keeps its position and, at an event,
        its probability.  After the operation, the labels must be nonempty and unique.

        `infoset` is a `Selector` (an `H`-built expression, evaluated against this
        game) that must resolve to exactly one node.

        .. versionadded:: 17.0.0
        .. versionchanged:: 17.0.0
            `infoset` is now a `Selector`; a `Node` or `str` is no longer accepted
            directly -- build one with `H`.

        Parameters
        ----------
        infoset : Selector
            A `Selector` resolving to a single node belonging to the information
            set or event at which to relabel actions.
        labels : Mapping[str, str]
            A mapping from current action labels to replacement labels.  Entries
            whose key equals their value are ignored.
        strict : bool, default True
            If `True`, every key of `labels` must be the label of an action at
            `infoset`, and unknown keys raise ``KeyError``.  If `False`, unknown
            keys are ignored.

        Raises
        ------
        TypeError
            If `infoset` is not a `Selector`; or if `labels` is not a mapping, or
            any key or value is not a string.
        KeyError
            If, when `strict` is `True`, a key of `labels` matches no action at
            `infoset`.
        ValueError
            If `infoset` does not resolve to exactly one node; if a key of
            `labels` matches more than one action at `infoset` (possible in games
            read from files predating unique-label enforcement); or if any
            replacement label is empty, is not a valid label, or would result in a
            duplicate label at the information set.
        """
        if not isinstance(infoset, Selector):
            raise TypeError(
                f"relabel_actions(): infoset must be a Selector, not {infoset.__class__.__name__}"
            )
        resolved_infoset = cython.cast(
            Node, self._resolve_infoset_or_event(infoset, "relabel_actions")
        )
        if not hasattr(labels, "items"):
            raise TypeError(
                f"relabel_actions(): labels must be a mapping, "
                f"not {labels.__class__.__name__}"
            )
        current = list(resolved_infoset.actions)
        remap = _compute_relabeling(
            current, labels, "relabel_actions", "action", strict,
            "at this information set"
        )
        if not remap:
            return
        c_labels = stdmap[string, string]()
        for old, new in remap.items():
            c_labels[old.encode("utf-8")] = new.encode("utf-8")
        self.game.deref().RelabelActions(resolved_infoset._infoset_handle(), c_labels)

    def make_infoset(self,
                     nodes: Selector | GroupedSelector,
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

        `nodes` is a `Selector` (an `H`-built expression, evaluated against this game
        and treated as a flat set of nodes) or a `GroupedSelector` (an `H`-built
        `.by(...)` expression, whose groups are pooled together into the one
        information set).

        .. versionadded:: 17.0.0
        .. versionchanged:: 17.0.0
            `nodes` is now a `Selector` or `GroupedSelector`; a `Node` or
            `NodeReferenceSet` is no longer accepted directly -- build one with `H`.

        Parameters
        ----------
        nodes : Selector or GroupedSelector
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
        TypeError
            If `nodes` is not a `Selector` or `GroupedSelector`, or `player` is not
            of an accepted type.
        KeyError
            If `player` is a label matching no such object in the game.
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
        if isinstance(nodes, GroupedSelector):
            nodes = [n for group in self._group_nodes(nodes).values() for n in group]
        elif not isinstance(nodes, Selector):
            raise TypeError(
                f"make_infoset(): nodes must be a Selector or GroupedSelector, "
                f"not {nodes.__class__.__name__}"
            )
        resolved_nodes = self._resolve_nodes(nodes, "make_infoset")
        resolved_player = self._resolve_player(player, "make_infoset")
        for n in resolved_nodes:
            if n.is_terminal:
                raise UndefinedOperationError(
                    "make_infoset(): all nodes must be decision nodes"
                )
        c_nodes = stdvector[c_GameNode]()
        for n in resolved_nodes:
            c_nodes.push_back(cython.cast(Node, n).node)
        self.game.deref().MakeInfoset(c_nodes, resolved_player, (label or "").encode())

    def set_players(self,
                    players: list[str],
                    drop: bool = False,
                    add: bool = True) -> None:
        """Set the players of the game to be `players`, matching by label.

        An entry of `players` matching the label of a current player refers to that
        player, which keeps its moves or strategies and its payoffs at every outcome;
        an entry matching no current player creates a new player there, with no
        decisions in an extensive game, or a single strategy labeled ``"1"`` in a
        strategic game.  A current player whose label is not in `players` is deleted.
        Listing the current labels in a new order reorders the players.

        A player can only be deleted if it has no decisions in the game (in an
        extensive game) or exactly one strategy (in a strategic game); otherwise the
        operation raises.

        The defaults permit creation and forbid deletion: adding a player -- inserting
        its label into the current list -- is the common, non-destructive edit, while
        deletion discards the player's payoffs at every outcome, so it must be
        confirmed.

        .. versionadded:: 17.0.0
            Subsumes and replaces `Game.add_player`.

        Parameters
        ----------
        players : list of str
            The labels of the players the game is to have, in order.  Must be nonempty
            and without duplicates; each label must be a valid, nonempty label, and in
            an extensive game must not be the reserved chance player label.
        drop : bool, default False
            Deleting players is destructive, so it must be explicitly confirmed: if any
            current player is missing from `players` and `drop` is `False`, the
            operation raises without modifying the game.
        add : bool, default True
            If `False`, entries of `players` matching no current player raise.

        Raises
        ------
        TypeError
            If `players` is a string, or not an iterable of strings.
        UndefinedOperationError
            If `players` is empty; or if a player to be deleted has decisions in the
            game, or more than one strategy.
        ValueError
            If a label in `players` is repeated, empty, is not a valid label, or (in an
            extensive game) is the reserved label of the chance player.
        """
        if isinstance(players, str) or not hasattr(players, "__iter__"):
            raise TypeError("set_players(): players must be an iterable of str")
        labels = list(players)
        for label in labels:
            if not isinstance(label, str):
                raise TypeError("set_players(): players must be an iterable of str")
        if not labels:
            raise UndefinedOperationError("set_players(): `players` must be a nonempty list")
        current = list(self.players)
        _, missing = _reconcile_labels(
            current, labels, add, drop, "set_players",
            "game", "player", "players", "their payoffs at every outcome"
        )
        for label in missing:
            if self.is_tree and len(self.get_infosets(label)) > 0:
                raise UndefinedOperationError(
                    f"set_players(): player '{label}' has decisions in the game "
                    f"and cannot be deleted"
                )
            if not self.is_tree and len(self.get_strategies(label)) != 1:
                raise UndefinedOperationError(
                    f"set_players(): player '{label}' has more than one strategy "
                    f"and cannot be deleted"
                )
        c_labels = stdvector[string]()
        for label in labels:
            c_labels.push_back(label.encode("utf-8"))
        self.game.deref().SetPlayers(c_labels)

    def _resolve_outcome_location(self, location, funcname: str) -> tuple:
        """Resolve `location` for `make_outcome`/`make_outcome_null`: for a tree game,
        into a list of `Node` (via `_resolve_nodes`, so `location` must be a
        `Selector` or `GroupedSelector`); for a strategic game, into a list of
        pure-strategy contingencies (each a mapping from player label to strategy
        label).

        Returns (is_tree, resolved).

        Raises
        ------
        TypeError
            If `location` is not a `Selector` or `GroupedSelector` (tree game
            only); or is not a contingency or an iterable of contingencies
            (strategic game only).
        ValueError
            If `location` is empty or contains a repeat, or (strategic game only) if
            a contingency does not specify exactly one strategy for each player.
        """
        if self.is_tree:
            if isinstance(location, GroupedSelector):
                location = [n for group in self._group_nodes(location).values() for n in group]
            elif not isinstance(location, Selector):
                raise TypeError(
                    f"{funcname}(): location must be a Selector or GroupedSelector, "
                    f"not {location.__class__.__name__}"
                )
            return True, self._resolve_nodes(location, funcname)
        if isinstance(location, collections.abc.Mapping):
            entries = [location]
        else:
            try:
                entries = list(location)
            except TypeError:
                raise TypeError(
                    f"{funcname}(): location must be a contingency or an "
                    f"iterable of contingencies"
                ) from None
        return False, [
            self._resolve_contingency(entry, funcname, "location") for entry in entries
        ]

    def make_outcome(self,
                     location,
                     payoffs: typing.Mapping,
                     label: str) -> Outcome:
        """Create an outcome with `payoffs` and `label` and attach it at `location`.

        For an extensive game, `location` is a `Selector` (an `H`-built
        expression, evaluated against this game and treated as a flat set of
        nodes) or a `GroupedSelector` (an `H`-built `.by(...)` expression, whose
        groups are pooled together, all receiving the same outcome).  For a
        strategic game, `location` is a pure-strategy contingency — a complete
        mapping from the game's players' labels to strategy labels — or an
        iterable of such contingencies.

        Any outcome all of whose references are among `location` is absorbed by the
        operation: it is removed from the game, and `label` may reuse its label.

        .. versionadded:: 17.0.0
        .. versionchanged:: 17.0.0
            For an extensive game, `location` is now a `Selector` or
            `GroupedSelector`; a `Node`, `History`, or iterable of these is no
            longer accepted directly -- build one with `H`.

        Parameters
        ----------
        location : Selector, GroupedSelector, contingency, or iterable of contingencies
            Where to attach the new outcome.  Nonempty; each node or contingency may
            be referenced only once.
        payoffs : Mapping
            A complete mapping from the game's players (or their labels) to payoffs.
            Every player must be present; zeroes must be given explicitly.
        label : str
            The label of the new outcome; must be nonempty and, after the operation,
            unique within the game.

        Returns
        -------
        Outcome
            A reference to the newly-created outcome.

        Raises
        ------
        TypeError
            If, for an extensive game, `location` is not a `Selector` or
            `GroupedSelector`.
        ValueError
            If `location` is empty or contains a repeat; if `payoffs` is not a complete
            mapping over exactly the game's players; if a contingency does not specify
            exactly one strategy for each player; or if `label` is empty or is held
            by an outcome that is not absorbed by the operation.
        UndefinedOperationError
            If the game is in action-graph representation, where outcomes are not
            represented explicitly.
        """
        if self.game.deref().IsAgg():
            raise UndefinedOperationError(
                "make_outcome(): operation not defined for games in action-graph representation"
            )
        if not hasattr(payoffs, "items"):
            raise TypeError(
                f"make_outcome(): payoffs must be a mapping, not {payoffs.__class__.__name__}"
            )
        resolved_payoffs = {}
        for player, value in payoffs.items():
            self._resolve_player(player, "make_outcome", "payoffs")
            if player in resolved_payoffs:
                raise ValueError("make_outcome(): each player may appear only once in payoffs")
            resolved_payoffs[player] = value
        if set(resolved_payoffs) != set(self.players):
            raise ValueError(
                "make_outcome(): payoffs must be specified for each player of the game"
            )
        c_payoffs = stdvector[c_Number]()
        for player in self.players:
            c_payoffs.push_back(_to_number(resolved_payoffs[player]))
        is_tree, resolved = self._resolve_outcome_location(location, "make_outcome")
        if is_tree:
            c_nodes = stdvector[c_GameNode]()
            for n in resolved:
                c_nodes.push_back(cython.cast(Node, n).node)
            return Outcome.wrap(
                self.game.deref().MakeOutcome(c_nodes, c_payoffs, label.encode("utf-8"))
            )
        c_contingencies = stdvector[stdvector[c_GameStrategy]]()
        for contingency in resolved:
            c_one = stdvector[c_GameStrategy]()
            for player in self.players:
                c_one.push_back(
                    self._resolve_strategy(player, contingency[player], "make_outcome")
                )
            c_contingencies.push_back(c_one)
        return Outcome.wrap(
            self.game.deref().MakeOutcome(c_contingencies, c_payoffs, label.encode("utf-8"))
        )

    def make_outcome_null(self, location) -> None:
        """Reset the outcome at `location` to the null outcome.

        For an extensive game, `location` is a `Selector` (an `H`-built
        expression, evaluated against this game and treated as a flat set of
        nodes) or a `GroupedSelector` (an `H`-built `.by(...)` expression, whose
        groups are pooled together).  For a strategic game, `location` is a
        pure-strategy contingency — a complete mapping from the game's players'
        labels to strategy labels — or an iterable of such contingencies.

        Any outcome all of whose references are among `location` is removed from the game.

        .. versionadded:: 17.0.0
        .. versionchanged:: 17.0.0
            For an extensive game, `location` is now a `Selector` or
            `GroupedSelector`; a `Node`, `History`, or iterable of these is no
            longer accepted directly -- build one with `H`.

        Parameters
        ----------
        location : Selector, GroupedSelector, contingency, or iterable of contingencies
            The nodes or contingencies to reset to the null outcome.  Nonempty; each
            node or contingency may be referenced only once.

        Raises
        ------
        TypeError
            If, for an extensive game, `location` is not a `Selector` or
            `GroupedSelector`.
        ValueError
            If `location` is empty or contains a repeat, or if a contingency does not
            specify exactly one strategy for each player.
        UndefinedOperationError
            If the game is in action-graph representation, where outcomes are not
            represented explicitly.
        """
        if self.game.deref().IsAgg():
            raise UndefinedOperationError(
                "make_outcome_null(): operation not defined for games in "
                "action-graph representation"
            )
        is_tree, resolved = self._resolve_outcome_location(location, "make_outcome_null")
        if is_tree:
            c_nodes = stdvector[c_GameNode]()
            for n in resolved:
                c_nodes.push_back(cython.cast(Node, n).node)
            self.game.deref().MakeOutcomeNull(c_nodes)
            return
        c_contingencies = stdvector[stdvector[c_GameStrategy]]()
        for contingency in resolved:
            c_one = stdvector[c_GameStrategy]()
            for player in self.players:
                c_one.push_back(
                    self._resolve_strategy(player, contingency[player], "make_outcome_null")
                )
            c_contingencies.push_back(c_one)
        self.game.deref().MakeOutcomeNull(c_contingencies)

    def relabel_strategies(self,
                           player: str,
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
        player : str
            The label of the player whose strategies to relabel.
        labels : Mapping[str, str]
            A mapping from current strategy labels to replacement labels.  Entries
            whose key equals their value are ignored.
        strict : bool, default True
            If `True`, every key of `labels` must be the label of a strategy of
            `player`, and unknown keys raise ``KeyError``.  If `False`, unknown keys
            are ignored.

        Raises
        ------
        KeyError
            If no player in the game has label `player`; or, when `strict` is `True`,
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
        resolved_player = self._resolve_player(player, "relabel_strategies")
        if not hasattr(labels, "items"):
            raise TypeError(
                f"relabel_strategies(): labels must be a mapping, "
                f"not {labels.__class__.__name__}"
            )
        current = [
            s.deref().GetLabel().decode("utf-8") for s in resolved_player.deref().GetStrategies()
        ]
        remap = _compute_relabeling(
            current, labels, "relabel_strategies", "strategy", strict,
            "for this player"
        )
        if not remap:
            return
        c_labels = stdmap[string, string]()
        for old, new in remap.items():
            c_labels[old.encode("utf-8")] = new.encode("utf-8")
        self.game.deref().RelabelStrategies(resolved_player, c_labels)

    def set_strategies(self,
                       player: str,
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
        player : str
            The label of the player whose strategies to set.
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
        KeyError
            If no player in the game has label `player`.
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
        resolved_player = self._resolve_player(player, "set_strategies")
        if isinstance(strategies, str) or not hasattr(strategies, "__iter__"):
            raise TypeError("set_strategies(): strategies must be an iterable of str")
        labels = list(strategies)
        for label in labels:
            if not isinstance(label, str):
                raise TypeError("set_strategies(): strategies must be an iterable of str")
        if not labels:
            raise UndefinedOperationError("set_strategies(): `strategies` must be a nonempty list")
        current = [
            s.deref().GetLabel().decode("utf-8") for s in resolved_player.deref().GetStrategies()
        ]
        _reconcile_labels(
            current, labels, add, drop, "set_strategies",
            "player", "strategy", "strategies", "the outcomes at their contingencies"
        )
        c_labels = stdvector[string]()
        for label in labels:
            c_labels.push_back(label.encode("utf-8"))
        self.game.deref().SetStrategies(resolved_player, c_labels)

    def relabel_players(self,
                        labels: typing.Mapping[str, str],
                        strict: bool = True) -> None:
        """Simultaneously reassign the labels of the game's players.

        `labels` maps current player labels to their replacements.  The reassignment
        is simultaneous, so labels can be swapped directly. Players are not re-ordered: each
        relabelled player keeps its position.

        The chance player is not part of the operation: its label is reserved, and a
        key of `labels` equal to it raises ``ValueError`` even when `strict` is `False`.

        .. versionadded:: 17.0.0

        Parameters
        ----------
        labels : Mapping[str, str]
            A mapping from current player labels to replacement labels.
            Entries whose key equals their value are ignored.
        strict : bool, default True
            If `True`, every key of `labels` must be the label of a player of the game,
            and unknown keys raise ``KeyError``.  If `False`, unknown keys are ignored.

        Raises
        ------
        KeyError
            When `strict` is `True`, if a key of `labels` matches no player of the game.
        TypeError
            If `labels` is not a mapping, or any key or value is not a string.
        ValueError
            If a key of `labels` matches more than one player; if a key of `labels`
            is the label of the chance player; or if any replacement label is empty,
            is not a valid label, would result in a duplicate label, or (in an
            extensive game) is the reserved label of the chance player.

        See Also
        --------
        relabel_actions : Change the labels of actions at an information set.
        relabel_strategies : Change the labels of a player's strategies.
        """
        if not hasattr(labels, "items"):
            raise TypeError(
                f"relabel_players(): labels must be a mapping, "
                f"not {labels.__class__.__name__}"
            )
        current = list(self.players)
        chance_label = (
            self.game.deref().GetChance().deref().GetLabel().decode("utf-8")
            if self.is_tree else None
        )
        remap = _compute_relabeling(
            current, labels, "relabel_players", "player", strict,
            "in this game", reserved=chance_label,
            reserved_desc="the chance player's label is reserved"
        )
        if not remap:
            return
        c_labels = stdmap[string, string]()
        for old, new in remap.items():
            c_labels[old.encode("utf-8")] = new.encode("utf-8")
        self.game.deref().RelabelPlayers(c_labels)


@dataclasses.dataclass
class NodeCoordinates:
    level: int
    sublevel: int
    offset: float


@cython.cfunc
def _layout_tree(game: Game) -> dict[Node, NodeCoordinates]:
    layout = CreateLayout(game.game)
    data = {}
    for node in game.nodes:
        data[node] = NodeCoordinates(deref(layout).GetNodeLevel(cython.cast(Node, node).node),
                                     deref(layout).GetNodeSublevel(cython.cast(Node, node).node),
                                     deref(layout).GetNodeOffset(cython.cast(Node, node).node))
    return data


def layout_tree(game: Game) -> dict[Node, NodeCoordinates]:
    return _layout_tree(game)
