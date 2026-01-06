//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/libgambit/gameexpl.h
// Declaration of base class for explicit game representations
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

#ifndef GAMEEXPL_H
#define GAMEEXPL_H

#include "game.h"

namespace Gambit {

class GameExplicitRep : public GameRep {
  template <class T> friend class MixedStrategyProfile;

public:
  /// @name General data access
  //@{
  /// Returns the smallest payoff to any player in any play of the game
  Rational GetMinPayoff() const override;
  /// Returns the largest payoff to any player in any play of the game
  Rational GetMaxPayoff() const override;
  //@}

  /// @name Outcomes
  //@{
  /// Creates a new outcome in the game
  GameOutcome NewOutcome() override;

  /// @name Writing data files
  //@{
  void Write(std::ostream &p_stream, const std::string &p_format = "native") const override;
  //@}
};

} // namespace Gambit

#endif // GAMEEXPL_H
