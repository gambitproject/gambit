//
// FILE: dualtope.h 
//
// $Id$
//

#include "glist.h"
#include "double.h"
#include "rectangl.h"
#include "glpsolv1.h"
#include "glpsolv2.h"
#include "gpoly.h"

#ifndef DUALTOPE_H
#define DUALTOPE_H

/*

   We begin by developing a class that represents halfspaces.

*/

template <class T> class gHalfSpc {
private:

  const gSpace*    Space;
  const T          Constant;
  const gVector<T> NormalVector;

// Overwriting is illegal, due to const members
  operator = (const gHalfSpc& rhs);

public:

  gHalfSpc(const gSpace*, const T&, const gVector<T>&);
  gHalfSpc(const gHalfSpc&);
  ~gHalfSpc();

// Operators

  bool operator == (const gHalfSpc<T>&) const;
  bool operator != (const gHalfSpc<T>&) const;

// Information

  const gSpace*    TheSpace()              const;
  const int        Dmnsn()                 const;
  const gVector<T> NormalVect()            const;
  const T          Coefficient(const int&) const;
  const T          BoundaryValue()         const;

// Printing

  friend gOutput &operator<< (gOutput &f, const gHalfSpc<T> &y);
};


template <class T> class gDualTope {
private:

  const gSpace*             Space;
  const gList<gHalfSpc<T> > Inequalities;
  const bool                Nonempty;

// Overwriting is illegal, due to const members
  operator = (const gDualTope& rhs);
  
// Initial Computations
  const bool                isnonempty()                            const;
  const gList<gHalfSpc<T> > minimallist(const gList<gHalfSpc<T> >&) const;
  const gList<gHalfSpc<T> > listfromrectangle(const gRectangle<T>&) const;

public:

  gDualTope(const gSpace*, const gList<gHalfSpc<T> >&);
  gDualTope(const gSpace*, const gRectangle<T>&);
  gDualTope(const gDualTope&);
  ~gDualTope();

// Operators

  bool operator == (const gDualTope<T>&) const;
  bool operator != (const gDualTope<T>&) const;

// Information

  const int           Dmnsn()                              const;
  const int           NumberOfFacets()                     const;
  const bool          IsNonempty()                         const;
  const gDualTope<T>  IntersectedWith(const gDualTope<T>&) const;
  const gRectangle<T> BoundingRectangle()                  const;

// Printing

  friend gOutput &operator<< (gOutput &f, const gDualTope<T> &y);
};

#endif //# DUALTOPE_H
