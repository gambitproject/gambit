//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to vertex enumerator
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
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
#include "lptab.h"
#include "bfs.h"


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
  const gbtVector<T> &b;
  gbtVector<T> btemp,c;
  gbtList<BFS<T> > List;
  gbtList<BFS<T> > DualList;
  gbtList<gbtVector<T> > Verts;
  long npivots,nodes;
  gbtList<long> visits,branches;

  void Enum();
  void Deeper();
  void Report();
  void Search(LPTableau<T> &tab);
  void DualSearch(LPTableau<T> &tab);
public:
  VertEnum(const Gambit::Matrix<T> &, const gbtVector<T> &);
  VertEnum(LPTableau<T> &);
  virtual ~VertEnum();

  const gbtList<BFS<T> > &VertexList() const;
  const gbtList<BFS<T> > &DualVertexList() const;
  void Vertices(gbtList<gbtVector<T> > &verts) const;
  long NumPivots() const;
};
#ifdef _A
#undef _A
#endif

#endif // VERTENUM_H
