//#
//# FILE: egobit.h -- Interface to Gobit solution module
//#
//# $Id$
//#

#ifndef EGOBIT_H
#define EGOBIT_H

#include "efg.h"
#include "gobit.h"

template <class T> class EFGobitParams : public GobitParams<T>  {
  public:
    EFGobitParams(gStatus &status_=gstatus);
    EFGobitParams(gOutput &out, gOutput &pxi, gStatus &status_=gstatus);
};


template <class T> class EFGobitModule : public GobitModule<T>  {
  protected:
    const Efg<T> &E;

    GobitFunc<T> *CreateFunc(void);

  public:
    EFGobitModule(const Efg<T> &EF, EFGobitParams<T> &p);
    EFGobitModule(const Efg<T> &EF, EFGobitParams<T> &p, gDPVector<T> &s);
    virtual ~EFGobitModule();
};

#endif    // EGOBIT_H






