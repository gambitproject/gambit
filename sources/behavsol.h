
#ifndef BEHAVSOL_H
#define BEHAVSOL_H


#include "gsmincl.h"
#include "behav.h"


template <class T> class BehavSolution : public BehavProfile<T>
{
protected:
  int _Creator;
  int _IsNash;
  int _IsSubgamePerfect;
  int _IsSequential;
  T _GobitLambda;
  T _GobitValue;
  T _LiapValue;
  gVector<T> _Beliefs;

public:
  BehavSolution(const Efg<T> &, bool truncated = false);
  BehavSolution(const Efg<T> &, const gDPVector<T> &);
  BehavSolution(const EFSupport &);
  BehavSolution(const BehavProfile<T> &);

  BehavSolution(const BehavSolution<T> &);
  ~BehavSolution();

  int Creator() const; // Who created this object?  (algorithm ID or user)
  int IsNash() const; // Is it Nash? Y/N/DK
  int IsSubgamePerfect() const; // Is it Subgame Perfect? Y/N/DK
  int IsSequential() const; // Is it Sequential? Y/N/DK
  EFSupport Support() const; // Support of Profile
  T GobitLambda() const; // lambda from gobit alg
  T GobitValue() const; // objective function from gobit alg
  T LiapValue() const; // liapnov function value (to test for Nash)
  gVector<T> Beliefs(); // Belief vector, if a sequential equilibrium

  bool operator==(const BehavSolution<T> &) const;
};

#ifndef __BORLANDC__
template <class T> gOutput &operator<<(gOutput &f, const BehavSolution<T> &);
#endif

#endif    // BEHAVSOL_H
