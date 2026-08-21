//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/linalg/lpsolve.h
// Interface to LP solvers
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

#ifndef GAMBIT_SOLVERS_LINALG_LPSOLVE_H
#define GAMBIT_SOLVERS_LINALG_LPSOLVE_H

#include "core/cancel.h"
#include "core/core.h"
#include "lptab.h"

namespace Gambit::linalg {

/// The outcome of solving a linear program of the form
/// maximize c x subject to A x <= b, x >= 0 (the last `nequals` rows of A
/// holding with equality). The components of `primalSolution` and `bfs` are
/// indexed by the columns of A, with the excess columns representing the
/// artificial and slack variables.
template <class T> struct LPSolveResult {
  bool wellFormed{true};
  bool feasible{true};
  bool bounded{true};
  T cost{0};
  Vector<T> primalSolution;
  BFS<T> bfs;
  LPTableau<T> tableau;
};

/// Solves the LP maximize c x subject to A x <= b, x >= 0, where the last
/// `nequals` rows of A hold with equality.
template <class T>
LPSolveResult<T> SolveLP(const Matrix<T> &A, const Vector<T> &b, const Vector<T> &c, int nequals,
                         const CancelToken &p_cancel = CancelToken());

} // end namespace Gambit::linalg

#endif // GAMBIT_SOLVERS_LINALG_LPSOLVE_H
