//
// File: gpolylst.h  --  Declaration of gpolylst data type
// 
// $Id$
//

#ifndef GPOLYLST_H
#define GPOLYLST_H

#include "gpoly.h"
#include "gsmatrix.h"
// #include "objcount.h"

// ***********************
//      class gPolyList
// ***********************


template <class T> class gPolyList
//  : private Counted<gPolyList<T> >
{ 
 private:
   const gSpace*      Space;
   const term_order*  Order;
   gList< gPoly<T> *> List;
   
   // SubProcedures of ToSortedReducedGrobner   
   void        Sort(const term_order &);
//   void        OldGrobnerize(const term_order &);
   void        CriterionTwo(      gList<index_pair>&, 
			    const gList<index_pair>&, 
			    const int&,
			    const term_order&) const;
                     // See Adams and Loustaunau, p. 130
   void        Grobnerize(const term_order &);
   void        GrobnerToMinimalGrobner(const term_order &);
   void        MinimalGrobnerToReducedGrobner(const term_order &);

 public:
   gPolyList(const gSpace *, const term_order*);  
   gPolyList(const gSpace *, const term_order*, const gList< gPoly<T> *> &);
   gPolyList(const gSpace *, const term_order*, const gList< gPoly<T> > &);
   gPolyList(const gPolyList<T> &);

   ~gPolyList();                 // Deletes all pointees

   // Operators
   gPolyList<T>& operator= (const gPolyList<T> &);

   bool       operator==(const gPolyList<T> &) const;
   bool       operator!=(const gPolyList<T> &) const;
   void       operator+=(const gPoly<T> &); 

   void       operator+=(      gPoly<T> *); // NB - Doesn't copy pointee
                              // This can save a copy when one must create a
                              // polynomial, then do something in order to 
                              // decide whether it should be added to the List

   gPoly<T>   operator[](const int)         const;

   // Residue of repeated reduction by members of the list
   gPoly<T>    ReductionOf(const gPoly<T> &, const term_order &) const;
   int         SelfReduction(const int &, const term_order &);

   // Transform to canonical basis for associated ideal
//   gPolyList<T>&  OldToSortedReducedGrobner(const term_order &);
   gPolyList<T>&  ToSortedReducedGrobner(const term_order &);

   // Information
   inline const gSpace*     AmbientSpace() const { return Space; }
   inline const term_order* TermOrder()    const { return Order; }
   inline const int         Length()       const { return List.Length(); }
   inline const int         Dmnsn()        const { return Space->Dmnsn(); }
   const gList<gPoly<T> >   UnderlyingList()                          const;
   const gVector<T>         Evaluate(const gVector<T>&)               const;
   const bool               IsRoot(const gVector<T>&)                 const;
   const gMatrix<T>         DerivativeMatrix(const gVector<T>&)       const;
   const gSquareMatrix<T>   SquareDerivativeMatrix(const gVector<T>&) const;

//  inline int static Count() { return Counted<gPolyList<T> >::objCount(); }

   // Conversion
   gList<gPoly<gDouble> > ListTogDouble() const;

friend gOutput& operator << (gOutput& output, const gPolyList<T>& x);
};  

// The following utility class belongs in gmisc.h, but not yet to 
// avoid cluttering everything at Caltech up.

//
// Simple class for compact reference to pairs of indices
//

class index_pair {
 private:
  const int first;
  const int second;

 public:
  index_pair(const int&, const int&);
  ~index_pair();

  operator == (const index_pair&) const;
  operator != (const index_pair&) const;
  operator [] (const int&) const; 

friend gOutput& operator << (gOutput& output, const index_pair& x);  
};

#endif GPOLYLST_H
