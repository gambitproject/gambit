//
// FILE: efg.h -- Declaration of extensive form data type
//
// $Id$
//

#ifndef EFG_H
#define EFG_H

#include "gstring.h"
#include "gblock.h"
#include "glist.h"

#include "gpvector.h"
#include "grblock.h"
#include "gnumber.h"

#include "gpolyctr.h"

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

class Efg     {

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
    gString title;
    gBlock<EFPlayer *> players;
    gBlock<EFOutcome *> outcomes;
    Node *root;
    EFPlayer *chance;
#ifndef EFG_ONLY
    mutable Nfg *afg;
#endif  EFG_ONLY
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
    void Payoff(Node *n, gNumber, const gArray<gArray<int> *> &, gVector<gNumber> &) const;

    void InfosetProbs(Node *n, gNumber, const gPVector<int> &, gPVector<gNumber> &) const;


// These are used in identification of subgames
    void MarkTree(Node *, Node *);
    bool CheckTree(Node *, Node *);
    void MarkSubgame(Node *, Node *);

  public:
    Efg(void);
    Efg(const Efg &, Node * = 0);

       //# DESTRUCTOR
    ~Efg();

       //# TITLE ACCESS AND MANIPULATION
    void SetTitle(const gString &s);
    const gString &GetTitle(void) const;

       //# WRITING DATA FILES
    void WriteEfgFile(gOutput &f) const;

       //# DATA ACCESS -- GENERAL INFORMATION
    bool IsConstSum(void) const;
    gNumber MinPayoff(int pl = 0) const;
    gNumber MaxPayoff(int pl = 0) const;

    Node *RootNode(void) const;
    bool IsSuccessor(const Node *n, const Node *from) const;
    bool IsPredecessor(const Node *n, const Node *of) const;

       //# DATA ACCESS -- PLAYERS
    int NumPlayers(void) const;
    EFPlayer *GetChance(void) const;
    EFPlayer *NewPlayer(void);
    const gArray<EFPlayer *> &Players(void) const  { return players; }

       //# DATA ACCESS -- OUTCOMES
    int NumOutcomes(void) const;
    const gArray<EFOutcome *> &Outcomes(void) const  { return outcomes; }
    EFOutcome *NewOutcome(void);
    void DeleteOutcome(EFOutcome *c);
 
       //# EDITING OPERATIONS
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

    Action *InsertAction(Infoset *s);
    Action *InsertAction(Infoset *s, Action *at);
    Infoset *DeleteAction(Infoset *s, Action *a);

    void Reveal(Infoset *, const gArray<EFPlayer *> &);

    void SetChanceProb(Infoset *, int, const gNumber &);
    gNumber GetChanceProb(Infoset *, int) const;
    gArray<gNumber> GetChanceProbs(Infoset *) const;

    void SetPayoff(EFOutcome *, int pl, const gNumber &value);
    gNumber Payoff(EFOutcome *, int pl) const;
    gVector<gNumber> Payoff(EFOutcome *) const;


    // Unmarks all subgames in the subtree rooted at n
    void UnmarkSubgames(Node *n);
    bool IsLegalSubgame(Node *n);

    // Mark all the (legal) subgames in the list
    void MarkSubgames(const gList<Node *> &list);
    bool DefineSubgame(Node *n);
    void RemoveSubgame(Node *n);


    int ProfileLength(void) const;
    gArray<int> NumInfosets(void) const;
    gPVector<int> NumActions(void) const;
    gPVector<int> NumMembers(void) const;

        //# COMPUTING VALUES OF PROFILES
    void Payoff(const gPVector<int> &profile, gVector<gNumber> &payoff) const;
    void Payoff(const gArray<gArray<int> *> &profile,
		gVector<gNumber> &payoff) const;

    void InfosetProbs(const gPVector<int> &profile, gPVector<gNumber> &prob) const;

    // defined in efgutils.cc
    friend void RandomEfg(Efg &);
#ifndef EFG_ONLY
    // This function put in to facilitate error-detection in MixedToBehav[]
    friend Nfg *AssociatedNfg(const Efg *E);
    friend Nfg *AssociatedAfg(const Efg *E);
#endif   // EFG_ONLY

#ifndef EFG_ONLY
    friend Nfg *MakeReducedNfg(const Efg &, const EFSupport &);
    friend Nfg *MakeAfg(const Efg &);
    friend void MixedToBehav(const Nfg &N, const MixedProfile<double> &mp,
		             const Efg &E, BehavProfile<double> &bp);
    friend void BehavToMixed(const Efg &, const BehavProfile<double> &,
			     const Nfg &, MixedProfile<double> &);
    friend void MixedToBehav(const Nfg &N, const MixedProfile<gRational> &mp,
		             const Efg &E, BehavProfile<gRational> &bp);
    friend void BehavToMixed(const Efg &, const BehavProfile<gRational> &,
			     const Nfg &, MixedProfile<gRational> &);
#endif   // EFG_ONLY
};

#include "behav.h"

#include "efplayer.h"
#include "infoset.h"
#include "node.h"
#include "outcome.h"

// These functions are provided in readefg.y/readefg.cc
int ReadEfgFile(gInput &, Efg *&);

template <class T> class PureBehavProfile   {
  protected:
    const Efg *E;
    gArray<gArray<Action *> *> profile;
    gPVector<T> *chanceprobs;
    gRectArray<T> *payoffs;

    void Payoff(Node *n, T, gArray<T> &) const;
    void InfosetProbs(Node *n, T, gPVector<T> &) const;

  public:
    PureBehavProfile(const Efg &);
    PureBehavProfile(const PureBehavProfile<T> &);
    ~PureBehavProfile();

    PureBehavProfile<T> &operator=(const PureBehavProfile<T> &);

    Efg &Game(void) const   { return const_cast< Efg& >( *E ); }

    T operator()(Action *) const;
    void Set(Action *);
    void Set(const EFPlayer *, const gArray<Action *> &);
    Action *Get(Infoset *) const;

    void Payoff(gArray<T> &payoff) const;
    void InfosetProbs(gPVector<T> &prob) const;
};


#endif   // EFG_H


