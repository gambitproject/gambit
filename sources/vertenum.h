//
// FILE: vertenum.h -- Vertex Enumeration module
//
// $Id$
//

#ifndef VERTENUM_H
#define VERTENUM_H

#include "lptab.h"
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
  gList<BFS<T> > DualList;
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
  class BadDim : public gException  {
  public:
    virtual ~BadDim();
    gText Description(void) const;
  };

  VertEnum(const gMatrix<T> &, const gVector<T> &, gStatus &status_ = gstatus);
  VertEnum(LPTableau<T> &, gStatus &status_ = gstatus);
  virtual ~VertEnum();

  const gList<BFS<T> > &VertexList() const;
  const gList<BFS<T> > &DualVertexList() const;
  void Vertices(gList<gVector<T> > &verts) const;
  long NumPivots() const;
  void Dump(gOutput &) const;
};


template <class T> class DoubleVertEnum {
private:
  int mult_opt,depth;
  int n;  // N is the number of columns, which is the # of dimensions.
  int k;  // K is the number of inequalities given.
    // Removed const on A and b (Geoff)
  const gMatrix<T> &A, &A2;   
  const gVector<T> &b, &b2;
  gVector<T> btemp,c;
  gList<BFS<T> > List, List2;
  gList<gVector<T> > Verts;
  long npivots,nodes;
  gStatus &status;
  gList<long> visits,branches;

  void Enum();
  void Deeper();
  void Report();
  void Search(LPTableau<T> &, Tableau<T> &);
  void DualSearch(LPTableau<T> &, Tableau<T> &);
  void EnumerateComplementaryFace(LPTableau<T> &, Tableau<T> &);
public:
  class BadDim : public gException  {
  public:
    virtual ~BadDim();
    gText Description(void) const;
  };
  DoubleVertEnum(const gMatrix<T> &_A, const gVector<T> &_b,
		 const gMatrix<T> &_A2, const gVector<T> &_b2, 
		 gStatus &status_ = gstatus);
  ~DoubleVertEnum();
  const gList<BFS<T> > &VertexList() const;
  const gList<BFS<T> > &VertexList2() const;
  void Vertices(gList<gVector<T> > &verts, gList<gVector<T> > &verts2) const;
  long NumPivots() const;
  void Dump(gOutput &) const;
};

#endif // VERTENUM_H
