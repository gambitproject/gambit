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

#include "gpvector.h"
#include "gdpvect.h"
#include "game.h"


class gbtEfgGame;
class gbtEfgPlayer;
class gbtEfgInfoset;
class gbtEfgNode;
class gbtEfgAction;
class gbtEfgSupport;
class gbtNfgGame;
class gbtNumber;

template <class T> class gbtBehavProfile;
template <class T> class gbtMixedProfile;

class gbtEfgException : public gbtException   {
public:
  virtual ~gbtEfgException()   { }
  std::string GetDescription(void) const  
    { return "Internal error in extensive form representation"; }
};


class gbtEfgOutcome : public gbtGameObject {
  friend class gbtEfgGame;
  friend class gbtBehavProfile<double>;
  friend class gbtBehavProfile<gbtRational>;
  friend class gbtBehavProfile<gbtNumber>;
protected:
  gbtEfgGame *m_efg; 
  int m_number;
  std::string m_label;
  gbtArray<std::string> m_textPayoffs;
  gbtArray<gbtRational> m_ratPayoffs;
  gbtArray<double> m_doublePayoffs;

  gbtEfgOutcome(gbtEfgGame *p_efg, int p_number);
  ~gbtEfgOutcome()  { }

public:
  gbtEfgGame *GetGame(void) const { return m_efg; }
  int GetNumber(void) const { return m_number; }
  
  const std::string &GetLabel(void) const { return m_label; }
  void SetLabel(const std::string &p_label) { m_label = p_label; }

  void SetPayoff(int pl, const std::string &value);
  const gbtRational GetPayoff(int pl) const  { return m_ratPayoffs[pl]; }
  const std::string &GetPayoffText(int pl) const { return m_textPayoffs[pl]; }
};

class gbtEfgAction : public gbtGameObject {
  friend class gbtEfgGame;
  friend class gbtBehavProfile<double>;
  friend class gbtBehavProfile<gbtRational>;
  friend class gbtBehavProfile<gbtNumber>;
  friend class gbtEfgInfoset;
private:
  int m_number;
  std::string m_label;
  gbtEfgInfoset *m_infoset;

  gbtEfgAction(int p_number, const std::string &p_label, 
	       gbtEfgInfoset *p_infoset)
    : m_number(p_number), m_label(p_label), m_infoset(p_infoset) { }
  ~gbtEfgAction()   { }

public:
  int GetNumber(void) const { return m_number; }
  gbtEfgInfoset *GetInfoset(void) const { return m_infoset; }

  const std::string &GetLabel(void) const { return m_label; }
  void SetLabel(const std::string &p_label) { m_label = p_label; }

  bool Precedes(const gbtEfgNode *) const;
};

class gbtEfgInfoset : public gbtGameObject {
  friend class gbtEfgGame;
  friend class gbtEfgPlayer;
  friend class gbtEfgNode;
  friend class gbtBehavProfile<double>;
  friend class gbtBehavProfile<gbtRational>;
  friend class gbtBehavProfile<gbtNumber>;
  friend void MakeStrategy(gbtEfgPlayer *);
  friend void MakeReducedStrats(gbtEfgPlayer *p, gbtEfgNode *n, gbtEfgNode *nn);

protected:
  gbtEfgGame *m_efg;
  int m_number;
  std::string m_label;
  gbtEfgPlayer *m_player;
  gbtArray<gbtEfgAction *> m_actions;
  gbtArray<gbtEfgNode *> m_members;
  int flag, whichbranch;
  gbtArray<std::string> m_textProbs;
  gbtArray<gbtRational> m_ratProbs;
  
  gbtEfgInfoset(gbtEfgGame *p_efg, int p_number, gbtEfgPlayer *p_player, 
		int p_actions);
  ~gbtEfgInfoset();  

public:
  gbtEfgGame *GetGame(void) const { return m_efg; }
  int GetNumber(void) const { return m_number; }
  
  gbtEfgPlayer *GetPlayer(void) const { return m_player; }
  bool IsChanceInfoset(void) const;

  void SetLabel(const std::string &p_label) { m_label = p_label; }
  const std::string &GetLabel(void) const { return m_label; }
  
  gbtEfgAction *InsertAction(int where);
  void RemoveAction(int which);

  int NumActions(void) const { return m_actions.Length(); }
  gbtEfgAction *GetAction(int p_index) const { return m_actions[p_index]; }

  int NumMembers(void) const { return m_members.Length(); }
  gbtEfgNode *GetMember(int p_index) const { return m_members[p_index]; }

  bool Precedes(gbtEfgNode *) const;

  void SetActionProb(int i, const std::string &p_value);
  const gbtRational &GetActionProb(int i) const { return m_ratProbs[i]; }
  const std::string &GetActionProbText(int i) const { return m_textProbs[i]; }
};

class gbtEfgPlayer : public gbtGameObject  {
  friend class gbtEfgGame;
  friend class gbtBehavProfile<double>;
  friend class gbtBehavProfile<gbtRational>;
  friend class gbtBehavProfile<gbtNumber>;
  friend class gbtMixedProfile<double>;
  friend class gbtMixedProfile<gbtRational>;
  friend class gbtMixedProfile<gbtNumber>;
  friend void MakeStrategy(gbtEfgPlayer *);

private:
  gbtEfgGame *m_efg;
  int m_number;
  std::string m_label;
  gbtArray<gbtEfgInfoset *> m_infosets;
  gbtList<gbtArray<int> > m_strategies;

  gbtEfgPlayer(gbtEfgGame *p_efg, int p_id) : m_number(p_id), m_efg(p_efg)  { }
  ~gbtEfgPlayer();

public:
  int GetNumber(void) const { return m_number; }
  gbtEfgGame *GetGame(void) const { return m_efg; }
  
  const std::string &GetLabel(void) const { return m_label; }
  void SetLabel(const std::string &p_label) { m_label = p_label; }
  
  bool IsChance(void) const { return (m_number == 0); }

  int NumInfosets(void) const { return m_infosets.Length(); }
  gbtEfgInfoset *GetInfoset(int p_index) const { return m_infosets[p_index]; }
};

class gbtEfgNode : public gbtGameObject {
  friend class gbtEfgGame;
  friend class gbtBehavProfile<double>;
  friend class gbtBehavProfile<gbtRational>;
  friend class gbtBehavProfile<gbtNumber>;
  friend void MakeReducedStrats(gbtEfgPlayer *p, gbtEfgNode *n, gbtEfgNode *nn);
  
protected:
  bool mark;
  int number; 
  gbtEfgGame *m_efg;
  std::string m_label;
  gbtEfgInfoset *infoset;
  gbtEfgNode *parent;
  gbtEfgOutcome *outcome;
  gbtArray<gbtEfgNode *> children;
  gbtEfgNode *whichbranch, *ptr;

  gbtEfgNode(gbtEfgGame *e, gbtEfgNode *p);
  ~gbtEfgNode();

  void DeleteOutcome(gbtEfgOutcome *outc);

public:
  gbtEfgGame *GetGame(void) const { return m_efg; }

  const std::string &GetLabel(void) const { return m_label; } 
  void SetLabel(const std::string &p_label) { m_label = p_label; }

  int GetNumber(void) const { return number; }
  int NumberInInfoset(void) const
    { return infoset->m_members.Find(const_cast<gbtEfgNode *>(this)); }

  int NumChildren(void) const    { return children.Length(); }

  gbtEfgInfoset *GetInfoset(void) const   { return infoset; }
  bool IsTerminal(void) const { return (children.Length() == 0); }
  gbtEfgPlayer *GetPlayer(void) const
    { return (infoset) ? infoset->GetPlayer() : 0; }
  gbtEfgAction *GetPriorAction(void) const; // returns null if root node
  gbtEfgNode *GetChild(int i) const    { return children[i]; }
  gbtEfgNode *GetParent(void) const    { return parent; }
  gbtEfgNode *GetNextSibling(void) const;
  gbtEfgNode *GetPriorSibling(void) const;

  gbtEfgOutcome *GetOutcome(void) const { return outcome; }
  void SetOutcome(gbtEfgOutcome *p_outcome);

  bool IsSuccessorOf(gbtEfgNode *from) const;
  bool IsSubgameRoot(void) const;
};

class gbtEfgGame : public gbtGame {
private:
  friend class EfgFileReader;
  friend class EfgFile;
  friend class gbtNfgGame;
  friend class gbtEfgNode;
  friend class gbtEfgOutcome;
  friend class gbtEfgInfoset;
  friend class gbtBehavProfile<double>;
  friend class gbtBehavProfile<gbtRational>;
  friend class gbtBehavProfile<gbtNumber>;
  friend class gbtMixedProfile<double>;
  friend class gbtMixedProfile<gbtRational>;
  friend class gbtMixedProfile<gbtNumber>;
  
protected:
  bool sortisets;
  std::string title, comment;
  gbtArray<gbtEfgPlayer *> players;
  gbtArray<gbtEfgOutcome *> outcomes;
  gbtEfgNode *m_root;
  gbtEfgPlayer *chance;
  mutable gbtNfgGame *m_reducedNfg;
  
  // this is for use with the copy constructor
  void CopySubtree(gbtEfgNode *, gbtEfgNode *);

  void CopySubtree(gbtEfgNode *, gbtEfgNode *, gbtEfgNode *);
  void MarkSubtree(gbtEfgNode *);
  void UnmarkSubtree(gbtEfgNode *);

  void SortInfosets(void);
  void NumberNodes(gbtEfgNode *, int &);
  
  void DeleteLexicon(void) const;

  gbtEfgOutcome *NewOutcome(int index);

  void WriteEfgFile(std::ostream &, gbtEfgNode *) const;

  void Payoff(gbtEfgNode *n, gbtRational, const gbtPVector<int> &, gbtVector<gbtRational> &) const;
  void Payoff(gbtEfgNode *n, gbtRational, const gbtArray<gbtArray<int> > &, gbtArray<gbtRational> &) const;
  
  void InfosetProbs(gbtEfgNode *n, gbtRational, const gbtPVector<int> &, gbtPVector<gbtRational> &) const;
    
    
  // These are used in identification of subgames
  void MarkTree(gbtEfgNode *, gbtEfgNode *);
  bool CheckTree(gbtEfgNode *, gbtEfgNode *);

  // Recursive calls
  void DescendantNodes(const gbtEfgNode *, const gbtEfgSupport &, 
		       gbtList<gbtEfgNode *> &) const;
  void NonterminalDescendants(const gbtEfgNode *, const gbtEfgSupport&, 
			      gbtList<const gbtEfgNode*> &) const;
  void TerminalDescendants(const gbtEfgNode *, const gbtEfgSupport&, 
			   gbtList<gbtEfgNode *> &) const;

  gbtEfgInfoset *CreateInfoset(int n, gbtEfgPlayer *pl, int br);

public:
  /// @name Lifecycle
  //@{
  /// Construct a new trivial extensive game
  gbtEfgGame(void);
  /// Create a copy of the extensive game, starting at the specified node
  gbtEfgGame(const gbtEfgGame &, gbtEfgNode * = 0);
  /// Clean up the extensive game
  ~gbtEfgGame();
  //@}

  /// @name General data access
  //@{
  /// Get the text label associated with the game
  const std::string &GetTitle(void) const;
  /// Set the text label associated with the game
  void SetTitle(const std::string &s);

  /// Get the text comment associated with the game
  const std::string &GetComment(void) const;
  /// Set the text comment associated with the game
  void SetComment(const std::string &);

  /// Returns true if the game is constant-sum
  bool IsConstSum(void) const; 
  /// Returns the smallest payoff in any outcome of the game
  gbtRational GetMinPayoff(int pl = 0) const;
  /// Returns the largest payoff in any outcome of the game
  gbtRational GetMaxPayoff(int pl = 0) const;

  /// Returns true if the game is perfect recall.  If not, the specified
  /// a pair of violating information sets is returned in the parameters.  
  bool IsPerfectRecall(gbtEfgInfoset *&, gbtEfgInfoset *&) const;
  /// Returns true if the game is perfect recall
  bool IsPerfectRecall(void) const
    { gbtEfgInfoset *s, *t; return IsPerfectRecall(s, t); }

  /// Returns the reduced normal form representation of the game,
  /// if computed.
  gbtNfgGame *AssociatedNfg(void) const { return m_reducedNfg; }

  /// Builds the reduced normal form representation of the game
  gbtNfgGame *MakeReducedNfg(void);
  //@}

  /// @name Writing data files
  //@{
  /// Write the game in .efg format to the specified stream
  void WriteEfgFile(std::ostream &p_file) const;
  //@}

  /// @name Players
  //@{
  /// Returns the number of players in the game
  int NumPlayers(void) const;
  /// Returns the pl'th player in the game
  gbtEfgPlayer *GetPlayer(int pl) const { return players[pl]; }
  /// Returns the chance (nature) player
  gbtEfgPlayer *GetChance(void) const;
  /// Creates a new player in the game, with no moves
  gbtEfgPlayer *NewPlayer(void);
  //@}

  /// @name Nodes
  //@{
  /// Returns the root node of the game
  gbtEfgNode *GetRoot(void) const { return m_root; }
  /// Returns the number of nodes in the game
  int NumNodes(void) const;
  //@}

  /// @name Information sets
  //@{
  /// Returns the iset'th information set in the game (numbered globally)
  gbtEfgInfoset *GetInfoset(int iset) const;
  /// Returns an array with the number of information sets per personal player
  gbtArray<int> NumInfosets(void) const;
  /// Returns the act'th action in the game (numbered globally)
  gbtEfgAction *GetAction(int act) const;
  /// Returns the total number of actions in the game
  int ProfileLength(void) const;
  //@}

  /// @name Outcomes
  //@{
  /// Returns the number of outcomes defined in the game
  int NumOutcomes(void) const;
  /// Returns the index'th outcome defined in the game
  gbtEfgOutcome *GetOutcome(int index) const;

  /// Creates a new outcome in the game
  gbtEfgOutcome *NewOutcome(void);
  /// Deletes the specified outcome from the game
  void DeleteOutcome(gbtEfgOutcome *);
  //@}

  // EDITING OPERATIONS
  gbtEfgInfoset *AppendNode(gbtEfgNode *n, gbtEfgPlayer *p, int br);
  gbtEfgInfoset *AppendNode(gbtEfgNode *n, gbtEfgInfoset *s);
  gbtEfgNode *DeleteNode(gbtEfgNode *n, gbtEfgNode *keep);
  gbtEfgInfoset *InsertNode(gbtEfgNode *n, gbtEfgPlayer *p, int br);
  gbtEfgInfoset *InsertNode(gbtEfgNode *n, gbtEfgInfoset *s);

  gbtEfgInfoset *CreateInfoset(gbtEfgPlayer *pl, int br);
  bool DeleteEmptyInfoset(gbtEfgInfoset *);
  void DeleteEmptyInfosets(void);
  gbtEfgInfoset *JoinInfoset(gbtEfgInfoset *s, gbtEfgNode *n);
  gbtEfgInfoset *LeaveInfoset(gbtEfgNode *n);
  gbtEfgInfoset *SplitInfoset(gbtEfgNode *n);
  gbtEfgInfoset *MergeInfoset(gbtEfgInfoset *to, gbtEfgInfoset *from);

  gbtEfgInfoset *SwitchPlayer(gbtEfgInfoset *s, gbtEfgPlayer *p);
  
  gbtEfgNode *CopyTree(gbtEfgNode *src, gbtEfgNode *dest);
  gbtEfgNode *MoveTree(gbtEfgNode *src, gbtEfgNode *dest);
  gbtEfgNode *DeleteTree(gbtEfgNode *n);

  gbtEfgAction *InsertAction(gbtEfgInfoset *s);
  gbtEfgAction *InsertAction(gbtEfgInfoset *s, const gbtEfgAction *at);
  gbtEfgInfoset *DeleteAction(gbtEfgInfoset *s, const gbtEfgAction *a);

  void Reveal(gbtEfgInfoset *, const gbtArray<gbtEfgPlayer *> &);

  gbtPVector<int> NumActions(void) const;
  gbtPVector<int> NumMembers(void) const;
  
  // COMPUTING VALUES OF PROFILES
  void Payoff(const gbtPVector<int> &profile, gbtVector<gbtRational> &payoff) const;
  void Payoff(const gbtArray<gbtArray<int> > &profile,
	      gbtArray<gbtRational> &payoff) const;

  void InfosetProbs(const gbtPVector<int> &profile, gbtPVector<gbtRational> &prob) const;
    
};

gbtEfgGame *ReadEfg(std::istream &);

class gbtPureBehavProfile   {
protected:
  gbtEfgGame *m_efg;
  gbtArray<gbtArray<gbtEfgAction *> > m_profile;

  void GetPayoff(const gbtEfgNode *n, const gbtRational &, 
		 gbtArray<gbtRational> &) const;
  void InfosetProbs(gbtEfgNode *n, const gbtRational &, 
		    gbtPVector<gbtRational> &) const;

public:
  gbtPureBehavProfile(gbtEfgGame *);

  // Operators
  gbtPureBehavProfile &operator=(const gbtPureBehavProfile &);
  gbtRational operator()(gbtEfgAction *) const;

  // Manipulation
  void Set(gbtEfgAction *);
  void Set(gbtEfgPlayer *, const gbtArray<gbtEfgAction *> &);
  
  // Information
  gbtEfgAction *GetAction(gbtEfgInfoset *) const;
   
  gbtRational Payoff(const gbtEfgNode *, int pl) const;
  void Payoff(gbtArray<gbtRational> &payoff) const;
  void InfosetProbs(gbtPVector<gbtRational> &prob) const;
  gbtEfgGame *GetGame(void) const   { return m_efg; }
};

#endif   // EFG_H


