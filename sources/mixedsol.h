//#
//# FILE: mixedsol.h -- Mixed strategy solution classes
//#
//# $Id$
//#



#ifndef MIXEDSOL_H
#define MIXEDSOL_H

#include "gmisc.h"
#include "mixed.h"
#include "gambitio.h"
#include "gstring.h"

typedef enum 
{
  NfgAlg_USER,
  NfgAlg_GOBIT,
  NfgAlg_LIAP,
  NfgAlg_LEMKE,
  NfgAlg_ZSUM,
  NfgAlg_ENUM,
  NfgAlg_GOBITGRID,
  NfgAlg_SIMPDIV,
  NfgAlg_PURENASH,
  NfgAlg_SEQFORM,
} NfgAlgType;

gString NameNfgAlgType(NfgAlgType i);
void DisplayNfgAlgType(gOutput& o, NfgAlgType i);



template <class T> class MixedSolution : public MixedProfile<T>
{
protected:
  NfgAlgType _Creator;
  TriState _IsNash;
  TriState _IsPerfect;
  TriState _IsProper;
  T _GobitLambda;
  T _GobitValue;
  T _LiapValue;
  gArray<T> _Payoff;
  void EvalEquilibria();

public:
  MixedSolution(const Nfg<T> &, bool truncated = false);
  MixedSolution(const Nfg<T> &, const NFSupport &);
  MixedSolution(const Nfg<T> &, const gPVector<T> &);
  MixedSolution(const MixedProfile<T> &, NfgAlgType creator = NfgAlg_USER);

  MixedSolution(const MixedSolution<T> &);
  ~MixedSolution();

  void SetCreator(NfgAlgType);
  NfgAlgType Creator() const; //Who created this object? (algorithm ID or user)
  void SetIsNash(TriState);
  TriState IsNash(); // Is it Nash? Y/N/DK
  void SetIsPerfect(TriState);
  TriState IsPerfect(); //Is it Perfect? Y/N/DK
  void SetIsProper(TriState);
  TriState IsProper(); //Is it Proper? Y/N/DK

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
