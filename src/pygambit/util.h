//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/pygambit/util.h
// Convenience functions for Cython wrapper
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

//
// This file is used by the Cython wrappers to import the necessary namespaces
// and to provide some convenience functions to make interfacing with C++
// classes easier.
//

#include <algorithm>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>
#include "games/game.h"
#include "games/writer.h"
#include "games/stratspt.h"
#include "games/gameagg.h"
#include "games/gamebagg.h"
#include "solvers/nash.h"

using namespace std;
using namespace Gambit;
using namespace Gambit::Nash;

Game ParseGbtGame(std::string const &s)
{
  std::istringstream f(s);
  return ReadGbtFile(f);
}
Game ParseEfgGame(std::string const &s)
{
  std::istringstream f(s);
  return ReadEfgFile(f);
}
Game ParseNfgGame(std::string const &s)
{
  std::istringstream f(s);
  return ReadNfgFile(f);
}
Game ParseAggGame(std::string const &s)
{
  std::istringstream f(s);
  return ReadAggFile(f);
}
Game ParseBaggGame(std::string const &s)
{
  std::istringstream f(s);
  return ReadBaggFile(f);
}

std::string WriteEfgFile(const Game &p_game)
{
  std::ostringstream f;
  p_game->WriteEfgFile(f);
  return f.str();
}

std::string WriteNfgFile(const Game &p_game)
{
  std::ostringstream f;
  p_game->WriteNfgFile(f);
  return f.str();
}

std::string WriteHTMLFile(const Game &p_game)
{
  return WriteHTMLFile(p_game, p_game->GetPlayer(1), p_game->GetPlayer(2));
}

std::string WriteLaTeXFile(const Game &p_game)
{
  return WriteLaTeXFile(p_game, p_game->GetPlayer(1), p_game->GetPlayer(2));
}

std::string WriteNfgFileSupport(const StrategySupportProfile &p_support)
{
  std::ostringstream f;
  p_support.WriteNfgFile(f);
  return f.str();
}

template <template <class> class C, class T, class X>
std::shared_ptr<T> sharedcopyitem(const C<T> &p_container, const X &p_index)
{
  return make_shared<T>(p_container[p_index]);
}

// Set item p_index to value p_value in container p_container
template <class C, class X, class T>
void setitem(C *p_container, const X &p_index, const T &p_value)
{
  (*p_container)[p_index] = p_value;
}

template <class C, class X, class T>
void setitem(C &p_container, const X &p_index, const T &p_value)
{
  p_container[p_index] = p_value;
}

/// Derivatives of action values with respect to log action probabilities, as nested vectors
/// indexed from 0 by position in the profile
inline std::vector<std::vector<double>>
ProfileDiffActionValues(const MixedBehaviorProfile<double> &p_profile)
{
  Matrix<double> derivs;
  p_profile.DiffActionValues(derivs);
  std::vector<std::vector<double>> result(derivs.NumRows(),
                                          std::vector<double>(derivs.NumColumns()));
  for (size_t i = 0; i < derivs.NumRows(); i++) {
    for (size_t j = 0; j < derivs.NumColumns(); j++) {
      result[i][j] = derivs(derivs.MinRow() + i, derivs.MinCol() + j);
    }
  }
  return result;
}

/// Largest absolute difference, over every player, between the payoffs and derivatives given by
/// MixedStrategyProfile::GetPayoffDerivBlock and the same quantities computed one at a time.
/// The profile checked is on the support of p_profile's game with p_removed taken out, with the
/// probabilities of the remaining strategies taken from p_profile.  Private, for testing.
inline double PayoffDerivBlockDiscrepancy(const MixedStrategyProfile<double> &p_profile,
                                          const std::vector<GameStrategy> &p_removed)
{
  const Game game = p_profile.GetGame();
  StrategySupportProfile support(game);
  for (const auto &strategy : p_removed) {
    support.RemoveStrategy(strategy);
  }
  auto profile = support.NewMixedStrategyProfile<double>();
  for (const auto &player : game->GetPlayers()) {
    for (const auto &strategy : support.GetStrategies(player)) {
      profile[strategy] = p_profile[strategy];
    }
  }
  double worst = 0.0;
  Vector<double> values;
  Matrix<double> derivs;
  for (const auto &player : game->GetPlayers()) {
    profile.GetPayoffDerivBlock(player, values, derivs);
    size_t row = 1;
    for (const auto &strategy : support.GetStrategies(player)) {
      worst = std::max(worst, std::abs(values[row] - profile.GetPayoff(strategy)));
      size_t column = 1;
      for (const auto &other : game->GetPlayers()) {
        for (const auto &otherStrategy : support.GetStrategies(other)) {
          const double expected =
              (other == player)
                  ? 0.0
                  : profile.GetPayoffDeriv(player->GetNumber(), strategy, otherStrategy);
          worst = std::max(worst, std::abs(derivs(row, column) - expected));
          column++;
        }
      }
      row++;
    }
  }
  return worst;
}

template <class T> std::list<std::shared_ptr<T>> make_list_of_pointer(const std::list<T> &p_list)
{
  std::list<std::shared_ptr<T>> result;
  for (const auto &element : p_list) {
    result.push_back(std::make_shared<T>(element));
  }
  return result;
}
