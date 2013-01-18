//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/gametree.h
// Declaration of extensive game representation
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

#ifndef GAMETREE_H
#define GAMETREE_H

#include "gameexpl.h"

namespace Gambit {

class GameTreeRep;

class GameTreeActionRep : public GameActionRep {
  friend class GameTreeRep;
  friend class GameTreeInfosetRep;
  template <class T> friend class MixedBehavProfile;

private:
  int m_number;
  std::string m_label;
  GameTreeInfosetRep *m_infoset;

  GameTreeActionRep(int p_number, const std::string &p_label, 
		    GameTreeInfosetRep *p_infoset)
    : m_number(p_number), m_label(p_label), m_infoset(p_infoset) { }
  virtual ~GameTreeActionRep()   { }

public:
  int GetNumber(void) const { return m_number; }
  GameInfoset GetInfoset(void) const;

  const std::string &GetLabel(void) const { return m_label; }
  void SetLabel(const std::string &p_label) { m_label = p_label; }

  bool Precedes(const GameNode &) const;

  void DeleteAction(void);
};

class GameTreeInfosetRep : public GameInfosetRep {
  friend class GameTreeRep;
  friend class GameTreeActionRep;
  friend class GamePlayerRep;
  friend class GameTreeNodeRep;
  template <class T> friend class MixedBehavProfile;

protected:
  GameTreeRep *m_efg;
  int m_number;
  std::string m_label;
  GamePlayerRep *m_player;
  Array<GameTreeActionRep *> m_actions;
  Array<GameTreeNodeRep *> m_members;
  int flag, whichbranch;
  Array<Number> m_probs;
  
  GameTreeInfosetRep(GameTreeRep *p_efg, int p_number, GamePlayerRep *p_player, 
		 int p_actions);
  virtual ~GameTreeInfosetRep();  

  /// Adds the node to the information set
  void AddMember(GameTreeNodeRep *p_node) { m_members.Append(p_node); }
  /// Removes the node from the information set, invalidating if emptied
  void RemoveMember(GameTreeNodeRep *);

  void RemoveAction(int which);

public:
  virtual Game GetGame(void) const;
  virtual int GetNumber(void) const { return m_number; }
  
  virtual GamePlayer GetPlayer(void) const;
  virtual void SetPlayer(GamePlayer p);

  virtual bool IsChanceInfoset(void) const;

  virtual void SetLabel(const std::string &p_label) { m_label = p_label; }
  virtual const std::string &GetLabel(void) const { return m_label; }
  
  virtual GameAction InsertAction(GameAction p_where = 0);

  /// @name Actions
  //@{
  /// Returns the number of actions available at the information set
  virtual int NumActions(void) const { return m_actions.Length(); }
  /// Returns the p_index'th action at the information set
  virtual GameAction GetAction(int p_index) const { return m_actions[p_index]; }
  /// Returns a forward iterator over the available actions
  //virtual GameActionIterator Actions(void) const 
  //  { return GameActionIterator(m_actions); }
  //@}

  virtual int NumMembers(void) const { return m_members.Length(); }
  virtual GameNode GetMember(int p_index) const;

  virtual bool Precedes(GameNode) const;

  virtual void SetActionProb(int i, const std::string &p_value);
  virtual double GetActionProb(int pl, double) const
  { return (double) m_probs[pl]; }
  virtual Rational GetActionProb(int pl, const Rational &) const
  { return (const Rational &) m_probs[pl]; }
  virtual std::string GetActionProb(int pl, const std::string &) const
  { return (const std::string &) m_probs[pl]; }

  virtual void Reveal(GamePlayer);
};


class GameTreeNodeRep : public GameNodeRep {
  friend class GameTreeRep;
  friend class GameTreeActionRep;
  friend class GameTreeInfosetRep;
  friend class GamePlayerRep;
  friend class PureBehavProfile;
  template <class T> friend class MixedBehavProfile;
  
protected:
  int number; 
  GameTreeRep *m_efg;
  std::string m_label;
  GameTreeInfosetRep *infoset;
  GameTreeNodeRep *m_parent;
  GameOutcomeRep *outcome;
  Array<GameTreeNodeRep *> children;
  GameTreeNodeRep *whichbranch, *ptr;

  GameTreeNodeRep(GameTreeRep *e, GameTreeNodeRep *p);
  virtual ~GameTreeNodeRep();

  void DeleteOutcome(GameOutcomeRep *outc);
  void CopySubtree(GameTreeNodeRep *, GameTreeNodeRep *);

public:
  virtual Game GetGame(void) const; 

  virtual const std::string &GetLabel(void) const { return m_label; } 
  virtual void SetLabel(const std::string &p_label) { m_label = p_label; }

  virtual int GetNumber(void) const { return number; }
  virtual int NumberInInfoset(void) const
  { return infoset->m_members.Find(const_cast<GameTreeNodeRep *>(this)); }

  virtual int NumChildren(void) const    { return children.Length(); }

  virtual GameInfoset GetInfoset(void) const   { return infoset; }
  virtual void SetInfoset(GameInfoset);
  virtual GameInfoset LeaveInfoset(void);

  virtual bool IsTerminal(void) const { return (children.Length() == 0); }
  virtual GamePlayer GetPlayer(void) const
    { return (infoset) ? infoset->GetPlayer() : 0; }
  virtual GameAction GetPriorAction(void) const; // returns null if root node
  virtual GameNode GetChild(int i) const    { return children[i]; }
  virtual GameNode GetParent(void) const    { return m_parent; }
  virtual GameNode GetNextSibling(void) const;
  virtual GameNode GetPriorSibling(void) const;

  virtual GameOutcome GetOutcome(void) const { return outcome; }
  virtual void SetOutcome(const GameOutcome &p_outcome);

  virtual bool IsSuccessorOf(GameNode from) const;
  virtual bool IsSubgameRoot(void) const;

  virtual void DeleteParent(void);
  virtual void DeleteTree(void);

  virtual void CopyTree(GameNode src);
  virtual void MoveTree(GameNode src);

  virtual GameInfoset AppendMove(GamePlayer p_player, int p_actions);
  virtual GameInfoset AppendMove(GameInfoset p_infoset);
  virtual GameInfoset InsertMove(GamePlayer p_player, int p_actions);
  virtual GameInfoset InsertMove(GameInfoset p_infoset);
};


class GameTreeRep : public GameExplicitRep {
  friend class GameTreeNodeRep;
  friend class GameTreeInfosetRep;
  friend class GameTreeActionRep;
protected:
  mutable bool m_computedValues;
  GameTreeNodeRep *m_root;
  GamePlayerRep *m_chance;

  /// @name Private auxiliary functions
  //@{
  void NumberNodes(GameTreeNodeRep *, int &);
  //@}

  /// @name Managing the representation
  //@{
  virtual void Canonicalize(void);
  virtual void BuildComputedValues(void);
  virtual void ClearComputedValues(void) const;
  /// Have computed values been built?
  virtual bool HasComputedValues(void) const { return m_computedValues; }
  //@}

public: 
  /// @name Lifecycle
  //@{
  GameTreeRep(void);
  virtual ~GameTreeRep();
  virtual Game Copy(void) const;
  //@}

  /// @name General data access
  //@{
  virtual bool IsTree(void) const { return true; }
  virtual bool IsConstSum(void) const;
  virtual bool IsPerfectRecall(GameInfoset &, GameInfoset &) const;
  //@}

  /// @name Players
  //@{
  /// Returns the chance (nature) player
  virtual GamePlayer GetChance(void) const { return m_chance; } 
  /// Creates a new player in the game, with no moves
  virtual GamePlayer NewPlayer(void);
  //@}

  /// @name Nodes
  //@{
  /// Returns the root node of the game
  virtual GameNode GetRoot(void) const { return m_root; } 
  /// Returns the number of nodes in the game
  int NumNodes(void) const;
  //@}

  virtual void DeleteOutcome(const GameOutcome &);

  /// @name Writing data files
  //@{
  virtual void WriteEfgFile(std::ostream &) const;
  virtual void WriteEfgFile(std::ostream &, const GameNode &p_node) const;
  virtual void WriteNfgFile(std::ostream &) const;
  //@}

  /// @name Dimensions of the game
  //@{
  /// The number of actions in each information set
  virtual PVector<int> NumActions(void) const;
  /// The number of members in each information set
  virtual PVector<int> NumMembers(void) const;
  /// Returns the total number of actions in the game
  virtual int BehavProfileLength(void) const;
  //@}

  /// @name Information sets
  //@{
  /// Returns the iset'th information set in the game (numbered globally)
  virtual GameInfoset GetInfoset(int iset) const;
  /// Returns an array with the number of information sets per personal player
  virtual Array<int> NumInfosets(void) const;
  /// Returns the act'th action in the game (numbered globally)
  virtual GameAction GetAction(int act) const;
  //@}

  virtual PureStrategyProfile NewPureStrategyProfile(void) const;
  virtual MixedStrategyProfile<double> NewMixedStrategyProfile(double) const;
  virtual MixedStrategyProfile<Rational> NewMixedStrategyProfile(const Rational &) const; 
};

}



#endif  // GAMETREE_H
