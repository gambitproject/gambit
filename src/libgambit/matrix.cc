//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/matrix.cc
// Instantiation of common matrix types
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

#include "libgambit.h"
#include "matrix.imp"

using namespace Gambit;

template class Matrix<double>;
template class Matrix<Rational>;
template class Matrix<Integer>;
template class Matrix<int>;

template Vector<double> Gambit::operator*(const Vector<double> &,
					  const Matrix<double> &);
template Vector<Rational> Gambit::operator*(const Vector<Rational> &,
					    const Matrix<Rational> &);
template Vector<Integer> Gambit::operator*(const Vector<Integer> &,
					   const Matrix<Integer> &);
template Vector<int> Gambit::operator*(const Vector<int> &,
				       const Matrix<int> &);
