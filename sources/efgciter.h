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
//       set to be "frozen".  There is a list of "active" information
//       sets, which are those whose actions are cycled over, the idea
//       being that behavior at inactive information sets is irrelevant.
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
    gBlock<gBlock<bool> > _is_active;
    gBlock<int> _num_active_infosets;
    mutable gVector<gNumber> _payoff;

  public:
    EfgContIter(const EFSupport &);
    EfgContIter(const EFSupport &, const gList<Infoset *> &);
    ~EfgContIter();
  
    void First(void);
  
    void Freeze(int pl, int iset);
  
  // These next two only work on frozen infosets
    void Set(int pl, int iset, int act);
    int Next(int pl, int iset);
  
    const PureBehavProfile<gNumber> &GetProfile(void) const   
      { return _profile; }

    int NextContingency(void);
  
    gNumber Payoff(int pl) const;
  
    void Dump(gOutput &) const;
};

// The following class is like the above, but intended for iteration
// over contingencies that are relevant once a particular node 
// has been reached.
class EfgConditionalContIter    {
  friend class EfgIter;
  private:
    const Efg *_efg;
    EFSupport _support;
    PureBehavProfile<gNumber> _profile;
    gPVector<int> _current;
    gBlock<gBlock<bool> > _is_active;
    gBlock<int> _num_active_infosets;
    mutable gVector<gNumber> _payoff;

  public:
    EfgConditionalContIter(const EFSupport &);
    EfgConditionalContIter(const EFSupport &, const gList<const Infoset *> &);
    ~EfgConditionalContIter();
  
    void First(void); // Sets each infoset's action to the first in the support
  
    void Set(int pl, int iset, int act);
    int Next(int pl, int iset); 
  
    const PureBehavProfile<gNumber> &GetProfile(void) const   
      { return _profile; }

    int NextContingency(void);   // Needs rewriting
  
    gNumber Payoff(int pl) const;
    gNumber Payoff(const Node *, int pl) const;
  
    void Dump(gOutput &) const;
};

#endif   // NFGCITER_H




