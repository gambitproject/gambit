//
// FILE: efg.h -- Declaration of extensive form data type
//
// $Id$
//

#ifndef EFG_H
#define EFG_H

#include "gtext.h"
#include "gblock.h"
#include "glist.h"

#include "gpvector.h"
#include "grblock.h"
#include "gnumber.h"

class EFOutcome;
class EFPlayer;
class Infoset;
class Node;
class Action;
class EFSupport;
class Lexicon;
class Nfg;
template <class T> class BehavProfile;
template <class T> class MixedProfile;
template <class T> class PureBehavProfile;

class Efg {
public:
  class Exception : public gException   {
  public:
    virtual ~Exception()   { }
    gText Description(void) const    { return "Efg error"; }
  };

  virtual ~Efg() { }

  virtual const gText &GetTitle(void) const = 0;
  virtual long RevisionNumber(void) const = 0;

  virtual int NumPlayers(void) const = 0;
  virtual const EFPlayer *GetPlayer(const int pl) const = 0;
  virtual const gArray<EFPlayer *> &Players(void) const = 0; 

  virtual int ProfileLength(void) const = 0;
  virtual int TotalNumInfosets(void) const = 0;

  virtual gArray<int> NumInfosets(void) const = 0;
  virtual int NumPlayerInfosets(void) const = 0;
  virtual int NumChanceInfosets(void) const = 0;
  virtual int NumPlayersInfosets(const int pl) const = 0;
  virtual gPVector<int> NumActions(void) const = 0;
  virtual int NumPlayerActions(void) const = 0;
  virtual int NumChanceActions(void) const = 0;
  virtual int NumActionsAtInfoset(const int pl, const int iset) const = 0;
  virtual int NumNodesInInfoset(const int pl, const int iset) const = 0;
  virtual gPVector<int> NumMembers(void) const = 0;
  virtual Infoset *GetInfosetByIndex(const int&, const int&) const = 0;

  virtual gNumber GetChanceProb(Infoset *, int) const = 0;
  virtual gNumber GetChanceProb(const Action *) const = 0;

  virtual Node *RootNode(void) const = 0;
  virtual bool IsSuccessor(const Node *n, const Node *from) const = 0;
  virtual bool IsPredecessor(const Node *n, const Node *of) const = 0;
  virtual gList<const Node*> TerminalNodes(void) const = 0;  

  virtual void UnmarkSubgames(Node *n) = 0;
  virtual bool IsLegalSubgame(Node *n) = 0;
  virtual void MarkSubgames(const gList<Node *> &list) = 0;
  virtual bool DefineSubgame(Node *n) = 0;
  virtual void RemoveSubgame(Node *n) = 0;

  virtual gNumber MinPayoff(int pl = 0) const = 0;
  virtual gNumber MaxPayoff(int pl = 0) const = 0;
  virtual bool IsConstSum(void) const = 0;
  virtual void InitPayoffs(void) const = 0;
  virtual gNumber Payoff(const EFOutcome *, int pl) const = 0;
  virtual gArray<gNumber> Payoff(const EFOutcome *) const = 0;
  virtual void Payoff(const gPVector<int> &profile,
		      gVector<gNumber> &payoff) const = 0;
  virtual void Payoff(const gArray<gArray<int> *> &profile,
		      gArray<gNumber> &payoff) const = 0;

  virtual Nfg *AssociatedNfg(void) const = 0;
  virtual Nfg *AssociatedAfg(void) const = 0;
  virtual Lexicon *GetLexicon(void) const = 0;
};

class FullEfg : public Efg  {
private:
  // this is used to track memory leakage; #define MEMCHECK to use it
#ifdef MEMCHECK
  static int _NumObj;
#endif   // MEMCHECK

friend class EfgFileReader;
friend class EfgFile;
friend class Nfg;

protected:
  bool sortisets;
  mutable bool m_dirty;
  mutable long m_revision;
  mutable long m_outcome_revision;
  gText title, comment;
  gBlock<EFPlayer *> players;
  gBlock<EFOutcome *> outcomes;
  Node *root;
  EFPlayer *chance;
  mutable Nfg *afg;
  mutable Lexicon *lexicon;

  // this is for use with the copy constructor
  void CopySubtree(Node *, Node *);

  void CopySubtree(Node *, Node *, Node *);
  void MarkSubtree(Node *);
  void UnmarkSubtree(Node *);

  Infoset *CreateInfoset(int n, EFPlayer *pl, int br);
  
  void SortInfosets(void);
  void NumberNodes(Node *, int &);
  
// These are auxiliary functions used by the .efg file reader code
  Infoset *GetInfosetByIndex(EFPlayer *p, int index) const;
  Infoset *CreateInfosetByIndex(EFPlayer *p, int index, int br);
  EFOutcome *GetOutcomeByIndex(int index) const;
  EFOutcome *CreateOutcomeByIndex(int index);
  void Reindex(void);

  void DeleteLexicon(void) const;

  EFOutcome *NewOutcome(int index);

  void WriteEfgFile(gOutput &, Node *) const;

  void Payoff(Node *n, gNumber, const gPVector<int> &, gVector<gNumber> &) const;
  void Payoff(Node *n, gNumber, const gArray<gArray<int> *> &, gArray<gNumber> &) const;
  
  void InfosetProbs(Node *n, gNumber, const gPVector<int> &, gPVector<gNumber> &) const;


// These are used in identification of subgames
  void MarkTree(Node *, Node *);
  bool CheckTree(Node *, Node *);
  void MarkSubgame(Node *, Node *);

public:
  FullEfg(void);
  FullEfg(const FullEfg &, Node * = 0);
  virtual ~FullEfg();

  // TITLE ACCESS AND MANIPULATION
  void SetTitle(const gText &s);
  const gText &GetTitle(void) const;

  void SetComment(const gText &);
  const gText &GetComment(void) const;

  // WRITING DATA FILES
  void WriteEfgFile(gOutput &p_file, int p_nDecimals) const;

  // DATA ACCESS -- GENERAL INFORMATION
  bool IsConstSum(void) const; 
  long RevisionNumber(void) const { return m_revision; }
  bool IsDirty(void) const { return m_dirty; }
  void SetIsDirty(bool p_dirty) { m_dirty = p_dirty; }
  gNumber MinPayoff(int pl = 0) const;
  gNumber MaxPayoff(int pl = 0) const;
  
  Node *RootNode(void) const;
  bool IsSuccessor(const Node *n, const Node *from) const;
  bool IsPredecessor(const Node *n, const Node *of) const;
  //    const Node* Consequence(const Node&, Action&) const;
  gList<Node*> Children(const Node&) const;
  void DescendantNodesRECURSION(const Node*, 
				const EFSupport&, 
				gList<const Node*>&) const;
  void NonterminalDescendantsRECURSION(const Node*, 
				       const EFSupport&, 
				       gList<const Node*>&) const;
  void TerminalDescendantsRECURSION(const Node*, 
				    const EFSupport&, 
				    gList<const Node*>&) const;
  gList<const Node*> DescendantNodes(const Node&, const EFSupport&) const;
  gList<const Node*> NonterminalDescendants(const Node&, 
					    const EFSupport&) const;
  gList<const Node*> TerminalDescendants(const Node&, 
					 const EFSupport&) const;
  gList<const Node*> TerminalNodes(void) const;
  gList<Infoset*> DescendantInfosets(const Node&, const EFSupport&) const;

  // DATA ACCESS -- PLAYERS
  int NumPlayers(void) const;
  EFPlayer *GetChance(void) const;
  EFPlayer *NewPlayer(void);
  const EFPlayer *GetPlayer(const int pl) const { return players[pl]; }
  const gArray<EFPlayer *> &Players(void) const  { return players; }

  // DATA ACCESS -- INFOSETS
  Infoset *GetInfosetByIndex(const int&, const int&) const;
  gBlock<Infoset *> Infosets() const;

  // DATA ACCESS -- OUTCOMES
  int NumOutcomes(void) const;
  const gArray<EFOutcome *> &Outcomes(void) const  { return outcomes; }
  EFOutcome *NewOutcome(void);
  void DeleteOutcome(EFOutcome *c);
 
  // EDITING OPERATIONS
  Infoset *AppendNode(Node *n, EFPlayer *p, int br);
  Infoset *AppendNode(Node *n, Infoset *s);
  Node *DeleteNode(Node *n, Node *keep);
  Infoset *InsertNode(Node *n, EFPlayer *p, int br);
  Infoset *InsertNode(Node *n, Infoset *s);

  Infoset *CreateInfoset(EFPlayer *pl, int br);
  bool DeleteEmptyInfoset(Infoset *);
  Infoset *JoinInfoset(Infoset *s, Node *n);
  Infoset *LeaveInfoset(Node *n);
  Infoset *SplitInfoset(Node *n);
  Infoset *MergeInfoset(Infoset *to, Infoset *from);

  Infoset *SwitchPlayer(Infoset *s, EFPlayer *p);
  
  Node *CopyTree(Node *src, Node *dest);
  Node *MoveTree(Node *src, Node *dest);
  Node *DeleteTree(Node *n);

  const Action *InsertAction(Infoset *s);
  const Action *InsertAction(Infoset *s, const Action *at);
  Infoset *DeleteAction(Infoset *s, const Action *a);

  void Reveal(Infoset *, const gArray<EFPlayer *> &);

  void SetChanceProb(Infoset *, int, const gNumber &);
  gNumber GetChanceProb(Infoset *, int) const;
  gNumber GetChanceProb(const Action *) const;
  gArray<gNumber> GetChanceProbs(Infoset *) const;

  void SetPayoff(EFOutcome *, int pl, const gNumber &value);
  gNumber Payoff(const EFOutcome *, int pl) const;
  gArray<gNumber> Payoff(const EFOutcome *) const;

  void InitPayoffs(void) const;
  
  // Unmarks all subgames in the subtree rooted at n
  void UnmarkSubgames(Node *n);
  bool IsLegalSubgame(Node *n);

  // Mark all the (legal) subgames in the list
  void MarkSubgames(const gList<Node *> &list);
  bool DefineSubgame(Node *n);
  void RemoveSubgame(Node *n);

  int ProfileLength(void) const;
  int TotalNumInfosets(void) const;

  gArray<int>   NumInfosets(void) const;  // Does not include chance infosets
  int           NumPlayerInfosets(void) const;
  int           NumChanceInfosets(void) const;
  int           NumPlayersInfosets(const int pl) const; // pl ==0 is chance
  gPVector<int> NumActions(void) const;
  int           NumPlayerActions(void) const;
  int           NumChanceActions(void) const;
  int           NumActionsAtInfoset(const int pl, const int iset) const;
  int           NumNodesInInfoset(const int pl, const int iset) const;
  gPVector<int> NumMembers(void) const;
  
  //# COMPUTING VALUES OF PROFILES
  void Payoff(const gPVector<int> &profile, gVector<gNumber> &payoff) const;
  void Payoff(const gArray<gArray<int> *> &profile,
	      gArray<gNumber> &payoff) const;

  void InfosetProbs(const gPVector<int> &profile, gPVector<gNumber> &prob) const;

  // defined in efgutils.cc
  friend void RandomEfg(FullEfg &);

  Nfg *AssociatedNfg(void) const;
  Nfg *AssociatedAfg(void) const;
  Lexicon *GetLexicon(void) const { return lexicon; }

  friend Nfg *MakeReducedNfg(const EFSupport &);
  friend Nfg *MakeAfg(const Efg &);
};

#include "behav.h"

#include "efplayer.h"
#include "infoset.h"
#include "node.h"
#include "outcome.h"

// These functions are provided in readefg.y/readefg.cc
int ReadEfgFile(gInput &, FullEfg *&);

template <class T> class PureBehavProfile   {
  protected:
    const Efg *E;
    gArray<gArray<const Action *> *> profile;

    //    void IndPayoff(const Node *n, const int &pl, const T, T &) const;
    // This aims at efficiency, but leads to a problem described in behav.imp

    void Payoff(const Node *n, const T, gArray<T> &) const;
    void InfosetProbs(Node *n, T, gPVector<T> &) const;

  public:
    PureBehavProfile(const Efg &);
    PureBehavProfile(const PureBehavProfile<T> &);
    ~PureBehavProfile();

    // Operators
    PureBehavProfile<T> &operator=(const PureBehavProfile<T> &);
    T operator()(Action *) const;

    // Manipulation
    void Set(const Action *);
    void Set(const EFPlayer *, const gArray<const Action *> &);

    // Information
    const Action *GetAction(const Infoset *) const;
    
    const T Payoff(const EFOutcome *, const int &pl) const;
    const T ChanceProb(const Infoset *, const int &act) const;
    
    const T Payoff(const Node *, const int &pl) const;
  //    T    Payoff(const int &pl) const;
    void Payoff(gArray<T> &payoff) const;
    void InfosetProbs(gPVector<T> &prob) const;
    Efg &Game(void) const   { return const_cast< Efg& >( *E ); }
};


#include "efgutils.h"


#endif   // EFG_H


