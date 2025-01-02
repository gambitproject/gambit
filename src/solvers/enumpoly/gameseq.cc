//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/solvers/enumpoly/sfg.cc
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

#include "gambit.h"
#include "gameseq.h"

using namespace Gambit;

namespace Gambit {

void GameSequenceForm::BuildSequences(const GameNode &n,
                                      std::map<GamePlayer, GameSequence> &p_currentSequences)
{
  if (!n->GetInfoset()) {
    return;
  }
  if (n->GetPlayer()->IsChance()) {
    for (auto child : n->GetChildren()) {
      BuildSequences(child, p_currentSequences);
    }
  }
  else {
    m_infosets.insert(n->GetInfoset());
    auto tmp_sequence = p_currentSequences.at(n->GetPlayer());
    for (auto action : m_support.GetActions(n->GetInfoset())) {
      if (m_correspondence.find(action) == m_correspondence.end()) {
        m_sequences[n->GetPlayer()].push_back(std::make_shared<GameSequenceRep>(
            n->GetPlayer(), action, m_sequences[n->GetPlayer()].size() + 1, tmp_sequence));
        m_correspondence[action] = m_sequences[n->GetPlayer()].back();
      }
      p_currentSequences[n->GetPlayer()] = m_correspondence[action];
      BuildSequences(n->GetChild(action), p_currentSequences);
    }
    p_currentSequences[n->GetPlayer()] = tmp_sequence;
  }
}

void GameSequenceForm::BuildSequences()
{
  std::map<GamePlayer, GameSequence> currentSequences;
  for (auto player : GetPlayers()) {
    m_sequences[player] = {
        std::make_shared<GameSequenceRep>(player, nullptr, 1, std::weak_ptr<GameSequenceRep>())};
    currentSequences[player] = m_sequences[player].front();
  }
  BuildSequences(m_support.GetGame()->GetRoot(), currentSequences);
}

void GameSequenceForm::FillTableau(const GameNode &n, const Rational &prob,
                                   std::map<GamePlayer, GameSequence> &p_currentSequences)
{
  if (n->GetOutcome()) {
    for (auto player : m_support.GetGame()->GetPlayers()) {
      GetPayoffEntry(p_currentSequences, player) +=
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
                  p_currentSequences);
    }
  }
  else {
    auto tmp_sequence = p_currentSequences.at(n->GetPlayer());
    m_constraints[{n->GetInfoset(), p_currentSequences.at(n->GetPlayer())->action}] = 1;
    for (auto action : m_support.GetActions(n->GetInfoset())) {
      m_constraints[{n->GetInfoset(), action}] = -1;
      p_currentSequences[n->GetPlayer()] = m_correspondence.at(action);
      FillTableau(n->GetChild(action), prob, p_currentSequences);
    }
    p_currentSequences[n->GetPlayer()] = tmp_sequence;
  }
}

void GameSequenceForm::FillTableau()
{
  Array<int> dim(m_sequences.size());
  for (auto player : GetPlayers()) {
    dim[player->GetNumber()] = m_sequences.at(player).size();
  }
  m_payoffs = NDArray<Rational>(dim, dim.size());

  std::map<GamePlayer, GameSequence> currentSequence;
  for (auto player : GetPlayers()) {
    currentSequence[player] = m_sequences[player].front();
  }
  FillTableau(m_support.GetGame()->GetRoot(), Rational(1), currentSequence);
}

MixedBehaviorProfile<double>
GameSequenceForm::ToMixedBehaviorProfile(const std::map<GameSequence, double> &x) const
{
  MixedBehaviorProfile<double> b(m_support);
  for (auto sequence : GetSequences()) {
    if (sequence->action == nullptr) {
      continue;
    }
    if (double parent_prob = x.at(sequence->parent.lock()) > 0) {
      b[sequence->action] = x.at(sequence) / parent_prob;
    }
    else {
      b[sequence->action] = 0;
    }
  }
  return b;
}

} // end namespace Gambit
