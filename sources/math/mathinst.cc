//
// $Source$
// $Date$
// $Revision$
// 
// DESCRIPTION:
// Instantiation of math container types
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

#include "base/garray.imp"
#include "base/gblock.imp"
#include "base/glist.imp"
#include "base/grarray.imp"
#include "base/grblock.imp"
#include "gnumber.h"
#include "complex.h"

template class gArray<gInteger>;
template class gArray<gRational>;
template class gArray<gComplex>;
template class gArray<gNumber>;
template gOutput &operator<<(gOutput &, const gArray<gInteger> &);
template gOutput &operator<<(gOutput &, const gArray<gRational> &);
template gOutput &operator<<(gOutput &, const gArray<gText> &);

template class gBlock<gRational>;
template class gBlock<gNumber>;
template gOutput &operator<<(gOutput &, const gBlock<gRational> &);
template gOutput &operator<<(gOutput &, const gBlock<gNumber> &);

template class gList<gRational>;
template class gList<gNumber>;

template class gRectArray<gInteger>;
template class gRectArray<gRational>;
template class gRectArray<gNumber>;
template gOutput &operator<<(gOutput &, const gRectArray<gInteger> &);
template gOutput &operator<<(gOutput &, const gRectArray<gRational> &);
template gOutput &operator<<(gOutput &, const gRectArray<gNumber> &);

template class gRectBlock<gNumber>;

