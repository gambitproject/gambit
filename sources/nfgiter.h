//
// FILE: nfgiter.h -- Normal form iterator class
//
// @(#)nfgiter.h	2.7 4/3/97
//

#ifndef NFGITER_H
#define NFGITER_H

template <class T> class gArray;
template <class T> class Nfg;

class StrategyProfile;
#include "nfstrat.h"
class NFSupport;
class NfgContIter;
class NFOutcome;

//
// This class is useful for iterating around the normal form.
//

class NfgIter    {
private:
  NFSupport support;
  NFGameForm *N;
  gArray<int> current_strat;
  StrategyProfile profile;

public:
  NfgIter(NFGameForm &);
  NfgIter(const NFSupport &s);
  NfgIter(const NfgIter &);
  NfgIter(const NfgContIter &);
  ~NfgIter();

  NfgIter &operator=(const NfgIter &);

  void First(void);
  int Next(int p);
  int Set(int p, int s);

  void Get(gArray<int> &t) const;
  void Set(const gArray<int> &t);

  long GetIndex(void) const;

  NFOutcome *GetOutcome(void) const;
  void SetOutcome(NFOutcome *);

  const NFSupport &Support(void) const { return support; }
};

#endif   // NFGITER_H




