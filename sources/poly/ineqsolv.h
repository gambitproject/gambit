//
// File: ineqsolv.h  --  Declaration of gpolylst data type
// 
// $Id$
//

#ifndef INEQSOLV_H
#define INEQSOLV_H

#include "base/base.h"
#include "base/gstatus.h"

#include "gsolver.h"
#include "base/odometer.h"
#include "rectangl.h"
#include "gpoly.h"
#include "gpolylst.h"
#include "gpartltr.h"

/*
    The class described in this file is a method of determining whether a
system of weak inequalities has a solution (a point where all are satisfied)
in a given rectangle.  Ir is modeled on QuikSolv, but simpler.  There is
no Newton search, only repeated subdivision, queries at the center, and
tests against whether one of the inequalities is provably everywhere 
negative in the rectangle.
*/

/*
   The main constructor for this takes a gPolyList<T>, interpreted as 
inequalities in the sense that, at a solution, all the polynomials
are required to be nonnegative.
*/


// ***********************
//      class IneqSolv
// ***********************

template <class T> class IneqSolv {
 private:
  const gPolyList<T>                 System;
  const ListOfPartialTrees<T>        TreesOfPartials;
        T                            Epsilon;
  //        bool                         HasBeenSolved;
  //        gTriState                    HasASolution;
  //        gVector<T>                   Sample;
  gStatus &m_status;

  // Routines Doing the Actual Work

  bool IsASolution(const gVector<T>&)                              const;

  bool SystemHasNoSolutionIn(const gRectangle<T>& r, gArray<int>&) const;

  const bool ASolutionExistsRecursion(const gRectangle<T>&, 
				            gVector<T>&,
				            gArray<int>&)          const;

 public:
   IneqSolv(const gPolyList<T> &, gStatus &);  
   IneqSolv(const IneqSolv<T> &);
   ~IneqSolv();

   // Operators
   IneqSolv<T>& operator= (const IneqSolv<T> &);
   bool         operator==(const IneqSolv<T> &) const;
   bool         operator!=(const IneqSolv<T> &) const;

   // Information
   inline const gSpace*                  AmbientSpace()              const 
     { return System.AmbientSpace(); }
   inline const term_order*              TermOrder()                 const 
     { return System.TermOrder(); }
   inline const int                      Dmnsn()                     const 
     { return System.Dmnsn(); }
   inline const gPolyList<T>             UnderlyingEquations()       const 
     { return System; }
   inline const T                        ErrorTolerance()            const
     { return Epsilon; }

  // The function that does everything
  const bool ASolutionExists(const gRectangle<T>&, gVector<T>& sample);

  void Output(gOutput &) const;
};  

template <class T> gOutput &operator<<(gOutput &output, const IneqSolv<T> &x);
#endif // INEQSOLV_H
