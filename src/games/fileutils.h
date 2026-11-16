//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/games/fileutils.h
// Helpers shared by the readers of game files
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

#ifndef GAMBIT_GAMES_FILEUTILS_H
#define GAMBIT_GAMES_FILEUTILS_H

#include <cctype>
#include <map>
#include <set>
#include <string>

namespace Gambit {

// std::isspace/std::isdigit are undefined behavior when given a (possibly
// negative) plain `char` other than EOF; a UTF-8 continuation or lead byte
// has the high bit set and so is negative on a platform with signed char.
// These wrappers convert to `unsigned char` first, restricting the check to
// the ASCII whitespace/digit characters that terminate lexer tokens.
inline bool IsAsciiSpace(char c) { return std::isspace(static_cast<unsigned char>(c)) != 0; }
inline bool IsAsciiDigit(char c) { return std::isdigit(static_cast<unsigned char>(c)) != 0; }

/// Normalizes labels in place so the resulting set is distinct and nonempty:
/// empty labels are given a suffix and repeated labels are de-duplicated by
/// appending "_n", choosing the next n not already present in the scope.
/// `p_get(element)` reads an element's label and `p_set(element, label)`
/// writes it, so this works both on a container of game objects (via GetLabel/SetLabel)
/// and on a container of raw label strings (read/write the string directly).
template <class Container, class Getter, class Setter>
inline void NormalizeLabels(Container &&p_container, Getter p_get, Setter p_set)
{
  // NOLINTBEGIN(misc-const-correctness)
  std::map<std::string, std::size_t> counts;
  std::set<std::string> used;
  // NOLINTEND(misc-const-correctness)
  for (auto &&element : p_container) {
    counts[p_get(element)] += 1;
    used.insert(p_get(element));
  }
  // NOLINTBEGIN(misc-const-correctness)
  std::map<std::string, std::size_t> visited;
  // NOLINTEND(misc-const-correctness)
  for (auto &&element : p_container) {
    const auto label = p_get(element);
    // A special case: If only one label is the empty string we still want to
    // convert it to "_1"
    if (counts[label] == 1 && label != "") {
      continue;
    }
    // Generate the next "label_n" that is not already used in this scope, so
    // that e.g. {"x", "x", "x_1"} does not renumber to a duplicate "x_1".
    std::string candidate;
    do {
      const auto index = ++visited[label];
      candidate = label + "_" + std::to_string(index);
    } while (used.contains(candidate));
    used.insert(candidate);
    p_set(element, candidate);
  }
}

/// Normalizes a list of raw label strings.
template <class Container> inline void NormalizeLabelStrings(Container &p_labels)
{
  NormalizeLabels(
      p_labels, [](const std::string &s) { return s; },
      [](std::string &s, const std::string &v) { s = v; });
}

} // end namespace Gambit

#endif // GAMBIT_GAMES_FILEUTILS_H
