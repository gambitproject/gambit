//#
//# FILE: lemketest.cc -- Lemke test program
//#
//# $Id$
//#

#include "normiter.h"
#include "normal.h"
#include "lemke.h"

main()
{
  NormalForm<double> N(gin);
  LemkeParams LP;
  LP.plev=2;
//  LP.nequilib=1;
  LemkeModule<double> LM((NormalForm<double> &) N, LP);
  LM.Lemke();
  LM.GetSolutions().Dump(gout);
  gout << "\nNum Pivots = " << LM.NumPivots();
  gout << ", Time = " << LM.Time() << "\n";
  return 1;
}

