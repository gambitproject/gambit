//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to gbtPolyPartialTree
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

#ifndef GPARTLTR_H
#define GPARTLTR_H

#include "base/base.h"
#include "rectangl.h"
#include "gpoly.h"
#include "gpolylst.h"

// ****************************
//      class gbtPolyPartialTree
// ****************************

template <class T> class gbtPolyPartialTree {
private:
  gbtTree<gbtPolyMulti<T> > PartialTree;

  /// Recursive Constructions and Computations ///

   void TreeOfPartialsRECURSIVE(gbtTree<gbtPolyMulti<T> >&,
				gbtTreeNode<gbtPolyMulti<T> >*)         const;

   T MaximalNonconstantContributionRECURSIVE(
				const gbtTreeNode<gbtPolyMulti<T> >*,
		      	        const gbtVector<T>&,
			        const gbtVector<T>&,
		       	              gbtVector<int>&)           const;

   T MaximalNonconstantDifferenceRECURSIVE(
				const gbtTreeNode<gbtPolyMulti<T> >*,
				const gbtTreeNode<gbtPolyMulti<T> >*,
		      	        const gbtVector<T>&,
			        const gbtVector<T>&,
		       	              gbtVector<int>&)           const;

public:
   gbtPolyPartialTree(const gbtPolyMulti<T> &);  
   gbtPolyPartialTree(const gbtPolyPartialTree<T> &);
   ~gbtPolyPartialTree();

   inline bool operator ==(const gbtPolyPartialTree<T>& rhs) const 
     { return (PartialTree == rhs.PartialTree); }
   inline bool operator !=(const gbtPolyPartialTree<T>& rhs) const 
     { return !(*this == rhs); }
   inline int Dmnsn()                                    const 
     { return RootNode()->GetData().Dmnsn(); }
   T EvaluateRootPoly(const gbtVector<T>& point)    const; 


   T MaximalNonconstantContribution(const gbtVector<T>&, 
				    const gbtVector<T>&)         const;
   T MaximalNonconstantDifference(const gbtPolyPartialTree<T>&,
				  const gbtVector<T>&, 
				  const gbtVector<T>&)           const;

   inline gbtTreeNode<gbtPolyMulti<T> >* RootNode()                     const 
     { return PartialTree.RootNode(); }
   inline gbtPolyMulti<T> RootPoly()                                  const 
     { return RootNode()->GetData(); }
   T ValueOfRootPoly(const gbtVector<T>& point)           const;
   T ValueOfPartialOfRootPoly(const int&, const gbtVector<T>&)   const;
   gbtVector<T> VectorOfPartials(const gbtVector<T>&)              const;
   bool      PolyHasNoRootsIn(const gRectangle<T>&)            const;
   bool MultiaffinePolyHasNoRootsIn(const gRectangle<T>&)      const;
   bool PolyEverywhereNegativeIn(const gRectangle<T>&)         const;
   bool MultiaffinePolyEverywhereNegativeIn(const gRectangle<T>&) const;

   //friend gbtOutput& operator << (gbtOutput& output, const gbtPolyPartialTree<T>& x);
};


// *********************************
//      class gbtPolyPartialTreeList
// *********************************

template <class T> class gbtPolyPartialTreeList {
private:
  gbtList<gbtPolyPartialTree<T> >     PartialTreeList;

  // Disabling this operator -- we don't want it called
  gbtPolyPartialTreeList<T> &operator=(const gbtPolyPartialTreeList<T> &);

public:
   gbtPolyPartialTreeList(const gbtList<gbtPolyMulti<T> >&);  
   gbtPolyPartialTreeList(const gbtPolyMultiList<T>&);  
   gbtPolyPartialTreeList(const gbtPolyPartialTreeList<T> &);
   ~gbtPolyPartialTreeList();

  // operators
  bool operator == (const gbtPolyPartialTreeList&) const;
  bool operator != (const gbtPolyPartialTreeList&) const;

  inline const gbtPolyPartialTree<T>& operator[](const int& i)        const 
    { return PartialTreeList[i]; }

  // Information
  inline int Length()                                             const
    { return PartialTreeList.Length(); }
  inline int Dmnsn()                                              const
    { assert (Length() > 0); return PartialTreeList[1].Dmnsn(); }
  gbtMatrix<T> DerivativeMatrix(const gbtVector<T>&)                  const; 
  gbtMatrix<T> DerivativeMatrix(const gbtVector<T>&, const int&)      const; 
  gbtSquareMatrix<T> SquareDerivativeMatrix(const gbtVector<T>&)      const; 
  gbtVector<T> ValuesOfRootPolys(const gbtVector<T>&, const int&)     const;
   T         MaximalNonconstantDifference(const int&,
					  const int&,
					  const gbtVector<T>&, 
					  const gbtVector<T>&)      const;

   //friend gbtOutput& operator << (gbtOutput& output, const gbtPolyPartialTreeList<T>& x);
};

#endif // GPARTLTR_H
