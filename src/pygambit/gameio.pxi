#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/gameio.pxi
# Functions to construct a Game by reading its serialized representation from a file
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
import pathlib

ctypedef c_Game (*GameParser)(const string &) except +IOError


@cython.cfunc
def read_game(filepath_or_buffer: str | pathlib.Path | io.IOBase,
              parser: GameParser):

    g = cython.declare(Game)
    if isinstance(filepath_or_buffer, io.TextIOBase):
        data = filepath_or_buffer.read().encode("utf-8")
    elif isinstance(filepath_or_buffer, io.IOBase):
        data = filepath_or_buffer.read()
    else:
        with open(filepath_or_buffer, "rb") as f:
            data = f.read()
    try:
        g = Game._wrap(parser(data))
    except Exception as exc:
        raise ValueError(f"Parse error in game file: {exc}") from None
    return g


def read_gbt(filepath_or_buffer: str | pathlib.Path | io.IOBase) -> Game:
    """Construct a game from its serialised representation in a GBT file.

    Parameters
    ----------
    filepath_or_buffer : str, pathlib.Path or io.IOBase
        The path to the file containing the game representation or file-like object

    Returns
    -------
    Game
        A game constructed from the representation in the file.

    Raises
    ------
    IOError
        If the file cannot be opened or read
    ValueError
        If the contents of the file are not a valid game representation.

    See Also
    --------
    read_efg, read_nfg, read_agg, read_bagg, read_gtdraw
    """
    return read_game(filepath_or_buffer, parser=ParseGbtGame)


def read_efg(filepath_or_buffer: str | pathlib.Path | io.IOBase) -> Game:
    """Construct a game from its serialised representation in an EFG file.

    Parameters
    ----------
    filepath_or_buffer : str, pathlib.Path or io.IOBase
        The path to the file containing the game representation or file-like object

    Returns
    -------
    Game
        A game constructed from the representation in the file.

    Raises
    ------
    IOError
        If the file cannot be opened or read
    ValueError
        If the contents of the file are not a valid game representation.

    See Also
    --------
    read_gbt, read_nfg, read_agg, read_bagg, read_gtdraw
    """
    return read_game(filepath_or_buffer, parser=ParseEfgGame)


def read_nfg(filepath_or_buffer: str | pathlib.Path | io.IOBase) -> Game:
    """Construct a game from its serialised representation in a NFG file.

    Parameters
    ----------
    filepath_or_buffer : str, pathlib.Path or io.IOBase
        The path to the file containing the game representation or file-like object

    Returns
    -------
    Game
        A game constructed from the representation in the file.

    Raises
    ------
    IOError
        If the file cannot be opened or read
    ValueError
        If the contents of the file are not a valid game representation.

    See Also
    --------
    read_gbt, read_efg, read_agg, read_bagg, read_gtdraw
    """
    return read_game(filepath_or_buffer, parser=ParseNfgGame)


def read_agg(filepath_or_buffer: str | pathlib.Path | io.IOBase) -> Game:
    """Construct a game from its serialised representation in an AGG file.

    Parameters
    ----------
    filepath_or_buffer : str, pathlib.Path or io.IOBase
        The path to the file containing the game representation or file-like object

    Returns
    -------
    Game
        A game constructed from the representation in the file.

    Raises
    ------
    IOError
        If the file cannot be opened or read
    ValueError
        If the contents of the file are not a valid game representation.

    See Also
    --------
    read_gbt, read_efg, read_nfg, read_bagg, read_gtdraw
    """
    return read_game(filepath_or_buffer, parser=ParseAggGame)


def read_bagg(filepath_or_buffer: str | pathlib.Path | io.IOBase) -> Game:
    """Construct a game from its serialised representation in a BAGG file.

    Parameters
    ----------
    filepath_or_buffer : str, pathlib.Path or io.IOBase
        The path to the file containing the game representation or file-like object

    Returns
    -------
    Game
        A game constructed from the representation in the file.

    Raises
    ------
    IOError
        If the file cannot be opened or read
    ValueError
        If the contents of the file are not a valid game representation.

    See Also
    --------
    read_gbt, read_efg, read_nfg, read_agg, read_gtdraw
    """
    return read_game(filepath_or_buffer, parser=ParseBaggGame)


def read_gtdraw(filepath_or_buffer: str | pathlib.Path | io.IOBase) -> Game:
    """Construct an extensive game from a drawing in gtdraw's format.

    A drawing restates a game: its players, the moves along each history, its
    information sets, chance probabilities and payoffs.  The positions of the nodes are
    checked and then dropped.  The drawing has no title and no outcome labels, and the
    outcomes of the game are numbered.  The format is documented with gtdraw, and
    both of its versions (EF 0 and EF 1) are read.  A drawing has no header, so the format
    is never detected: a drawing is read only by this function.

    Numbers in payoffs and in chance probabilities are an integer, a decimal, ``a/b`` or
    ``\\frac{a}{b}``, each with an optional leading minus and optional surrounding ``$``.
    A chance move's probability is the last ``~``-separated part of its label, bare or in
    parentheses, and the rest of the label (if any) is the action's label.  A label
    reads ``~`` as a space.  A terminal node that lists fewer payoffs than the game has
    players pays zero to the rest, and one with no payoffs has no outcome.

    Parameters
    ----------
    filepath_or_buffer : str, pathlib.Path or io.IOBase
        The path to the file containing the drawing or file-like object

    Returns
    -------
    Game
        A game constructed from the drawing.

    Raises
    ------
    IOError
        If the file cannot be opened or read
    ValueError
        If the contents of the file are not a valid drawing.  The message gives the
        number of the offending line.

    See Also
    --------
    ExtensiveGame.to_gtdraw, read_efg
    """
    return read_game(filepath_or_buffer, parser=ParseGtdrawGame)
