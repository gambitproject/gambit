//#
//# FILE: behavsol.h -- Behav strategy solution classes
//#
//# $Id$
//#

#ifndef BEHAVSOL_H
#define BEHAVSOL_H


#include "gmisc.h"
#include "behav.h"
#include "gambitio.h"
#include "gstring.h"


typedef enum 
{
  EfgAlg_USER,
  EfgAlg_LIAP,
  EfgAlg_GOBIT,
  EfgAlg_PURENASH,
  EfgAlg_SEQFORM,
  EfgAlg_LEMKESUB,
  EfgAlg_SIMPDIVSUB,
  EfgAlg_ZSUMSUB,
  EfgAlg_PURENASHSUB,
  EfgAlg_SEQFORMSUB,
  EfgAlg_NLIAPSUB,
  EfgAlg_ELIAPSUB,
  EfgAlg_ENUMSUB,
  EfgAlg_CSSEQFORM
} EfgAlgType;

gString NameEfgAlgType(EfgAlgType i);
void DisplayEfgAlgType(gOutput& o, EfgAlgType i);



template <class T> class BehavSolution : public BehavProfile<T>
{
protected:
  EfgAlgType _Creator;
  TriState _IsNash;
  TriState _IsSubgamePerfect;
  TriState _IsSequential;
  T _GobitLambda;
  T _GobitValue;
  T _LiapValue;
  gDPVector<T> *_Beliefs;
  void EvalEquilibria();

public:
  BehavSolution(const Efg<T> &, bool truncated = false);
  BehavSolution(const Efg<T> &, const gDPVector<T> &);
  BehavSolution(const EFSupport &);
  BehavSolution(const BehavProfile<T> &, EfgAlgType creator = EfgAlg_USER);

  BehavSolution(const BehavSolution<T> &);
  ~BehavSolution();

  void SetCreator(EfgAlgType);
  EfgAlgType Creator() const; //Who created this object? (algorithm ID or user)
  void SetIsNash(TriState);
  TriState IsNash(); // Is it Nash? Y/N/DK
  void SetIsSubgamePerfect(TriState);
  TriState IsSubgamePerfect(); // Is it Subgame Perfect? Y/N/DK
  void SetIsSequential(TriState);
  TriState IsSequential(); // Is it Sequential? Y/N/DK

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
	const T& operator[](int) const;
	T& operator()(int, int, int);
	const T& operator()(int, int, int) const;
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
