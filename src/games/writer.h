//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
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

/// @brief Render text as an explicit double-quoted string, escaping any double-quotes
///        in the text with a backslash
inline std::string QuoteString(const std::string &s)
{
  std::ostringstream ss;
  ss << std::quoted(s);
  return ss.str();
}

/// @brief Render a list of objects as a space-separated list of elements, delimited
///        by curly braces on either side
/// @param[in] p_container  The container over which to iterate
/// @param[in] p_renderer   A callable which returns a string representing each item
/// @param[in] p_commas     Use comma as delimiter between items (default is no)
/// @param[in] p_braces     Whether to include curly braces on either side of the list
/// @returns  The formatted list as a string following the conventions of Gambit savefiles.
template <class C, class T>
std::string FormatList(const C &p_container, T p_renderer, bool p_commas = false,
                       bool p_braces = true)
{
  std::string s, delim;
  if (p_braces) {
    s = "{";
    delim = " ";
  }
  for (auto element : p_container) {
    s += delim + p_renderer(element);
    if (p_commas) {
      delim = ", ";
    }
    else {
      delim = " ";
    }
  }
  if (p_braces) {
    s += " }";
  }
  return s;
}

///
/// Abstract base class for objects that write games to various formats
///
class GameWriter {
public:
  ///
  /// Convert the game to a string-based representation
  ///
  virtual std::string Write(const Game &) const = 0;
};

///
/// Convert the game to HTML, selecting the row and column player.
///
std::string WriteHTMLFile(const Game &p_game, const GamePlayer &p_rowPlayer,
                          const GamePlayer &p_colPlayer);

///
/// Format the strategic representation of a game to LaTeX sgame style
///
std::string WriteLaTeXFile(const Game &p_game, const GamePlayer &p_rowPlayer,
                           const GamePlayer &p_colPlayer);

} // end namespace Gambit

#endif // LIBGAMBIT_WRITER_H
