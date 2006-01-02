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

#ifndef LIBGAMBIT_TREEGAME_H
#define LIBGAMBIT_TREEGAME_H

#include "gpvector.h"
#include "gdpvect.h"
#include "game.h"

class gbtPureBehavProfile;

class gbtEfgSupport;
class gbtNumber;

template <class T> class gbtBehavProfile;
template <class T> class gbtMixedProfile;

namespace Gambit {

//
// Forward declarations of classes defined in this file.
//

class GameActionRep;
typedef Gambit::GameObjectPtr<GameActionRep> GameAction;

class GameInfosetRep;
typedef Gambit::GameObjectPtr<GameInfosetRep> GameInfoset;

class GamePlayerRep;
typedef Gambit::GameObjectPtr<GamePlayerRep> GamePlayer;

class GameNodeRep;
typedef Gambit::GameObjectPtr<GameNodeRep> GameNode;

class GameTreeRep;
typedef Gambit::GameObjectPtr<GameTreeRep> GameTree;

//
// External declarations
//
class GameTableRep;
typedef Gambit::GameObjectPtr<GameTableRep> GameTable;

class gbtEfgException : public gbtException   {
public:
  virtual ~gbtEfgException()   { }
  std::string GetDescription(void) const  
    { return "Internal error in extensive form representation"; }
};


class GameActionRep : public Gambit::GameObject {
  friend class GameTreeRep;
  friend class gbtBehavProfile<double>;
  friend class gbtBehavProfile<gbtRational>;
  friend class gbtBehavProfile<gbtNumber>;
  friend class GameInfosetRep;
private:
  int m_number;
  std::string m_label;
  GameInfosetRep *m_infoset;

  GameActionRep(int p_number, const std::string &p_label, 
		  GameInfosetRep *p_infoset)
    : m_number(p_number), m_label(p_label), m_infoset(p_infoset) { }
  ~GameActionRep()   { }

public:
  int GetNumber(void) const { return m_number; }
  GameInfoset GetInfoset(void) const { return m_infoset; }

  const std::string &GetLabel(void) const { return m_label; }
  void SetLabel(const std::string &p_label) { m_label = p_label; }

  bool Precedes(const GameNode &) const;
};


class GameInfosetRep : public Gambit::GameObject {
  friend class GameTreeRep;
  friend class GamePlayerRep;
  friend class GameNodeRep;
  friend class gbtBehavProfile<double>;
  friend class gbtBehavProfile<gbtRational>;
  friend class gbtBehavProfile<gbtNumber>;
  friend void MakeStrategy(GamePlayerRep *);
  friend void MakeReducedStrats(GamePlayerRep *p, GameNodeRep *n, GameNodeRep *nn);

protected:
  GameTreeRep *m_efg;
  int m_number;
  std::string m_label;
  GamePlayerRep *m_player;
  gbtArray<GameActionRep *> m_actions;
  gbtArray<GameNodeRep *> m_members;
  int flag, whichbranch;
  gbtArray<std::string> m_textProbs;
  gbtArray<gbtRational> m_ratProbs;
  
  GameInfosetRep(GameTreeRep *p_efg, int p_number, GamePlayerRep *p_player, 
		   int p_actions);
  ~GameInfosetRep();  

public:
  GameTree GetGame(void) const { return m_efg; }
  int GetNumber(void) const { return m_number; }
  
  GamePlayer GetPlayer(void) const { return m_player; }
  bool IsChanceInfoset(void) const;

  void SetLabel(const std::string &p_label) { m_label = p_label; }
  const std::string &GetLabel(void) const { return m_label; }
  
  GameAction InsertAction(int where);
  void RemoveAction(int which);

  int NumActions(void) const { return m_actions.Length(); }
  GameAction GetAction(int p_index) const { return m_actions[p_index]; }

  int NumMembers(void) const { return m_members.Length(); }
  GameNode GetMember(int p_index) const { return m_members[p_index]; }

  bool Precedes(GameNode) const;

  void SetActionProb(int i, const std::string &p_value);
  const gbtRational &GetActionProb(int i) const { return m_ratProbs[i]; }
  const std::string &GetActionProbText(int i) const { return m_textProbs[i]; }
};

class GamePlayerRep : public Gambit::GameObject  {
  friend class GameTreeRep;
  friend class gbtPureBehavProfile;
  friend class gbtBehavProfile<double>;
  friend class gbtBehavProfile<gbtRational>;
  friend class gbtBehavProfile<gbtNumber>;
  friend class gbtMixedProfile<double>;
  friend class gbtMixedProfile<gbtRational>;
  friend class gbtMixedProfile<gbtNumber>;
  friend void MakeStrategy(GamePlayerRep *);

private:
  GameTreeRep *m_efg;
  int m_number;
  std::string m_label;
  gbtArray<GameInfosetRep *> m_infosets;
  gbtList<gbtArray<int> > m_strategies;

  GamePlayerRep(GameTreeRep *p_efg, int p_id) : m_number(p_id), m_efg(p_efg)  { }
  ~GamePlayerRep();

public:
  int GetNumber(void) const { return m_number; }
  GameTree GetGame(void) const { return m_efg; }
  
  const std::string &GetLabel(void) const { return m_label; }
  void SetLabel(const std::string &p_label) { m_label = p_label; }
  
  bool IsChance(void) const { return (m_number == 0); }

  int NumInfosets(void) const { return m_infosets.Length(); }
  GameInfoset GetInfoset(int p_index) const { return m_infosets[p_index]; }
};

class GameNodeRep : public Gambit::GameObject {
  friend class GameTreeRep;
  friend class gbtBehavProfile<double>;
  friend class gbtBehavProfile<gbtRational>;
  friend class gbtBehavProfile<gbtNumber>;
  friend class ::gbtPureBehavProfile;
  friend void MakeReducedStrats(GamePlayerRep *p, GameNodeRep *n, GameNodeRep *nn);
  
protected:
  bool mark;
  int number; 
  GameTreeRep *m_efg;
  std::string m_label;
  GameInfosetRep *infoset;
  GameNodeRep *parent;
  Gambit::GameOutcomeRep *outcome;
  gbtArray<GameNodeRep *> children;
  GameNodeRep *whichbranch, *ptr;

  GameNodeRep(GameTreeRep *e, GameNodeRep *p);
  ~GameNodeRep();

  void DeleteOutcome(Gambit::GameOutcomeRep *outc);

public:
  GameTree GetGame(void) const { return m_efg; }

  const std::string &GetLabel(void) const { return m_label; } 
  void SetLabel(const std::string &p_label) { m_label = p_label; }

  int GetNumber(void) const { return number; }
  int NumberInInfoset(void) const
  { return infoset->m_members.Find(const_cast<GameNodeRep *>(this)); }

  int NumChildren(void) const    { return children.Length(); }

  GameInfoset GetInfoset(void) const   { return infoset; }
  bool IsTerminal(void) const { return (children.Length() == 0); }
  GamePlayer GetPlayer(void) const
    { return (infoset) ? infoset->GetPlayer() : 0; }
  GameAction GetPriorAction(void) const; // returns null if root node
  GameNode GetChild(int i) const    { return children[i]; }
  GameNode GetParent(void) const    { return parent; }
  GameNode GetNextSibling(void) const;
  GameNode GetPriorSibling(void) const;

  Gambit::GameOutcome GetOutcome(void) const { return outcome; }
  void SetOutcome(const Gambit::GameOutcome &p_outcome);

  bool IsSuccessorOf(GameNode from) const;
  bool IsSubgameRoot(void) const;
};

class GameTreeRep : public Gambit::GameRep {
private:
  friend class EfgFileReader;
  friend class EfgFile;
  friend class gbtNfgGameRep;
  friend class GameNodeRep;
  friend class gbtEfgOutcomeRep;
  friend class GameInfosetRep;
  friend class gbtBehavProfile<double>;
  friend class gbtBehavProfile<gbtRational>;
  friend class gbtBehavProfile<gbtNumber>;
  friend class gbtMixedProfile<double>;
  friend class gbtMixedProfile<gbtRational>;
  friend class gbtMixedProfile<gbtNumber>;
  
protected:
  std::string title, comment;
  gbtArray<GamePlayerRep *> players;
  gbtArray<Gambit::GameOutcomeRep *> outcomes;
  GameNodeRep *m_root;
  GamePlayerRep *chance;
  mutable GameTable m_reducedNfg;
  
  void CopySubtree(GameNodeRep *, GameNodeRep *, GameNodeRep *);
  void MarkSubtree(GameNodeRep *);
  void UnmarkSubtree(GameNodeRep *);

  void NumberNodes(GameNodeRep *, int &);
  
  void ClearComputedValues(void) const;

  void WriteEfgFile(std::ostream &, GameNodeRep *) const;

  void Payoff(GameNodeRep *n, gbtRational, const gbtPVector<int> &, gbtVector<gbtRational> &) const;
  void Payoff(GameNodeRep *n, gbtRational, const gbtArray<gbtArray<int> > &, gbtArray<gbtRational> &) const;
  
  void InfosetProbs(GameNodeRep *n, gbtRational, const gbtPVector<int> &, gbtPVector<gbtRational> &) const;
    
    
  // These are used in identification of subgames
  void MarkTree(GameNodeRep *, GameNodeRep *);
  bool CheckTree(GameNodeRep *, GameNodeRep *);

  // Recursive calls
  void DescendantNodes(const GameNodeRep *, const gbtEfgSupport &, 
		       gbtList<GameNode> &) const;
  void NonterminalDescendants(const GameNodeRep *, const gbtEfgSupport&, 
			      gbtList<GameNode> &) const;
  void TerminalDescendants(const GameNodeRep *, const gbtEfgSupport&, 
			   gbtList<GameNode> &) const;

  GameInfosetRep *CreateInfoset(int n, GamePlayerRep *pl, int br);

public:
  /// @name Lifecycle
  //@{
  /// Construct a new trivial extensive game
  GameTreeRep(void);
  /// Clean up the extensive game
  virtual ~GameTreeRep();
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
  bool IsPerfectRecall(GameInfoset &, GameInfoset &) const;
  /// Returns true if the game is perfect recall
  bool IsPerfectRecall(void) const
    { GameInfoset s, t; return IsPerfectRecall(s, t); }

  /// Returns the reduced normal form representation of the game,
  /// if computed.
  GameTable AssociatedNfg(void) const { return m_reducedNfg; }

  /// Builds the reduced normal form representation of the game
  GameTable MakeReducedNfg(void);
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
  GamePlayer GetPlayer(int pl) const { return players[pl]; }
  /// Returns the chance (nature) player
  GamePlayer GetChance(void) const;
  /// Creates a new player in the game, with no moves
  GamePlayer NewPlayer(void);
  //@}

  /// @name Nodes
  //@{
  /// Returns the root node of the game
  GameNode GetRoot(void) const { return m_root; }
  /// Returns the number of nodes in the game
  int NumNodes(void) const;
  //@}

  /// @name Information sets
  //@{
  /// Returns the iset'th information set in the game (numbered globally)
  GameInfoset GetInfoset(int iset) const;
  /// Returns an array with the number of information sets per personal player
  gbtArray<int> NumInfosets(void) const;
  /// Returns the act'th action in the game (numbered globally)
  GameAction GetAction(int act) const;
  /// Returns the total number of actions in the game
  int ProfileLength(void) const;
  //@}

  /// @name Outcomes
  //@{
  /// Returns the number of outcomes defined in the game
  int NumOutcomes(void) const;
  /// Returns the index'th outcome defined in the game
  Gambit::GameOutcome GetOutcome(int index) const;

  /// Creates a new outcome in the game
  Gambit::GameOutcome NewOutcome(void);
  /// Deletes the specified outcome from the game
  void DeleteOutcome(const Gambit::GameOutcome &);
  //@}

  /// Renumber all game objects in a canonical way
  void Canonicalize(void);

  // EDITING OPERATIONS
  GameInfoset AppendNode(GameNode n, GamePlayer p, int br);
  GameInfoset AppendNode(GameNode n, GameInfoset s);
  GameNode DeleteNode(GameNode n, GameNode keep);
  GameInfoset InsertNode(GameNode n, GamePlayer p, int br);
  GameInfoset InsertNode(GameNode n, GameInfoset s);

  GameInfoset JoinInfoset(GameInfoset s, GameNode n);
  GameInfoset LeaveInfoset(GameNode n);
  GameInfoset SplitInfoset(GameNode n);
  GameInfoset MergeInfoset(GameInfoset to, GameInfoset from);

  GameInfoset SwitchPlayer(GameInfoset s, GamePlayer p);
  
  GameNode CopyTree(GameNode src, GameNode dest);
  GameNode MoveTree(GameNode src, GameNode dest);
  GameNode DeleteTree(GameNode n);

  GameAction InsertAction(GameInfoset s);
  GameAction InsertAction(GameInfoset s, const GameAction &at);
  GameInfoset DeleteAction(GameInfoset s, const GameAction &a);

  void Reveal(GameInfoset, const gbtArray<GamePlayer> &);

  gbtPVector<int> NumActions(void) const;
  gbtPVector<int> NumMembers(void) const;
  
  // COMPUTING VALUES OF PROFILES
  void Payoff(const gbtPVector<int> &profile, gbtVector<gbtRational> &payoff) const;
  void Payoff(const gbtArray<gbtArray<int> > &profile,
	      gbtArray<gbtRational> &payoff) const;

  void InfosetProbs(const gbtPVector<int> &profile, gbtPVector<gbtRational> &prob) const;
    
};

GameTree ReadEfg(std::istream &);

}  // end namespace Gambit

class gbtPureBehavProfile   {
protected:
  Gambit::GameTree m_efg;
  gbtArray<gbtArray<Gambit::GameAction> > m_profile;

  void GetPayoff(const Gambit::GameNode &n, const gbtRational &, 
		 gbtArray<gbtRational> &) const;
  void InfosetProbs(Gambit::GameNode n, const gbtRational &, 
		    gbtPVector<gbtRational> &) const;

public:
  gbtPureBehavProfile(Gambit::GameTree);

  // Operators
  gbtPureBehavProfile &operator=(const gbtPureBehavProfile &);
  gbtRational operator()(Gambit::GameAction) const;

  // Manipulation
  void Set(Gambit::GameAction);
  void Set(Gambit::GamePlayer, const gbtArray<Gambit::GameAction> &);
  
  // Information
  Gambit::GameAction GetAction(Gambit::GameInfoset) const;
   
  gbtRational Payoff(const Gambit::GameNode &, int pl) const;
  void Payoff(gbtArray<gbtRational> &payoff) const;
  void InfosetProbs(gbtPVector<gbtRational> &prob) const;
  Gambit::GameTree GetGame(void) const   { return m_efg; }
};

#endif   // LIBGAMBIT_TREEGAME_H


