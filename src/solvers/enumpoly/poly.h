//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/poly.h
// Declaration of polynomial classes
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

#include "gambit.h"
#include "interval.h"
#include "gcomplex.h"

/*  This file supplies the template class

    polynomial

These are univariate polynomials with coefficients of class T.
Polynomials are implemented as Gambit::List's of coefficients.  There is no 
attempt to maintain sparseness.

*/

template <class T> class polynomial {

private:
  Gambit::List<T> coeflist; 

public: 
    // constructors and destructor
  explicit polynomial(int=-1);
  polynomial(const polynomial<T> &);
  explicit polynomial(const Gambit::List<T> &);
  explicit polynomial(const Gambit::Vector<T> &);
  polynomial(const T&, const int&);
  ~polynomial() = default;

    // unary operators
         polynomial<T>    operator -  () const;
         polynomial<T>    Derivative  () const;

    // binary operators
         polynomial<T>&     operator =  (const polynomial<T>& y);
         bool               operator == (const polynomial<T>& y) const;
         bool               operator != (const polynomial<T>& y) const;
         const T&           operator [] (int index)        const;
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

  polynomial<double>         TogDouble()                       const;

  // information
  bool                   IsZero()                                     const;
  T                      EvaluationAt(const T& arg)                   const;  
  int                    Degree()                                     const;
  T                      LeadingCoefficient()                         const;
  Gambit::List<T> CoefficientList()                            const;
  polynomial<T>          GcdWith(const polynomial<T>&)                const;
  bool                   IsQuadratfrei()                              const;
  bool                   CannotHaveRootsIn(const gInterval<T>&)       const;
  Gambit::List< gInterval<T> >  RootSubintervals(const gInterval<T>&)        const;
  gInterval<T>           NeighborhoodOfRoot(const gInterval<T>&, T&)  const;
  Gambit::List< gInterval<T> >  PreciseRootIntervals(const gInterval<T>&,T&) const;
  Gambit::List<T>               PreciseRoots(const gInterval<T>&, T&)        const;
};


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
  Gambit::List<gComplex> coeflist; 

public: 
    // constructors and destructor
  explicit complexpoly(int=-1);
  complexpoly(const complexpoly &);
  explicit complexpoly(const Gambit::List<gComplex> &);
  complexpoly(const gComplex&, const int&);
  ~complexpoly();

    // unary operators
  complexpoly   operator -  ()                               const;
  complexpoly   Derivative  ()                               const;

    // binary operators
  complexpoly&  operator =  (const complexpoly& y);
  bool                    operator == (const complexpoly& y) const;
  bool                    operator != (const complexpoly& y) const;
  const gComplex&         operator [] (int index)      const;
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
  Gambit::List<gComplex>        Roots()                                      const;

};






