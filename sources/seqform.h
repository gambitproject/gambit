//#
//# FILE: seqform.h -- Interface to Sequence Form solution module
//#
//# $Id$ 
//#

#ifndef SEQFORM_H
#define SEQFORM_H

#include "efg.h"
#include "rational.h"
#include "glist.h"
#include "gdpvect.h"
#include "gmatrix.h"
#include "lemketab.h"

class SeqFormParams     {
  public:
    int  plev, stopAfter, maxdepth;
    gOutput *output;
    
    SeqFormParams(void);
};

//
// The general-purpose interface to the solver routines
//

template <class T> class SeqFormModule  {
private:
  const Efg<T> &EF;
  const EFSupport &support;
  const SeqFormParams &params;
  gMatrix<T> *A;
  gVector<T> *b;
  LTableau<T> *tab;
  int ns1,ns2,ni1,ni2;
  T maxpay;
  long npivots;
  double time;
  BFS_List List;
  gList< BehavProfile<T> > solutions;

public:
  SeqFormModule(const Efg<T> &E, const SeqFormParams &p, 
		const EFSupport &);
  virtual ~SeqFormModule();
  
  int Lemke(int dup = 0);
  
  int Add_BFS(const LTableau<T> &tab);
  long NumPivots(void) const;
  double Time(void) const;
  
  void FillTableau(const Node *n,T prob,int s1,int s2,int i1,int i2);
  int LCPPath(); // follow a path of ACBFS's from one CBFS to another
  void GetProfile(gDPVector<T> &, const gVector<T> &, 
		  const Node *n, int,int);
  const gList<BehavProfile<T> > &GetSolutions() const;
  int NumSequences(int j);
  int NumInfosets(int j);
};

//
// Convenience functions for "one-shot" evaluations
//

/*
template <class T> int SeqForm(const Efg<T> &N, const SeqFormParams &p,
			     gList<gDPVector<T> > &solutions,
			     long &npivots, double &time);
*/				   

#endif    // SEQFORM_H



