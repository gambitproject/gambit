//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Instantiation of interval type
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

#include <assert.h>
#include "interval.imp"
#include "base/glist.imp"
#include "math/double.h"
#include "math/gnumber.h"

template class gInterval<gbtRational>;
template class gbtList< gInterval<gbtRational> >;
#ifndef __BCC55__
template gbtOutput& operator << (gbtOutput& output, const gInterval<gbtRational>& x);
#endif  // __BCC55__

template class gInterval<int>;
template class gbtList< gInterval<int> >;
#ifndef __BCC55__
template gbtOutput& operator << (gbtOutput& output, const gInterval<int>& x);
#endif  // __BCC55__


template class gInterval<double>;
template class gbtList< gInterval<double> >;
#ifndef __BCC55__
template gbtOutput& operator << (gbtOutput& output, const gInterval<double>& x);
#endif  // __BCC55__

template class gInterval<gbtDouble>;
template class gbtList< gInterval<gbtDouble> >;
#ifndef __BCC55__
template gbtOutput& operator << (gbtOutput& output, const gInterval<gbtDouble>& x);
#endif  // __BCC55__

template class gInterval<gbtNumber>;
template class gbtList< gInterval<gbtNumber> >;
#ifndef __BCC55__
template gbtOutput& operator << (gbtOutput& output, const gInterval<gbtNumber>& x);
#endif  // __BCC55__
