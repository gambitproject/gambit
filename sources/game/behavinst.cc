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
gNumber BehavProfile<gNumber>::Payoff(const gbtEfgOutcome &p_outcome,
				      int pl) const
{ 
  //  return p_outcome->m_payoffs[pl];
  return m_efg->Payoff(p_outcome, m_efg->GetPlayer(pl));
}

template<>
gRational BehavProfile<gRational>::Payoff(const gbtEfgOutcome &p_outcome,
					  int pl) const
{ 
  //  return p_outcome->m_payoffs[pl];
  return m_efg->Payoff(p_outcome, m_efg->GetPlayer(pl));
}

template<>
double BehavProfile<double>::Payoff(const gbtEfgOutcome &p_outcome, int pl) const
{ 
  //  return p_outcome->m_doublePayoffs[pl];
  return m_efg->Payoff(p_outcome, m_efg->GetPlayer(pl));
}

template class BehavProfile<double>;
template class BehavAssessment<double>;
template gOutput &operator<<(gOutput &, const BehavProfile<double> &);
template gOutput &operator<<(gOutput &, const BehavAssessment<double> &);

template class BehavProfile<gRational>;
template class BehavAssessment<gRational>;
template gOutput &operator<<(gOutput &, const BehavProfile<gRational> &);
template gOutput &operator<<(gOutput &, const BehavAssessment<gRational> &);

template class BehavProfile<gNumber>;
template class BehavAssessment<gNumber>;
template gOutput &operator<<(gOutput &, const BehavProfile<gNumber> &);
template gOutput &operator<<(gOutput &, const BehavAssessment<gNumber> &);

template class PureBehavProfile<gNumber>;

#include "base/glist.imp"

template class gList<BehavProfile<double> >;
template class gList<BehavProfile<gRational> >;
template class gList<BehavProfile<gNumber> >;



