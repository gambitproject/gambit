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

#include "gmatrix.imp"
#include "rational.h"
#include "gnumber.h"

template class gMatrix<double>;
template class gMatrix<gRational>;
template class gMatrix<gInteger>;
template class gMatrix<int>;
template class gMatrix<gNumber>;

template gVector<double> operator*(const gVector<double> &,
				   const gMatrix<double> &);
template gVector<gRational> operator*(const gVector<gRational> &,
				      const gMatrix<gRational> &);
template gVector<gInteger> operator*(const gVector<gInteger> &,
				     const gMatrix<gInteger> &);
template gVector<int> operator*(const gVector<int> &,
				const gMatrix<int> &);
template gVector<gNumber> operator*(const gVector<gNumber> &,
				    const gMatrix<gNumber> &);

template gOutput &operator<<(gOutput &, const gMatrix<double> &);
template gOutput &operator<<(gOutput &, const gMatrix<gRational> &);
template gOutput &operator<<(gOutput &, const gMatrix<gInteger> &);
template gOutput &operator<<(gOutput &, const gMatrix<gDouble> &);
template gOutput &operator<<(gOutput &, const gMatrix<int> &);
template gOutput &operator<<(gOutput &, const gMatrix<gNumber> &);
