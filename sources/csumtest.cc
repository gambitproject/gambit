//#
//# FILE: csumtest.cc -- Constant Sum  test program
//#
//# $Id$
//#

#include "nfg.h"
#include "csum.h"

main()
{
  Nfg<double> *N = 0;
  ReadNfgFile(gin, N);
  ZSumParams P;
//  P.trace=2;
//  P.tracefile=&gout;
//  P.nequilib=1;
  NFSupport S(*N);
  ZSumModule<double> M(*N, P,S);
  M.ZSum();
  gList<MixedProfile<double> > list;
  M.GetSolutions(list);
  list.Dump(gout);
  gout << "\nNum Pivots = " << M.NumPivots();
  gout << ", Time = " << M.Time() << "\n";
  return 1;
}

