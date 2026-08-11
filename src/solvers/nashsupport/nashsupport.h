//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/nfgensup.h
// Enumerate undominated subsupports of a support
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

#ifndef GAMBIT_SOLVERS_NASHSUPPORT_NASHSUPPORT_H
#define GAMBIT_SOLVERS_NASHSUPPORT_NASHSUPPORT_H

#include <memory>
#include <optional>

#include "gambit.h"

using namespace Gambit;

// Enumerates, one at a time, the strategy support profiles which can be the support of a
// totally-mixed Nash equilibrium, using the heuristic search method of Porter, Nudelman &
// Shoham (2004).
//
// This is a single-pass generator: construct one instance per game and pull candidates
// from it, either by repeated calls to Next() or via a range-based for loop.
class PossibleNashStrategySupports {
public:
  explicit PossibleNashStrategySupports(const Game &p_game);
  ~PossibleNashStrategySupports();
  PossibleNashStrategySupports(PossibleNashStrategySupports &&) noexcept;
  PossibleNashStrategySupports &operator=(PossibleNashStrategySupports &&) noexcept;
  PossibleNashStrategySupports(const PossibleNashStrategySupports &) = delete;
  PossibleNashStrategySupports &operator=(const PossibleNashStrategySupports &) = delete;

  // Returns the next candidate support, or std::nullopt once the search is exhausted.
  std::optional<StrategySupportProfile> Next();

  class iterator {
  public:
    using iterator_category = std::input_iterator_tag;
    using value_type = StrategySupportProfile;
    using difference_type = std::ptrdiff_t;
    using pointer = const StrategySupportProfile *;
    using reference = const StrategySupportProfile &;

    reference operator*() const { return *m_current; }
    pointer operator->() const { return &*m_current; }
    iterator &operator++()
    {
      m_current = m_generator->Next();
      return *this;
    }
    bool operator==(const iterator &p_other) const
    {
      return m_current.has_value() == p_other.m_current.has_value();
    }

  private:
    friend class PossibleNashStrategySupports;
    iterator(PossibleNashStrategySupports *p_generator,
             std::optional<StrategySupportProfile> p_current)
      : m_generator(p_generator), m_current(std::move(p_current))
    {
    }

    PossibleNashStrategySupports *m_generator;
    std::optional<StrategySupportProfile> m_current;
  };

  // NOTE: as this is a single-pass generator, begin() must be called only once; each call
  // pulls the first candidate from the underlying search.
  iterator begin() { return {this, Next()}; }
  iterator end() { return {this, std::nullopt}; }

private:
  class Impl;
  std::unique_ptr<Impl> m_impl;
};

// Enumerates, one at a time, the behavior support profiles which can be the support of a
// totally-mixed Nash equilibrium.
//
// This is a single-pass generator: construct one instance per game and pull candidates
// from it, either by repeated calls to Next() or via a range-based for loop.  Candidates
// are produced lazily, so a consumer that only needs the first few supports (e.g. to find
// up to k equilibria) never pays for the cost of enumerating the rest.
//
// TODO: This is a naive implementation that does not take into account that removing
//       actions from a support profile can (and often does) lead to information sets
//       becoming unreachable.
class PossibleNashBehaviorSupports {
public:
  explicit PossibleNashBehaviorSupports(const Game &p_game);
  ~PossibleNashBehaviorSupports();
  PossibleNashBehaviorSupports(PossibleNashBehaviorSupports &&) noexcept;
  PossibleNashBehaviorSupports &operator=(PossibleNashBehaviorSupports &&) noexcept;
  PossibleNashBehaviorSupports(const PossibleNashBehaviorSupports &) = delete;
  PossibleNashBehaviorSupports &operator=(const PossibleNashBehaviorSupports &) = delete;

  // Returns the next candidate support, or std::nullopt once the search is exhausted.
  std::optional<BehaviorSupportProfile> Next();

  class iterator {
  public:
    using iterator_category = std::input_iterator_tag;
    using value_type = BehaviorSupportProfile;
    using difference_type = std::ptrdiff_t;
    using pointer = const BehaviorSupportProfile *;
    using reference = const BehaviorSupportProfile &;

    reference operator*() const { return *m_current; }
    pointer operator->() const { return &*m_current; }
    iterator &operator++()
    {
      m_current = m_generator->Next();
      return *this;
    }
    bool operator==(const iterator &p_other) const
    {
      return m_current.has_value() == p_other.m_current.has_value();
    }

  private:
    friend class PossibleNashBehaviorSupports;
    iterator(PossibleNashBehaviorSupports *p_generator,
             std::optional<BehaviorSupportProfile> p_current)
      : m_generator(p_generator), m_current(std::move(p_current))
    {
    }

    PossibleNashBehaviorSupports *m_generator;
    std::optional<BehaviorSupportProfile> m_current;
  };

  // NOTE: as this is a single-pass generator, begin() must be called only once; each call
  // pulls the first candidate from the underlying search.
  iterator begin() { return {this, Next()}; }
  iterator end() { return {this, std::nullopt}; }

private:
  class Impl;
  std::unique_ptr<Impl> m_impl;
};

#endif // GAMBIT_SOLVERS_NASHSUPPORT_NASHSUPPORT_H
