//
// File: quiksolv.h  --  Declaration of gpolylst data type
// 
// $Id$
//

#ifndef QUIKSOLV_H
#define QUIKSOLV_H

#include "gstatus.h"

#include "gsolver.h"
#include "odometer.h"
#include "rectangl.h"
#include "gtree.h"
#include "gpoly.h"
#include "gpolylst.h"
#include "gpartltr.h"

/*
    The (optimistically named) class described in this file is a method
of finding the roots of a system of polynomials and inequalities, with
equal numbers of equations and unknowns, that lie inside a given
rectangle.  The general idea is to first ask whether the Taylor's
series information at the center of the rectangle precludes the
existence of roots, and if it does not, whether Newton's method leads
to a root, and if it does, whether the Taylor's series information at
the root precludes the existence of another root.  If the roots in the
rectangle are not resolved by these queries, the rectangle is
subdivided into 2^d subrectangles, and the process is repeated on
each.  This continues until it has been shown that all roots have been
found, or a predetermined search depth is reached.  The bound on depth
is necessary because the procedure will not terminate if there are
singular roots.
*/

/*
   The main constructor for this takes a gPolyList<T>.  The list must
be at least as long as the dimension Dmnsn() of the space of the
system.  The first Dmnsn() polynomials are interpreted as equations,
while remaining polynomials are interpreted as inequalities in the
sense that the polynomial is required to be nonnegative.
*/


// ***********************
//      class QuikSolv
// ***********************

template <class T> class QuikSolv {
 private:
  const gPolyList<T>                 System;
  const gPolyList<gDouble>           gDoubleSystem;
  const int                          NoEquations;
  const int                          NoInequalities;
  const ListOfPartialTrees<gDouble>  TreesOfPartials;
        bool                         HasBeenSolved;
        gList<gVector<gDouble> >     Roots;
  const bool                         isMultiaffine;
  const gRectArray<bool>             Equation_i_uses_var_j;
  gStatus &m_status;

  // Supporting routines for the constructors

  gRectArray<bool> Eq_i_Uses_j()                                         const;

  // Check whether roots are impossible

   bool SystemHasNoRootsIn(const gRectangle<gDouble>& r, gArray<int>&)   const;


  // Ask whether Newton's method leads to a root 

   bool NewtonRootInRectangle(  const gRectangle<gDouble>&, 
			              gVector<gDouble>&) const;
   bool NewtonRootNearRectangle(const gRectangle<gDouble>&, 
			              gVector<gDouble>&) const;


  // Ask whether we can prove that there is no root other than 
  // the one produced by the last step

   gDouble MaxDistanceFromPointToVertexAfterTransformation(
				      const gRectangle<gDouble>&,
				      const gVector<gDouble>&,
				      const gSquareMatrix<gDouble>&)     const;

   bool HasNoOtherRootsIn(const gRectangle<gDouble>&,
			  const gVector<gDouble>&,
			  const gSquareMatrix<gDouble>&)                 const;


  // Combine the last two steps into a single query

   bool NewtonRootIsOnlyInRct(const gRectangle<gDouble>&, 
			            gVector<gDouble>&) const;


  // Recursive parts of recursive methods

  void               FindRootsRecursion(      gList<gVector<gDouble> >*,
					const gRectangle<gDouble>&, 
					const int&,
					      gArray<int>&,
					      int&,
					const int&,
					      int*)                  const;

  const bool         ARootExistsRecursion(const gRectangle<gDouble>&, 
					        gVector<gDouble>&,
					  const gRectangle<gDouble>&, 
					        gArray<int>&)        const;

 public:
   QuikSolv(const gPolyList<T> &, gStatus &);  
   QuikSolv(const gPolyList<T> &, const int &, gStatus &);  
   QuikSolv(const QuikSolv<T> &);
   ~QuikSolv();

   // Operators
   QuikSolv<T>& operator= (const QuikSolv<T> &);
   bool         operator==(const QuikSolv<T> &) const;
   bool         operator!=(const QuikSolv<T> &) const;

   // Information
   inline const gSpace*                  AmbientSpace()              const 
     { return System.AmbientSpace(); }
   inline const term_order*              TermOrder()                 const 
     { return System.TermOrder(); }
   inline const int                      Dmnsn()                     const 
     { return System.Dmnsn(); }
   inline const gPolyList<T>             UnderlyingEquations()       const 
     { return System; }
   inline const bool                     WasSolved()                 const
     { return HasBeenSolved; }
   inline const gList<gVector<gDouble> > RootList()                  const
     { return Roots; }
   inline const bool                     IsMultiaffine()             const
     { return isMultiaffine; }

  // Refines the accuracy of roots obtained from other algorithms
  gVector<gDouble> NewtonPolishOnce(const gVector<gDouble> &)        const;
  gVector<gDouble> SlowNewtonPolishOnce(const gVector<gDouble> &)    const;
  gVector<gDouble> NewtonPolishedRoot(const gVector<gDouble> &)      const;

  // Checks for complex singular roots
   bool     MightHaveSingularRoots()                                 const;

  // The grand calculation - returns true if successful
   bool     FindCertainNumberOfRoots  (const gRectangle<T>&, 
				       const int&,
				       const int&);
   bool     FindRoots  (const gRectangle<T>&, const int&);
   bool     ARootExists (const gRectangle<T>&, gVector<gDouble>&)    const;

friend gOutput& operator << (gOutput& output, const QuikSolv<T>& x);
};  

#endif QUIKSOLV_H
