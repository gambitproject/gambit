//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Definition of pure behavior profile for trees
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

#include "tree-behav-pure.h"

//-------------------------------------------------------------------------
//                    gbtPureBehavProfile member functions
//-------------------------------------------------------------------------

gbtPureBehavProfile::gbtPureBehavProfile(const gbtGame &efg)
  : m_efg(efg), profile(efg->NumPlayers())
{
  for (int pl = 1; pl <= efg->NumPlayers(); pl++)  {
    gbtGamePlayer player = efg->GetPlayer(pl);
    profile[pl] = new gbtArray<gbtGameAction>(player->NumInfosets());
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      (*profile[pl])[iset] = player->GetInfoset(iset)->GetAction(1);
    }
  }
}

gbtPureBehavProfile::gbtPureBehavProfile(const gbtPureBehavProfile &p)
  : m_efg(p.m_efg), profile(p.profile.Length())
{
  for (int pl = 1; pl <= profile.Length(); pl++)   {
    profile[pl] = new gbtArray<gbtGameAction>(p.profile[pl]->Length());
    for (int iset = 1; iset <= profile[pl]->Length(); iset++)
      (*profile[pl])[iset] = (*p.profile[pl])[iset];
  }
}

gbtPureBehavProfile::~gbtPureBehavProfile()
{
  for (int pl = 1; pl <= profile.Length(); delete profile[pl++]);
}

gbtPureBehavProfile &gbtPureBehavProfile::operator=(const gbtPureBehavProfile &p) 
{
  if (this != &p && m_efg == p.m_efg)   {
    for(int pl = 1; pl <= profile.Length(); pl++)
    for(int iset = 1; iset <= profile[pl]->Length(); iset++)
    (*profile[pl])[iset] = (*p.profile[pl])[iset];
  }
  return *this;
}

gbtRational gbtPureBehavProfile::operator()(const gbtGameAction &action) const
{
  if ((*profile[action->GetInfoset()->GetPlayer()->GetId()])
      [action->GetInfoset()->GetId()] == action)
    return 1;
  else
    return 0;
}

void gbtPureBehavProfile::Set(const gbtGameAction &action)
{
  (*profile[action->GetInfoset()->GetPlayer()->GetId()])
    [action->GetInfoset()->GetId()] = action;
}


gbtGameAction 
gbtPureBehavProfile::GetAction(const gbtGameInfoset &infoset) const
{
  return (*profile[infoset->GetPlayer()->GetId()])[infoset->GetId()];
}


gbtRational gbtPureBehavProfile::Payoff(const gbtGameNode &n, int pl) const
{
  gbtArray<gbtRational> payoff(m_efg->NumPlayers());
  for (int i = 1; i <= m_efg->NumPlayers(); i++)
    payoff[i] = 0;
  Payoff(n, 1, payoff);
  return payoff[pl];
}

void gbtPureBehavProfile::Payoff(const gbtGameNode &n, 
				 const gbtRational &prob, 
				 gbtArray<gbtRational> &payoff) const
{
  if (n->IsTerminal()) {
    // FIXME: vector version of this?
    for (int pl = 1; pl <= m_efg->NumPlayers(); pl++) {
      payoff[pl] += prob * n->GetOutcome()->GetPayoff(m_efg->GetPlayer(pl));
    }
  }
  else {
    if (n->GetPlayer()->IsChance()) {
      for (int i = 1; i <= n->NumChildren(); i++) {
	Payoff(n->GetChild(i),
	       prob * n->GetInfoset()->GetAction(i)->GetChanceProb(), payoff);
      }
    }
    else {
      Payoff(n->GetChild(GetAction(n->GetInfoset())), prob, payoff);
    }
  }
}

void gbtPureBehavProfile::InfosetProbs(const gbtGameNode &n,
				       const gbtRational &prob, 
				       gbtPVector<gbtRational> &probs) const
{
  if (!n->GetInfoset().IsNull() && n->GetPlayer()->IsChance()) {
    for (int i = 1; i <= n->NumChildren(); i++) {
      InfosetProbs(n->GetChild(i),
		   prob * n->GetInfoset()->GetAction(i)->GetChanceProb(), probs);
    }
  }
  else if (!n->GetInfoset().IsNull())  {
    probs(n->GetPlayer()->GetId(), n->GetInfoset()->GetId()) += prob;
    InfosetProbs(n->GetChild((*profile[n->GetPlayer()->GetId()])[n->GetInfoset()->GetId()]->GetId()),
		 prob, probs);
  }
}

void gbtPureBehavProfile::Payoff(gbtArray<gbtRational> &payoff) const
{
  for (int pl = 1; pl <= payoff.Length(); payoff[pl++] = 0);
  Payoff(m_efg->GetRoot(), 1, payoff);
}

void gbtPureBehavProfile::InfosetProbs(gbtPVector<gbtRational> &probs) const
{
  ((gbtVector<gbtRational> &) probs).operator=(gbtRational(0));
  InfosetProbs(m_efg->GetRoot(), 1, probs);
}

