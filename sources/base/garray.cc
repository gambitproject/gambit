//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Instantiations for common gArray classes
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

template class gArray<bool>;
template class gArray<int>;
template class gArray<long>;
template class gArray<float>;
template class gArray<double>;
template class gArray<long double>;
template class gArray<gText>;
template class gArray<gArray<int> >;
template class gArray<gArray<int> *>;
template class gArray<gArray<double> >;
template class gArray<void *>;
template class gArray<gList<bool> >;
template class gArray<gList<gList<bool> > >;
template bool operator==(const gArray<int> &, const gArray<int> &);
template bool operator!=(const gArray<int> &, const gArray<int> &);
template bool operator==(const gArray<gList<bool> > &, 
			 const gArray<gList<bool> > &);
template bool operator!=(const gArray<gList<bool> > &, 
			 const gArray<gList<bool> > &);
template bool operator==(const gArray<gList<gList<bool> > > &, 
			 const gArray<gList<gList<bool> > > &);
template bool operator!=(const gArray<gList<gList<bool> > > &, 
			 const gArray<gList<gList<bool> > > &);

template gOutput &operator<<(gOutput &, const gArray<bool> &);
#ifndef __BCC55__
template gOutput &operator<<(gOutput &, const gArray<int> &);
#endif  // __BCC55__
template gOutput &operator<<(gOutput &, const gArray<long> &);
template gOutput &operator<<(gOutput &, const gArray<float> &);
#ifndef __BCC55__
template gOutput &operator<<(gOutput &, const gArray<double> &);
#endif  // __BCC55__
template gOutput &operator<<(gOutput &, const gArray<gArray<int> > &);
template gOutput &operator<<(gOutput &, const gArray<gArray<int> *> &);
template gOutput &operator<<(gOutput &, const gArray<gArray<double> > &);

