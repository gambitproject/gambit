//#
//# FILE: enum.h -- Interface to Enum solution module
//#
//# $Id$
//#

#ifndef ENUM_H
#define ENUM_H

#include "nfg.h"
#include "glist.h"
#include "gstatus.h"
#include "lhtab.h"
#include "mixed.h"

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
  BFS_List List;
  const Nfg<T> &NF;
  EnumParams params;
  int rows,cols,level;
  long count,npivots;
  double time;
  gList<MixedProfile<T> > solutions;
  
//  void AddSolution(const gPVector<T> &s);
  
public:
  EnumModule(const Nfg<T> &N, const EnumParams &p); 
  
  int Enum(void);
  void SubSolve(int pr, int pcl, LTableau<T> &B1, gBlock<int> &targ1);
  
  long NumPivots(void) const;
  double Time(void) const;
  
  EnumParams &Parameters(void);

  gList<MixedProfile<T> > &GetSolutions(void);
};

//
// Convenience functions for "one-shot" evaluations
//
template <class T> int Enum(const Nfg<T> &N, const EnumParams &p,
			    gList<MixedProfile<T> > &solutions,
			    long &npivots, double &time);

#endif    // ENUM_H




