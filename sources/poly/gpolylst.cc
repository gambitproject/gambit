//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Instantiations of gbtPolyUni list types
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

//template class gbtPolyMultiList<int>;
//template gbtOutput &operator<<(gbtOutput &f, const gbtPolyMultiList<int> &y);

template class gbtPolyMultiList<gbtRational>;
template gbtOutput &operator<<(gbtOutput &f, const gbtPolyMultiList<gbtRational> &y);

//template class gbtPolyMultiList<double>;
//template gbtOutput &operator<<(gbtOutput &f, const gbtPolyMultiList<double> &y);

template class gbtPolyMultiList<gbtDouble>;
template gbtOutput &operator<<(gbtOutput &f, const gbtPolyMultiList<gbtDouble> &y);

template class gbtList<gbtIndexPair>;

/*
#include "objcount.imp"

template class Counted<gbtPolyMultiList<gbtDouble> >;
int Counted<gbtPolyMultiList<gbtDouble> >::numObjects = 0;

template class Counted<gbtPolyMultiList<double> >;
int Counted<gbtPolyMultiList<double> >::numObjects = 0;

template class Counted<gbtPolyMultiList<gbtRational> >;
int Counted<gbtPolyMultiList<gbtRational> >::numObjects = 0;
*/

template class gbtRectArray<gbtPolyMulti<gbtDouble>*>;
template class gbtArray<gbtPolyMulti<gbtDouble>*>;

template class gbtRectArray<gbtPolyMulti<gbtRational>*>;
//template class gbtArray<gbtPolyMulti<gbtRational>*>;

//template class gbtRectArray<gbtPolyMulti<double>*>;
//template class gbtArray<gbtPolyMulti<double>*>;
