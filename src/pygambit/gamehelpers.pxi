#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/gamehelpers.pxi
# Private helpers shared by several of Game's label-reconciliation, relabeling,
# and random-profile-generation methods
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

import numpy as np
import scipy.stats


def _reconcile_labels(
    current: list, labels: list, add: bool, drop: bool, funcname: str,
    owner_noun: str, item_singular: str, item_plural: str, consequence: str
) -> tuple:
    """Shared add/drop reconciliation for a "set the collection to be `labels`,
    matching by label" operation (`set_move_actions`/`set_event_actions`/
    `set_players`/`set_strategies`): determines which of `labels` are new
    (`added`) and which of `current` are missing from `labels` (`missing`).

    Raises ValueError if `current` has duplicate labels (matching by label is then
    ill-defined), or if a nonempty `added`/`missing` is not confirmed by `add`/`drop`.
    """
    if len(set(current)) != len(current):
        raise ValueError(
            f"{funcname}(): the {owner_noun} has duplicate {item_singular} labels, "
            f"so matching by label is not well-defined"
        )
    current_set = set(current)
    declared = set(labels)
    added = [label for label in labels if label not in current_set]
    missing = [label for label in current if label not in declared]
    if added and not add:
        raise ValueError(f"{funcname}(): would create new {item_plural} {added}")
    if missing and not drop:
        raise ValueError(
            f"{funcname}(): would delete {item_plural} {missing} and {consequence}; "
            f"pass drop=True to confirm"
        )
    return added, missing


def _compute_relabeling(
    current: list, labels: typing.Mapping, funcname: str, item_noun: str, strict: bool,
    ambiguous_desc: str, reserved: str | None = None, reserved_desc: str = ""
) -> dict:
    """Shared validation for a "simultaneously reassign labels" operation
    (`relabel_actions`/`relabel_strategies`/`relabel_players`): validates `labels`
    (a mapping from current label to replacement) against `current`, and returns
    a plain `dict` of only the entries that are a real, confirmed relabeling
    (unknown keys dropped when not `strict`; no-op entries where key equals value
    are dropped unconditionally).

    Raises TypeError if `labels` is not a str-to-str mapping; KeyError if `strict`
    and a key of `labels` matches no current label; ValueError if a key matches
    `reserved`, or matches more than one current label.
    """
    remap = {}
    for old, new in labels.items():
        if not isinstance(old, str) or not isinstance(new, str):
            raise TypeError(f"{funcname}(): labels must map str to str")
        if reserved is not None and old == reserved:
            raise ValueError(f"{funcname}(): {reserved_desc}")
        matches = current.count(old)
        if matches > 1:
            raise ValueError(f"{funcname}(): label '{old}' is ambiguous {ambiguous_desc}")
        if matches == 0:
            if strict:
                raise KeyError(f"{funcname}(): no {item_noun} with label '{old}'")
            continue
        if new == old:
            continue
        remap[old] = new
    return remap


def _resolve_infoset_or_event_kind(
    this: object, other: object, this_bare: str, this_full: str, other_full: str,
    funcname: str, argname: str
) -> object:
    """Shared error-raising shape for `_resolve_infoset`/`_resolve_event`: `this` is
    the already-resolved `Infoset`/`Event` of the desired kind, falsy if the
    anchoring node's partition element is not of that kind; `other` is the opposite
    kind, consulted only to raise a more specific error when `this` does not apply.
    """
    if not this:
        if other:
            raise ValueError(f"{funcname}(): {argname} resolves to {other_full}, not {this_full}")
        raise ValueError(
            f"{funcname}(): {argname} resolves to no {this_bare} (the node is terminal)"
        )
    return this


def _dirichlet_distribution(items: list, gen: object) -> dict:
    """A uniform-random probability distribution over `items` (a flat Dirichlet(1,...,1)
    draw), keyed by item, as `float`. Shared by `random_strategy_profile`/
    `random_behavior_profile`'s `denom=None` case.
    """
    weights = scipy.stats.dirichlet(alpha=[1 for _ in items], seed=gen).rvs(size=1)[0]
    return dict(zip(items, weights, strict=True))


def _grid_distribution(items: list, denom: int, gen: object) -> dict:
    """A uniform-random probability distribution over `items`, restricted to the grid
    with denominator `denom` (a uniformly-random composition of `denom` into
    `len(items)` nonnegative parts), keyed by item, as `Rational`. Shared by
    `random_strategy_profile`/`random_behavior_profile`'s `denom` grid case.
    """
    k = len(items)
    sample = (
        [0] +
        sorted((gen or np.random).choice(np.arange(1, denom + k), size=k - 1, replace=False)) +
        [denom + k]
    )
    return {
        item: Rational(hi - lo - 1, denom)
        for item, (hi, lo) in zip(items, zip(sample[1:], sample[:-1], strict=True), strict=True)
    }
