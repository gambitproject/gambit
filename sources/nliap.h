//#
//# FILE: nliap.h -- Interface to normal form Liapunov solution module
//#
//# $Id$
//#

#ifndef NLIAP_H
#define NLIAP_H

#include "nfg.h"
#include "liap.h"
#include "glist.h"
#include "mixed.h"

class NFLiapParams : public LiapParams  {
  public:
    NFLiapParams(gStatus &status_ = gstatus);
};

template <class T> class NFLiapModule : public LiapModule<T>  {
  private:
    const Nfg<T> &N;
    MixedProfile<T> S;
    gList<MixedProfile<T> > solutions;

    LiapFunc<T> *CreateFunc(void);
    void AddSolution(const LiapFunc<T> *const);

  public:
    NFLiapModule(const Nfg<T> &N, NFLiapParams &p, MixedProfile<T> &s); 
    virtual ~NFLiapModule();
    const gList<MixedProfile<T> > &GetSolutions(void) const;
};


#endif    // NLIAP_H



