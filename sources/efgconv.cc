//
// FILE: efgconv.cc -- Convert between types of efgs
//
// $Id$
//


#include "efg.h"

Efg<gRational> *ConvertEfg(const Efg<double> &orig)
{
  Efg<gRational> *E = new Efg<gRational>((const BaseEfg &) orig);

  for (int outc = 1; outc <= E->NumOutcomes(); outc++)   {
    OutcomeVector<double> *c1 = (OutcomeVector<double> *) orig.OutcomeList()[outc];
    OutcomeVector<gRational> *c2 = (OutcomeVector<gRational> *) E->OutcomeList()[outc];

    for (int pl = 1; pl <= E->NumPlayers(); pl++)  
      (*c2)[pl] = gRational((*c1)[pl]);
  }

  EFPlayer *ochance = orig.GetChance();
  EFPlayer *echance = E->GetChance();
  
  for (int iset = 1; iset <= echance->NumInfosets(); iset++)  {
    ChanceInfoset<double> *s1 = (ChanceInfoset<double> *) ochance->InfosetList()[iset];
    ChanceInfoset<gRational> *s2 = (ChanceInfoset<gRational> *) echance->InfosetList()[iset];

    for (int act = 1; act <= s2->NumActions(); act++)
      s2->SetActionProb(act, gRational(s1->GetActionProb(act)));
  }

  return E;
}


Efg<double> *ConvertEfg(const Efg<gRational> &orig)
{
  Efg<double> *E = new Efg<double>((const BaseEfg &) orig);
  
  for (int outc = 1; outc <= E->NumOutcomes(); outc++)   {
    OutcomeVector<gRational> *c1 = (OutcomeVector<gRational> *) orig.OutcomeList()[outc];
    OutcomeVector<double> *c2 = (OutcomeVector<double> *) E->OutcomeList()[outc];

    for (int pl = 1; pl <= E->NumPlayers(); pl++)  
      (*c2)[pl] = (double) (*c1)[pl];
  }

  EFPlayer *ochance = orig.GetChance();
  EFPlayer *echance = E->GetChance();
  
  for (int iset = 1; iset <= echance->NumInfosets(); iset++)  {
    ChanceInfoset<gRational> *s1 = (ChanceInfoset<gRational> *) ochance->InfosetList()[iset];
    ChanceInfoset<double> *s2 = (ChanceInfoset<double> *) echance->InfosetList()[iset];

    for (int act = 1; act <= s2->NumActions(); act++)
      s2->SetActionProb(act, (double) s1->GetActionProb(act));
  }

  return E;
}

