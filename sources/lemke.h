//#
//# FILE: lemke.h -- Interface to Lemke solution module
//#
//# $Id$
//#

#ifndef LEMKE_H
#define LEMKE_H

#include "normal.h"
#include "mixed.h"
#include "glist.h"

class LemkeParams     {
  public:
    int trace, stopAfter, maxDepth;
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
    long npivots;
    double time;
    gList<MixedProfile<T> > solutions;

  public:
    LemkeModule(const NormalForm<T> &N, const LemkeParams &p);
    virtual ~LemkeModule();

    int Lemke(void);
    
    long NumPivots(void) const;
    double Time(void) const;

    const gList<MixedProfile<T> > &GetSolutions(void) const;
};

//
// Convenience functions for "one-shot" evaluations
//
template <class T> int Lemke(const NormalForm<T> &N, const LemkeParams &p,
			     gList<MixedProfile<T> > &solutions,
			     long &npivots, double &time);
				   
#endif    // LEMKE_H



