//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Instantiations of polynomial list types
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

#include "gpolylst.imp"
#include "math/double.h"
#include "base/glist.imp"
#include "base/garray.imp"
#include "base/grarray.imp"
#include "math/gnumber.h"
#include "math/gmatrix.imp"

//template class gPolyList<int>;
//template gbtOutput &operator<<(gbtOutput &f, const gPolyList<int> &y);

template class gPolyList<gRational>;
template gbtOutput &operator<<(gbtOutput &f, const gPolyList<gRational> &y);

//template class gPolyList<double>;
//template gbtOutput &operator<<(gbtOutput &f, const gPolyList<double> &y);

template class gPolyList<gDouble>;
template gbtOutput &operator<<(gbtOutput &f, const gPolyList<gDouble> &y);

template class gbtList<gbtIndexPair>;

/*
#include "objcount.imp"

template class Counted<gPolyList<gDouble> >;
int Counted<gPolyList<gDouble> >::numObjects = 0;

template class Counted<gPolyList<double> >;
int Counted<gPolyList<double> >::numObjects = 0;

template class Counted<gPolyList<gRational> >;
int Counted<gPolyList<gRational> >::numObjects = 0;
*/

template class gbtRectArray<gPoly<gDouble>*>;
template class gbtArray<gPoly<gDouble>*>;

template class gbtRectArray<gPoly<gRational>*>;
//template class gbtArray<gPoly<gRational>*>;

//template class gbtRectArray<gPoly<double>*>;
//template class gbtArray<gPoly<double>*>;
