//#
//# FILE: ngobit.h -- Interface to normal form Gobit solution module
//#
//# $Id$
//#

#ifndef NGOBIT_H
#define NGOBIT_H

#include "normal.h"
#include "gobit.h"

template <class T> class NFGobitParams : public GobitParams<T>   {
  public:
    NFGobitParams(gStatus &status_=gstatus);
    NFGobitParams(gOutput &out, gOutput &err, gOutput &pxi,
		  gStatus &status_=gstatus);
};


template <class T> class NFGobitModule : public GobitModule<T>   {
  protected:
    const NormalForm<T> &N;

    GobitFunc<T> *CreateFunc(void);

  public:
    NFGobitModule(const NormalForm<T> &NF, NFGobitParams<T> &p);
    NFGobitModule(const NormalForm<T> &NF, NFGobitParams<T> &p,gPVector<T> &s);
    virtual ~NFGobitModule();
};



#endif    // NGOBIT_H



