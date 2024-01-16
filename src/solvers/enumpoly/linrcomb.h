//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/linrcomb.h
// Find linear weights for dependencies between rows of a matrix
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

/*
   This file contains the file for a class that performs a very specific
computation, namely asking whether the last row of a matrix is a linear
combination of the other rows, and if so computing the coefficients of
a linear dependence.
   All computation is done in the constructor.  The constructor should
only be called when there is a guarantee (which this class may be used
to compute!) that the rows other than the last are linearly
independent.
*/

#ifndef LINRCOMB_H
#define LINRCOMB_H

#include "core/rational.h"
#include "core/matrix.h"

template <class T> class LinearCombination {
private:
  Gambit::Matrix<T> scrambled;
  Gambit::Vector<T> weights;
  bool last_row_is_spanned;

  void AddMultipleOfRowiToRowj(const int &i, const int &j, const T &scalar);
  void AddMultipleOfRowiToRowj(const int &i, const int &j, const T &scalar, Gambit::Matrix<T> &B);

  // This function is left unimplemented to avoid copying
  LinearCombination<T> &operator=(const LinearCombination<T> &);

public:
  // Constructors, Destructor, Constructive Operators
  explicit LinearCombination(const Gambit::Matrix<T> &);
  LinearCombination(const LinearCombination<T> &);

  virtual ~LinearCombination();

  // Comparison Operators
  bool operator==(const LinearCombination<T> &) const;
  bool operator!=(const LinearCombination<T> &) const;

  // Information
  bool LastRowIsSpanned() const;
  Gambit::Vector<T> LinearDependence() const;
};

#endif // LinearCombination_H
