//
// File: gpartltr.h  --  Declaration of TreeOfPartials and ListOfPartialTrees
// 
// $Id$
//

#ifndef GPARTLTR_H
#define GPARTLTR_H

//#include "gsolver.h"
//#include "odometer.h"
#include "rectangl.h"
#include "gtree.h"
#include "gpoly.h"
#include "gpolylst.h"

// ****************************
//      class TreeOfPartials
// ****************************

template <class T> class TreeOfPartials {
private:
  gTree<gPoly<T> > PartialTree;

  /// Recursive Constructions and Computations ///

   void TreeOfPartialsRECURSIVE(gTree<gPoly<T> >&,
				gTreeNode<gPoly<T> >*)         const;

   T MaximalNonconstantContributionRECURSIVE(
				const gTreeNode<gPoly<T> >*,
		      	        const gVector<T>&,
			        const gVector<T>&,
		       	              gVector<int>&)           const;

   T MaximalNonconstantDifferenceRECURSIVE(
				const gTreeNode<gPoly<T> >*,
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
   T MaximalNonconstantDifference(const TreeOfPartials<T>&,
				  const gVector<T>&, 
				  const gVector<T>&)           const;

   inline gTreeNode<gPoly<T> >* RootNode()                     const 
     { return PartialTree.RootNode(); }
   inline gPoly<T> RootPoly()                                  const 
     { return RootNode()->GetData(); }
   inline T ValueOfRootPoly(const gVector<T>& point)           const 
     { return RootPoly().Evaluate(point); }
   T ValueOfPartialOfRootPoly(const int&, const gVector<T>&)   const;
   gVector<T> VectorOfPartials(const gVector<T>&)              const;
   bool      PolyHasNoRootsIn(const gRectangle<T>&)            const;
   bool MultiaffinePolyHasNoRootsIn(const gRectangle<T>&)      const;
   bool PolyEverywhereNegativeIn(const gRectangle<T>&)         const;
   bool MultiaffinePolyEverywhereNegativeIn(const gRectangle<T>&) const;

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

  inline operator =  (const ListOfPartialTrees&) const { 
    gout << "We have not yet defined operator = in ListOfPartialTrees\n";
    exit(0);
  }

  // operators
  bool operator == (const ListOfPartialTrees&) const;
  bool operator != (const ListOfPartialTrees&) const;

  inline const TreeOfPartials<T>& operator[](const int& i)        const 
    { return PartialTreeList[i]; }

  // Information
  inline int Length()                                             const
    { return PartialTreeList.Length(); }
  inline int Dmnsn()                                              const
    { assert (Length() > 0); return PartialTreeList[1].Dmnsn(); }
  gMatrix<T> DerivativeMatrix(const gVector<T>&)                  const; 
  gMatrix<T> DerivativeMatrix(const gVector<T>&, const int&)      const; 
  gSquareMatrix<T> SquareDerivativeMatrix(const gVector<T>&)      const; 
  gVector<T> ValuesOfRootPolys(const gVector<T>&, const int&)     const;
   T         MaximalNonconstantDifference(const int&,
					  const int&,
					  const gVector<T>&, 
					  const gVector<T>&)      const;

friend gOutput& operator << (gOutput& output, const ListOfPartialTrees<T>& x);
};

#endif // GPARTLTR_H
