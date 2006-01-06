//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Instantiation of common matrix types
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

#include "libgambit.h"
#include "matrix.imp"

template class Gambit::Matrix<double>;
template class Gambit::Matrix<Gambit::Rational>;
template class Gambit::Matrix<Gambit::Integer>;
template class Gambit::Matrix<int>;

template gbtVector<double> Gambit::operator*(const gbtVector<double> &,
					     const Gambit::Matrix<double> &);
template gbtVector<Gambit::Rational> Gambit::operator*(const gbtVector<Gambit::Rational> &,
						  const Gambit::Matrix<Gambit::Rational> &);
template gbtVector<Gambit::Integer> Gambit::operator*(const gbtVector<Gambit::Integer> &,
						 const Gambit::Matrix<Gambit::Integer> &);
template gbtVector<int> Gambit::operator*(const gbtVector<int> &,
					  const Gambit::Matrix<int> &);
