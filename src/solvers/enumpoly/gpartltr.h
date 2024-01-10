//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/gpartltr.h
// Interface to TreeOfPartials
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

#include "gambit.h"
#include "gtree.h"
#include "rectangl.h"
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
		      	        const Gambit::Vector<T>&,
			        const Gambit::Vector<T>&,
		       	              Gambit::Vector<int>&)           const;

   T MaximalNonconstantDifferenceRECURSIVE(
				const gTreeNode<gPoly<T> >*,
				const gTreeNode<gPoly<T> >*,
		      	        const Gambit::Vector<T>&,
			        const Gambit::Vector<T>&,
		       	              Gambit::Vector<int>&)           const;

public:
   explicit TreeOfPartials(const gPoly<T> &);
   TreeOfPartials(const TreeOfPartials<T> &);
   ~TreeOfPartials();

   inline bool operator ==(const TreeOfPartials<T>& rhs) const 
     { return (PartialTree == rhs.PartialTree); }
   inline bool operator !=(const TreeOfPartials<T>& rhs) const 
     { return !(*this == rhs); }
   inline int Dmnsn()                                    const 
     { return RootNode()->GetData().Dmnsn(); }
   T EvaluateRootPoly(const Gambit::Vector<T>& point)    const; 


   T MaximalNonconstantContribution(const Gambit::Vector<T>&, 
				    const Gambit::Vector<T>&)         const;
   T MaximalNonconstantDifference(const TreeOfPartials<T>&,
				  const Gambit::Vector<T>&, 
				  const Gambit::Vector<T>&)           const;

   inline gTreeNode<gPoly<T> >* RootNode()                     const 
     { return PartialTree.RootNode(); }
   inline gPoly<T> RootPoly()                                  const 
     { return RootNode()->GetData(); }
   T ValueOfRootPoly(const Gambit::Vector<T>& point)           const;
   T ValueOfPartialOfRootPoly(const int&, const Gambit::Vector<T>&)   const;
   Gambit::Vector<T> VectorOfPartials(const Gambit::Vector<T>&)              const;
   bool      PolyHasNoRootsIn(const gRectangle<T>&)            const;
   bool MultiaffinePolyHasNoRootsIn(const gRectangle<T>&)      const;
   bool PolyEverywhereNegativeIn(const gRectangle<T>&)         const;
   bool MultiaffinePolyEverywhereNegativeIn(const gRectangle<T>&) const;

   //friend gOutput& operator << (gOutput& output, const TreeOfPartials<T>& x);
};


// *********************************
//      class ListOfPartialTrees
// *********************************

template <class T> class ListOfPartialTrees {
private:
  Gambit::List<TreeOfPartials<T> >     PartialTreeList;

  // Disabling this operator -- we don't want it called
  ListOfPartialTrees<T> &operator=(const ListOfPartialTrees<T> &);

public:
   explicit ListOfPartialTrees(const Gambit::List<gPoly<T> >&);
   explicit ListOfPartialTrees(const gPolyList<T>&);
   ListOfPartialTrees(const ListOfPartialTrees<T> &);
   ~ListOfPartialTrees();

  // operators
  bool operator == (const ListOfPartialTrees&) const;
  bool operator != (const ListOfPartialTrees&) const;

  const TreeOfPartials<T> &operator[](int i) const { return PartialTreeList[i]; }

  // Information
  int Length() const { return PartialTreeList.Length(); }
  int Dmnsn() const { return PartialTreeList.front().Dmnsn(); }
  Gambit::Matrix<T> DerivativeMatrix(const Gambit::Vector<T>&)                  const; 
  Gambit::Matrix<T> DerivativeMatrix(const Gambit::Vector<T>&, const int&)      const; 
  Gambit::SquareMatrix<T> SquareDerivativeMatrix(const Gambit::Vector<T>&)      const; 
  Gambit::Vector<T> ValuesOfRootPolys(const Gambit::Vector<T>&, const int&)     const;
   T         MaximalNonconstantDifference(const int&,
					  const int&,
					  const Gambit::Vector<T>&, 
					  const Gambit::Vector<T>&)      const;

   //friend gOutput& operator << (gOutput& output, const ListOfPartialTrees<T>& x);
};

#endif // GPARTLTR_H
