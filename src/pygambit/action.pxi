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
    """A choice available at an ``Infoset`` in a ``Game``."""
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
        if self.label:
            return f"Action(infoset={self.infoset}, label='{self.label}')"
        else:
            return f"Action(infoset={self.infoset}, number={self.number})"

    def __eq__(self, other: typing.Any) -> bool:
        return (
            isinstance(other, Action) and
            self.action.deref() == cython.cast(Action, other).action.deref()
        )

    def __hash__(self) -> int:
        return cython.cast(cython.long, self.action.deref())

    @property
    def number(self) -> int:
        """Returns the number of the action at its information set.
        Actions are numbered starting with 0.
        """
        return self.action.deref().GetNumber() - 1

    def precedes(self, node: Node) -> bool:
        """Returns whether `node` precedes this action in the
        extensive game.

        Raises
        ------
        MismatchError
            If `node` is not in the same game as the action.
        """
        if self.infoset.game != node.game:
            raise MismatchError("precedes() requires a node from the same game as the action")
        return self.action.deref().Precedes(cython.cast(Node, node).node)

    @property
    def label(self) -> str:
        """Get or set the text label of the action."""
        return self.action.deref().GetLabel().decode("ascii")

    @label.setter
    def label(self, value: str) -> None:
        if value == self.label:
            return
        if value == "" or value in (act.label for act in self.infoset.actions):
            warnings.warn("In a future version, actions must have unique labels "
                          "within their information set",
                          FutureWarning)
        self.action.deref().SetLabel(value.encode("ascii"))

    @property
    def infoset(self) -> Infoset:
        """Get the information set to which the action belongs."""
        return Infoset.wrap(self.action.deref().GetInfoset())

    @property
    def prob(self) -> decimal.Decimal | Rational:
        """
        Get the probability a chance action is played.

        Raises
        ------
        UndefinedOperationError
            If the action does not belong to the chance player.
        """
        if not self.infoset.is_chance:
            raise UndefinedOperationError(
                "action probabilities are only defined at chance information sets"
            )
        py_string = cython.cast(
            string,
            self.action.deref().GetInfoset().deref().GetActionProb(self.action)
        )
        if "." in py_string.decode("ascii"):
            return decimal.Decimal(py_string.decode("ascii"))
        else:
            return Rational(py_string.decode("ascii"))

    @property
    def plays(self) -> list[Node]:
        """Returns a list of all terminal `Node` objects consistent with it.
        """
        return [
            Node.wrap(n) for n in
            self.action.deref().GetInfoset().deref().GetGame().deref().GetPlays(self.action)
        ]
