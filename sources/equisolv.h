//
// File: equisolv.h  --  
// 
// @(#)equisolv.h	1.5 01/24/98
//

#ifndef EQUISOLV_H
#define EQUISOLV_H

#include "quiksolv.h"
#include "gsmpltpe.h"

/* 

   The solver in this class is intended to be a general stationary
point solver, meaning that there is assumed to be some sort of
complementarity relation between the facets of the product of polytopes
and the equality conditions that define equilibrium.  In particular,
it is intended that there be some form of descent to problems on the
boundary.
   At this point I see the following agenda:

1.  Figuring out how this actually works for the sequence form. UNRESOLVED

2.  Little fix to get compiling.  DONE

3.  Initialize new member as partial tree for diffs of Utilities.

4.  Generally, try to reform away ListOfPartialTrees.

4.  Make sure all "no more roots" routines are brought up to date.

5.  Generalize to simpletope.

6.  Add descent feature.

*/


// ***********************
//      class EquiSolv
// ***********************

template <class T> class EquiSolv {
 private:
// New Data Members
  const gList<gPolyList<T> >                            UtilityLists;
  const gList<gPolyList<gDouble> >                      gDoubleULists;
  const gList<gList<gList<TreeOfPartials<gDouble> > > > UDiffPartialTrees;
  const gList<ListOfPartialTrees<gDouble> >             ListsOfTreesOfPartials;

// Old Data Members
  const gPolyList<T>                 System;
  const gPolyList<gDouble>           gDoubleSystem;
  const ListOfPartialTrees<gDouble>  TreesOfPartials;

// Retained Data Members
        bool                         HasBeenSolved;
        gList<gVector<gDouble> >     Roots;

  // FUNCTIONS USED IN CONSTRUCTOR INITITALIZATIONS

  const gList<gPolyList<T> > 
        CopyGivenLists(const gList<gPolyList<T> >&)                   const;
  const gPolyList<T>                 
        DerivedEquationSystem()                                       const;
  const gPolyList<gDouble>        
        ListTogDouble(const gPolyList<T>&)                            const;
  const gList<gPolyList<gDouble> > 
        ListsTogDouble(const gList<gPolyList<T> >&)                   const;
  const gList<ListOfPartialTrees<gDouble> > 
        GenerateTreesOfPartials()                                     const;
  const gList<gList<gList<TreeOfPartials<gDouble> > > > 
        UtilityDiffPartialTrees()                                     const;

  // SUPPORTING CALCULATIONS - conceptually, some of these belong elsewhere

  // Check whether roots are impossible
   bool PolyHasNoRootsIn(const gRectangle<gDouble>&, const int&)         const;
   bool PolyEverywhereNegativeIn(const gRectangle<gDouble>&, const int&) const;
   bool SystemHasNoRootsIn(const gRectangle<gDouble>& r, gArray<int>&)   const;
// bool SystemHasNoEquilibriaIn(const gRectangle<gDouble>& r)            const;

  // Ask whether Newton's method leads to a root without leaving the rectangle
   gSquareMatrix<gDouble> DerivativeMatrix(const gVector<gDouble>&)      const;
   gVector<gDouble>       UtilityDiffs(const gVector<gDouble>&)          const;
   bool                   NewtonRootInRectangle(const gRectangle<gDouble>&, 
						gVector<gDouble>&)       const;
   bool                   NewtonEquiInRectangle(const gRectangle<gDouble>&, 
						gVector<gDouble>&)       const;

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
   bool NewtonEquiIsOnlyInRct(const gRectangle<gDouble>&, 
			            gVector<gDouble>&) const;

  // Recursive part of recursive method //ORIG
  void               FindRootsRecursion(      gList<gVector<gDouble> >*,
					const gRectangle<gDouble>&, 
					const int&,
					      gArray<int>&,
					      int&,
					const int&,
					      int*)                  const;

  // Recursive part of recursive method
  void               FindRootsRecursion(      gList<gVector<gDouble> >*,
					const gSimpletope<gDouble>&, 
					const int&,
					      int&,
					const int&,
					      int*)                  const;

 public:

// Should be returned to privacy after testing
   bool SystemHasNoEquilibriaIn(const gRectangle<gDouble>& r)            const;



   EquiSolv(const gList<gPolyList<T> > &);  
   EquiSolv(const EquiSolv<T> &);
   ~EquiSolv();

   // Operators
   EquiSolv<T>& operator= (const EquiSolv<T> &);
   bool         operator==(const EquiSolv<T> &) const;
   bool         operator!=(const EquiSolv<T> &) const;

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
   bool     FindCertainNumberOfRoots  (const gRectangle<gDouble>&, 
				       const int&,
				       const int&); //ORIG
   bool     FindCertainNumberOfRoots  (const int&, const int&);
   bool     FindRoots  (const gRectangle<gDouble>&, const int&); //ORIG
   bool     FindRoots  (const int&);

friend gOutput& operator << (gOutput& output, const EquiSolv<T>& x);
};  

#endif EQUISOLV_H
