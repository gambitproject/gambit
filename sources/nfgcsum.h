//
// FILE: nfgcsum.h -- Interface to Constant Sum Game Solution Solver
//
// $Id$
//

#ifndef NFGCSUM_H
#define NFGCSUM_H

#include "nfg.h"
#include "rational.h"
#include "glist.h"
#include "gstatus.h"
#include "tableau.h"
#include "lpsolve.h"
#include "mixed.h"
#include "mixedsol.h"

class ZSumParams     {
  public:
    int trace, stopAfter;
    gOutput *tracefile;
    gStatus &status;
    
    ZSumParams(gStatus &status_ = gstatus);
};

//
// The general-purpose interface to the solver routines
//

template <class T> class ZSumModule  {
private:
  const Nfg &NF;
  const ZSumParams &params;
  const NFSupport &support;
  gMatrix<T> *A;
  gVector<T> *b, *c;
  T minpay;
  long npivots;
  double time;
  gList<BFS<T> > List;

public:
  ZSumModule(const Nfg &N, const ZSumParams &p, const NFSupport &);
  virtual ~ZSumModule();

  bool IsConstSum();
  
  int ZSum(int dup = 0);

  void Make_Abc();
  
  int Add_BFS(const LPSolve<T> &B);
  long NumPivots(void) const;
  double Time(void) const;
  
  void GetSolutions(gList<MixedSolution > &) const;
};

#endif    // NFGCSUM_H



