//
// FILE: efgciter.h -- Extensive form contingency iterator class
//
// $Id$
//

#ifndef EFGCITER_H
#define EFGCITER_H

#include "gblock.h"

#include "efg.h"

template <class T> class EfgIter;

//
// N.B.: Currently, the contingency iterator only allows one information
//       set to be "frozen".  This functionality will be expanded in the
//       future, but all current applications require only one to be
//       frozen at this time.
//
//       Iterating across all contingencies can be achieved by freezing
//       player number 0 (this is the default state on initialization)
//
template <class T> class EfgContIter    {
  friend class EfgIter<T>;
  private:
    int _frozen_pl, _frozen_iset;
    const Efg *_efg;
    EFSupport _support;
    PureBehavProfile<T> _profile;
    gPVector<int> _current;
    mutable gVector<T> _payoff;

  public:
    EfgContIter(const EFSupport &);
    ~EfgContIter();
  
    void First(void);
  
    void Freeze(int pl, int iset);
  
  // These next two only work on frozen infosets
    void Set(int pl, int iset, int act);
    int Next(int pl, int iset);
  
    const PureBehavProfile<T> &GetProfile(void) const   { return _profile; }

    int NextContingency(void);
  
    T Payoff(int pl) const;
  
    void Dump(gOutput &) const;
};

#endif   // NFGCITER_H




