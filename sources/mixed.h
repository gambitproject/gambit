//#
//# FILE: mixed.h -- Mixed strategy profile classes
//#
//# $Id$
//#

#ifndef MIXED_H
#define MIXED_H

#include "gmisc.h"

class BaseNormalForm;

class BaseMixedProfile     {
  protected:
    const BaseNormalForm *N;
    bool truncated;

    BaseMixedProfile(const BaseNormalForm &, bool trunc);
    BaseMixedProfile(const BaseMixedProfile &);
    BaseMixedProfile &operator=(const BaseMixedProfile &);

  public:
    virtual ~BaseMixedProfile();

    DataType Type(void) const;
    virtual bool IsPure(void) const = 0;
    virtual bool IsPure(int pl) const = 0;
    bool IsTruncated(void) const    { return truncated; }
};


#include "gpvector.h"

template <class T> class NormalForm;

template <class T> class MixedProfile 
  : public BaseMixedProfile, public gPVector<T>  {
  public:
    MixedProfile(const NormalForm<T> &, bool truncated = false);
    MixedProfile(const NormalForm<T> &, const gPVector<T> &);
    MixedProfile(const MixedProfile<T> &);
    ~MixedProfile();

    MixedProfile<T> &operator=(const MixedProfile<T> &);

    NormalForm<T> *BelongsTo(void) const  { return (NormalForm<T> *) N; }

    bool IsPure(void) const;
    bool IsPure(int pl) const;
};

template <class T> gOutput &operator<<(gOutput &f, const MixedProfile<T> &);

#endif    // MIXED_H
