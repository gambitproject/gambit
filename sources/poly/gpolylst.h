//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of gbtPolyUni list type
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

#ifndef GPOLYLST_H
#define GPOLYLST_H

#include "base/odometer.h"
#include "math/gsmatrix.h"
#include "gpoly.h"

// ***********************
//      class gbtPolyMultiList
// ***********************

template <class T> class gbtPolyMultiList
//  : private Counted<gbtPolyMultiList<T> >
{ 
 private:
   const gbtPolySpace*      Space;
   const gbtPolyTermOrder*  Order;
   gbtList< gbtPolyMulti<T> *> List;
   
   // SubProcedures of ToSortedReducedGrobner   
   void        Sort(const gbtPolyTermOrder &);
   void        CriterionTwo(      gbtList<gbtIndexPair>&, 
			    const gbtList<gbtIndexPair>&, 
			    const int&,
			    const gbtPolyTermOrder&) const;
                     // See Adams and Loustaunau, p. 130
   void        Grobnerize(const gbtPolyTermOrder &);
   void        GrobnerToMinimalGrobner(const gbtPolyTermOrder &);
   void        MinimalGrobnerToReducedGrobner(const gbtPolyTermOrder &);

 public:
   gbtPolyMultiList(const gbtPolySpace *, const gbtPolyTermOrder*);  
   gbtPolyMultiList(const gbtPolySpace *, const gbtPolyTermOrder*, const gbtList< gbtPolyMulti<T> *> &);
   gbtPolyMultiList(const gbtPolySpace *, const gbtPolyTermOrder*, const gbtList< gbtPolyMulti<T> > &);
   gbtPolyMultiList(const gbtPolyMultiList<T> &);

   ~gbtPolyMultiList();                 // Deletes all pointees

   // Operators
   gbtPolyMultiList<T>& operator= (const gbtPolyMultiList<T> &);

   bool       operator==(const gbtPolyMultiList<T> &) const;
   bool       operator!=(const gbtPolyMultiList<T> &) const;
   void       operator+=(const gbtPolyMulti<T> &); 
   void       operator+=(const gbtPolyMultiList<T> &); 

   void       operator+=(      gbtPolyMulti<T> *); // NB - Doesn't copy pointee
                              // This can save a copy when one must create a
                              // gbtPolyUni, then do something in order to 
                              // decide whether it should be added to the List

   gbtPolyMulti<T>   operator[](const int)         const;

   // Residue of repeated reduction by members of the list
   gbtPolyMulti<T>    ReductionOf(const gbtPolyMulti<T> &, const gbtPolyTermOrder &) const;
   bool        SelfReduction(const int &, const gbtPolyTermOrder &);

   // Transform to canonical basis for associated ideal
   gbtPolyMultiList<T>&  ToSortedReducedGrobner(const gbtPolyTermOrder &);

  // New Coordinate Systems
   gbtPolyMultiList<T> TranslateOfSystem(const gbtVector<T>&)            const;
   gbtPolyMultiList<T> SystemInNewCoordinates(const gbtSquareMatrix<T>&) const;

  // Truncations
   gbtPolyMultiList<T> InteriorSegment(int, int)                       const;

   // Information
   const gbtPolySpace*            AmbientSpace()                            const;
   const gbtPolyTermOrder*        TermOrder()                               const;
   const int                Length()                                  const;
   const int                Dmnsn()                                   const;
   const bool               IsMultiaffine()                           const;
   gbtList<gbtPolyMulti<T> >         UnderlyingbtList()                          const;
   const gbtVector<T>         Evaluate(const gbtVector<T>&)               const;
   const bool               IsRoot(const gbtVector<T>&)                 const;
   const gbtRectArray<gbtPolyMulti<T>*> DerivativeMatrix()                     const;
   const gbtPolyMulti<T>           DetOfDerivativeMatrix()                   const;
   const gbtMatrix<T>         DerivativeMatrix(const gbtVector<T>&)       const;
   const gbtSquareMatrix<T>   SquareDerivativeMatrix(const gbtVector<T>&) const;

//  inline int static Count() { return Counted<gbtPolyMultiList<T> >::objCount(); }

   // Conversion
   gbtList<gbtPolyMulti<gbtDouble> > ListTogDouble()  const;
   gbtList<gbtPolyMulti<gbtDouble> > NormalizedList() const;
};  

template <class T> gbtOutput &operator<<(gbtOutput &, const gbtPolyMultiList<T> &);

#endif // GPOLYLST_H
