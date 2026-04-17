//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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

void GameSequenceForm::IndexSequences()
{
  for (const auto &player : GetPlayers()) {
    for (const auto &sequence : player->GetSequences()) {
      if (!sequence->GetAction() || m_support.Contains(sequence->GetAction())) {
        m_sequences[player].emplace_back(sequence);
        if (sequence->GetAction()) {
          m_correspondence[sequence->GetAction()] = sequence;
        }
      }
    }
  }
}

void GameSequenceForm::FillTableau()
{
  struct Callback {
    GameSequenceForm &m_sfg;
    const BehaviorSupportProfile &m_support;

    std::map<GamePlayer, std::vector<GameSequence>> m_sequences;
    std::vector<GameOutcome> m_outcomes;
    std::vector<Rational> m_probs;

    Callback(GameSequenceForm &p_sfg, const BehaviorSupportProfile &p_support)
      : m_sfg(p_sfg), m_support(p_support)
    {
      for (auto player : m_sfg.GetPlayers()) {
        m_sequences[player].push_back(m_sfg.m_sequences.at(player).front());
      }
    }

    GameRep::DFSCallbackResult OnEnter(const GameNode &p_node, int)
    {
      if (p_node->GetParent()) {
        const auto parent = p_node->GetParent();
        const auto player = parent->GetPlayer();
        const auto action = p_node->GetPriorAction();

        if (player->IsChance()) {
          m_probs.push_back(static_cast<Rational>(action->GetInfoset()->GetActionProb(action)));
        }
        else {
          m_sequences[player].push_back(m_sfg.m_correspondence.at(action));
        }
      }

      if (p_node->GetOutcome()) {
        m_outcomes.push_back(p_node->GetOutcome());
      }

      if (p_node->GetInfoset() && !p_node->GetPlayer()->IsChance()) {
        const auto player = p_node->GetPlayer();
        const auto currentSequence = m_sequences.at(player).back();
        m_sfg.m_constraints[{p_node->GetInfoset(), currentSequence->m_action}] = 1;
      }

      return GameRep::DFSCallbackResult::Continue;
    }

    GameRep::DFSCallbackResult OnAction(const GameNode &p_node, const GameNode &p_child, int) const
    {
      const auto action = p_child->GetPriorAction();
      if (!action) {
        return GameRep::DFSCallbackResult::Continue;
      }

      if (!action->GetInfoset()->IsChanceInfoset()) {
        if (!m_support.Contains(action)) {
          return GameRep::DFSCallbackResult::Prune;
        }

        m_sfg.m_constraints[{p_node->GetInfoset(), action}] = -1;
      }

      return GameRep::DFSCallbackResult::Continue;
    }

    void OnVisit(const GameNode &p_node, int)
    {
      if (!p_node->IsTerminal()) {
        return;
      }

      std::map<GamePlayer, GameSequence> currentSequence;
      for (auto player : m_sfg.GetPlayers()) {
        currentSequence[player] = m_sequences.at(player).back();
      }

      const auto prob =
          std::accumulate(m_probs.begin(), m_probs.end(), Rational(1), std::multiplies<>());

      for (auto player : m_support.GetGame()->GetPlayers()) {
        for (const auto &outcome : m_outcomes) {
          m_sfg.GetPayoffEntry(currentSequence, player) +=
              prob * outcome->GetPayoff<Rational>(player);
        }
      }
    }

    GameRep::DFSCallbackResult OnExit(const GameNode &p_node, int)
    {
      if (p_node->GetParent()) {
        const auto parent = p_node->GetParent();
        const auto player = parent->GetPlayer();

        if (player->IsChance()) {
          m_probs.pop_back();
        }
        else {
          m_sequences[player].pop_back();
        }
      }

      if (p_node->GetOutcome()) {
        m_outcomes.pop_back();
      }

      return GameRep::DFSCallbackResult::Continue;
    }
  };

  Array<int> dim(m_sequences.size());
  for (auto player : GetPlayers()) {
    dim[player->GetNumber()] = m_sequences.at(player).size();
  }
  m_payoffs = NDArray<Rational>(dim, dim.size());

  std::map<GamePlayer, GameSequence> currentSequence;
  for (auto player : GetPlayers()) {
    currentSequence[player] = m_sequences[player].front();
  }

  Callback callback(*this, m_support);
  GameRep::WalkDFS(m_support.GetGame(), m_support.GetGame()->GetRoot(), TraversalOrder::Preorder,
                   callback);
}

} // end namespace Gambit
