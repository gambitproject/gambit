//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Instantiation of gbtPolyUni classes
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

#include "poly.imp"
#include "math/double.h"
#include "math/gnumber.h"

template class gbtPolyUni<gbtRational>;
template gbtOutput& operator << (gbtOutput& output, 
			       const gbtPolyUni<gbtRational>& x);

template class gbtPolyUni<int>;
template gbtOutput& operator << (gbtOutput& output,
			       const gbtPolyUni<int>& x);

template class gbtPolyUni<double>;
template gbtOutput& operator << (gbtOutput& output, 
			       const gbtPolyUni<double>& x);

template class gbtPolyUni<gbtDouble>;
template gbtOutput& operator << (gbtOutput& output, 
			       const gbtPolyUni<gbtDouble>& x);

template class gbtPolyUni<gbtNumber>;
template gbtOutput& operator << (gbtOutput& output, 
			       const gbtPolyUni<gbtNumber>& x);

#include "base/glist.imp"
template class gbtList< gbtPolyUni<gbtRational> >;
template gbtOutput& operator << (gbtOutput& output, 
			       const gbtList<gbtPolyUni<gbtRational> >&);

template class gbtList< gbtPolyUni<int> >;

template class gbtList< gbtPolyUni<double> >;
template gbtOutput& operator << (gbtOutput& output, 
			       const gbtList<gbtPolyUni<double> >&);

template class gbtList< gbtPolyUni<gbtDouble> >;

template class gbtList< gbtPolyUni<gbtNumber> >;
template gbtOutput& operator << (gbtOutput& output, 
			       const gbtList<gbtPolyUni<gbtNumber> >&);
