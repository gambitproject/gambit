#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/infoset.pxi
# Cython wrapper for information sets
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
class _InfosetOrEvent:
    """Shared implementation for `Infoset` and `Event`: a lazy, node-anchored view over
    an information set, filtered to whichever of the two subclasses' concept currently
    applies at the anchoring node (see each subclass's `_try_resolve`).

    Not exported; only `Infoset` and `Event` are part of the public API.
    """
    node = cython.declare(c_GameNode)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError(f"Cannot create an {type(self).__name__} outside a Game.")

    @cython.cfunc
    def _try_resolve(self) -> c_GameInfoset:
        """Returns the resolved handle, filtered to this subclass's applicable case;
        null if that case does not currently apply at the anchoring node (including,
        but not limited to, a terminal node)."""
        raise NotImplementedError

    @cython.cfunc
    def _resolve(self) -> c_GameInfoset:
        """Returns the resolved handle, raising if this subclass's case does not
        currently apply at the anchoring node."""
        resolved: c_GameInfoset = self._try_resolve()
        if resolved == cython.cast(c_GameInfoset, NULL):
            raise AttributeError(
                f"node's {type(self).__name__.lower()} is currently None"
            )
        return resolved

    def __repr__(self) -> str:
        if self._try_resolve() == cython.cast(c_GameInfoset, NULL):
            return "None"
        name = type(self).__name__
        if self.label:
            return f"{name}(player={self.player}, label='{self.label}')"
        else:
            return f"{name}(player={self.player}, number={self.number})"

    def __eq__(self, other: typing.Any):
        if type(other) is not type(self):
            return NotImplemented
        mine: c_GameInfoset = self._try_resolve()
        theirs: c_GameInfoset = cython.cast(_InfosetOrEvent, other)._try_resolve()
        if mine == cython.cast(c_GameInfoset, NULL) or theirs == cython.cast(c_GameInfoset, NULL):
            return (
                mine == cython.cast(c_GameInfoset, NULL) and
                theirs == cython.cast(c_GameInfoset, NULL)
            )
        return mine == theirs

    def __bool__(self) -> bool:
        return self._try_resolve() != cython.cast(c_GameInfoset, NULL)

    def __hash__(self) -> int:
        resolved: c_GameInfoset = self._try_resolve()
        if resolved == cython.cast(c_GameInfoset, NULL):
            return 0
        return cython.cast(cython.long, resolved.deref())

    def precedes(self, node: Node) -> bool:
        """Return whether this information set precedes `node` in the game tree."""
        return self._resolve().deref().Precedes(cython.cast(Node, node).node)

    @property
    def game(self) -> Game:
        """The ``Game`` to which the information set belongs."""
        return Game.wrap(self._resolve().deref().GetGame())

    @property
    def label(self) -> str:
        """Get or set the text label of the information set.

        .. versionchanged:: 17.0.0
            A label may now be any well-formed UTF-8 text, not just ASCII; it must still
            contain no control characters, and must not begin/end with whitespace or have
            two consecutive whitespace characters.  "Whitespace" means any Unicode space
            separator (e.g. U+00A0 NO-BREAK SPACE), not just the ASCII space.
        """
        return self._resolve().deref().GetLabel().decode("utf-8")

    @label.setter
    def label(self, value: str) -> None:
        self._resolve().deref().SetLabel(value.encode("utf-8"))

    @property
    def number(self) -> int:
        """Returns the number of the information set for its player.
        Information sets are numbered starting with 0.
        """
        return self._resolve().deref().GetNumber() - 1

    @property
    def is_absent_minded(self) -> bool:
        """
        Whether the information set is absent-minded.

        An information set is absent-minded if there exists a path of play
        in the game tree that intersects the information set more than once.

        .. versionadded:: 16.5.0
        """
        resolved: c_GameInfoset = self._resolve()
        return resolved.deref().GetGame().deref().IsAbsentMinded(resolved)

    @property
    def actions(self) -> list[str]:
        """The labels of the actions available at the information set, in order.

        .. versionchanged:: 17.0.0
            Returns bare labels rather than ``Action`` objects, following its removal.
        """
        resolved: c_GameInfoset = self._resolve()
        return [a.deref().GetLabel().decode("utf-8") for a in resolved.deref().GetActions()]

    @property
    def members(self) -> list[Node]:
        """The nodes which are members of the information set.

        The order of information set members is the order in which they are
        encountered in the pre-order depth first traversal of the game tree.

        .. versionchanged:: 17.0.0
            Returns a plain ``list`` rather than a lazily-resolved collection; a
            member is no longer accessible by label, following the removal of
            ``Action``/``Strategy`` label-indexed collections elsewhere in the API.
        """
        resolved: c_GameInfoset = self._resolve()
        return [Node.wrap(member) for member in resolved.deref().GetMembers()]

    @property
    def player(self) -> str:
        """The label of the player who has the move at this information set."""
        return self._resolve().deref().GetPlayer().deref().GetLabel().decode("utf-8")


@cython.cclass
class Infoset(_InfosetOrEvent):
    """An information set belonging to a personal player in a ``Game``: the point at
    which that player chooses an action, and so the object of potential optimisation.
    The corresponding concept for the chance player is an ``Event``.

    A lazy, node-anchored view: holds a member node and resolves the information set
    on each access, so the value reflects the current state of the game even after
    the game is mutated. For a node currently belonging to no personal player's
    information set (a terminal node, or a chance event -- see ``Node.event``), the
    view is falsy and equals ``None``.

    .. versionchanged:: 17.0.0
        Now a node-anchored view (see ``Node.infoset``) rather than an object with
        identity of its own; equality/hashing are still based on the information set
        currently resolved, not on the anchoring node. No longer used for the chance
        player's events; see ``Event``.
    """
    @staticmethod
    @cython.cfunc
    def wrap(node: c_GameNode) -> Infoset:
        obj: Infoset = Infoset.__new__(Infoset)
        obj.node = node
        return obj

    @cython.cfunc
    def _try_resolve(self) -> c_GameInfoset:
        resolved: c_GameInfoset = self.node.deref().GetInfoset()
        if resolved != cython.cast(c_GameInfoset, NULL) and resolved.deref().IsChanceInfoset():
            return cython.cast(c_GameInfoset, NULL)
        return resolved


@cython.cclass
class Event(_InfosetOrEvent):
    """An event belonging to the chance player in a ``Game``: a point of exogenous
    randomness, with a probability distribution over its actions that is specified
    rather than chosen. The corresponding concept for a personal player is an
    ``Infoset``.

    A lazy, node-anchored view: holds a member node and resolves the event on each
    access, so the value reflects the current state of the game even after the game
    is mutated. For a node not currently belonging to a chance event (a terminal
    node, or a personal player's information set -- see ``Node.infoset``), the view
    is falsy and equals ``None``.

    .. versionadded:: 17.0.0
    """
    @staticmethod
    @cython.cfunc
    def wrap(node: c_GameNode) -> Event:
        obj: Event = Event.__new__(Event)
        obj.node = node
        return obj

    @cython.cfunc
    def _try_resolve(self) -> c_GameInfoset:
        resolved: c_GameInfoset = self.node.deref().GetInfoset()
        if resolved != cython.cast(c_GameInfoset, NULL) and not resolved.deref().IsChanceInfoset():
            return cython.cast(c_GameInfoset, NULL)
        return resolved
