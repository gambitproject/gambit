//
// FILE: efgiter.h -- Extensive form strategy profile iterator class
//
// $Id$
//

#ifndef EFGITER_H
#define EFGITER_H

#include "efg.h"

class EfgContIter;

class EfgIter    {
  private:
    const Efg *_efg;
    EFSupport _support;
    PureBehavProfile<gNumber> _profile;
    gPVector<int> _current;
    mutable gVector<gNumber> _payoff;

  public:
    EfgIter(Efg &);
    EfgIter(const EFSupport &);
    EfgIter(const EfgIter &);
    EfgIter(const EfgContIter &);
    ~EfgIter();
  
    EfgIter &operator=(const EfgIter &);
  
    void First(void);
    int Next(int p, int iset);
    int Set(int p, int iset, int act);
  
    gNumber Payoff(int p) const;
    void Payoff(gVector<gNumber> &) const;

    const EFSupport &Support(void) const;

    void Dump(gOutput &) const;
};

#endif   // EFGITER_H




