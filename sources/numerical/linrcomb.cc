//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Instantiation of linear combination types
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

#include "linrcomb.imp"
#include "math/gvector.imp"
#include "math/gmatrix.imp"
#include "math/double.h"

template gbtVector<gbtDouble> operator* (const gbtVector<gbtDouble>&,
			  	     const gbtMatrix<gbtDouble>&);

template class LinearCombination<double>;
template class LinearCombination<gbtDouble>;
template class LinearCombination<gbtRational>;

template gbtOutput & operator<< (gbtOutput&, const LinearCombination<double>&);
template gbtOutput & operator<< (gbtOutput&, const LinearCombination<gbtDouble>&);
template gbtOutput & operator<< (gbtOutput&, const LinearCombination<gbtRational>&);
