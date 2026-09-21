//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/games/stratmixed.cc
// Implementation of mixed strategy profile classes
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

#include "games.h"

// The reference to the tree representation violates the logic
// of separating implementation types.  This will be fixed when we move
// editing operations into the game itself instead of in the member-object
// classes.
#include "gametree.h"

namespace Gambit {

//========================================================================
//                     MixedStrategyProfileRep<T>
//========================================================================

template <class T>
MixedStrategyProfileRep<T>::MixedStrategyProfileRep(const StrategySupportProfile &p_support)
  : m_probs(p_support.GetShape()), m_offsets(p_support.GetShape()), m_support(p_support),
    m_gameversion(p_support.GetGame()->GetVersion())
{
  int index = 1;
  for (auto player : p_support.GetGame()->GetPlayers()) {
    for (auto strategy : player->GetStrategies()) {
      if (p_support.Contains(strategy)) {
        m_offsets.GetFlattened()[index] = StrategyOffset(strategy);
        m_profileIndex[strategy] = index;
        index++;
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
        if (strategy->m_behav.contains(infoset.get()) && strategy->m_behav.at(infoset.get()) > 0) {
          prob *= p_profile[infoset->GetAction(strategy->m_behav.at(infoset.get()))];
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
    const auto &strategies = m_rep->GetSupport().GetStrategies(player);
    Vector<T> values(strategies.size());
    if (m_rep->GetPayoffDerivs(player->GetNumber(), values)) {
      auto value_it = values.begin();
      for (const auto &strategy : strategies) {
        newCache.m_strategyValues[player][strategy] = *value_it;
        ++value_it;
      }
    }
    else {
      for (const auto &strategy : m_rep->GetSupport().GetStrategies(player)) {
        newCache.m_strategyValues[player][strategy] = GetPayoff(strategy);
      }
    }
  }
  newCache.m_valid = true;
  m_cache = std::move(newCache);
}

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
  if (player->m_strategies.size() == 1) {
    return T{0};
  }
  T best_other_payoff = maximize_function(
      filter_if(player->GetStrategies(), [&](const auto &s) { return s != p_strategy; }),
      [this, &player](const auto &strategy) -> T {
        return m_cache.m_strategyValues.at(player).at(strategy);
      });
  return std::max(best_other_payoff - m_cache.m_strategyValues.at(player).at(p_strategy), T{0});
}

template <class T> T MixedStrategyProfile<T>::GetRegret(const GamePlayer &p_player) const
{
  CheckVersion();
  ComputePayoffs();
  auto strategies = p_player->GetStrategies();
  if (strategies.size() == 0) {
    return T{0};
  }
  auto br_payoff = maximize_function(strategies, [this, p_player](const auto &strategy) -> T {
    return m_cache.m_strategyValues.at(p_player).at(strategy);
  });
  return br_payoff - m_cache.m_payoffs.at(p_player);
}

template <class T> T MixedStrategyProfile<T>::GetMaxRegret() const
{
  CheckVersion();
  if (GetGame()->GetPlayers().size() == 0) {
    return T{0};
  }
  return maximize_function(GetGame()->GetPlayers(),
                           [this](const auto &player) -> T { return this->GetRegret(player); });
}

MixedStrategyProfile<Rational> PureStrategyProfileRep::ToMixedStrategyProfile() const
{
  auto temp = m_game->NewMixedStrategyProfile(Rational(0));
  temp = Rational(0);
  for (const auto &player : m_game->GetPlayers()) {
    temp[GetStrategy(player)] = Rational(1);
  }
  return temp;
}

template class MixedStrategyProfileRep<double>;
template class MixedStrategyProfileRep<Rational>;

template class MixedStrategyProfile<double>;
template class MixedStrategyProfile<Rational>;

} // end namespace Gambit
