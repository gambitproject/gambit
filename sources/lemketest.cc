//#
//# FILE: lemketest.cc -- Lemke test program
//#
//# $Id$
//#

#include "nfg.h"
#include "lemke.h"

main()
{
  Nfg<double> *N = 0;
  ReadNfgFile(gin, N);
  LemkeParams P;
//  P.trace=2;
//  P.tracefile=&gout;
//  P.nequilib=1;
  NFSupport S(*N);
  LemkeModule<double> M(*N, P,S);
  M.Lemke();
  M.GetSolutions().Dump(gout);
  gout << "\nNum Pivots = " << M.NumPivots();
  gout << ", Time = " << M.Time() << "\n";
  return 1;
}

