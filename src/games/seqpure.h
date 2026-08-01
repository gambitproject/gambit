//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/games/seqpure.h
// Declaration of pure sequence profile
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

#ifndef GAMBIT_GAMES_SEQPURE_H
#define GAMBIT_GAMES_SEQPURE_H

#include <map>
#include "game.h"

namespace Gambit {

/// This class represents an assignment of one sequence to each (personal)
/// player of an extensive game.  It is the analogue, for the sequence form,
/// of PureStrategyProfile for the strategic form and PureBehaviorProfile
/// for the behavior representation of the extensive form.
class PureSequenceProfile {
  Game m_efg;
  std::map<GamePlayer, GameSequence> m_profile;

public:
  /// @name Lifecycle
  //@{
  /// Construct a new sequence profile on the specified game.  No sequence
  /// is assigned to any player until SetSequence() is called.
  explicit PureSequenceProfile(const Game &);
  //@}

  Game GetGame() const { return m_efg; }

  bool operator==(const PureSequenceProfile &p_other) const
  {
    return m_profile == p_other.m_profile;
  }
  bool operator!=(const PureSequenceProfile &p_other) const { return !(*this == p_other); }

  /// @name Data access and manipulation
  //@{
  /// Get the sequence assigned to the player
  const GameSequence &GetSequence(const GamePlayer &p_player) const
  {
    return m_profile.at(p_player);
  }
  /// Assign the sequence to (its) player
  void SetSequence(const GameSequence &p_sequence)
  {
    m_profile[p_sequence->GetPlayer()] = p_sequence;
  }
  //@}

  /// Returns the payoff to a player that arises when each player realises
  /// the sequence assigned to them in this profile.  This is computed by
  /// an (iterative, non-recursive) traversal of the game tree, pruning as
  /// soon as a player's move is inconsistent with their designated
  /// sequence.
  Rational GetPayoff(const GamePlayer &p_player) const;

  /// Returns the probability, taken over chance's moves alone, that the
  /// game terminates with every player having realised exactly the
  /// sequence assigned to them in this profile.  (This need not be 1: it
  /// is less than 1 whenever some player's designated sequence is not the
  /// last move they make along a possible continuation, and 0 when the
  /// profile can never be jointly realised at all.)
  Rational GetRealizationProbability() const;
};

} // end namespace Gambit

#endif // GAMBIT_GAMES_SEQPURE_H
