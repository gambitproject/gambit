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
    Efg *_efg;
    EFSupport _support;
    gPVector<int> _efgidx, _supidx;
    mutable gVector<gNumber> _payoff;

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




