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
#include "seqpure.h"

namespace Gambit {

class GameSequenceForm {
  friend class BehaviorSupportProfile;

  BehaviorSupportProfile m_support;
  std::map<GamePlayer, std::vector<GameSequence>> m_sequences;

  void BuildSequences();

public:
  explicit GameSequenceForm(const BehaviorSupportProfile &p_support) : m_support(p_support)
  {
    BuildSequences();
  }

  ~GameSequenceForm() = default;

  const BehaviorSupportProfile &GetSupport() const { return m_support; }

  GameRep::Players GetPlayers() const { return m_support.GetGame()->GetPlayers(); }
};

} // end namespace Gambit

#endif // GAMESEQ_H
