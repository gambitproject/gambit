//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/core/util.h
// Core (game theory-independent) utilities for Gambit
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

#ifndef GAMBIT_CORE_UTIL_H
#define GAMBIT_CORE_UTIL_H

#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <map>
#include <optional>

namespace Gambit {

// Naming compatible with Boost's lexical_cast concept for potential future compatibility.

template <class D, class S> D lexical_cast(const S &p_value)
{
  std::ostringstream s;
  s << p_value;
  return s.str();
}

template <class D, class S> D lexical_cast(const S &p_value, int p_prec)
{
  std::ostringstream s;
  s.setf(std::ios::fixed);
  s << std::setprecision(p_prec) << p_value;
  return s.str();
}

inline double abs(double x) { return std::fabs(x); }

inline double sqr(double x) { return x * x; }

template <class C, class T> bool contains(const C &p_container, const T &p_value)
{
  return std::find(p_container.cbegin(), p_container.cend(), p_value) != p_container.cend();
}

template <class Key, class T> bool contains(const std::map<Key, T> &map, const Key &key)
// TODO: remove when we move to C++20 which already includes a "contains" method
{
  return map.find(key) != map.end();
}

template <class C> class EnumerateView {
public:
  explicit EnumerateView(C &p_range) : m_range(p_range) {}

  class iterator {
  public:
    using base_iterator = decltype(std::begin(std::declval<C &>()));

    iterator(const std::size_t p_index, base_iterator p_current)
      : m_index(p_index), m_current(p_current)
    {
    }

    iterator &operator++()
    {
      ++m_index;
      ++m_current;
      return *this;
    }

    bool operator!=(const iterator &p_other) const { return m_current != p_other.m_current; }
    auto operator*() const { return std::tie(m_index, *m_current); }

  private:
    std::size_t m_index;
    base_iterator m_current;
  };

  iterator begin() { return iterator{0, std::begin(m_range)}; }
  iterator end() { return iterator{0, std::end(m_range)}; }

private:
  C &m_range;
};

template <class C> auto enumerate(C &p_range) { return EnumerateView<C>(p_range); }

/// @brief A container adaptor which returns only the elements matching the predicate
///        This is intended to look forward to C++20-style ranges
template <typename Container, typename Pred> class filter_if {
public:
  using Iter = decltype(std::begin(std::declval<Container &>()));

  class iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = typename std::iterator_traits<Iter>::value_type;
    using difference_type = typename std::iterator_traits<Iter>::difference_type;
    using reference = typename std::iterator_traits<Iter>::reference;
    using pointer = typename std::iterator_traits<Iter>::pointer;

    iterator(Iter current, Iter end, Pred pred)
      : m_current(current), m_end(end), m_pred(std::move(pred))
    {
      advance_next_valid();
    }

    value_type operator*() const { return *m_current; }
    pointer operator->() const { return std::addressof(*m_current); }

    iterator &operator++()
    {
      ++m_current;
      advance_next_valid();
      return *this;
    }

    iterator operator++(int)
    {
      auto tmp = *this;
      ++(*this);
      return tmp;
    }

    friend bool operator==(const iterator &a, const iterator &b)
    {
      return a.m_current == b.m_current;
    }

    friend bool operator!=(const iterator &a, const iterator &b) { return !(a == b); }

  private:
    Iter m_current, m_end;
    Pred m_pred;

    void advance_next_valid()
    {
      while (m_current != m_end && !m_pred(*m_current)) {
        ++m_current;
      }
    }
  };

  filter_if(const Container &c, Pred pred)
    : m_begin(c.begin(), c.end(), pred), m_end(c.end(), c.end(), pred)
  {
  }

  iterator begin() const { return m_begin; }
  iterator end() const { return m_end; }

private:
  iterator m_begin, m_end;
};

template <typename Value, typename Range> class prepend_value {
public:
  using Iter = decltype(std::begin(std::declval<Range &>()));

  class iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    ;
    using value_type = Value;
    using difference_type = std::ptrdiff_t;
    using reference = Value;
    using pointer = Value;

    iterator(std::optional<Value> first, Iter current, Iter end)
      : m_first(std::move(first)), m_current(current), m_end(end)
    {
    }

    reference operator*() const { return m_first ? *m_first : *m_current; }

    iterator &operator++()
    {
      if (m_first) {
        m_first.reset();
      }
      else {
        ++m_current;
      }
      return *this;
    }

    bool operator==(const iterator &other) const
    {
      return m_first == other.m_first && m_current == other.m_current;
    }

    bool operator!=(const iterator &other) const { return !(*this == other); }

  private:
    std::optional<Value> m_first;
    Iter m_current, m_end;
  };

  prepend_value(Value first, Range range) : m_first(first), m_range(std::move(range)) {}

  iterator begin() const { return {m_first, std::begin(m_range), std::end(m_range)}; }

  iterator end() const { return {std::nullopt, std::end(m_range), std::end(m_range)}; }

private:
  Value m_first;
  Range m_range;
};

/// @brief Returns the maximum value of the function over the *non-empty* container
template <class Container, class Func>
auto maximize_function(const Container &p_container, const Func &p_function)
{
  auto it = p_container.begin();
  using T = decltype(p_function(*it));
  return std::transform_reduce(
      std::next(it), p_container.end(), p_function(*it),
      [](const T &a, const T &b) -> T { return std::max(a, b); }, p_function);
}

/// @brief Returns the minimum value of the function over the *non-empty* container
template <class Container, class Func>
auto minimize_function(const Container &p_container, const Func &p_function)
{
  auto it = p_container.begin();
  using T = decltype(p_function(*it));
  return std::transform_reduce(
      std::next(it), p_container.end(), p_function(*it),
      [](const T &a, const T &b) -> T { return std::min(a, b); }, p_function);
}

/// @brief Returns the sum of the function over the container
template <class Container, class Func>
auto sum_function(const Container &p_container, const Func &p_function)
{
  using T = decltype(p_function(*(p_container.begin())));
  return std::transform_reduce(p_container.begin(), p_container.end(), static_cast<T>(0),
                               std::plus<>{}, p_function);
}

//========================================================================
//                        Exception classes
//========================================================================

/// Exception thrown on dimension mismatches
class DimensionException final : public std::runtime_error {
public:
  DimensionException() : std::runtime_error("Mismatched dimensions") {}
  explicit DimensionException(const std::string &s) : std::runtime_error(s) {}
  ~DimensionException() noexcept override = default;
};

/// Exception thrown on invalid value
class ValueException final : public std::runtime_error {
public:
  ValueException() : std::runtime_error("Invalid value") {}
  explicit ValueException(const std::string &s) : std::runtime_error(s) {}
  ~ValueException() noexcept override = default;
};

/// Exception thrown on attempted division by zero
class ZeroDivideException final : public std::runtime_error {
public:
  ZeroDivideException() : std::runtime_error("Attempted division by zero") {}
  explicit ZeroDivideException(const std::string &s) : std::runtime_error(s) {}
  ~ZeroDivideException() noexcept override = default;
};

} // end namespace Gambit

#endif // GAMBIT_CORE_UTIL_H
