#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/action.pxi
# Cython wrapper for actions
#
# This program is free software; you can redistribute it and/or modify
# It under the terms of the GNU General Public License as published by
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
class Action:
    """A choice available at an information set or event in a ``Game``.

    .. versionchanged:: 17.0.0
        Stripped down to only ``label`` and ``prob``; an action is identified purely by
        its label within its information set or event, and no longer carries its own
        identity beyond that. Use ``Node.actions`` to look up the action available at a
        node's information set or event.
    """
    action = cython.declare(c_GameAction)

    def __init__(self, *args, **kwargs) -> None:
        raise ValueError("Cannot create an Action outside a Game.")

    @staticmethod
    @cython.cfunc
    def wrap(action: c_GameAction) -> Action:
        obj: Action = Action.__new__(Action)
        obj.action = action
        return obj

    def __repr__(self) -> str:
        return f"Action(label='{self.label}')"

    def __eq__(self, other: typing.Any) -> bool:
        return (
            isinstance(other, Action) and
            self.action.deref() == cython.cast(Action, other).action.deref()
        )

    def __hash__(self) -> int:
        return cython.cast(cython.long, self.action.deref())

    @property
    def label(self) -> str:
        """The text label of the action.

        .. versionchanged:: 17.0.0
            A label may now be any well-formed UTF-8 text, not just ASCII; it must still
            contain no control characters, and must not begin/end with whitespace or have
            two consecutive whitespace characters.  "Whitespace" means any Unicode space
            separator (e.g. U+00A0 NO-BREAK SPACE), not just the ASCII space.

            The label is now read-only, and must be nonempty and unique within its
            information set; use `Game.relabel_actions` to change it.
        """
        return self.action.deref().GetLabel().decode("utf-8")

    @property
    def prob(self) -> decimal.Decimal | Rational:
        """
        Get the probability a chance action is played.

        Raises
        ------
        UndefinedOperationError
            If the action does not belong to the chance player.
        """
        if not self.action.deref().GetInfoset().deref().IsChanceInfoset():
            raise UndefinedOperationError(
                "action probabilities are only defined at events"
            )
        py_string = cython.cast(
            string,
            self.action.deref().GetInfoset().deref().GetActionProb(self.action)
        )
        if "." in py_string.decode("ascii"):
            return decimal.Decimal(py_string.decode("ascii"))
        else:
            return Rational(py_string.decode("ascii"))
