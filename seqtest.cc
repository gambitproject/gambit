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
//  gout << "\nNumPlayers = " << E->NumPlayers();
  EFSupport S(*E);
  SeqFormModule<double> M(*E, P, S);
  M.Lemke();
  M.GetSolutions().Dump(gout);
  gout << "\nNum Pivots = " << M.NumPivots();
  gout << ", Time = " << M.Time() << "\n";
  return 1;
}

