//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/games/gametree.h
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

#ifndef GAMBIT_GAMES_GAMETREE_H
#define GAMBIT_GAMES_GAMETREE_H

#include "core/lazy.h"
#include "gameexpl.h"
#include "seqpure.h"
#include "stratmixed.h"
#include <unordered_map>

namespace Gambit {

//=======================================================================
//  Tree-only game object classes (relocated from game.h)
//=======================================================================

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
  int GetNumber() const;

  GamePlayer GetPlayer() const;

  bool IsChanceInfoset() const;

  void SetLabel(const std::string &p_label);
  /// Validate that p_label is a nonempty, valid label for an action of this
  /// information set, unique among its actions disregarding any in p_ignore.
  void CheckActionLabel(const std::string &p_label,
                        const std::set<const GameActionRep *> &p_ignore) const;
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
  /// Returns the sequences corresponding to the actions available at the
  /// information set, in the same order as GetActions().
  std::vector<GameSequence> GetSequences() const;
  //@}

  GameNode GetMember(int p_index) const;
  Members GetMembers() const;

  bool Precedes(GameNode) const;

  std::set<GameAction> GetOwnPriorActions() const;

  const Number &GetActionProb(const GameAction &p_action) const
  {
    if (p_action->GetInfoset().get() != this) {
      throw MismatchException();
    }
    return m_probs.at(p_action->GetNumber() - 1);
  }
};

class GameSequenceRep : public std::enable_shared_from_this<GameSequenceRep> {
  friend class GameTreeRep;
  friend class BehaviorSupportProfile;

  bool m_valid{true};
  GamePlayerRep *m_player;
  GameActionRep *m_action;
  size_t m_number;
  std::weak_ptr<GameSequenceRep> m_parent;

public:
  explicit GameSequenceRep(GamePlayerRep *p_player, GameActionRep *p_action, size_t p_number,
                           const std::weak_ptr<GameSequenceRep> &p_parent)
    : m_player(p_player), m_action(p_action), m_number(p_number), m_parent(p_parent)
  {
  }

  bool IsValid() const { return m_valid; }
  void Invalidate() { m_valid = false; }

  Game GetGame() const;
  GamePlayer GetPlayer() const;
  GameInfoset GetInfoset() const { return (m_action) ? m_action->GetInfoset() : nullptr; }
  GameAction GetAction() const { return (m_action) ? m_action->shared_from_this() : nullptr; }
  GameSequence GetParent() const { return m_parent.lock(); }

  std::strong_ordering operator<=>(const GameSequenceRep &other) const
  {
    if (const auto cmp = m_player <=> other.m_player; cmp != 0) {
      return cmp;
    }
    return m_action <=> other.m_action;
  }
  bool operator==(const GameSequenceRep &other) const
  {
    return m_player == other.m_player && m_action == other.m_action;
  }
};

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
  GameOutcomeRep *m_outcome;
  std::vector<std::shared_ptr<GameNodeRep>> m_children;

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
  void SetLabel(const std::string &p_label);

  int GetNumber() const;
  GameNode GetChild(const GameAction &p_action)
  {
    if (p_action->GetInfoset().get() != m_infoset) {
      throw MismatchException("Action is from a different information set than node");
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
  GameAction GetOwnPriorAction() const;
  GameNode GetParent() const { return (m_parent) ? m_parent->shared_from_this() : nullptr; }
  GameNode GetNextSibling() const;
  GameNode GetPriorSibling() const;

  GameOutcome GetOutcome() const { return m_outcome->shared_from_this(); }

  bool IsSuccessorOf(GameNode from) const;
  bool IsSubgameRoot() const;
  bool IsStrategyReachable() const;
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

class GameSubgameRep : public std::enable_shared_from_this<GameSubgameRep> {
  friend class GameTreeRep;

  bool m_valid{true};
  GameRep *m_game;
  GameNodeRep *m_root;
  std::weak_ptr<GameSubgameRep> m_parent;
  std::vector<std::shared_ptr<GameSubgameRep>> m_children;
  std::vector<std::shared_ptr<GameInfosetRep>> m_subgameDifference;

public:
  using SubgameCollection = ElementCollection<GameSubgame, GameSubgameRep>;
  using InfosetCollection = ElementCollection<GameSubgame, GameInfosetRep>;

  GameSubgameRep(GameRep *p_game, GameNodeRep *p_root) : m_game(p_game), m_root(p_root) {}
  ~GameSubgameRep() = default;

  bool IsValid() const { return m_valid; }
  void Invalidate() { m_valid = false; }

  Game GetGame() const;
  GameNode GetRoot() const { return m_root->shared_from_this(); }

  GameSubgame GetParent() const;
  SubgameCollection GetChildren() const;
  InfosetCollection GetSubgameDifference() const;
};

//=======================================================================
//  Tree-only GameRep members (relocated from game.h)
//=======================================================================

class GameRep::Nodes {
  Game m_owner{nullptr};
  TraversalOrder m_order{TraversalOrder::Preorder};

public:
  class iterator {
    friend class Nodes;

    using ChildIterator = ElementCollection<GameNode, GameNodeRep>::iterator;

    // One frame per node currently open on the path from the root; this mirrors
    // WalkDFS's own stack, but is kept alive across calls to advance() (instead of
    // living inside a single call to WalkDFS) so the walk can pause after each node
    // and resume from exactly where it left off, rather than visiting the whole tree
    // up front.
    struct Frame {
      GameNode m_node;
      ChildIterator m_current;
      ChildIterator m_end;
      bool m_entered{false};
    };

    Game m_owner{nullptr};
    TraversalOrder m_order{TraversalOrder::Preorder};
    std::stack<Frame> m_stack;
    GameNode m_current{nullptr};

    iterator(const Game &p_game, TraversalOrder p_order) : m_owner(p_game), m_order(p_order)
    {
      if (!p_game) {
        m_owner = nullptr;
        return;
      }
      m_stack.push(Frame{p_game->GetRoot(), {}, {}, false});
      advance();
    }

    void advance()
    {
      while (!m_stack.empty()) {
        Frame &f = m_stack.top();

        if (!f.m_entered) {
          f.m_entered = true;
          if (!f.m_node->IsTerminal()) {
            auto children = f.m_node->GetChildren();
            f.m_current = children.begin();
            f.m_end = children.end();
          }
          if (m_order == TraversalOrder::Preorder) {
            m_current = f.m_node;
            return;
          }
        }

        if (!f.m_node->IsTerminal() && f.m_current != f.m_end) {
          GameNode const child = *f.m_current;
          ++f.m_current;
          m_stack.push(Frame{child, {}, {}, false});
          continue;
        }

        GameNode const finished = f.m_node;
        m_stack.pop();
        if (m_order == TraversalOrder::Postorder) {
          m_current = finished;
          return;
        }
      }
      m_current = nullptr;
      m_owner = nullptr;
    }

  public:
    using iterator_category = std::input_iterator_tag;
    using value_type = GameNode;
    using difference_type = std::ptrdiff_t;
    using reference = GameNode;
    using pointer = GameNode;

    iterator() = default;

    value_type operator*() const
    {
      if (!m_current) {
        throw std::runtime_error("Dereferencing end iterator");
      }
      return m_current;
    }
    iterator &operator++()
    {
      advance();
      return *this;
    }
    bool operator==(const iterator &p_other) const
    {
      return m_owner == p_other.m_owner && m_current == p_other.m_current;
    }
  };

  Nodes() = default;
  Nodes(const Game &p_owner, const TraversalOrder p_order = TraversalOrder::Preorder)
    : m_owner(p_owner), m_order(p_order)
  {
  }

  iterator begin() const { return m_owner ? iterator{m_owner, m_order} : iterator{}; }
  static iterator end() { return iterator{}; }
};

template <class Callback>
void GameRep::WalkDFS(const Game &p_game, const GameNode &p_root, TraversalOrder p_order,
                      Callback &p_callback)
{
  using ChildIterator = ElementCollection<GameNode, GameNodeRep>::iterator;

  struct Frame {
    GameNode m_node;
    ChildIterator m_current;
    ChildIterator m_end;
    int m_depth{};
    bool m_entered{}, m_pruned{};
  };

  if (!p_root) {
    return;
  }
  if (p_root->GetGame() != p_game) {
    throw MismatchException();
  }

  std::stack<Frame> stack;
  stack.push(Frame{p_root, {}, {}, 0, false, false});

  while (!stack.empty()) {
    Frame &f = stack.top();

    if (!f.m_entered) {
      f.m_entered = true;
      const DFSCallbackResult result = p_callback.OnEnter(f.m_node, f.m_depth);
      if (result == DFSCallbackResult::Stop) {
        return;
      }
      if (result == DFSCallbackResult::Prune) {
        f.m_pruned = true;
      }
      if (p_order == TraversalOrder::Preorder) {
        p_callback.OnVisit(f.m_node, f.m_depth);
      }
      if (!f.m_pruned && !f.m_node->IsTerminal()) {
        auto children = f.m_node->GetChildren();
        f.m_current = children.begin();
        f.m_end = children.end();
      }
    }

    if (!f.m_pruned && !f.m_node->IsTerminal() && f.m_current != f.m_end) {
      GameNode const child = *f.m_current;
      ++f.m_current;
      const DFSCallbackResult result = p_callback.OnAction(f.m_node, child, f.m_depth);

      if (result == DFSCallbackResult::Stop) {
        return;
      }
      if (result == DFSCallbackResult::Prune) {
        continue;
      }
      stack.push(Frame{child, {}, {}, f.m_depth + 1, false, false});
      continue;
    }

    const DFSCallbackResult result = p_callback.OnExit(f.m_node, f.m_depth);
    if (result == DFSCallbackResult::Stop) {
      return;
    }
    if (p_order == TraversalOrder::Postorder) {
      p_callback.OnVisit(f.m_node, f.m_depth);
    }
    stack.pop();
  }
}

inline GameRep::Nodes GameRep::GetNodes(TraversalOrder p_traversal) const
{
  return {std::const_pointer_cast<GameRep>(shared_from_this()), p_traversal};
}
inline auto GameRep::GetTerminalNodes() const
{
  return filter_if(GetNodes(), [](const auto &node) -> bool { return node->IsTerminal(); });
}
inline auto GameRep::GetNonterminalNodes(TraversalOrder p_traversal) const
{
  return filter_if(GetNodes(p_traversal),
                   [](const auto &node) -> bool { return !node->IsTerminal(); });
}
inline bool GameRep::IsAbsentMinded(const GameInfoset &p_infoset) const
{
  if (p_infoset->GetGame().get() != this) {
    throw MismatchException();
  }
  return false;
}

//=======================================================================
//  Inline members of tree-only game object classes (relocated from game.h)
//=======================================================================

inline void
GamePlayerRep::CheckInfosetLabel(const std::string &p_label,
                                 const std::set<const GameInfosetRep *> &p_ignore) const
{
  CheckLabel(p_label);
  // Infoset labels may be empty; a nonempty label must be unique among the infosets of the player.
  if (p_label.empty()) {
    return;
  }
  for (const auto &infoset : m_infosets) {
    if (!p_ignore.contains(infoset.get()) && infoset->GetLabel() == p_label) {
      throw ValueException("Infoset label must be unique for the player");
    }
  }
}
inline Game GameSequenceRep::GetGame() const { return m_player->GetGame(); }
inline GamePlayer GameSequenceRep::GetPlayer() const { return m_player->shared_from_this(); }
inline Game GameActionRep::GetGame() const { return m_infoset->GetGame(); }
inline Game GameInfosetRep::GetGame() const { return m_game->shared_from_this(); }
inline GamePlayer GameInfosetRep::GetPlayer() const { return m_player->shared_from_this(); }
inline void GameInfosetRep::CheckActionLabel(const std::string &p_label,
                                             const std::set<const GameActionRep *> &p_ignore) const
{
  if (p_label.empty()) {
    throw ValueException("Action label must not be empty");
  }
  CheckLabel(p_label);
  for (const auto &action : m_actions) {
    if (p_ignore.count(action.get()) == 0 && action->GetLabel() == p_label) {
      throw ValueException("Action label must be unique within the information set");
    }
  }
}
inline void GameInfosetRep::SetLabel(const std::string &p_label)
{
  if (p_label == m_label) {
    return;
  }
  m_player->CheckInfosetLabel(p_label, {this});
  m_label = p_label;
}
inline bool GameInfosetRep::IsChanceInfoset() const { return m_player->IsChance(); }
inline std::vector<GameSequence> GameInfosetRep::GetSequences() const
{
  std::vector<GameSequence> result;
  for (auto sequence : GetPlayer()->GetSequences()) {
    if (sequence->GetAction() && sequence->GetInfoset().get() == this) {
      result.push_back(sequence);
    }
  }
  return result;
}
inline Game GameNodeRep::GetGame() const { return m_game->shared_from_this(); }
inline void GameNodeRep::SetLabel(const std::string &p_label)
{
  if (p_label == m_label) {
    return;
  }
  CheckLabel(p_label);
  // Node labels may be empty, but a non-empty label must be unique within the game.
  if (!p_label.empty()) {
    for (const auto &node : GetGame()->GetNodes()) {
      if (node.get() != this && node->GetLabel() == p_label) {
        throw ValueException("Node label must be unique within the game");
      }
    }
  }
  m_label = p_label;
}
inline int GameNodeRep::GetNumber() const
{
  m_game->EnsureNodeOrdering();
  return m_number;
}
inline GameNode GameInfosetRep::GetMember(int p_index) const
{
  m_game->EnsureInfosetOrdering();
  return m_members.at(p_index - 1);
}

inline int GameInfosetRep::GetNumber() const
{
  m_game->EnsureInfosetOrdering();
  return m_number;
}

inline GameInfosetRep::Members GameInfosetRep::GetMembers() const
{
  m_game->EnsureInfosetOrdering();
  return Members(std::const_pointer_cast<GameInfosetRep>(shared_from_this()), &m_members);
}
inline GameInfoset GamePlayerRep::GetInfoset(int p_index) const
{
  m_game->EnsureInfosetOrdering();
  return m_infosets.at(p_index - 1);
}

inline GamePlayerRep::Infosets GamePlayerRep::GetInfosets() const
{
  m_game->EnsureInfosetOrdering();
  return Infosets(std::const_pointer_cast<GamePlayerRep>(shared_from_this()), &m_infosets);
}
inline Game GameSubgameRep::GetGame() const { return m_game->shared_from_this(); }

class GameTreeRep final : public GameExplicitRep {
  friend class GameNodeRep;
  friend class GameInfosetRep;
  friend class GameActionRep;
  friend class GamePlayerRep;

  struct OwnPriorActionInfo {
    std::map<GameNodeRep *, GameActionRep *> node_map;
    std::map<GameInfosetRep *, std::set<GameActionRep *>> infoset_map;
  };

protected:
  mutable LazyAction m_nodeOrdering, m_infosetOrdering, m_strategies, m_sequences;
  std::shared_ptr<GameNodeRep> m_root;
  std::shared_ptr<GamePlayerRep> m_chance;
  std::size_t m_numNodes = 1;
  std::size_t m_numNonterminalNodes = 0;
  std::map<GameNodeRep *, std::vector<GameNodeRep *>> m_nodePlays;
  mutable LazyAction m_ownPriorActions;
  mutable std::shared_ptr<OwnPriorActionInfo> m_ownPriorActionInfo;
  mutable Lazy<std::set<GameNodeRep *>> m_unreachableNodes;
  mutable std::set<GameInfosetRep *> m_absentMindedInfosets;
  mutable std::vector<std::pair<GameInfosetRep *, GameNodeRep *>> m_absentMindedReentries;
  // The subgames of the game, held in two synchronized forms:
  // m_subgamePostorder for iteration (children before parents),
  // m_subgameByRoot for O(1) lookup by root node and ownership of the GameSubgameRep objects.
  struct SubgameData {
    std::vector<GameNodeRep *> m_subgamePostorder;
    std::unordered_map<GameNodeRep *, std::shared_ptr<GameSubgameRep>> m_subgameByRoot;
  };
  mutable Lazy<SubgameData> m_subgameData;

  /// @name Private auxiliary functions
  //@{
  static void SortInfosets(GamePlayerRep *);
  template <class Aggregator>
  Rational AggregateSubtreePayoff(const GamePlayer &p_player, Aggregator p_aggregator) const;
  static void RenumberInfosets(GamePlayerRep *);
  /// Returns the subset of p_covered no longer referenced by any node outside p_selected.
  std::set<const GameOutcomeRep *>
  ComputeAbsorbedOutcomes(const std::set<GameNodeRep *> &p_selected,
                          const std::set<const GameOutcomeRep *> &p_covered) const;
  //@}

  /// @name Managing the representation
  //@{
  /// Jointly invalidates the ordering of the nodes and the ordering of the information sets.
  void InvalidateTreeOrdering() const
  {
    m_nodeOrdering.Invalidate();
    m_infosetOrdering.Invalidate();
  }
  void InvalidateInfosetOrdering() const { m_infosetOrdering.Invalidate(); }
  void EnsureNodeOrdering() const override;
  void EnsureInfosetOrdering() const override;

  void EnsureStrategies() const override;
  void BuildConsistentPlays();
  void ClearComputedValues() const;

  void EnsureSequences() const override;
  void BuildSequences(const GameNode &n, PureSequenceProfile &p_currentSequences) const;

  /// Removes the node from the information set, invalidating if emptied
  void RemoveMember(GameInfosetRep *, GameNodeRep *);

  void CopySubtree(GameNodeRep *, GameNodeRep *, GameNodeRep *);
  //@}

public:
  /// @name Lifecycle
  //@{
  GameTreeRep();
  ~GameTreeRep() override;
  Game Copy() const override;
  //@}

  /// @name General data access
  //@{
  bool IsConstSum() const override;
  bool IsPerfectRecall() const override;
  bool HasPerfectRecall(const GamePlayer &p_player) const override;

  /// Returns the smallest payoff to the player in any play of the game
  Rational GetPlayerMinPayoff(const GamePlayer &) const override;
  /// Returns the largest payoff to the player in any play of the game
  Rational GetPlayerMaxPayoff(const GamePlayer &) const override;
  bool IsAbsentMinded(const GameInfoset &p_infoset) const override;
  std::vector<std::pair<GameInfoset, GameNode>> GetAbsentMindedReentries() const override;
  std::vector<GameSubgame> GetSubgames() const override;
  GameSubgame GetMinimalSubgame(const GameInfoset &) const override;
  //@}

  /// @name Players
  //@{
  /// Returns the chance (nature) player
  GamePlayer GetChance() const override { return m_chance->shared_from_this(); }
  void SetPlayers(const std::vector<std::string> &) override;
  //@}

  /// @name Nodes
  //@{
  /// Returns the root node of the game
  GameNode GetRoot() const override { return m_root; }
  /// Returns the number of nodes in the game
  size_t NumNodes() const override { return m_numNodes; }
  /// Returns the number of non-terminal nodes in the game
  size_t NumNonterminalNodes() const override { return m_numNonterminalNodes; }
  /// Returns the last action taken by the node's owner before reaching this node
  GameAction GetOwnPriorAction(const GameNode &p_node) const override;
  //@}

  /// @name Writing data files
  //@{
  void WriteEfgFile(std::ostream &, const GameNode &p_node = nullptr) const override;
  void WriteNfgFile(std::ostream &) const override;
  //@}

  /// @name Dimensions of the game
  //@{
  /// Returns the total number of actions in the game
  int BehavProfileLength() const override;
  //@}

  /// @name Information sets
  //@{
  /// Returns the iset'th information set in the game (numbered globally)
  GameInfoset GetInfoset(int iset) const override;
  /// Returns the set of actions taken by the infoset's owner before reaching this infoset
  std::set<GameAction> GetOwnPriorActions(const GameInfoset &p_infoset) const override;
  //@}

  /// @name Modification
  //@{
  GameInfoset AppendMove(GameNode p_node, GamePlayer p_player,
                         const std::vector<std::string> &p_actions) override;
  GameInfoset AppendMove(GameNode p_node, GameInfoset p_infoset) override;
  GameInfoset InsertMove(GameNode p_node, GamePlayer p_player, int p_actions) override;
  GameInfoset InsertMove(GameNode p_node, GamePlayer p_player,
                         const std::vector<std::string> &p_actions) override;
  GameInfoset InsertMove(GameNode p_node, GameInfoset p_infoset) override;
  GameInfoset AppendEvent(GameNode p_node, const std::vector<std::string> &p_actions,
                          const std::vector<Number> &p_probs) override;
  GameInfoset InsertEvent(GameNode p_node, const std::vector<std::string> &p_actions,
                          const std::vector<Number> &p_probs) override;
  void CopyTree(GameNode dest, GameNode src) override;
  void MoveTree(GameNode dest, GameNode src) override;
  void DeleteParent(GameNode) override;
  void DeleteTree(GameNode) override;
  GameOutcome MakeOutcome(const std::vector<GameNode> &, const std::vector<Number> &,
                          const std::string &) override;
  void MakeOutcomeNull(const std::vector<GameNode> &) override;
  GameInfoset MakeInfoset(const std::vector<GameNode> &, const GamePlayer &,
                          const std::string &) override;
  void Reveal(GameInfoset, GamePlayer) override;
  GameInfoset MakeEvent(const std::vector<GameNode> &, const std::vector<Number> &,
                        const std::string &) override;
  void RelabelActions(const GameInfoset &, const std::map<std::string, std::string> &) override;
  void SetMoveActions(const GameInfoset &, const std::vector<std::string> &) override;
  void SetEventActions(const GameInfoset &, const std::vector<std::string> &,
                       const std::vector<Number> &) override;
  std::vector<GameNode> GetPlays(GameNode node) const override;
  std::vector<GameNode> GetPlays(GameInfoset infoset) const override;
  std::vector<GameNode> GetPlays(GameAction action) const override;

  Game CopySubgame(GameNode) const override;
  //@}

  PureStrategyProfile NewPureStrategyProfile() const override;
  MixedStrategyProfile<double> NewMixedStrategyProfile(double) const override;
  MixedStrategyProfile<Rational> NewMixedStrategyProfile(const Rational &) const override;
  MixedStrategyProfile<double>
  NewMixedStrategyProfile(double, const StrategySupportProfile &) const override;
  MixedStrategyProfile<Rational>
  NewMixedStrategyProfile(const Rational &, const StrategySupportProfile &) const override;

private:
  std::vector<GameNodeRep *> BuildConsistentPlaysRecursiveImpl(GameNodeRep *node);
  void EnsureOwnPriorActions() const;
  const std::set<GameNodeRep *> &GetUnreachableNodes() const;
  const SubgameData &GetSubgameData() const;
  /// Shared implementation of SetMoveActions/SetEventActions: declares the ordered action
  /// list of p_infoset, matching by label, and (for an event) the probability distribution
  /// over them. Callers are responsible for validating p_probs and that p_infoset is of the
  /// right kind (personal move or event) before calling this.
  void DoSetActions(const GameInfoset &, const std::vector<std::string> &,
                    const std::vector<Number> &);
};

template <class T> class TreeMixedStrategyProfileRep : public MixedStrategyProfileRep<T> {
public:
  explicit TreeMixedStrategyProfileRep(const StrategySupportProfile &p_support)
    : MixedStrategyProfileRep<T>(p_support)
  {
  }
  explicit TreeMixedStrategyProfileRep(const MixedBehaviorProfile<T> &);
  ~TreeMixedStrategyProfileRep() override = default;

  std::unique_ptr<MixedStrategyProfileRep<T>> Copy() const override;
  T GetPayoff(int pl) const override;
  T GetPayoffDeriv(int pl, const GameStrategy &) const override;
  T GetPayoffDeriv(int pl, const GameStrategy &, const GameStrategy &) const override;

private:
  mutable std::shared_ptr<MixedBehaviorProfile<T>> m_mixedBehavior;

  void MakeBehavior() const;
  void OnProfileChanged() const override;
};

} // namespace Gambit

#endif // GAMBIT_GAMES_GAMETREE_H
