//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: library/include/gambit/writer.h
// Classes for writing out games to various formats
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#ifndef LIBGAMBIT_WRITER_H
#define LIBGAMBIT_WRITER_H

#include <string>

namespace Gambit {

///
/// Abstract base class for objects that write games to various formats
///
class GameWriter  {
public:
  ///
  /// Convert the game to a string-based representation
  ///
  virtual std::string Write(const Game &) const = 0;
};

///
/// Format the strategic representation of a game to HTML tables.
///
class HTMLGameWriter : public GameWriter {
public:
  ///
  /// Convert the game to HTML, with player 1 on the rows and player 2
  /// on the columns.
  ///
  std::string Write(const Game &p_game) const override
  { return Write(p_game, 1, 2); }

  ///
  /// Convert the game to HTML, selecting the row and column player numbers.
  ///
  std::string Write(const Game &p_game, int p_rowPlayer, int p_colPlayer) const;
};

///
/// Format the strategic representation of a game to LaTeX sgame style
///
class LaTeXGameWriter : public GameWriter {
public:
  ///
  /// Convert the game to LaTeX, with player 1 on the rows and player 2
  /// on the columns.
  ///
  std::string Write(const Game &p_game) const override
  { return Write(p_game, 1, 2); }

  ///
  /// Convert the game to LaTeX, selecting the row and column player numbers.
  ///
  std::string Write(const Game &p_game, int p_rowPlayer, int p_colPlayer) const;
};

} // end namespace Gambit

#endif // LIBGAMBIT_WRITER_H
