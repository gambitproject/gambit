#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/gametable.pxi
# Cython wrapper for the strategic (payoff table) representation
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
import numpy as np


@cython.cclass
class StrategicGame(Game):
    """A `Game` with a strategic (payoff table) representation.

    Constructing ``StrategicGame(dim, title)`` creates a new game with a strategic
    representation. Players are labeled ``"1"``, ``"2"``, and so on; each player's
    strategies are likewise labeled ``"1"``, ``"2"``, and so on.

    .. versionchanged:: 16.1.0
        Added the `title` parameter.
    .. versionchanged:: 17.0.0
        Replaces `Game.new_table`.

    Parameters
    ----------
    dim : array-like
        A list specifying the number of strategies for each player.
    title : str, optional
        The title of the game.  If no title is specified, "Untitled strategic game"
        is used.
    """

    def __init__(self, dim, title: str = "Untitled strategic game") -> None:
        self.game = NewTable(list(dim), True)
        self.title = title

    @classmethod
    def from_arrays(cls, *arrays, title: str = "Untitled strategic game") -> StrategicGame:
        """Create a new strategic game from payoff arrays.

        Each entry in `arrays` gives the payoff matrix for the
        corresponding player.  The arrays must all have the same shape,
        and have the same number of dimensions as the total number of
        players.

        Players are labeled ``"1"``, ``"2"``, and so on;
        each player's strategies are likewise labeled ``"1"``, ``"2"``, and so on.

        .. versionchanged:: 16.1.0
            Added the `title` parameter.
        .. versionchanged:: 17.0.0
            Replaces `Game.from_arrays`.

        Parameters
        ----------
        arrays : array-like of array-like
            The payoff matrices for the players.
        title : str, optional
            The title of the game.  If no title is specified, "Untitled strategic game"
            is used.

        Returns
        -------
        StrategicGame
            The newly-created strategic game.

        See Also
        --------
        from_dict : Create strategic game and set player labels
        Game.to_arrays: Generate the payoff tables for players represented as numpy arrays
        """
        arrays = [np.array(a) for a in arrays]
        if len(set(a.shape for a in arrays)) > 1:
            raise ValueError("All specified arrays must have the same shape")
        shape = arrays[0].shape
        g: StrategicGame = StrategicGame.__new__(StrategicGame)
        g.game = NewTable(list(shape), False)
        players = list(g.players)
        for profile in itertools.product(*(range(s) for s in shape)):
            contingency = {p: str(i + 1) for p, i in zip(players, profile, strict=True)}
            resolved_outcome = g._get_contingency_outcome(contingency, "from_arrays")
            for array, player in zip(arrays, players, strict=True):
                resolved_outcome.deref().SetPayoff(
                    g._resolve_player(player, "from_arrays"), _to_number(array[profile])
                )
        g.title = title
        return g

    @classmethod
    def from_dict(cls, payoffs, title: str = "Untitled strategic game") -> StrategicGame:
        """Create a new strategic game from a dict of payoff arrays.

        Each entry in `payoffs` is a key-value pair
        giving the label and the payoff matrix for a player.
        The payoff matrices must all have the same shape,
        and have the same number of dimensions as the total number of
        players.

        The players are labeled with the keys of `payoffs`, and therefore
        must be valid player labels.  Each player's strategies are labeled
        ``"1"``, ``"2"``, and so on.

        .. versionchanged:: 17.0.0
            Replaces `Game.from_dict`.

        Parameters
        ----------
        payoffs : dict-like mapping str to array-like
            The names and corresponding payoff matrices for the players.
        title : str, optional
            The title of the game.  If no title is specified, "Untitled strategic game"
            is used.

        Returns
        -------
        StrategicGame
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
        g: StrategicGame = StrategicGame.__new__(StrategicGame)
        g.game = NewTable(list(shape), False)
        g.relabel_players(
            {player: label for player, label in zip(g.players, payoffs, strict=True)}
        )
        players = list(g.players)
        for profile in itertools.product(*(range(s) for s in shape)):
            contingency = {p: str(i + 1) for p, i in zip(players, profile, strict=True)}
            resolved_outcome = g._get_contingency_outcome(contingency, "from_dict")
            for array, player in zip(arrays, players, strict=True):
                resolved_outcome.deref().SetPayoff(
                    g._resolve_player(player, "from_dict"), _to_number(array[profile])
                )
        g.title = title
        return g

    @cython.cfunc
    def _get_contingency_outcome(
        self, contingency: typing.Mapping, funcname: str
    ) -> c_GameOutcome:
        """Resolve the outcome attached at a pure-strategy `contingency` in a
        strategic (table) game, as a raw C++ handle. Not part of the public API;
        used internally by `get_outcome`, `from_arrays`, and `from_dict`.
        """
        resolved = self._resolve_contingency(contingency, funcname)
        psp = self._make_pure_strategy_profile(resolved)
        return deref(deref(psp).deref()).GetOutcome()

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

    def get_outcome(self, location: typing.Mapping) -> str | None:
        """Returns the label of the outcome attached at `location`.

        `location` is a pure-strategy contingency -- a complete mapping from the
        game's players' labels to the label of the strategy played by that player.

        .. versionadded:: 17.0.0

        Parameters
        ----------
        location : Mapping
            A pure-strategy contingency.

        Returns
        -------
        str or None
            The label of the outcome attached at `location`, or `None` if it
            is the null outcome.

        Raises
        ------
        TypeError
            If `location` is not a mapping, or a key or value is not a `str`.
        ValueError
            If `location` does not specify exactly one strategy for each player of
            the game, or a key is an empty or all-whitespace string.
        KeyError
            If a player label, or a player's strategy label, does not match any
            player, or that player's strategies, in the game.
        """
        resolved_outcome = self._get_contingency_outcome(location, "get_outcome")
        if resolved_outcome.deref().IsNull():
            return None
        return resolved_outcome.deref().GetLabel().decode("utf-8")

    @cython.cfunc
    def _resolve_outcome_contingencies(self, location, funcname: str) -> list:
        """Resolve `location` for `make_outcome`/`make_outcome_null` into a list of
        pure-strategy contingencies (each a mapping from player label to strategy
        label). Not part of the public API.

        Raises
        ------
        TypeError
            If `location` is not a contingency or an iterable of contingencies.
        ValueError
            If a contingency does not specify exactly one strategy for each player.
        """
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
        return [self._resolve_contingency(entry, funcname, "location") for entry in entries]

    def make_outcome(self,
                     location: typing.Mapping | typing.Iterable[typing.Mapping],
                     payoffs: typing.Mapping,
                     label: str) -> None:
        """Create an outcome with `payoffs` and `label` and attach it at `location`.

        `location` is a pure-strategy contingency -- a complete mapping from the
        game's players' labels to strategy labels -- or an iterable of such
        contingencies.

        Any outcome all of whose references are among `location` is absorbed by the
        operation: it is removed from the game, and `label` may reuse its label.

        .. versionadded:: 17.0.0

        Parameters
        ----------
        location : contingency, or iterable of contingencies
            Where to attach the new outcome.  Nonempty; each contingency may be
            referenced only once.
        payoffs : Mapping
            A complete mapping from the game's players (or their labels) to payoffs.
            Every player must be present; zeroes must be given explicitly.
        label : str
            The label of the new outcome; must be nonempty and, after the operation,
            unique within the game.

        Raises
        ------
        TypeError
            If `location` is not a contingency or an iterable of contingencies.
        ValueError
            If `location` is empty or contains a repeat; if `payoffs` is not a complete
            mapping over exactly the game's players; if a contingency does not specify
            exactly one strategy for each player; or if `label` is empty or is held
            by an outcome that is not absorbed by the operation.

        See Also
        --------
        get_outcome_payoffs : Get the payoffs at an outcome.
        set_outcome_payoffs : Set the payoffs at an outcome.
        relabel_outcomes : Change the labels of the game's outcomes.
        """
        resolved_payoffs = self._resolve_payoff_mapping(payoffs, "make_outcome")
        c_payoffs = stdvector[c_Number]()
        for player in self.players:
            c_payoffs.push_back(_to_number(resolved_payoffs[player]))
        resolved = self._resolve_outcome_contingencies(location, "make_outcome")
        c_contingencies = stdvector[stdvector[c_GameStrategy]]()
        for contingency in resolved:
            c_one = stdvector[c_GameStrategy]()
            for player in self.players:
                c_one.push_back(
                    self._resolve_strategy(player, contingency[player], "make_outcome")
                )
            c_contingencies.push_back(c_one)
        self.game.deref().MakeOutcome(c_contingencies, c_payoffs, label.encode("utf-8"))

    def make_outcome_null(self,
                          location: typing.Mapping | typing.Iterable[typing.Mapping]) -> None:
        """Reset the outcome at `location` to the null outcome.

        `location` is a pure-strategy contingency -- a complete mapping from the
        game's players' labels to strategy labels -- or an iterable of such
        contingencies.

        Any outcome all of whose references are among `location` is removed from the game.

        .. versionadded:: 17.0.0

        Parameters
        ----------
        location : contingency, or iterable of contingencies
            The contingencies to reset to the null outcome.  Nonempty; each
            contingency may be referenced only once.

        Raises
        ------
        TypeError
            If `location` is not a contingency or an iterable of contingencies.
        ValueError
            If `location` is empty or contains a repeat, or if a contingency does not
            specify exactly one strategy for each player.
        """
        resolved = self._resolve_outcome_contingencies(location, "make_outcome_null")
        c_contingencies = stdvector[stdvector[c_GameStrategy]]()
        for contingency in resolved:
            c_one = stdvector[c_GameStrategy]()
            for player in self.players:
                c_one.push_back(
                    self._resolve_strategy(player, contingency[player], "make_outcome_null")
                )
            c_contingencies.push_back(c_one)
        self.game.deref().MakeOutcomeNull(c_contingencies)

    def relabel_outcomes(self, labels: typing.Mapping[str, str], strict: bool = True) -> None:
        """Simultaneously reassign the labels of the game's outcomes.

        `labels` maps current outcome labels to their replacements.  The reassignment
        is simultaneous, so labels can be swapped directly, e.g. ``{"a": "b", "b": "a"}``.

        .. versionadded:: 17.0.0

        Parameters
        ----------
        labels : Mapping[str, str]
            A mapping from current outcome labels to replacement labels.
            Entries whose key equals their value are ignored.
        strict : bool, default True
            If `True`, every key of `labels` must be the label of an outcome of the
            game, and unknown keys raise ``KeyError``.  If `False`, unknown keys are
            ignored.

        Raises
        ------
        KeyError
            When `strict` is `True`, if a key of `labels` matches no outcome of the game.
        TypeError
            If `labels` is not a mapping, or any key or value is not a string.
        ValueError
            If a key of `labels` matches more than one outcome; or if any replacement
            label is empty, is not a valid label, or would result in a duplicate label.

        See Also
        --------
        relabel_players : Simultaneously reassign the labels of the game's players.
        relabel_strategies : Change the labels of a player's strategies.
        """
        self._do_relabel_outcomes(labels, strict)

    def set_players(self,
                    players: list[str],
                    drop: bool = False,
                    add: bool = True) -> None:
        """Set the players of the game to be `players`, matching by label.

        An entry of `players` matching the label of a current player refers to that
        player, which keeps its strategies and its payoffs at every outcome; an entry
        matching no current player creates a new player there, with a single strategy
        labeled ``"1"``.  A current player whose label is not in `players` is deleted.
        Listing the current labels in a new order reorders the players.

        A player can only be deleted if it has exactly one strategy; otherwise the
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
            and without duplicates; each label must be a valid, nonempty label.
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
            If `players` is empty; or if a player to be deleted has more than one
            strategy.
        ValueError
            If a label in `players` is repeated, empty, or is not a valid label.
        """
        labels = self._validate_player_labels(players, "set_players")
        current = list(self.players)
        _, missing = _reconcile_labels(
            current, labels, add, drop, "set_players",
            "game", "player", "players", "their payoffs at every outcome"
        )
        for label in missing:
            if len(self.get_strategies(label)) != 1:
                raise UndefinedOperationError(
                    f"set_players(): player '{label}' has more than one strategy "
                    f"and cannot be deleted"
                )
        self._apply_set_players(labels)
