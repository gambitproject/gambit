//#
//# FILE: seqtest.cc -- Sequence form test program
//#
//# $Id$
//#

#include "efg.h"
#include "behav.h"
#include "seqform.h"
 
main()
{
  Efg<double> *E = 0;
  ReadEfgFile(gin, E);
  SeqFormParams P;
//  P.trace=2;
//  P.tracefile=&gout;
//  P.nequilib=1;
  SeqFormModule<double> M(*E, P);
  M.Lemke();
  M.GetSolutions().Dump(gout);
  gout << "\nNum Pivots = " << M.NumPivots();
  gout << ", Time = " << M.Time() << "\n";
  return 1;
}

