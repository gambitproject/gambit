//
// FILE: behavsol.h -- Behav strategy solution classes
//
// $Id$
//

#ifndef BEHAVSOL_H
#define BEHAVSOL_H


#include "gmisc.h"
#include "behav.h"
#include "gstream.h"
#include "gtext.h"
#include "gnumber.h"
#include "efg.h"

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
  EfgAlg_CSSEQFORM,
  EfgAlg_POLENSUB,
  EfgAlg_POLENUM,
} EfgAlgType;

gText NameEfgAlgType(EfgAlgType i);
void DisplayEfgAlgType(gOutput& o, EfgAlgType i);

class BehavSolution {
protected:
  BehavProfile<gNumber> *m_profile;
  mutable EfgAlgType m_creator;
  mutable gTriState m_isNash, m_isSubgamePerfect, m_isSequential;
  mutable gNumber m_epsilon, m_gobitLambda, m_gobitValue, m_liapValue;
  mutable gDPVector<gNumber> *m_beliefs, *m_regret;
  unsigned int m_id;

  void EvalEquilibria(void) const;

public:
  // CONSTRUCTORS, DESTRUCTOR, CONSTRUCTIVE OPERATORS
  BehavSolution(const BehavProfile<double> &, EfgAlgType = EfgAlg_USER);
  BehavSolution(const BehavProfile<gRational> &, EfgAlgType = EfgAlg_USER);
  BehavSolution(const BehavProfile<gNumber> &, EfgAlgType = EfgAlg_USER);
  BehavSolution(const BehavSolution &);
  virtual ~BehavSolution();

  BehavSolution &operator=(const BehavSolution &);

  // OPERATOR OVERLOADING
  bool Equals(const BehavProfile<double> &) const;
  bool operator==(const BehavSolution &) const;
  bool operator!=(const BehavSolution &p_solution) const
    { return !(*this == p_solution); }

  gNumber &operator()(int, int, int);
  const gNumber &operator()(int, int, int) const;

  BehavSolution &operator+=(const BehavSolution &);
  BehavSolution &operator-=(const BehavSolution &);
  BehavSolution &operator*=(const gNumber &);

  // GENERAL DATA ACCESS
  Efg &Game(void) const   { return m_profile->Game(); }

  // Do probabilities sum to one (within m_epsilon) for each infoset?
  bool IsComplete(void) const;

  unsigned int Id(void) const { return m_id; }
  EfgAlgType Creator(void) const { return m_creator; }
  gTriState IsNash(void) const;
  gTriState IsSubgamePerfect(void) const;
  gTriState IsSequential(void) const;
  const gNumber &Epsilon(void) const { return m_epsilon; }
  const gNumber &GobitLambda(void) const { return m_gobitLambda; }
  const gNumber &GobitValue(void) const { return m_gobitValue; }
  const gNumber &LiapValue(void) const;
  const gDPVector<gNumber> &Beliefs(void) const;
  const gDPVector<gNumber> &Regret(void) const;

  void SetId(unsigned int p_id) { m_id = p_id; }
  void SetCreator(EfgAlgType p_creator) { m_creator = p_creator; }
  void SetIsNash(gTriState p_isNash) { m_isNash = p_isNash; }
  void SetIsSubgamePerfect(gTriState p_isSubgamePerfect)
    { m_isSubgamePerfect = p_isSubgamePerfect; }
  void SetIsSequential(gTriState p_isSequential)
    { m_isSequential = p_isSequential; }
  void SetEpsilon(const gNumber &p_epsilon) { m_epsilon = p_epsilon; }
  void SetGobit(const gNumber &p_gobitLambda, const gNumber &p_gobitValue)
    { m_gobitLambda = p_gobitLambda; m_gobitValue = p_gobitValue; }
  void SetLiap(const gNumber &p_liapValue) { m_liapValue = p_liapValue; }
	 
  // Force the invalidation of cached quantities
  void Invalidate(void) const;

  // FUNCTIONS FOR COMPATIBILITY WITH GUI
  // these are all obsolescent :)

  EFSupport Support(void) const { return EFSupport(Game()); }


  // COMPUTATION OF INTERESTING QUANTITIES
  gNumber Payoff(int pl) const   { return m_profile->Payoff(pl); }
  void CondPayoff(gDPVector<gNumber> &value, gPVector<gNumber> &probs) const
    { m_profile->CondPayoff(value, probs); }
  gArray<gNumber> NodeValues(int pl) const
    { return m_profile->NodeValues(pl); }
  gArray<gNumber> NodeRealizProbs(void) const
    { return m_profile->NodeRealizProbs(); }
  const gNumber &GetValue(Infoset *s, int act) const
    { return m_profile->GetValue(s, act); }

  // OUTPUT
  void Dump(gOutput &) const;
  void DumpInfo(gOutput &) const;
};

gOutput &operator<<(gOutput &f, const BehavSolution &);

#endif    // BEHAVSOL_H
