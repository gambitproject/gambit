//#
//# FILE: eliap.h -- Interface to extensive form Liapunov solution module
//#
//#

#ifndef ELIAP_H
#define ELIAP_H

#include "extform.h"
#include "liap.h"
#include "glist.h"

template <class T> class EFLiapParams : public LiapParams<T>  {
  public:
    EFLiapParams(void);
};

template <class T> class EFLiapModule : public LiapModule<T>  {
  private:
    const ExtForm<T> &ef;
    gList<gPVector<T> > solutions;

    LiapFunc<T> *CreateFunc(void);
    void AddSolution(const LiapFunc<T> *const);

  public:
    EFLiapModule(const ExtForm<T> &E, EFLiapParams<T> &p); 

    const gList<gPVector<T> > &GetSolutions(void) const;
};


#endif    // ELIAP_H



