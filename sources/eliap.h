//#
//# FILE: eliap.h -- Interface to extensive form Liapunov solution module
//#
//# $Id$
//#

#ifndef ELIAP_H
#define ELIAP_H

#include "extform.h"
#include "liap.h"
#include "glist.h"
#include "rational.h"
//#include "gdpvect.h"

template <class T> class EFLiapParams : public LiapParams<T>  {
public:
  EFLiapParams(void);
};


template <class T> class EFLiapModule : public LiapModule<T>  {
  private:
    const ExtForm<T> &E;
    gList<gDPVector<T> > solutions;

    LiapFunc<T> *CreateFunc(void);
    void AddSolution(const LiapFunc<T> *const);

  public:
    EFLiapModule(const ExtForm<T> &E, EFLiapParams<T> &p); 
    EFLiapModule(const ExtForm<T> &E, EFLiapParams<T> &p, gDPVector<T> &s); 
    virtual ~EFLiapModule();
    const gList<gDPVector<T> > &GetSolutions(void) const;
};


#endif    // ELIAP_H



