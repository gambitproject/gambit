//
// File: equisolv.h  --  
// 
// @(#)equisolv.h	1.5 01/24/98
//

#ifndef EQUISOLV_H
#define EQUISOLV_H

#include "quiksolv.h"
#include "gsmpltpe.h"


// ***********************
//      class EquiSolv
// ***********************

template <class T> class EquiSolv {
 private:
// New Data Members
  const gList<gPolyList<T> >                 UtilityLists;
  const gList<gPolyList<gDouble> >           gDoubleULists;
  const gList<ListOfPartialTrees<gDouble> >  ListsOfTreesOfPartials;

// Old Data Members
  const gPolyList<T>                 System;
  const gPolyList<gDouble>           gDoubleSystem;
  const ListOfPartialTrees<gDouble> TreesOfPartials;

// Retained Data Members
        bool                         HasBeenSolved;
        gList<gVector<gDouble> >     Roots;

  // FUNCTIONS USED IN CONSTRUCTOR INITITALIZATIONS

  const gList<gPolyList<T> > CopyGivenLists(const gList<gPolyList<T> >&)
                                                                      const;
  const gPolyList<T>                 DerivedEquationSystem()          const;
  const gList<gPolyList<gDouble> > ListsTogDouble(const gList<gPolyList<T> >&)
                                                                      const;
  const gList<ListOfPartialTrees<gDouble> > GenerateTreesOfPartials() const;

  // SUPPORTING CALCULATIONS - conceptually, some of these belong elsewhere

  // Check whether roots are impossible

   bool PolyHasNoRootsIn(const gRectangle<gDouble>&, const int&)         const;
   bool PolyEverywhereNegativeIn(const gRectangle<gDouble>&, 
				 const int&)                             const;
   bool SystemHasNoRootsIn(const gRectangle<gDouble>& r, gArray<int>&)   const;
   bool SystemHasNoEquilibriaIn(const gRectangle<gDouble>& r)            const;

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
