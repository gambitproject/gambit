//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// A class for representing and manipulating an extensive form game
//

#ifndef EFG_H
#define EFG_H

#include "base/base.h"
#include "math/gnumber.h"
#include "math/gpvector.h"

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

class efgOutcome;

class efgGame {
private:
  friend class EfgFileReader;
  friend class EfgFile;
  friend class Nfg;
  friend class Node;
  friend class BehavProfile<double>;
  friend class BehavProfile<gRational>;
  friend class BehavProfile<gNumber>;
  
protected:
  bool sortisets;
  mutable bool m_dirty;
  mutable long m_revision;
  mutable long m_outcome_revision;
  gText title, comment;
  gBlock<EFPlayer *> players;
  gBlock<efgOutcome *> outcomes;
  Node *root;
  EFPlayer *chance;
  mutable Nfg *afg;
  mutable Lexicon *lexicon;
  
  // this is for use with the copy constructor
  void CopySubtree(Node *, Node *);

  void CopySubtree(Node *, Node *, Node *);
  void MarkSubtree(Node *);
  void UnmarkSubtree(Node *);

  void SortInfosets(void);
  void NumberNodes(Node *, int &);
  
  void DeleteLexicon(void) const;

  efgOutcome *NewOutcome(int index);

  void WriteEfgFile(gOutput &, Node *) const;

  void Payoff(Node *n, gNumber, const gPVector<int> &, gVector<gNumber> &) const;
  void Payoff(Node *n, gNumber, const gArray<gArray<int> *> &, gArray<gNumber> &) const;
  
  void InfosetProbs(Node *n, gNumber, const gPVector<int> &, gPVector<gNumber> &) const;
    
    
  // These are used in identification of subgames
  void MarkTree(Node *, Node *);
  bool CheckTree(Node *, Node *);
  void MarkSubgame(Node *, Node *);

  // Recursive calls
  void DescendantNodes(const Node *, const EFSupport &, 
		       gList<Node *> &) const;
  void NonterminalDescendants(const Node *, const EFSupport&, 
			      gList<const Node*> &) const;
  void TerminalDescendants(const Node *, const EFSupport&, 
			   gList<Node *> &) const;

public:
  class Exception : public gException   {
  public:
    virtual ~Exception()   { }
    gText Description(void) const    { return "Efg error"; }
  };

  efgGame(void);
  efgGame(const efgGame &, Node * = 0);
  virtual ~efgGame();
  
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
  gArray<int> PathToNode(const Node *) const;
  //    const Node* Consequence(const Node&, Action&) const;
  gList<Node *> DescendantNodes(const Node&, const EFSupport&) const;
  gList<const Node*> NonterminalDescendants(const Node&, 
					    const EFSupport&) const;
  gList<Node *> TerminalDescendants(const Node&, 
				    const EFSupport&) const;
  gList<Node *> TerminalNodes(void) const;
  gList<Infoset*> DescendantInfosets(const Node&, const EFSupport&) const;
  const gArray<Node *> &Children(const Node *n) const;
  int NumChildren(const Node *n) const;

  // DATA ACCESS -- PLAYERS
  int NumPlayers(void) const;
  EFPlayer *GetChance(void) const;
  EFPlayer *NewPlayer(void);
  const gArray<EFPlayer *> &Players(void) const  { return players; }

  // DATA ACCESS -- INFOSETS
  gBlock<Infoset *> Infosets(void) const;

  // DATA ACCESS -- OUTCOMES
  int NumOutcomes(void) const;
  efgOutcome *GetOutcome(int index) const;
  efgOutcome *NewOutcome(void);
  void DeleteOutcome(efgOutcome *);

  efgOutcome *GetOutcome(const Node *const) const;
  void SetOutcome(Node *, efgOutcome *);
  
  void SetOutcomeName(efgOutcome *, const gText &);
  const gText &GetOutcomeName(efgOutcome *) const;
 
  // EDITING OPERATIONS
  Infoset *AppendNode(Node *n, EFPlayer *p, int br);
  Infoset *AppendNode(Node *n, Infoset *s);
  Node *DeleteNode(Node *n, Node *keep);
  Infoset *InsertNode(Node *n, EFPlayer *p, int br);
  Infoset *InsertNode(Node *n, Infoset *s);

  Infoset *CreateInfoset(EFPlayer *pl, int br);
  bool DeleteEmptyInfoset(Infoset *);
  void DeleteEmptyInfosets(void);
  Infoset *JoinInfoset(Infoset *s, Node *n);
  Infoset *LeaveInfoset(Node *n);
  Infoset *SplitInfoset(Node *n);
  Infoset *MergeInfoset(Infoset *to, Infoset *from);

  Infoset *SwitchPlayer(Infoset *s, EFPlayer *p);
  
  Node *CopyTree(Node *src, Node *dest);
  Node *MoveTree(Node *src, Node *dest);
  Node *DeleteTree(Node *n);

  Action *InsertAction(Infoset *s);
  Action *InsertAction(Infoset *s, const Action *at);
  Infoset *DeleteAction(Infoset *s, const Action *a);

  void Reveal(Infoset *, const gArray<EFPlayer *> &);

  void SetChanceProb(Infoset *, int, const gNumber &);
  gNumber GetChanceProb(Infoset *, int) const;
  gNumber GetChanceProb(const Action *) const;
  gArray<gNumber> GetChanceProbs(Infoset *) const;

  void SetPayoff(efgOutcome *, int pl, const gNumber &value);
  gNumber Payoff(efgOutcome *, const EFPlayer *) const;
  gNumber Payoff(const Node *, const EFPlayer *) const;
  gArray<gNumber> Payoff(efgOutcome *) const;

  void InitPayoffs(void) const;
  
  bool IsLegalSubgame(Node *n);
  void MarkSubgames(void);
  void MarkSubgames(const gList<Node *> &list);
  bool MarkSubgame(Node *n);
  void UnmarkSubgame(Node *n);
  void UnmarkSubgames(Node *n);

  int ProfileLength(void) const;
  int TotalNumInfosets(void) const;

  gArray<int>   NumInfosets(void) const;  // Does not include chance infosets
  int           NumPlayerInfosets(void) const;
  int           NumChanceInfosets(void) const;
  gPVector<int> NumActions(void) const;
  int           NumPlayerActions(void) const;
  int           NumChanceActions(void) const;
  gPVector<int> NumMembers(void) const;
  
  // COMPUTING VALUES OF PROFILES
  void Payoff(const gPVector<int> &profile, gVector<gNumber> &payoff) const;
  void Payoff(const gArray<gArray<int> *> &profile,
	      gArray<gNumber> &payoff) const;

  void InfosetProbs(const gPVector<int> &profile, gPVector<gNumber> &prob) const;
    
  Nfg *AssociatedNfg(void) const;
  Nfg *AssociatedAfg(void) const;
  Lexicon *GetLexicon(void) const { return lexicon; }

  friend Nfg *MakeReducedNfg(const EFSupport &);
  friend Nfg *MakeAfg(const efgGame &);

  // These are auxiliary functions used by the .efg file reader code
  Infoset *GetInfosetByIndex(EFPlayer *p, int index) const;
  Infoset *CreateInfosetByIndex(EFPlayer *p, int index, int br);
  efgOutcome *GetOutcomeByIndex(int index) const;
  efgOutcome *CreateOutcomeByIndex(int index);
  void Reindex(void);
  Infoset *CreateInfoset(int n, EFPlayer *pl, int br);
};

//#include "behav.h"

#include "efplayer.h"
#include "infoset.h"
#include "node.h"

class efgOutcome   {
friend class efgGame;
friend class BehavProfile<double>;
friend class BehavProfile<gRational>;
friend class BehavProfile<gNumber>;
protected:
  efgGame *m_efg; 
  int m_number;
  gText m_name;
  gBlock<gNumber> m_payoffs;
  gBlock<gNumber> m_doublePayoffs;

  efgOutcome(efgGame *p_efg, int p_number)
    : m_efg(p_efg), m_number(p_number), 
      m_payoffs(p_efg->NumPlayers()), m_doublePayoffs(p_efg->NumPlayers())
    { }
  ~efgOutcome()  { }

public:
  efgGame *GetGame(void) const { return m_efg; }
};

efgGame *ReadEfgFile(gInput &);

template <class T> class PureBehavProfile   {
  protected:
    const efgGame *E;
    gArray<gArray<const Action *> *> profile;

    //    void IndPayoff(const Node *n, const int &pl, const T, T &) const;
    // This aims at efficiency, but leads to a problem described in behav.imp

    void Payoff(const Node *n, const T, gArray<T> &) const;
    void InfosetProbs(Node *n, T, gPVector<T> &) const;

  public:
    PureBehavProfile(const efgGame &);
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
    
    const T Payoff(efgOutcome *, const int &pl) const;
    const T ChanceProb(const Infoset *, const int &act) const;
    
    const T Payoff(const Node *, const int &pl) const;
  //    T    Payoff(const int &pl) const;
    void Payoff(gArray<T> &payoff) const;
    void InfosetProbs(gPVector<T> &prob) const;
    efgGame &GetGame(void) const   { return const_cast<efgGame &>(*E); }
};


#include "efgutils.h"

#endif   // EFG_H


