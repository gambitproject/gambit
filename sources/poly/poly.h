//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of polynomial classes
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
#include "math/double.h"
#include "math/complex.h"

/*  This file supplies the template class

    polynomial

These are univariate polynomials with coefficients of class T.
Polynomials are implemented as gList's of coefficients.  There is no 
attempt to maintain sparseness.

*/

template <class T> class polynomial {

private:
  gList<T> coeflist; 

public: 
    // constructors and destructor
  polynomial(const int=-1);
  polynomial(const polynomial<T> &);
  polynomial(const gList<T> &);
  polynomial(const gVector<T> &);
  polynomial(const T&, const int&);
  ~polynomial();

    // unary operators
         polynomial<T>    operator -  () const;
         polynomial<T>    Derivative  () const;

    // binary operators
         polynomial<T>&     operator =  (const polynomial<T>& y);
         bool               operator == (const polynomial<T>& y) const;
         bool               operator != (const polynomial<T>& y) const;
         const T&           operator [] (const int index)        const;
         polynomial<T>      operator +  (const polynomial<T>& y) const;
         polynomial<T>      operator -  (const polynomial<T>& y) const;
         polynomial<T>      operator *  (const polynomial<T>& y) const;
         polynomial<T>      operator /  (const polynomial<T>& y) const;
         polynomial<T>&     operator += (const polynomial<T>& y);
         polynomial<T>&     operator -= (const polynomial<T>& y);
         polynomial<T>&     operator *= (const polynomial<T>& y);
         polynomial<T>&     operator /= (const polynomial<T>& y);
         polynomial<T>      operator %  (const polynomial<T>& y) const;

  // manipulation
  void                   ToMonic()                                         ;
//  polynomial<gDouble>    Togdouble()                                  const;

  polynomial<gDouble>         TogDouble()                       const;

  // information
  bool                   IsZero()                                     const;
  T                      EvaluationAt(const T& arg)                   const;  
  int                    Degree()                                     const;
  T                      LeadingCoefficient()                         const;
  gList<T>               CoefficientList()                            const;
  polynomial<T>          GcdWith(const polynomial<T>&)                const;
  bool                   IsQuadratfrei()                              const;
  bool                   CannotHaveRootsIn(const gInterval<T>&)       const;
  gList< gInterval<T> >  RootSubintervals(const gInterval<T>&)        const;
  gInterval<T>           NeighborhoodOfRoot(const gInterval<T>&, T&)  const;
  gList< gInterval<T> >  PreciseRootIntervals(const gInterval<T>&,T&) const;
  gList<T>               PreciseRoots(const gInterval<T>&, T&)        const;

  void Output(gOutput &) const;
};

template <class T> gOutput& operator <<(gOutput &, const polynomial<T> &);


/*                       REMARKS

   The function cannot_have_roots_in is based on the principle that if

f = a_0 + a_1x + ... + a_dx^d

with a_0 > 0, then

abs(f(t)) >= a_0 - max{abs(a_1),...,abs(a_d)}*(abs(t) + ... + abs(t)^d)

and the RHS will be positive whenever

//WRONG! abs(t) < a_0/(a_0 + max{abs(a_1),...,abs(a_d)}).

*/

class complexpoly {

private:
  gList<gComplex> coeflist; 

public: 
    // constructors and destructor
  complexpoly(const int=-1);
  complexpoly(const complexpoly &);
  complexpoly(const gList<gComplex> &);
  complexpoly(const gComplex&, const int&);
  ~complexpoly();

    // unary operators
  complexpoly   operator -  ()                               const;
  complexpoly   Derivative  ()                               const;

    // binary operators
  complexpoly&  operator =  (const complexpoly& y);
  bool                    operator == (const complexpoly& y) const;
  bool                    operator != (const complexpoly& y) const;
  const gComplex&         operator [] (const int index)      const;
  complexpoly             operator +  (const complexpoly& y) const;
  complexpoly             operator -  (const complexpoly& y) const;
  complexpoly             operator *  (const complexpoly& y) const;
  complexpoly             operator /  (const complexpoly& y) const;
  complexpoly&            operator += (const complexpoly& y);
  complexpoly&            operator -= (const complexpoly& y);
  complexpoly&            operator *= (const complexpoly& y);
  complexpoly&            operator /= (const complexpoly& y);
  complexpoly             operator %  (const complexpoly& y) const;
  
  // manipulation
  void                   ToMonic()                                         ;

  // information
  bool                   IsZero()                                     const;
  gComplex               EvaluationAt(const gComplex& arg)            const;  
  int                    Degree()                                     const;
  gComplex               LeadingCoefficient()                         const;
  complexpoly            GcdWith(const complexpoly&)                  const;
  bool                   IsQuadratfrei()                              const;
  gList<gComplex>        Roots()                                      const;
  
  // output
friend gOutput&        operator << (gOutput& output, const complexpoly& x);
};






