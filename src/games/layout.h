//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/games/layout.h
// Interface to generic tree layout representation
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

#ifndef GAMBIT_GAMES_TREELAYOUT_H
#define GAMBIT_GAMES_TREELAYOUT_H

#include <map>

#include "gambit.h"

namespace Gambit {
struct LayoutEntry {
  friend class Layout;
  double m_offset{-1};        // Cartesian coordinates of node
  int m_level, m_sublevel{0}; // depth of the node in tree
  bool m_inSupport{true};

  explicit LayoutEntry(int p_level) : m_level(p_level) {}
};

class Layout {
  Game m_game;
  std::map<GameNode, std::shared_ptr<LayoutEntry>> m_nodeMap;
  std::vector<int> m_numSublevels;
  std::map<std::pair<int, GameInfoset>, int> m_infosetSublevels;

  double m_maxOffset{0};

  void LayoutSubtree(const GameNode &, const BehaviorSupportProfile &, int, double &);

public:
  explicit Layout(const Game &p_game) : m_game(p_game) {}
  ~Layout() = default;

  void LayoutTree(const BehaviorSupportProfile &);

  const std::map<GameNode, std::shared_ptr<LayoutEntry>> &GetNodeMap() const { return m_nodeMap; }
  const std::vector<int> &GetNumSublevels() const { return m_numSublevels; }
  double GetMinOffset() const { return 0; }
  double GetMaxOffset() const { return m_maxOffset; }
};
} // namespace Gambit

#endif // GAMBIT_GAMES_TREELAYOUT_H
