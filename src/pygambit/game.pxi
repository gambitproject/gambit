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

    def __init__(self) -> None:
        raise ValueError("Use ExtensiveGame(...) or StrategicGame(...) to create a new game")

    @staticmethod
    @cython.cfunc
    def _wrap(game: c_Game) -> Game:
        cls: type = Game
        if As[c_GameTreeRep](game).get() != NULL:
            cls = ExtensiveGame
        elif As[c_GameTableRep](game).get() != NULL:
            cls = StrategicGame
        elif As[c_GameAGGRep](game).get() != NULL:
            cls = ActionGraphGame
        elif As[c_GameBAGGRep](game).get() != NULL:
            cls = BayesianActionGraphGame
        obj: Game = cls.__new__(cls)
        obj.game = game
        return obj

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
        StrategicGame.from_arrays : Create game from list-like of array-like
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

    def __repr__(self) -> str:
        if self.title:
            return f"Game(title='{self.title}')"
        else:
            return f"Game(id={hash(self)}"

    def _repr_html_(self):
        if isinstance(self, ExtensiveGame):
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

    def get_outcomes(self) -> list[str]:
        """Returns the labels of the outcomes in the game.

        .. versionadded:: 17.0.0
        """
        return [
            o.deref().GetLabel().decode("utf-8") for o in self.game.deref().GetOutcomes()
        ]

    @property
    def contingencies(self) -> pygambit.gameiter.Contingencies:
        """An iterator over the contingencies in the game."""
        return pygambit.gameiter.Contingencies(self)

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

    @cython.cfunc
    def _resolve_outcome(
        self, label: typing.Any, funcname: str, argname: str = "label"
    ) -> c_GameOutcome:
        """Resolve `label` to the C++ handle of one of the game's outcomes.

        Not part of the public API -- used internally to bridge an outcome label to
        the underlying C++ object without ever constructing a Python wrapper for it.

        Raises
        ------
        KeyError
            If no outcome has label `label`.
        TypeError
            If `label` is not a `str`.
        ValueError
            If `label` is an empty string or all spaces.
        """
        if not isinstance(label, str):
            raise TypeError(
                f"{funcname}(): {argname} must be str, not {label.__class__.__name__}"
            )
        if not label.strip():
            raise ValueError(f"{funcname}(): {argname} cannot be an empty string or all spaces")
        for outcome in self.game.deref().GetOutcomes():
            if outcome.deref().GetLabel().decode("utf-8") == label:
                return outcome
        raise KeyError(f"{funcname}(): no outcome with label '{label}'")

    @cython.cfunc
    def _validate_player_labels(self, players, funcname: str) -> list:
        """Validate `players` as a nonempty iterable of `str`, for `set_players`.
        Not part of the public API; shared by `ExtensiveGame.set_players` and
        `StrategicGame.set_players`.

        Raises
        ------
        TypeError
            If `players` is a string, or not an iterable of strings.
        UndefinedOperationError
            If `players` is empty.
        """
        if isinstance(players, str) or not hasattr(players, "__iter__"):
            raise TypeError(f"{funcname}(): players must be an iterable of str")
        labels = list(players)
        for label in labels:
            if not isinstance(label, str):
                raise TypeError(f"{funcname}(): players must be an iterable of str")
        if not labels:
            raise UndefinedOperationError(f"{funcname}(): `players` must be a nonempty list")
        return labels

    @cython.cfunc
    def _apply_set_players(self, labels: list) -> cython.void:
        """Reconcile the game's current players against `labels` (already validated
        and confirmed deletable by the caller) and apply the change. Not part of the
        public API; shared by `ExtensiveGame.set_players` and `StrategicGame.set_players`.
        """
        c_labels = stdvector[string]()
        for label in labels:
            c_labels.push_back(label.encode("utf-8"))
        self.game.deref().SetPlayers(c_labels)

    @cython.cfunc
    def _resolve_payoff_mapping(self, payoffs: typing.Mapping, funcname: str) -> dict:
        """Validate `payoffs` as a complete mapping from the game's players to payoff
        values: every player of the game must appear exactly once. Not part of the
        public API; shared by `make_outcome` and `set_outcome_payoffs`.

        Raises
        ------
        TypeError
            If `payoffs` is not a mapping.
        KeyError
            If a key of `payoffs` matches no player of the game.
        ValueError
            If a player appears more than once in `payoffs`, or `payoffs` does not
            specify exactly one value for each player of the game.
        """
        if not hasattr(payoffs, "items"):
            raise TypeError(
                f"{funcname}(): payoffs must be a mapping, not {payoffs.__class__.__name__}"
            )
        resolved_payoffs = {}
        for player, value in payoffs.items():
            self._resolve_player(player, funcname, "payoffs")
            if player in resolved_payoffs:
                raise ValueError(f"{funcname}(): each player may appear only once in payoffs")
            resolved_payoffs[player] = value
        if set(resolved_payoffs) != set(self.players):
            raise ValueError(
                f"{funcname}(): payoffs must be specified for each player of the game"
            )
        return resolved_payoffs

    @cython.cfunc
    def _do_relabel_outcomes(self, labels: typing.Mapping[str, str], strict: bool) -> cython.void:
        """Simultaneously reassign the labels of the game's outcomes. Not part of the
        public API; shared by `ExtensiveGame.relabel_outcomes` and
        `StrategicGame.relabel_outcomes`, whose docstrings document this behavior.
        """
        if not hasattr(labels, "items"):
            raise TypeError(
                f"relabel_outcomes(): labels must be a mapping, "
                f"not {labels.__class__.__name__}"
            )
        current = [
            o.deref().GetLabel().decode("utf-8") for o in self.game.deref().GetOutcomes()
        ]
        remap = _compute_relabeling(
            current, labels, "relabel_outcomes", "outcome", strict, "in this game"
        )
        if not remap:
            return
        c_labels = stdmap[string, string]()
        for old, new in remap.items():
            c_labels[old.encode("utf-8")] = new.encode("utf-8")
        self.game.deref().RelabelOutcomes(c_labels)

    def get_outcome_payoffs(self, label: str) -> PayoffVector:
        """Returns the payoff to each player at the outcome labeled `label`.

        .. versionadded:: 17.0.0

        Parameters
        ----------
        label : str
            The label of the outcome.

        Returns
        -------
        PayoffVector

        Raises
        ------
        KeyError
            If no outcome has label `label`.
        TypeError
            If `label` is not a `str`.
        ValueError
            If `label` is an empty string or all whitespace.

        See Also
        --------
        set_outcome_payoffs : Set the payoffs at an outcome.
        get_payoffs : Get the payoffs at a pure-strategy contingency.
        """
        resolved_outcome: c_GameOutcome = self._resolve_outcome(label, "get_outcome_payoffs")
        values = {}
        for player in self.players:
            resolved_player = self._resolve_player(player, "get_outcome_payoffs")
            values[player] = _decode_number(resolved_outcome.deref().GetPayoff[string](
                resolved_player
            ))
        return PayoffVector(values)

    def set_outcome_payoffs(self, label: str, payoffs: typing.Mapping) -> None:
        """Sets the payoff to each player at the outcome labeled `label`.

        .. versionadded:: 17.0.0

        Parameters
        ----------
        label : str
            The label of the outcome to modify.
        payoffs : Mapping
            A complete mapping from the game's players (or their labels) to payoffs.
            Every player must be present; zeroes must be given explicitly.

        Raises
        ------
        KeyError
            If no outcome has label `label`; or if a key of `payoffs` matches no player.
        TypeError
            If `payoffs` is not a mapping, or `label` is not a `str`.
        ValueError
            If `label` is an empty string or all whitespace; or if `payoffs` is not a
            complete mapping over exactly the game's players.

        See Also
        --------
        get_outcome_payoffs : Get the payoffs at an outcome.
        make_outcome : Create a new outcome with payoffs.
        """
        resolved_outcome: c_GameOutcome = self._resolve_outcome(label, "set_outcome_payoffs")
        resolved_payoffs = self._resolve_payoff_mapping(payoffs, "set_outcome_payoffs")
        for player in self.players:
            resolved_outcome.deref().SetPayoff(
                self._resolve_player(player, "set_outcome_payoffs"),
                _to_number(resolved_payoffs[player])
            )

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
            if isinstance(self, ExtensiveGame) else None
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
