//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//                          Albert Xin Jiang <albertjiang@gmail.com>
//
// FILE: library/include/gambit/gtracer/aggame.h
// Interface to GNM-specific routines for action graph games
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

#ifndef GAMBIT_GTRACER_AGGAME_H
#define GAMBIT_GTRACER_AGGAME_H

#include "cmatrix.h"
#include "gnmgame.h"
#include "games/agg/agg.h"
#include "gambit.h"
#include "games/gameagg.h"

namespace Gambit {
namespace gametracer {

class aggame : public gnmgame {
public:
  explicit aggame(const Gambit::GameAGGRep &g)
    : gnmgame(g.GetUnderlyingAGG()->actions), aggPtr(g.GetUnderlyingAGG())
  {
  }

  ~aggame() override = default;
  int getNumActionNodes() { return aggPtr->getNumActionNodes(); }

  double getMixedPayoff(int player, const cvector &s) const override
  {
    std::vector<double> sp(s.values(), s.values() + s.getm());
    return (double)aggPtr->getMixedPayoff(player, sp);
  }

  double getKSymMixedPayoff(int cls, cvector &s)
  {
    std::vector<double> sp(s.values(), s.values() + s.getm());
    return (double)aggPtr->getKSymMixedPayoff(cls, sp);
  }

  void payoffMatrix(cmatrix &dest, const cvector &s, double fuzz) const override;

  void KSymPayoffMatrix(cmatrix &dest, const cvector &s, double fuzz) const;

  void payoffMatrix(cmatrix &dest, const cvector &s, double fuzz, bool ksym) const override
  {
    return (ksym) ? KSymPayoffMatrix(dest, s, fuzz) : payoffMatrix(dest, s, fuzz);
  }

  void getPayoffVector(cvector &dest, int player, const cvector &s) const override
  {
    auto ss = const_cast<cvector &>(s);
    std::vector<double> sp(ss.values(), ss.values() + ss.getm());
    std::vector<double> d(aggPtr->getNumActions(player));
    aggPtr->getPayoffVector(d, player, sp);
    std::copy(d.begin(), d.end(), dest.values());
  }
  double getPurePayoff(int player, const std::vector<int> &s) const override
  {
    return aggPtr->getPurePayoff(player, s);
  }

  void setPurePayoff(int player, const std::vector<int> &s, double value) override
  {
    throw Gambit::UndefinedException();
  }

  int getNumPlayerClasses() const override { return aggPtr->getNumPlayerClasses(); }

  int getNumKSymActions(int p) const override { return aggPtr->getNumKSymActions(p); }
  int firstKSymAction(int p) const override { return aggPtr->firstKSymAction(p); }
  int lastKSymAction(int p) const override { return aggPtr->lastKSymAction(p); }

private:
  std::shared_ptr<Gambit::agg::AGG> aggPtr;

  // helper functions for computing jacobian
  void computePartialP_PureNode(int player, int act, std::vector<int> &tasks) const;
  void computePartialP_bisect(int player, int act, std::vector<int>::iterator f,
                              std::vector<int>::iterator l, Gambit::agg::aggdistrib &temp) const;
  void computePayoff(cmatrix &dest, int player1, int act1, int player2, int act2,
                     Gambit::agg::trie_map<Gambit::agg::AggNumber> &cache) const;
  void savePayoff(cmatrix &dest, int player1, int act1, int player2, int act2,
                  Gambit::agg::AggNumber result,
                  Gambit::agg::trie_map<Gambit::agg::AggNumber> &cache,
                  bool partial = false) const;
  void computeUndisturbedPayoff(Gambit::agg::AggNumber &undisturbedPayoff, bool &has, int player1,
                                int act1, int player2) const;
};

} // namespace gametracer
} // end namespace Gambit

#endif // GAMBIT_GTRACER_AGGAME_H
