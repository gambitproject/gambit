//
// File: quiksolv.h  --  Declaration of gpolylst data type
// 
// @(#)quiksolv.h	1.2 01/11/98
//

#ifndef QUIKSOLV_H
#define QUIKSOLV_H

#include "gsolver.h"
#include "odometer.h"
#include "rectangl.h"
#include "gtree.h"

/*
    The (optimistically named) class described in this file is a method
of finding the roots of a system of polynomials, with equal numbers of
equations and unknowns, that lie inside a given rectangle.  The
general idea is to first ask whether the Taylor's series information
at the center of the rectangle precludes the existence of roots, and
if it does not, whether Newton's method leads to a root, and if it does,
whether the Taylor's series information at the root precludes the existence
of another root.  If the roots in the rectangle are not resolved by
these queries, the rectangle is subdivided into 2^d subrectangles, and
the process is repeated on each.  This continues until it has been shown
that all roots have been found, or a predetermined searach depth is reached.
The bound on depth is necessary because the procedure will not terminate
if there are singular roots.
 */

// ***********************
//      class QuikSolv
// ***********************

template <class T> class QuikSolv {
 private:
   const gPolyList<T>             System;
         bool                     HasBeenSolved;
         gList<gVector<gDouble> > Roots;

  // SUPPORTING CALCULATIONS - conceptually, some of these belong elsewhere

  // Generate all partial derivatives for the equations in the given system
   void                     TreeOfPartialsRECURSIVE(gTree<gPoly<T> >&,
				            gTreeNode<gPoly<T> >*) const;
   gTree<gPoly<T> >         TreeOfPartials(const gPoly<T>&)        const;
   gList<gTree<gPoly<T> > > ListOfPartialTrees()                   const;

  // Check whether roots are impossible
   T  MaximalNonconstantContributionRECURSIVE(const gTree<gPoly<T> >&,
					      const gTreeNode<gPoly<T> >*,
					      const gVector<T>&,
					      const gVector<T>&,
					      gVector<int>&)       const;
   bool PolyHasNoRootsIn(const gRectangle<T>&, const gTree<gPoly<T> >&) const;
   bool SystemHasNoRootsIn(const gRectangle<T>& r, 
			   const gList<gTree<gPoly<T> > >&,
			         gArray<int>&)       const;

  // Ask whether Newton's method leads to a root without leaving the rectangle
   gVector<gDouble>      VectorTogDouble(gVector<T>&)              const;
   bool NewtonRootInRectangle(const gRectangle<T>&, gVector<T>&)   const;

  // Ask whether we can prove that there is no root other than 
  // the one produced by the last step
   gPoly<T> TranslateOfMono(const gMono<T>&, const gVector<T>&)    const;
   gPoly<T> TranslateOfPoly(const gPoly<T>&, const gVector<T>&)    const;
   gPolyList<T> TranslateOfSystem(const gPolyList<T>&,
				  const gVector<T>&)               const;
   gPoly<T> MonoInNewCoordinates(const gMono<T>&, 
				 const gSquareMatrix<T>&)          const;
   gPoly<T> PolyInNewCoordinates(const gPoly<T>&, 
				 const gSquareMatrix<T>&)          const;
   gPolyList<T> SystemInNewCoordinates(const gPolyList<T>&,
				       const gSquareMatrix<T>&)    const;
   gDouble MaxDistanceFromPointToVertexAfterTransformation(
				      const gRectangle<T>&,
				      const gVector<T>&,
				      const gSquareMatrix<T>&)     const;
   gDouble MaximalContributionOfHigherOrderTerms(
                                      const gPoly<gDouble>&,
				      const gDouble&)              const;
   bool HasNoOtherRootsIn(const gRectangle<T>&,
			  const gVector<T>&,
			  const gSquareMatrix<T>&)                 const;

  // Combine the last two steps into a single query
   bool NewtonRootIsOnlyInRct(const gRectangle<T>&, gVector<T>&)   const;

  // Subdivide the rectangle in preparation for iteration
   gList<gRectangle<T> > BinarySubdivision(const gRectangle<T>&)   const;

 public:
   QuikSolv(const gPolyList<T> &);  
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

  // The grand calculation - returns true if successful
   bool     FindRoots     (const gRectangle<T>&, const int&, const int&);

friend gOutput& operator << (gOutput& output, const QuikSolv<T>& x);
};  

#endif QUIKSOLV_H
