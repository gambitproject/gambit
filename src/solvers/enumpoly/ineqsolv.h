//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/ineqsolv.h
// Declaration of IneqSolv
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

#include "gambit.h"

#include "gsolver.h"
#include "odometer.h"
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
  const gPolyList<T> System;
  const ListOfPartialTrees<T> TreesOfPartials;
  T Epsilon;
  //        bool                         HasBeenSolved;
  //        gTriState                    HasASolution;
  //        Gambit::Vector<T>                   Sample;

  // Routines Doing the Actual Work

  bool IsASolution(const Gambit::Vector<T> &) const;

  bool SystemHasNoSolutionIn(const gRectangle<T> &r, Gambit::Array<int> &) const;

  bool ASolutionExistsRecursion(const gRectangle<T> &, Gambit::Vector<T> &,
                                Gambit::Array<int> &) const;

public:
  explicit IneqSolv(const gPolyList<T> &);
  IneqSolv(const IneqSolv<T> &);
  ~IneqSolv();

  // Operators
  IneqSolv<T> &operator=(const IneqSolv<T> &);
  bool operator==(const IneqSolv<T> &) const;
  bool operator!=(const IneqSolv<T> &) const;

  // Information
  inline const gSpace *AmbientSpace() const { return System.AmbientSpace(); }
  inline const term_order *TermOrder() const { return System.TermOrder(); }
  inline int Dmnsn() const { return System.Dmnsn(); }
  inline gPolyList<T> UnderlyingEquations() const { return System; }
  inline T ErrorTolerance() const { return Epsilon; }

  // The function that does everything
  bool ASolutionExists(const gRectangle<T> &, Gambit::Vector<T> &sample);
};

#endif // INEQSOLV_H
