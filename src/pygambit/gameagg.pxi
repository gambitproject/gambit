#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/gameagg.pxi
# Cython wrapper for the action-graph game representations
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

@cython.cclass
class ActionGraphGame(Game):
    """A `Game` with an action-graph game representation."""


@cython.cclass
class BAGGPlayers:
    """The labels of the players of a Bayesian action-graph game.

    .. versionadded:: 17.0.0
    """
    game = cython.declare(c_Game)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create BAGGPlayers outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(game: c_Game) -> BAGGPlayers:
        obj: BAGGPlayers = BAGGPlayers.__new__(BAGGPlayers)
        obj.game = game
        return obj

    def __repr__(self) -> str:
        return f"BAGGPlayers(game={Game._wrap(self.game)})"

    def __len__(self) -> int:
        return As[c_GameBAGGRep](self.game).get().NumBaggPlayers()

    def __iter__(self) -> typing.Iterator[str]:
        for i in range(len(self)):
            yield str(i + 1)

    def __contains__(self, label: str) -> bool:
        return isinstance(label, str) and label in set(self)


def _unsupported(*args, **kwargs):
    """Not supported for Bayesian games."""
    raise UnsupportedForBayesianGameError("not supported for Bayesian games")


@cython.cclass
class BayesianActionGraphGame(Game):
    """A `Game` with a Bayesian action-graph game representation.

    The game is presented as what it is: a simultaneous-move Bayesian game.  Its players
    are the Bayesian players, and each player's types are presented as information sets,
    labelled by position.  Members of `Game` that concern strategies have no meaning for a
    Bayesian game and raise `UnsupportedForBayesianGameError`.

    .. versionchanged:: 17.0.0
        Players are the Bayesian players rather than one player per type, and members
        that concern strategies raise `UnsupportedForBayesianGameError`.
    """

    @cython.cfunc
    def _agent_player(self, player: str, infoset: str) -> c_GamePlayer:
        """The one place that resolves public labels to the underlying agent player.

        Agent labels overlap public player labels, so no other code may look up a player
        of the underlying game by label.

        Raises
        ------
        KeyError
            If `player` or `infoset` is not a label of the game.
        """
        bagg: shared_ptr[c_GameBAGGRep] = As[c_GameBAGGRep](self.game)
        return self.game.deref().GetPlayer(
            bagg.get().GetAgentNumber(
                self._player_position(player), self._type_position(player, infoset)
            )
        )

    @cython.cfunc
    def _type_position(self, player: str, infoset: str) -> cython.int:
        """The 1-based position among `player`'s types of its information set `infoset`.

        Raises
        ------
        KeyError
            If `player` is not a label of a player, or has no information set `infoset`.
        TypeError
            If `infoset` is not a `str`.
        """
        if not isinstance(infoset, str):
            raise TypeError(f"infoset must be str, not {infoset.__class__.__name__}")
        infoset_labels = self.get_infosets(player)
        if infoset not in infoset_labels:
            raise KeyError(f"player '{player}' has no information set with label '{infoset}'")
        return infoset_labels.index(infoset) + 1

    @cython.cfunc
    def _type_prior(self, player: str, infoset: str) -> c_Rational:
        """The exact prior probability of `player`'s information set `infoset`."""
        position = self._player_position(player)
        return As[c_GameBAGGRep](self.game).get().GetTypePrior(
            position, self._type_position(player, infoset)
        )

    @cython.cfunc
    def _player_position(self, player: str) -> cython.int:
        """The one place that maps a public player label to its 1-based position.

        Raises
        ------
        KeyError
            If `player` is not a label of a player of the game.
        TypeError
            If `player` is not a `str`.
        ValueError
            If `player` is an empty string or all whitespace.
        """
        if not isinstance(player, str):
            raise TypeError(f"player must be str, not {player.__class__.__name__}")
        if not player.strip():
            raise ValueError("player cannot be an empty string or all spaces")
        for i, label in enumerate(self.players):
            if label == player:
                return i + 1
        raise KeyError(f"no player with label '{player}'")

    @property
    def players(self) -> BAGGPlayers:
        """The set of players in the game.

        .. versionchanged:: 17.0.0
            The Bayesian players, rather than one player per type.
        """
        return BAGGPlayers.wrap(self.game)

    def get_infosets(self, player: str) -> list[str]:
        """Returns the labels of the information sets of `player`.

        Each information set is one of the player's types.

        .. versionadded:: 17.0.0

        Parameters
        ----------
        player : str
            The label of the player.

        Returns
        -------
        list of str

        Raises
        ------
        KeyError
            If no player has label `player`.
        TypeError
            If `player` is not a `str`.
        ValueError
            If `player` is an empty string or all whitespace.
        """
        return [
            str(t + 1)
            for t in range(
                As[c_GameBAGGRep](self.game).get().NumTypes(self._player_position(player))
            )
        ]

    @cython.cfunc
    def _wrap_strategy_profile_double(
            self, profile: shared_ptr[c_MixedStrategyProfile[float]]
    ) -> object:
        return BAGGMixedBehaviorProfileDouble.wrap(profile)

    @cython.cfunc
    def _wrap_strategy_profile_rational(
            self, profile: shared_ptr[c_MixedStrategyProfile[c_Rational]]
    ) -> object:
        return BAGGMixedBehaviorProfileRational.wrap(profile)

    @cython.cfunc
    def _wrap_logit_qre_strategy(
            self, qre: shared_ptr[c_LogitQREMixedStrategyProfile]
    ) -> object:
        return BAGGLogitQREMixedBehaviorProfile.wrap(qre)

    @cython.cfunc
    def _new_profile(self, rational: bool) -> BAGGMixedBehaviorProfile:
        if rational:
            return BAGGMixedBehaviorProfileRational.wrap(
                make_shared[c_MixedStrategyProfile[c_Rational]](
                    self.game.deref().NewMixedStrategyProfile(c_Rational())
                )
            )
        return BAGGMixedBehaviorProfileDouble.wrap(
            make_shared[c_MixedStrategyProfile[double]](
                self.game.deref().NewMixedStrategyProfile(0.0)
            )
        )

    def mixed_behavior_profile(self, data=None, rational=False) -> BAGGMixedBehaviorProfile:
        """Create a mixed behavior profile over the game.

        If `data` is not specified, the profile is initialized to uniform randomization
        at each information set.

        .. versionadded:: 17.0.0

        Parameters
        ----------
        data : array_like of array_like of array_like, optional
            A nested list (or compatible type) indexed by player, then information set,
            then action, specifying the probabilities of the actions.
        rational : bool, optional
            If True, probabilities are represented using rational numbers;
            otherwise floating point numbers are used.

        See Also
        --------
        random_behavior_profile :
           Create a `BAGGMixedBehaviorProfile` with randomly-drawn probabilities.
        """
        profile = self._new_profile(rational)
        if data is not None:
            players = list(self.players)
            if len(data) != len(players):
                raise ValueError("data must have one entry for each player")
            for player, player_data in zip(players, data, strict=True):
                infosets = self.get_infosets(player)
                if len(player_data) != len(infosets):
                    raise ValueError(
                        f"data for player '{player}' must have one entry for each "
                        f"information set"
                    )
                for infoset, values in zip(infosets, player_data, strict=True):
                    actions = profile._action_labels(player, infoset)
                    if len(values) != len(actions):
                        raise ValueError(
                            f"data for player '{player}' information set '{infoset}' must "
                            f"have one entry for each action"
                        )
                    profile[player, infoset] = dict(zip(actions, values, strict=True))
        return profile

    def random_behavior_profile(
            self,
            denom: int = None,
            gen: np.random.Generator | None = None
    ) -> BAGGMixedBehaviorProfile:
        """Create a `BAGGMixedBehaviorProfile` on the game, with probabilities drawn
        from the uniform distribution over the set of mixed behavior profiles.

        .. versionadded:: 17.0.0

        Parameters
        ----------
        denom : int, optional
            If specified, the probabilities are generated on a grid with denominator
            `denom`, and the resulting profile will be a `BAGGMixedBehaviorProfileRational`.
            If not specified, the probabilities will be floating point numbers, and
            the resulting profile will be a `BAGGMixedBehaviorProfileDouble`.
        gen : np.random.Generator, optional
            If specified, uses the `numpy` random number generator `gen` to generate
            uniform random samples.  Otherwise, uses the default generation method
            in `numpy`.

        See Also
        --------
        mixed_behavior_profile : Create a `BAGGMixedBehaviorProfile` with specified probabilities.
        """
        if denom is not None and denom < 1:
            raise ValueError("random_behavior_profile(): denom must be positive")
        profile = self._new_profile(denom is not None)
        for player in self.players:
            for infoset in self.get_infosets(player):
                actions = profile._action_labels(player, infoset)
                profile[player, infoset] = (
                    _dirichlet_distribution(actions, gen) if denom is None
                    else _grid_distribution(actions, denom, gen)
                )
        return profile

    def _repr_html_(self):
        return repr(self)

    contingencies = property(_unsupported)
    is_const_sum = property(_unsupported)
    get_strategies = _unsupported
    get_payoffs = _unsupported
    mixed_strategy_profile = _unsupported
    random_strategy_profile = _unsupported
    strategy_support_profile = _unsupported
    to_arrays = _unsupported
    to_nfg = _unsupported
    to_html = _unsupported
    to_latex = _unsupported
    to_efg = _unsupported
    relabel_players = _unsupported
    get_min_payoff = _unsupported
    get_max_payoff = _unsupported
    has_perfect_recall = _unsupported
