//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Instantiation of behavior profile classes
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

#include "behav.imp"
#include "math/rational.h"

class ChanceInfoset;

template<>
gbtNumber BehavProfile<gbtNumber>::Payoff(const gbtEfgOutcome &p_outcome,
				      int pl) const
{ 
  return p_outcome.GetPayoff(m_efg.GetPlayer(pl));
}

template<>
gbtRational BehavProfile<gbtRational>::Payoff(const gbtEfgOutcome &p_outcome,
					  int pl) const
{ 
  return p_outcome.GetPayoff(m_efg.GetPlayer(pl));
}

template<>
double BehavProfile<double>::Payoff(const gbtEfgOutcome &p_outcome, int pl) const
{ 
  return p_outcome.GetPayoff(m_efg.GetPlayer(pl));
}

template class BehavProfile<double>;
template class BehavAssessment<double>;
template gbtOutput &operator<<(gbtOutput &, const BehavProfile<double> &);
template gbtOutput &operator<<(gbtOutput &, const BehavAssessment<double> &);

template class BehavProfile<gbtRational>;
template class BehavAssessment<gbtRational>;
template gbtOutput &operator<<(gbtOutput &, const BehavProfile<gbtRational> &);
template gbtOutput &operator<<(gbtOutput &, const BehavAssessment<gbtRational> &);

template class BehavProfile<gbtNumber>;
template class BehavAssessment<gbtNumber>;
template gbtOutput &operator<<(gbtOutput &, const BehavProfile<gbtNumber> &);
template gbtOutput &operator<<(gbtOutput &, const BehavAssessment<gbtNumber> &);

template class PureBehavProfile<gbtNumber>;

#include "base/glist.imp"

template class gbtList<BehavProfile<double> >;
template class gbtList<BehavProfile<gbtRational> >;
template class gbtList<BehavProfile<gbtNumber> >;
