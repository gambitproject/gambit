//#
//# FILE: ngobit.h -- Interface to normal form Gobit solution module
//#
//# $Id$
//#

#ifndef NGOBIT_H
#define NGOBIT_H

#include "normal.h"
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
    const NormalForm<T> &N;
    gList<MixedProfile<T> > solutions;

    GobitFunc<T> *CreateFunc(void);
    void AddSolution(const GobitFunc<T> *const);
  public:
    NFGobitModule(const NormalForm<T> &NF, NFGobitParams<T> &p);
    NFGobitModule(const NormalForm<T> &NF, NFGobitParams<T> &p,gPVector<T> &s);
    virtual ~NFGobitModule();
    const gList<MixedProfile<T> > &GetSolutions(void) const;
};



#endif    // NGOBIT_H



