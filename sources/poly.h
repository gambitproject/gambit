//@(#)poly.h	1.11 11/13/95

#include <assert.h>
#include "rational.h"
#include "gambitio.h"
#include "interval.h"
#include "gvector.h"
#include "glist.h"
#include "double.h"
#include "complex.h"

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

    // output
  friend gOutput& operator << (gOutput& output, const polynomial<T>& x);
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






