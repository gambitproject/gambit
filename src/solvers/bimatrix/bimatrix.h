//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/bimatrix/bimatrix.h
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

#ifndef GAMBIT_SOLVERS_BIMATRIX_BIMATRIX_H
#define GAMBIT_SOLVERS_BIMATRIX_BIMATRIX_H

#include "core/matrix.h"
#include "games/game.h"

namespace Gambit::Nash {

/// @brief The payoffs of a two-player strategic game, shifted and scaled so that every entry
///        lies in (0, 1]
///
/// Entry `A1(i, j)` is player 1's normalised payoff when player 1 plays strategy `i` and
/// player 2 plays strategy `j`.  Entry `A2(j, i)` is player 2's normalised payoff in the same
/// profile.  Both matrices are indexed from 1.
template <class T> struct NormalisedBimatrix {
  Matrix<T> A1;
  Matrix<T> A2;
};

/// @brief Build the normalised bimatrix of a two-player game
/// @throws UndefinedException if the game does not have two players
/// @throws UndefinedException if the game does not have perfect recall
template <class T> NormalisedBimatrix<T> BuildNormalisedBimatrix(const Game &p_game);

} // end namespace Gambit::Nash

#endif // GAMBIT_SOLVERS_BIMATRIX_BIMATRIX_H
