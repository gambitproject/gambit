//#
//# FILE: egobittest.cc -- extensive form Gobit test program
//#
//# $Id$ 
//#

#include "efg.h"
#include "behav.h"
#include "egobit.h"

main()
{
  Efg<double> *E = 0;
  ReadEfgFile(gin, E);
  EFGobitParams<double> P;
//  P.trace=2;
//  P.tracefile=&gout;
  gFileOutput pxi("pxi.out");
  P.pxifile = &pxi;
  BehavProfile<double> B(*E);
  EFGobitModule<double> M( *E, P, B);
  M.Gobit(1);
  M.GetSolutions().Dump(gout);
  gout << "\nNum Evals = " << M.NumEvals();
  gout << ", Time = " << M.Time() << "\n";
  return 1;
}

