//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/sfg.cc
// Implementation of sequence form classes
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

#include "sfg.h"
#include "gambit.h"

using namespace Gambit;

namespace Gambit {

Sfg::Sfg(const BehaviorSupportProfile &S) : support(S)
{
  for (auto player : support.GetGame()->GetPlayers()) {
    m_actionParents[player] = {};
  }

  PureSequenceProfile parent;
  for (auto player : support.GetGame()->GetPlayers()) {
    parent[player] = nullptr;
  }
  BuildSequences(support.GetGame()->GetRoot(), parent);

  Array<int> dim(support.GetGame()->NumPlayers());
  for (int pl = 1; pl <= support.GetGame()->NumPlayers(); pl++) {
    dim[pl] = m_actionParents[support.GetGame()->GetPlayer(pl)].size() + 1;
  }
  SF = NDArray<Rational>(dim, support.GetGame()->NumPlayers());

  for (auto player : support.GetGame()->GetPlayers()) {
    m_sequenceColumns[player] = {{nullptr, 1}};
    E.try_emplace(player, infoset_row[player].size() + 1, m_actionParents[player].size() + 1);
    E.at(player)(1, 1) = Rational(1);
    parent[player] = nullptr;
  }

  FillTableau(support.GetGame()->GetRoot(), Rational(1), parent);
}

Rational &Sfg::GetMatrixEntry(const PureSequenceProfile &profile, const GamePlayer &player)
{
  Array<int> index(profile.size());
  for (int pl = 1; pl <= support.GetGame()->NumPlayers(); pl++) {
    index[pl] = m_sequenceColumns.at(support.GetGame()->GetPlayer(pl))
                    .at(profile.at(support.GetGame()->GetPlayer(pl)));
  }
  return SF.at(index, player->GetNumber());
}

Rational &Sfg::GetConstraintEntry(const GameInfoset &infoset, const GameAction &action)
{
  return E.at(infoset->GetPlayer())(infoset_row[infoset->GetPlayer()][infoset],
                                    m_sequenceColumns[infoset->GetPlayer()][action]);
}

void Sfg::BuildSequences(const GameNode &n, PureSequenceProfile &p_previousActions)
{
  if (!n->GetInfoset()) {
    return;
  }
  if (n->GetPlayer()->IsChance()) {
    for (auto child : n->GetChildren()) {
      BuildSequences(child, p_previousActions);
    }
  }
  else {
    auto &player_infoset_row = infoset_row[n->GetPlayer()];
    if (player_infoset_row.find(n->GetInfoset()) == player_infoset_row.end()) {
      player_infoset_row[n->GetInfoset()] = player_infoset_row.size() + 1;
    }
    for (auto action : support.GetActions(n->GetInfoset())) {
      m_actionParents[n->GetPlayer()][action] = p_previousActions[n->GetPlayer()];
      p_previousActions[n->GetPlayer()] = action;
      BuildSequences(n->GetChild(action), p_previousActions);
      p_previousActions[n->GetPlayer()] = m_actionParents[n->GetPlayer()][action];
    }
  }
}

void Sfg::FillTableau(const GameNode &n, const Rational &prob,
                      PureSequenceProfile &previousActions)
{
  if (n->GetOutcome()) {
    for (auto player : support.GetGame()->GetPlayers()) {
      GetMatrixEntry(previousActions, player) +=
          prob * static_cast<Rational>(n->GetOutcome()->GetPayoff(player));
    }
  }
  if (!n->GetInfoset()) {
    return;
  }
  if (n->GetPlayer()->IsChance()) {
    for (auto action : n->GetInfoset()->GetActions()) {
      FillTableau(n->GetChild(action),
                  prob * static_cast<Rational>(n->GetInfoset()->GetActionProb(action)),
                  previousActions);
    }
  }
  else {
    GetConstraintEntry(n->GetInfoset(), previousActions.at(n->GetPlayer())) = Rational(1);
    for (auto action : support.GetActions(n->GetInfoset())) {
      if (m_sequenceColumns[n->GetPlayer()].find(action) ==
          m_sequenceColumns[n->GetPlayer()].end()) {
        m_sequenceColumns[n->GetPlayer()][action] = m_sequenceColumns[n->GetPlayer()].size() + 1;
      }
      previousActions[n->GetPlayer()] = action;
      GetConstraintEntry(n->GetInfoset(), action) = Rational(-1);
      FillTableau(n->GetChild(action), prob, previousActions);
      previousActions[n->GetPlayer()] = m_actionParents[n->GetPlayer()][action];
    }
  }
}

MixedBehaviorProfile<double> Sfg::ToBehav(const PVector<double> &x) const
{
  MixedBehaviorProfile<double> b(support);
  b = 0;

  for (int i = 1; i <= support.GetGame()->NumPlayers(); i++) {
    GamePlayer player = support.GetGame()->GetPlayer(i);
    for (auto action_entry : m_sequenceColumns.at(player)) {
      if (action_entry.first == nullptr) {
        continue;
      }
      auto parent = m_actionParents.at(player).at(action_entry.first);
      auto parent_prob = x(player->GetNumber(), m_sequenceColumns.at(player).at(parent));
      if (parent_prob > 0) {
        b[action_entry.first] = x(player->GetNumber(), action_entry.second) / parent_prob;
      }
    }
  }
  return b;
}

} // end namespace Gambit
