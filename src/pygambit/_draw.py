#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/_draw.py
# Drawing games in notebooks, by way of gtdraw
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

from __future__ import annotations

import io
import shutil
import typing

if typing.TYPE_CHECKING:
    from .gambit import _StrategicFormBase

_LAYOUT_OPTIONS = ("level_scaling", "sublevel_scaling", "width_scaling", "shared_terminal_depth")


class _HtmlTable(str):
    """HTML text that a notebook displays as HTML."""

    def _repr_html_(self) -> str:
        return str(self)


def repr_html(game: _StrategicFormBase) -> str:
    """The HTML that a notebook displays for a game; it never runs TeX."""
    from .gambit import ExtensiveGame

    return repr(game) if isinstance(game, ExtensiveGame) else game.to_html()


class _NotDrawn:
    """What a notebook shows in place of a drawing that the installation cannot make."""

    def __init__(self, reason: str) -> None:
        self._reason = reason

    def __repr__(self) -> str:
        return (
            f"{self._reason}, so the game is not drawn. Drawing needs gtdraw "
            "(`pip install pygambit[draw]`) and a LaTeX installation "
            "(pdflatex and pdftocairo)."
        )


def _missing() -> str | None:
    """Why games cannot be drawn here, or None if the prerequisites are present."""
    try:
        import gtdraw  # noqa: F401
    except ImportError:
        return "gtdraw is not installed"
    for tool in ("pdflatex", "pdftocairo"):
        if shutil.which(tool) is None:
            return f"{tool} is not installed"
    return None


def draw(game: _StrategicFormBase, style: typing.Any = None, **options: typing.Any) -> typing.Any:
    """Implementation of `draw` on the games that have a strategic form."""
    from .gambit import ExtensiveGame

    tree = isinstance(game, ExtensiveGame)

    reason = _missing()
    if reason is not None:
        return _NotDrawn(reason) if tree else _HtmlTable(game.to_html())

    import gtdraw

    if tree:
        layout = {name: options.pop(name) for name in _LAYOUT_OPTIONS if name in options}
        text = game.to_gtdraw(**layout)
        return gtdraw.draw(io.StringIO(text), format="ef", style=style, **options)
    return gtdraw.draw(io.StringIO(game.to_latex()), format="sgame", style=style, **options)
