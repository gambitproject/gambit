//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/efglayout.cc
// Implementation of tree layout representation
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

#include "layout.h"

namespace Gambit {

void Layout::LayoutSubtree(const GameNode &p_node, int p_level, double &p_offset)
{
  const auto entry = std::make_shared<LayoutEntry>(p_level);
  m_nodeMap[p_node] = entry;
  try {
    entry->m_sublevel = m_infosetSublevels.at({entry->m_level, p_node->GetInfoset()});
  }
  catch (std::out_of_range &) {
    if (p_level - 1 < static_cast<int>(m_numSublevels.size())) {
      m_numSublevels.push_back(0);
    }
    entry->m_sublevel = ++m_numSublevels[p_level];
    m_infosetSublevels[{entry->m_level, p_node->GetInfoset()}] = entry->m_sublevel;
  }

  if (p_node->IsTerminal()) {
    entry->m_offset = p_offset;
    p_offset += 1;
    return;
  }
  if (p_node->GetInfoset() && !p_node->GetInfoset()->GetPlayer()->IsChance()) {
    const auto actions = p_node->GetInfoset()->GetActions();
    for (const auto &action : p_node->GetInfoset()->GetActions()) {
      LayoutSubtree(p_node->GetChild(action), p_level + 1, p_offset);
    }
    entry->m_offset = (m_nodeMap.at(p_node->GetChild(actions.front()))->m_offset +
                       m_nodeMap.at(p_node->GetChild(actions.back()))->m_offset) /
                      2;
  }
  else {
    for (const auto &child : p_node->GetChildren()) {
      LayoutSubtree(child, p_level + 1, p_offset);
    }
    entry->m_offset = (m_nodeMap.at(p_node->GetChildren().front())->m_offset +
                       m_nodeMap.at(p_node->GetChildren().back())->m_offset) /
                      2;
  }
}

void Layout::LayoutTree(const Game &p_game)
{
  m_nodeMap.clear();
  m_numSublevels.clear();
  m_infosetSublevels.clear();

  m_maxOffset = 0;
  LayoutSubtree(p_game->GetRoot(), 0, m_maxOffset);
}

} // namespace Gambit
