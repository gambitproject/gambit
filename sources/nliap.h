//#
//# FILE: nliap.h -- Interface to normal form Liapunov solution module
//#
//# $Id$
//#

#ifndef NLIAP_H
#define NLIAP_H

#include "normal.h"
#include "liap.h"
#include "glist.h"

template <class T> class NFLiapParams : public LiapParams<T>  {
  public:
    NFLiapParams(void);
};

template <class T> class NFLiapModule : public LiapModule<T>  {
  private:
    const NormalForm<T> &N;
    gList<gPVector<T> > solutions;

    LiapFunc<T> *CreateFunc(void);
    void AddSolution(const LiapFunc<T> *const);

  public:
    NFLiapModule(const NormalForm<T> &N, NFLiapParams<T> &p); 
    NFLiapModule(const NormalForm<T> &N, NFLiapParams<T> &p,gPVector<T> &s); 
    virtual ~NFLiapModule();
    const gList<gPVector<T> > &GetSolutions(void) const;
};


#endif    // NLIAP_H



