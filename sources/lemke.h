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
    gOutput *output;
    
    LemkeParams(void);
};

//
// The general-purpose interface to the solver routines
//
template <class T> class LemkeModule  {
  private:
    const NormalForm<T> &nf;
    const LemkeParams &params;
    int npivots;
    double time;
    gList<gPVector<T> > solutions;

  public:
    LemkeModule(const NormalForm<T> &N, const LemkeParams &p);
    virtual ~LemkeModule();

    int Lemke(void);
    
    int NumPivots(void) const;
    double Time(void) const;

    const gList<gPVector<T> > &GetSolutions(void) const;
};

//
// Convenience functions for "one-shot" evaluations
//
template <class T> int Lemke(const NormalForm<T> &N, const LemkeParams &p,
			     gList<gPVector<T> > &solutions,
			     int &npivots, gRational &time);
				   
#endif    // LEMKE_H



