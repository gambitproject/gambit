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

template class gbtBlock<bool>;
template class gbtBlock<int>;
template class gbtBlock<long>;
template class gbtBlock<float>;
template class gbtBlock<double>;
template class gbtBlock<gbtText>;

template class gbtArray<gbtBlock<int> >;
template class gbtArray<gbtBlock<double> >;
template class gbtBlock<gbtArray<int> *>;
template class gbtBlock<gbtBlock<int> >;
template class gbtBlock<gbtBlock<double> >;
template class gbtArray<gbtArray<gbtText> >;
template class gbtBlock<gbtArray<gbtText> >;

#ifndef __BCC55__
template bool operator==(const gbtArray<gbtText> &, const gbtArray<gbtText> &);
template bool operator!=(const gbtArray<gbtText> &, const gbtArray<gbtText> &);
#endif  // __BCC55__

template gbtOutput &operator<<(gbtOutput &, const gbtBlock<bool> &);
#ifndef __BCC55__
template gbtOutput &operator<<(gbtOutput &, const gbtBlock<int> &);
#endif  // __BCC55__
template gbtOutput &operator<<(gbtOutput &, const gbtBlock<long> &);
template gbtOutput &operator<<(gbtOutput &, const gbtBlock<float> &);
#ifndef __BCC55__
template gbtOutput &operator<<(gbtOutput &, const gbtBlock<double> &);
#endif  // __BCC55__
template gbtOutput &operator<<(gbtOutput &, const gbtBlock<gbtText> &);
template gbtOutput &operator<<(gbtOutput &, const gbtArray<gbtBlock<int> > &);
template gbtOutput &operator<<(gbtOutput &, const gbtArray<gbtBlock<double> > &);
template gbtOutput &operator<<(gbtOutput &, const gbtBlock<gbtArray<int> *> &);
template gbtOutput &operator<<(gbtOutput &, const gbtBlock<gbtBlock<int> > &);
template gbtOutput &operator<<(gbtOutput &, const gbtBlock<gbtBlock<double> > &);
