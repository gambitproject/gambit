//
// FILE: nfgconv.cc -- Convert between types of normal forms
//
// $Id$
//


#include "nfg.h"
#include "nfplayer.h"
#include "nfstrat.h"
#include "nfgciter.h"
#include "rational.h"

Nfg<gRational> *ConvertNfg(const Nfg<double> &orig)
{
  Nfg<gRational> *N = new Nfg<gRational>(orig.NumStrats());
  
  N->SetTitle(orig.GetTitle());

  for (int pl = 1; pl <= N->NumPlayers(); pl++)  {
    NFPlayer *p1 = orig.Players()[pl];
    NFPlayer *p2 = N->Players()[pl];

    p2->SetName(p1->GetName());
    
    for (int st = 1; st <= p2->NumStrats(); st++)
      p2->Strategies()[st]->name = p1->Strategies()[st]->name;
  }

  NFSupport S1(orig);
  NFSupport S2(*N);

  NfgContIter<double> C1(S1);
  NfgContIter<gRational> C2(S2);
  
  do   {
    for (int pl = 1; pl <= N->NumPlayers(); pl++)
      (*C2.Outcome())[pl] = gRational((*C1.Outcome())[pl]);

    C2.NextContingency();
  } while (C1.NextContingency());

  return N;
}



Nfg<double> *ConvertNfg(const Nfg<gRational> &orig)
{
  Nfg<double> *N = new Nfg<double>(orig.NumStrats());
  
  N->SetTitle(orig.GetTitle());

  for (int pl = 1; pl <= N->NumPlayers(); pl++)  {
    NFPlayer *p1 = orig.Players()[pl];
    NFPlayer *p2 = N->Players()[pl];

    p2->SetName(p1->GetName());
    
    for (int st = 1; st <= p2->NumStrats(); st++)
      p2->Strategies()[st]->name = p1->Strategies()[st]->name;
  }

  NFSupport S1(orig);
  NFSupport S2(*N);

  NfgContIter<gRational> C1(S1);
  NfgContIter<double> C2(S2);
  
  do   {
    for (int pl = 1; pl <= N->NumPlayers(); pl++)
      (*C2.Outcome())[pl] = (double) (*C1.Outcome())[pl];

    C2.NextContingency();
  } while (C1.NextContingency());

  return N;
}



