//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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
#include <list>
#include <set>
#include <random>

#include "core/dvector.h"
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

private:
  GameRep *m_game;
  int m_number;
  std::string m_label;
  Array<Number> m_payoffs;

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
  /// Gets the payoff associated with the outcome to the player
  const Number &GetPayoff(const GamePlayer &p_player) const;
  /// Sets the payoff to player 'pl'
  void SetPayoff(int pl, const Number &p_value);
  /// Sets the payoff to the player
  void SetPayoff(const GamePlayer &p_player, const Number &p_value);
  //@}
};

/// An action at an information set in an extensive game
class GameActionRep : public GameObject {
protected:
  GameActionRep() = default;
  ~GameActionRep() override = default;

public:
  virtual int GetNumber() const = 0;
  virtual GameInfoset GetInfoset() const = 0;

  virtual const std::string &GetLabel() const = 0;
  virtual void SetLabel(const std::string &p_label) = 0;

  virtual bool Precedes(const GameNode &) const = 0;

  virtual void DeleteAction() = 0;
};

/// An information set in an extensive game
class GameInfosetRep : public GameObject {
protected:
  GameInfosetRep() = default;
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
  /// Returns the actions available at the information set
  virtual Array<GameAction> GetActions() const = 0;
  //@}

  virtual int NumMembers() const = 0;
  virtual GameNode GetMember(int p_index) const = 0;
  virtual Array<GameNode> GetMembers() const = 0;

  virtual bool Precedes(GameNode) const = 0;

  virtual const Number &GetActionProb(int i) const = 0;
  virtual const Number &GetActionProb(const GameAction &) const = 0;
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

private:
  int m_number, m_id;
  GamePlayerRep *m_player;
  long m_offset;
  std::string m_label;
  Array<int> m_behav;

  /// @name Lifecycle
  //@{
  /// Creates a new strategy for the given player.
  explicit GameStrategyRep(GamePlayerRep *p_player)
    : m_number(0), m_id(0), m_player(p_player), m_offset(0L)
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
  /// Returns the global number of the strategy in the game
  int GetId() const { return m_id; }

  /// Remove this strategy from the game
  void DeleteStrategy();
  //@}
};

/// A player in a game
class GamePlayerRep : public GameObject {
  friend class GameExplicitRep;
  friend class GameTreeRep;
  friend class GameTableRep;
  friend class GameAGGRep;
  friend class GameBAGGRep;
  friend class GameTreeInfosetRep;
  friend class GameStrategyRep;
  friend class GameTreeNodeRep;
  friend class StrategySupportProfile;
  template <class T> friend class MixedBehaviorProfile;
  template <class T> friend class MixedStrategyProfile;

  /// @name Building reduced form strategies
  //@{
  void MakeStrategy();
  void MakeReducedStrats(class GameTreeNodeRep *, class GameTreeNodeRep *);
  //@}

private:
  GameRep *m_game;
  int m_number;
  std::string m_label;
  Array<class GameTreeInfosetRep *> m_infosets;
  Array<GameStrategyRep *> m_strategies;

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
  /// Returns the number of information sets at which the player makes a choice
  int NumInfosets() const { return m_infosets.size(); }
  /// Returns the p_index'th information set
  GameInfoset GetInfoset(int p_index) const;
  /// Returns the information sets for the players
  Array<GameInfoset> GetInfosets() const;

  /// @name Strategies
  //@{
  /// Returns the number of strategies available to the player
  int NumStrategies() const;
  /// Returns the st'th strategy for the player
  GameStrategy GetStrategy(int st) const;
  /// Returns the array of strategies available to the player
  Array<GameStrategy> GetStrategies() const;
  /// Creates a new strategy for the player
  GameStrategy NewStrategy();
  //@}

  /// @name Sequences
  //@{
  /// Returns the number of sequences available to the player
  int NumSequences() const;
  //@}
};

/// A node in an extensive game
class GameNodeRep : public GameObject {
protected:
  GameNodeRep() = default;
  ~GameNodeRep() override = default;

public:
  virtual Game GetGame() const = 0;

  virtual const std::string &GetLabel() const = 0;
  virtual void SetLabel(const std::string &p_label) = 0;

  virtual int GetNumber() const = 0;
  virtual int NumberInInfoset() const = 0;

  virtual int NumChildren() const = 0;
  virtual GameNode GetChild(int i) const = 0;
  virtual GameNode GetChild(const GameAction &) const = 0;
  virtual Array<GameNode> GetChildren() const = 0;

  virtual GameInfoset GetInfoset() const = 0;
  virtual void SetInfoset(GameInfoset) = 0;
  virtual GameInfoset LeaveInfoset() = 0;

  virtual bool IsTerminal() const = 0;
  virtual GamePlayer GetPlayer() const = 0;
  virtual GameAction GetPriorAction() const = 0;
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
};

/// This is the class for representing an arbitrary finite game.
class GameRep : public BaseGameRep {
  friend class GameOutcomeRep;
  friend class GameTreeInfosetRep;
  friend class GameTreeActionRep;
  friend class GameStrategyRep;
  friend class GamePlayerRep;
  friend class GameTreeNodeRep;
  friend class PureStrategyProfileRep;
  friend class TablePureStrategyProfileRep;
  template <class T> friend class MixedBehaviorProfile;
  template <class T> friend class MixedStrategyProfile;
  template <class T> friend class TableMixedStrategyProfileRep;

protected:
  std::string m_title, m_comment;
  unsigned int m_version;

  GameRep() : m_version(0) {}

  /// @name Managing the representation
  //@{
  /// Mark that the content of the game has changed
  void IncrementVersion() { m_version++; }
  /// Build any computed values anew
  virtual void BuildComputedValues() {}

public:
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
  /// Returns the smallest payoff in any outcome of the game
  virtual Rational GetMinPayoff(int pl = 0) const = 0;
  /// Returns the largest payoff in any outcome of the game
  virtual Rational GetMaxPayoff(int pl = 0) const = 0;

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
  virtual void WriteEfgFile(std::ostream &) const { throw UndefinedException(); }
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
  /// Gets the set of strategies in the game
  Array<GameStrategy> GetStrategies() const;
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
  virtual int NumPlayers() const = 0;
  /// Returns the pl'th player in the game
  virtual GamePlayer GetPlayer(int pl) const = 0;
  /// Returns the set of players in the game
  virtual Array<GamePlayer> GetPlayers() const;
  /// Returns the chance (nature) player
  virtual GamePlayer GetChance() const = 0;
  /// Creates a new player in the game, with no moves
  virtual GamePlayer NewPlayer() = 0;
  //@}

  /// @name Information sets
  //@{
  /// Returns the iset'th information set in the game (numbered globally)
  virtual GameInfoset GetInfoset(int iset) const = 0;
  /// Returns the set of information sets in the game
  virtual Array<GameInfoset> GetInfosets() const = 0;
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

//=======================================================================
//          Inline members of game representation classes
//=======================================================================

// These must be postponed to here in the file because they require
// all classes to be defined.

inline Game GameOutcomeRep::GetGame() const { return m_game; }
inline const Number &GameOutcomeRep::GetPayoff(const GamePlayer &p_player) const
{
  if (p_player->GetGame() != GetGame()) {
    throw MismatchException();
  }
  return m_payoffs[p_player->GetNumber()];
}

inline void GameOutcomeRep::SetPayoff(int pl, const Number &p_value)
{
  m_game->IncrementVersion();
  m_payoffs[pl] = p_value;
}

inline void GameOutcomeRep::SetPayoff(const GamePlayer &p_player, const Number &p_value)
{
  if (p_player->GetGame() != GetGame()) {
    throw MismatchException();
  }
  m_game->IncrementVersion();
  m_payoffs[p_player->GetNumber()] = p_value;
}

inline GamePlayer GameStrategyRep::GetPlayer() const { return m_player; }

inline Game GamePlayerRep::GetGame() const { return m_game; }
inline int GamePlayerRep::NumStrategies() const
{
  m_game->BuildComputedValues();
  return m_strategies.size();
}
inline GameStrategy GamePlayerRep::GetStrategy(int st) const
{
  m_game->BuildComputedValues();
  return m_strategies[st];
}

//=======================================================================

/// Factory function to create new game tree
Game NewTree();
/// Factory function to create new game table
Game NewTable(const Array<int> &p_dim, bool p_sparseOutcomes = false);
/// Reads a game in .efg or .nfg format from the input stream
Game ReadGame(std::istream &);

/// @brief Generate a distribution over a simplex restricted to rational numbers of given
/// denominator
template <class Generator>
std::list<Rational> UniformOnSimplex(int p_denom, size_t p_dim, Generator &generator)
{
  std::uniform_int_distribution dist(1, p_denom + int(p_dim) - 1);
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
