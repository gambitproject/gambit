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

template class gbtList<int>;
template class gbtList<long>;
template class gbtList<double>;
template class gbtList<gbtText>;

template class gbtList<gbtBlock<int> >;
template class gbtList<gbtArray<int> >;
template class gbtList<gbtList<int> >;
#ifndef __BCC55__
template gbtOutput &operator<<(gbtOutput &, const gbtList<int> &);
#endif  // __BCC55__

template class gbtList<bool>;
template gbtOutput &operator<<(gbtOutput &, const gbtList<bool> &);
template class gbtList<gbtList<bool> >;
template gbtOutput &operator<<(gbtOutput &, const gbtList<gbtList<bool> > &);
template class gbtList<gbtList<gbtList<bool> > >;
