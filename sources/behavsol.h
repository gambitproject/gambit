//
// FILE: behavsol.h -- Behav strategy solution classes
//
// $Id$
//

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



template <class T> class BehavSolution : public BehavProfile<T>  {
protected:
  EfgAlgType _Creator;
  TriState _IsNash;
  TriState _IsSubgamePerfect;
  TriState _IsSequential;
  T _Epsilon;
  T _GobitLambda;
  T _GobitValue;
  T _LiapValue;
  gDPVector<T> *_Beliefs;
  gDPVector<T> *_Regret;
  unsigned int _Id;

  void EvalEquilibria(void);

public:
  BehavSolution(const Efg<T> &);
  BehavSolution(const Efg<T> &, const gDPVector<T> &);
  BehavSolution(const EFSupport &);
  BehavSolution(const BehavProfile<T> &, EfgAlgType creator = EfgAlg_USER);
  BehavSolution(const BehavSolution<T> &);
  virtual ~BehavSolution();

  unsigned int Id(void) const;
  void SetId(unsigned int );
  void SetCreator(EfgAlgType);
  bool IsComplete(void) const;

  EfgAlgType Creator(void) const; //Who created this object? (algorithm ID or user)
  void SetIsNash(TriState);
  TriState IsNash(void) const; // Is it Nash? Y/N/DK
  void SetIsSubgamePerfect(TriState);
  TriState IsSubgamePerfect(void) const; // Is it Subgame Perfect? Y/N/DK
  void SetIsSequential(TriState);
  TriState IsSequential(void) const; // Is it Sequential? Y/N/DK

  void SetEpsilon(T value);
  T Epsilon(void) const; // epsilon for zero tolerance

  void SetGobit(T lambda, T value);
  T GobitLambda(void) const; // lambda from gobit alg
  T GobitValue(void) const; // objective function from gobit alg
  void SetLiap(T value);
  T LiapValue(void) const; // liapunov function value (to test for Nash)
  const gDPVector<T> &Beliefs(void);
     // Belief vector, if a sequential equilibrium

  const gDPVector<T> &Regret(void);
	 
  bool operator==(const BehavSolution<T> &) const;
  void Dump(gOutput& f) const;

  void Invalidate(void);

  T& operator[](int);
  const T& operator[](int) const;
  T& operator()(int, int, int);
  const T& operator()(int, int, int) const;

  BehavSolution<T>& operator=(const BehavSolution<T> &);
  BehavSolution<T>& operator=(const BehavProfile<T>&);
  BehavSolution<T>& operator=(const gDPVector<T>&);
  BehavSolution<T>& operator=(const gPVector<T>&);
  BehavSolution<T>& operator=(const gVector<T>&);
  BehavSolution<T>& operator=(T);
  
  BehavSolution<T>& operator+=(const gDPVector<T>&);
  BehavSolution<T>& operator+=(const gPVector<T>&);
  BehavSolution<T>& operator+=(const gVector<T>&);
  BehavSolution<T>& operator-=(const gDPVector<T>&);
  BehavSolution<T>& operator-=(const gPVector<T>&);
  BehavSolution<T>& operator-=(const gVector<T>&);
  BehavSolution<T>& operator*=(T);
};

#ifndef __BORLANDC__
template <class T> gOutput &operator<<(gOutput &f, const BehavSolution<T> &);
#endif

#endif    // BEHAVSOL_H
