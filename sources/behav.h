//#
//# FILE: behav.h -- Behavioral strategy profile classes
//#
//# @(#)behav.h	1.2 6/8/95
//#

#ifndef BEHAV_H
#define BEHAV_H

#include "basic.h"
#include "gstring.h"

class BaseExtForm;

class BaseBehavProfile   {
  protected:
    const BaseExtForm *E;
    bool truncated;

    BaseBehavProfile(const BaseExtForm &, bool trunc);
    BaseBehavProfile &operator=(const BaseBehavProfile &);

  public:
    virtual ~BaseBehavProfile();

    DataType Type(void) const;
    virtual bool IsPure(void) const = 0;
    virtual bool IsPure(int pl) const = 0;
    bool IsTruncated(void) const    { return truncated; }

    const gString &GetPlayerName(int p) const;
    const gString &GetInfosetName(int p, int iset) const;
    const gString &GetActionName(int p, int iset, int act) const;
};


#include "gdpvect.h"

template <class T> class ExtForm;
class Infoset;

template <class T> class BehavProfile
  : public BaseBehavProfile, public gDPVector<T>  {
  public:
    BehavProfile(const ExtForm<T> &, bool truncated = false);
    BehavProfile(const ExtForm<T> &, const gDPVector<T> &);
    ~BehavProfile();

    BehavProfile<T> &operator=(const BehavProfile<T> &);

    bool IsPure(void) const;
    bool IsPure(int pl) const;

    const T &GetValue(Infoset *s, int act) const;
};

template <class T> gOutput &operator<<(gOutput &f, const BehavProfile<T> &p);

#endif   // BEHAV_H
