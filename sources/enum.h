//#
//# FILE: enum.h -- Interface to Nash Enum solution module
//#
//# $Id$
//#

//
// Enum implements the procedure described in 
// Mangasarian, O. L., "Equilibrium points of bimatrix games", 
// SIAM 12 (1964): 778-780 for finding all extreme points of 
// the Nash equilibrium components of a two person game. 
// 

#ifndef ENUM_H
#define ENUM_H

#include "nfg.h"
#include "glist.h"
#include "gstatus.h"
#include "lhtab.h"
#include "mixed.h"
#include "mixedsol.h"
#include "vertenum.h"

class EnumParams     {
public:
  int trace, stopAfter;
  gOutput *tracefile;
  gStatus &status;

  EnumParams(gStatus &status_=gstatus);
};

//
// The general-purpose interface to the Module routines
//

template <class T> class EnumModule  {
private:
  T eps;
  const Nfg<T> &NF;
  const NFSupport &support;
  EnumParams params;
  int rows,cols,level;
  long count,npivots;
  double time;
  gList<MixedSolution<T> > solutions;

  bool EqZero(T x) const;
//  void AddSolution(const gPVector<T> &s);
  
public:
  EnumModule(const Nfg<T> &N, const EnumParams &p, const NFSupport &s); 
  
  int Enum(void);
  
  long NumPivots(void) const;
  double Time(void) const;
  
  EnumParams &Parameters(void);

  const gList<MixedSolution<T> > &GetSolutions(void) const;
};


#endif    // ENUM_H




