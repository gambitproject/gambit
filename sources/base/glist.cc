//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Provide instantiations for commonly-used lists
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

#include "base.h"
#include "glist.imp"

template class gList<int>;
template class gList<long>;
template class gList<double>;
template class gList<gText>;

template class gList<gBlock<int> >;
template class gList<gArray<int> >;
template class gList<gList<int> >;
#ifndef __BCC55__
template gOutput &operator<<(gOutput &, const gList<int> &);
#endif  // __BCC55__

template class gList<bool>;
template gOutput &operator<<(gOutput &, const gList<bool> &);
template class gList<gList<bool> >;
template gOutput &operator<<(gOutput &, const gList<gList<bool> > &);
template class gList<gList<gList<bool> > >;

