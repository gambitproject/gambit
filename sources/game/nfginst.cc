//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Instantiation of template classes used in normal forms
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

#include "behav.h"
#include "lexicon.h"
#include "mixed.imp"
#include "math/rational.h"

TEMPLATE_SPECIALIZATION()
const gNumber MixedProfile<gNumber>::Payoff(const gbtNfgOutcome &o, int pl) const
{ 
  return N->Payoff(o, pl);
}

TEMPLATE_SPECIALIZATION()
const gRational MixedProfile<gRational>::Payoff(const gbtNfgOutcome &o, int pl) const
{ 
  return N->Payoff(o, pl);
}

TEMPLATE_SPECIALIZATION()
const double MixedProfile<double>::Payoff(const gbtNfgOutcome &o, int pl) const
{ 
  return N->Payoff(o, pl);
}

template class MixedProfile<double>;
template gOutput &operator<<(gOutput &, const MixedProfile<double> &);

template class MixedProfile<gRational>;
template gOutput &operator<<(gOutput &, const MixedProfile<gRational> &);

template class MixedProfile<gNumber>;
template gOutput &operator<<(gOutput &, const MixedProfile<gNumber> &);


#include "base/glist.imp"

template class gList<MixedProfile<double> >;
template class gList<MixedProfile<gRational> >;
template class gList<MixedProfile<gNumber> >;

#include "base/grblock.imp"

template class gRectBlock<double>;
template class gRectBlock<gRational>;

#include "base/garray.imp"
#include "base/gblock.imp"

template class gArray<nfgSupportPlayer *>;
template class gArray<Strategy *>;
template class gArray<gbt_nfg_outcome_rep *>;

template class gArray<NFPlayer *>;
template class gBlock<Strategy *>;

template class gBlock<gbt_nfg_outcome_rep *>;

template class gList<Strategy *>;

template class gList<const NFSupport>;



