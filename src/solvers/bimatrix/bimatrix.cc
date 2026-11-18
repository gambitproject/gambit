//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/bimatrix/bimatrix.cc
// Normalised payoff matrices of a two-player strategic game
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

#include <algorithm>

#include "games.h"
#include "solvers/bimatrix/bimatrix.h"
#include "solvers/nash.h"

namespace Gambit::Nash {

template <class T> NormalisedBimatrix<T> BuildNormalisedBimatrix(const Game &p_game)
{
  if (p_game->NumPlayers() != 2) {
    throw UndefinedException("Method only valid for two-player games.");
  }
  RequirePerfectRecall(p_game);

  const GamePlayer player1 = p_game->GetPlayer(1);
  const GamePlayer player2 = p_game->GetPlayer(2);
  const int n1 = player1->GetStrategies().size();
  const int n2 = player2->GetStrategies().size();

  const Rational min = std::min(p_game->GetMinPayoff(), Rational(0)) - Rational(1);
  const Rational max = std::max(p_game->GetMaxPayoff(), Rational(0));
  const Rational fac = Rational(1) / (max - min);

  NormalisedBimatrix<T> result{Matrix<T>(1, n1, 1, n2), Matrix<T>(1, n2, 1, n1)};
  const PureStrategyProfile profile = p_game->NewPureStrategyProfile();
  for (int i = 1; i <= n1; i++) {
    profile->SetStrategy(player1->GetStrategy(i));
    for (int j = 1; j <= n2; j++) {
      profile->SetStrategy(player2->GetStrategy(j));
      result.A1(i, j) = static_cast<T>(fac * (profile->GetPayoff(player1) - min));
      result.A2(j, i) = static_cast<T>(fac * (profile->GetPayoff(player2) - min));
    }
  }
  return result;
}

template NormalisedBimatrix<double> BuildNormalisedBimatrix(const Game &);
template NormalisedBimatrix<Rational> BuildNormalisedBimatrix(const Game &);

} // end namespace Gambit::Nash
