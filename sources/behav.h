//#
//# FILE: behav.h -- Behavioral strategy profile classes
//#
//# $Id$
//#

#ifndef BEHAV_H
#define BEHAV_H

#include "gmisc.h"
#include "gstring.h"

class BaseEfg;

class BaseBehavProfile   {
  protected:
    const BaseEfg *E;
    bool truncated;

    BaseBehavProfile(const BaseEfg &, bool trunc);
    BaseBehavProfile(const BaseBehavProfile &);
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

template <class T> class Efg;
class Infoset;

template <class T> class BehavProfile
  : public BaseBehavProfile, public gDPVector<T>  {
  public:
    BehavProfile(const Efg<T> &, bool truncated = false);
    BehavProfile(const Efg<T> &, const gDPVector<T> &);
    BehavProfile(const BehavProfile<T> &);
    ~BehavProfile();

    BehavProfile<T> &operator=(const BehavProfile<T> &);

    bool IsPure(void) const;
    bool IsPure(int pl) const;

    T Payoff(int pl) const;

    Efg<T> *BelongsTo(void) const   { return (Efg<T> *) E; }

    const T &GetValue(Infoset *s, int act) const;
};
#ifndef __BORLANDC__
template <class T> gOutput &operator<<(gOutput &f, const BehavProfile<T> &p);
#endif
#endif   // BEHAV_H
