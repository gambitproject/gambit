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
#include "mixed.imp"
#include "math/rational.h"

template<>
gNumber MixedProfile<gNumber>::Payoff(const gbtNfgOutcome &o, int pl) const
{ 
  return o.GetPayoff(m_nfg.GetPlayer(pl));
}

template<>
gRational MixedProfile<gRational>::Payoff(const gbtNfgOutcome &o, int pl) const
{ 
  return o.GetPayoff(m_nfg.GetPlayer(pl));
}

template<>
double MixedProfile<double>::Payoff(const gbtNfgOutcome &o, int pl) const
{ 
  return o.GetPayoffDouble(pl);
}

template class MixedProfile<double>;
template gbtOutput &operator<<(gbtOutput &, const MixedProfile<double> &);

template class MixedProfile<gRational>;
template gbtOutput &operator<<(gbtOutput &, const MixedProfile<gRational> &);

template class MixedProfile<gNumber>;
template gbtOutput &operator<<(gbtOutput &, const MixedProfile<gNumber> &);


#include "base/glist.imp"

template class gbtList<MixedProfile<double> >;
template class gbtList<MixedProfile<gRational> >;
template class gbtList<MixedProfile<gNumber> >;

#include "base/grblock.imp"

template class gbtRectBlock<double>;
template class gbtRectBlock<gRational>;

#include "base/garray.imp"
#include "base/gblock.imp"

template class gbtArray<gbt_nfg_strategy_rep *>;
template class gbtArray<gbt_nfg_outcome_rep *>;
template class gbtArray<gbt_nfg_player_rep *>;
template class gbtArray<gbtNfgStrategy>;

template class gbtBlock<gbt_nfg_strategy_rep *>;
template class gbtBlock<gbt_nfg_outcome_rep *>;
template class gbtBlock<gbt_nfg_player_rep *>;
template class gbtBlock<gbtNfgStrategy>;

template class gbtList<gbtNfgStrategy>;

template class gbtList<const gbtNfgSupport>;
