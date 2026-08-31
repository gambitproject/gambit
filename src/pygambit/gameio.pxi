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
        g = Game.wrap(parser(data))
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
    read_efg, read_nfg, read_agg, read_bagg
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
    read_gbt, read_nfg, read_agg, read_bagg
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
    read_gbt, read_efg, read_agg, read_bagg
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
    read_gbt, read_efg, read_nfg, read_bagg
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
    read_gbt, read_efg, read_nfg, read_agg
    """
    return read_game(filepath_or_buffer, parser=ParseBaggGame)
