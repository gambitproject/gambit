//
// FILE: behav.h -- Behavioral strategy profile classes
//
// $Id$
//

#ifndef BEHAV_H
#define BEHAV_H

#include "gmisc.h"
#include "gtext.h"
#include "efstrat.h"

#include "gdpvect.h"

class Infoset;
template <class T> class BehavNode;
class Nfg;
template <class T> class MixedProfile;
template <class T> class gPVector;
template <class T> class gRectArray;
class BehavSolution;

template <class T> class BehavProfile : public gDPVector<T>  {
friend void MixedToBehav(const MixedProfile<T> &, BehavProfile<T> &);
protected:
  const Efg *m_efg;
  BehavNode<T> *m_root;
  gArray<BehavNode<T> *> m_nodes;
  EFSupport m_support;
  gPVector<T> *m_chanceprobs;
  gRectArray<T> *m_payoffs;

  // AUXILIARY MEMBER FUNCTIONS FOR COMPUTATION OF INTERESTING QUANTITES
  void Payoff(Node *, T, int, T &) const;
  void NodeValues(BehavNode<T> *, int, gArray<T> &, int &) const;
  void CondPayoff(BehavNode<T> *, T, gPVector<T> &, gDPVector<T> &) const;
  void NodeRealizProbs(BehavNode<T> *, T, int &, gArray<T> &) const;
  void Beliefs(Node *, T, gDPVector<T> &, gPVector<T> &) const;
  
public:
  // CONSTRUCTORS, DESTRUCTOR, CONSTRUCTIVE OPERATORS
  BehavProfile(const EFSupport &);
  BehavProfile(const BehavProfile<T> &);
  BehavProfile(const BehavSolution &);
  virtual ~BehavProfile();

  BehavProfile<T> &operator=(const BehavProfile<T> &);

  // GENERAL DATA ACCESS
  Efg &Game(void) const   { return const_cast<Efg &>(*m_efg); }
  const EFSupport &Support(void) const   { return m_support; }

  virtual bool IsAssessment(void) const { return false; }

  const T &GetValue(Infoset *, int) const;

  T LiapValue(void) const;
  void Gripe(gDPVector<T> &value) const;
  T MaxGripe(void) const;

  // OPERATOR OVERLOADING
  bool operator==(const BehavProfile<T> &) const;

  void Dump(gOutput &) const;

  // COMPUTATION OF INTERESTING QUANTITIES
  T Payoff(int p_player) const;

  gArray<T> NodeValues(int p_player) const;
  gArray<T> NodeRealizProbs(void) const;

  virtual void CondPayoff(gDPVector<T> &p_payoff, gPVector<T> &p_probs) const;
  virtual gDPVector<T> Beliefs(void) const;

  void Centroid(void) const;
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
  void CondPayoff(BehavNode<T> *, T, gPVector<T> &, gDPVector<T> &) const;

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



