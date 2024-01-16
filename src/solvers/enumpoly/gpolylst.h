//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/gpolylist.h
// Declaration of polynomial list type
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

#include "odometer.h"
#include "core/sqmatrix.h"
#include "gpoly.h"

//!
//! Simple class for compact reference to pairs of indices
//!
class index_pair {
private:
  const int m_first, m_second;

public:
  index_pair(int p_first, int p_second) : m_first(p_first), m_second(p_second) {}

  bool operator==(const index_pair &p_other) const
  {
    return (m_first == p_other.m_first && m_second == p_other.m_second);
  }
  bool operator!=(const index_pair &p_other) const
  {
    return (m_first != p_other.m_first || m_second != p_other.m_second);
  }
  int operator[](int p_index) const { return (p_index == 1) ? m_first : m_second; }
};

// ***********************
//      class gPolyList
// ***********************

template <class T>
class gPolyList
//  : private Counted<gPolyList<T> >
{
private:
  const gSpace *Space;
  const term_order *Order;
  Gambit::List<gPoly<T> *> List;

  // SubProcedures of ToSortedReducedGrobner
  void Sort(const term_order &);
  void CriterionTwo(Gambit::List<index_pair> &, const Gambit::List<index_pair> &, const int &,
                    const term_order &) const;
  // See Adams and Loustaunau, p. 130
  void Grobnerize(const term_order &);
  void GrobnerToMinimalGrobner(const term_order &);
  void MinimalGrobnerToReducedGrobner(const term_order &);

public:
  gPolyList(const gSpace *, const term_order *);
  gPolyList(const gSpace *, const term_order *, const Gambit::List<gPoly<T> *> &);
  gPolyList(const gSpace *, const term_order *, const Gambit::List<gPoly<T>> &);
  gPolyList(const gPolyList<T> &);

  ~gPolyList(); // Deletes all pointees

  // Operators
  gPolyList<T> &operator=(const gPolyList<T> &);

  bool operator==(const gPolyList<T> &) const;
  bool operator!=(const gPolyList<T> &) const;
  void operator+=(const gPoly<T> &);
  void operator+=(const gPolyList<T> &);

  void operator+=(gPoly<T> *); // NB - Doesn't copy pointee
                               // This can save a copy when one must create a
                               // polynomial, then do something in order to
                               // decide whether it should be added to the List

  gPoly<T> operator[](const int) const;

  // Residue of repeated reduction by members of the list
  gPoly<T> ReductionOf(const gPoly<T> &, const term_order &) const;
  bool SelfReduction(const int &, const term_order &);

  // Transform to canonical basis for associated ideal
  gPolyList<T> &ToSortedReducedGrobner(const term_order &);

  // New Coordinate Systems
  gPolyList<T> TranslateOfSystem(const Gambit::Vector<T> &) const;
  gPolyList<T> SystemInNewCoordinates(const Gambit::SquareMatrix<T> &) const;

  // Truncations
  gPolyList<T> InteriorSegment(int, int) const;

  // Information
  const gSpace *AmbientSpace() const;
  const term_order *TermOrder() const;
  int Length() const;
  int Dmnsn() const;
  bool IsMultiaffine() const;
  Gambit::List<gPoly<T>> UnderlyingList() const;
  Gambit::Vector<T> Evaluate(const Gambit::Vector<T> &) const;
  bool IsRoot(const Gambit::Vector<T> &) const;
  Gambit::RectArray<gPoly<T> *> DerivativeMatrix() const;
  gPoly<T> DetOfDerivativeMatrix() const;
  Gambit::Matrix<T> DerivativeMatrix(const Gambit::Vector<T> &) const;
  Gambit::SquareMatrix<T> SquareDerivativeMatrix(const Gambit::Vector<T> &) const;

  //  inline int static Count() { return Counted<gPolyList<T> >::objCount(); }

  // Conversion
  Gambit::List<gPoly<double>> ListTogDouble() const;
  Gambit::List<gPoly<double>> NormalizedList() const;
};

#endif // GPOLYLST_H
