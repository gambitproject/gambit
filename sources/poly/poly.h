//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of gbtPolyUni classes
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

#include <assert.h>
#include "base/base.h"
#include "math/rational.h"
#include "interval.h"
#include "math/gvector.h"
#include "math/complex.h"

/*  This file supplies the template class

    gbtPolyUni

These are univariate polynomials with coefficients of class T.
Polynomials are implemented as gbtList's of coefficients.  There is no 
attempt to maintain sparseness.

*/

template <class T> class gbtPolyUni {

private:
  gbtList<T> coeflist; 

public: 
    // constructors and destructor
  gbtPolyUni(const int=-1);
  gbtPolyUni(const gbtPolyUni<T> &);
  gbtPolyUni(const gbtList<T> &);
  gbtPolyUni(const gbtVector<T> &);
  gbtPolyUni(const T&, const int&);
  ~gbtPolyUni();

    // unary operators
         gbtPolyUni<T>    operator -  () const;
         gbtPolyUni<T>    Derivative  () const;

    // binary operators
         gbtPolyUni<T>&     operator =  (const gbtPolyUni<T>& y);
         bool               operator == (const gbtPolyUni<T>& y) const;
         bool               operator != (const gbtPolyUni<T>& y) const;
         const T&           operator [] (const int index)        const;
         gbtPolyUni<T>      operator +  (const gbtPolyUni<T>& y) const;
         gbtPolyUni<T>      operator -  (const gbtPolyUni<T>& y) const;
         gbtPolyUni<T>      operator *  (const gbtPolyUni<T>& y) const;
         gbtPolyUni<T>      operator /  (const gbtPolyUni<T>& y) const;
         gbtPolyUni<T>&     operator += (const gbtPolyUni<T>& y);
         gbtPolyUni<T>&     operator -= (const gbtPolyUni<T>& y);
         gbtPolyUni<T>&     operator *= (const gbtPolyUni<T>& y);
         gbtPolyUni<T>&     operator /= (const gbtPolyUni<T>& y);
         gbtPolyUni<T>      operator %  (const gbtPolyUni<T>& y) const;

  // manipulation
  void                   ToMonic()                                         ;

  // information
  bool                   IsZero()                                     const;
  T                      EvaluationAt(const T& arg)                   const;  
  int                    Degree()                                     const;
  T                      LeadingCoefficient()                         const;
  gbtList<T>               CoefficientList()                            const;
  gbtPolyUni<T>          GcdWith(const gbtPolyUni<T>&)                const;
  bool                   IsQuadratfrei()                              const;
  bool                   CannotHaveRootsIn(const gInterval<T>&)       const;
  gbtList< gInterval<T> >  RootSubintervals(const gInterval<T>&)        const;
  gInterval<T>           NeighborhoodOfRoot(const gInterval<T>&, T&)  const;
  gbtList< gInterval<T> >  PreciseRootIntervals(const gInterval<T>&,T&) const;
  gbtList<T>               PreciseRoots(const gInterval<T>&, T&)        const;

  void Output(std::ostream &) const;
};

template <class T> std::ostream& operator <<(std::ostream &, const gbtPolyUni<T> &);


/*                       REMARKS

   The function cannot_have_roots_in is based on the principle that if

f = a_0 + a_1x + ... + a_dx^d

with a_0 > 0, then

abs(f(t)) >= a_0 - max{abs(a_1),...,abs(a_d)}*(abs(t) + ... + abs(t)^d)

and the RHS will be positive whenever

//WRONG! abs(t) < a_0/(a_0 + max{abs(a_1),...,abs(a_d)}).

*/

class gbtPolyComplex {

private:
  gbtList<gbtComplex> coeflist; 

public: 
    // constructors and destructor
  gbtPolyComplex(const int=-1);
  gbtPolyComplex(const gbtPolyComplex &);
  gbtPolyComplex(const gbtList<gbtComplex> &);
  gbtPolyComplex(const gbtComplex&, const int&);
  ~gbtPolyComplex();

    // unary operators
  gbtPolyComplex   operator -  ()                               const;
  gbtPolyComplex   Derivative  ()                               const;

    // binary operators
  gbtPolyComplex&  operator =  (const gbtPolyComplex& y);
  bool                    operator == (const gbtPolyComplex& y) const;
  bool                    operator != (const gbtPolyComplex& y) const;
  const gbtComplex&         operator [] (const int index)      const;
  gbtPolyComplex             operator +  (const gbtPolyComplex& y) const;
  gbtPolyComplex             operator -  (const gbtPolyComplex& y) const;
  gbtPolyComplex             operator *  (const gbtPolyComplex& y) const;
  gbtPolyComplex             operator /  (const gbtPolyComplex& y) const;
  gbtPolyComplex&            operator += (const gbtPolyComplex& y);
  gbtPolyComplex&            operator -= (const gbtPolyComplex& y);
  gbtPolyComplex&            operator *= (const gbtPolyComplex& y);
  gbtPolyComplex&            operator /= (const gbtPolyComplex& y);
  gbtPolyComplex             operator %  (const gbtPolyComplex& y) const;
  
  // manipulation
  void                   ToMonic()                                         ;

  // information
  bool                   IsZero()                                     const;
  gbtComplex               EvaluationAt(const gbtComplex& arg)            const;  
  int                    Degree()                                     const;
  gbtComplex               LeadingCoefficient()                         const;
  gbtPolyComplex            GcdWith(const gbtPolyComplex&)                  const;
  bool                   IsQuadratfrei()                              const;
  gbtList<gbtComplex>        Roots()                                      const;
  
  // output
friend std::ostream&        operator << (std::ostream& output, const gbtPolyComplex& x);
};
