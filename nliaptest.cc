//#
//# FILE: nliaptest.cc -- normal form Lypaunov test program
//#
//# $Id$ 
//#

#include "nfg.h"
#include "mixed.h"
#include "nliap.h"

main()
{
  Nfg<double> *N = 0;
  ReadNfgFile(gin, N);
  NFLiapParams<double> P;
  P.trace=2;
//  P.tracefile=&gout;
//  P.nequilib=1;
  MixedProfile<double> S(*N);
  NFLiapModule<double> M( *N, P, S);
  M.Liap();
  M.GetSolutions().Dump(gout);
  gout << "\nNum Evals = " << M.NumEvals();
  gout << ", Time = " << M.Time() << "\n";
  return 1;
}

