


#ifndef MIXEDSOL_H
#define MIXEDSOL_H


#include "gsmincl.h"
#include "mixed.h"


template <class T> class MixedSolution : public MixedProfile<T>
{
protected:
  int _Creator;
  int _IsNash;
  int _IsPerfect;
  int _IsProper;
  T _GobitLambda;
  T _GobitValue;
  T _LiapValue;
  gArray<T> _Payoff;

public:
  MixedSolution(const Nfg<T> &, bool truncated = false);
  MixedSolution(const Nfg<T> &, const NFSupport &);
  MixedSolution(const Nfg<T> &, const gPVector<T> &);
  MixedSolution(const MixedProfile<T> &);

  MixedSolution(const MixedSolution<T> &);
  ~MixedSolution();

  int Creator() const; // Who created this object? (algorithm ID or user)
  int IsNash(); // Is it Nash? Y/N/DK
  int IsPerfect() const; //Is it Perfect? Y/N/DK
  int IsProper() const; //Is it Proper? Y/N/DK
  NFSupport Support() const; //Support of Profile
  T GobitLambda() const; // lambda from gobit alg
  T GobitValue() const; // objective function from gobit alg
  T LiapValue() const; // liaponuv function value (to test for Nash)

  bool operator==(const MixedSolution<T> &) const;
};


#ifndef __BORLANDC__
template <class T> gOutput &operator<<(gOutput &f, const MixedSolution<T> &);
#endif

#endif    // MIXEDSOL_H
