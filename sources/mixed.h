//#
//# FILE: mixed.h -- Mixed strategy profile classes
//#
//# $Id$
//#

#ifndef MIXED_H
#define MIXED_H

#include "basic.h"

class BaseNormalForm;

class BaseMixedProfile     {
  protected:
    BaseNormalForm &N;

  public:
    BaseMixedProfile(BaseNormalForm &);
    virtual ~BaseMixedProfile();

    BaseMixedProfile &operator=(const BaseMixedProfile &);

    DataType Type(void) const;
    virtual bool IsPure(void) const = 0;
    virtual bool IsPure(int pl) const = 0;
};


#include "gpvector.h"

template <class T> class NormalForm;

template <class T> class MixedProfile 
  : public BaseMixedProfile, public gPVector<T>  {
  public:
    MixedProfile(NormalForm<T> &);
    MixedProfile(NormalForm<T> &, const gPVector<T> &);
    ~MixedProfile();

    MixedProfile<T> &operator=(const MixedProfile<T> &);

    bool IsPure(void) const;
    bool IsPure(int pl) const;
};

#endif    // MIXED_H
