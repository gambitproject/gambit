//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of extensive form contingency class
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

#include "efgcont.h"

//-------------------------------------------------------------------------
//                    gbtEfgContingency member functions
//-------------------------------------------------------------------------

gbtEfgContingency::gbtEfgContingency(const gbtEfgGame &efg)
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


gbtEfgContingency::gbtEfgContingency(const gbtEfgContingency &p)
  : m_efg(p.m_efg), profile(p.profile.Length())
{
  for (int pl = 1; pl <= profile.Length(); pl++)   {
    profile[pl] = new gbtArray<gbtGameAction>(p.profile[pl]->Length());
    for (int iset = 1; iset <= profile[pl]->Length(); iset++)
      (*profile[pl])[iset] = (*p.profile[pl])[iset];
  }
}

gbtEfgContingency::~gbtEfgContingency()
{
  for (int pl = 1; pl <= profile.Length(); delete profile[pl++]);
}

gbtEfgContingency &gbtEfgContingency::operator=(const gbtEfgContingency &p) 
{
  if (this != &p && m_efg == p.m_efg)   {
    for(int pl = 1; pl <= profile.Length(); pl++)
    for(int iset = 1; iset <= profile[pl]->Length(); iset++)
    (*profile[pl])[iset] = (*p.profile[pl])[iset];
  }
  return *this;
}

gbtNumber gbtEfgContingency::operator()(const gbtGameAction &action) const
{
  if ((*profile[action->GetInfoset()->GetPlayer()->GetId()])
      [action->GetInfoset()->GetId()] == action)
    return (gbtNumber) 1;
  else
    return (gbtNumber) 0;
}

void gbtEfgContingency::Set(const gbtGameAction &action)
{
  (*profile[action->GetInfoset()->GetPlayer()->GetId()])
    [action->GetInfoset()->GetId()] = action;
}

gbtGameAction gbtEfgContingency::GetAction(const gbtGameInfoset &infoset) const
{
  return (*profile[infoset->GetPlayer()->GetId()])[infoset->GetId()];
}

/* - The following is an attempt to eliminate all the extra work of
computing a vector of payoffs when only one agent's payoff is of
interest.  It runs into a problem that I don't know how to solve,
namely that compilation leads to a request for a cast from gbtNumber to
gbtRational. amm-8.98


void gbtEfgContingency::IndPayoff(const Node *n, 
				 const int &pl, 
				 const gbtNumber prob, 
				       gbtNumber &payoff) const
{
  if (n->IsTerminal())
    payoff += prob * (*payoffs)(n->GetOutcome()->GetNumber(), pl);
  
  if (n->IsNonterminal() && n->GetPlayer()->IsChance())
    for (int i = 1; i <= n->NumChildren(); i++) {
      IndPayoff(n->GetChild(i), pl,
	     prob * (gbtNumber) E->GetChanceProb(n->GetInfoset(), i), payoff);
    }
  else if (n->IsNonterminal())
    IndPayoff(n->GetChild((*profile[n->GetPlayer()->GetNumber()])[n->GetInfoset()->GetNumber()]->GetNumber()), pl,
	   prob, payoff);
}
*/

// The following could be shortened by using the player-specific one above.
// Whether this would be more efficient in fact is unknown. - AMM 7.6.98


gbtNumber gbtEfgContingency::Payoff(const gbtGameNode &n, int pl) const
{
  gbtArray<gbtNumber> payoff(m_efg->NumPlayers());
  for (int i = 1; i <= m_efg->NumPlayers(); i++)
    payoff[i] = (gbtNumber)0;
  Payoff(n, (gbtNumber)1, payoff);
  return payoff[pl];
}


void gbtEfgContingency::Payoff(const gbtGameNode &n, const gbtNumber &prob, 
				    gbtArray<gbtNumber> &payoff) const
{
  if (n->IsTerminal()) {
    // FIXME: vector version of this?
    if (!n->GetOutcome().IsNull()) {
      for (int pl = 1; pl <= m_efg->NumPlayers(); pl++) {
	payoff[pl] += prob * n->GetOutcome()->GetPayoff(m_efg->GetPlayer(pl));
      }
    }
  }
  else {
    if (n->GetPlayer()->IsChance()) {
      for (int i = 1; i <= n->NumChildren(); i++) {
	Payoff(n->GetChild(i),
	       prob * (gbtNumber) n->GetInfoset()->GetChanceProb(i), payoff);
      }
    }
    else {
      Payoff(n->GetChild(GetAction(n->GetInfoset())), prob, payoff);
    }
  }
}


void gbtEfgContingency::InfosetProbs(const gbtGameNode &n,
				       gbtNumber prob, gbtPVector<gbtNumber> &probs) const
{
  if (!n->GetInfoset().IsNull() && n->GetPlayer()->IsChance()) {
    for (int i = 1; i <= n->NumChildren(); i++) {
      InfosetProbs(n->GetChild(i),
		   prob * (gbtNumber) n->GetInfoset()->GetChanceProb(i), probs);
    }
  }
  else if (!n->GetInfoset().IsNull())  {
    probs(n->GetPlayer()->GetId(), n->GetInfoset()->GetId()) += prob;
    InfosetProbs(n->GetChild((*profile[n->GetPlayer()->GetId()])[n->GetInfoset()->GetId()]->GetId()),
		 prob, probs);
  }
}


void gbtEfgContingency::Payoff(gbtArray<gbtNumber> &payoff) const
{
  for (int pl = 1; pl <= payoff.Length(); payoff[pl++] = (gbtNumber) 0);
  Payoff(m_efg->GetRoot(), (gbtNumber) 1, payoff);
}


void gbtEfgContingency::InfosetProbs(gbtPVector<gbtNumber> &probs) const
{
  ((gbtVector<gbtNumber> &) probs).operator=((gbtNumber) 0);
  InfosetProbs(m_efg->GetRoot(), (gbtNumber) 1, probs);
}

gbtNumber gbtEfgContingency::Payoff(const gbtGameOutcome &p_outcome,
				 int pl) const
{
  if (!p_outcome.IsNull()) {
    return p_outcome->GetPayoff(m_efg->GetPlayer(pl));
  }
  else {
    return (gbtNumber) 0;
  }
}

