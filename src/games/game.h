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
#include <stack>
#include <memory>

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
using Game = std::shared_ptr<GameRep>;

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
class UndefinedException final : public std::runtime_error {
public:
  UndefinedException() : std::runtime_error("Undefined operation on game") {}
  explicit UndefinedException(const std::string &s) : std::runtime_error(s) {}
  ~UndefinedException() noexcept override = default;
};

/// Exception thrown on an operation between incompatible objects
class MismatchException : public std::runtime_error {
public:
  MismatchException() : std::runtime_error("Operation between objects in different games") {}
  ~MismatchException() noexcept override = default;
};

/// Exception thrown when comparing different versions of a game
class GameStructureChangedException : public std::runtime_error {
public:
  GameStructureChangedException()
    : std::runtime_error("Game structure has changed since object was defined")
  {
  }
  ~GameStructureChangedException() noexcept override = default;
};

/// Exception thrown on a parse error when reading a game savefile
class InvalidFileException : public std::runtime_error {
public:
  InvalidFileException() : std::runtime_error("File not in a recognized format") {}
  explicit InvalidFileException(const std::string &s) : std::runtime_error(s) {}
  ~InvalidFileException() noexcept override = default;
};

//=======================================================================
//             Classes representing objects in a game
//=======================================================================

/// This class represents an outcome in a game.  An outcome
/// specifies a vector of payoffs to players.
class GameOutcomeRep : public std::enable_shared_from_this<GameOutcomeRep> {
  friend class GameExplicitRep;
  friend class GameTreeRep;
  friend class GameTableRep;

  bool m_valid{true};
  GameRep *m_game;
  int m_number;
  std::string m_label;
  std::map<GamePlayerRep *, Number> m_payoffs;

public:
  /// @name Lifecycle
  //@{
  /// Creates a new outcome object, with payoffs set to zero
  GameOutcomeRep(GameRep *p_game, int p_number);
  ~GameOutcomeRep() = default;
  //@}

  bool IsValid() const { return m_valid; }
  void Invalidate() { m_valid = false; }

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
class GameActionRep : public std::enable_shared_from_this<GameActionRep> {
  friend class GameTreeRep;
  friend class GameInfosetRep;
  template <class T> friend class MixedBehaviorProfile;

  bool m_valid{true};
  int m_number;
  std::string m_label;
  GameInfosetRep *m_infoset;

public:
  GameActionRep(int p_number, const std::string &p_label, GameInfosetRep *p_infoset)
    : m_number(p_number), m_label(p_label), m_infoset(p_infoset)
  {
  }
  ~GameActionRep() = default;

  bool IsValid() const { return m_valid; }
  void Invalidate() { m_valid = false; }

  int GetNumber() const { return m_number; }
  Game GetGame() const;
  GameInfoset GetInfoset() const;

  const std::string &GetLabel() const { return m_label; }
  void SetLabel(const std::string &p_label) { m_label = p_label; }

  bool Precedes(const GameNode &) const;
};

/// An information set in an extensive game
class GameInfosetRep : public std::enable_shared_from_this<GameInfosetRep> {
  friend class GameTreeRep;
  friend class GamePlayerRep;
  friend class GameNodeRep;
  template <class T> friend class MixedBehaviorProfile;

  bool m_valid{true};
  GameRep *m_game;
  int m_number;
  std::string m_label;
  GamePlayerRep *m_player;
  std::vector<std::shared_ptr<GameActionRep>> m_actions;
  std::vector<std::shared_ptr<GameNodeRep>> m_members;
  std::vector<Number> m_probs;

  void RenumberActions()
  {
    std::for_each(
        m_actions.begin(), m_actions.end(),
        [act = 1](const std::shared_ptr<GameActionRep> &a) mutable { a->m_number = act++; });
  }

public:
  using Actions = ElementCollection<GameInfoset, GameActionRep>;
  using Members = ElementCollection<GameInfoset, GameNodeRep>;

  GameInfosetRep(GameRep *p_efg, int p_number, GamePlayerRep *p_player, int p_actions);
  ~GameInfosetRep();

  bool IsValid() const { return m_valid; }
  void Invalidate() { m_valid = false; }

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
  Actions GetActions() const
  {
    return Actions(std::const_pointer_cast<GameInfosetRep>(shared_from_this()), &m_actions);
  }
  //@}

  GameNode GetMember(int p_index) const { return m_members.at(p_index - 1); }
  Members GetMembers() const
  {
    return Members(std::const_pointer_cast<GameInfosetRep>(shared_from_this()), &m_members);
  }

  bool Precedes(GameNode) const;

  const Number &GetActionProb(const GameAction &p_action) const
  {
    if (p_action->GetInfoset().get() != this) {
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
class GameStrategyRep : public std::enable_shared_from_this<GameStrategyRep> {
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

  bool m_valid{true};
  GamePlayerRep *m_player;
  int m_number;
  long m_offset{-1L};
  std::string m_label;
  std::map<GameInfosetRep *, int> m_behav;

public:
  /// @name Lifecycle
  //@{
  /// Creates a new strategy for the given player.
  explicit GameStrategyRep(GamePlayerRep *p_player, int p_number, const std::string &p_label)
    : m_player(p_player), m_number(p_number), m_label(p_label)
  {
  }
  //@}

  bool IsValid() const { return m_valid; }
  void Invalidate() { m_valid = false; }

  /// @name Data access
  //@{
  /// Returns the text label associated with the strategy
  const std::string &GetLabel() const { return m_label; }
  /// Sets the text label associated with the strategy
  void SetLabel(const std::string &p_label) { m_label = p_label; }

  /// Returns the game on which the strategy is defined
  Game GetGame() const;
  /// Returns the player for whom this is a strategy
  GamePlayer GetPlayer() const;
  /// Returns the index of the strategy for its player
  int GetNumber() const { return m_number; }

  /// Returns the action specified by the strategy at the information set
  GameAction GetAction(const GameInfoset &) const;
  //@}
};

/// A player in a game
class GamePlayerRep : public std::enable_shared_from_this<GamePlayerRep> {
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

  /// @name Building reduced form strategies
  //@{
  void MakeStrategy(const std::map<GameInfosetRep *, int> &);
  void MakeReducedStrats(class GameNodeRep *, class GameNodeRep *,
                         std::map<GameInfosetRep *, int> &,
                         std::map<GameNodeRep *, GameNodeRep *> &ptr,
                         std::map<GameNodeRep *, GameNodeRep *> &whichbranch);
  //@}

  bool m_valid{true};
  GameRep *m_game;
  int m_number;
  std::string m_label;
  std::vector<std::shared_ptr<GameInfosetRep>> m_infosets;
  std::vector<std::shared_ptr<GameStrategyRep>> m_strategies;

public:
  using Infosets = ElementCollection<GamePlayer, GameInfosetRep>;
  using Strategies = ElementCollection<GamePlayer, GameStrategyRep>;

  GamePlayerRep(GameRep *p_game, int p_id) : m_game(p_game), m_number(p_id) {}
  GamePlayerRep(GameRep *p_game, int p_id, int m_strats);
  ~GamePlayerRep();

  bool IsValid() const { return m_valid; }
  void Invalidate() { m_valid = false; }

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
  Infosets GetInfosets() const
  {
    return Infosets(std::const_pointer_cast<GamePlayerRep>(shared_from_this()), &m_infosets);
  }

  /// @name Strategies
  //@{
  /// Returns the st'th strategy for the player
  GameStrategy GetStrategy(int st) const;
  /// Returns the array of strategies available to the player
  Strategies GetStrategies() const;
  //@}

  /// @name Sequences
  //@{
  /// Returns the number of sequences available to the player
  size_t NumSequences() const;
  //@}
};

/// A node in an extensive game
class GameNodeRep : public std::enable_shared_from_this<GameNodeRep> {
  friend class GameTreeRep;
  friend class GameActionRep;
  friend class GameInfosetRep;
  friend class GamePlayerRep;
  friend class PureBehaviorProfile;
  template <class T> friend class MixedBehaviorProfile;

  bool m_valid{true};
  int m_number{0};
  GameRep *m_game;
  std::string m_label;
  GameInfosetRep *m_infoset{nullptr};
  GameNodeRep *m_parent;
  GameOutcomeRep *m_outcome{nullptr};
  std::vector<std::shared_ptr<GameNodeRep>> m_children;

  void DeleteOutcome(GameOutcomeRep *outc);

public:
  using Children = ElementCollection<GameNode, GameNodeRep>;

  /// @brief A range class for iterating over a node's (action, child) pairs.
  class Actions {
  private:
    const GameNodeRep *m_owner{nullptr};

  public:
    class iterator;

    Actions(const GameNodeRep *p_owner);

    iterator begin() const;
    iterator end() const;
  };

  GameNodeRep(GameRep *e, GameNodeRep *p);
  ~GameNodeRep();

  bool IsValid() const { return m_valid; }
  void Invalidate() { m_valid = false; }

  /// @brief Returns a collection for iterating over this node's (action, child) pairs.
  Actions GetActions() const;

  Game GetGame() const;

  const std::string &GetLabel() const { return m_label; }
  void SetLabel(const std::string &p_label) { m_label = p_label; }

  int GetNumber() const { return m_number; }
  GameNode GetChild(const GameAction &p_action)
  {
    if (p_action->GetInfoset().get() != m_infoset) {
      throw MismatchException();
    }
    return m_children.at(p_action->GetNumber() - 1);
  }
  Children GetChildren() const
  {
    return Children(std::const_pointer_cast<GameNodeRep>(shared_from_this()), &m_children);
  }

  GameInfoset GetInfoset() const { return (m_infoset) ? m_infoset->shared_from_this() : nullptr; }

  bool IsTerminal() const { return m_children.empty(); }
  GamePlayer GetPlayer() const { return (m_infoset) ? m_infoset->GetPlayer() : nullptr; }
  GameAction GetPriorAction() const; // returns null if root node
  GameNode GetParent() const { return (m_parent) ? m_parent->shared_from_this() : nullptr; }
  GameNode GetNextSibling() const;
  GameNode GetPriorSibling() const;

  GameOutcome GetOutcome() const { return (m_outcome) ? m_outcome->shared_from_this() : nullptr; }

  bool IsSuccessorOf(GameNode from) const;
  bool IsSubgameRoot() const;
};

class GameNodeRep::Actions::iterator {
public:
  /// @name Iterator
  //@{
  using iterator_category = std::forward_iterator_tag;
  using value_type = std::pair<GameAction, GameNode>;
  using difference_type = std::ptrdiff_t;
  using pointer = value_type *;
  using reference = value_type;
  //@}

private:
  /// @brief An iterator to the action at the parent's information set.
  GameInfosetRep::Actions::iterator m_action_it;
  /// @brief An iterator to the child node.
  GameNodeRep::Children::iterator m_child_it;

public:
  /// @name Lifecycle
  //@{
  /// Default constructor. Creates an iterator in a past-the-end state.
  iterator() = default;

  /// Creates a new iterator that zips an action iterator and a child iterator.
  iterator(GameInfosetRep::Actions::iterator p_action_it,
           GameNodeRep::Children::iterator p_child_it);
  //@}

  /// @name Iterator Operations
  //@{
  /// Returns the current action-child pair.
  reference operator*() const { return {*m_action_it, *m_child_it}; }

  /// Advances the iterator to the next pair (pre-increment).
  iterator &operator++()
  {
    ++m_action_it;
    ++m_child_it;
    return *this;
  }

  /// Advances the iterator to the next pair (post-increment).
  iterator operator++(int)
  {
    iterator tmp = *this;
    ++(*this);
    return tmp;
  }

  /// Compares two iterators for equality.
  bool operator==(const iterator &p_other) const
  {
    // Comparing one of the wrapped iterators is sufficient as they move in lockstep.
    return m_child_it == p_other.m_child_it;
  }

  /// Compares two iterators for inequality.
  bool operator!=(const iterator &p_other) const { return !(*this == p_other); }
  //@}

  GameNode GetOwner() const;
};

inline GameNodeRep::Actions::Actions(const GameNodeRep *p_owner) : m_owner(p_owner) {}

inline GameNodeRep::Actions GameNodeRep::GetActions() const { return {Actions(this)}; }

inline GameNodeRep::Actions::iterator GameNodeRep::Actions::begin() const
{
  if (m_owner->IsTerminal()) {
    return end();
  }
  return {m_owner->GetInfoset()->GetActions().begin(), m_owner->GetChildren().begin()};
}

inline GameNodeRep::Actions::iterator GameNodeRep::Actions::end() const
{
  if (m_owner->IsTerminal()) {
    return {};
  }
  return {m_owner->GetInfoset()->GetActions().end(), m_owner->GetChildren().end()};
}

inline GameNodeRep::Actions::iterator::iterator(GameInfosetRep::Actions::iterator p_action_it,
                                                GameNodeRep::Children::iterator p_child_it)
  : m_action_it(p_action_it), m_child_it(p_child_it)
{
}

inline GameNode GameNodeRep::Actions::iterator::GetOwner() const { return m_child_it.GetOwner(); }

/// This is the class for representing an arbitrary finite game.
class GameRep : public std::enable_shared_from_this<GameRep> {
  friend class GameOutcomeRep;
  friend class GameNodeRep;
  friend class PureStrategyProfileRep;
  friend class TablePureStrategyProfileRep;
  template <class T> friend class MixedBehaviorProfile;
  template <class T> friend class MixedStrategyProfile;
  template <class T> friend class TableMixedStrategyProfileRep;

protected:
  std::vector<std::shared_ptr<GamePlayerRep>> m_players;
  std::vector<std::shared_ptr<GameOutcomeRep>> m_outcomes;
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

  class Nodes {
    Game m_owner{nullptr};

  public:
    class iterator {
      friend class Nodes;
      using ChildIterator = ElementCollection<GameNode, GameNodeRep>::iterator;

      Game m_owner{nullptr};
      GameNode m_current_node{nullptr};
      std::stack<std::pair<ChildIterator, ChildIterator>> m_stack{};

      iterator(const Game &game) : m_owner(game) {}

    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = GameNode;
      using pointer = value_type *;

      iterator() = default;

      iterator(const Game &game, const GameNode &start_node)
        : m_owner(game), m_current_node(start_node)
      {
        if (!start_node) {
          return;
        }
        if (start_node->GetGame() != m_owner) {
          throw MismatchException();
        }
      }

      value_type operator*() const
      {
        if (!m_current_node) {
          throw std::runtime_error("Cannot dereference an end iterator");
        }
        return m_current_node;
      }

      iterator &operator++()
      {
        if (!m_current_node) {
          throw std::out_of_range("Cannot increment an end iterator");
        }

        if (!m_current_node->IsTerminal()) {
          auto children = m_current_node->GetChildren();
          m_stack.emplace(children.begin(), children.end());
        }

        while (!m_stack.empty()) {
          auto &[current_it, end_it] = m_stack.top();

          if (current_it != end_it) {
            m_current_node = *current_it;
            ++current_it;
            return *this;
          }
          m_stack.pop();
        }

        m_current_node = nullptr;
        return *this;
      }

      bool operator==(const iterator &other) const
      {
        return m_owner == other.m_owner && m_current_node == other.m_current_node;
      }
      bool operator!=(const iterator &other) const { return !(*this == other); }
    };

    Nodes() = default;
    explicit Nodes(const Game &p_owner) : m_owner(p_owner) {}

    iterator begin() const
    {
      return (m_owner) ? iterator{m_owner, m_owner->GetRoot()} : iterator{};
    }
    iterator end() const { return (m_owner) ? iterator{m_owner} : iterator{}; }
  };

  /// @name Lifecycle
  //@{
  /// Clean up the game
  virtual ~GameRep();

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

  /// Returns true if the game is perfect recall
  virtual bool IsPerfectRecall() const = 0;
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
    return std::transform_reduce(
        m_players.begin(), m_players.end(), 0, std::multiplies<>(),
        [](const std::shared_ptr<GamePlayerRep> &p) { return p->m_strategies.size(); });
  }
  /// Returns the total number of actions in the game
  virtual int BehavProfileLength() const = 0;
  /// Returns the total number of strategies in the game
  int MixedProfileLength() const
  {
    BuildComputedValues();
    return std::transform_reduce(
        m_players.begin(), m_players.end(), 0, std::plus<>(),
        [](const std::shared_ptr<GamePlayerRep> &p) { return p->m_strategies.size(); });
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
  GamePlayer GetPlayer(int pl) const { return m_players.at(pl - 1)->shared_from_this(); }
  /// Returns the set of players in the game
  Players GetPlayers() const
  {
    return Players(std::const_pointer_cast<GameRep>(shared_from_this()), &m_players);
  }
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
  /// Sort the information sets for each player in a canonical order
  virtual void SortInfosets() {}
  //@}

  /// @name Outcomes
  //@{
  /// Returns the index'th outcome defined in the game
  GameOutcome GetOutcome(int index) const { return m_outcomes.at(index - 1); }
  /// Returns the set of outcomes in the game
  Outcomes GetOutcomes() const
  {
    return Outcomes(std::const_pointer_cast<GameRep>(shared_from_this()), &m_outcomes);
  }
  /// Creates a new outcome in the game
  virtual GameOutcome NewOutcome() { throw UndefinedException(); }
  /// Deletes the specified outcome from the game
  virtual void DeleteOutcome(const GameOutcome &) { throw UndefinedException(); }
  //@}

  /// @name Nodes
  //@{
  /// Returns the root node of the game
  virtual GameNode GetRoot() const = 0;
  /// Returns a range that can be used to iterate over the nodes of the game
  Nodes GetNodes() const { return Nodes(std::const_pointer_cast<GameRep>(shared_from_this())); }
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

inline Game GameOutcomeRep::GetGame() const { return m_game->shared_from_this(); }

template <class T> const T &GameOutcomeRep::GetPayoff(const GamePlayer &p_player) const
{
  try {
    return static_cast<const T &>(m_payoffs.at(p_player.get()));
  }
  catch (const std::out_of_range &) {
    throw MismatchException();
  }
}

template <> inline const Number &GameOutcomeRep::GetPayoff(const GamePlayer &p_player) const
{
  try {
    return m_payoffs.at(p_player.get());
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
  m_payoffs[p_player.get()] = p_value;
  m_game->IncrementVersion();
}

inline GamePlayer GameStrategyRep::GetPlayer() const { return m_player->shared_from_this(); }
inline Game GameStrategyRep::GetGame() const { return m_player->GetGame(); }

inline Game GameActionRep::GetGame() const { return m_infoset->GetGame(); }

inline Game GameInfosetRep::GetGame() const { return m_game->shared_from_this(); }
inline GamePlayer GameInfosetRep::GetPlayer() const { return m_player->shared_from_this(); }
inline bool GameInfosetRep::IsChanceInfoset() const { return m_player->IsChance(); }

inline Game GamePlayerRep::GetGame() const { return m_game->shared_from_this(); }
inline GameStrategy GamePlayerRep::GetStrategy(int st) const
{
  m_game->BuildComputedValues();
  return m_strategies.at(st - 1);
}
inline GamePlayerRep::Strategies GamePlayerRep::GetStrategies() const
{
  m_game->BuildComputedValues();
  return Strategies(std::const_pointer_cast<GamePlayerRep>(shared_from_this()), &m_strategies);
}

inline Game GameNodeRep::GetGame() const { return m_game->shared_from_this(); }

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
