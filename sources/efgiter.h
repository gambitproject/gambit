//
// FILE: efgiter.h -- Extensive form strategy profile iterator class
//
// $Id$
//

#ifndef EFGITER_H
#define EFGITER_H

#include "efg.h"

template <class T> class EfgContIter;

template <class T> class EfgIter    {
  private:
    const Efg *_efg;
    EFSupport _support;
    PureBehavProfile<T> _profile;
    gPVector<int> _current;
    mutable gVector<T> _payoff;

  public:
    EfgIter(Efg &);
    EfgIter(const EFSupport &);
    EfgIter(const EfgIter<T> &);
    EfgIter(const EfgContIter<T> &);
    ~EfgIter();
  
    EfgIter<T> &operator=(const EfgIter<T> &);
  
    void First(void);
    int Next(int p, int iset);
    int Set(int p, int iset, int act);
  
    T Payoff(int p) const;
    void Payoff(gVector<T> &) const;

    const EFSupport &GetEFSupport(void) const;

    void Dump(gOutput &) const;
};

#endif   // EFGITER_H




