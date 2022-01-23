//
// This file is part of Gambit
// Copyright (c) 1994-2022, The Gambit Project (http://www.gambit-project.org)
//
// FILE: library/include/gambit/nash/gnm.h
// Compute Nash equilibria via the global Newton method
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

#ifndef GAMBIT_NASH_GNM_H
#define GAMBIT_NASH_GNM_H

#include "games/nash.h"
#include "solvers/gtracer/gtracer.h"

namespace Gambit {
namespace Nash {

class NashGNMStrategySolver : public StrategySolver<double> {
public:
  NashGNMStrategySolver(shared_ptr<StrategyProfileRenderer<double> > p_onEquilibrium = 0,
			bool p_verbose=false)
    : StrategySolver<double>(p_onEquilibrium),
      m_verbose(p_verbose)
  { }
  virtual ~NashGNMStrategySolver() { }

  List<MixedStrategyProfile<double> > Solve(const Game &p_game) const;
  List<MixedStrategyProfile<double> > Solve(const Game &p_game,
					    const MixedStrategyProfile<double> &p_pert) const;

private:
  bool m_verbose;
  
  List<MixedStrategyProfile<double> > Solve(const Game &p_game,
					    shared_ptr<gametracer::gnmgame> A,
					    const gametracer::cvector &p_pert) const;
  shared_ptr<gametracer::gnmgame> BuildRepresentation(const Game &p_game) const;

  static MixedStrategyProfile<double> ToProfile(const Game &p_game,
						const gametracer::cvector &p_pert);
};

}  // end namespace Gambit::Nash
}  // end namespace Gambit

#endif  // GAMBIT_NASH_GNM_H
