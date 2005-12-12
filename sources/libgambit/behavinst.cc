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

#include "libgambit.h"
#include "behav.imp"

template<>
gbtNumber gbtBehavProfile<gbtNumber>::Payoff(gbtEfgOutcome *p_outcome,
				      int pl) const
{ 
  return p_outcome->m_ratPayoffs[pl];
}

template<>
gbtRational gbtBehavProfile<gbtRational>::Payoff(gbtEfgOutcome *p_outcome,
					  int pl) const
{ 
  return p_outcome->m_ratPayoffs[pl];
}

template<>
double gbtBehavProfile<double>::Payoff(gbtEfgOutcome *p_outcome, int pl) const
{ 
  return p_outcome->m_doublePayoffs[pl];
}

template class gbtBehavProfile<double>;
template class gbtBehavProfile<gbtRational>;
template class gbtBehavProfile<gbtNumber>;

//-------------------------------------------------------------------------
//                    gbtPureBehavProfile member functions
//-------------------------------------------------------------------------

gbtPureBehavProfile::gbtPureBehavProfile(gbtEfgGame *p_efg)
  : m_efg(p_efg), m_profile(m_efg->NumPlayers())
{
  for (int pl = 1; pl <= m_efg->NumPlayers(); pl++)  {
    gbtEfgPlayer *player = m_efg->GetPlayer(pl);
    m_profile[pl] = gbtArray<gbtEfgAction *>(player->NumInfosets());
    for (int iset = 1; iset <= player->NumInfosets(); iset++)
      m_profile[pl][iset] = player->GetInfoset(iset)->GetAction(1);
  }
}

gbtPureBehavProfile &gbtPureBehavProfile::operator=(const gbtPureBehavProfile &p) 
{
  if (this != &p && m_efg == p.m_efg)   {
    for (int pl = 1; pl <= m_profile.Length(); pl++) {
      for (int iset = 1; iset <= m_profile[pl].Length(); iset++) {
	m_profile[pl][iset] = p.m_profile[pl][iset];
      }
    }
  }
  return *this;
}

gbtRational gbtPureBehavProfile::operator()(gbtEfgAction *action) const
{
  if (m_profile[action->GetInfoset()->GetPlayer()->GetNumber()]
      [action->GetInfoset()->GetNumber()] == action)
    return 1;
  else
    return 0;
}

void gbtPureBehavProfile::Set(gbtEfgAction *action)
{
  m_profile[action->GetInfoset()->GetPlayer()->GetNumber()]
    [action->GetInfoset()->GetNumber()] = action;
}

void gbtPureBehavProfile::Set(gbtEfgPlayer *player,
			   const gbtArray<gbtEfgAction *> &actions)
{
  m_profile[player->GetNumber()] = actions;
}

gbtEfgAction *gbtPureBehavProfile::GetAction(gbtEfgInfoset *infoset) const
{
  return m_profile[infoset->GetPlayer()->GetNumber()][infoset->GetNumber()];
}

gbtRational gbtPureBehavProfile::Payoff(const gbtEfgNode *n, int pl) const
{
  gbtArray<gbtRational> payoff(m_efg->NumPlayers());
  for (int i = 1; i <= m_efg->NumPlayers(); payoff[i++] = 0);
  GetPayoff(n, gbtRational(1), payoff);
  return payoff[pl];
}

void gbtPureBehavProfile::GetPayoff(const gbtEfgNode *n, const gbtRational &prob, 
				 gbtArray<gbtRational> &payoff) const
{
  if (n->IsTerminal()) {
    if (n->GetOutcome()) {
      for (int pl = 1; pl <= m_efg->NumPlayers(); pl++) {
	payoff[pl] += prob * n->GetOutcome()->GetPayoff(pl); 
      }
    }
  }
  
  else
    if (n->GetPlayer()->IsChance()) {
      for (int i = 1; i <= n->NumChildren(); i++) {
	GetPayoff(n->GetChild(i),
		  prob * n->GetInfoset()->GetActionProb(i), payoff);
      }
    }
    else {
      GetPayoff(n->GetChild(GetAction(n->GetInfoset())->GetNumber()), 
		prob, payoff);
    }
}

void gbtPureBehavProfile::InfosetProbs(gbtEfgNode *n, const gbtRational &prob, 
				    gbtPVector<gbtRational> &probs) const
{
  if (n->GetInfoset() && n->GetPlayer()->IsChance())
    for (int i = 1; i <= n->NumChildren(); i++)
      InfosetProbs(n->GetChild(i),
		   prob * n->GetInfoset()->GetActionProb(i), probs);
  else if (n->GetInfoset())  {
    probs(n->GetPlayer()->GetNumber(), n->GetInfoset()->GetNumber()) += prob;
    InfosetProbs(n->GetChild(m_profile[n->GetPlayer()->GetNumber()][n->GetInfoset()->GetNumber()]->GetNumber()),
		 prob, probs);
  }
}

void gbtPureBehavProfile::Payoff(gbtArray<gbtRational> &payoff) const
{
  for (int pl = 1; pl <= payoff.Length(); payoff[pl++] = 0);
  GetPayoff(m_efg->GetRoot(), gbtRational(1), payoff);
}

void gbtPureBehavProfile::InfosetProbs(gbtPVector<gbtRational> &probs) const
{
  probs = gbtRational(0);
  InfosetProbs(m_efg->GetRoot(), gbtRational(1), probs);
}




