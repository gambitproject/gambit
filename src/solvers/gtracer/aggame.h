//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//                          Albert Xin Jiang <albertjiang@gmail.com>
//
// FILE: src/solvers/gtracer/aggame.h
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

#ifndef GAMBIT_SOLVERS_GTRACER_AGGAME_H
#define GAMBIT_SOLVERS_GTRACER_AGGAME_H

#include "cmatrix.h"
#include "gnmgame.h"
#include "games/agg/agg.h"
#include "games/gameagg.h"

namespace Gambit::gametracer {

class aggame : public gnmgame {
public:
  explicit aggame(const Gambit::GameAGGRep &g)
    : gnmgame(g.GetUnderlyingAGG()->getActionCounts()), aggPtr(g.GetUnderlyingAGG())
  {
  }

  ~aggame() override = default;
  int getNumActionNodes() { return aggPtr->getNumActionNodes(); }

  double getKSymMixedPayoff(int cls, cvector &s)
  {
    std::vector<double> sp(s.values(), s.values() + s.getm());
    return aggPtr->getKSymMixedPayoff(cls, sp);
  }

  void payoffMatrix(cmatrix &dest, const cvector &s, double fuzz) const override;

  void KSymPayoffMatrix(cmatrix &dest, const cvector &s, double fuzz) const;

  void payoffMatrix(cmatrix &dest, const cvector &s, double fuzz, bool ksym) const override
  {
    return (ksym) ? KSymPayoffMatrix(dest, s, fuzz) : payoffMatrix(dest, s, fuzz);
  }

  double getPurePayoff(int player, const std::vector<int> &s) const override
  {
    return aggPtr->getPurePayoff<double>(player, s);
  }

  void setPurePayoff(int /*player*/, const std::vector<int> & /*s*/, double /*value*/) override
  {
    throw Gambit::UndefinedException();
  }

  int getNumPlayerClasses() const override { return aggPtr->getNumPlayerClasses(); }

  int getNumKSymActions(int p) const override { return aggPtr->getNumKSymActions(p); }
  int firstKSymAction(int p) const override { return aggPtr->firstKSymAction(p); }
  int lastKSymAction(int p) const override { return aggPtr->lastKSymAction(p); }

private:
  std::shared_ptr<Gambit::agg::AGG> aggPtr;
};

} // end namespace Gambit::gametracer

#endif // GAMBIT_SOLVERS_GTRACER_AGGAME_H
