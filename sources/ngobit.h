//#
//# FILE: ngobit.h -- Interface to normal form Gobit solution module
//#
//# $Id$
//#

#ifndef NGOBIT_H
#define NGOBIT_H

#include "nfg.h"
#include "gobit.h"
#include "glist.h"
#include "mixed.h"

template <class T> class NFGobitParams : public GobitParams<T>   {
  public:
    NFGobitParams(gStatus &status_=gstatus);
    NFGobitParams(gOutput &out, gOutput &pxi,
		  gStatus &status_=gstatus);
};


template <class T> class NFGobitModule : public GobitModule<T>   {
  protected:
    const Nfg<T> &N;
    gList<MixedProfile<T> > solutions;

    GobitFunc<T> *CreateFunc(void);
    void AddSolution(const GobitFunc<T> *const);
  public:
    NFGobitModule(const Nfg<T> &NF, NFGobitParams<T> &p);
    NFGobitModule(const Nfg<T> &NF, NFGobitParams<T> &p,
		  MixedProfile<T> &s);
    virtual ~NFGobitModule();
    const gList<MixedProfile<T> > &GetSolutions(void) const;
};



#endif    // NGOBIT_H



