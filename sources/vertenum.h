//#
//# FILE: vertenum.h -- Vertex Enumeration module
//#
//# $Id$
//#

#ifndef VERTENUM_H
#define VERTENUM_H

#include "tableau.h"
#include "bfs.h"
#include "gstatus.h"

//
// This class enumerates the vertices of the convex polyhedron 
//
//        P = {y:Ay <= b, y>=0 }
// 
// where b >= 0.  Enumeration starts from the vertex y = 0.
// All computation is done in the class constructor. The 
// list of vertices can be accessed by VertexList()
//  
// The code is based on the reverse Pivoting algorithm of Avis 
// and Fukuda, Discrete Computational Geom (1992) 8:295-313.
//

template <class T> class VertEnum {
private:
  int mult_opt,depth;
  const gMatrix<T> &A;   
  const gVector<T> &b;
  gVector<T> btemp,c;
  gList<BFS<T> > List;
  gList<gVector<T> > Verts;
  long npivots,nodes;
  gStatus &status;
  gList<long> visits,branches;

  void Enum();
  void Deeper();
  void Report();
  void Search(LPTableau<T> &tab);
  void DualSearch(LPTableau<T> &tab);
public:
  VertEnum(const gMatrix<T> &A, const gVector<T> &b, gStatus &status_ = gstatus);
  VertEnum(const gMatrix<T> &A, const gVector<T> &b, 
	   const gVector<T> &start, gStatus &status_ = gstatus);
  ~VertEnum();

  const gList<BFS<T> > &VertexList() const;
  void Vertices(gList<gVector<T> > &verts) const;
  long NumPivots() const;
  void Dump(gOutput &) const;
};

#endif   // VERTENUM_H






