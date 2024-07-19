//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/sfg.h
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

#ifndef SFG_H
#define SFG_H

#include <memory>

#include "gambit.h"
#include "odometer.h"
#include "ndarray.h"

namespace Gambit {

class Sfg {
  using PureSequenceProfile = std::map<GamePlayer, GameAction>;

private:
  BehaviorSupportProfile support;
  std::map<GamePlayer, std::map<GameAction, int>> m_sequenceColumns;
  NDArray<Rational> SF;                        // sequence form
  std::map<GamePlayer, RectArray<Rational>> E; // constraint matrices for sequence form.
  std::map<GamePlayer, std::map<GameInfoset, int>> infoset_row;
  std::map<GamePlayer, std::map<GameAction, GameAction>> m_actionParents;

  void BuildSequences(const GameNode &, PureSequenceProfile &);
  void FillTableau(const GameNode &, const Rational &, PureSequenceProfile &);

  Rational &GetMatrixEntry(const PureSequenceProfile &, const GamePlayer &);
  Rational &GetConstraintEntry(const GameInfoset &, const GameAction &);

public:
  explicit Sfg(const BehaviorSupportProfile &);

  ~Sfg() = default;

  int NumSequences(const GamePlayer &p_player) const
  {
    return m_sequenceColumns.at(p_player).size();
  }

  int NumInfosets(const GamePlayer &p_player) const { return infoset_row.at(p_player).size(); }

  const Array<int> &NumSequences() const { return SF.GetIndexDimension(); }

  int TotalNumSequences() const
  {
    auto &dim = SF.GetIndexDimension();
    return std::accumulate(dim.cbegin(), dim.cend(), 0);
  }

  int NumPlayerInfosets() const
  {
    auto players = support.GetGame()->GetPlayers();
    return std::accumulate(
        players.cbegin(), players.cend(), 0,
        [this](int accum, const GamePlayer &player) { return accum + NumInfosets(player); });
  }

  int NumPlayers() const { return support.GetGame()->NumPlayers(); }

  const Rational &GetPayoff(const Array<int> &index, int pl) const { return SF.at(index, pl); }

  const Rational &GetConstraintEntry(const GamePlayer &p_player, int infoset_row,
                                     int seq_col) const
  {
    return E.at(p_player)(infoset_row, seq_col);
  }

  int InfosetRowNumber(const GamePlayer &p_player, int sequence) const
  {
    if (sequence == 1) {
      return 0;
    }
    for (auto entry : m_sequenceColumns.at(p_player)) {
      if (entry.second == sequence) {
        return infoset_row.at(p_player).at(entry.first->GetInfoset());
      }
    }
    return 0;
  }

  int ActionNumber(const GamePlayer &p_player, int sequence) const
  {
    if (sequence == 1) {
      return 0;
    }
    for (auto entry : m_sequenceColumns.at(p_player)) {
      if (entry.second == sequence) {
        return support.GetIndex(entry.first);
      }
    }
    return 0;
  }

  GameInfoset GetInfoset(const GamePlayer &p_player, int sequence) const
  {
    if (sequence == 1) {
      return 0;
    }
    for (auto entry : m_sequenceColumns.at(p_player)) {
      if (entry.second == sequence) {
        return entry.first->GetInfoset();
      }
    }
    return 0;
  }

  MixedBehaviorProfile<double> ToBehav(const PVector<double> &x) const;
};

} // end namespace Gambit

#endif // SFG_H
