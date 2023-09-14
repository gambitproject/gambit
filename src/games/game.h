//
// This file is part of Gambit
// Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
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
#include "core/dvector.h"
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
  GameObject() : m_refCount(0), m_valid(true) { }
  /// Destructor
  virtual ~GameObject() = default;
  //@}

  /// @name Validation
  //@{
  /// Is the object still valid?
  bool IsValid() const { return m_valid; }
  /// Invalidate the object; delete if not referenced elsewhere
  void Invalidate()
  { if (!m_refCount) delete this; else m_valid = false; }
  //@}

  /// @name Reference counting
  //@{
  /// Increment the reference count
  void IncRef() { m_refCount++; }
  /// Decrement the reference count; delete if reference count is zero.
  void DecRef() { if (!--m_refCount && !m_valid) delete this; }
  /// Returns the reference count
  int RefCount() const { return m_refCount; }
  //@}
};


class BaseGameRep {
protected:
    int m_refCount;

public:
    /// @name Lifecycle
    //@{
    /// Constructor; initializes reference count
    BaseGameRep() : m_refCount(0) { }
    /// Destructor
    virtual ~BaseGameRep() = default;
    //@}

    /// @name Validation
    //@{
    /// Is the object still valid?
    bool IsValid() const { return true; }
    /// Invalidate the object; delete if not referenced elsewhere
    void Invalidate()
    { if (!m_refCount) delete this; }
    //@}

    /// @name Reference counting
    //@{
    /// Increment the reference count
    void IncRef() { m_refCount++; }
    /// Decrement the reference count; delete if reference count is zero.
    void DecRef() { if (!--m_refCount) delete this; }
    /// Returns the reference count
    int RefCount() const { return m_refCount; }
    //@}
};

/// An exception thrown when attempting to dereference an invalidated object 
class InvalidObjectException : public Exception {
public:
  ~InvalidObjectException() noexcept override = default;
  const char *what() const noexcept override  { return "Dereferencing an invalidated object"; }
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
  GameObjectPtr(T *r = nullptr) : rep(r)
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

  T *operator->() const 
    { if (!rep) throw NullException();
      if (!rep->IsValid()) throw InvalidObjectException(); 
      return rep; }

  bool operator==(const GameObjectPtr<T> &r) const
  { return (rep == r.rep); }
  bool operator==(T *r) const { return (rep == r); }
  bool operator!=(const GameObjectPtr<T> &r) const 
  { return (rep != r.rep); }
  bool operator!=(T *r) const { return (rep != r); }

  operator T *() const { return rep; }

  bool operator!() const { return !rep; }
};

//
// Forward declarations of classes defined in this file.
//

class GameOutcomeRep;
typedef GameObjectPtr<GameOutcomeRep> GameOutcome;

class GameActionRep;
typedef GameObjectPtr<GameActionRep> GameAction;

class GameInfosetRep;
typedef GameObjectPtr<GameInfosetRep> GameInfoset;
class GameTreeInfosetRep;

class GameStrategyRep;
typedef GameObjectPtr<GameStrategyRep> GameStrategy;

class GamePlayerRep;
typedef GameObjectPtr<GamePlayerRep> GamePlayer;

class GameNodeRep;
typedef GameObjectPtr<GameNodeRep> GameNode;
class GameTreeNodeRep;

class GameRep;
typedef GameObjectPtr<GameRep> Game;

class PureStrategyProfileRep;
class PureStrategyProfile;


// 
// Forward declarations of classes defined elsewhere.
//
template <class T> class MixedStrategyProfile;
template <class T> class MixedBehaviorProfile;
class StrategySupportProfile;

//=======================================================================
//         Exceptions thrown from game representation classes
//=======================================================================

/// Exception thrown when an operation that is undefined is attempted
class UndefinedException : public Exception {
public:
  UndefinedException() : Exception("Undefined operation on game") { }
  explicit UndefinedException(const std::string &s) : Exception(s) { }
  ~UndefinedException() noexcept override = default;
};

/// Exception thrown on an operation between incompatible objects
class MismatchException : public Exception {
public:
  ~MismatchException() noexcept override = default;
  const char *what() const noexcept override  
  { return "Operation between objects in different games"; }
};

/// Exception thrown on a parse error when reading a game savefile
class InvalidFileException : public Exception {
public:
  InvalidFileException() : Exception("File not in a recognized format") { }
  explicit InvalidFileException(const std::string &s) : Exception(s) { }
  ~InvalidFileException() noexcept override = default;
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
  GameOutcome m_unrestricted;

  /// @name Lifecycle
  //@{
  /// Creates a new outcome object, with payoffs set to zero
  GameOutcomeRep(GameRep *p_game, int p_number);
  ~GameOutcomeRep() override = default;
  //@}

public:
  /// @name Data access
  //@{
  /// Returns the strategic game on which the outcome is defined.
  Game GetGame() const;
  /// Returns the index number of the outcome
  int GetNumber() const { return m_number; }

  /// Returns the text label associated with the outcome
  const std::string &GetLabel() const { return m_label; }
  /// Sets the text label associated with the outcome
  void SetLabel(const std::string &p_label) { m_label = p_label; }

  /// Gets the payoff associated with the outcome to player 'pl'
  const Number &GetPayoff(int pl) const { return m_payoffs[pl]; }
  /// Sets the payoff to player 'pl'
  void SetPayoff(int pl, const Number &p_value)
    { m_payoffs[pl] = p_value; }

  /// Map the outcome to the corresponding outcome in the unrestricted game
  GameOutcome Unrestrict() const 
  { if (m_unrestricted) return m_unrestricted; else throw UndefinedException(); }
  //@}
};

typedef GameObjectPtr<GameOutcomeRep> GameOutcome;

/// An action at an information set in an extensive game
class GameActionRep : public GameObject {
protected:
  GameAction m_unrestricted;

  GameActionRep() : m_unrestricted(nullptr) { }
  ~GameActionRep() override = default;

public:
  virtual int GetNumber() const = 0;
  virtual GameInfoset GetInfoset() const = 0;

  virtual const std::string &GetLabel() const = 0;
  virtual void SetLabel(const std::string &p_label) = 0;

  virtual bool Precedes(const GameNode &) const = 0;

  virtual void DeleteAction() = 0;

  /// Map the action to the corresponding action in the unrestricted game
  GameAction Unrestrict() const 
  { if (m_unrestricted) return m_unrestricted; else throw UndefinedException(); }

};

/// An information set in an extensive game
class GameInfosetRep : public GameObject {
protected:
  GameInfoset m_unrestricted;
 
  GameInfosetRep(): m_unrestricted(nullptr) { }
  ~GameInfosetRep() override = default;

public:
  virtual Game GetGame() const = 0;
  virtual int GetNumber() const = 0;
  
  virtual GamePlayer GetPlayer() const = 0;
  virtual void SetPlayer(GamePlayer p) = 0;

  virtual bool IsChanceInfoset() const = 0;

  virtual void SetLabel(const std::string &p_label) = 0;
  virtual const std::string &GetLabel() const = 0;
  
  virtual GameAction InsertAction(GameAction p_where = nullptr) = 0;

  /// @name Actions
  //@{
  /// Returns the number of actions available at the information set
  virtual int NumActions() const = 0;
  /// Returns the p_index'th action at the information set
  virtual GameAction GetAction(int p_index) const = 0;
  /// Returns a forward iterator over the available actions
  //virtual GameActionIterator Actions(void) const = 0; 
  //@}

  virtual int NumMembers() const = 0;
  virtual GameNode GetMember(int p_index) const = 0;

  virtual bool Precedes(GameNode) const = 0;

  virtual const Number &GetActionProb(int i) const = 0;
  virtual void Reveal(GamePlayer) = 0;

  /// Map the infoset to the corresponding infoset in the unrestricted game
  GameInfoset Unrestrict() const 
  { if (m_unrestricted) return m_unrestricted; else throw UndefinedException(); }
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
  friend class GameAggRep;
  friend class GameBagentRep;
  friend class GamePlayerRep;
  friend class PureStrategyProfileRep;
  friend class TreePureStrategyProfileRep;
  friend class TablePureStrategyProfileRep;
  friend class StrategySupportProfile;
  template <class T> friend class MixedStrategyProfile;
  template <class T> friend class TableMixedStrategyProfileRep;
  template <class T> friend class MixedBehaviorProfile;

private:
  int m_number, m_id;
  GamePlayerRep *m_player;
  long m_offset;
  std::string m_label;
  Array<int> m_behav;
  GameStrategy m_unrestricted;

  /// @name Lifecycle
  //@{
  /// Creates a new strategy for the given player.
  explicit GameStrategyRep(GamePlayerRep *p_player)
    : m_number(0), m_id(0), m_player(p_player), m_offset(0L), m_unrestricted(nullptr) { }
  //@}

public:
  /// @name Data access
  //@{
  /// Returns the text label associated with the strategy
  const std::string &GetLabel() const { return m_label; }
  /// Sets the text label associated with the strategy
  void SetLabel(const std::string &p_label) { m_label = p_label; }
  
  /// Returns the player for whom this is a strategy
  GamePlayer GetPlayer() const;
  /// Returns the index of the strategy for its player
  int GetNumber() const { return m_number; }
  /// Returns the global number of the strategy in the game
  int GetId() const { return m_id; }

  /// Remove this strategy from the game
  void DeleteStrategy();

  /// Map the strategy to the corresponding strategy in the unrestricted game
  GameStrategy Unrestrict() const
  { if (m_unrestricted) return m_unrestricted; else throw UndefinedException(); }
  //@}
};

typedef Array<GameStrategyRep *> GameStrategyArray;

/// A player in a game
class GamePlayerRep : public GameObject {
  friend class GameExplicitRep;
  friend class GameTreeRep;
  friend class GameTableRep;
  friend class GameAggRep;
  friend class GameBagentRep;
  friend class GameBaggRep;
  friend class GameTreeInfosetRep;
  friend class GameStrategyRep;
  friend class GameTreeNodeRep;
  friend class StrategySupportProfile;
  template <class T> friend class MixedBehaviorProfile;
  template <class T> friend class MixedStrategyProfile;

  /// @name Building reduced form strategies
  //@{
  void MakeStrategy();
  void MakeReducedStrats(GameTreeNodeRep *, GameTreeNodeRep *);
  //@}
  
private:
  GameRep *m_game;
  int m_number;
  std::string m_label;
  Array<GameTreeInfosetRep *> m_infosets;
  GameStrategyArray m_strategies;
  GamePlayer m_unrestricted;

  GamePlayerRep(GameRep *p_game, int p_id) 
    : m_game(p_game), m_number(p_id), m_unrestricted(nullptr) { }
  GamePlayerRep(GameRep *p_game, int p_id, int m_strats);
  ~GamePlayerRep() override;

public:
  int GetNumber() const { return m_number; }
  Game GetGame() const;
  
  const std::string &GetLabel() const { return m_label; }
  void SetLabel(const std::string &p_label) { m_label = p_label; }
  
  bool IsChance() const { return (m_number == 0); }

  /// @name Information sets
  //@{
  /// Returns the number of information sets at which the player makes a choice
  int NumInfosets() const { return m_infosets.size(); }
  /// Returns the p_index'th information set
  GameInfoset GetInfoset(int p_index) const;

  /// @name Strategies
  //@{
  /// Returns the number of strategies available to the player
  int NumStrategies() const; 
  /// Returns the st'th strategy for the player
  GameStrategy GetStrategy(int st) const;
  /// Returns the array of strategies available to the player
  const GameStrategyArray &Strategies() const;
  /// Creates a new strategy for the player
  GameStrategy NewStrategy();
  //@}

  /// Map the player to the corresponding player in the unrestricted game
  GamePlayer Unrestrict() const 
  { if (m_unrestricted) return m_unrestricted; else throw UndefinedException(); }

};

/// A node in an extensive game
class GameNodeRep : public GameObject {
protected:
  GameNode m_unrestricted;

  GameNodeRep() : m_unrestricted(nullptr) { }
  ~GameNodeRep() override = default;

public:
  virtual Game GetGame() const = 0; 

  virtual const std::string &GetLabel() const = 0;
  virtual void SetLabel(const std::string &p_label) = 0;

  virtual int GetNumber() const = 0;
  virtual int NumberInInfoset() const = 0;

  virtual int NumChildren() const = 0;

  virtual GameInfoset GetInfoset() const = 0;
  virtual void SetInfoset(GameInfoset) = 0;
  virtual GameInfoset LeaveInfoset() = 0;

  virtual bool IsTerminal() const = 0;
  virtual GamePlayer GetPlayer() const = 0;
  virtual GameAction GetPriorAction() const = 0;
  virtual GameNode GetChild(int i) const = 0;
  virtual GameNode GetParent() const = 0;
  virtual GameNode GetNextSibling() const = 0;
  virtual GameNode GetPriorSibling() const = 0;

  virtual GameOutcome GetOutcome() const = 0;
  virtual void SetOutcome(const GameOutcome &p_outcome) = 0;

  virtual bool IsSuccessorOf(GameNode from) const = 0;
  virtual bool IsSubgameRoot() const = 0;

  virtual void DeleteParent() = 0;
  virtual void DeleteTree() = 0;

  virtual void CopyTree(GameNode src) = 0;
  virtual void MoveTree(GameNode src) = 0;

  /// Create a separate Game object containing the subgame rooted at the node
  virtual Game CopySubgame() const = 0;

  virtual GameInfoset AppendMove(GamePlayer p_player, int p_actions) = 0;
  virtual GameInfoset AppendMove(GameInfoset p_infoset) = 0;
  virtual GameInfoset InsertMove(GamePlayer p_player, int p_actions) = 0;
  virtual GameInfoset InsertMove(GameInfoset p_infoset) = 0;

  /// Map the node to the corresponding node in the unrestricted game
  GameNode Unrestrict() const 
  { if (m_unrestricted) return m_unrestricted; else throw UndefinedException(); }
};


/// This class represents a strategy profile on a strategic game.
/// It specifies exactly one strategy for each player defined on the
/// game.
class PureStrategyProfileRep {
  friend class GameTableRep;
  friend class GameTreeRep;
  friend class GameAggRep;
  friend class PureStrategyProfile;

protected:
  Game m_nfg;
  Array<GameStrategy> m_profile;

  /// Construct a new strategy profile
  explicit PureStrategyProfileRep(const Game &p_game);

  /// Create a copy of the strategy profile.
  /// Caller is responsible for memory management of the created object.
  virtual PureStrategyProfileRep *Copy() const = 0;

public:
  virtual ~PureStrategyProfileRep() = default;
  
  /// @name Data access and manipulation
  //@{
  /// Get the index uniquely identifying the strategy profile
  virtual long GetIndex() const { throw UndefinedException(); }
  /// Get the strategy played by player pl  
  const GameStrategy &GetStrategy(int pl) const { return m_profile[pl]; }
  /// Get the strategy played by the player
  const GameStrategy &GetStrategy(const GamePlayer &p_player) const
    { return m_profile[p_player->GetNumber()]; }
  /// Set the strategy for a player
  virtual void SetStrategy(const GameStrategy &) = 0;

  /// Get the outcome that results from the profile
  virtual GameOutcome GetOutcome() const = 0;
  /// Set the outcome that results from the profile
  virtual void SetOutcome(GameOutcome p_outcome) = 0; 

  /// Get the payoff to player pl that results from the profile
  virtual Rational GetPayoff(int pl) const = 0;
  /// Get the payoff to the player resulting from the profile
  Rational GetPayoff(const GamePlayer &p_player) const
  { return GetPayoff(p_player->GetNumber()); }
  /// Get the value of playing strategy against the profile
  virtual Rational GetStrategyValue(const GameStrategy &) const = 0;

  /// Is the profile a pure strategy Nash equilibrium?
  bool IsNash() const;

  /// Is the profile a strict pure stategy Nash equilibrium?
  bool IsStrictNash() const;

  /// Is the specificed player playing a best response?
  bool IsBestResponse(const GamePlayer &p_player) const;

  /// Get the list of best response strategies for a player
  List<GameStrategy> GetBestResponse(const GamePlayer &p_player) const;

  /// Convert to a mixed strategy representation
  MixedStrategyProfile<Rational> ToMixedStrategyProfile() const;

  /// Map strategy profile to the unrestriction of the game
  PureStrategyProfile Unrestrict() const;

  //@}
};

class PureStrategyProfile {
private:
  PureStrategyProfileRep *rep;

public:
  PureStrategyProfile(const PureStrategyProfile &r) : rep(r.rep->Copy())  { }
  explicit PureStrategyProfile(PureStrategyProfileRep *p_rep) : rep(p_rep) { }
  ~PureStrategyProfile() { delete rep; }

  PureStrategyProfile &operator=(const PureStrategyProfile &r) 
    {
      if (&r != this) {
	delete rep;
	rep = r.rep->Copy();
      }
      return *this;
    }

  PureStrategyProfileRep *operator->() const { return rep; }
  explicit operator PureStrategyProfileRep *() const { return rep; }
};
    

/// This class represents a behavior profile on an extensive game.
/// It specifies exactly one strategy for each information set in the
/// game.
class PureBehaviorProfile {
private:
  Game m_efg;
  Array<Array<GameAction> > m_profile;

public:
  /// @name Lifecycle
  //@{
  /// Construct a new behavior profile on the specified game
  explicit PureBehaviorProfile(Game);

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
  template <class T> T GetPayoff(const GameNode &, int pl) const;
  /// Get the payoff to playing the action, conditional on the profile
  template <class T> T GetPayoff(const GameAction &) const;

  /// Is the profile a pure strategy agent Nash equilibrium?
  bool IsAgentNash() const;

  /// Convert to a mixed behavior representation
  MixedBehaviorProfile<Rational> ToMixedBehaviorProfile() const;
  //@}
};


/// This is the class for representing an arbitrary finite game.
class GameRep : public BaseGameRep {
  friend class GameTreeInfosetRep;
  friend class GamePlayerRep;
  friend class GameTreeNodeRep;
  friend class PureStrategyProfileRep;
  friend class TablePureStrategyProfileRep;
  template <class T> friend class MixedBehaviorProfile;
  template <class T> friend class MixedStrategyProfile;
  template <class T> friend class TableMixedStrategyProfileRep;

protected:
  std::string m_title, m_comment;

  GameRep() = default;

  /// @name Managing the representation
  //@{
  /// Renumber all game objects in a canonical way
  virtual void Canonicalize() { }  
  /// Clear out any computed values
  virtual void ClearComputedValues() const { }
  /// Build any computed values anew
  virtual void BuildComputedValues() { }
  /// Have computed values been built?
  virtual bool HasComputedValues() const { return false; }
  //@}

public:
  class Players {
    friend class GameRep;
  public:
    class iterator {
    private:
      Game m_game;
      int m_index;
    public:
      iterator(Game &p_game, int p_index)
        : m_game(p_game), m_index(p_index)  { }
      GamePlayer operator*()  { return m_game->GetPlayer(m_index); }
      GamePlayer operator->()  { return m_game->GetPlayer(m_index); }
      iterator &operator++()  { m_index++; return *this; }
      bool operator==(const iterator &it) const
      { return (m_game == it.m_game) && (m_index == it.m_index); }
      bool operator!=(const iterator &it) const
      { return !(*this == it); }
    };

    class const_iterator {
    private:
      const Game m_game;
      int m_index;
    public:
      const_iterator(const Game &p_game, int p_index)
        : m_game(p_game), m_index(p_index)  { }
      const GamePlayer operator*()  { return m_game->GetPlayer(m_index); }
      const GamePlayer operator->()  { return m_game->GetPlayer(m_index); }
      const_iterator &operator++()  { m_index++; return *this; }
      bool operator==(const const_iterator &it) const
      { return (m_game == it.m_game) && (m_index == it.m_index); }
      bool operator!=(const const_iterator &it) const
      { return !(*this == it); }
    };

    /// Return a forward iterator starting at the beginning of the array
    iterator begin()  { return iterator(m_game, 1); }
    /// Return a forward iterator past the end of the array
    iterator end()    { return iterator(m_game, m_game->NumPlayers() + 1); }
    /// Return a const forward iterator starting at the beginning of the array
    const_iterator begin() const { return const_iterator(m_game, 1); }
    /// Return a const forward iterator past the end of the array
    const_iterator end() const   { return const_iterator(m_game, m_game->NumPlayers()); }
    /// Return a const forward iterator starting at the beginning of the array
    const_iterator cbegin() const { return const_iterator(m_game, 1); }
    /// Return a const forward iterator past the end of the array
    const_iterator cend() const   { return const_iterator(m_game, m_game->NumPlayers()); }

  private:
    Game m_game;

    Players(Game p_game) : m_game(p_game) { }
  };


  /// @name Lifecycle
  //@{
  /// Clean up the game
  ~GameRep() override = default;
  /// Create a copy of the game, as a new game
  virtual Game Copy() const = 0;
  //@}

  /// @name General data access
  //@{
  /// Returns true if the game has a game tree representation
  virtual bool IsTree() const = 0;

  /// Returns true if the game has a action-graph game representation
  virtual bool IsAgg() const { return false; }

  /// Returns true if the game is a restriction of a more general game
  virtual bool IsRestriction() const { return false; }
  /// Returns the unrestricted version of the game
  virtual Game Unrestrict() const { throw UndefinedException(); }

  /// Get the text label associated with the game
  virtual const std::string &GetTitle() const { return m_title; }
  /// Set the text label associated with the game
  virtual void SetTitle(const std::string &p_title) { m_title = p_title; }

  /// Get the text comment associated with the game
  virtual const std::string &GetComment() const { return m_comment; }
  /// Set the text comment associated with the game
  virtual void SetComment(const std::string &p_comment) { m_comment = p_comment; }

  /// Returns true if the game is constant-sum
  virtual bool IsConstSum() const = 0; 
  /// Returns the smallest payoff in any outcome of the game
  virtual Rational GetMinPayoff(int pl = 0) const = 0;
  /// Returns the largest payoff in any outcome of the game
  virtual Rational GetMaxPayoff(int pl = 0) const = 0;

  /// Returns true if the game is perfect recall.  If not, the specified
  /// a pair of violating information sets is returned in the parameters.  
  virtual bool IsPerfectRecall(GameInfoset &, GameInfoset &) const = 0;
  /// Returns true if the game is perfect recall
  virtual bool IsPerfectRecall() const
  { GameInfoset s, t; return IsPerfectRecall(s, t); }
  //@}

  /// @name Writing data files
  //@{
  /// Write the game to a savefile in the specified format.
  virtual void Write(std::ostream &p_stream,
		     const std::string &p_format="native") const
  { throw UndefinedException(); }
  /// Write the game to a file in .nfg payoff format.
  virtual void WriteNfgFile(std::ostream &p_stream) const;
  //@}

  /// @name Dimensions of the game
  //@{
  /// The number of actions in each information set
  virtual PVector<int> NumActions() const = 0;
  /// The number of members in each information set
  virtual PVector<int> NumMembers() const = 0;
  /// The number of strategies for each player
  virtual Array<int> NumStrategies() const = 0;
  /// Gets the i'th strategy in the game, numbered globally
  virtual GameStrategy GetStrategy(int p_index) const = 0;
  /// Returns the number of strategy contingencies in the game
  virtual int NumStrategyContingencies() const = 0;
  /// Returns the total number of actions in the game
  virtual int BehavProfileLength() const = 0;
  /// Returns the total number of strategies in the game
  virtual int MixedProfileLength() const = 0;
  //@}

  virtual PureStrategyProfile NewPureStrategyProfile() const = 0;
  virtual MixedStrategyProfile<double> NewMixedStrategyProfile(double) const = 0;
  virtual MixedStrategyProfile<Rational> NewMixedStrategyProfile(const Rational &) const = 0; 
  virtual MixedStrategyProfile<double> NewMixedStrategyProfile(double, const StrategySupportProfile&) const = 0;
  virtual MixedStrategyProfile<Rational> NewMixedStrategyProfile(const Rational &, const StrategySupportProfile&) const = 0;

  /// @name Players
  //@{
  /// Returns the number of players in the game
  virtual int NumPlayers() const = 0;
  /// Returns the pl'th player in the game
  virtual GamePlayer GetPlayer(int pl) const = 0;
  /// Returns the set of players in the game 
  Players GetPlayers()
  { return Players(Game(this)); }
  /// Returns the chance (nature) player
  virtual GamePlayer GetChance() const = 0;
  /// Creates a new player in the game, with no moves
  virtual GamePlayer NewPlayer() = 0;
  //@}

  /// @name Information sets
  //@{
  /// Returns the iset'th information set in the game (numbered globally)
  virtual GameInfoset GetInfoset(int iset) const = 0;
  /// Returns an array with the number of information sets per personal player
  virtual Array<int> NumInfosets() const = 0;
  /// Returns the act'th action in the game (numbered globally)
  virtual GameAction GetAction(int act) const = 0;
  //@}

  /// @name Outcomes
  //@{
  /// Returns the number of outcomes defined in the game
  virtual int NumOutcomes() const = 0;
  /// Returns the index'th outcome defined in the game
  virtual GameOutcome GetOutcome(int index) const = 0;
  /// Creates a new outcome in the game
  virtual GameOutcome NewOutcome() = 0;
  /// Deletes the specified outcome from the game
  virtual void DeleteOutcome(const GameOutcome &) = 0;
  //@}

  /// @name Nodes
  //@{
  /// Returns the root node of the game
  virtual GameNode GetRoot() const = 0;
  /// Returns the number of nodes in the game
  virtual int NumNodes() const = 0;
  //@}

  /// @name Modification
  //@{
  /// Set the probability distribution of actions at a chance node
  virtual Game SetChanceProbs(const GameInfoset &, const Array<Number> &) = 0;
  //@}
};

typedef GameObjectPtr<GameRep> Game;

/// Factory function to create new game tree
Game NewTree();
/// Factory function to create new game table
Game NewTable(const Array<int> &p_dim, bool p_sparseOutcomes = false);

//=======================================================================
//          Inline members of game representation classes
//=======================================================================

// These must be postponed to here in the file because they require
// all classes to be defined.

inline Game GameOutcomeRep::GetGame() const { return m_game; }

inline GamePlayer GameStrategyRep::GetPlayer() const { return m_player; }

inline Game GamePlayerRep::GetGame() const { return m_game; }
inline int GamePlayerRep::NumStrategies() const 
{ m_game->BuildComputedValues(); return m_strategies.size(); }
inline GameStrategy GamePlayerRep::GetStrategy(int st) const 
{ m_game->BuildComputedValues(); return m_strategies[st]; }
inline const GameStrategyArray &GamePlayerRep::Strategies() const
{ m_game->BuildComputedValues(); return m_strategies; }

template<> inline double PureBehaviorProfile::GetPayoff(int pl) const
{ return GetPayoff<double>(m_efg->GetRoot(), pl); }

template<> inline Rational PureBehaviorProfile::GetPayoff(int pl) const
{ return GetPayoff<Rational>(m_efg->GetRoot(), pl); }

template<> inline std::string PureBehaviorProfile::GetPayoff(int pl) const
{ return lexical_cast<std::string>(GetPayoff<Rational>(m_efg->GetRoot(), pl)); }

//=======================================================================


/// Reads a game in .efg or .nfg format from the input stream
Game ReadGame(std::istream &);

} // end namespace gambit

#endif   // LIBGAMBIT_GAME_H
