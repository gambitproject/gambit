//
// FILE: efgciter.h -- Extensive form contingency iterator class
//
// $Id$
//

#ifndef EFGCITER_H
#define EFGCITER_H

#include "gblock.h"

#include "efg.h"

class EfgIter;

//
// N.B.: Currently, the contingency iterator only allows one information
//       set to be "frozen".  This functionality will be expanded in the
//       future, but all current applications require only one to be
//       frozen at this time.
//
//       Iterating across all contingencies can be achieved by freezing
//       player number 0 (this is the default state on initialization)
//
class EfgContIter    {
  friend class EfgIter;
  private:
    int _frozen_pl, _frozen_iset;
    const Efg *_efg;
    EFSupport _support;
    PureBehavProfile<gNumber> _profile;
    gPVector<int> _current;
    mutable gVector<gNumber> _payoff;

  public:
    EfgContIter(const EFSupport &);
    ~EfgContIter();
  
    void First(void);
  
    void Freeze(int pl, int iset);
  
  // These next two only work on frozen infosets
    void Set(int pl, int iset, int act);
    int Next(int pl, int iset);
  
    const PureBehavProfile<gNumber> &GetProfile(void) const   { return _profile; }

    int NextContingency(void);
  
    gNumber Payoff(int pl) const;
  
    void Dump(gOutput &) const;
};

#endif   // NFGCITER_H




