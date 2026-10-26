//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/games/treelayout.h
// Flattened representation of the structure of a game tree
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

#ifndef GAMBIT_GAMES_TREELAYOUT_H
#define GAMBIT_GAMES_TREELAYOUT_H

#include <map>
#include <utility>
#include <vector>

#include "core/lazy.h"
#include "core/matrix.h"
#include "game.h"

namespace Gambit {

/// @brief The structure of a game tree, flattened into arrays for repeated evaluation of
///        quantities such as realization probabilities and expected payoffs
///
/// Nodes are indexed from 0 in the order of GameRep::GetNodes(), so a node's parent always
/// precedes it, and a node's index is one less than GameNodeRep::GetNumber().  Personal
/// information sets are indexed in the order of GameRep::GetInfosets(), followed by those of
/// the chance player.  Actions at personal information sets are indexed from 1 in the same
/// order, matching a MixedBehaviorProfile on the full game; index 0 means that no personal
/// action leads to a node.
///
/// A layout describes one version of a game; see GameRep::GetTreeLayout().
struct TreeLayout {
  struct Infoset {
    GameInfosetRep *infoset{nullptr};
    int player{-1};        ///< position of the player in GameRep::GetPlayers(), or -1 for chance
    size_t firstAction{0}; ///< index of the first action, for a personal information set
    size_t numActions{0};
    std::vector<size_t> members; ///< in the order of GameInfosetRep::GetMembers()
  };

  /// Numerical data of the game, converted to the arithmetic type used for evaluation
  template <class T> struct Numbers {
    std::vector<T> chanceProb; ///< by node: probability of the chance action leading to it
    std::vector<T> payoffs;    ///< numPlayers per node: payoffs of the outcome at the node
  };

  size_t numPlayers{0};
  size_t numActions{0};            ///< number of actions at personal information sets
  size_t numPersonalInfosets{0};   ///< personal information sets come first in `infosets`
  std::vector<int> parent;         ///< by node; -1 at the root
  std::vector<size_t> action;      ///< by node: personal action leading to it, or 0
  std::vector<int> infoset;        ///< by node: its information set, or -1 if terminal
  std::vector<size_t> firstChild;  ///< by node: children are the numChildren entries
  std::vector<size_t> numChildren; ///<   of childList starting at firstChild
  std::vector<size_t> childList;
  std::vector<Infoset> infosets;
  std::vector<std::pair<size_t, size_t>> reentries; ///< (information set, node) absent-minded
  std::map<const GameInfosetRep *, size_t> infosetIndex;

  /// Numerical data as type T (double or Rational), computed on first use
  template <class T> const Numbers<T> &GetNumbers() const;

  /// Build the layout of the current version of the game.  The layout holds no reference
  /// to the game itself, so the game can own the layout it hands out.
  explicit TreeLayout(const GameRep &p_game);

private:
  // Sources of the numerical data, by node
  std::vector<GamePlayer> m_players;
  std::vector<GameOutcome> m_outcomes;
  std::vector<GameAction> m_chanceActions; // null where no chance action leads to the node

  Lazy<Numbers<double>> m_doubleNumbers;
  Lazy<Numbers<Rational>> m_rationalNumbers;

  template <class T> Numbers<T> BuildNumbers() const;
};

/// @brief Compute the derivatives of action values with respect to the log probabilities
///        of actions at personal information sets
///
/// The quantities are those of a behavior profile, indexed as in the layout: beliefs by node,
/// node values by node and player, and action values by action.  Node values may be any
/// satisfying v(n) = c(n) + sum over children of Pr(child) v(child), for payoffs c(n) which do
/// not depend on the profile: for example, the payoff of the outcome at the node (as in
/// MixedBehaviorProfile), or the total of the outcomes from the root to a terminal node and
/// zero elsewhere.  Action values must be the belief-weighted node values of the children of
/// the members of each information set.  `p_pathProb(p_from, p_to)` must return the probability
/// of reaching node `p_to` from node `p_from`, an ancestor of it.
///
/// Assumes the game has perfect recall.  See Turocy (2001), "Computing the Quantal Response
/// Equilibrium Correspondence".
///
/// @param[out] p_derivs Square matrix indexed by action; entry (a, b) is the derivative of the
///             value of action a with respect to the log probability of action b.  Entries
///             where a and b are at the same information set are zero.
template <class T, class PathProb>
void DiffActionValues(const TreeLayout &p_layout, const std::vector<T> &p_beliefs,
                      const std::vector<T> &p_nodeValues, const std::vector<T> &p_actionValues,
                      PathProb p_pathProb, Matrix<T> &p_derivs)
{
  // The value of action a at information set I of player i is
  // V(a) = sum_{h in I} mu(h) v_i(h a).  Its derivative with respect to the log probability
  // of action b has two parts:
  //  - for b taken above h, the change in beliefs: mu(h) (v_i(h a) - V(a)) for each member h
  //    whose path contains b;
  //  - for b taken at a node n below h a, the change in v_i(h a):
  //    mu(h) Pr(n b | h a) v_i(n b).
  // Both are accumulated by visiting each node together with the actions taken above it.
  const size_t numPlayers = p_layout.numPlayers;
  p_derivs = static_cast<T>(0);
  std::vector<size_t> above; // nodes reached by the personal actions taken above a node
  for (size_t n = 0; n < p_layout.parent.size(); n++) {
    const int info = p_layout.infoset[n];
    if (info < 0 || std::cmp_greater_equal(info, p_layout.numPersonalInfosets)) {
      continue;
    }
    above.clear();
    for (size_t c = n; p_layout.parent[c] >= 0; c = p_layout.parent[c]) {
      if (p_layout.action[c] != 0) {
        above.push_back(c);
      }
    }
    if (above.empty()) {
      continue;
    }
    const size_t player = p_layout.infosets[info].player;
    for (size_t j = 0; j < p_layout.numChildren[n]; j++) {
      const size_t child = p_layout.childList[p_layout.firstChild[n] + j];
      const size_t a = p_layout.action[child];
      const T beliefWeight =
          p_beliefs[n] * (p_nodeValues[child * numPlayers + player] - p_actionValues[a]);
      for (const size_t up : above) {
        p_derivs(a, p_layout.action[up]) += beliefWeight;
        const size_t h = p_layout.parent[up];
        const size_t owner = p_layout.infosets[p_layout.infoset[h]].player;
        p_derivs(p_layout.action[up], a) +=
            p_beliefs[h] * p_pathProb(up, child) * p_nodeValues[child * numPlayers + owner];
      }
    }
  }
}

template <> inline const TreeLayout::Numbers<double> &TreeLayout::GetNumbers<double>() const
{
  return m_doubleNumbers.Get([this] { return BuildNumbers<double>(); });
}

template <> inline const TreeLayout::Numbers<Rational> &TreeLayout::GetNumbers<Rational>() const
{
  return m_rationalNumbers.Get([this] { return BuildNumbers<Rational>(); });
}

} // end namespace Gambit

#endif // GAMBIT_GAMES_TREELAYOUT_H
