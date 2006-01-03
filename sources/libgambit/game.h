//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of base class for representing games
//
// This file is part of Gambit
// Copyright (c) 2005, The Gambit Project
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


#ifndef LIBGAMBIT_GAME_H
#define LIBGAMBIT_GAME_H

#include "gdpvect.h"

class gbtStrategyProfile;
class gbtPureBehavProfile;

class gbtEfgSupport;
class gbtNumber;

template <class T> class gbtBehavProfile;
template <class T> class gbtMixedProfile;


namespace Gambit {

/// This is a base class for all game-related objects.  Primary among
/// its responsibility is maintaining a reference count.  Calling code
/// which maintains pointers to objects representing parts of a game
/// (e.g., nodes) which may be deleted should increment the reference
/// count for that object.  The library guarantees that any object
/// with a positive reference count will not have its memory deleted,
/// but will instead be marked as deleted.  Calling code should always
/// be careful to check the deleted status of the object before any
/// operations on it.
class GameObject {
protected:
  int m_refCount;
  bool m_valid;

public:
  /// @name Lifecycle
  //@{
  /// Constructor; initializes reference count
  GameObject(void) : m_refCount(0), m_valid(true) { }
  //@}

  /// @name Validation
  //@{
  /// Is the object still valid?
  bool IsValid(void) const { return m_valid; }
  /// Invalidate the object; delete if not referenced elsewhere
  void Invalidate(void)
  { if (!m_refCount) delete this; else m_valid = false; }
  //@}

  /// @name Reference counting
  //@{
  /// Increment the reference count
  void IncRef(void) { m_refCount++; }
  /// Decrement the reference count; delete if reference count is zero.
  void DecRef(void) { if (!--m_refCount && !m_valid) delete this; }
  //@}
};


class NullException : public gbtException {
public:
  virtual ~NullException() { }
  std::string GetDescription(void) const
    { return "Dereferencing null pointer"; }
};

//
// This is a handle class that is used by all calling code to refer to
// member objects of games.  It takes care of all the reference-counting
// considerations.
//
template <class T> class GameObjectPtr {
private:
  T *rep;

public:
  GameObjectPtr(T *r = 0) : rep(r)
    { if (rep) rep->IncRef(); }
  GameObjectPtr(const GameObjectPtr<T> &r) : rep(r.rep)
    { if (rep) rep->IncRef(); }
  ~GameObjectPtr() { if (rep) rep->DecRef(); }

  GameObjectPtr<T> &operator=(const GameObjectPtr<T> &r)
    { if (&r != this) {
	if (rep) rep->DecRef();
	rep = r.rep;
	if (rep) rep->IncRef();
      }
      return *this;
    }

  T *operator->(void) const { if (!rep) throw NullException(); return rep; }

  bool operator==(const GameObjectPtr<T> &r) const
  { return (rep == r.rep); }
  bool operator==(T *r) const { return (rep == r); }
  bool operator!=(const GameObjectPtr<T> &r) const 
  { return (rep != r.rep); }
  bool operator!=(T *r) const { return (rep != r); }

  operator T *(void) const { if (!rep) throw NullException(); return rep; }

  bool operator!(void) const { return !rep; }
};


//
// Forward declarations of classes defined in this file.
//

class GameActionRep;
typedef GameObjectPtr<GameActionRep> GameAction;

class GameInfosetRep;
typedef GameObjectPtr<GameInfosetRep> GameInfoset;

class GameStrategyRep;
typedef GameObjectPtr<GameStrategyRep> GameStrategy;

class GamePlayerRep;
typedef GameObjectPtr<GamePlayerRep> GamePlayer;

class GameNodeRep;
typedef GameObjectPtr<GameNodeRep> GameNode;

class GameRep;
typedef GameObjectPtr<GameRep> Game;


/// This class represents an outcome in a game.  An outcome
/// specifies a vector of payoffs to players.  Payoffs are specified
/// using text strings, in either decimal or rational format.  All
/// payoffs are treated as exact (that is, no conversion to floating
/// point is done).
class GameOutcomeRep : public GameObject  {
  friend class GameRep;

private:
  GameRep *m_game;
  int m_number;
  std::string m_label;
  gbtArray<std::string> m_textPayoffs;
  gbtArray<gbtRational> m_ratPayoffs;

  /// @name Lifecycle
  //@{
  /// Creates a new outcome object, with payoffs set to zero
  GameOutcomeRep(GameRep *p_game, int p_number);
  virtual ~GameOutcomeRep() { }
  //@}

public:
  /// @name Data access
  //@{
  /// Returns the strategic game on which the outcome is defined.
  Game GetGame(void) const { return m_game; }
  /// Returns the index number of the outcome
  int GetNumber(void) const { return m_number; }

  /// Returns the text label associated with the outcome
  const std::string &GetLabel(void) const { return m_label; }
  /// Sets the text label associated with the outcome 
  void SetLabel(const std::string &p_label) { m_label = p_label; }

  /// Gets the payoff associated with the outcome to player 'pl'
  const gbtRational &GetPayoff(int pl) const { return m_ratPayoffs[pl]; }
  /// Gets the text representation of the payoff to player 'pl'
  const std::string &GetPayoffText(int pl) const { return m_textPayoffs[pl]; }
  /// Sets the payoff to player 'pl'
  void SetPayoff(int pl, const std::string &p_value)
  {
    m_textPayoffs[pl] = p_value;
    m_ratPayoffs[pl] = ToRational(p_value);
    //m_game->ClearComputedValues();
  }
  //@}

};

typedef GameObjectPtr<GameOutcomeRep> GameOutcome;


class GameActionRep : public GameObject {
  friend class GameRep;
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


class GameInfosetRep : public GameObject {
  friend class GameRep;
  friend class GameTreeRep;
  friend class GamePlayerRep;
  friend class GameNodeRep;
  friend class gbtBehavProfile<double>;
  friend class gbtBehavProfile<gbtRational>;
  friend class gbtBehavProfile<gbtNumber>;
  friend void MakeStrategy(GamePlayerRep *);
  friend void MakeReducedStrats(GamePlayerRep *p, GameNodeRep *n, GameNodeRep *nn);

protected:
  GameRep *m_efg;
  int m_number;
  std::string m_label;
  GamePlayerRep *m_player;
  gbtArray<GameActionRep *> m_actions;
  gbtArray<GameNodeRep *> m_members;
  int flag, whichbranch;
  gbtArray<std::string> m_textProbs;
  gbtArray<gbtRational> m_ratProbs;
  
  GameInfosetRep(GameRep *p_efg, int p_number, GamePlayerRep *p_player, 
		 int p_actions);
  ~GameInfosetRep();  

public:
  Game GetGame(void) const { return m_efg; }
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

/// This class represents a strategy in a strategic game.
/// Internally, this strategy stores an 'index'.  This index has the
/// property that, for a strategy profile, adding the indices of the
/// strategies gives the index into the strategic game's table to
/// find the outcome for that strategy profile, making payoff computation
/// relatively efficient.
class GameStrategyRep : public GameObject  {
  friend class GameRep;
  friend class GamePlayerRep;
  friend class ::gbtStrategyProfile;
  friend class gbtMixedProfile<double>;
  friend class gbtMixedProfile<gbtRational>;
  friend class gbtMixedProfile<gbtNumber>;
  friend class gbtBehavProfile<double>;
  friend class gbtBehavProfile<gbtRational>;
  friend class gbtBehavProfile<gbtNumber>;
  friend void MakeStrategy(GamePlayerRep *);
private:
  int m_number;
  GamePlayerRep *m_player;
  long m_index;
  std::string m_name;
  gbtArray<int> m_behav;

  /// @name Lifecycle
  //@{
  /// Creates a new strategy for the given player.
  GameStrategyRep(GamePlayerRep *p_player)
    : m_number(0), m_player(p_player), m_index(0L) { }
  //@}

public:
  /// @name Data access
  //@{
  /// Returns the text label associated with the strategy
  const std::string &GetName(void) const { return m_name; }
  /// Sets the text label associated with the strategy
  void SetName(const std::string &s) { m_name = s; }
  
  // Returns the player for whom this is a strategy
  GamePlayer GetPlayer(void) const;
  /// Returns the index of the strategy for its player
  int GetNumber(void) const { return m_number; }

  /// Remove this strategy from the game
  void DeleteStrategy(void);
  //@}
};


class GamePlayerRep : public GameObject  {
  friend class GameRep;
  friend class GameStrategyRep;
  friend class gbtPureBehavProfile;
  friend class gbtBehavProfile<double>;
  friend class gbtBehavProfile<gbtRational>;
  friend class gbtBehavProfile<gbtNumber>;
  friend class gbtMixedProfile<double>;
  friend class gbtMixedProfile<gbtRational>;
  friend class gbtMixedProfile<gbtNumber>;
  friend void MakeStrategy(GamePlayerRep *);

private:
  GameRep *m_game;
  int m_number;
  std::string m_label;
  gbtArray<GameInfosetRep *> m_infosets;
  gbtArray<GameStrategyRep *> m_strategies;

  GamePlayerRep(GameRep *p_game, int p_id) : m_game(p_game), m_number(p_id)
    { }
  GamePlayerRep(GameRep *p_game, int p_id, int m_strats);
  ~GamePlayerRep();

public:
  int GetNumber(void) const { return m_number; }
  Game GetGame(void) const { return m_game; }
  
  const std::string &GetLabel(void) const { return m_label; }
  void SetLabel(const std::string &p_label) { m_label = p_label; }
  
  bool IsChance(void) const { return (m_number == 0); }

  int NumInfosets(void) const { return m_infosets.Length(); }
  GameInfoset GetInfoset(int p_index) const { return m_infosets[p_index]; }

  /// @name Strategies
  //@{
  /// Returns the number of strategies available to the player
  int NumStrategies(void) const { return m_strategies.Length(); }
  /// Returns the st'th strategy for the player
  GameStrategy GetStrategy(int st) { return m_strategies[st]; }
  /// Creates a new strategy for the player
  GameStrategy NewStrategy(void);
  //@}
};

class GameNodeRep : public GameObject {
  friend class GameRep;
  friend class gbtBehavProfile<double>;
  friend class gbtBehavProfile<gbtRational>;
  friend class gbtBehavProfile<gbtNumber>;
  friend class ::gbtPureBehavProfile;
  friend void MakeReducedStrats(GamePlayerRep *p, GameNodeRep *n, GameNodeRep *nn);
  
protected:
  bool mark;
  int number; 
  GameRep *m_efg;
  std::string m_label;
  GameInfosetRep *infoset;
  GameNodeRep *parent;
  GameOutcomeRep *outcome;
  gbtArray<GameNodeRep *> children;
  GameNodeRep *whichbranch, *ptr;

  GameNodeRep(GameRep *e, GameNodeRep *p);
  ~GameNodeRep();

  void DeleteOutcome(GameOutcomeRep *outc);

public:
  Game GetGame(void) const { return m_efg; }

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

  GameOutcome GetOutcome(void) const { return outcome; }
  void SetOutcome(const GameOutcome &p_outcome);

  bool IsSuccessorOf(GameNode from) const;
  bool IsSubgameRoot(void) const;
};


//
// This is a base class for representing an arbitrary finite game.
//
class GameRep : public GameObject {
  friend class ::gbtStrategyProfile;
  friend class GameNodeRep;
  friend class gbtBehavProfile<double>;
  friend class gbtBehavProfile<gbtRational>;
  friend class gbtBehavProfile<gbtNumber>;
  friend class gbtMixedProfile<double>;
  friend class gbtMixedProfile<gbtRational>;
  friend class gbtMixedProfile<gbtNumber>;
protected:
  std::string m_title, m_comment;
  gbtArray<GamePlayerRep *> m_players;
  GamePlayerRep *m_chance;

  gbtArray<GameOutcomeRep *> m_outcomes;

  GameNodeRep *m_root;
  gbtArray<GameOutcomeRep *> m_results;

  void NumberNodes(GameNodeRep *, int &);

  // These are used in identification of subgames
  void MarkTree(GameNodeRep *, GameNodeRep *);
  bool CheckTree(GameNodeRep *, GameNodeRep *);
  // These are used in Reveal (and only reveal?)
  void MarkSubtree(GameNodeRep *);
  void UnmarkSubtree(GameNodeRep *);

  void CopySubtree(GameNodeRep *, GameNodeRep *, GameNodeRep *);
  
  void Payoff(GameNodeRep *n, gbtRational, const gbtPVector<int> &, gbtVector<gbtRational> &) const;
  void Payoff(GameNodeRep *n, gbtRational, const gbtArray<gbtArray<int> > &, gbtArray<gbtRational> &) const;
  
  void InfosetProbs(GameNodeRep *n, gbtRational, const gbtPVector<int> &, gbtPVector<gbtRational> &) const;
    
  /// @name Private auxiliary functions
  //@{
  void IndexStrategies(void);
  void RebuildTable(void);
  //@}

public:
  /// @name Lifecycle
  //@{
  /// Construct a new trivial game
  GameRep(void);
  /// Construct a new table game with the given dimension
  GameRep(const gbtArray<int> &p_dim);
  /// Clean up the game
  virtual ~GameRep();
  //@}

  /// @name General data access
  //@{
  /// Returns true if the game has a game tree representation
  bool IsTree(void) const { return (m_root != 0); }

  /// Get the text label associated with the game
  const std::string &GetTitle(void) const { return m_title; }
  /// Set the text label associated with the game
  void SetTitle(const std::string &p_title) { m_title = p_title; }

  /// Get the text comment associated with the game
  const std::string &GetComment(void) const { return m_comment; }
  /// Set the text comment associated with the game
  void SetComment(const std::string &p_comment) { m_comment = p_comment; }

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
  //@}

  /// @name Managing the representation
  //@{
  /// Renumber all game objects in a canonical way
  void Canonicalize(void);  
  /// Clear out any computed values
  void ClearComputedValues(void) const;
  /// Build any computed values anew
  void BuildComputedValues(void);
  /// Have computed values been built?
  bool HasComputedValues(void) const;
  //@}

  /// @name Writing data files
  //@{
  /// Write the game in .efg format to the specified stream
  void WriteEfgFile(std::ostream &) const;
  /// Write the game in .nfg format to the specified stream
  void WriteNfgFile(std::ostream &) const;
  //@}

  /// @name Dimensions of the game
  //@{
  /// The number of actions in each information set
  gbtPVector<int> NumActions(void) const;
  /// The number of members in each information set
  gbtPVector<int> NumMembers(void) const;
  /// The number of strategies for each player
  gbtArray<int> NumStrategies(void) const;
  /// Returns the total number of actions in the game
  int BehavProfileLength(void) const;
  /// Returns the total number of strategies in the game
  int MixedProfileLength(void) const;
  //@}


  /// @name Players
  //@{
  /// Returns the number of players in the game
  int NumPlayers(void) const { return m_players.Length(); }
  /// Returns the pl'th player in the game
  GamePlayer GetPlayer(int pl) const { return m_players[pl]; }
  /// Returns the chance (nature) player
  GamePlayer GetChance(void) const { return m_chance; }
  /// Creates a new player in the game, with no moves
  GamePlayer NewPlayer(void);
  //@}

  /// @name Information sets
  //@{
  /// Returns the iset'th information set in the game (numbered globally)
  GameInfoset GetInfoset(int iset) const;
  /// Returns an array with the number of information sets per personal player
  gbtArray<int> NumInfosets(void) const;
  /// Returns the act'th action in the game (numbered globally)
  GameAction GetAction(int act) const;
  //@}

  /// @name Outcomes
  //@{
  /// Returns the number of outcomes defined in the game
  int NumOutcomes(void) const { return m_outcomes.Length(); }
  /// Returns the index'th outcome defined in the game
  GameOutcome GetOutcome(int index) const { return m_outcomes[index]; }

  /// Creates a new outcome in the game
  GameOutcome NewOutcome(void);
  /// Deletes the specified outcome from the game
  void DeleteOutcome(const GameOutcome &);
  //@}

  /// @name Nodes
  //@{
  /// Returns the root node of the game
  GameNode GetRoot(void) const { return m_root; }
  /// Returns the number of nodes in the game
  int NumNodes(void) const;
  //@}

  /// @name Editing game trees
  //@{
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
  //@}

  /// @name Computing payoffs of pure behavior profiles
  //@{
  void Payoff(const gbtPVector<int> &profile, gbtVector<gbtRational> &payoff) const;
  void Payoff(const gbtArray<gbtArray<int> > &profile,
	      gbtArray<gbtRational> &payoff) const;

  void InfosetProbs(const gbtPVector<int> &profile, gbtPVector<gbtRational> &prob) const;
  //@}

};

typedef GameObjectPtr<GameRep> Game;


inline GameOutcomeRep::GameOutcomeRep(GameRep *p_game, int p_number)
  : m_game(p_game), m_number(p_number),
    m_textPayoffs(m_game->NumPlayers()),
    m_ratPayoffs(m_game->NumPlayers())
{
  for (int pl = 1; pl <= m_textPayoffs.Length(); pl++) {
    m_textPayoffs[pl] = "0";
  }
}

class gbtEfgException : public gbtException   {
public:
  virtual ~gbtEfgException()   { }
  std::string GetDescription(void) const  
    { return "Internal error in extensive form representation"; }
};

class UndefinedException : public gbtException {
public:
  virtual ~UndefinedException() { }
  std::string GetDescription(void) const
    { return "Undefined operation on game"; }
};


Game ReadEfg(std::istream &);

// Exception thrown by ReadNfg if not valid .nfg file
class gbtNfgParserException : public gbtException {
private:
  std::string m_description;

public:
  gbtNfgParserException(void)
    : m_description("Not a valid .nfg file") { }
  gbtNfgParserException(const std::string &p_description) 
    : m_description(p_description) { }
  gbtNfgParserException(int p_line, const std::string &p_description);
  virtual ~gbtNfgParserException() { }

  std::string GetDescription(void) const { return m_description; }
};

/// Reads a strategic game in .nfg format from the input stream
Game ReadNfg(std::istream &);

} // end namespace gambit

#endif   // LIBGAMBIT_GAME_H
