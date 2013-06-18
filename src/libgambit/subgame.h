//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/subgame.h
// Utilities for computing and verifying subgame-perfection
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

#ifndef LIBGAMBIT_SUBGAME_H
#define LIBGAMBIT_SUBGAME_H

namespace Gambit {

typedef List<MixedBehavProfile<double> > (*DoubleSolver)(const BehavSupport &p_support);

typedef List<MixedBehavProfile<Rational> > (*RationalSolver)(const BehavSupport &p_support);


template <class T, typename SolverType>
List<MixedBehavProfile<T> > SolveBySubgames(const BehavSupport &p_support,
					    SolverType p_solver);

}

#endif   // LIBGAMBIT_SUBGAME_H



