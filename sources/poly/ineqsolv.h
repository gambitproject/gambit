//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of gbtPolyIneqSolve 
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
in a given rectangle.  Ir is modeled on gbtPolyQuickSolve, but simpler.  There is
no Newton search, only repeated subdivision, queries at the center, and
tests against whether one of the inequalities is provably everywhere 
negative in the rectangle.
*/

/*
   The main constructor for this takes a gbtPolyMultiList<T>, interpreted as 
inequalities in the sense that, at a solution, all the polynomials
are required to be nonnegative.
*/


// ***********************
//      class gbtPolyIneqSolve
// ***********************

template <class T> class gbtPolyIneqSolve {
 private:
  const gbtPolyMultiList<T>                 System;
  const gbtPolyPartialTreeList<T>        TreesOfPartials;
        T                            Epsilon;
  //        bool                         HasBeenSolved;
  //        gbtTriState                    HasASolution;
  //        gbtVector<T>                   Sample;
  gbtStatus &m_status;

  // Routines Doing the Actual Work

  bool IsASolution(const gbtVector<T>&)                              const;

  bool SystemHasNoSolutionIn(const gRectangle<T>& r, gbtArray<int>&) const;

  const bool ASolutionExistsRecursion(const gRectangle<T>&, 
				            gbtVector<T>&,
				            gbtArray<int>&)          const;

 public:
   gbtPolyIneqSolve(const gbtPolyMultiList<T> &, gbtStatus &);  
   gbtPolyIneqSolve(const gbtPolyIneqSolve<T> &);
   ~gbtPolyIneqSolve();

   // Operators
   gbtPolyIneqSolve<T>& operator= (const gbtPolyIneqSolve<T> &);
   bool         operator==(const gbtPolyIneqSolve<T> &) const;
   bool         operator!=(const gbtPolyIneqSolve<T> &) const;

   // Information
   inline const gbtPolySpace*                  AmbientSpace()              const 
     { return System.AmbientSpace(); }
   inline const gbtPolyTermOrder*              TermOrder()                 const 
     { return System.TermOrder(); }
   inline const int                      Dmnsn()                     const 
     { return System.Dmnsn(); }
   inline const gbtPolyMultiList<T>             UnderlyingEquations()       const 
     { return System; }
   inline const T                        ErrorTolerance()            const
     { return Epsilon; }

  // The function that does everything
  const bool ASolutionExists(const gRectangle<T>&, gbtVector<T>& sample);

  void Output(gbtOutput &) const;
};  

template <class T> gbtOutput &operator<<(gbtOutput &output, const gbtPolyIneqSolve<T> &x);
#endif // INEQSOLV_H
