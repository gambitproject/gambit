//
// FILE: nfgconv.cc -- Convert between types of normal forms
//
// $Id$
//


#include "nfg.h"
#include "nfplayer.h"
#include "nfstrat.h"
#include "contiter.h"
#include "rational.h"

Nfg<gRational> *ConvertNfg(const Nfg<double> &orig)
{
  Nfg<gRational> *N = new Nfg<gRational>(orig.Dimensionality());
  
  N->SetTitle(orig.GetTitle());

  for (int pl = 1; pl <= N->NumPlayers(); pl++)  {
    NFPlayer *p1 = orig.PlayerList()[pl];
    NFPlayer *p2 = N->PlayerList()[pl];

    p2->SetName(p1->GetName());
    
    for (int st = 1; st <= p2->NumStrats(); st++)
      p2->StrategyList()[st]->name = p1->StrategyList()[st]->name;
  }

  NFSupport S1(orig);
  NFSupport S2(*N);

  ContIter<double> C1(&S1);
  ContIter<gRational> C2(&S2);
  
  do   {
    for (int pl = 1; pl <= N->NumPlayers(); pl++)
      C2.SetPayoff(pl, gRational(C1.Payoff(pl)));

    C2.NextContingency();
  } while (C1.NextContingency());

  return N;
}



Nfg<double> *ConvertNfg(const Nfg<gRational> &orig)
{
  Nfg<double> *N = new Nfg<double>(orig.Dimensionality());
  
  N->SetTitle(orig.GetTitle());

  for (int pl = 1; pl <= N->NumPlayers(); pl++)  {
    NFPlayer *p1 = orig.PlayerList()[pl];
    NFPlayer *p2 = N->PlayerList()[pl];

    p2->SetName(p1->GetName());
    
    for (int st = 1; st <= p2->NumStrats(); st++)
      p2->StrategyList()[st]->name = p1->StrategyList()[st]->name;
  }

  NFSupport S1(orig);
  NFSupport S2(*N);

  ContIter<gRational> C1(&S1);
  ContIter<double> C2(&S2);
  
  do   {
    for (int pl = 1; pl <= N->NumPlayers(); pl++)
      C2.SetPayoff(pl, (double) C1.Payoff(pl));

    C2.NextContingency();
  } while (C1.NextContingency());

  return N;
}



