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
#include "math/gmath.h"

template<> 
gbtNumber gbtMixedProfile<gbtNumber>::Payoff(const gbtNfgOutcome &o, 
					     int pl) const
{ 
  return o.GetPayoff(m_nfg.GetPlayer(pl));
}

template<>
gbtRational gbtMixedProfile<gbtRational>::Payoff(const gbtNfgOutcome &o, 
						 int pl) const
{ 
  return o.GetPayoff(m_nfg.GetPlayer(pl));
}

template<>
double gbtMixedProfile<double>::Payoff(const gbtNfgOutcome &o, int pl) const
{ 
  return o.GetPayoffDouble(pl);
}

#if GBT_WITH_MP_FLOAT
template<>
gbtMPFloat gbtMixedProfile<gbtMPFloat>::Payoff(const gbtNfgOutcome &o,
					       int pl) const
{
  return o.GetPayoffDouble(pl);
}
#endif  // GBT_WITH_MP_FLOAT

template class gbtMixedProfile<double>;
template gbtOutput &operator<<(gbtOutput &, const gbtMixedProfile<double> &);

template class gbtMixedProfile<gbtRational>;
template gbtOutput &operator<<(gbtOutput &, const gbtMixedProfile<gbtRational> &);

template class gbtMixedProfile<gbtNumber>;
template gbtOutput &operator<<(gbtOutput &, const gbtMixedProfile<gbtNumber> &);

#if GBT_WITH_MP_FLOAT
template class gbtMixedProfile<gbtMPFloat>;
template gbtOutput &operator<<(gbtOutput &,
			       const gbtMixedProfile<gbtMPFloat> &);
#endif // GBT_WITH_MP_FLOAT


#include "base/glist.imp"

template class gbtList<gbtMixedProfile<double> >;
template class gbtList<gbtMixedProfile<gbtRational> >;
template class gbtList<gbtMixedProfile<gbtNumber> >;

#include "base/grblock.imp"

template class gbtRectBlock<double>;
template class gbtRectBlock<gbtRational>;

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
