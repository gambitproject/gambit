//
// FILE: behav.h -- Behavioral strategy profile classes
//
// $Id$
//

#ifndef BEHAV_H
#define BEHAV_H

#include "base/base.h"
#include "math/gdpvect.h"
#include "math/gmatrix.h"
#include "efstrat.h"

class Infoset;
class Nfg;
template <class T> class MixedProfile;
template <class T> class gPVector;
template <class T> class gRectArray;
class BehavSolution;

//
//  BehavProfile<T> implements a behavior profile on an Efg.  
//
//  The class assumes that the underlying Efg does not change during the 
//  life of the profile, and will not correctly invalidate itself if 
//  the game does change.  
// 
//  The BehavSolution class should be used For interactive use, where 
//  the game payoffs or probabilities may change.  
// 

template <class T> class BehavProfile : public gDPVector<T>  {
  friend BehavSolution;
protected:
  const Efg::Game *m_efg;
  EFSupport m_support;
  mutable bool m_cached_data;

  // structures for storing cached data: nodes
  mutable gVector<T> m_realizProbs, m_beliefs, m_nvals, m_bvals;
  mutable gMatrix<T> m_nodeValues;

  // structures for storing cached data: information sets
  mutable gPVector<T> m_infosetValues;

  // structures for storing cached data: actions
  mutable gDPVector<T> m_actionValues;   // aka conditional payoffs
  mutable gDPVector<T> m_gripe;

  void InitPayoffs(void) const;
  void InitProfile(void);

  //
  // FUNCTIONS FOR DATA ACCESS
  //
  // NOTE: These functions all assume that profile is installed, and that relevant 
  // data has been computed.  
  // Use public versions (GetNodeValue, GetIsetProb, etc) if this is not known.

  const T &RealizProb(const Node *node) const;
  T &RealizProb(const Node *node);

  const T &BeliefProb(const Node *node) const;
  T &BeliefProb(const Node *node);
  
  gVector<T> NodeValues(const Node *node) const
    { return m_nodeValues.Row(node->number); }
  const T &NodeValue(const Node *node, int pl) const
    { return m_nodeValues(node->number, pl); }
  T &NodeValue(const Node *node, int pl)
    { return m_nodeValues(node->number, pl); }

  T IsetProb(const Infoset *iset) const;

  const T &IsetValue(const Infoset *iset) const;
  T &IsetValue(const Infoset *iset);

  const T &ActionValue(const Action *act) const 
    { return m_actionValues(act->BelongsTo()->GetPlayer()->GetNumber(),
			    act->BelongsTo()->GetNumber(),
			    act->number); }
  T &ActionValue(const Action *act)
    { return m_actionValues(act->BelongsTo()->GetPlayer()->GetNumber(),
			    act->BelongsTo()->GetNumber(),
			    act->number); }
  
  T ActionProb(const Action *act) const;

  const T &Regret(const Action * act) const;
  T &Regret(const Action *);

  // AUXILIARY MEMBER FUNCTIONS FOR COMPUTATION OF INTERESTING QUANTITES

  void Payoff(Node *, T, int, T &) const;
  T Payoff(FullEfgNamespace::Outcome *, int pl) const;
  
  void ComputeSolutionDataPass2(const Node *node);
  void ComputeSolutionDataPass1(const Node *node);
  void ComputeSolutionData(void);

  void BehaviorStrat(const Efg::Game &, int, Node *);
  void RealizationProbs(const MixedProfile<T> &, const Efg::Game &,
			int pl, const gArray<int> *const, Node *);

public:
  class BadStuff : public gException  {
  public:
    virtual ~BadStuff();
    gText Description(void) const;
  };

  // CONSTRUCTORS, DESTRUCTOR

  BehavProfile(const EFSupport &);
  BehavProfile(const BehavProfile<T> &);
  BehavProfile(const MixedProfile<T> &);
  BehavProfile(const BehavSolution &);
  virtual ~BehavProfile();
  
  // OPERATOR OVERLOADING

  BehavProfile<T> &operator=(const BehavProfile<T> &);
  inline BehavProfile<T> &operator=(const gVector<T> &p)
    {Invalidate(); gVector<T>::operator=(p); return *this;}

  bool operator==(const BehavProfile<T> &) const;

  // INITIALIZATION, VALIDATION
  inline void Invalidate(void) const {m_cached_data=false;}
  virtual bool IsAssessment(void) const { return false; }
  void Centroid(void) const;

  // GENERAL DATA ACCESS

  Efg::Game &GetGame(void) const   { return const_cast<Efg::Game &>(*m_efg); }
  const EFSupport &Support(void) const   { return m_support; }
  
  const T &GetRealizProb(const Node *node);
  const T &GetBeliefProb(const Node *node);
  gVector<T> GetNodeValue(const Node *node);
  T GetIsetProb(const Infoset *iset);
  const T &GetIsetValue(const Infoset *iset);
  T GetActionProb(const Action *act) const;
  const T &GetActionValue(const Action *act) const;
  const T &GetRegret(const Action *act);

  // COMPUTATION OF INTERESTING QUANTITIES

  T Payoff(int p_player) const;
  gDPVector<T> Beliefs(void);
  T LiapValue(void);
  T QreValue(const gVector<T> &lambda, bool &);
  T MaxRegret(void);

  T DiffActionValue(const Action *action, const Action *oppAction) const;
  T DiffRealizProb(const Node *node, const Action *oppAction) const;
  T DiffNodeValue(const Node *node, const EFPlayer *player,
		  const Action *oppAction) const;

  void Dump(gOutput &) const;
};


//
// Behavioral assessment class.
// Allows for explicit storage and manipulation of a belief system in
// addition to a profile of behavioral strategies
//

template <class T> class BehavAssessment : public BehavProfile<T> {
protected:
  gDPVector<T> m_beliefs;
  
  // AUXILIARY MEMBER FUNCTIONS FOR COMPUTATION OF INTERESTING QUANTITES
  void CondPayoff(Node *, T,
		  gPVector<T> &, gDPVector<T> &) const;
  
public:
  // CONSTRUCTORS, DESTRUCTOR, CONSTRUCTIVE OPERATORS
  BehavAssessment(const EFSupport &);
  BehavAssessment(const BehavProfile<T> &);
  BehavAssessment(const BehavAssessment<T> &);
  virtual ~BehavAssessment();
  
  BehavAssessment<T> &operator=(const BehavAssessment<T> &);
  
  // GENERAL DATA ACCESS
  bool IsAssessment(void) const { return true; }
  
  // ACCESS AND MANIPULATION OF BELIEFS
  gDPVector<T> Beliefs(void) const;
  gDPVector<T> &Beliefs(void);
  
  void CondPayoff(gDPVector<T> &p_payoff, gPVector<T> &p_probs) const;
  
  // OUTPUT
  void Dump(gOutput &) const;
};


#ifndef __BORLANDC__
template <class T> gOutput &operator<<(gOutput &, const BehavProfile<T> &);
template <class T> gOutput &operator<<(gOutput &, const BehavAssessment<T> &);
#endif

#endif   // BEHAV_H
