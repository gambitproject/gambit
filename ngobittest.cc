//#
//# FILE: ngobittest.cc -- normal form Gobit test program
//#
//# $Id$ 
//#

#include "nfg.h"
#include "mixed.h"
#include "ngobit.h"

main()
{
  Nfg<double> *N = 0;
  ReadNfgFile(gin, N);
  NFGobitParams<double> P;
  P.trace=2;
  P.tracefile=&gout;
//  P.nequilib=1;
  NFSupport S(*N);
//  NFStrategySet *S1(S.GetNFStrategySet(1));
//  S1->RemoveStrategy(2);
//  S.SetNFStrategySet(1,S1);
  MixedProfile<double> MP(*N,S);
  NFGobitModule<double> M( *N, P, MP);
  M.Gobit(1);
  M.GetSolutions().Dump(gout);
  gout << "\nNum Evals = " << M.NumEvals();
  gout << ", Time = " << M.Time() << "\n";
  return 1;
}

