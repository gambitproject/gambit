//
// FILE: behavsol.h -- Behavior strategy solution class
//
// $Id$
//

#ifndef BEHAVSOL_H
#define BEHAVSOL_H

#include "base/base.h"
#include "math/gnumber.h"
#include "game/behav.h"
#include "game/efg.h"
#include "algutils.h"  // needed for gFact

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
  algorithmEfg_QREALL_NFG,
  algorithmEfg_POLISH_NASH
} EfgAlgType;

gText ToText(EfgAlgType);
gOutput &operator<<(gOutput &, EfgAlgType);

class BehavSolution {
protected:
  BehavProfile<gNumber> *m_profile;
  gPrecision m_precision;
  mutable EFSupport m_support;
  mutable EfgAlgType m_creator;
  mutable gFact<gTriState> m_ANFNash, m_Nash, m_SubgamePerfect, m_Sequential;
  mutable gNumber m_epsilon, m_qreLambda, m_qreValue;
  mutable gFact<gNumber> m_liapValue;
  mutable gFact<gPVector<gNumber> > m_rnfRegret;
  gText m_name;
  mutable long m_revision;

  // PRIVATE AUXILIARY MEMBER FUNCTIONS
  gTriState GetNash(void) const;
  gTriState GetANFNash(void) const;
  gTriState GetSubgamePerfect(void) const;
  gTriState GetSequential(void) const;
  gPVector<gNumber> GetRNFRegret(void) const;

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
  gNumber operator[](Action *) const;
  gNumber &operator[](Action *);

  BehavSolution &operator+=(const BehavSolution &);
  BehavSolution &operator-=(const BehavSolution &);
  BehavSolution &operator*=(const gNumber &);

  // GENERAL DATA ACCESS
  efgGame &GetGame(void) const { return m_profile->GetGame(); }
  const BehavProfile<gNumber> *Profile(void) const { CheckIsValid(); return m_profile; }
  gPrecision Precision(void) const { return m_precision; }

  // Do probabilities sum to one (within m_epsilon) for each infoset?
  bool IsComplete(void) const;

  const gText &GetName(void) const { return m_name; }
  EfgAlgType Creator(void) const { CheckIsValid(); return m_creator; }
  EFSupport Support(void) const { CheckIsValid(); return m_support; }
  const gTriState &IsNash(void) const;
  BehavSolution PolishEq(void) const;
  const gTriState &IsANFNash(void) const;
  const gTriState &IsSubgamePerfect(void) const;
  const gTriState &IsSequential(void) const;
  const gNumber &Epsilon(void) const { CheckIsValid(); return m_epsilon; }
  const gNumber &QreLambda(void) const { CheckIsValid(); return m_qreLambda; }
  const gNumber &QreValue(void) const { CheckIsValid(); return m_qreValue; }
  const gNumber &LiapValue(void) const;
  const gPVector<gNumber> &ReducedNormalFormRegret(void) const;
  const gNumber MaxRegret(void) const;
  const gNumber MaxRNFRegret(void) const;

  void SetName(const gText &p_name) { m_name = p_name; }
  void SetCreator(EfgAlgType p_creator) { m_creator = p_creator; }
  void SetEpsilon(const gNumber &p_epsilon) { m_epsilon = p_epsilon; }
  void SetQre(const gNumber &p_qreLambda, const gNumber &p_qreValue)
    { m_qreLambda = p_qreLambda; m_qreValue = p_qreValue; }
	 
  // Force the invalidation of cached data
  void Invalidate(void) const;
  void CheckIsValid(void) const {if(!IsValid()) Invalidate();}
  bool IsValid(void) const {return (m_revision == GetGame().RevisionNumber());}

  // COMPUTATION OF INTERESTING QUANTITIES
  gNumber Payoff(int pl) const   { return m_profile->Payoff(pl); }
  
  // DATA ACCESS
  gNumber RealizProb(const Node *node) const
    { return m_profile->GetRealizProb(node); }
  gNumber BeliefProb(const Node *node) const
    { return m_profile->GetBeliefProb(node); }
  gVector<gNumber> NodeValue(const Node *node) const
    { return m_profile->GetNodeValue(node); }
  gNumber IsetProb(const Infoset *iset) const
    { return m_profile->GetIsetProb(iset); }
  gNumber IsetValue(const Infoset *iset) const
    { return m_profile->GetIsetValue(iset); }
  gNumber ActionProb(const Action *act) const
    { return m_profile->GetActionProb(act); }
  gNumber ActionValue(const Action *act) const
    { return m_profile->GetActionValue(act); }
  gNumber Regret(const Action *act) const
    { return m_profile->GetRegret(act); }

  // OUTPUT
  void Dump(gOutput &) const;
  void DumpInfo(gOutput &) const;
};

gOutput &operator<<(gOutput &f, const BehavSolution &);

#endif    // BEHAVSOL_H
