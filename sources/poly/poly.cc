//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Instantiation of polynomial classes
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

template class polynomial<gbtRational>;
template gbtOutput& operator << (gbtOutput& output, 
			       const polynomial<gbtRational>& x);

template class polynomial<int>;
template gbtOutput& operator << (gbtOutput& output,
			       const polynomial<int>& x);

template class polynomial<double>;
template gbtOutput& operator << (gbtOutput& output, 
			       const polynomial<double>& x);

template class polynomial<gbtDouble>;
template gbtOutput& operator << (gbtOutput& output, 
			       const polynomial<gbtDouble>& x);

template class polynomial<gbtNumber>;
template gbtOutput& operator << (gbtOutput& output, 
			       const polynomial<gbtNumber>& x);

#include "base/glist.imp"
template class gbtList< polynomial<gbtRational> >;
template gbtOutput& operator << (gbtOutput& output, 
			       const gbtList<polynomial<gbtRational> >&);

template class gbtList< polynomial<int> >;

template class gbtList< polynomial<double> >;
template gbtOutput& operator << (gbtOutput& output, 
			       const gbtList<polynomial<double> >&);

template class gbtList< polynomial<gbtDouble> >;

template class gbtList< polynomial<gbtNumber> >;
template gbtOutput& operator << (gbtOutput& output, 
			       const gbtList<polynomial<gbtNumber> >&);
