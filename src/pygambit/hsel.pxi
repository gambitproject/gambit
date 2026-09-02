#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/hsel.pxi
# First sketch of the H selector algebra: game-neutral expressions built by
# pygambit.H, evaluated only when handed to a Game.  Deliberately minimal --
# just enough operations to validate the architecture, not the full roster.
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


class _PathStep:
    """One `H.path(*steps)` operation: each step is an exact action label or
    the wildcard `...`.  Root-anchored if it is the first op in a Selector,
    "this many more steps from here" otherwise -- the evaluator doesn't need
    to distinguish the two cases, since they're the same operation applied to
    whatever's already been selected (the root, for a bare seed)."""

    def __init__(self, steps: tuple) -> None:
        self.steps = steps

    def __repr__(self) -> str:
        return f"_PathStep(steps={self.steps!r})"


class _PlaysStep:
    """One `.plays` operation: expand to the current terminal frontier."""

    def __repr__(self) -> str:
        return "_PlaysStep()"


class Selector:
    """A game-neutral description of a set of nodes.  Carries no reference to
    any game -- it's just a recipe, evaluated only when handed to a Game
    method such as `get_nodes`.

    .. versionadded:: 17.0.0
    """

    def __init__(self, ops: tuple = ()) -> None:
        self._ops = ops

    def __repr__(self) -> str:
        return f"Selector(ops={self._ops!r})"

    def _extend(self, op) -> Selector:
        return Selector(self._ops + (op,))

    def path(self, *steps: str) -> Selector:
        """`N` more steps from wherever this selection currently is.  Each
        step is an exact action label, or `...` to match any single action.
        """
        return self._extend(_PathStep(steps))

    @property
    def plays(self) -> Selector:
        """The current terminal frontier of this selection -- not
        necessarily one step forward, whatever is currently terminal beneath
        each already-selected node."""
        return self._extend(_PlaysStep())


class H:
    """Namespace of seed constructors for the node-selector algebra.  Not
    meant to be instantiated -- use as `H.path(...)`, conventionally imported
    as `import pygambit.H as H`.

    .. versionadded:: 17.0.0
    """

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("H is a namespace of selector constructors, not instantiable.")

    @staticmethod
    def path(*steps: str) -> Selector:
        """A root-anchored selection.  Each step is an exact action label, or
        `...` to match any single action.  `H.path()` with no steps selects
        the root itself.
        """
        return Selector().path(*steps)

    plays: Selector = Selector((_PlaysStep(),))
    """All currently-terminal nodes in the whole game."""
