//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Instantiation of mixed profile classes
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

#include "gamebase.h"
#include "behav.h"
#include "mixed.imp"
#include "mixedtable.imp"
#include "mixedtree.imp"
#include "base/glist.imp"

template class gbtMixedProfileRep<double>;
template class gbtMixedProfileRep<gbtRational>;
template class gbtMixedProfileRep<gbtNumber>;

template class gbtMixedProfileTable<double>;
template class gbtMixedProfileTable<gbtRational>;
template class gbtMixedProfileTable<gbtNumber>;

template class gbtMixedProfileTree<double>;
template class gbtMixedProfileTree<gbtRational>;
template class gbtMixedProfileTree<gbtNumber>;

template class gbtMixedProfile<double>;
template class gbtMixedProfile<gbtRational>;
template class gbtMixedProfile<gbtNumber>;
template gbtOutput &operator<<(gbtOutput &, 
			       const gbtMixedProfile<double> &);

template gbtOutput &operator<<(gbtOutput &,
			       const gbtMixedProfile<gbtRational> &);

template gbtOutput &operator<<(gbtOutput &,
			       const gbtMixedProfile<gbtNumber> &);


template class gbtList<gbtMixedProfile<double> >;
template class gbtList<gbtMixedProfile<gbtRational> >;
template class gbtList<gbtMixedProfile<gbtNumber> >;

