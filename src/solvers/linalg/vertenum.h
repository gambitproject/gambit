//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enummixed/vertenum.h
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

#ifndef GAMBIT_LINALG_VERTENUM_H
#define GAMBIT_LINALG_VERTENUM_H

#include "gambit.h"
#include "lptab.h"
#include "bfs.h"

namespace Gambit {
namespace linalg {

//
// This class enumerates the vertices of the convex polyhedron
//
//        P = { y : Ay + b <= 0, y>=0 }
//
// where b <= 0.  Enumeration starts from the vertex y = 0.
// All computation is done in the class constructor. The
// list of vertices can be accessed by VertexList()
//
// The code is based on the reverse Pivoting algorithm of Avis
// and Fukuda, Discrete Computational Geom (1992) 8:295-313.
//
template <class T> class VertexEnumerator {
private:
  int mult_opt, depth;
  int n; // N is the number of columns, which is the # of dimensions.
  int k; // K is the number of inequalities given.
  const Matrix<T> &A;
  const Vector<T> &b;
  Vector<T> btemp, c;
  Gambit::List<BFS<T>> List;
  Gambit::List<BFS<T>> DualList;
  Gambit::List<Vector<T>> Verts;
  long npivots, nodes;
  Gambit::List<long> visits, branches;

  void Enum();
  void Deeper();
  void Search(LPTableau<T> &tab);
  void DualSearch(LPTableau<T> &tab);

public:
  VertexEnumerator(const Matrix<T> &, const Vector<T> &);
  explicit VertexEnumerator(LPTableau<T> &);
  ~VertexEnumerator() = default;

  const Gambit::List<BFS<T>> &VertexList() const { return List; }
  const Gambit::List<BFS<T>> &DualVertexList() const { return DualList; }
  void Vertices(Gambit::List<Vector<T>> &verts) const;
  long NumPivots() const { return npivots; }
};

} // namespace linalg
} // end namespace Gambit

#endif // GAMBIT_LINALG_VERTENUM_H
