//#
//# FILE: nfgiter.h -- Normal form iterator class
//#
//# $Id$
//#

#ifndef NFGITER_H
#define NFGITER_H

template <class T> class gArray;
template <class T> class Nfg;
template <class T> class ContIter;
class StrategyProfile;
class NFSupport;

//
// This class is useful for iterating around the normal form.
//

template <class T> class NfgIter    {
private:
  const NFSupport *stratset;
  Nfg<T> *N;
  gArray<int> current_strat;
  StrategyProfile *profile;
  
public:
  NfgIter(const NFSupport *s);
  NfgIter(const NfgIter<T> &);
  NfgIter(const ContIter<T> &);
  ~NfgIter();
  
  NfgIter<T> &operator=(const NfgIter<T> &);
  
  void First(void);
  int Next(int p);
  int Set(int p, int s);
  
  void Get(gArray<int> &t) const;
  void Set(const gArray<int> &t);
  
  long GetIndex(void) const;
  
  const T &Payoff(int p) const;
  void SetPayoff(int p, const T &);
//  void Payoff(gVector<T> &value) const;
  
//  const gArray<int> *const GetActions(int pl) const;

  Strategy *GetStrategy(int pl, int num);
};

#endif   // NFGITER_H




