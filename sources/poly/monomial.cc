//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Instantiation of monomial classes
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

#include "monomial.imp"
#include "base/glist.imp"
#include "math/double.h"
#include "base/garray.imp"
#include "base/gblock.imp"
#include "math/gnumber.h"

template class gMono<int>;
template gOutput& operator << (gOutput& output, const gMono<int>& x);

template class gMono<double>;
template gOutput& operator << (gOutput& output, const gMono<double>& x);

template class gMono<gRational>;
template gOutput& operator << (gOutput& output, const gMono<gRational>& x);

template class gMono<gNumber>;
template gOutput& operator << (gOutput& output, const gMono<gNumber>& x);

template class gMono<gDouble>;
template gOutput& operator << (gOutput& output, const gMono<gDouble>& x);

template class gList< gMono<int> >;
template class gList< gMono<double> >;
template class gList< gMono<gRational> >;
template class gList< gMono<gNumber> >;

template class gList< gMono<gDouble> >;
