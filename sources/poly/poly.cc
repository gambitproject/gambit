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

template class polynomial<gRational>;
template gOutput& operator << (gOutput& output, 
			       const polynomial<gRational>& x);

template class polynomial<int>;
template gOutput& operator << (gOutput& output,
			       const polynomial<int>& x);

template class polynomial<double>;
template gOutput& operator << (gOutput& output, 
			       const polynomial<double>& x);

template class polynomial<gDouble>;
template gOutput& operator << (gOutput& output, 
			       const polynomial<gDouble>& x);

template class polynomial<gNumber>;
template gOutput& operator << (gOutput& output, 
			       const polynomial<gNumber>& x);

#include "base/glist.imp"
template class gList< polynomial<gRational> >;
template gOutput& operator << (gOutput& output, 
			       const gList<polynomial<gRational> >&);

template class gList< polynomial<int> >;

template class gList< polynomial<double> >;
template gOutput& operator << (gOutput& output, 
			       const gList<polynomial<double> >&);

template class gList< polynomial<gDouble> >;

template class gList< polynomial<gNumber> >;
template gOutput& operator << (gOutput& output, 
			       const gList<polynomial<gNumber> >&);

