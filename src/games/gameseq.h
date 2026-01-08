//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/enumpoly/gameseq.h
// Interface to sequence form classes
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

#ifndef GAMESEQ_H
#define GAMESEQ_H

#include "gambit.h"
#include "ndarray.h"

namespace Gambit {

class GameSequenceForm {
  friend class BehaviorSupportProfile;

  BehaviorSupportProfile m_support;
  std::map<GamePlayer, std::vector<GameSequence>> m_sequences;
  NDArray<Rational> m_payoffs;
  std::map<std::pair<GameInfoset, GameAction>, int> m_constraints; // (sparse) constraint matrices
  std::set<GameInfoset> m_infosets; // infosets actually reachable given support
  std::map<GameAction, GameSequence> m_correspondence;

  void BuildSequences();
  void BuildSequences(const GameNode &, std::map<GamePlayer, GameSequence> &);
  void FillTableau();
  void FillTableau(const GameNode &, const Rational &, std::map<GamePlayer, GameSequence> &);

  Array<int> ProfileToIndex(const std::map<GamePlayer, GameSequence> &p_profile) const
  {
    Array<int> index(p_profile.size());
    for (auto player : GetPlayers()) {
      index[player->GetNumber()] = p_profile.at(player)->number;
    }
    return index;
  }

  Rational &GetPayoffEntry(const std::map<GamePlayer, GameSequence> &p_profile,
                           const GamePlayer &p_player)
  {
    return m_payoffs.at(ProfileToIndex(p_profile), p_player->GetNumber());
  }

public:
  class Infosets {
    const GameSequenceForm *m_sfg;

  public:
    Infosets(const GameSequenceForm *p_sfg) : m_sfg(p_sfg) {}

    size_t size() const { return m_sfg->m_infosets.size(); }
  };

  explicit GameSequenceForm(const BehaviorSupportProfile &p_support) : m_support(p_support)
  {
    BuildSequences();
    FillTableau();
  }

  ~GameSequenceForm() = default;

  const BehaviorSupportProfile &GetSupport() const { return m_support; }

  GameRep::Players GetPlayers() const { return m_support.GetGame()->GetPlayers(); }

  const Rational &GetPayoff(const std::map<GamePlayer, GameSequence> &p_profile,
                            const GamePlayer &p_player) const
  {
    return m_payoffs.at(ProfileToIndex(p_profile), p_player->GetNumber());
  }

  int GetConstraintEntry(const GameInfoset &p_infoset, const GameAction &p_action) const
  {
    try {
      return m_constraints.at({p_infoset, p_action});
    }
    catch (std::out_of_range &) {
      return 0;
    }
  }
};

} // end namespace Gambit

#endif // GAMESEQ_H
