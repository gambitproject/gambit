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

template class gbtMatrix<double>;
template class gbtMatrix<gbtRational>;
template class gbtMatrix<gbtInteger>;
template class gbtMatrix<int>;
template class gbtMatrix<gbtNumber>;

template gbtVector<double> operator*(const gbtVector<double> &,
				   const gbtMatrix<double> &);
template gbtVector<gbtRational> operator*(const gbtVector<gbtRational> &,
				      const gbtMatrix<gbtRational> &);
template gbtVector<gbtInteger> operator*(const gbtVector<gbtInteger> &,
				     const gbtMatrix<gbtInteger> &);
template gbtVector<int> operator*(const gbtVector<int> &,
				const gbtMatrix<int> &);
template gbtVector<gbtNumber> operator*(const gbtVector<gbtNumber> &,
				    const gbtMatrix<gbtNumber> &);

template gbtOutput &operator<<(gbtOutput &, const gbtMatrix<double> &);
template gbtOutput &operator<<(gbtOutput &, const gbtMatrix<gbtRational> &);
template gbtOutput &operator<<(gbtOutput &, const gbtMatrix<gbtInteger> &);
template gbtOutput &operator<<(gbtOutput &, const gbtMatrix<gbtDouble> &);
template gbtOutput &operator<<(gbtOutput &, const gbtMatrix<int> &);
template gbtOutput &operator<<(gbtOutput &, const gbtMatrix<gbtNumber> &);
