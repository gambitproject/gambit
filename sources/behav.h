//
// FILE: behav.h -- Behavioral strategy profile classes
//
// $Id$
//

#ifndef BEHAV_H
#define BEHAV_H

#include "base/base.h"
#include "math/gdpvect.h"
#include "efstrat.h"

#include "rectangl.h"
#include "ineqsolv.h"

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
public:
  struct BehavInfoset;
  struct BehavAction;
  struct BehavNode;
protected:
  const Efg *m_efg;
  BehavNode *m_root;
  EFSupport *m_support;
  gArray<BehavInfoset *> m_isets;
  mutable bool m_cached_data;
  
  //
  // FUNCTIONS FOR INSTALLATION AND INITIALIZATION
  //
  // Installation sets back-pointers in EFG to point to relevant 
  // stuff in BehavProfile

  void InstallMe(void) const;  
  void InstallMe(BehavNode *) const; 
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

  inline const gVector<T> &NodeValue(const Node *node) const
    {return ((BehavNode *)node->solution)->nodeValue;}
  inline gVector<T> &NodeValue(const Node *node)
    {return ((BehavNode *)node->solution)->nodeValue;}
 
  const T &IsetProb(const Infoset *iset) const;
  T &IsetProb(const Infoset *iset);

  const T &IsetValue(const Infoset *iset) const;
  T &IsetValue(const Infoset *iset);

  inline const T &ActionValue(const Action * act) const 
    { return ((BehavAction *)(act->solution))->condPayoff;}	
  inline T &ActionValue(const Action * act)
    { return ((BehavAction *)(act->solution))->condPayoff;}
  
  inline const T &ActionProb(const Action *act) const 
    { return *(((BehavAction *)(act->solution))->probability);}	
  inline T &ActionProb(const Action *act) 
    { return *(((BehavAction *)(act->solution))->probability);}	

  const T &Regret(const Action * act) const;
  T &Regret(const Action *);

  // AUXILIARY MEMBER FUNCTIONS FOR COMPUTATION OF INTERESTING QUANTITES

  void Payoff(Node *, T, int, T &) const;
  const T Payoff(const EFOutcome *o, int pl) const;
  
  void ComputeSolutionDataPass2(const Node *node);
  void ComputeSolutionDataPass1(const Node *node);
  void ComputeSolutionData(void);

  void BehaviorStrat(const Efg &, int, BehavNode *);
  void RealizationProbs(const MixedProfile<T> &, const Efg &,
			int pl, const gArray<int> *const, BehavNode *);

  // USED IN TEST WHETHER PROFILE (RESTRICTED TO SUPPORT) EXTENDS TO ANF NASH
  
  gPolyList<gDouble> ActionProbsSumToOneIneqs(const gSpace &, 
					      const term_order &,
					      const EFSupport &,
					      const gList<gList<int> > &) const;
  
  bool ANFNodeProbabilityPoly(gPoly<gDouble> &,
			      const gSpace &, 
			      const term_order &,
			      const EFSupport &,
			      const gList<gList<int> > &,
			      const Node *,
			      const int &pl,
			      const int &i,
			      const int &j) const;
  
  gPolyList<gDouble> ANFExpectedPayoffDiffPolys(const gSpace &, 
						const term_order&,
						const EFSupport &,
						const EFSupport &,
						const gList<gList<int> > &) const;
  
  gPolyList<gDouble> ExtendsToANFNashIneqs(const gSpace &, 
					   const term_order&,
					   const EFSupport&,
					   const EFSupport&,
					   const gList<gList<int> > &) const;
  
  // USED IN TEST WHETHER PROFILE (RESTRICTED TO SUPPORT) EXTENDS TO BEHAV NASH
  
  void DeviationInfosetsRECURSION(gList<const Infoset *> &,
				  const EFSupport & big_supp,
				  const EFPlayer *pl,
				  const Node* node,
				  const Action *act) const;
  
  const gList<const Infoset *> DeviationInfosets(const EFSupport &,
						 const EFPlayer *,
						 const Infoset *,
						 const Action *) const;
  
  const gList<const EFSupport> DeviationSupports(const EFSupport &,
						 const gList<const Infoset*> &,
						 const EFPlayer *,
						 const Infoset *,
						 const Action *) const;
  
  bool NashNodeProbabilityPoly(      gPoly<gDouble> &,
				     const gSpace &, 
				     const term_order &,
				     const EFSupport &,
				     const gList<gList<int> > &,
				     const Node *,
				     const EFPlayer *,
				     const Infoset *,
				     const Action *) const;
  
  gPolyList<gDouble> NashExpectedPayoffDiffPolys(const gSpace &, 
						 const term_order&,
						 const EFSupport &,
						 const EFSupport &,
						 const gList<gList<int> > &) const;
  
  gPolyList<gDouble> ExtendsToNashIneqs(const gSpace &, 
					const term_order&,
					const EFSupport&,
					const EFSupport&,
					const gList<gList<int> > &) const;
  
public:
  class BadStuff : public gException  {
  public:
    virtual ~BadStuff();
    gText Description(void) const;
  };

  //
  // STRUCTS FOR STORING SOLUTION DATA
  //

  struct BehavNode {
    Node *node;
    T nval, bval;
    gArray<BehavNode *> children;
    
    T realizProb, belief;
    gVector<T> nodeValue;
    
    BehavNode(Efg *e, Node *n, int pl);
    ~BehavNode();
    
    void ClearNodeProbs(void);
  };

  struct BehavInfoset {
    Infoset *iset;
    gArray<BehavAction *> actions;
    T value, prob;
    
    BehavInfoset(const EFSupport &s, Infoset *i, int pl);
    ~BehavInfoset();
  };
  struct BehavAction {
    Action *action;
    T *probability;
    T condPayoff, gripe;
    
    BehavAction(Action *act, int pl);
    ~BehavAction();
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

  // INSTALLATION, INITIALIZATION, VALIDATION

  bool IsInstalled(void) const;  
  inline void Invalidate(void) const {m_cached_data=false;}
  virtual bool IsAssessment(void) const { return false; }
  void Centroid(void) const;

  // GENERAL DATA ACCESS

  Efg &Game(void) const   { return const_cast<Efg &>(*m_efg); }
  const EFSupport &Support(void) const   { return *m_support; }
  const EFSupport *SupportPtr(void) const   { return m_support; }
  
  const T &GetRealizProb(const Node *node);
  const T &GetBeliefProb(const Node *node);
  const gVector<T> &GetNodeValue(const Node *node);
  const T &GetIsetProb(const Infoset *iset);
  const T &GetIsetValue(const Infoset *iset);
  const T &GetActionProb(const Action *act) const;
  const T &GetActionValue(const Action *act);
  const T &GetRegret(const Action *act);

  // TEST WHETHER PROFILE (RESTRICTED TO SUPPORT) EXTENDS TO ANF NASH, NASH

  bool ExtendsToANFNash(const EFSupport &, const EFSupport &, gStatus &) const;
  bool ExtendsToNash(const EFSupport &, const EFSupport &, gStatus &) const;
  
  // COMPUTATION OF INTERESTING QUANTITIES

  T Payoff(int p_player) const;
  void ComputeActionValues(Node *node, T prob);
  void ComputeActionValues(void);
  gDPVector<T> Beliefs(void);
  T LiapValue(void);
  T QreValue(const gVector<T> &lambda, bool &);
  T MaxRegret(void);

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
  void CondPayoff(BehavProfile<T>::BehavNode *, T,
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
