//#
//# FILE: enumtest.cc -- Enum test program
//#
//# $Id$
//#

#include "normiter.h"
#include "normal.h"
#include "enum.h"

main()
{
  NormalForm<double> N(gin);
  EnumParams EP;
//  EP.stopAfter=8;
//  EP.trace=2;
  EP.tracefile=&gout;
  EnumModule<double> LM((NormalForm<double> &) N, EP);
  LM.Enum();
  LM.GetSolutions().Dump(gout);
  gout << "\nNum Pivots =" <<  LM.NumPivots();
  gout << ", Time =" <<  LM.Time() << "\n";
  return 1;
}

