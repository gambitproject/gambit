//
// FILE: behavsol.h -- Behavior strategy solution class
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

typedef enum {
  algorithmEfg_USER,
  algorithmEfg_ENUMPURE_EFG, algorithmEfg_ENUMPURE_NFG,
  algorithmEfg_ENUMMIXED_NFG,
  algorithmEfg_LCP_EFG, algorithmEfg_LCP_NFG,
  algorithmEfg_LP_EFG, algorithmEfg_LP_NFG,
  algorithmEfg_LIAP_EFG, algorithmEfg_LIAP_NFG,
  algorithmEfg_SIMPDIV_NFG,
  algorithmEfg_POLENUM_EFG, algorithmEfg_POLENUM_NFG,
  algorithmEfg_QRE_EFG, algorithmEfg_QRE_NFG,
  algorithmEfg_QREALL_NFG
} EfgAlgType;

gText NameEfgAlgType(EfgAlgType i);
void DisplayEfgAlgType(gOutput& o, EfgAlgType i);

class BehavSolution {
protected:
  BehavProfile<gNumber> *m_profile;
  gPrecision m_precision;
  mutable EFSupport m_support;
  mutable EfgAlgType m_creator;
  mutable gTriState m_isNash, m_isSubgamePerfect, m_isSequential;
  mutable gNumber m_epsilon, m_qreLambda, m_qreValue, m_liapValue;
  mutable gDPVector<gNumber> *m_beliefs, *m_regret;
  unsigned int m_id;

  // PRIVATE AUXILIARY MEMBER FUNCTIONS
  void EvalEquilibria(void) const;
  void LevelPrecision(void);

public:
  // CONSTRUCTORS, DESTRUCTOR, CONSTRUCTIVE OPERATORS
  BehavSolution(const BehavProfile<double> &, EfgAlgType = algorithmEfg_USER);
  BehavSolution(const BehavProfile<gRational> &,
		EfgAlgType = algorithmEfg_USER);
  BehavSolution(const BehavProfile<gNumber> &, EfgAlgType = algorithmEfg_USER);
  BehavSolution(const BehavSolution &);
  virtual ~BehavSolution();

  BehavSolution &operator=(const BehavSolution &);

  // OPERATOR OVERLOADING
  bool Equals(const BehavProfile<double> &) const;
  bool operator==(const BehavSolution &) const;
  bool operator!=(const BehavSolution &p_solution) const
    { return !(*this == p_solution); }

  void Set(Action *, const gNumber &);
  void Set(int, int, int, const gNumber &);
  const gNumber &operator()(Action *) const;

  BehavSolution &operator+=(const BehavSolution &);
  BehavSolution &operator-=(const BehavSolution &);
  BehavSolution &operator*=(const gNumber &);

  // GENERAL DATA ACCESS
  Efg &Game(void) const { return m_profile->Game(); }
  gPrecision Precision(void) const { return m_precision; }

  // Do probabilities sum to one (within m_epsilon) for each infoset?
  bool IsComplete(void) const;

  unsigned int Id(void) const { return m_id; }
  EfgAlgType Creator(void) const { return m_creator; }
  EFSupport Support(void) const { return m_support; }
  gTriState IsNash(void) const;
  gTriState IsSubgamePerfect(void) const;
  gTriState IsSequential(void) const;
  const gNumber &Epsilon(void) const { return m_epsilon; }
  const gNumber &QreLambda(void) const { return m_qreLambda; }
  const gNumber &QreValue(void) const { return m_qreValue; }
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
  void SetQre(const gNumber &p_qreLambda, const gNumber &p_qreValue)
    { m_qreLambda = p_qreLambda; m_qreValue = p_qreValue; }
  void SetLiap(const gNumber &p_liapValue) { m_liapValue = p_liapValue; }
	 
  // Force the invalidation of cached quantities
  void Invalidate(void) const;



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
