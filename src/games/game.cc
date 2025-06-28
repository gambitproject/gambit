//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/libgambit/game.cc
// Implementation of extensive form game representation
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

#include <iostream>
#include <numeric>
#include <random>
#include <queue>

#include "gambit.h"
#include "writer.h"

// The references to the tree representations violate the logic
// of separating implementation types.  This will be fixed when we move
// editing operations into the game itself instead of in the member-object
// classes.
#include "gametree.h"

namespace Gambit {

//========================================================================
//                       class GameOutcomeRep
//========================================================================

GameOutcomeRep::GameOutcomeRep(GameRep *p_game, int p_number) : m_game(p_game), m_number(p_number)
{
  for (const auto &player : m_game->m_players) {
    m_payoffs[player] = Number();
  }
}

//========================================================================
//                      class GameStrategyRep
//========================================================================

GameAction GameStrategyRep::GetAction(const GameInfoset &p_infoset) const
{
  if (p_infoset->GetPlayer() != m_player) {
    throw MismatchException();
  }
  try {
    return *std::next(p_infoset->GetActions().cbegin(), m_behav.at(p_infoset) - 1);
  }
  catch (std::out_of_range &) {
    return nullptr;
  }
}

//========================================================================
//                       class GamePlayerRep
//========================================================================

GamePlayerRep::GamePlayerRep(GameRep *p_game, int p_id, int p_strats)
  : m_game(p_game), m_number(p_id)
{
  for (int j = 1; j <= p_strats; j++) {
    m_strategies.push_back(new GameStrategyRep(this, j, ""));
  }
}

GamePlayerRep::~GamePlayerRep()
{
  for (auto infoset : m_infosets) {
    infoset->Invalidate();
  }
  for (auto strategy : m_strategies) {
    strategy->Invalidate();
  }
}

void GamePlayerRep::MakeStrategy(const std::map<GameInfoset, GameAction> &p_behavMap)
{
  std::map<GameInfosetRep *, int> array_behav;
  for (auto [infoset, action] : p_behavMap) {
    array_behav[infoset] = action->GetNumber();
  }
  return MakeStrategy(array_behav);
}

void GamePlayerRep::MakeStrategy(const std::map<GameInfosetRep *, int> &behav)
{
  auto *strategy = new GameStrategyRep(this, m_strategies.size() + 1, "");
  strategy->m_behav = behav;
  for (const auto &infoset : m_infosets) {
    strategy->m_label +=
        (contains(strategy->m_behav, infoset)) ? std::to_string(strategy->m_behav[infoset]) : "*";
  }
  if (strategy->m_label.empty()) {
    strategy->m_label = "*";
  }
  m_strategies.push_back(strategy);
}

void GamePlayerRep::MakeReducedStratsPR(const PlayerConsequences &p_consequences)
{
  m_strategies.clear();
  std::map<GameInfoset, GameAction> current_strat;

  if (m_infosets.empty()) {
    MakeStrategy(current_strat);
    return;
  }

  // Initial frontier -- UNORDERED SET of roots; the range constructor builds an ORDERED SET
  if (!p_consequences.root_infosets.empty()) {
    const std::set<GameInfoset, InfosetNumberCmp> initial_frontier(
        p_consequences.root_infosets.begin(), p_consequences.root_infosets.end());
    BuildStratsRecursive(p_consequences, current_strat, initial_frontier);
  }
}

// related to Bernhard's third algorithm
void GamePlayerRep::BuildStratsRecursive(const PlayerConsequences &p_consequences,
                                         std::map<GameInfoset, GameAction> &current_strat,
                                         std::set<GameInfoset, InfosetNumberCmp> frontier)
{
  if (frontier.empty()) {
    MakeStrategy(current_strat);
    return;
  }

  // infoset with the smallest number
  const GameInfoset infoset_to_explore = *frontier.begin();
  frontier.erase(frontier.begin());

  for (const auto &action : infoset_to_explore->GetActions()) {
    current_strat[infoset_to_explore] = action;

    // Recursion: The new frontier is a copy of the old one, plus new consequences.
    auto next_frontier = frontier;
    const auto &consequences = p_consequences.transitions.at(infoset_to_explore).at(action);
    next_frontier.insert(consequences.begin(), consequences.end());

    BuildStratsRecursive(p_consequences, current_strat, next_frontier);

    current_strat.erase(infoset_to_explore);
  }
}

void GamePlayerRep::MakeReducedStrats(GameNodeRep *n, GameNodeRep *nn,
                                      std::map<GameInfosetRep *, int> &behav,
                                      std::map<GameNodeRep *, GameNodeRep *> &ptr,
                                      std::map<GameNodeRep *, GameNodeRep *> &whichbranch)
{
  if (!n->IsTerminal()) {
    if (n->m_infoset->m_player == this) {
      if (!contains(behav, n->m_infoset)) {
        // we haven't visited this infoset before
        for (size_t i = 1; i <= n->m_children.size(); i++) {
          GameNodeRep *m = n->m_children[i - 1];
          whichbranch[n] = m;
          behav[n->m_infoset] = i;
          MakeReducedStrats(m, nn, behav, ptr, whichbranch);
        }
        behav.erase(n->m_infoset);
      }
      else {
        // we have visited this infoset, take same action
        MakeReducedStrats(n->m_children[behav[n->m_infoset] - 1], nn, behav, ptr, whichbranch);
      }
    }
    else {
      if (nn != nullptr) {
        ptr[n] = nn->m_parent;
      }
      else {
        ptr.erase(n);
      }
      whichbranch[n] = n->m_children.front();
      MakeReducedStrats(n->m_children.front(), n->m_children.front(), behav, ptr, whichbranch);
    }
  }
  else if (nn) {
    GameNodeRep *m;
    for (;; nn = whichbranch.at(ptr.at(nn->m_parent))) {
      m = nn->GetNextSibling();
      if (m || !contains(ptr, nn->m_parent)) {
        break;
      }
    }
    if (m) {
      GameNodeRep *mm = whichbranch.at(m->m_parent);
      whichbranch[m->m_parent] = m;
      MakeReducedStrats(m, m, behav, ptr, whichbranch);
      whichbranch[m->m_parent] = mm;
    }
    else {
      MakeStrategy(behav);
    }
  }
  else {
    MakeStrategy(behav);
  }
}

size_t GamePlayerRep::NumSequences() const
{
  if (!m_game->IsTree()) {
    throw UndefinedException();
  }
  return std::transform_reduce(m_infosets.cbegin(), m_infosets.cend(), 1, std::plus<>(),
                               [](const GameInfosetRep *s) { return s->m_actions.size(); });
}

//========================================================================
//                            class GameRep
//========================================================================

GameRep::~GameRep()
{
  for (auto player : m_players) {
    player->Invalidate();
  }
  for (auto outcome : m_outcomes) {
    outcome->Invalidate();
  }
}

//------------------------------------------------------------------------
//                     GameRep: Writing data files
//------------------------------------------------------------------------

///
/// Write the game to a savefile in .nfg payoff format.
///
/// This implements writing a game to a .nfg savefile.  As it uses
/// only publicly-accessible operations, it is in principle valid
/// for any game.  It writes the payoff format, as it is not required
/// that a game representation implements an outcome for each possible
/// pure strategy profile.  For example, in extensive games with chance
/// moves, the outcome from a pure strategy profile is not deterministic,
/// but rather a probability distribution over outcomes.
///
void GameRep::WriteNfgFile(std::ostream &p_file) const
{
  auto players = GetPlayers();
  p_file << "NFG 1 R " << std::quoted(GetTitle()) << ' '
         << FormatList(players, [](const GamePlayer &p) { return QuoteString(p->GetLabel()); })
         << std::endl
         << std::endl;
  p_file << "{ ";
  for (auto player : players) {
    p_file << FormatList(player->GetStrategies(), [](const GameStrategy &s) {
      return QuoteString(s->GetLabel());
    }) << std::endl;
  }
  p_file << "}" << std::endl;
  p_file << std::quoted(GetComment()) << std::endl << std::endl;

  for (auto iter :
       StrategyContingencies(StrategySupportProfile(Game(const_cast<GameRep *>(this))))) {
    p_file << FormatList(
                  players,
                  [&iter](const GamePlayer &p) {
                    return lexical_cast<std::string>(iter->GetPayoff(p));
                  },
                  false, false)
           << std::endl;
  };
}

//========================================================================
//                     MixedStrategyProfileRep<T>
//========================================================================

template <class T>
MixedStrategyProfileRep<T>::MixedStrategyProfileRep(const StrategySupportProfile &p_support)
  : m_probs(p_support.MixedProfileLength()), m_support(p_support),
    m_gameversion(p_support.GetGame()->GetVersion())
{
  int index = 1;
  for (auto player : p_support.GetGame()->GetPlayers()) {
    for (auto strategy : player->GetStrategies()) {
      if (p_support.Contains(strategy)) {
        m_profileIndex[strategy] = index++;
      }
      else {
        m_profileIndex[strategy] = -1;
      }
    }
  }
  SetCentroid();
}

template <class T> void MixedStrategyProfileRep<T>::SetCentroid()
{
  for (auto player : m_support.GetGame()->GetPlayers()) {
    T center = T(1) / T(m_support.GetStrategies(player).size());
    for (auto strategy : m_support.GetStrategies(player)) {
      (*this)[strategy] = center;
    }
  }
}

template <class T> MixedStrategyProfileRep<T> *MixedStrategyProfileRep<T>::Normalize() const
{
  auto norm = Copy();
  for (auto player : m_support.GetGame()->GetPlayers()) {
    T sum = static_cast<T>(0);
    for (auto strategy : m_support.GetStrategies(player)) {
      sum += (*this)[strategy];
    }
    if (sum == static_cast<T>(0)) {
      continue;
    }
    for (auto strategy : m_support.GetStrategies(player)) {
      (*norm)[strategy] /= sum;
    }
  }
  return norm;
}

template <class T> T MixedStrategyProfileRep<T>::GetRegret(const GameStrategy &p_strategy) const
{
  const GamePlayer player = p_strategy->GetPlayer();
  T payoff = GetPayoffDeriv(player->GetNumber(), p_strategy);
  T brpayoff = payoff;
  for (auto strategy : player->GetStrategies()) {
    if (strategy != p_strategy) {
      brpayoff = std::max(brpayoff, GetPayoffDeriv(player->GetNumber(), strategy));
    }
  }
  return brpayoff - payoff;
}

template <class T> T MixedStrategyProfileRep<T>::GetRegret(const GamePlayer &p_player) const
{
  auto strategies = p_player->GetStrategies();
  T br_payoff = std::accumulate(std::next(strategies.begin()), strategies.end(),
                                GetPayoff(*strategies.begin()),
                                [this](const T &x, const GameStrategy &strategy) {
                                  return std::max(x, GetPayoff(strategy));
                                });
  return br_payoff - GetPayoff(p_player);
}

template <class T> T MixedStrategyProfileRep<T>::GetMaxRegret() const
{
  auto players = m_support.GetGame()->GetPlayers();
  return std::accumulate(players.begin(), players.end(), T(0),
                         [this](const T &x, const GamePlayer &player) {
                           return std::max(x, this->GetRegret(player));
                         });
}

//========================================================================
//                 MixedStrategyProfile<T>: Lifecycle
//========================================================================

template <class T>
MixedStrategyProfile<T>::MixedStrategyProfile(const MixedBehaviorProfile<T> &p_profile)
  : m_rep(new TreeMixedStrategyProfileRep<T>(p_profile))
{
  auto *efg = dynamic_cast<GameTreeRep *>(p_profile.GetGame().operator->());
  for (const auto &player : efg->m_players) {
    for (const auto &strategy : player->m_strategies) {
      auto prob = static_cast<T>(1);
      for (const auto &infoset : player->m_infosets) {
        if (strategy->m_behav[infoset] > 0) {
          prob *= p_profile[infoset->GetAction(strategy->m_behav[infoset])];
        }
      }
      (*m_rep)[strategy] = prob;
    }
  }
}

template <class T>
MixedStrategyProfile<T> &
MixedStrategyProfile<T>::operator=(const MixedStrategyProfile<T> &p_profile)
{
  if (this != &p_profile) {
    InvalidateCache();
    m_rep.reset(p_profile.m_rep->Copy());
  }
  return *this;
}

//========================================================================
//             MixedStrategyProfile<T>: General data access
//========================================================================

template <class T> Vector<T> MixedStrategyProfile<T>::operator[](const GamePlayer &p_player) const
{
  CheckVersion();
  auto strategies = m_rep->m_support.GetStrategies(p_player);
  Vector<T> probs(strategies.size());
  std::transform(strategies.begin(), strategies.end(), probs.begin(),
                 [this](const GameStrategy &s) { return (*m_rep)[s]; });
  return probs;
}

template <class T> MixedStrategyProfile<T> MixedStrategyProfile<T>::ToFullSupport() const
{
  CheckVersion();
  MixedStrategyProfile<T> full(m_rep->m_support.GetGame()->NewMixedStrategyProfile(T(0)));

  for (const auto &player : m_rep->m_support.GetGame()->GetPlayers()) {
    for (const auto &strategy : player->GetStrategies()) {
      full[strategy] =
          (m_rep->m_support.Contains(strategy)) ? (*m_rep)[strategy] : static_cast<T>(0);
    }
  }
  return full;
}

//========================================================================
//    MixedStrategyProfile<T>: Computation of interesting quantities
//========================================================================

template <class T> void MixedStrategyProfile<T>::ComputePayoffs() const
{
  if (!map_profile_payoffs.empty()) {
    // caches (map_profile_payoffs and map_strategy_payoffs) are valid,
    // so don't compute anything, simply return
    return;
  }
  for (const auto &player : m_rep->m_support.GetPlayers()) {
    map_profile_payoffs[player] = GetPayoff(player);
    // values of the player's strategies
    for (const auto &strategy : m_rep->m_support.GetStrategies(player)) {
      map_strategy_payoffs[player][strategy] = GetPayoff(strategy);
    }
  }
};

template <class T> T MixedStrategyProfile<T>::GetLiapValue() const
{
  CheckVersion();
  ComputePayoffs();

  auto liapValue = static_cast<T>(0);
  for (auto [player, payoff] : map_profile_payoffs) {
    for (auto v : map_strategy_payoffs[player]) {
      liapValue += sqr(std::max(v.second - payoff, static_cast<T>(0)));
    }
  }
  return liapValue;
}

template class MixedStrategyProfileRep<double>;
template class MixedStrategyProfileRep<Rational>;

template class MixedStrategyProfile<double>;
template class MixedStrategyProfile<Rational>;

} // end namespace Gambit
