//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Instantiations for common gbtArray classes
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

template class gbtArray<bool>;
template class gbtArray<int>;
template class gbtArray<double>;
template class gbtArray<gbtText>;

template class gbtArray<gbtArray<gbtText> >;
template class gbtArray<gbtList<bool> >;
template class gbtArray<gbtList<gbtList<bool> > >;

template class gbtArray<gbtArray<int> *>;

template bool operator==(const gbtArray<int> &, const gbtArray<int> &);
template bool operator!=(const gbtArray<int> &, const gbtArray<int> &);
template bool operator==(const gbtArray<gbtList<bool> > &, 
			 const gbtArray<gbtList<bool> > &);
template bool operator!=(const gbtArray<gbtList<bool> > &, 
			 const gbtArray<gbtList<bool> > &);
template bool operator==(const gbtArray<gbtList<gbtList<bool> > > &, 
			 const gbtArray<gbtList<gbtList<bool> > > &);
template bool operator!=(const gbtArray<gbtList<gbtList<bool> > > &, 
			 const gbtArray<gbtList<gbtList<bool> > > &);

template gbtOutput &operator<<(gbtOutput &, const gbtArray<bool> &);
template gbtOutput &operator<<(gbtOutput &, const gbtArray<int> &);
template gbtOutput &operator<<(gbtOutput &, const gbtArray<double> &);
template gbtOutput &operator<<(gbtOutput &, const gbtArray<gbtText> &);

