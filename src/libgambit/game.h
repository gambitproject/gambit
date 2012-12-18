//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/game.h
// Declaration of base class for representing games
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

#include <memory>
#include "dvector.h"
#include "number.h"

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
  /// Destructor
  virtual ~GameObject() { }
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
  /// Returns the reference count
  int RefCount(void) const { return m_refCount; }
  //@}
};

/// An exception thrown when attempting to dereference a null pointer
class NullException : public Exception {
public:
  virtual ~NullException() throw() { }
  const char *what(void) const throw()  { return "Dereferencing null pointer"; }
};

/// An exception thrown when attempting to dereference an invalidated object 
class InvalidObjectException : public Exception {
public:
  virtual ~InvalidObjectException() throw() { }
  const char *what(void) const throw()  { return "Dereferencing an invalidated object"; }
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

  T *operator->(void) const 
    { if (!rep) throw NullException();
      if (!rep->IsValid()) throw InvalidObjectException(); 
      return rep; }

  bool operator==(const GameObjectPtr<T> &r) const
  { return (rep == r.rep); }
  bool operator==(T *r) const { return (rep == r); }
  bool operator!=(const GameObjectPtr<T> &r) const 
  { return (rep != r.rep); }
  bool operator!=(T *r) const { return (rep != r); }

  operator T *(void) const { return rep; }

  bool operator!(void) const { return !rep; }
};

/// A constant forward iterator on an array of GameObjects
template <class R, class T> class GameObjectIterator {
private:
  const Array<R *> &m_array;
  int m_index;

public:
  /// @name Lifecycle
  //@{
  /// Constructor
  GameObjectIterator(const Array<R *> &p_array)
    : m_array(p_array), m_index(m_array.First()) { }
  //@}

  /// @name Iteration and data access
  //@{
  /// Advance to the next element (prefix version)
  void operator++(void) { m_index++; }
  /// Advance to the next element (postfix version)
  void operator++(int) { m_index++; }
  /// Has iterator gone past the end?
  bool AtEnd(void) const { return m_index > m_array.Last(); }
  /// Get the current index into the array
  int GetIndex(void) const { return m_index; }

  /// Get the current element
    ///T operator*(void) const { return m_array[m_index]; }
  /// Get the current element
  T operator->(void) const { return m_array[m_index]; }
  /// Get the current element
  operator T(void) const { return m_array[m_index]; }
  //@}
};


//
// Forward declarations of classes defined in this file.
//

class GameActionRep;
typedef GameObjectPtr<GameActionRep> GameAction;
typedef GameObjectIterator<GameActionRep, GameAction> GameActionIterator;

class GameInfosetRep;
typedef GameObjectPtr<GameInfosetRep> GameInfoset;
typedef GameObjectIterator<GameInfosetRep, GameInfoset> GameInfosetIterator;
class GameTreeInfosetRep;

class GameStrategyRep;
typedef GameObjectPtr<GameStrategyRep> GameStrategy;
typedef GameObjectIterator<GameStrategyRep, GameStrategy> GameStrategyIterator;

class GamePlayerRep;
typedef GameObjectPtr<GamePlayerRep> GamePlayer;
typedef GameObjectIterator<GamePlayerRep, GamePlayer> GamePlayerIterator;

class GameNodeRep;
typedef GameObjectPtr<GameNodeRep> GameNode;
class GameTreeNodeRep;

class GameRep;
typedef GameObjectPtr<GameRep> Game;

class PureStrategyProfileRep;
typedef std::auto_ptr<PureStrategyProfileRep> PureStrategyProfile;

// 
// Forward declarations of classes defined elsewhere.
//
template <class T> class MixedStrategyProfile;
template <class T> class MixedBehavProfile;


//=======================================================================
//         Exceptions thrown from game representation classes
//=======================================================================

/// Exception thrown when an operation that is undefined is attempted
class UndefinedException : public Exception {
public:
  virtual ~UndefinedException() throw() { }
  const char *what(void) const throw()   { return "Undefined operation on game"; }
};

/// Exception thrown on an operation between incompatible objects
class MismatchException : public Exception {
public:
  virtual ~MismatchException() throw() { }
  const char *what(void) const throw()  
  { return "Operation between objects in different games"; }
};

/// Exception thrown on a parse error when reading a game savefile
class InvalidFileException : public Exception {
public:
  virtual ~InvalidFileException() throw() { }
  const char *what(void) const throw()  { return "File not in a recognized format"; }
};

//=======================================================================
//             Classes representing objects in a game
//=======================================================================

/// This class represents an outcome in a game.  An outcome
/// specifies a vector of payoffs to players.  Payoffs are specified
/// using text strings, in either decimal or rational format.  All
/// payoffs are treated as exact (that is, no conversion to floating
/// point is done).
class GameOutcomeRep : public GameObject  {
  friend class GameExplicitRep;
  friend class GameTreeRep;
  friend class GameTableRep;
  friend class TableFileGameRep;

private:
  GameRep *m_game;
  int m_number;
  std::string m_label;
  Array<Number> m_payoffs;

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
  Game GetGame(void) const;
  /// Returns the index number of the outcome
  int GetNumber(void) const { return m_number; }

  /// Returns the text label associated with the outcome
  const std::string &GetLabel(void) const { return m_label; }
  /// Sets the text label associated with the outcome 
  void SetLabel(const std::string &p_label) { m_label = p_label; }

  /// Gets the payoff associated with the outcome to player 'pl'
  template <class T> const T &GetPayoff(int pl) const 
    { return (const T &) m_payoffs[pl]; }
  /// Sets the payoff to player 'pl'
  void SetPayoff(int pl, const std::string &p_value)
  {
    m_payoffs[pl] = p_value;
    //m_game->ClearComputedValues();
  }
  //@}
};

typedef GameObjectPtr<GameOutcomeRep> GameOutcome;

/// An action at an information set in an extensive game
class GameActionRep : public GameObject {
protected:
  virtual ~GameActionRep() { }

public:
  virtual int GetNumber(void) const = 0;
  virtual GameInfoset GetInfoset(void) const = 0;

  virtual const std::string &GetLabel(void) const = 0;
  virtual void SetLabel(const std::string &p_label) = 0;

  virtual bool Precedes(const GameNode &) const = 0;

  virtual void DeleteAction(void) = 0;
};

/// An information set in an extensive game
class GameInfosetRep : public GameObject {
protected:
  virtual ~GameInfosetRep() { }

public:
  virtual Game GetGame(void) const = 0;
  virtual int GetNumber(void) const = 0;
  
  virtual GamePlayer GetPlayer(void) const = 0;
  virtual void SetPlayer(GamePlayer p) = 0;

  virtual bool IsChanceInfoset(void) const = 0;

  virtual void SetLabel(const std::string &p_label) = 0;
  virtual const std::string &GetLabel(void) const = 0;
  
  virtual GameAction InsertAction(GameAction p_where = 0) = 0;

  /// @name Actions
  //@{
  /// Returns the number of actions available at the information set
  virtual int NumActions(void) const = 0;
  /// Returns the p_index'th action at the information set
  virtual GameAction GetAction(int p_index) const = 0;
  /// Returns a forward iterator over the available actions
  //virtual GameActionIterator Actions(void) const = 0; 
  //@}

  virtual int NumMembers(void) const = 0;
  virtual GameNode GetMember(int p_index) const = 0;

  virtual bool Precedes(GameNode) const = 0;

  virtual void SetActionProb(int i, const std::string &p_value) = 0;
  virtual double GetActionProb(int pl, double) const = 0;
  virtual Rational GetActionProb(int pl, const Rational &) const = 0;
  virtual std::string GetActionProb(int pl, const std::string &) const = 0;
  virtual void Reveal(GamePlayer) = 0;
};

/// \brief A strategy in a game.
///
/// This class represents a strategy in a game.
/// For strategic games, this object internally stores an offset.  
/// This offset has the
/// property that, for a strategy profile, adding the offsets of the
/// strategies gives the index into the strategic game's table to
/// find the outcome for that strategy profile, making payoff computation
/// relatively efficient.
class GameStrategyRep : public GameObject  {
  friend class GameExplicitRep;
  friend class GameTreeRep;
  friend class GameTableRep;
  friend class GamePlayerRep;
  friend class PureStrategyProfileRep;
  friend class TreePureStrategyProfileRep;
  friend class TablePureStrategyProfileRep;
  template <class T> friend class MixedStrategyProfile;
  template <class T> friend class TableMixedStrategyProfileRep;
  template <class T> friend class MixedBehavProfile;

private:
  int m_number, m_id;
  GamePlayerRep *m_player;
  long m_offset;
  std::string m_label;
  Array<int> m_behav;

  /// @name Lifecycle
  //@{
  /// Creates a new strategy for the given player.
  GameStrategyRep(GamePlayerRep *p_player)
    : m_number(0), m_id(0), m_player(p_player), m_offset(0L) { }
  //@}

public:
  /// @name Data access
  //@{
  /// Returns the text label associated with the strategy
  const std::string &GetLabel(void) const { return m_label; }
  /// Sets the text label associated with the strategy
  void SetLabel(const std::string &p_label) { m_label = p_label; }
  
  /// Returns the player for whom this is a strategy
  GamePlayer GetPlayer(void) const;
  /// Returns the index of the strategy for its player
  int GetNumber(void) const { return m_number; }
  /// Returns the global number of the strategy in the game
  int GetId(void) const { return m_id; }

  /// Remove this strategy from the game
  void DeleteStrategy(void);
  //@}
};

/// A player in a game
class GamePlayerRep : public GameObject {
  friend class GameExplicitRep;
  friend class GameTreeRep;
  friend class GameTableRep;
  friend class GameTreeInfosetRep;
  friend class GameStrategyRep;
  friend class GameTreeNodeRep;
  template <class T> friend class MixedBehavProfile;
  template <class T> friend class MixedStrategyProfile;

  /// @name Building reduced form strategies
  //@{
  void MakeStrategy(void);
  void MakeReducedStrats(GameTreeNodeRep *, GameTreeNodeRep *);
  //@}
  
private:
  GameRep *m_game;
  int m_number;
  std::string m_label;
  Array<GameTreeInfosetRep *> m_infosets;
  Array<GameStrategyRep *> m_strategies;

  GamePlayerRep(GameRep *p_game, int p_id) : m_game(p_game), m_number(p_id)
    { }
  GamePlayerRep(GameRep *p_game, int p_id, int m_strats);
  ~GamePlayerRep();

public:
  int GetNumber(void) const { return m_number; }
  Game GetGame(void) const;
  
  const std::string &GetLabel(void) const { return m_label; }
  void SetLabel(const std::string &p_label) { m_label = p_label; }
  
  bool IsChance(void) const { return (m_number == 0); }

  /// @name Information sets
  //@{
  /// Returns the number of information sets at which the player makes a choice
  int NumInfosets(void) const { return m_infosets.Length(); }
  /// Returns the p_index'th information set
  GameInfoset GetInfoset(int p_index) const;

  /// @name Strategies
  //@{
  /// Returns the number of strategies available to the player
  int NumStrategies(void) const; 
  /// Returns the st'th strategy for the player
  GameStrategy GetStrategy(int st) const;
  /// Returns a forward iterator over the strategies
  GameStrategyIterator Strategies(void) const; 
  /// Creates a new strategy for the player
  GameStrategy NewStrategy(void);
  //@}
};

/// A node in an extensive game
class GameNodeRep : public GameObject {
protected:
  virtual ~GameNodeRep() { }

public:
  virtual Game GetGame(void) const = 0; 

  virtual const std::string &GetLabel(void) const = 0;
  virtual void SetLabel(const std::string &p_label) = 0;

  virtual int GetNumber(void) const = 0;
  virtual int NumberInInfoset(void) const = 0;

  virtual int NumChildren(void) const = 0;

  virtual GameInfoset GetInfoset(void) const = 0;
  virtual void SetInfoset(GameInfoset) = 0;
  virtual GameInfoset LeaveInfoset(void) = 0;

  virtual bool IsTerminal(void) const = 0;
  virtual GamePlayer GetPlayer(void) const = 0;
  virtual GameAction GetPriorAction(void) const = 0;
  virtual GameNode GetChild(int i) const = 0;
  virtual GameNode GetParent(void) const = 0;
  virtual GameNode GetNextSibling(void) const = 0;
  virtual GameNode GetPriorSibling(void) const = 0;

  virtual GameOutcome GetOutcome(void) const = 0;
  virtual void SetOutcome(const GameOutcome &p_outcome) = 0;

  virtual bool IsSuccessorOf(GameNode from) const = 0;
  virtual bool IsSubgameRoot(void) const = 0;

  virtual void DeleteParent(void) = 0;
  virtual void DeleteTree(void) = 0;

  virtual void CopyTree(GameNode src) = 0;
  virtual void MoveTree(GameNode src) = 0;

  virtual GameInfoset AppendMove(GamePlayer p_player, int p_actions) = 0;
  virtual GameInfoset AppendMove(GameInfoset p_infoset) = 0;
  virtual GameInfoset InsertMove(GamePlayer p_player, int p_actions) = 0;
  virtual GameInfoset InsertMove(GameInfoset p_infoset) = 0;
};


/// This class represents a strategy profile on a strategic game.
/// It specifies exactly one strategy for each player defined on the
/// game.
class PureStrategyProfileRep : public GameObject {
  friend class GameTableRep;
  friend class GameTreeRep;

protected:
  Game m_nfg;
  Array<GameStrategy> m_profile;

  /// Construct a new strategy profile
  PureStrategyProfileRep(void) { }

public:
  virtual ~PureStrategyProfileRep() { }
  virtual PureStrategyProfile Copy(void) const = 0;
  
  /// @name Data access and manipulation
  //@{
  /// Get the index uniquely identifying the strategy profile
  virtual long GetIndex(void) const { throw UndefinedException(); }
  /// Get the strategy played by player pl  
  const GameStrategy &GetStrategy(int pl) const { return m_profile[pl]; }
  /// Get the strategy played by the player
  const GameStrategy &GetStrategy(const GamePlayer &p_player) const
    { return m_profile[p_player->GetNumber()]; }
  /// Set the strategy for a player
  virtual void SetStrategy(const GameStrategy &) = 0;

  /// Get the outcome that results from the profile
  virtual GameOutcome GetOutcome(void) const = 0;
  /// Set the outcome that results from the profile
  virtual void SetOutcome(GameOutcome p_outcome) = 0; 

  /// Get the payoff to player pl that results from the profile
  virtual Rational GetPayoff(int pl) const = 0;
  /// Get the payoff to the player resulting from the profile
  Rational GetPayoff(const GamePlayer &p_player) const
  { return GetPayoff(p_player->GetNumber()); }
  /// Get the value of playing strategy against the profile
  virtual Rational GetStrategyValue(const GameStrategy &) const = 0;
  //@}
};

/// This class represents a behavior profile on an extensive game.
/// It specifies exactly one strategy for each information set in the
/// game.
class PureBehavProfile {
private:
  Game m_efg;
  Array<Array<GameAction> > m_profile;

public:
  /// @name Lifecycle
  //@{
  /// Construct a new behavior profile on the specified game
  PureBehavProfile(Game);

  /// @name Data access and manipulation
  //@{
  /// Get the action played at an information set
  GameAction GetAction(const GameInfoset &) const;

  /// Set the action played at an information set
  void SetAction(const GameAction &);
   
  /// Get the payoff to player pl that results from the profile
  template <class T> T GetPayoff(int pl) const;
  /// Get the payoff to the player that results from the profile
  template <class T> T GetPayoff(const GamePlayer &p_player) const
    { return GetPayoff<T>(p_player->GetNumber()); }
  /// Get the payoff to player pl conditional on reaching a node
  template <class T> T GetNodeValue(const GameNode &, int pl) const;
  /// Get the payoff to playing the action, conditional on the profile
  template <class T> T GetActionValue(const GameAction &) const;
  //@}
};


/// This is the class for representing an arbitrary finite game.
class GameRep : public GameObject {
  friend class GameTreeInfosetRep;
  friend class GamePlayerRep;
  friend class GameTreeNodeRep;
  friend class PureStrategyProfileRep;
  friend class TablePureStrategyProfileRep;
  template <class T> friend class MixedBehavProfile;
  template <class T> friend class MixedStrategyProfile;
  template <class T> friend class TableMixedStrategyProfileRep;

protected:
  std::string m_title, m_comment;

  GameRep(void) { }

public:
  /// @name Lifecycle
  //@{
  /// Clean up the game
  virtual ~GameRep() { }
  /// Create a copy of the game, as a new game
  virtual Game Copy(void) const = 0;
  //@}

  /// @name General data access
  //@{
  /// Returns true if the game has a game tree representation
  virtual bool IsTree(void) const = 0;

  /// Get the text label associated with the game
  virtual const std::string &GetTitle(void) const { return m_title; }
  /// Set the text label associated with the game
  virtual void SetTitle(const std::string &p_title) { m_title = p_title; }

  /// Get the text comment associated with the game
  virtual const std::string &GetComment(void) const { return m_comment; }
  /// Set the text comment associated with the game
  virtual void SetComment(const std::string &p_comment) { m_comment = p_comment; }

  /// Returns true if the game is constant-sum
  virtual bool IsConstSum(void) const = 0; 
  /// Returns the smallest payoff in any outcome of the game
  virtual Rational GetMinPayoff(int pl = 0) const = 0;
  /// Returns the largest payoff in any outcome of the game
  virtual Rational GetMaxPayoff(int pl = 0) const = 0;

  /// Returns true if the game is perfect recall.  If not, the specified
  /// a pair of violating information sets is returned in the parameters.  
  virtual bool IsPerfectRecall(GameInfoset &, GameInfoset &) const = 0;
  /// Returns true if the game is perfect recall
  virtual bool IsPerfectRecall(void) const
  { GameInfoset s, t; return IsPerfectRecall(s, t); }
  //@}

  /// @name Managing the representation
  //@{
  /// Renumber all game objects in a canonical way
  virtual void Canonicalize(void) { }  
  /// Clear out any computed values
  virtual void ClearComputedValues(void) const { }
  /// Build any computed values anew
  virtual void BuildComputedValues(void) { }
  /// Have computed values been built?
  virtual bool HasComputedValues(void) const { return false; }
  //@}

  /// @name Writing data files
  //@{
  /// Write the game in .efg format to the specified stream
  virtual void WriteEfgFile(std::ostream &) const
  { throw UndefinedException(); }
  /// Write the subtree starting at node in .efg format to the specified stream
  virtual void WriteEfgFile(std::ostream &, const GameNode &p_node) const
  { throw UndefinedException(); }
  /// Write the game in .nfg format to the specified stream
  virtual void WriteNfgFile(std::ostream &) const
  { throw UndefinedException(); }
  //@}

  /// @name Dimensions of the game
  //@{
  /// The number of actions in each information set
  virtual PVector<int> NumActions(void) const = 0;
  /// The number of members in each information set
  virtual PVector<int> NumMembers(void) const = 0;
  /// The number of strategies for each player
  virtual Array<int> NumStrategies(void) const = 0;
  /// Gets the i'th strategy in the game, numbered globally
  virtual GameStrategy GetStrategy(int p_index) const = 0;
  /// Returns the total number of actions in the game
  virtual int BehavProfileLength(void) const = 0;
  /// Returns the total number of strategies in the game
  virtual int MixedProfileLength(void) const = 0;
  //@}

  virtual PureStrategyProfile NewPureStrategyProfile(void) const = 0;
  virtual MixedStrategyProfile<double> NewMixedStrategyProfile(double) const = 0;
  virtual MixedStrategyProfile<Rational> NewMixedStrategyProfile(const Rational &) const = 0; 

  /// @name Players
  //@{
  /// Returns the number of players in the game
  virtual int NumPlayers(void) const = 0;
  /// Returns the pl'th player in the game
  virtual GamePlayer GetPlayer(int pl) const = 0;
  /// Returns an iterator over the players
  virtual GamePlayerIterator Players(void) const = 0;
  /// Returns the chance (nature) player
  virtual GamePlayer GetChance(void) const = 0;
  /// Creates a new player in the game, with no moves
  virtual GamePlayer NewPlayer(void) = 0;
  //@}

  /// @name Information sets
  //@{
  /// Returns the iset'th information set in the game (numbered globally)
  virtual GameInfoset GetInfoset(int iset) const = 0;
  /// Returns an array with the number of information sets per personal player
  virtual Array<int> NumInfosets(void) const = 0;
  /// Returns the act'th action in the game (numbered globally)
  virtual GameAction GetAction(int act) const = 0;
  //@}

  /// @name Outcomes
  //@{
  /// Returns the number of outcomes defined in the game
  virtual int NumOutcomes(void) const = 0;
  /// Returns the index'th outcome defined in the game
  virtual GameOutcome GetOutcome(int index) const = 0;
  /// Creates a new outcome in the game
  virtual GameOutcome NewOutcome(void) = 0;
  /// Deletes the specified outcome from the game
  virtual void DeleteOutcome(const GameOutcome &) = 0;
  //@}

  /// @name Nodes
  //@{
  /// Returns the root node of the game
  virtual GameNode GetRoot(void) const = 0;
  /// Returns the number of nodes in the game
  virtual int NumNodes(void) const = 0;
  //@}
};

typedef GameObjectPtr<GameRep> Game;

/// Factory function to create new game tree
Game NewTree(void);
/// Factory function to create new game table
Game NewTable(const Array<int> &p_dim, bool p_sparseOutcomes = false);

//=======================================================================
//          Inline members of game representation classes
//=======================================================================

// These must be postponed to here in the file because they require
// all classes to be defined.

inline Game GameOutcomeRep::GetGame(void) const { return m_game; }

inline GamePlayer GameStrategyRep::GetPlayer(void) const { return m_player; }

inline Game GamePlayerRep::GetGame(void) const { return m_game; }
inline int GamePlayerRep::NumStrategies(void) const 
{ m_game->BuildComputedValues(); return m_strategies.Length(); }
inline GameStrategy GamePlayerRep::GetStrategy(int st) const 
{ m_game->BuildComputedValues(); return m_strategies[st]; }
inline GameStrategyIterator GamePlayerRep::Strategies(void) const 
{ m_game->BuildComputedValues(); return GameStrategyIterator(m_strategies); }

template<> inline double PureBehavProfile::GetPayoff(int pl) const
{ return GetNodeValue<double>(m_efg->GetRoot(), pl); }

template<> inline Rational PureBehavProfile::GetPayoff(int pl) const
{ return GetNodeValue<Rational>(m_efg->GetRoot(), pl); }

template<> inline std::string PureBehavProfile::GetPayoff(int pl) const
{ return lexical_cast<std::string>(GetNodeValue<Rational>(m_efg->GetRoot(), pl)); }

//=======================================================================


/// Reads a game in .efg or .nfg format from the input stream
Game ReadGame(std::istream &) throw (InvalidFileException);

} // end namespace gambit

#endif   // LIBGAMBIT_GAME_H
