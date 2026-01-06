//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/solvers/nashsupport/nfgsupport.cc
// Generate possible Nash supports based on the heuristic search approach of
// Porter, Nudelman and Shoham (2004)
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
#include <algorithm>
#include <numeric>
#include <functional>

#include "nashsupport.h"

using namespace Gambit;

namespace {

using StrategySupport = std::vector<GameStrategy>;
using CallbackFunction = std::function<void(const std::map<GamePlayer, StrategySupport> &)>;

class CartesianRange {
  Array<size_t> m_sizes;

public:
  CartesianRange(const Array<size_t> &p_sizes) : m_sizes(p_sizes) {}

  class iterator {
  private:
    Array<size_t> m_sizes;
    Array<size_t> m_indices;
    bool m_end;

  public:
    using iterator_category = std::forward_iterator_tag;

    iterator(const Array<size_t> &p_sizes, bool p_end = false)
      : m_sizes(p_sizes), m_indices(m_sizes.size()), m_end(p_end)
    {
      std::fill(m_indices.begin(), m_indices.end(), 1);
    }

    const Array<size_t> &operator*() const { return m_indices; }

    const Array<size_t> &operator->() const { return m_indices; }

    iterator &operator++()
    {
      for (size_t i = 1; i <= m_sizes.size(); i++) {
        if (++m_indices[i] <= m_sizes[i]) {
          return *this;
        }
        m_indices[i] = 1;
      }
      m_end = true;
      return *this;
    }

    bool operator==(const iterator &it) const
    {
      return (m_end == it.m_end && m_sizes == it.m_sizes && m_indices == it.m_indices);
    }
    bool operator!=(const iterator &it) const { return !(*this == it); }
  };

  iterator begin() { return {m_sizes}; }
  iterator end() { return {m_sizes, true}; }
};

StrategySupportProfile RestrictedGame(const Game &game, const GamePlayer &player,
                                      std::map<GamePlayer, StrategySupport> &domainStrategies)
{
  StrategySupportProfile profile(game);
  for (auto [player2, strategies] : domainStrategies) {
    if (player2 == player) {
      continue;
    }
    for (auto strategy : player2->GetStrategies()) {
      if (!contains(strategies, strategy)) {
        profile.RemoveStrategy(strategy);
      }
    }
  }
  return profile;
}

bool AnyDominatedStrategies(const Game &game, std::map<GamePlayer, StrategySupport> &domains)
{
  for (auto [player, strategies] : domains) {
    auto support_profile = RestrictedGame(game, player, domains);
    for (auto strategy : strategies) {
      if (support_profile.IsDominated(strategy, true)) {
        return true;
      }
    }
  }

  return false;
}

class StrategySubsets {
private:
  GamePlayer m_player;
  size_t m_size;

public:
  class iterator {
  private:
    GamePlayerRep::Strategies m_strategies;
    StrategySupport m_current;
    std::vector<bool> m_include;
    bool m_end;

  private:
    void UpdateCurrent()
    {
      m_current.clear();
      auto strategy = m_strategies.begin();
      for (const auto &value : m_include) {
        if (value) {
          m_current.push_back(*strategy);
        }
        ++strategy;
      }
    }

  public:
    using iterator_category = std::forward_iterator_tag;

    iterator(const GamePlayerRep::Strategies &p_strategies, size_t p_size, bool p_end = false)
      : m_strategies(p_strategies), m_include(m_strategies.size()), m_end(p_end)
    {
      std::fill(m_include.begin(), m_include.begin() + p_size, true);
      UpdateCurrent();
    }

    const StrategySupport &operator*() const { return m_current; }

    const StrategySupport &operator->() const { return m_current; }

    iterator &operator++()
    {
      m_end = !std::next_permutation(m_include.begin(), m_include.end(),
                                     [](bool x, bool y) { return y < x; });
      UpdateCurrent();
      return *this;
    }

    bool operator==(const iterator &it) const
    {
      return (m_end == it.m_end && m_strategies == it.m_strategies && m_include == it.m_include);
    }
    bool operator!=(const iterator &it) const { return !(*this == it); }
  };

  StrategySubsets(const GamePlayer &p_player, size_t p_size) : m_player(p_player), m_size(p_size)
  {
  }

  GamePlayer GetPlayer() const { return m_player; }

  iterator begin() { return {m_player->GetStrategies(), m_size}; }
  iterator end() { return {m_player->GetStrategies(), m_size, true}; }
};

void GenerateSupportProfiles(const Game &game,
                             std::map<GamePlayer, StrategySupport> currentSupports,
                             std::list<StrategySubsets> domains, CallbackFunction callback)
{
  auto domain = domains.front();
  domains.pop_front();
  for (auto support : domain) {
    currentSupports[domain.GetPlayer()] = support;
    if (AnyDominatedStrategies(game, currentSupports)) {
      continue;
    }
    if (domains.empty()) {
      callback(currentSupports);
    }
    else {
      GenerateSupportProfiles(game, currentSupports, domains, callback);
    }
  }
}

/// Solve over supports with a total number of strategies `size` and a maximum difference
/// in player support size of `diff`.
void GenerateSizeDiff(const Game &game, int size, int diff, CallbackFunction callback)
{
  auto players = game->GetPlayers();
  CartesianRange range(game->GetStrategies().shape_array());
  for (auto size_profile : range) {
    if (*std::max_element(size_profile.cbegin(), size_profile.cend()) -
                *std::min_element(size_profile.cbegin(), size_profile.cend()) !=
            diff ||
        std::accumulate(size_profile.cbegin(), size_profile.cend(), 0) != size) {
      continue;
    }
    std::list<StrategySubsets> domains;
    std::transform(players.begin(), players.end(), size_profile.begin(),
                   std::back_inserter(domains),
                   [](const GamePlayer &player, size_t sz) -> StrategySubsets {
                     return {player, sz};
                   });
    GenerateSupportProfiles(game, std::map<GamePlayer, StrategySupport>(), domains, callback);
  }
}

StrategySupportProfile
StrategiesToSupport(const Game &p_game, const std::map<GamePlayer, StrategySupport> &p_strategies)
{
  StrategySupportProfile support(p_game);
  for (auto player : p_game->GetPlayers()) {
    for (auto strategy : player->GetStrategies()) {
      if (!contains(p_strategies.at(player), strategy)) {
        support.RemoveStrategy(strategy);
      }
    }
  }
  return support;
}

} // end anonymous namespace

std::shared_ptr<PossibleNashStrategySupportsResult>
PossibleNashStrategySupports(const Game &p_game)
{
  auto result = std::make_shared<PossibleNashStrategySupportsResult>();
  auto numActions = p_game->GetStrategies().shape_array();

  const int maxsize =
      std::accumulate(numActions.begin(), numActions.end(), 0) - p_game->NumPlayers() + 1;
  const int maxdiff = *std::max_element(numActions.cbegin(), numActions.cend());

  const bool preferBalance = p_game->NumPlayers() == 2;
  Array<size_t> dim(2);
  dim[1] = (preferBalance) ? maxsize : maxdiff;
  dim[2] = (preferBalance) ? maxdiff : maxsize;

  CartesianRange range(dim);
  const std::list<MixedStrategyProfile<double>> solutions;
  for (auto x : range) {
    GenerateSizeDiff(p_game, ((preferBalance) ? x[1] : x[2]) + p_game->NumPlayers() - 1,
                     ((preferBalance) ? x[2] : x[1]) - 1,
                     [p_game, result](const std::map<GamePlayer, StrategySupport> &p_profile) {
                       result->m_supports.push_back(StrategiesToSupport(p_game, p_profile));
                     });
  }
  return result;
}
