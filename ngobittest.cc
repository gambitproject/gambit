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
//  P.tracefile=&gout;
//  P.nequilib=1;
  MixedProfile<double> S(*N);
  NFGobitModule<double> M( *N, P, S);
  M.Gobit(1);
  M.GetSolutions().Dump(gout);
  gout << "\nNum Evals = " << M.NumEvals();
  gout << ", Time = " << M.Time() << "\n";
  return 1;
}

