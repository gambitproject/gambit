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
#include "mixed.imp"
#include "base/glist.imp"

template<>
gbtNumber gbtBehavProfile<gbtNumber>::Payoff(const gbtGameOutcome &p_outcome,
					     int pl) const
{ 
  return p_outcome->GetPayoff(m_support.GetPlayer(pl));
}

template<>
gbtRational gbtBehavProfile<gbtRational>::Payoff(const gbtGameOutcome &p_outcome,
					  int pl) const
{ 
  return p_outcome->GetPayoff(m_support.GetPlayer(pl));
}

template<>
double gbtBehavProfile<double>::Payoff(const gbtGameOutcome &p_outcome, int pl) const
{ 
  return p_outcome->GetPayoff(m_support.GetPlayer(pl));
}

#if GBT_WITH_MP_FLOAT
template<>
gbtMPFloat gbtBehavProfile<gbtMPFloat>::Payoff(const gbtGameOutcome &p_outcome,
					       int pl) const
{
  return p_outcome->GetPayoff(m_support.GetPlayer(pl));
}
#endif // GBT_WITH_MP_FLOAT

template class gbtBehavProfile<double>;
template class gbtBehavAssessment<double>;
template gbtOutput &operator<<(gbtOutput &, const gbtBehavProfile<double> &);
template gbtOutput &operator<<(gbtOutput &, const gbtBehavAssessment<double> &);

template class gbtBehavProfile<gbtRational>;
template class gbtBehavAssessment<gbtRational>;
template gbtOutput &operator<<(gbtOutput &, const gbtBehavProfile<gbtRational> &);
template gbtOutput &operator<<(gbtOutput &, const gbtBehavAssessment<gbtRational> &);

template class gbtBehavProfile<gbtNumber>;
template class gbtBehavAssessment<gbtNumber>;
template gbtOutput &operator<<(gbtOutput &, const gbtBehavProfile<gbtNumber> &);
template gbtOutput &operator<<(gbtOutput &, const gbtBehavAssessment<gbtNumber> &);

#if GBT_WITH_MP_FLOAT
template class gbtBehavProfile<gbtMPFloat>;
template class gbtBehavAssessment<gbtMPFloat>;
template gbtOutput &operator<<(gbtOutput &, 
			       const gbtBehavProfile<gbtMPFloat> &);
template gbtOutput &operator<<(gbtOutput &,
			       const gbtBehavAssessment<gbtMPFloat> &);
#endif // GBT_WITH_MP_FLOAT

template class gbtList<gbtBehavProfile<double> >;
template class gbtList<gbtBehavProfile<gbtRational> >;
template class gbtList<gbtBehavProfile<gbtNumber> >;


template<>
gbtNumber gbtMixedProfile<gbtNumber>::Payoff(const gbtGameOutcome &p_outcome,
					     int pl) const
{ 
  return p_outcome->GetPayoff(m_support->GetPlayer(pl));
}

template<>
gbtRational gbtMixedProfile<gbtRational>::Payoff(const gbtGameOutcome &p_outcome, int pl) const
{ 
  return p_outcome->GetPayoff(m_support->GetPlayer(pl));
}

template<>
double gbtMixedProfile<double>::Payoff(const gbtGameOutcome &p_outcome, int pl) const
{ 
  return p_outcome->GetPayoff(m_support->GetPlayer(pl));
}

template class gbtMixedProfile<double>;
template gbtOutput &operator<<(gbtOutput &, const gbtMixedProfile<double> &);

template class gbtMixedProfile<gbtRational>;
template gbtOutput &operator<<(gbtOutput &, const gbtMixedProfile<gbtRational> &);

template class gbtMixedProfile<gbtNumber>;
template gbtOutput &operator<<(gbtOutput &, const gbtMixedProfile<gbtNumber> &);


template class gbtList<gbtMixedProfile<double> >;
template class gbtList<gbtMixedProfile<gbtRational> >;
template class gbtList<gbtMixedProfile<gbtNumber> >;

