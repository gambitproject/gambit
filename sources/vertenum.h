//#
//# FILE: vertenum.h -- Vertex Enumeration module
//#
//# @(#)vertenum.h	2.5 5/25/97
//#

#ifndef VERTENUM_H
#define VERTENUM_H

#include "tableau.h"
#include "bfs.h"
#include "gstatus.h"
#include "garray.h"
#include "gmatrix.h"
#include "gsmatrix.h"

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
  VertEnum(const gMatrix<T> &_A, const gVector<T> &_b, gStatus &status_ = gstatus);
  ~VertEnum();

  const gList<BFS<T> > &VertexList() const;
  void Vertices(gList<gVector<T> > &verts) const;
  long NumPivots() const;
  void Dump(gOutput &) const;
};



// This class enumerates the vertices of the convex polyhedron 
//
//        P = {y:Ay + b <= 0}
// 
// where b <= 0.  Enumeration starts from the vertex y = 0.
// All computation is done in the class constructor. The 
// list of vertices can be accessed by VertexList()
//  
// The code is based on the reverse Pivoting algorithm of Avis 
// and Fukuda, Discrete Computational Geom (1992) 8:295-313.
//

template <class T> class NewVertEnum {
private:
  int mult_opt,depth;
  int n;  // N is the number of columns, which is the # of dimensions.
  int k;  // K is the number of inequalities given.
    // Removed const on A and b (Geoff)
  gMatrix<T> *A;   
  gVector<T> *b;
  const gMatrix<T> &C;   
  const gVector<T> &d;
  gVector<T> btemp,c;
  gList<BFS<T> > List;
  gList<gVector<T> > Verts;
  long npivots,nodes;
  gStatus &status;

public:
  NewVertEnum(const gMatrix<T> &_A, const gVector<T> &_b, 
	   const gVector<T> &start, gStatus &status_ = gstatus);
  ~NewVertEnum();

  const gList<BFS<T> > &VertexList() const;
  void Vertices(gList<gVector<T> > &verts) const;
  long NumPivots() const;
  void Dump(gOutput &) const;
};

#endif   // VERTENUM_H










