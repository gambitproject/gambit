//#
//# FILE: behavsol.h -- Behav strategy solution classes
//#
//# $Id$
//#

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
  gDPVector<T> _Beliefs;
  void EvalEquilibria();

public:
  BehavSolution(const Efg<T> &, bool truncated = false);
  BehavSolution(const Efg<T> &, const gDPVector<T> &);
  BehavSolution(const EFSupport &);
  BehavSolution(const BehavProfile<T> &, int creator = id_USER);

  BehavSolution(const BehavSolution<T> &);
  ~BehavSolution();

  void SetCreator(int);
  int Creator() const; // Who created this object?  (algorithm ID or user)
  void SetIsNash(int);
  int IsNash(); // Is it Nash? Y/N/DK
  void SetIsSubgamePerfect(int);
  int IsSubgamePerfect(); // Is it Subgame Perfect? Y/N/DK
  void SetIsSequential(int);
  int IsSequential(); // Is it Sequential? Y/N/DK
  EFSupport Support() const; // Support of Profile

  void SetGobit(T lambda, T value);
  T GobitLambda() const; // lambda from gobit alg
  T GobitValue() const; // objective function from gobit alg
  void SetLiap(T value);
  T LiapValue(); // liapnov function value (to test for Nash)
  gDPVector<T> Beliefs(); // Belief vector, if a sequential equilibrium

  bool operator==(const BehavSolution<T> &) const;
  void Dump(gOutput& f) const;


  void Invalidate();

  T& operator[](int);
  T& operator()(int, int, int);
  gDPVector<T>& operator=(const gDPVector<T>&);
  gDPVector<T>& operator=(const gPVector<T>&);
  gDPVector<T>& operator=(const gVector<T>&);
  gDPVector<T>& operator=(T);
  gDPVector<T>& operator+=(const gDPVector<T>&);
  gPVector<T>& operator+=(const gPVector<T>&);
  gVector<T>& operator+=(const gVector<T>&);
  gDPVector<T>& operator-=(const gDPVector<T>&);
  gPVector<T>& operator-=(const gPVector<T>&);
  gVector<T>& operator-=(const gVector<T>&);
  gDPVector<T>& operator*=(T);
  BehavProfile<T>& operator=(const BehavProfile<T>&);
};

#ifndef __BORLANDC__
template <class T> gOutput &operator<<(gOutput &f, const BehavSolution<T> &);
#endif

#endif    // BEHAVSOL_H
