//#
//# FILE: lemke.h -- Interface to Lemke solution module
//#
//# $Id$
//#

#ifndef LEMKE_H
#define LEMKE_H

#include "normal.h"
#include "rational.h"
#include "glist.h"

class LemkeParams     {
  public:
    int dup_strat, plev, nequilib, maxdepth;
    gString outfile, errfile;
    
    LemkeParams(void);
};

//
// The general-purpose interface to the solver routines
//
template <class T> class LemkeSolver  {
  private:
    const NormalForm<T> &nf;
    LemkeParams params;
    int npivots;
    gRational time;
    gList<gPVector<T> > solutions;

  public:
    LemkeSolver(const NormalForm<T> &N, const LemkeParams &p); 

    int Lemke(void);
    
    int NumPivots(void) const;
    gRational Time(void) const;

    LemkeParams &Parameters(void);

    const gList<gPVector<T> > &GetSolutions(void) const;
};

//
// Convenience functions for "one-shot" evaluations
//
template <class T> int Lemke(const NormalForm<T> &N, const LemkeParams &p,
			     gList<gPVector<T> > &solutions,
			     int &npivots, gRational &time);
				   
#endif    // LEMKE_H



