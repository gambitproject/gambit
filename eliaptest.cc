//#
//# FILE: eliaptest.cc -- extensive form Lyapunov test program
//#
//# $Id$ 
//#

#include "efg.h"
#include "behav.h"
#include "eliap.h"

main()
{
  Efg<double> *E = 0;
  ReadEfgFile(gin, E);
  EFLiapParams<double> P;
//  P.trace=2;
//  P.tracefile=&gout;
  BehavProfile<double> B(*E);
  EFLiapModule<double> M( *E, P, B);
  M.Liap();
  M.GetSolutions().Dump(gout);
  gout << "\nNum Evals = " << M.NumEvals();
  gout << ", Time = " << M.Time() << "\n";
  return 1;
}

