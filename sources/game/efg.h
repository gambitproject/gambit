//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to extensive form representation class
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#ifndef EFG_H
#define EFG_H

#include "base/base.h"
#include "math/gnumber.h"
#include "math/gpvector.h"

class EFSupport;
class Lexicon;
class Nfg;
template <class T> class BehavProfile;
template <class T> class MixedProfile;
template <class T> class PureBehavProfile;

#include "outcome.h"
#include "infoset.h"
#include "efplayer.h"
#include "node.h"

class efgGame {
private:
  friend class EfgFileReader;
  friend class EfgFile;
  friend class Nfg;
  friend class BehavProfile<double>;
  friend class BehavProfile<gRational>;
  friend class BehavProfile<gNumber>;
  
protected:
  bool sortisets;
  mutable bool m_dirty;
  mutable long m_revision;
  mutable long m_outcome_revision;
  gText title, comment;
  gBlock<gbt_efg_player_rep *> players;
  gBlock<gbt_efg_outcome_rep *> outcomes;
  gbt_efg_node_rep *root;
  gbt_efg_player_rep *chance;
  mutable Nfg *afg;
  mutable Lexicon *lexicon;
  
  // this is for use with the copy constructor
  void CopySubtree(gbt_efg_node_rep *, gbt_efg_node_rep *);

  void CopySubtree(gbt_efg_node_rep *, gbt_efg_node_rep *,
		   gbt_efg_node_rep *);
  void MarkSubtree(gbt_efg_node_rep *);
  void UnmarkSubtree(gbt_efg_node_rep *);

  void SortInfosets(void);
  void NumberNodes(gbt_efg_node_rep *, int &);
  
  void DeleteLexicon(void) const;

  gbtEfgOutcome NewOutcome(int index);

  void WriteEfgFile(gOutput &, gbt_efg_node_rep *) const;

  void Payoff(gbt_efg_node_rep *n, gNumber,
	      const gPVector<int> &, gVector<gNumber> &) const;
  void Payoff(gbt_efg_node_rep *n, gNumber,
	      const gArray<gArray<int> *> &, gArray<gNumber> &) const;
  
  void InfosetProbs(gbt_efg_node_rep *n, gNumber,
		    const gPVector<int> &, gPVector<gNumber> &) const;
    
    
  // These are used in identification of subgames
  void MarkTree(gbt_efg_node_rep *, gbt_efg_node_rep *);
  bool CheckTree(gbt_efg_node_rep *, gbt_efg_node_rep *);
  void MarkSubgame(gbt_efg_node_rep *, gbt_efg_node_rep *);

  gbt_efg_infoset_rep *CreateInfoset(int n, gbtEfgPlayer, int br);

public:
  class Exception : public gException   {
  public:
    virtual ~Exception()   { }
    gText Description(void) const    { return "Efg error"; }
  };

  efgGame(void);
  efgGame(const efgGame &, gbtEfgNode = gbtEfgNode(0));
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
  
  gbtEfgNode RootNode(void) const;
  bool IsSuccessor(const gbtEfgNode &n, const gbtEfgNode &from) const;
  bool IsPredecessor(const gbtEfgNode &n, gbtEfgNode of) const;

  // DATA ACCESS -- PLAYERS
  int NumPlayers(void) const;
  gbtEfgPlayer GetChance(void) const;
  gbtEfgPlayer NewPlayer(void);
  gbtEfgPlayer GetPlayer(int index) const;

  // DATA ACCESS -- OUTCOMES
  int NumOutcomes(void) const;
  gbtEfgOutcome GetOutcome(int p_id) const;
  gbtEfgOutcome NewOutcome(void);
  void DeleteOutcome(gbtEfgOutcome &);

  // EDITING OPERATIONS
  gbtEfgInfoset AppendNode(gbtEfgNode n, gbtEfgPlayer, int br);
  gbtEfgInfoset AppendNode(gbtEfgNode n, gbtEfgInfoset s);
  gbtEfgNode DeleteNode(gbtEfgNode n, gbtEfgNode keep);
  gbtEfgInfoset InsertNode(gbtEfgNode n, gbtEfgPlayer, int br);
  gbtEfgInfoset InsertNode(gbtEfgNode n, gbtEfgInfoset s);

  gbtEfgInfoset CreateInfoset(gbtEfgPlayer, int br);
  bool DeleteEmptyInfoset(gbtEfgInfoset);
  void DeleteEmptyInfosets(void);
  gbtEfgInfoset JoinInfoset(gbtEfgInfoset, gbtEfgNode);
  gbtEfgInfoset LeaveInfoset(gbtEfgNode);
  gbtEfgInfoset SplitInfoset(gbtEfgNode);
  gbtEfgInfoset MergeInfoset(gbtEfgInfoset to, gbtEfgInfoset from);

  gbtEfgInfoset SwitchPlayer(gbtEfgInfoset s, gbtEfgPlayer p);
  
  gbtEfgNode CopyTree(gbtEfgNode src, gbtEfgNode dest);
  gbtEfgNode MoveTree(gbtEfgNode src, gbtEfgNode dest);
  gbtEfgNode DeleteTree(gbtEfgNode n);

  gbtEfgAction InsertAction(gbtEfgInfoset);
  gbtEfgAction InsertAction(gbtEfgInfoset, const gbtEfgAction &at);
  gbtEfgInfoset DeleteAction(gbtEfgInfoset s, const gbtEfgAction &a);

  void Reveal(gbtEfgInfoset, gbtEfgPlayer);

  void SetChanceProb(gbtEfgInfoset, int, const gNumber &);
  gNumber GetChanceProb(gbtEfgInfoset, int) const;
  gNumber GetChanceProb(const gbtEfgAction &) const;

  void SetPayoff(gbtEfgOutcome, int pl, const gNumber &value);
  gNumber Payoff(const gbtEfgOutcome &, const gbtEfgPlayer &) const;
  gNumber Payoff(const gbtEfgNode &, const gbtEfgPlayer &) const;
  gArray<gNumber> Payoff(const gbtEfgOutcome &) const;

  bool IsLegalSubgame(const gbtEfgNode &);
  void MarkSubgames(void);
  bool MarkSubgame(gbtEfgNode);
  void UnmarkSubgame(gbtEfgNode);
  void UnmarkSubgames(gbtEfgNode);

  int ProfileLength(void) const;
  int TotalNumInfosets(void) const;

  gArray<int>   NumInfosets(void) const;  // Does not include chance infosets
  int           NumPlayerInfosets(void) const;
  int           NumChanceInfosets(void) const;
  gPVector<int> NumActions(void) const;
  int           NumPlayerActions(void) const;
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
  gbtEfgInfoset GetInfosetByIndex(gbtEfgPlayer, int index) const;
  gbtEfgInfoset CreateInfosetByIndex(gbtEfgPlayer, int index, int br);
  gbtEfgOutcome GetOutcomeByIndex(int index) const;
  gbtEfgOutcome CreateOutcomeByIndex(int index);
  void Reindex(void);
};

//#include "behav.h"

efgGame *ReadEfgFile(gInput &);

template <class T> class PureBehavProfile   {
  protected:
    const efgGame *E;
    gArray<gArray<gbtEfgAction> *> profile;

    //    void IndPayoff(const Node *n, const int &pl, const T, T &) const;
    // This aims at efficiency, but leads to a problem described in behav.imp

    void Payoff(const gbtEfgNode &n, const T, gArray<T> &) const;
    void InfosetProbs(const gbtEfgNode &n, T, gPVector<T> &) const;

  public:
    PureBehavProfile(const efgGame &);
    PureBehavProfile(const PureBehavProfile<T> &);
    ~PureBehavProfile();

    // Operators
    PureBehavProfile<T> &operator=(const PureBehavProfile<T> &);
    T operator()(const gbtEfgAction &) const;

    // Manipulation
    void Set(const gbtEfgAction &);
    //    void Set(const gbtEfgPlayer &, const gArray<const Action *> &);

    // Information
    gbtEfgAction GetAction(const gbtEfgInfoset &) const;
    
    const T Payoff(const gbtEfgOutcome &, const int &pl) const;
    const T ChanceProb(const gbtEfgInfoset &, const int &act) const;
    
    const T Payoff(const gbtEfgNode &, const int &pl) const;
  //    T    Payoff(const int &pl) const;
    void Payoff(gArray<T> &payoff) const;
    void InfosetProbs(gPVector<T> &prob) const;
    efgGame &GetGame(void) const   { return const_cast<efgGame &>(*E); }
};


#include "efgutils.h"

#endif   // EFG_H


