//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/linalg/vertenum.h
// Interface to vertex enumerator
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

#ifndef GAMBIT_SOLVERS_LINALG_VERTENUM_H
#define GAMBIT_SOLVERS_LINALG_VERTENUM_H

#include <list>

#include "core/cancel.h"
#include "core/core.h"
#include "lptab.h"

namespace Gambit::linalg {

template <class T> struct VertexEnumerationResult {
  Array<BFS<T>> vertices;
  Array<BFS<T>> dualVertices;
  int numColumns;

  /// Vertices as dense vectors over the structural (column-indexed) variables
  std::list<Vector<T>> GetVertices() const;
};

/// Enumerates the vertices of the convex polyhedron
///
///     P = { y : Ay + b <= 0, y >= 0 }
///
/// where b <= 0. Enumeration starts from the vertex y = 0.
///
/// The code is based on the reverse Pivoting algorithm of Avis and Fukuda,
/// Discrete Computational Geom (1992) 8:295-313.
template <class T>
VertexEnumerationResult<T> EnumerateVertices(const Matrix<T> &A, const Vector<T> &b,
                                             const CancelToken &p_cancel = CancelToken());

} // namespace Gambit::linalg

#endif // GAMBIT_SOLVERS_LINALG_VERTENUM_H
