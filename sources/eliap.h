//#
//# FILE: eliap.h -- Interface to extensive form Liapunov solution module
//#
//# $Id$
//#

#ifndef ELIAP_H
#define ELIAP_H

#include "efg.h"
#include "liap.h"
#include "glist.h"
#include "rational.h"
#include "gdpvect.h"

template <class T> class EFLiapParams : public LiapParams<T>  {
public:
  EFLiapParams(void);
};


template <class T> class EFLiapModule : public LiapModule<T>  {
  private:
    const Efg<T> &E;
    gList<BehavProfile<T> > solutions;
    BehavProfile<T> start;

    LiapFunc<T> *CreateFunc(void);
    void AddSolution(const LiapFunc<T> *const);

  public:
    EFLiapModule(const Efg<T> &E, EFLiapParams<T> &p, BehavProfile<T> &s); 
    virtual ~EFLiapModule();
    const gList<BehavProfile<T> > &GetSolutions(void) const;
};


#endif    // ELIAP_H



