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

template <class T> class gPolyList {
private:
  const VariableSpace *Space;
  Gambit::List<gPoly<T> *> List;

public:
  gPolyList(const VariableSpace *sp) : Space(sp) {}

  gPolyList(const VariableSpace *, const Gambit::List<gPoly<T> *> &);
  gPolyList(const VariableSpace *, const Gambit::List<gPoly<T>> &);
  gPolyList(const gPolyList<T> &);

  ~gPolyList(); // Deletes all pointees

  // Operators
  gPolyList<T> &operator=(const gPolyList<T> &);

  bool operator==(const gPolyList<T> &) const;
  bool operator!=(const gPolyList<T> &x) const { return !(*this == x); }
  void operator+=(const gPoly<T> &);
  void operator+=(const gPolyList<T> &);

  // Takes ownership of pointer
  void operator+=(gPoly<T> *poly) { List.push_back(poly); }

  const gPoly<T> &operator[](const int index) const { return *(List[index]); }

  // New Coordinate Systems
  gPolyList<T> TranslateOfSystem(const Gambit::Vector<T> &) const;
  gPolyList<T> SystemInNewCoordinates(const Gambit::SquareMatrix<T> &) const;

  // Information
  const VariableSpace *AmbientSpace() const { return Space; }
  int Length() const { return List.size(); }
  int Dmnsn() const { return Space->Dmnsn(); }
  bool IsMultiaffine() const;
  Gambit::Vector<T> Evaluate(const Gambit::Vector<T> &) const;

  // Conversion
  Gambit::List<gPoly<double>> NormalizedList() const;
};

#endif // GPOLYLST_H
