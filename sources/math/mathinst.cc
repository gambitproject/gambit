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

template class gbtArray<gbtInteger>;
template class gbtArray<gbtRational>;
template class gbtArray<gbtComplex>;
template class gbtArray<gbtNumber>;
template gbtOutput &operator<<(gbtOutput &, const gbtArray<gbtInteger> &);
template gbtOutput &operator<<(gbtOutput &, const gbtArray<gbtRational> &);
template gbtOutput &operator<<(gbtOutput &, const gbtArray<gbtText> &);

template class gbtBlock<gbtRational>;
template class gbtBlock<gbtNumber>;
template gbtOutput &operator<<(gbtOutput &, const gbtBlock<gbtRational> &);
template gbtOutput &operator<<(gbtOutput &, const gbtBlock<gbtNumber> &);

template class gbtList<gbtRational>;
template class gbtList<gbtNumber>;

template class gbtRectArray<gbtInteger>;
template class gbtRectArray<gbtRational>;
template class gbtRectArray<gbtNumber>;
template gbtOutput &operator<<(gbtOutput &, const gbtRectArray<gbtInteger> &);
template gbtOutput &operator<<(gbtOutput &, const gbtRectArray<gbtRational> &);
template gbtOutput &operator<<(gbtOutput &, const gbtRectArray<gbtNumber> &);

template class gbtRectBlock<gbtNumber>;
