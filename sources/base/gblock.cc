//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION;
// Provide instantiations for commonly-used blocks
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
#include "garray.imp"
#include "gblock.imp"

template class gBlock<bool>;
template class gBlock<int>;
template class gBlock<long>;
template class gBlock<float>;
template class gBlock<double>;
template class gBlock<gText>;

template class gArray<gBlock<int> >;
template class gArray<gBlock<double> >;
template class gBlock<gArray<int> *>;
template class gBlock<gBlock<int> >;
template class gBlock<gBlock<double> >;
template class gArray<gArray<gText> >;
template class gBlock<gArray<gText> >;

#ifndef __BCC55__
template bool operator==(const gArray<gText> &, const gArray<gText> &);
template bool operator!=(const gArray<gText> &, const gArray<gText> &);
#endif  // __BCC55__

template gOutput &operator<<(gOutput &, const gBlock<bool> &);
#ifndef __BCC55__
template gOutput &operator<<(gOutput &, const gBlock<int> &);
#endif  // __BCC55__
template gOutput &operator<<(gOutput &, const gBlock<long> &);
template gOutput &operator<<(gOutput &, const gBlock<float> &);
#ifndef __BCC55__
template gOutput &operator<<(gOutput &, const gBlock<double> &);
#endif  // __BCC55__
template gOutput &operator<<(gOutput &, const gBlock<gText> &);
template gOutput &operator<<(gOutput &, const gArray<gBlock<int> > &);
template gOutput &operator<<(gOutput &, const gArray<gBlock<double> > &);
template gOutput &operator<<(gOutput &, const gBlock<gArray<int> *> &);
template gOutput &operator<<(gOutput &, const gBlock<gBlock<int> > &);
template gOutput &operator<<(gOutput &, const gBlock<gBlock<double> > &);


