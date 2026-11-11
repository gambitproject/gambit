#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/gameoutcomes.pxi
# Outcome members shared by the games that have outcomes
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

# ExtensiveGame and StrategicGame provide the outcome members as thin methods that call
# the functions below: a Cython extension type cannot have attributes assigned after it is
# created, so the functions cannot be bound into the classes.  The helpers name C++ handles,
# so they stay in Cython.


@cython.cfunc
def _resolve_outcome(
    self: Game, label: typing.Any, funcname: str, argname: str = "label"
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
def _resolve_payoff_mapping(self: Game, payoffs: typing.Mapping, funcname: str) -> dict:
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
def _do_relabel_outcomes(
    self: Game, labels: typing.Mapping[str, str], strict: bool
) -> cython.void:
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


def _get_outcomes(self: Game) -> list[str]:
    return [
        o.deref().GetLabel().decode("utf-8") for o in self.game.deref().GetOutcomes()
    ]


def _get_outcome_payoffs(self: Game, label: str) -> PayoffVector:
    resolved_outcome: c_GameOutcome = _resolve_outcome(self, label, "get_outcome_payoffs")
    values = {}
    for player in self.players:
        resolved_player = self._resolve_player(player, "get_outcome_payoffs")
        values[player] = _number_to_py(resolved_outcome.deref().GetPayoff[string](
            resolved_player
        ))
    return PayoffVector(values)


def _set_outcome_payoffs(self: Game, label: str, payoffs: typing.Mapping) -> None:
    resolved_outcome: c_GameOutcome = _resolve_outcome(self, label, "set_outcome_payoffs")
    resolved_payoffs = _resolve_payoff_mapping(self, payoffs, "set_outcome_payoffs")
    for player in self.players:
        resolved_outcome.deref().SetPayoff(
            self._resolve_player(player, "set_outcome_payoffs"),
            _to_number(resolved_payoffs[player])
        )
