//
// FILE: nfgciter.h -- Normal form contingency iterator class
//
// $Id$
//

#ifndef NFGCITER_H
#define NFGCITER_H

#include "gblock.h"

template <class T> class Nfg;
class StrategyProfile;

//
// This class is useful for iterating around the normal form.  This iterator
// allows the user choose to hold some players' strategies constant while
// iterating over the other players' strategies.  This is useful, for example,
// in displaying a 2D window into the n-D space of strategy profiles as in
// the normal form display code.  The constructor takes the normal form to
// work on and a gBlock<int> containing the numbers of players the user wants
// to hold constant.  The iteration is based on an index that goes from 1 to
// the total number of possible contingencies in increments of 1.
//
class NfgContIter    {
friend class NfgIter;
private:
  NFSupport support;
  gArray<int> current_strat;
  BaseNfg *N;
  StrategyProfile profile;
  gBlock<int> frozen, thawed;
  
public:
  NfgContIter(const NFSupport &s);
  ~NfgContIter();
  
  void First(void);
  
  void Freeze(const gBlock<int> &);
  void Freeze(int);
  void Thaw(int);
  
  // These next two only work on frozen strategies
  void Set(int pl, int num);
  int Next(int pl);
  
  gArray<int> Get(void) const;
  void Get(gArray<int> &t) const;
  
  int NextContingency(void);
  
  long GetIndex(void) const;
  
  NFOutcome *GetOutcome(void) const;
  void SetOutcome(NFOutcome *);
  
  void Dump(gOutput &) const;
};

#endif   // NFGCITER_H




