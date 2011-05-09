//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
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

#ifndef VERTENUM_H
#define VERTENUM_H

#include "libgambit/libgambit.h"
#include "liblinear/lptab.h"
#include "liblinear/bfs.h"


//
// This class enumerates the vertices of the convex polyhedron 
//
//        P = {y:Ay + b <= 0, y>=0 }
// 
// where b <= 0.  Enumeration starts from the vertex y = 0.
// All computation is done in the class constructor. The 
// list of vertices can be accessed by VertexList()
//  
// The code is based on the reverse Pivoting algorithm of Avis 
// and Fukuda, Discrete Computational Geom (1992) 8:295-313.
//

template <class T> class VertEnum {
private:
  int mult_opt,depth;
  int n;  // N is the number of columns, which is the # of dimensions.
  int k;  // K is the number of inequalities given.
    // Removed const on A and b (Geoff)
  const Gambit::Matrix<T> &A;   
  const Gambit::Vector<T> &b;
  Gambit::Vector<T> btemp,c;
  Gambit::List<BFS<T> > List;
  Gambit::List<BFS<T> > DualList;
  Gambit::List<Gambit::Vector<T> > Verts;
  long npivots,nodes;
  Gambit::List<long> visits,branches;

  void Enum();
  void Deeper();
  void Report();
  void Search(LPTableau<T> &tab);
  void DualSearch(LPTableau<T> &tab);
public:
  VertEnum(const Gambit::Matrix<T> &, const Gambit::Vector<T> &);
  VertEnum(LPTableau<T> &);
  virtual ~VertEnum();

  const Gambit::List<BFS<T> > &VertexList() const;
  const Gambit::List<BFS<T> > &DualVertexList() const;
  void Vertices(Gambit::List<Gambit::Vector<T> > &verts) const;
  long NumPivots() const;
};
#ifdef _A
#undef _A
#endif

#endif // VERTENUM_H
