//#
//# FILE: lemke.h -- Interface to Lemke solution module
//#
//# $Id$
//#

#ifndef LEMKE_H
#define LEMKE_H

#include "nfg.h"
#include "mixed.h"
#include "glist.h"
#include "gstatus.h"

class LemkeParams     {
  public:
    int trace, stopAfter, maxDepth;
    gOutput *output;
    gStatus &status;
    
    LemkeParams(gStatus &status_=gstatus);
};

//
// The general-purpose interface to the solver routines
//
template <class T> class LemkeModule  {
  private:
    const Nfg<T> &nf;
    const NFSupport &support;
    const LemkeParams &params;
    long npivots;
    double time;
    gList<MixedProfile<T> > solutions;

  public:
    LemkeModule(const Nfg<T> &N, const LemkeParams &p, const NFSupport &);
    virtual ~LemkeModule();

    int Lemke(void);
    
    long NumPivots(void) const;
    double Time(void) const;

    const gList<MixedProfile<T> > &GetSolutions(void) const;
};

//
// Convenience functions for "one-shot" evaluations
//
template <class T> int Lemke(const Nfg<T> &N, const LemkeParams &p,
			     gList<MixedProfile<T> > &solutions,
			     long &npivots, double &time);
				   
#endif    // LEMKE_H



