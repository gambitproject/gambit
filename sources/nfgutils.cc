//
// FILE: nfgutils.cc -- useful utilities for the normal form
//
// $Id$
//

#include "gmisc.h"
#include "nfg.h"
#include "nfplayer.h"
#include "nfstrat.h"
#include "nfgciter.h"


Nfg *CompressNfg(const Nfg &nfg, const NFSupport &S)
{
  Nfg *N = new Nfg(S.NumStrats());
  
  N->SetTitle(nfg.GetTitle());

  for (int pl = 1; pl <= N->NumPlayers(); pl++)  {
    NFPlayer *player = N->Players()[pl];
    player->SetName(nfg.Players()[pl]->GetName());
    for (int st = 1; st <= N->NumStrats(pl); st++) 
      player->Strategies()[st]->name = S.Strategies(pl)[st]->name;
  }

  for (int outc = 1; outc <= nfg.NumOutcomes(); outc++)  {
    NFOutcome *outcome = N->NewOutcome();

    outcome->SetName(nfg.Outcomes()[outc]->GetName());

    for (int pl = 1; pl <= N->NumPlayers(); pl++)
      N->SetPayoff(outcome, pl, nfg.Payoff(nfg.Outcomes()[outc], pl));
  }

  NfgContIter oiter(S);
  NFSupport newS(*N);
  NfgContIter niter(newS);
  
  do   {
    niter.SetOutcome(N->Outcomes()[oiter.GetOutcome()->GetNumber()]);

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



