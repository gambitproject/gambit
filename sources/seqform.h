//#
//# FILE: seqform.h -- Interface to Sequence Form solution module
//#
//# $Id$ 
//#

#ifndef SEQFORM_H
#define SEQFORM_H

#include "extform.h"
#include "rational.h"
#include "glist.h"
#include "gdpvect.h"
#include "sftab.h"

class SeqFormParams     {
  public:
    int  plev, nequilib, maxdepth;
    gOutput *output;
    
    SeqFormParams(void);
};

//
// The general-purpose interface to the solver routines
//

template <class T> class SeqFormModule  {
private:
  const ExtForm<T> &EF;
  const SeqFormParams &params;
  int npivots;
  double time;
  BFS_List List;
  gList< BehavProfile<T> > solutions;

public:
  SeqFormModule(const ExtForm<T> &E, const SeqFormParams &p);
  virtual ~SeqFormModule();
  
  int Lemke(int dup = 0);
  
  int Add_BFS(const SFTableau<T> &B);
  int NumPivots(void) const;
  double Time(void) const;
  
  const gList<BehavProfile<T> > &GetSolutions() const;
};

//
// Convenience functions for "one-shot" evaluations
//

/*
template <class T> int SeqForm(const ExtForm<T> &N, const SeqFormParams &p,
			     gList<gDPVector<T> > &solutions,
			     int &npivots, double &time);
*/				   

#endif    // SEQFORM_H



