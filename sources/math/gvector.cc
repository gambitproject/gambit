//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Instantiation of vector types
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

#include "gvector.imp"
#include "gnumber.h"
#include "complex.h"

template class gbtVector<int>;
template class gbtVector<long>;
template class gbtVector<double>;
template class gbtVector<gbtInteger>;
template class gbtVector<gbtRational>;
template class gbtVector<gbtComplex>;
template class gbtVector<gbtNumber>;

template gbtOutput & operator<< (gbtOutput&, const gbtVector<int>&);
template gbtOutput & operator<< (gbtOutput&, const gbtVector<long>&);
template gbtOutput & operator<< (gbtOutput&, const gbtVector<double>&);
template gbtOutput & operator<< (gbtOutput&, const gbtVector<gbtInteger>&);
template gbtOutput & operator<< (gbtOutput&, const gbtVector<gbtRational>&);
template gbtOutput & operator<< (gbtOutput&, const gbtVector<gbtComplex>&);
template gbtOutput & operator<< (gbtOutput&, const gbtVector<gbtNumber>&);

template gbtVector<gbtDouble> TogDouble(const gbtVector<gbtRational>&);
template gbtVector<gbtDouble> TogDouble(const gbtVector<double>&);
template gbtVector<gbtDouble> TogDouble(const gbtVector<gbtDouble>&);

#include "base/glist.imp"

template class gbtList<gbtVector<double> >;
template class gbtList<gbtVector<gbtRational> >;
template class gbtList<gbtVector<gbtComplex> >;
template gbtOutput& operator << (gbtOutput& output, 
			       const gbtList<gbtVector<gbtRational> >&);
