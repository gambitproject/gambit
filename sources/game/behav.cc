//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Instantiation of behavior profile classes
//
// This file is part of Gambit
// Copyright (c) 2003, The Gambit Project
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

#include "behavtree.imp"
#include "base/glist.imp"


template class gbtBehavProfileTree<double>;
template class gbtBehavProfileTree<gbtRational>;
template class gbtBehavProfileTree<gbtNumber>;

template class gbtBehavProfile<double>;
template gbtOutput &operator<<(gbtOutput &, const gbtBehavProfile<double> &);

template class gbtBehavProfile<gbtRational>;
template gbtOutput &operator<<(gbtOutput &, const gbtBehavProfile<gbtRational> &);

template class gbtBehavProfile<gbtNumber>;
template gbtOutput &operator<<(gbtOutput &, const gbtBehavProfile<gbtNumber> &);

#if GBT_WITH_MP_FLOAT
template class gbtBehavProfile<gbtMPFloat>;
template gbtOutput &operator<<(gbtOutput &, 
			       const gbtBehavProfile<gbtMPFloat> &);
#endif // GBT_WITH_MP_FLOAT

template class gbtList<gbtBehavProfile<double> >;
template class gbtList<gbtBehavProfile<gbtRational> >;
template class gbtList<gbtBehavProfile<gbtNumber> >;


