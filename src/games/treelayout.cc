//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/games/treelayout.cc
// Flattened representation of the structure of a game tree
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

#include "games.h"
#include "gametree.h"
#include "treelayout.h"

namespace Gambit {

TreeLayout::TreeLayout(const GameRep &p_game)
{
  std::map<const GamePlayerRep *, int> playerIndex;
  for (const auto &player : p_game.GetPlayers()) {
    playerIndex[player.get()] = static_cast<int>(m_players.size());
    m_players.push_back(player);
  }
  numPlayers = m_players.size();

  size_t first = 1;
  for (const auto &infoset : p_game.GetInfosets()) {
    const size_t count = infoset->GetActions().size();
    infosetIndex[infoset.get()] = infosets.size();
    infosets.push_back(
        {infoset.get(), playerIndex.at(infoset->GetPlayer().get()), first, count, {}});
    first += count;
  }
  numActions = first - 1;
  numPersonalInfosets = infosets.size();
  for (const auto &infoset : p_game.GetChance()->GetInfosets()) {
    infosetIndex[infoset.get()] = infosets.size();
    infosets.push_back({infoset.get(), -1, 0, infoset->GetActions().size(), {}});
  }

  std::vector<GameNode> nodes;
  for (const auto &node : p_game.GetNodes()) {
    nodes.push_back(node);
  }
  const size_t numNodes = nodes.size();
  parent.assign(numNodes, -1);
  action.assign(numNodes, 0);
  infoset.assign(numNodes, -1);
  firstChild.assign(numNodes, 0);
  numChildren.assign(numNodes, 0);
  m_outcomes.resize(numNodes);
  m_chanceActions.resize(numNodes);

  for (size_t index = 0; index < numNodes; index++) {
    const GameNode &node = nodes[index];
    m_outcomes[index] = node->GetOutcome();
    firstChild[index] = childList.size();
    const GameInfoset nodeInfoset = node->GetInfoset();
    if (!nodeInfoset) {
      continue;
    }
    const size_t info = infosetIndex.at(nodeInfoset.get());
    infoset[index] = static_cast<int>(info);
    for (const auto &[nodeAction, child] : node->GetActions()) {
      const size_t childIndex = child->GetNumber() - 1;
      childList.push_back(childIndex);
      parent[childIndex] = static_cast<int>(index);
      if (info < numPersonalInfosets) {
        action[childIndex] = infosets[info].firstAction + nodeAction->GetNumber() - 1;
      }
      else {
        m_chanceActions[childIndex] = nodeAction;
      }
    }
    numChildren[index] = childList.size() - firstChild[index];
  }

  for (auto &entry : infosets) {
    for (const auto &member : entry.infoset->GetMembers()) {
      entry.members.push_back(member->GetNumber() - 1);
    }
  }
  for (const auto &[reentryInfoset, node] : p_game.GetAbsentMindedReentries()) {
    reentries.emplace_back(infosetIndex.at(reentryInfoset.get()), node->GetNumber() - 1);
  }
}

template <class T> TreeLayout::Numbers<T> TreeLayout::BuildNumbers() const
{
  Numbers<T> numbers;
  const size_t numNodes = parent.size();
  numbers.chanceProb.resize(numNodes, static_cast<T>(1));
  numbers.payoffs.resize(numNodes * numPlayers);
  for (size_t k = 0; k < numNodes; k++) {
    if (const auto &chanceAction = m_chanceActions[k]) {
      numbers.chanceProb[k] =
          static_cast<T>(chanceAction->GetInfoset()->GetActionProb(chanceAction));
    }
    for (size_t pl = 0; pl < numPlayers; pl++) {
      numbers.payoffs[k * numPlayers + pl] = m_outcomes[k]->GetPayoff<T>(m_players[pl]);
    }
  }
  return numbers;
}

template TreeLayout::Numbers<double> TreeLayout::BuildNumbers<double>() const;
template TreeLayout::Numbers<Rational> TreeLayout::BuildNumbers<Rational>() const;

} // end namespace Gambit
