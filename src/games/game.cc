//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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
    m_payoffs[player.get()] = Number();
  }
}

//========================================================================
//                      class GameStrategyRep
//========================================================================

GameAction GameStrategyRep::GetAction(const GameInfoset &p_infoset) const
{
  if (p_infoset->GetPlayer().get() != m_player) {
    throw MismatchException();
  }
  try {
    return *std::next(p_infoset->GetActions().cbegin(), m_behav.at(p_infoset.get()) - 1);
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
    m_strategies.push_back(std::make_shared<GameStrategyRep>(this, j, ""));
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

void GamePlayerRep::MakeStrategy(const std::map<GameInfosetRep *, int> &behav)
{
  auto strategy = std::make_shared<GameStrategyRep>(this, m_strategies.size() + 1, "");
  strategy->m_behav = behav;
  for (const auto &infoset : m_infosets) {
    strategy->m_label += (contains(strategy->m_behav, infoset.get()))
                             ? std::to_string(strategy->m_behav[infoset.get()])
                             : "*";
  }
  if (strategy->m_label.empty()) {
    strategy->m_label = "*";
  }
  m_strategies.push_back(strategy);
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
          GameNodeRep *m = n->m_children[i - 1].get();
          whichbranch[n] = m;
          behav[n->m_infoset] = i;
          MakeReducedStrats(m, nn, behav, ptr, whichbranch);
        }
        behav.erase(n->m_infoset);
      }
      else {
        // we have visited this infoset, take same action
        MakeReducedStrats(n->m_children[behav[n->m_infoset] - 1].get(), nn, behav, ptr,
                          whichbranch);
      }
    }
    else {
      if (nn != nullptr) {
        ptr[n] = nn->m_parent;
      }
      else {
        ptr.erase(n);
      }
      whichbranch[n] = n->m_children.front().get();
      MakeReducedStrats(n->m_children.front().get(), n->m_children.front().get(), behav, ptr,
                        whichbranch);
    }
  }
  else if (nn) {
    GameNode m;
    for (;; nn = whichbranch.at(ptr.at(nn->m_parent))) {
      m = nn->GetNextSibling();
      if (m || !contains(ptr, nn->m_parent)) {
        break;
      }
    }
    if (m) {
      GameNodeRep *mm = whichbranch.at(m->m_parent);
      whichbranch[m->m_parent] = m.get();
      MakeReducedStrats(m.get(), m.get(), behav, ptr, whichbranch);
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
  return std::transform_reduce(
      m_infosets.cbegin(), m_infosets.cend(), 1, std::plus<>(),
      [](const std::shared_ptr<GameInfosetRep> &s) { return s->m_actions.size(); });
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

  for (auto iter : StrategyContingencies(
           StrategySupportProfile(std::const_pointer_cast<GameRep>(shared_from_this())))) {
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
    T center = static_cast<T>(1) / static_cast<T>(m_support.GetStrategies(player).size());
    for (auto strategy : m_support.GetStrategies(player)) {
      (*this)[strategy] = center;
    }
  }
}

template <class T>
std::unique_ptr<MixedStrategyProfileRep<T>> MixedStrategyProfileRep<T>::Normalize() const
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

//========================================================================
//                 MixedStrategyProfile<T>: Lifecycle
//========================================================================

template <class T>
MixedStrategyProfile<T>::MixedStrategyProfile(const MixedBehaviorProfile<T> &p_profile)
  : m_rep(std::make_unique<TreeMixedStrategyProfileRep<T>>(p_profile))
{
  auto *efg = dynamic_cast<GameTreeRep *>(p_profile.GetGame().operator->());
  for (const auto &player : efg->m_players) {
    for (const auto &strategy : player->m_strategies) {
      auto prob = static_cast<T>(1);
      for (const auto &infoset : player->m_infosets) {
        if (strategy->m_behav[infoset.get()] > 0) {
          prob *= p_profile[infoset->GetAction(strategy->m_behav[infoset.get()])];
        }
      }
      (*m_rep)[strategy] = prob;
    }
  }
}

template <class T>
MixedStrategyProfile<T> &MixedStrategyProfile<T>::operator=(const MixedStrategyProfile &p_profile)
{
  if (this != &p_profile) {
    InvalidateCache();
    m_rep = p_profile.m_rep->Copy();
  }
  return *this;
}

//========================================================================
//             MixedStrategyProfile<T>: General data access
//========================================================================

template <class T> Vector<T> MixedStrategyProfile<T>::GetStrategy(const GamePlayer &p_player) const
{
  CheckVersion();
  auto strategies = m_rep->GetSupport().GetStrategies(p_player);
  Vector<T> probs(strategies.size());
  std::transform(strategies.begin(), strategies.end(), probs.begin(),
                 [this](const GameStrategy &s) { return (*m_rep)[s]; });
  return probs;
}

template <class T> MixedStrategyProfile<T> MixedStrategyProfile<T>::ToFullSupport() const
{
  CheckVersion();
  MixedStrategyProfile<T> full(m_rep->GetSupport().GetGame()->NewMixedStrategyProfile(T(0)));

  for (const auto &player : m_rep->GetSupport().GetGame()->GetPlayers()) {
    for (const auto &strategy : player->GetStrategies()) {
      full[strategy] =
          (m_rep->GetSupport().Contains(strategy)) ? (*m_rep)[strategy] : static_cast<T>(0);
    }
  }
  return full;
}

//========================================================================
//    MixedStrategyProfile<T>: Computation of interesting quantities
//========================================================================

template <class T> void MixedStrategyProfile<T>::ComputePayoffs() const
{
  if (m_cache.m_valid) {
    return;
  }
  Cache newCache;
  for (const auto &player : m_rep->GetSupport().GetPlayers()) {
    newCache.m_payoffs[player] = GetPayoff(player);
    for (const auto &strategy : m_rep->GetSupport().GetStrategies(player)) {
      newCache.m_strategyValues[player][strategy] = GetPayoff(strategy);
    }
  }
  newCache.m_valid = true;
  m_cache = std::move(newCache);
};

template <class T> T MixedStrategyProfile<T>::GetLiapValue() const
{
  CheckVersion();
  ComputePayoffs();

  auto liapValue = static_cast<T>(0);
  for (const auto &p : m_cache.m_payoffs) {
    const auto &values = m_cache.m_strategyValues.at(p.first);
    liapValue += sum_function(values, [&](const auto &v) {
      return sqr(std::max(v.second - p.second, static_cast<T>(0)));
    });
  }
  return liapValue;
}

template <class T> T MixedStrategyProfile<T>::GetRegret(const GameStrategy &p_strategy) const
{
  CheckVersion();
  ComputePayoffs();

  auto player = p_strategy->GetPlayer();
  T best_other_payoff = maximize_function(
      filter_if(player->GetStrategies(), [&](const auto &s) { return s != p_strategy; }),
      [this, &player](const auto &strategy) -> T {
        return m_cache.m_strategyValues.at(player).at(strategy);
      });
  return std::max(best_other_payoff - m_cache.m_strategyValues.at(player).at(p_strategy),
                  static_cast<T>(0));
}

template <class T> T MixedStrategyProfile<T>::GetRegret(const GamePlayer &p_player) const
{
  CheckVersion();
  ComputePayoffs();
  auto br_payoff =
      maximize_function(p_player->GetStrategies(), [this, p_player](const auto &strategy) -> T {
        return m_cache.m_strategyValues.at(p_player).at(strategy);
      });
  return br_payoff - m_cache.m_payoffs.at(p_player);
}

template <class T> T MixedStrategyProfile<T>::GetMaxRegret() const
{
  CheckVersion();
  return maximize_function(GetGame()->GetPlayers(),
                           [this](const auto &player) -> T { return this->GetRegret(player); });
}

template class MixedStrategyProfileRep<double>;
template class MixedStrategyProfileRep<Rational>;

template class MixedStrategyProfile<double>;
template class MixedStrategyProfile<Rational>;

} // end namespace Gambit
