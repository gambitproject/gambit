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
#include "gnumber.h"

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



class BehavSolution : public BehavProfile<gNumber>  {
protected:
  EfgAlgType _Creator;
  mutable TriState _IsNash;
  mutable TriState _IsSubgamePerfect;
  mutable TriState _IsSequential;
  gNumber _Epsilon;
  gNumber _GobitLambda;
  gNumber _GobitValue;
  mutable gNumber _LiapValue;
  mutable gDPVector<gNumber> *_Beliefs;
  mutable gDPVector<gNumber> *_Regret;
  unsigned int _Id;

  void EvalEquilibria(void) const;

public:
  BehavSolution(const BehavProfile<double> &, EfgAlgType creator = EfgAlg_USER);
  BehavSolution(const BehavProfile<gRational> &, EfgAlgType creator = EfgAlg_USER);
  BehavSolution(const BehavProfile<gNumber> &, EfgAlgType creator = EfgAlg_USER);
  BehavSolution(const BehavSolution &);
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

  void SetEpsilon(gNumber value);
  gNumber Epsilon(void) const; // epsilon for zero tolerance

  void SetGobit(gNumber lambda, gNumber value);
  gNumber GobitLambda(void) const; // lambda from gobit alg
  gNumber GobitValue(void) const; // objective function from gobit alg
  void SetLiap(gNumber value);
  gNumber LiapValue(void) const; // liapunov function value (to test for Nash)
  const gDPVector<gNumber> &Beliefs(void) const;
     // Belief vector, if a sequential equilibrium

  const gDPVector<gNumber> &Regret(void) const;
	 
  bool Equals(const BehavProfile<double> &s) const;
  bool operator==(const BehavSolution &) const;
  void Dump(gOutput& f) const;

  void Invalidate(void);

  gNumber& operator[](int);
  const gNumber& operator[](int) const;
  gNumber& operator()(int, int, int);
  const gNumber& operator()(int, int, int) const;

  BehavSolution& operator=(const BehavSolution &);
};

gOutput &operator<<(gOutput &f, const BehavSolution &);

#endif    // BEHAVSOL_H
