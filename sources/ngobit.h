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
    NFGobitParams(void);
};


template <class T> class NFGobitModule : public GobitModule<T>   {
  protected:
    const NormalForm<T> &N;

    GobitFunc<T> *CreateFunc(void);

  public:
    NFGobitModule(const NormalForm<T> &NF, NFGobitParams<T> &p);
    virtual ~NFGobitModule();
};



#endif    // NGOBIT_H



