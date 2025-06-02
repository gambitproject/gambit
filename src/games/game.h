//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
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

#include <list>
#include <set>

#include "number.h"
#include "gameobject.h"

namespace Gambit {

//
// Forward declarations of classes defined in this file.
//
class GameOutcomeRep;
using GameOutcome = GameObjectPtr<GameOutcomeRep>;

class GameActionRep;
using GameAction = GameObjectPtr<GameActionRep>;

class GameInfosetRep;
using GameInfoset = GameObjectPtr<GameInfosetRep>;

class GameStrategyRep;
using GameStrategy = GameObjectPtr<GameStrategyRep>;

class GamePlayerRep;
using GamePlayer = GameObjectPtr<GamePlayerRep>;

class GameNodeRep;
using GameNode = GameObjectPtr<GameNodeRep>;

class GameRep;
using Game = GameObjectPtr<GameRep>;

template <class P, class T> class ElementCollection {
  P m_owner{nullptr};
  const std::vector<T *> *m_container{nullptr};

public:
  class iterator {
    P m_owner{nullptr};
    const std::vector<T *> *m_container{nullptr};
    size_t m_index{0};

  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = GameObjectPtr<T>;
    using pointer = value_type *;
    using reference = value_type &;

    iterator() = default;
    iterator(const P &p_owner, const std::vector<T *> *p_container, size_t p_index = 0)
      : m_owner(p_owner), m_container(p_container), m_index(p_index)
    {
    }
    iterator(const iterator &) = default;
    ~iterator() = default;
    iterator &operator=(const iterator &) = default;

    bool operator==(const iterator &p_iter) const
    {
      return m_owner == p_iter.m_owner && m_container == p_iter.m_container &&
             m_index == p_iter.m_index;
    }
    bool operator!=(const iterator &p_iter) const
    {
      return m_owner != p_iter.m_owner || m_container != p_iter.m_container ||
             m_index != p_iter.m_index;
    }

    iterator &operator++()
    {
      m_index++;
      return *this;
    }
    iterator &operator--()
    {
      m_index--;
      return *this;
    }
    value_type operator*() const { return m_container->at(m_index); }
  };

  ElementCollection() = default;
  explicit ElementCollection(const P &p_owner, const std::vector<T *> *p_container)
    : m_owner(p_owner), m_container(p_container)
  {
  }
  ElementCollection(const ElementCollection<P, T> &) = default;
  ~ElementCollection() = default;
  ElementCollection &operator=(const ElementCollection<P, T> &) = default;

  bool operator==(const ElementCollection<P, T> &p_other) const
  {
    return m_owner == p_other.m_owner && m_container == p_other.m_container;
  }
  size_t size() const { return m_container->size(); }
  GameObjectPtr<T> front() const { return m_container->front(); }
  GameObjectPtr<T> back() const { return m_container->back(); }

  iterator begin() const { return {m_owner, m_container, 0}; }
  iterator end() const { return {m_owner, m_container, (m_owner) ? m_container->size() : 0}; }
  iterator cbegin() const { return {m_owner, m_container, 0}; }
  iterator cend() const { return {m_owner, m_container, (m_owner) ? m_container->size() : 0}; }
};

//
// Forward declarations of classes defined elsewhere.
//
class PureStrategyProfile;
template <class T> class MixedStrategyProfile;
class StrategySupportProfile;

template <class T> class MixedBehaviorProfile;

//=======================================================================
//         Exceptions thrown from game representation classes
//=======================================================================

/// Exception thrown when an operation that is undefined is attempted
class UndefinedException : public Exception {
public:
  UndefinedException() : Exception("Undefined operation on game") {}
  explicit UndefinedException(const std::string &s) : Exception(s) {}
  ~UndefinedException() noexcept override = default;
};

/// Exception thrown on an operation between incompatible objects
class MismatchException : public Exception {
public:
  ~MismatchException() noexcept override = default;
  const char *what() const noexcept override
  {
    return "Operation between objects in different games";
  }
};

/// Exception thrown when comparing different versions of a game
class GameStructureChangedException : public Exception {
public:
  ~GameStructureChangedException() noexcept override = default;
  const char *what() const noexcept override
  {
    return "Game structure has changed since object was defined";
  }
};

/// Exception thrown on a parse error when reading a game savefile
class InvalidFileException : public Exception {
public:
  InvalidFileException() : Exception("File not in a recognized format") {}
  explicit InvalidFileException(const std::string &s) : Exception(s) {}
  ~InvalidFileException() noexcept override = default;
};

//=======================================================================
//             Classes representing objects in a game
//=======================================================================

/// This class represents an outcome in a game.  An outcome
/// specifies a vector of payoffs to players.
class GameOutcomeRep : public GameObject {
  friend class GameExplicitRep;
  friend class GameTreeRep;
  friend class GameTableRep;

  GameRep *m_game;
  int m_number;
  std::string m_label;
  std::map<GamePlayerRep *, Number> m_payoffs;

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

  /// Gets the payoff associated with the outcome to the player
  template <class T> const T &GetPayoff(const GamePlayer &p_player) const;
  /// Sets the payoff to the player
  void SetPayoff(const GamePlayer &p_player, const Number &p_value);
  //@}
};

/// An action at an information set in an extensive game
class GameActionRep : public GameObject {
  friend class GameTreeRep;
  friend class GameInfosetRep;
  template <class T> friend class MixedBehaviorProfile;

  int m_number;
  std::string m_label;
  GameInfosetRep *m_infoset;

  GameActionRep(int p_number, const std::string &p_label, GameInfosetRep *p_infoset)
    : m_number(p_number), m_label(p_label), m_infoset(p_infoset)
  {
  }
  ~GameActionRep() override = default;

public:
  int GetNumber() const { return m_number; }
  GameInfoset GetInfoset() const;

  const std::string &GetLabel() const { return m_label; }
  void SetLabel(const std::string &p_label) { m_label = p_label; }

  bool Precedes(const GameNode &) const;
};

/// An information set in an extensive game
class GameInfosetRep : public GameObject {
  friend class GameTreeRep;
  friend class GamePlayerRep;
  friend class GameNodeRep;
  template <class T> friend class MixedBehaviorProfile;

  GameRep *m_game;
  int m_number;
  std::string m_label;
  GamePlayerRep *m_player;
  std::vector<GameActionRep *> m_actions;
  std::vector<GameNodeRep *> m_members;
  int flag{0}, whichbranch{0};
  std::vector<Number> m_probs;

  GameInfosetRep(GameRep *p_efg, int p_number, GamePlayerRep *p_player, int p_actions);
  ~GameInfosetRep() override;

  void RenumberActions()
  {
    std::for_each(m_actions.begin(), m_actions.end(),
                  [act = 1](GameActionRep *a) mutable { a->m_number = act++; });
  }

public:
  using Actions = ElementCollection<GameInfoset, GameActionRep>;
  using Members = ElementCollection<GameInfoset, GameNodeRep>;

  Game GetGame() const;
  int GetNumber() const { return m_number; }

  GamePlayer GetPlayer() const;

  bool IsChanceInfoset() const;

  void SetLabel(const std::string &p_label) { m_label = p_label; }
  const std::string &GetLabel() const { return m_label; }

  /// @name Actions
  //@{
  /// Returns the p_index'th action at the information set
  GameAction GetAction(int p_index) const { return m_actions.at(p_index - 1); }
  /// Returns the actions available at the information set
  Actions GetActions() const { return Actions(this, &m_actions); }
  //@}

  GameNode GetMember(int p_index) const { return m_members.at(p_index - 1); }
  Members GetMembers() const { return Members(this, &m_members); }

  bool Precedes(GameNode) const;

  const Number &GetActionProb(const GameAction &p_action) const
  {
    if (p_action->GetInfoset() != GameInfoset(const_cast<GameInfosetRep *>(this))) {
      throw MismatchException();
    }
    return m_probs.at(p_action->GetNumber() - 1);
  }
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
class GameStrategyRep : public GameObject {
  friend class GameExplicitRep;
  friend class GameTreeRep;
  friend class GameTableRep;
  friend class GameAGGRep;
  friend class GameBAGGRep;
  friend class GamePlayerRep;
  friend class PureStrategyProfileRep;
  friend class TreePureStrategyProfileRep;
  friend class TablePureStrategyProfileRep;
  friend class StrategySupportProfile;
  template <class T> friend class MixedStrategyProfile;
  template <class T> friend class TableMixedStrategyProfileRep;
  template <class T> friend class MixedBehaviorProfile;

  GamePlayerRep *m_player;
  int m_number;
  long m_offset{-1L};
  std::string m_label;
  Array<int> m_behav;

  /// @name Lifecycle
  //@{
  /// Creates a new strategy for the given player.
  explicit GameStrategyRep(GamePlayerRep *p_player, int p_number, const std::string &p_label)
    : m_player(p_player), m_number(p_number), m_label(p_label)
  {
  }
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
  //@}
};

/// A player in a game
class GamePlayerRep : public GameObject {
  friend class GameRep;
  friend class GameExplicitRep;
  friend class GameTreeRep;
  friend class GameTableRep;
  friend class GameAGGRep;
  friend class GameBAGGRep;
  friend class GameInfosetRep;
  friend class GameNodeRep;
  friend class StrategySupportProfile;
  template <class T> friend class MixedBehaviorProfile;
  template <class T> friend class MixedStrategyProfile;

public:
  using Infosets = ElementCollection<GamePlayer, GameInfosetRep>;
  using Strategies = ElementCollection<GamePlayer, GameStrategyRep>;

private:
  /// @name Building reduced form strategies
  //@{
  void MakeStrategy();
  void MakeReducedStrats(class GameNodeRep *, class GameNodeRep *);
  void MakeNonReducedStrats();
  void MakeNonReducedStratsRecursiveImpl(GameStrategyRep *, Infosets::iterator);
  //@}

  GameRep *m_game;
  int m_number;
  std::string m_label;
  std::vector<GameInfosetRep *> m_infosets;
  std::vector<GameStrategyRep *> m_strategies;
  std::vector<GameStrategyRep *> m_nonReducedStrategies;

  GamePlayerRep(GameRep *p_game, int p_id) : m_game(p_game), m_number(p_id) {}
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
  /// Returns the p_index'th information set
  GameInfoset GetInfoset(int p_index) const { return m_infosets.at(p_index - 1); }
  /// Returns the information sets for the player
  Infosets GetInfosets() const { return Infosets(this, &m_infosets); }

  /// @name Strategies
  //@{
  /// Returns the st'th strategy for the player
  GameStrategy GetStrategy(int st) const;
  /// Returns the array of strategies available to the player
  Strategies GetStrategies() const;
  /// Returns the st'th non-reduced strategy for the player
  GameStrategy GetNonReducedStrategy(int st) const;
  /// Returns the array of non-reduced strategies available to the player
  Strategies GetNonReducedStrategies() const;
  //@}

  /// @name Sequences
  //@{
  /// Returns the number of sequences available to the player
  size_t NumSequences() const;
  //@}
};

/// A node in an extensive game
class GameNodeRep : public GameObject {
  friend class GameTreeRep;
  friend class GameActionRep;
  friend class GameInfosetRep;
  friend class GamePlayerRep;
  friend class PureBehaviorProfile;
  template <class T> friend class MixedBehaviorProfile;

  int m_number{0};
  GameRep *m_game;
  std::string m_label;
  GameInfosetRep *m_infoset{nullptr};
  GameNodeRep *m_parent;
  GameOutcomeRep *m_outcome{nullptr};
  std::vector<GameNodeRep *> m_children;
  GameNodeRep *whichbranch{nullptr}, *ptr{nullptr};

  GameNodeRep(GameRep *e, GameNodeRep *p);
  ~GameNodeRep() override;

  void DeleteOutcome(GameOutcomeRep *outc);

public:
  using Children = ElementCollection<GameNode, GameNodeRep>;

  Game GetGame() const;

  const std::string &GetLabel() const { return m_label; }
  void SetLabel(const std::string &p_label) { m_label = p_label; }

  int GetNumber() const { return m_number; }
  GameNode GetChild(const GameAction &p_action)
  {
    if (p_action->GetInfoset() != m_infoset) {
      throw MismatchException();
    }
    return m_children.at(p_action->GetNumber() - 1);
  }
  Children GetChildren() const { return Children(this, &m_children); }

  GameInfoset GetInfoset() const { return m_infoset; }

  bool IsTerminal() const { return m_children.empty(); }
  GamePlayer GetPlayer() const { return (m_infoset) ? m_infoset->GetPlayer() : nullptr; }
  GameAction GetPriorAction() const; // returns null if root node
  GameNode GetParent() const { return m_parent; }
  GameNode GetNextSibling() const;
  GameNode GetPriorSibling() const;

  GameOutcome GetOutcome() const { return m_outcome; }

  bool IsSuccessorOf(GameNode from) const;
  bool IsSubgameRoot() const;
};

/// This is the class for representing an arbitrary finite game.
class GameRep : public BaseGameRep {
  friend class GameOutcomeRep;
  friend class GameNodeRep;
  friend class PureStrategyProfileRep;
  friend class TablePureStrategyProfileRep;
  template <class T> friend class MixedBehaviorProfile;
  template <class T> friend class MixedStrategyProfile;
  template <class T> friend class TableMixedStrategyProfileRep;

protected:
  std::vector<GamePlayerRep *> m_players;
  std::vector<GameOutcomeRep *> m_outcomes;
  std::string m_title, m_comment;
  unsigned int m_version{0};

  GameRep() = default;

  /// @name Managing the representation
  //@{
  /// Mark that the content of the game has changed
  void IncrementVersion() { m_version++; }
  //@}

public:
  using Players = ElementCollection<Game, GamePlayerRep>;
  using Outcomes = ElementCollection<Game, GameOutcomeRep>;

  /// @name Lifecycle
  //@{
  /// Clean up the game
  ~GameRep() override;

  /// Create a copy of the game, as a new game
  virtual Game Copy() const = 0;
  //@}

  /// @name General data access
  //@{
  /// Returns true if the game has a game tree representation
  virtual bool IsTree() const = 0;

  /// Returns true if the game has an action-graph game representation
  virtual bool IsAgg() const { return false; }

  /// Get the text label associated with the game
  virtual const std::string &GetTitle() const { return m_title; }
  /// Set the text label associated with the game
  virtual void SetTitle(const std::string &p_title) { m_title = p_title; }

  /// Get the text comment associated with the game
  virtual const std::string &GetComment() const { return m_comment; }
  /// Set the text comment associated with the game
  virtual void SetComment(const std::string &p_comment) { m_comment = p_comment; }

  /// Return the version number of the game.  The version is incremented after each
  /// substantive change to the game (i.e. not merely involving labels)
  unsigned int GetVersion() const { return m_version; }

  /// Returns true if the game is constant-sum
  virtual bool IsConstSum() const = 0;
  /// Returns the smallest payoff to any player in any outcome of the game
  virtual Rational GetMinPayoff() const = 0;
  /// Returns the smallest payoff to the player in any outcome of the game
  virtual Rational GetMinPayoff(const GamePlayer &p_player) const = 0;
  /// Returns the largest payoff to any player in any outcome of the game
  virtual Rational GetMaxPayoff() const = 0;
  /// Returns the largest payoff to the player in any outcome of the game
  virtual Rational GetMaxPayoff(const GamePlayer &p_player) const = 0;

  /// Returns the set of terminal nodes which are descendants of node
  virtual std::vector<GameNode> GetPlays(GameNode node) const { throw UndefinedException(); }
  /// Returns the set of terminal nodes which are descendants of members of an infoset
  virtual std::vector<GameNode> GetPlays(GameInfoset infoset) const { throw UndefinedException(); }
  /// Returns the set of terminal nodes which are descendants of members of an action
  virtual std::vector<GameNode> GetPlays(GameAction action) const { throw UndefinedException(); }

  /// Returns true if the game is perfect recall.  If not,
  /// a pair of violating information sets is returned in the parameters.
  virtual bool IsPerfectRecall(GameInfoset &, GameInfoset &) const = 0;
  /// Returns true if the game is perfect recall
  bool IsPerfectRecall() const
  {
    GameInfoset s, t;
    return IsPerfectRecall(s, t);
  }
  //@}

  /// @name Writing data files
  //@{
  /// Write the game to a savefile in the specified format.
  virtual void Write(std::ostream &p_stream, const std::string &p_format = "native") const
  {
    throw UndefinedException();
  }
  /// Write the game in .efg format to the specified stream
  virtual void WriteEfgFile(std::ostream &, const GameNode &subtree = nullptr) const
  {
    throw UndefinedException();
  }
  /// Write the game to a file in .nfg payoff format.
  virtual void WriteNfgFile(std::ostream &p_stream) const;
  //@}

  virtual void SetPlayer(GameInfoset p_infoset, GamePlayer p_player)
  {
    throw UndefinedException();
  }
  virtual GameInfoset AppendMove(GameNode p_node, GamePlayer p_player, int p_actions)
  {
    throw UndefinedException();
  }
  virtual GameInfoset AppendMove(GameNode p_node, GameInfoset p_infoset)
  {
    throw UndefinedException();
  }
  virtual GameInfoset InsertMove(GameNode p_node, GamePlayer p_player, int p_actions)
  {
    throw UndefinedException();
  }
  virtual GameInfoset InsertMove(GameNode p_node, GameInfoset p_infoset)
  {
    throw UndefinedException();
  }
  virtual void DeleteParent(GameNode) { throw UndefinedException(); }
  virtual void DeleteTree(GameNode) { throw UndefinedException(); }
  virtual void CopyTree(GameNode dest, GameNode src) { throw UndefinedException(); }
  /// Create a separate Game object containing the subgame rooted at the node
  virtual Game CopySubgame(GameNode) const { throw UndefinedException(); }
  virtual void MoveTree(GameNode dest, GameNode src) { throw UndefinedException(); }
  virtual void Reveal(GameInfoset, GamePlayer) { throw UndefinedException(); }
  virtual void SetInfoset(GameNode, GameInfoset) { throw UndefinedException(); }
  virtual GameInfoset LeaveInfoset(GameNode) { throw UndefinedException(); }
  virtual GameAction InsertAction(GameInfoset, GameAction p_where = nullptr)
  {
    throw UndefinedException();
  }
  virtual void DeleteAction(GameAction) { throw UndefinedException(); }
  virtual void SetOutcome(GameNode, const GameOutcome &p_outcome) { throw UndefinedException(); }

  /// @name Dimensions of the game
  //@{
  /// The number of strategies for each player
  virtual Array<int> NumStrategies() const = 0;
  /// Gets the i'th strategy in the game, numbered globally
  virtual GameStrategy GetStrategy(int p_index) const = 0;
  /// Creates a new strategy for the player
  virtual GameStrategy NewStrategy(const GamePlayer &p_player, const std::string &p_label)
  {
    throw UndefinedException();
  }
  /// Remove the strategy from the game
  virtual void DeleteStrategy(const GameStrategy &p_strategy) { throw UndefinedException(); }
  /// Returns the number of strategy contingencies in the game
  int NumStrategyContingencies() const
  {
    BuildComputedValues();
    return std::transform_reduce(m_players.begin(), m_players.end(), 0, std::multiplies<>(),
                                 [](const GamePlayerRep *p) { return p->m_strategies.size(); });
  }
  /// Returns the total number of actions in the game
  virtual int BehavProfileLength() const = 0;
  /// Returns the total number of strategies in the game
  int MixedProfileLength() const
  {
    BuildComputedValues();
    return std::transform_reduce(m_players.begin(), m_players.end(), 0, std::plus<>(),
                                 [](const GamePlayerRep *p) { return p->m_strategies.size(); });
  }
  //@}

  virtual PureStrategyProfile NewPureStrategyProfile() const = 0;
  virtual MixedStrategyProfile<double> NewMixedStrategyProfile(double) const = 0;
  virtual MixedStrategyProfile<Rational> NewMixedStrategyProfile(const Rational &) const = 0;

  /// @brief Generate a mixed strategy profile by drawing from the uniform distribution over the
  /// set of
  ///        mixed strategy profiles
  template <class Generator>
  MixedStrategyProfile<double> NewRandomStrategyProfile(Generator &generator) const;
  /// @brief Generate a mixed strategy profile by drawing from the uniform distribution over the
  /// set of
  ///        mixed strategy profiles, restricted to rational probabilities with denominator
  ///        `denom`.
  template <class Generator>
  MixedStrategyProfile<Rational> NewRandomStrategyProfile(int denom, Generator &generator) const;
  virtual MixedStrategyProfile<double>
  NewMixedStrategyProfile(double, const StrategySupportProfile &) const = 0;
  virtual MixedStrategyProfile<Rational>
  NewMixedStrategyProfile(const Rational &, const StrategySupportProfile &) const = 0;

  /// @brief Generate a mixed behavior profile by drawing from the uniform distribution over the
  /// set of
  ///        mixed behavior profiles
  template <class Generator>
  MixedBehaviorProfile<double> NewRandomBehaviorProfile(Generator &generator) const;
  /// @brief Generate a mixed behavior profile by drawing from the uniform distribution over the
  /// set of
  ///        mixed behavior profiles, restricted to rational probabilities with denominator
  ///        `denom`.
  template <class Generator>
  MixedBehaviorProfile<Rational> NewRandomBehaviorProfile(int denom, Generator &generator) const;
  /// @name Players
  //@{
  /// Returns the number of players in the game
  size_t NumPlayers() const { return m_players.size(); }
  /// Returns the pl'th player in the game
  GamePlayer GetPlayer(int pl) const { return m_players.at(pl - 1); }
  /// Returns the set of players in the game
  Players GetPlayers() const { return Players(this, &m_players); }
  /// Returns the chance (nature) player
  virtual GamePlayer GetChance() const = 0;
  /// Creates a new player in the game, with no moves
  virtual GamePlayer NewPlayer() = 0;
  //@}

  /// @name Information sets
  //@{
  /// Returns the iset'th information set in the game (numbered globally)
  virtual GameInfoset GetInfoset(int iset) const { throw UndefinedException(); }
  /// Returns the set of information sets in the game
  virtual std::vector<GameInfoset> GetInfosets() const { throw UndefinedException(); }
  //@}

  /// @name Outcomes
  //@{
  /// Returns the index'th outcome defined in the game
  GameOutcome GetOutcome(int index) const { return m_outcomes.at(index - 1); }
  /// Returns the set of outcomes in the game
  Outcomes GetOutcomes() const { return Outcomes(this, &m_outcomes); }
  /// Creates a new outcome in the game
  virtual GameOutcome NewOutcome() { throw UndefinedException(); }
  /// Deletes the specified outcome from the game
  virtual void DeleteOutcome(const GameOutcome &) { throw UndefinedException(); }
  //@}

  /// @name Nodes
  //@{
  /// Returns the root node of the game
  virtual GameNode GetRoot() const = 0;
  /// Returns the number of nodes in the game
  virtual size_t NumNodes() const = 0;
  /// Returns the number of non-terminal nodes in the game
  virtual size_t NumNonterminalNodes() const = 0;
  //@}

  /// @name Modification
  //@{
  /// Set the probability distribution of actions at a chance node
  virtual Game SetChanceProbs(const GameInfoset &, const Array<Number> &) = 0;
  //@}

  /// Build any computed values anew
  virtual void BuildComputedValues() const {}
};

//=======================================================================
//          Inline members of game representation classes
//=======================================================================

// These must be postponed to here in the file because they require
// all classes to be defined.

inline Game GameOutcomeRep::GetGame() const { return m_game; }

template <class T> const T &GameOutcomeRep::GetPayoff(const GamePlayer &p_player) const
{
  try {
    return static_cast<const T &>(m_payoffs.at(p_player));
  }
  catch (const std::out_of_range &) {
    throw MismatchException();
  }
}

template <> inline const Number &GameOutcomeRep::GetPayoff(const GamePlayer &p_player) const
{
  try {
    return m_payoffs.at(p_player);
  }
  catch (const std::out_of_range &) {
    throw MismatchException();
  }
}

inline void GameOutcomeRep::SetPayoff(const GamePlayer &p_player, const Number &p_value)
{
  if (p_player->GetGame() != GetGame()) {
    throw MismatchException();
  }
  m_payoffs[p_player] = p_value;
  m_game->IncrementVersion();
}

inline GamePlayer GameStrategyRep::GetPlayer() const { return m_player; }

inline Game GameInfosetRep::GetGame() const { return m_game; }
inline GamePlayer GameInfosetRep::GetPlayer() const { return m_player; }
inline bool GameInfosetRep::IsChanceInfoset() const { return m_player->IsChance(); }

inline Game GamePlayerRep::GetGame() const { return m_game; }
inline GameStrategy GamePlayerRep::GetStrategy(int st) const
{
  m_game->BuildComputedValues();
  return m_strategies.at(st - 1);
}
inline GamePlayerRep::Strategies GamePlayerRep::GetStrategies() const
{
  m_game->BuildComputedValues();
  return Strategies(this, &m_strategies);
}
inline GameStrategy GamePlayerRep::GetNonReducedStrategy(int st) const
{
  m_game->BuildComputedValues();
  return m_nonReducedStrategies.at(st - 1);
}
inline GamePlayerRep::Strategies GamePlayerRep::GetNonReducedStrategies() const
{
  m_game->BuildComputedValues();
  return Strategies(this, &m_nonReducedStrategies);
}

inline Game GameNodeRep::GetGame() const { return m_game; }

//=======================================================================

/// Factory function to create new game tree
Game NewTree();
/// Factory function to create new game table
Game NewTable(const std::vector<int> &p_dim, bool p_sparseOutcomes = false);

/// @brief Reads a game representation in .efg format
///
/// @param[in] p_stream An input stream, positioned at the start of the text in .efg format
/// @return A handle to the game representation constructed
/// @throw InvalidFileException If the stream does not contain a valid serialisation
///                             of a game in .efg format.
/// @sa Game::WriteEfgFile, ReadNfgFile, ReadAggFile, ReadBaggFile
Game ReadEfgFile(std::istream &p_stream);

/// @brief Reads a game representation in .nfg format
/// @param[in] p_stream An input stream, positioned at the start of the text in .nfg format
/// @return A handle to the game representation constructed
/// @throw InvalidFileException If the stream does not contain a valid serialisation
///                             of a game in .nfg format.
/// @sa Game::WriteNfgFile, ReadEfgFile, ReadAggFile, ReadBaggFile
Game ReadNfgFile(std::istream &p_stream);

/// @brief Reads a game representation from a graphical interface XML saveflie
/// @param[in] p_stream An input stream, positioned at the start of the text
/// @return A handle to the game representation constructed
/// @throw InvalidFileException If the stream does not contain a valid serialisation
///                             of a game in an XML savefile
/// @sa ReadEfgFile, ReadNfgFile, ReadAggFile, ReadBaggFile
Game ReadGbtFile(std::istream &p_stream);

/// @brief Reads a game from the input stream, attempting to autodetect file format
/// @deprecated Deprecated in favour of the various ReadXXXGame functions.
/// @sa ReadEfgFile, ReadNfgFile, ReadGbtFile, ReadAggFile, ReadBaggFile
Game ReadGame(std::istream &p_stream);

/// @brief Generate a distribution over a simplex restricted to rational numbers of given
/// denominator
template <class Generator>
std::list<Rational> UniformOnSimplex(int p_denom, size_t p_dim, Generator &generator)
{
  // NOLINTBEGIN(misc-const-correctness)
  std::uniform_int_distribution dist(1, p_denom + static_cast<int>(p_dim) - 1);
  // NOLINTEND(misc-const-correctness)
  std::set<int> cutoffs;
  while (cutoffs.size() < p_dim - 1) {
    cutoffs.insert(dist(generator));
  }
  cutoffs.insert(0);
  cutoffs.insert(p_denom + p_dim);
  // The cutoffs need to be sorted in ascending order; this is done automatically by C++ sets
  std::list<Rational> output;
  for (auto prob = cutoffs.cbegin(); std::next(prob) != cutoffs.cend(); prob++) {
    output.emplace_back(*std::next(prob) - *prob - 1, p_denom);
  }
  return output;
}

} // namespace Gambit

#endif // LIBGAMBIT_GAME_H
