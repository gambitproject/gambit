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

#include "nashsupport.h"

using namespace Gambit;

namespace {

using StrategySupport = std::vector<GameStrategy>;

class CartesianRange {
  std::vector<size_t> m_sizes;

public:
  CartesianRange(const std::vector<size_t> &p_sizes) : m_sizes(p_sizes) {}

  class iterator {
    std::vector<size_t> m_sizes;
    std::vector<size_t> m_indices;
    bool m_end;

  public:
    using iterator_category = std::forward_iterator_tag;

    iterator(const std::vector<size_t> &p_sizes, bool p_end = false)
      : m_sizes(p_sizes), m_indices(m_sizes.size()), m_end(p_end)
    {
      std::fill(m_indices.begin(), m_indices.end(), 1);
    }

    const std::vector<size_t> &operator*() const { return m_indices; }

    const std::vector<size_t> &operator->() const { return m_indices; }

    iterator &operator++()
    {
      for (size_t i = 0; i < m_sizes.size(); i++) {
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
  GamePlayer m_player;
  size_t m_size;

public:
  class iterator {
    GamePlayerRep::Strategies m_strategies;
    StrategySupport m_current;
    std::vector<bool> m_include;
    bool m_end;

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

std::vector<size_t> ComputeOuterDim(const std::vector<size_t> &numActions, bool preferBalance,
                                    int numPlayers)
{
  const int maxsize = std::accumulate(numActions.begin(), numActions.end(), 0) - numPlayers + 1;
  const int maxdiff = *std::max_element(numActions.cbegin(), numActions.cend());
  std::vector<size_t> dim(2);
  dim[0] = preferBalance ? maxsize : maxdiff;
  dim[1] = preferBalance ? maxdiff : maxsize;
  return dim;
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

class PossibleNashStrategySupports::Impl {
public:
  explicit Impl(const Game &p_game)
    : m_game(p_game), m_numActions(p_game->GetStrategies().shape()),
      m_preferBalance(p_game->NumPlayers() == 2),
      m_xRange(ComputeOuterDim(m_numActions, m_preferBalance, p_game->NumPlayers())),
      m_xIt(m_xRange.begin()), m_xEnd(m_xRange.end()), m_sizeProfileRange(m_numActions),
      m_spIt(m_sizeProfileRange.end()), m_spEnd(m_sizeProfileRange.end()), m_current(p_game)
  {
  }

  std::optional<StrategySupportProfile> Next()
  {
    if (!Advance()) {
      return std::nullopt;
    }
    return m_current;
  }

private:
  struct Frame {
    GamePlayer player;
    StrategySubsets::iterator cur;
    StrategySubsets::iterator end;
  };

  bool Advance()
  {
    while (true) {
      if (!m_stack.empty() && AdvanceDFS()) {
        return true;
      }
      while (!NextSizeProfile()) {
        if (!NextX()) {
          return false;
        }
      }
    }
  }

  bool AdvanceDFS()
  {
    while (!m_stack.empty()) {
      Frame &frame = m_stack.back();
      if (frame.cur == frame.end) {
        m_currentSupports.erase(frame.player);
        m_stack.pop_back();
        if (!m_stack.empty()) {
          ++m_stack.back().cur;
        }
        continue;
      }
      m_currentSupports[frame.player] = *frame.cur;
      if (AnyDominatedStrategies(m_game, m_currentSupports)) {
        ++frame.cur;
        continue;
      }
      if (m_stack.size() == m_domains.size()) {
        m_current = StrategiesToSupport(m_game, m_currentSupports);
        ++frame.cur;
        return true;
      }
      StrategySubsets &nextDomain = m_domains[m_stack.size()];
      m_stack.push_back(Frame{nextDomain.GetPlayer(), nextDomain.begin(), nextDomain.end()});
    }
    return false;
  }

  // Advances to the next (size, diff) preference pair, resetting the size-profile search
  // to its start.  Returns false once all pairs have been tried.
  bool NextX()
  {
    if (m_xIt == m_xEnd) {
      return false;
    }
    const std::vector<size_t> &x = *m_xIt;
    m_size = static_cast<int>(m_preferBalance ? x[0] : x[1]) + m_game->NumPlayers() - 1;
    m_diff = (m_preferBalance ? x[1] : x[0]) - 1;
    ++m_xIt;
    m_spIt = m_sizeProfileRange.begin();
    m_spEnd = m_sizeProfileRange.end();
    return true;
  }

  // Advances to the next size profile (for the current (size, diff) pair) whose total and
  // spread match, setting up the per-player subset domains and DFS stack for it. Returns
  // false once all size profiles have been tried for the current pair.
  bool NextSizeProfile()
  {
    while (m_spIt != m_spEnd) {
      const std::vector<size_t> &sizeProfile = *m_spIt;
      ++m_spIt;
      if (*std::max_element(sizeProfile.cbegin(), sizeProfile.cend()) -
                  *std::min_element(sizeProfile.cbegin(), sizeProfile.cend()) !=
              m_diff ||
          std::accumulate(sizeProfile.cbegin(), sizeProfile.cend(), 0) != m_size) {
        continue;
      }
      SetupDomains(sizeProfile);
      return true;
    }
    return false;
  }

  void SetupDomains(const std::vector<size_t> &p_sizeProfile)
  {
    auto players = m_game->GetPlayers();
    m_domains.clear();
    std::transform(players.begin(), players.end(), p_sizeProfile.begin(),
                   std::back_inserter(m_domains),
                   [](const GamePlayer &player, size_t sz) -> StrategySubsets {
                     return {player, sz};
                   });
    m_currentSupports.clear();
    m_stack.clear();
    m_stack.push_back(
        Frame{m_domains.front().GetPlayer(), m_domains.front().begin(), m_domains.front().end()});
  }

  Game m_game;
  std::vector<size_t> m_numActions;
  bool m_preferBalance;

  CartesianRange m_xRange;
  CartesianRange::iterator m_xIt;
  CartesianRange::iterator m_xEnd;
  int m_size = 0;
  size_t m_diff = 0;

  CartesianRange m_sizeProfileRange;
  CartesianRange::iterator m_spIt;
  CartesianRange::iterator m_spEnd;

  std::vector<StrategySubsets> m_domains;
  std::vector<Frame> m_stack;
  std::map<GamePlayer, StrategySupport> m_currentSupports;
  StrategySupportProfile m_current;
};

PossibleNashStrategySupports::PossibleNashStrategySupports(const Game &p_game)
  : m_impl(std::make_unique<Impl>(p_game))
{
}

PossibleNashStrategySupports::~PossibleNashStrategySupports() = default;
PossibleNashStrategySupports::PossibleNashStrategySupports(
    PossibleNashStrategySupports &&) noexcept = default;
PossibleNashStrategySupports &
PossibleNashStrategySupports::operator=(PossibleNashStrategySupports &&) noexcept = default;

std::optional<StrategySupportProfile> PossibleNashStrategySupports::Next()
{
  return m_impl->Next();
}
