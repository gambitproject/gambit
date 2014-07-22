//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/liap/nfgliap.h
// Compute Nash equilibria by minimizing Liapunov function on strategic game
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

#ifndef NFGLIAP_H
#define NFGLIAP_H

#include "libgambit/nash.h"

using namespace Gambit;

class NashLiapStrategySolver : public NashStrategySolver<double> {
public:
  NashLiapStrategySolver(int p_maxitsN, bool p_verbose = false,
			 shared_ptr<StrategyProfileRenderer<double> > p_onEquilibrium = 0)
    : NashStrategySolver<double>(p_onEquilibrium),
      m_maxitsN(p_maxitsN), m_verbose(p_verbose)
  { }
  virtual ~NashLiapStrategySolver() { }

  List<MixedStrategyProfile<double> > Solve(const MixedStrategyProfile<double> &p_start) const;
  List<MixedStrategyProfile<double> > Solve(const Game &p_game) const
    { return Solve(p_game->NewMixedStrategyProfile(0.0)); }

private:
  int m_maxitsN;
  bool m_verbose;
};

#endif  // NFGLIAP_H
