//
// File: quiksolv.h  --  Declaration of gpolylst data type
// 
// $Id$
//

#ifndef QUIKSOLV_H
#define QUIKSOLV_H

#include "gsolver.h"
#include "odometer.h"
#include "rectangl.h"
#include "gtree.h"
#include "gpoly.h"
#include "gpolylst.h"

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

// ****************************
//      class TreeOfPartials
// ****************************

template <class T> class TreeOfPartials {
private:
  gTree<gPoly<T> > PartialTree;

// Recursive Construction
   void TreeOfPartialsRECURSIVE(gTree<gPoly<T> >&,
				gTreeNode<gPoly<T> >*)         const;

   T MaximalNonconstantContributionRECURSIVE(
				const gTreeNode<gPoly<T> >*,
		      	        const gVector<T>&,
			        const gVector<T>&,
		       	              gVector<int>&)           const;

public:
   TreeOfPartials(const gPoly<T> &);  
   TreeOfPartials(const TreeOfPartials<T> &);
   ~TreeOfPartials();

   inline const bool operator ==(const TreeOfPartials<T>& rhs) const 
     { return (PartialTree == rhs.PartialTree); }
   inline const bool operator !=(const TreeOfPartials<T>& rhs) const 
     { return !(*this == rhs); }
   inline const int Dmnsn()                                    const 
     { return RootNode()->GetData().Dmnsn(); }
   inline const T EvaluateRootPoly(const gVector<T>& point)    const 
     { return RootNode()->GetData().Evaluate(point); }


   T MaximalNonconstantContribution(const gVector<T>&, 
				    const gVector<T>&)         const;

   inline gTreeNode<gPoly<T> >* RootNode()                     const 
     { return PartialTree.RootNode(); }
   inline gPoly<T> RootPoly()                                  const 
     { return RootNode()->GetData(); }
   inline T ValueOfRootPoly(const gVector<T>& point)           const 
     { return RootPoly().Evaluate(point); }
   T ValueOfPartialOfRootPoly(const int&, const gVector<T>&)   const;

friend gOutput& operator << (gOutput& output, const TreeOfPartials<T>& x);
};


// *********************************
//      class ListOfPartialTrees
// *********************************

template <class T> class ListOfPartialTrees {
private:
  gList<TreeOfPartials<T> >     PartialTreeList;

public:
   ListOfPartialTrees(const gList<gPoly<T> >&);  
   ListOfPartialTrees(const gPolyList<T>&);  
   ListOfPartialTrees(const ListOfPartialTrees<T> &);
   ~ListOfPartialTrees();

  inline const TreeOfPartials<T>& operator[](const int& i)        const 
    { return PartialTreeList[i]; }

  // Information
  inline int Length()                                             const
    { return PartialTreeList.Length(); }
  inline int Dmnsn()                                              const
    { assert (Length() > 0); return PartialTreeList[1].Dmnsn(); }
  gMatrix<T> DerivativeMatrix(const gVector<T>&)                  const; 
  gSquareMatrix<T> SquareDerivativeMatrix(const gVector<T>&)      const; 
  gVector<T> ValuesOfRootPolys(const gVector<T>&)                 const;
};

/*
   The main constructor for this takes a gPolyList<T>.  The list must
be at least as long as the dimension Dmnsn() of the space of the
system.  The first Dmnsn() polynomials are interpreted as equtions,
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
  const ListOfPartialTrees<gDouble>  TreesOfPartials;
        bool                         HasBeenSolved;
        gList<gVector<gDouble> >     Roots;

  // SUPPORTING CALCULATIONS - conceptually, some of these belong elsewhere

  // Check whether roots are impossible

   bool PolyHasNoRootsIn(const gRectangle<gDouble>&, const int&)         const;
   bool PolyEverywhereNegativeIn(const gRectangle<gDouble>&, 
				 const int&)                             const;
   bool SystemHasNoRootsIn(const gRectangle<gDouble>& r, gArray<int>&)   const;

  // Ask whether Newton's method leads to a root without leaving the rectangle
   bool NewtonRootInRectangle(const gRectangle<gDouble>&, 
			            gVector<gDouble>&) const;

  // Ask whether we can prove that there is no root other than 
  // the one produced by the last step

   gPoly<gDouble> TranslateOfMono(const gMono<gDouble>&, 
				  const gVector<gDouble>&)               const;
   gPoly<gDouble> TranslateOfPoly(const gPoly<gDouble>&, 
				  const gVector<gDouble>&)               const;
   gPolyList<gDouble> TranslateOfSystem(const gPolyList<gDouble>&,
				  const gVector<gDouble>&)               const;
   gPoly<gDouble> MonoInNewCoordinates(const gMono<gDouble>&, 
				 const gSquareMatrix<gDouble>&)          const;
   gPoly<gDouble> PolyInNewCoordinates(const gPoly<gDouble>&, 
				 const gSquareMatrix<gDouble>&)          const;
   gPolyList<gDouble> SystemInNewCoordinates(const gPolyList<gDouble>&,
				       const gSquareMatrix<gDouble>&)    const;
   gDouble MaxDistanceFromPointToVertexAfterTransformation(
				      const gRectangle<gDouble>&,
				      const gVector<gDouble>&,
				      const gSquareMatrix<gDouble>&)     const;

   gDouble MaximalContributionOfHigherOrderTerms(const gPoly<gDouble>&,
						 const gDouble&)         const;
   bool HasNoOtherRootsIn(const gRectangle<gDouble>&,
			  const gVector<gDouble>&,
			  const gSquareMatrix<gDouble>&)                 const;
  // Combine the last two steps into a single query
   bool NewtonRootIsOnlyInRct(const gRectangle<gDouble>&, 
			            gVector<gDouble>&) const;

  // Recursive part of recursive method
  void               FindRootsRecursion(      gList<gVector<gDouble> >*,
					const gRectangle<gDouble>&, 
					const int&,
					      gArray<int>&,
					      int&,
					const int&,
					      int*)                  const;

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

  // Checks for complex singular roots
   bool     MightHaveSingularRoots()                                 const;

  // The grand calculation - returns true if successful
   bool     FindCertainNumberOfRoots  (const gRectangle<T>&, 
				       const int&,
				       const int&);
   bool     FindRoots  (const gRectangle<T>&, const int&);

friend gOutput& operator << (gOutput& output, const QuikSolv<T>& x);
};  

#endif QUIKSOLV_H
