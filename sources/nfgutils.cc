//
// FILE nfgutils.cc -- useful utilities for the normal form
//
// @(#)nfgutils.cc	2.9 4/4/97
//

#include "gmisc.h"
#include "nfg.h"
#include "nfplayer.h"
#include "nfstrat.h"
#include "nfgciter.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

template <class T> Nfg<T> *CompressNfg(const Nfg<T> &nfg, const NFSupport &S)
{
  Nfg<T> *N = new Nfg<T>(S.NumStrats());
  
  N->GameForm().SetTitle(nfg.GameForm().GetTitle());

  for (int pl = 1; pl <= N->NumPlayers(); pl++)  {
    NFPlayer *player = N->Players()[pl];
    player->SetName(nfg.Players()[pl]->GetName());
    for (int st = 1; st <= N->NumStrats(pl); st++) 
      player->Strategies()[st]->name = S.Strategies(pl)[st]->name;
  }

  for (int outc = 1; outc <= nfg.NumOutcomes(); outc++)  {
    NFOutcome *outcome = 
      (outc > 1) ? N->GameForm().NewOutcome() : N->Outcomes()[1];

    outcome->SetName(nfg.Outcomes()[outc]->GetName());

    for (int pl = 1; pl <= N->NumPlayers(); pl++)
      N->SetPayoff(outcome, pl, nfg.Payoff(nfg.Outcomes()[outc], pl));
  }

  NfgContIter oiter(S);
  NFSupport newS(N->GameForm());
  NfgContIter niter(newS);
  
  do   {
    niter.SetOutcome(N->Outcomes()[oiter.GetOutcome()->GetNumber()]);

    oiter.NextContingency();
  }  while (niter.NextContingency());

  return N;
}

template <class T> bool IsConstSum(const Nfg<T> &nfg)
{
  int pl, index;
  T cvalue = (T) 0;
  T eps;
  gEpsilon(eps);

  for (pl = 1; pl <= nfg.gameform->NumPlayers(); pl++)
    cvalue += nfg.payoffs(1, pl);
  
  for (index = 2; index <= nfg.payoffs.NumRows(); index++)  {
    T thisvalue = (T) 0;

    for (pl = 1; pl <= nfg.gameform->NumPlayers(); pl++)
      thisvalue += nfg.payoffs(index, pl);

    if (thisvalue > cvalue + eps || thisvalue < cvalue - eps)
      return false;
  }
  
  return true;
}

template <class T> T MinPayoff(const Nfg<T> &nfg, int player)
{
  int index, p, p1, p2;
  T minpay;
  
  if (player) 
    p1 = p2 = player;
  else   {
    p1 = 1;
    p2 = nfg.gameform->NumPlayers();
  }

  minpay = nfg.payoffs(1, p1);
  for (index = 2; index <= nfg.payoffs.NumRows(); index++)  {
    for (p = p1; p <= p2; p++)
      if (nfg.payoffs(index, p) < minpay)
	minpay = nfg.payoffs(index, p);
  }
  return minpay;
}

template <class T> T MaxPayoff(const Nfg<T> &nfg, int player)
{
  int index, p, p1, p2;
  T maxpay;

  if (player) 
    p1 = p2 = player;
  else   {
    p1 = 1;
    p2 = nfg.gameform->NumPlayers();
  }

  maxpay = nfg.payoffs(1, p1);
  for (index = 2; index <= nfg.payoffs.NumRows(); index++)  {
    for (p = p1; p <= p2; p++)
      if (nfg.payoffs(index, p) > maxpay)
	maxpay = nfg.payoffs(index, p);
  }
  return maxpay;
}

template <class T> void RandomNfg(Nfg<T> &nfg)
{
  for (int pl = 1; pl <= nfg.NumPlayers(); pl++)
    for (int outc = 1; outc <= nfg.payoffs.NumRows(); outc++)
      nfg.payoffs(outc, pl) = (T) Uniform();
}  

#include "rational.h"

TEMPLATE void RandomNfg(Nfg<double> &nfg);
TEMPLATE void RandomNfg(Nfg<gRational> &nfg);

TEMPLATE Nfg<double> *CompressNfg(const Nfg<double> &, const NFSupport &);
TEMPLATE Nfg<gRational> *CompressNfg(const Nfg<gRational> &, const NFSupport &);

TEMPLATE bool IsConstSum(const Nfg<double> &nfg);
TEMPLATE double MinPayoff(const Nfg<double> &nfg, int player);
TEMPLATE double MaxPayoff(const Nfg<double> &nfg, int player);

TEMPLATE bool IsConstSum(const Nfg<gRational> &nfg);
TEMPLATE gRational MinPayoff(const Nfg<gRational> &nfg, int player);
TEMPLATE gRational MaxPayoff(const Nfg<gRational> &nfg, int player);
