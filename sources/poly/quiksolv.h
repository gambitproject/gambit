//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to quick-solver classes
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

#ifndef QUIKSOLV_H
#define QUIKSOLV_H

#include "base/base.h"
#include "base/gstatus.h"
#include "base/odometer.h"
#include "gsolver.h"
#include "rectangl.h"
#include "gpoly.h"
#include "gpolylst.h"
#include "gpartltr.h"
#include "pelican/pelqhull.h"
#include "poly/pelclass.h"

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
   The main constructor for this takes a gbtPolyMultiList<T>.  The list must
be at least as long as the dimension Dmnsn() of the space of the
system.  The first Dmnsn() polynomials are interpreted as equations,
while remaining polynomials are interpreted as inequalities in the
sense that the gbtPolyUni is required to be nonnegative.
*/


// ***********************
//      class gbtPolyQuickSolve
// ***********************

template <class T> class gbtPolyQuickSolve {
 private:
  const gbtPolyMultiList<T>                 System;
  const gbtPolyMultiList<gbtDouble>           gDoubleSystem;
  const int                          NoEquations;
  const int                          NoInequalities;
  const gbtPolyPartialTreeList<gbtDouble>  TreesOfPartials;
        bool                         HasBeenSolved;
        gbtList<gbtVector<gbtDouble> >     Roots;
  const bool                         isMultiaffine;
  const gbtRectArray<bool>             Equation_i_uses_var_j;
  gbtStatus &m_status;

  // Supporting routines for the constructors

  gbtRectArray<bool> Eq_i_Uses_j()                                         const;

  // Get Roots Using Pelican

  bool AllRealRootsFromPelican(const gbtPolyMultiList<gbtDouble> &, 
			             gbtList<gbtVector<gbtDouble> > &)         const;
  bool PelicanRoots(const gRectangle<T> &, 
		          gbtList<gbtVector<gbtDouble> > &)                    const;

  // Check whether roots are impossible

   bool SystemHasNoRootsIn(const gRectangle<gbtDouble>& r, gbtArray<int>&)   const;


  // Ask whether Newton's method leads to a root 

   bool NewtonRootInRectangle(  const gRectangle<gbtDouble>&, 
			              gbtVector<gbtDouble>&) const;
   bool NewtonRootNearRectangle(const gRectangle<gbtDouble>&, 
			              gbtVector<gbtDouble>&) const;


  // Ask whether we can prove that there is no root other than 
  // the one produced by the last step

   gbtDouble MaxDistanceFromPointToVertexAfterTransformation(
				      const gRectangle<gbtDouble>&,
				      const gbtVector<gbtDouble>&,
				      const gbtSquareMatrix<gbtDouble>&)     const;

   bool HasNoOtherRootsIn(const gRectangle<gbtDouble>&,
			  const gbtVector<gbtDouble>&,
			  const gbtSquareMatrix<gbtDouble>&)                 const;


  // Combine the last two steps into a single query

   bool NewtonRootIsOnlyInRct(const gRectangle<gbtDouble>&, 
			            gbtVector<gbtDouble>&) const;


  // Recursive parts of recursive methods

  void               FindRootsRecursion(      gbtList<gbtVector<gbtDouble> >*,
					const gRectangle<gbtDouble>&, 
					const int&,
					      gbtArray<int>&,
					      int&,
					const int&,
					      int*)                  const;

  const bool         ARootExistsRecursion(const gRectangle<gbtDouble>&, 
					        gbtVector<gbtDouble>&,
					  const gRectangle<gbtDouble>&, 
					        gbtArray<int>&)        const;

 public:
  class NewtonError : public gbtException  {
  public:
    virtual ~NewtonError();
    gbtText Description(void) const;   
  };
   gbtPolyQuickSolve(const gbtPolyMultiList<T> &, gbtStatus &);  
   gbtPolyQuickSolve(const gbtPolyMultiList<T> &, const int &, gbtStatus &);  
   gbtPolyQuickSolve(const gbtPolyQuickSolve<T> &);
   ~gbtPolyQuickSolve();

   // Operators
   gbtPolyQuickSolve<T>& operator= (const gbtPolyQuickSolve<T> &);
   bool         operator==(const gbtPolyQuickSolve<T> &) const;
   bool         operator!=(const gbtPolyQuickSolve<T> &) const;

   // Information
   inline const gbtPolySpace*                  AmbientSpace()              const 
     { return System.AmbientSpace(); }
   inline const gbtPolyTermOrder*              TermOrder()                 const 
     { return System.TermOrder(); }
   inline const int                      Dmnsn()                     const 
     { return System.Dmnsn(); }
   inline const gbtPolyMultiList<T>             UnderlyingEquations()       const 
     { return System; }
   inline const bool                     WasSolved()                 const
     { return HasBeenSolved; }
   inline const gbtList<gbtVector<gbtDouble> > RootList()                  const
     { return Roots; }
   inline const bool                     IsMultiaffine()             const
     { return isMultiaffine; }

  // Refines the accuracy of roots obtained from other algorithms
  gbtVector<gbtDouble> NewtonPolishOnce(const gbtVector<gbtDouble> &)        const;
  gbtVector<gbtDouble> SlowNewtonPolishOnce(const gbtVector<gbtDouble> &)    const;
  gbtVector<gbtDouble> NewtonPolishedRoot(const gbtVector<gbtDouble> &)      const;

  // Checks for complex singular roots
   bool     MightHaveSingularRoots()                                 const;

  // The grand calculation - returns true if successful
   bool     FindCertainNumberOfRoots  (const gRectangle<T>&, 
				       const int&,
				       const int&);
   bool     FindRoots  (const gRectangle<T>&, const int&);
   bool     ARootExists (const gRectangle<T>&, gbtVector<gbtDouble>&)    const;
  
   void Output(gbtOutput &) const;
};  

template <class T> gbtOutput &operator<<(gbtOutput &output, const gbtPolyQuickSolve<T> &);

#endif // QUIKSOLV_H
