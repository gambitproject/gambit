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
  Nfg<T> *N = new Nfg<T>(S.SupportDimensions());
  
  N->SetTitle(nfg.GetTitle());

  for (int pl = 1; pl <= N->NumPlayers(); pl++)  {
    NFPlayer *player = N->PlayerList()[pl];
    player->SetName(nfg.PlayerList()[pl]->GetName());
    for (int st = 1; st <= N->Dimensionality()[pl]; st++) 
      player->StrategyList()[st]->name = S.GetStrategy(pl, st)->name;
  }

  NfgContIter<T> oiter(&S);
  NFSupport newS(*N);
  NfgContIter<T> niter(&newS);
  
  do   {
    for (int pl = 1; pl <= nfg.NumPlayers(); pl++)
      niter.SetPayoff(pl, oiter.Payoff(pl));

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
  for (int i = 1; i <= nfg.NumPlayers(); i++)
    for (int j = 0; j < nfg.NumPayPerPlayer; j++)
      (nfg.payoffs)[i][j] = (T) Uniform();
}  

TEMPLATE void RandomNfg(Nfg<double> &nfg);
TEMPLATE void RandomNfg(Nfg<gRational> &nfg);

TEMPLATE Nfg<double> *CompressNfg(const Nfg<double> &, const NFSupport &);
TEMPLATE Nfg<gRational> *CompressNfg(const Nfg<gRational> &, const NFSupport &);
