//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Utility functions for normal form games
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

#include "base/gmisc.h"
#include "nfg.h"
#include "nfstrat.h"
#include "nfgciter.h"


Nfg *CompressNfg(const Nfg &nfg, const gbtNfgSupport &S)
{
  Nfg *N = new Nfg(S.NumStrats());
  
  N->SetTitle(nfg.GetTitle());

  for (int pl = 1; pl <= N->NumPlayers(); pl++)  {
    gbtNfgPlayer player = N->GetPlayer(pl);
    player.SetLabel(nfg.GetPlayer(pl).GetLabel());
    for (int st = 1; st <= N->NumStrats(pl); st++) {
      player.GetStrategy(st).SetLabel(S.GetStrategy(pl, st).GetLabel());
    }
  }

  for (int outc = 1; outc <= nfg.NumOutcomes(); outc++)  {
    gbtNfgOutcome outcome = N->NewOutcome();

    outcome.SetLabel(nfg.GetOutcomeId(outc).GetLabel());

    for (int pl = 1; pl <= N->NumPlayers(); pl++) {
      N->SetPayoff(outcome, pl, nfg.Payoff(nfg.GetOutcomeId(outc), pl));
    }
  }

  NfgContIter oiter(S);
  gbtNfgSupport newS(*N);
  NfgContIter niter(newS);
  
  do   {
    if (!oiter.GetOutcome().IsNull()) {
      niter.SetOutcome(N->GetOutcomeId(oiter.GetOutcome().GetId()));
    }
    else {
      niter.SetOutcome(0);
    }

    oiter.NextContingency();
  }  while (niter.NextContingency());

  return N;
}

bool IsConstSum(const Nfg &nfg)
{
  int pl, index;
  gNumber cvalue = (gNumber) 0;

  if (nfg.NumOutcomes() == 0)  return true;

  for (pl = 1; pl <= nfg.NumPlayers(); pl++)
    cvalue += nfg.Payoff(nfg.outcomes[1], pl);

  for (index = 2; index <= nfg.outcomes.Length(); index++)  {
    gNumber thisvalue = (gNumber) 0;

    for (pl = 1; pl <= nfg.NumPlayers(); pl++)
      thisvalue += nfg.Payoff(nfg.outcomes[index], pl);

    if (thisvalue > cvalue || thisvalue < cvalue)
      return false;
  }
  
  return true;
}

gNumber MinPayoff(const Nfg &nfg, int player)
{
  int index, p, p1, p2;
  gNumber minpay;
  
  if (nfg.NumOutcomes() == 0)  return (gNumber) 0;

  if (player)
    p1 = p2 = player;
  else   {
    p1 = 1;
    p2 = nfg.NumPlayers();
  }

  minpay = nfg.Payoff(nfg.outcomes[1], p1);
  for (index = 2; index <= nfg.outcomes.Length(); index++)  {
    for (p = p1; p <= p2; p++)
      if (nfg.Payoff(nfg.outcomes[index], p) < minpay)
	minpay = nfg.Payoff(nfg.outcomes[index], p);
  }
  return minpay;
}

gNumber MaxPayoff(const Nfg &nfg, int player)
{
  int index, p, p1, p2;
  gNumber maxpay;

  if (nfg.NumOutcomes() == 0)  return (gNumber) 0;

  if (player)
    p1 = p2 = player;
  else   {
    p1 = 1;
    p2 = nfg.NumPlayers();
  }

  maxpay = nfg.Payoff(nfg.outcomes[1], p1);
  for (index = 2; index <= nfg.outcomes.Length(); index++)  {
    for (p = p1; p <= p2; p++)
      if (nfg.Payoff(nfg.outcomes[index], p) > maxpay)
	maxpay = nfg.Payoff(nfg.outcomes[index], p);
  }
  return maxpay;
}

void RandomNfg(Nfg &nfg)
{
  for (int pl = 1; pl <= nfg.NumPlayers(); pl++)
    for (int outc = 1; outc <= nfg.outcomes.Length(); outc++)
      nfg.SetPayoff(nfg.outcomes[outc], pl, gNumber(Uniform()));
}  



