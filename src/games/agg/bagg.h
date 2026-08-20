//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//                          Albert Xin Jiang <albertjiang@gmail.com>
//
// FILE: src/games/agg/bagg.h
// Interface of Bayesian Action Graph Game representation
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

#ifndef GAMBIT_GAMES_AGG_BAGG_H
#define GAMBIT_GAMES_AGG_BAGG_H

#include <iostream>
#include "agg.h"

namespace Gambit {

class BAGGPureStrategyProfileRep;
template <class T> class BAGGMixedStrategyProfileRep;

namespace agg {

using ProbDist = std::vector<double>;

class BAGG {
public:
  friend class Gambit::BAGGPureStrategyProfileRep;
  template <class T> friend class Gambit::BAGGMixedStrategyProfileRep;

  friend std::ostream &operator<<(std::ostream &s, const BAGG &g);

  static std::shared_ptr<BAGG> makeBAGG(std::istream &in);

  BAGG(int N, int S, std::vector<int> &numTypes, std::vector<ProbDist> &TDist,
       std::vector<std::vector<Number>> &exactTDist,
       std::vector<std::vector<std::vector<int>>> &typeActionSets,
       std::vector<std::vector<std::vector<int>>> &ta2a, std::shared_ptr<AGG> aggPtr);

  ~BAGG() = default;

  int getNumPlayers() const { return numPlayers; }
  int getNumTypes() const { return typeOffset[numPlayers]; }
  int getNumTypes(int i) const { return numTypes[i]; }
  int getNumActions(int pl, int t) const { return typeActionSets[pl][t].size(); }

  int getNumActionNodes() const { return numActionNodes; }
  int getNumFunctionNodes() const { return aggPtr->getNumFunctionNodes(); }

  int firstAction(int pl, int t) const { return strategyOffset[typeOffset[pl] + t]; }
  int lastAction(int pl, int t) const { return strategyOffset[typeOffset[pl] + t + 1]; }

  template <class V> V getMaxPayoff() const { return aggPtr->getMaxPayoff<V>(); }
  template <class V> V getMinPayoff() const { return aggPtr->getMinPayoff<V>(); }

  // exp. payoff under mixed strat profile, summed over player's types. No Rational counterpart;
  // callers go through the per-type overload below.
  double getMixedPayoff(int player, StrategyProfile<double> &s);

  // exp payoff for player, conditioned on her receiving type tp, via the convolution algorithm.
  // V=double runs the floating-point engine; V=Rational runs the exact-arithmetic counterpart
  // throughout -- see agg.h::getMixedPayoff for how the dispatch works.
  template <class V> V getMixedPayoff(int player, int tp, const StrategyProfile<V> &s) const;

  void getPayoffVector(std::vector<double> &dest, int player, int tp,
                       const StrategyProfile<double> &s);
  template <class V> V getV(int player, int tp, int action, const StrategyProfile<V> &s) const;

  // payoff for player, conditioned on receiving type tp, under the pure profile s, via the
  // degenerate mixed profile and the convolution algorithm (mirrors AGG::getPurePayoff<V>).
  template <class V> V getPurePayoff(int player, int tp, const std::vector<int> &s) const;
  double getPurePayoff(int player, std::vector<int> &s)
  {
    double r = 0;
    for (int i = 0; i < numTypes[player]; ++i) {
      r += indepTypeDist[player][i] * getPurePayoff<double>(player, i, s);
    }
    return r;
  }

  double getSymMixedPayoff(StrategyProfile<double> &s);

  double getSymMixedPayoff(int tp, StrategyProfile<double> &s);

  double getSymMixedPayoff(int tp, int act, StrategyProfile<double> &s);

  bool isSymmetric() const { return symmetric; }

private:
  int numPlayers;
  int numActionNodes;
  std::vector<int> numTypes;
  std::vector<ProbDist> indepTypeDist;
  std::vector<std::vector<Number>> exactIndepTypeDist;

  // for each player, each of her types, the set of actions
  std::vector<std::vector<std::vector<int>>> typeActionSets;

  std::vector<std::vector<std::vector<int>>> typeAction2ActionIndex;

  std::vector<int> typeOffset;
  std::vector<int> strategyOffset;

  // AGG that stores the action graph and utility functions
  std::shared_ptr<AGG> aggPtr;

  bool symmetric;

  template <class V>
  void getAGGStrat(StrategyProfile<V> &as, const StrategyProfile<V> &s, int player = -1,
                   int tp = -1, int action = -1) const;
  void getSymAGGStrat(StrategyProfile<double> &as, const StrategyProfile<double> &s);
};

std::ostream &operator<<(std::ostream &s, const BAGG &g);

} // namespace agg

} // end namespace Gambit

#endif // GAMBIT_GAMES_AGG_BAGG_H
