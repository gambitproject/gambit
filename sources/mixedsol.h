//#
//# FILE: mixedsol.h -- Mixed strategy solution classes
//#
//# $Id$
//#



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
  void EvalEquilibria();

public:
  MixedSolution(const Nfg<T> &, bool truncated = false);
  MixedSolution(const Nfg<T> &, const NFSupport &);
  MixedSolution(const Nfg<T> &, const gPVector<T> &);
  MixedSolution(const MixedProfile<T> &, int creator = id_USER);

  MixedSolution(const MixedSolution<T> &);
  ~MixedSolution();

  void SetCreator(int);
  int Creator() const; // Who created this object? (algorithm ID or user)
  void SetIsNash(int);
  int IsNash(); // Is it Nash? Y/N/DK
  void SetIsPerfect(int);
  int IsPerfect(); //Is it Perfect? Y/N/DK
  void SetIsProper(int);
  int IsProper(); //Is it Proper? Y/N/DK

  void SetGobit(T lambda, T value);
  T GobitLambda() const; // lambda from gobit alg
  T GobitValue() const; // objective function from gobit alg
  void SetLiap(T value);
  T LiapValue(); // liaponuv function value (to test for Nash)

  bool operator==(const MixedSolution<T> &) const;
  void Dump(gOutput& f) const;


  void Invalidate();

  T& operator[](int);
	const T& operator[](int) const;
	T& operator()(int, int);
	const T& operator()(int, int) const;
	gPVector<T>& operator=(const gPVector<T>&);
  gPVector<T>& operator=(const gVector<T>&);
  gPVector<T>& operator=(T);
  gPVector<T>& operator+=(const gPVector<T>&);
  gVector<T>& operator+=(const gVector<T>&);
  gPVector<T>& operator-=(const gPVector<T>&);
  gVector<T>& operator-=(const gVector<T>&);
  gPVector<T>& operator*=(T);
  MixedProfile<T>& operator=(const MixedProfile<T>&);
};


#ifndef __BORLANDC__
template <class T> gOutput &operator<<(gOutput &f, const MixedSolution<T> &);
#endif

#endif    // MIXEDSOL_H
