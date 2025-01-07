//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/libgambit/stratspt.cc
// Implementation of strategy classes for normal forms
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

#include <algorithm>
#include <numeric>

#include "gambit.h"
#include "gametable.h"

namespace Gambit {

//===========================================================================
//                        class StrategySupportProfile
//===========================================================================

StrategySupportProfile::StrategySupportProfile(const Game &p_nfg) : m_nfg(p_nfg)
{
  for (auto player : m_nfg->GetPlayers()) {
    for (auto strategy : player->GetStrategies()) {
      m_support[player].push_back(strategy);
    }
  }
}

Array<int> StrategySupportProfile::NumStrategies() const
{
  Array<int> dim(m_support.size());
  std::transform(
      m_support.cbegin(), m_support.cend(), dim.begin(),
      [](std::pair<GamePlayer, std::vector<GameStrategy>> a) { return a.second.size(); });
  return dim;
}

int StrategySupportProfile::MixedProfileLength() const
{
  return std::accumulate(m_support.cbegin(), m_support.cend(), 0,
                         [](size_t tot, std::pair<GamePlayer, std::vector<GameStrategy>> a) {
                           return tot + a.second.size();
                         });
}

template <> MixedStrategyProfile<double> StrategySupportProfile::NewMixedStrategyProfile() const
{
  return m_nfg->NewMixedStrategyProfile(0.0, *this);
}

template <> MixedStrategyProfile<Rational> StrategySupportProfile::NewMixedStrategyProfile() const
{
  return m_nfg->NewMixedStrategyProfile(Rational(0), *this);
}

bool StrategySupportProfile::IsSubsetOf(const StrategySupportProfile &p_support) const
{
  for (auto player : m_nfg->GetPlayers()) {
    if (!std::includes(p_support.m_support.at(player).begin(),
                       p_support.m_support.at(player).end(), m_support.at(player).begin(),
                       m_support.at(player).end(),
                       [](const GameStrategy &s, const GameStrategy &t) {
                         return s->GetNumber() < t->GetNumber();
                       })) {
      return false;
    }
  }
  return true;
}

void StrategySupportProfile::WriteNfgFile(std::ostream &p_file) const
{
  auto players = m_nfg->GetPlayers();
  p_file << "NFG 1 R " << std::quoted(m_nfg->GetTitle()) << ' '
         << FormatList(players, [](const GamePlayer &p) { return QuoteString(p->GetLabel()); })
         << std::endl
         << std::endl;
  p_file << "{ ";
  for (auto player : players) {
    p_file << FormatList(GetStrategies(player), [](const GameStrategy &s) {
      return QuoteString(s->GetLabel());
    }) << std::endl;
  }
  p_file << "}" << std::endl;
  p_file << std::quoted(m_nfg->GetComment()) << std::endl << std::endl;

  for (auto iter : StrategyContingencies(*this)) {
    p_file << FormatList(
                  players,
                  [&iter](const GamePlayer &p) {
                    return lexical_cast<std::string>(iter->GetPayoff(p));
                  },
                  false, false)
           << std::endl;
  };
}

void StrategySupportProfile::AddStrategy(const GameStrategy &p_strategy)
{
  auto &support = m_support[p_strategy->GetPlayer()];
  auto pos = std::find_if(support.begin(), support.end(), [p_strategy](const GameStrategy &s) {
    return s->GetNumber() >= p_strategy->GetNumber();
  });
  if (pos == support.end() || *pos != p_strategy) {
    // Strategy is not in the support for the player; add at this location to keep sorted by number
    support.insert(pos, p_strategy);
  }
}

bool StrategySupportProfile::RemoveStrategy(const GameStrategy &p_strategy)
{
  auto &support = m_support[p_strategy->GetPlayer()];
  if (support.size() == 1) {
    return false;
  }
  auto pos = std::find(support.begin(), support.end(), p_strategy);
  if (pos != support.end()) {
    support.erase(pos);
    return true;
  }
  return false;
}

//---------------------------------------------------------------------------
//                 Identification of dominated strategies
//---------------------------------------------------------------------------

bool StrategySupportProfile::Dominates(const GameStrategy &s, const GameStrategy &t,
                                       bool p_strict) const
{
  bool equal = true;

  for (auto iter : StrategyContingencies(*this)) {
    Rational ap = iter->GetStrategyValue(s);
    Rational bp = iter->GetStrategyValue(t);
    if (p_strict && ap <= bp) {
      return false;
    }
    else if (!p_strict) {
      if (ap < bp) {
        return false;
      }
      else if (ap > bp) {
        equal = false;
      }
    }
  }

  return (p_strict || !equal);
}

bool StrategySupportProfile::IsDominated(const GameStrategy &s, bool p_strict,
                                         bool p_external) const
{
  if (p_external) {
    for (auto strategy : s->GetPlayer()->GetStrategies()) {
      if (strategy != s && Dominates(strategy, s, p_strict)) {
        return true;
      }
    }
    return false;
  }
  else {
    for (auto strategy : GetStrategies(s->GetPlayer())) {
      if (strategy != s && Dominates(strategy, s, p_strict)) {
        return true;
      }
    }
    return false;
  }
}

namespace {

/// @brief Sort a range by a partial ordering and indicate minimal elements
///
/// Sorts the range bracketed by `first` and `last` according to a partial ordering.
/// The partial ordering is specified by `greater`, which should be a binary
/// operation which returns `true` if the first argument is greater than the
/// second argument in the partial ordering.
///
/// On termination, the range between `first` and `last` is sorted in decreasing
/// order by the partial ordering, in the sense that, if x > y according to the
/// partial ordering, then x will come before y in the sorted output.
///
/// By this convention the set of **minimal elements** will all appear at the end
/// of the sorted output.  The function returns an iterator pointing to the first
/// minimal element in the sorted range.
template <class Iterator, class Comparator>
Iterator find_minimal_elements(Iterator first, Iterator last, Comparator greater)
{
  auto min = first, dis = std::prev(last);
  while (min <= dis) {
    auto pp = std::adjacent_find(first, last, greater);
    if (pp < min) {
      dis--;
    }
    else {
      std::iter_swap(dis, min);
      auto inc = std::next(min);
      while (inc <= dis) {
        if (greater(*min, *dis)) {
          dis--;
        }
        else if (greater(*dis, *min)) {
          std::iter_swap(dis, min);
          dis--;
        }
        else {
          std::iter_swap(dis, inc);
          inc++;
        }
      }
      min++;
    }
  }
  return min;
}

} // end anonymous namespace

bool UndominatedForPlayer(const StrategySupportProfile &p_support,
                          StrategySupportProfile &p_newSupport, const GamePlayer &p_player,
                          bool p_strict, bool p_external)
{
  std::vector<GameStrategy> set((p_external) ? p_player->NumStrategies()
                                             : p_support.GetStrategies(p_player).size());
  if (p_external) {
    auto strategies = p_player->GetStrategies();
    std::copy(strategies.begin(), strategies.end(), set.begin());
  }
  else {
    auto strategies = p_support.GetStrategies(p_player);
    std::copy(strategies.begin(), strategies.end(), set.begin());
  }
  auto min =
      find_minimal_elements(set.begin(), set.end(),
                            [&p_support, p_strict](const GameStrategy &s, const GameStrategy &t) {
                              return p_support.Dominates(s, t, p_strict);
                            });

  for (auto s = min; s != set.end(); s++) {
    p_newSupport.RemoveStrategy(*s);
  }
  return min != set.end();
}

StrategySupportProfile StrategySupportProfile::Undominated(bool p_strict, bool p_external) const
{
  StrategySupportProfile newSupport(*this);
  for (auto player : m_nfg->GetPlayers()) {
    UndominatedForPlayer(*this, newSupport, player, p_strict, p_external);
  }
  return newSupport;
}

//---------------------------------------------------------------------------
//                Identification of overwhelmed strategies
//---------------------------------------------------------------------------

bool StrategySupportProfile::Overwhelms(const GameStrategy &s, const GameStrategy &t,
                                        bool p_strict) const
{
  auto cont = StrategyContingencies(*this);
  auto iter = cont.begin();
  Rational sMin = (*iter)->GetStrategyValue(s);
  Rational tMax = (*iter)->GetStrategyValue(t);

  while (iter != cont.end()) {
    if ((*iter)->GetStrategyValue(s) < sMin) {
      sMin = (*iter)->GetStrategyValue(s);
    }

    if ((*iter)->GetStrategyValue(t) > tMax) {
      tMax = (*iter)->GetStrategyValue(t);
    }

    if (sMin < tMax || (sMin == tMax && p_strict)) {
      return false;
    }
  }

  return true;
}

} // end namespace Gambit
