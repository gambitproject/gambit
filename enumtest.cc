//#
//# FILE: enumtest.cc -- Enum test program
//#
//# $Id$ 
//#

#include "nfg.h"
#include "mixed.h"
#include "enum.h"

main()
{
  Nfg<double> *N = 0;
  ReadNfgFile(gin, N);
  EnumParams EP;
//  EP.stopAfter=8;
//   EP.trace=3;
  EP.tracefile=&gout;
  NFSupport S(*N);
  EnumModule<double> LM( *N, EP,S);
  LM.Enum();
  LM.GetSolutions().Dump(gout);
  gout << "\nNum Pivots =" <<  LM.NumPivots();
  gout << ", Time =" <<  LM.Time() << "\n";
  return 1;
}

