//
// FILE nfgutils.cc -- useful utilities for the normal form
//
// $Id$
//

#include "gmisc.h"
#include "nfg.h"
#include "nfplayer.h"
#include "nfstrat.h"
#include "nfgciter.h"


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

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

#include "rational.h"
// prototype in nfg.h

template <class T> void RandomNfg(Nfg<T> &nfg)
{
  for (int pl = 1; pl <= nfg.NumPlayers(); pl++)
    for (int outc = 1; outc <= nfg.payoffs.NumRows(); outc++)
      nfg.payoffs(outc, pl) = (T) Uniform();
}  

TEMPLATE void RandomNfg(Nfg<double> &nfg);
TEMPLATE void RandomNfg(Nfg<gRational> &nfg);

TEMPLATE Nfg<double> *CompressNfg(const Nfg<double> &, const NFSupport &);
TEMPLATE Nfg<gRational> *CompressNfg(const Nfg<gRational> &, const NFSupport &);
