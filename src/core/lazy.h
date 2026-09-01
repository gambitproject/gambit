//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/core/lazy.h
// Lazily-computed, invalidatable cached values and setup actions
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

#ifndef GAMBIT_CORE_LAZY_H
#define GAMBIT_CORE_LAZY_H

#include <mutex>
#include <optional>
#include <utility>

namespace Gambit {

/// @brief A lazily-computed value, rebuilt on demand after invalidation.
///
/// Get() runs `p_builder` at most once since construction or the last
/// Invalidate() (std::call_once semantics), then returns the cached value on
/// every subsequent call without re-running the builder.
///
/// Concurrent Get() calls race safely: the standard guarantees exactly one
/// builder call completes before any Get() call returns, so multiple reader
/// threads first-touching the same Lazy<T> is well-defined. Invalidate() is
/// NOT safe to call concurrently with Get(); invalidation must happen only
/// when no reader could be in flight (e.g. before handing the owning object
/// to a reader thread, never while one is active).
///
/// Copyable, for embedding in value-type classes that are themselves copied
/// (e.g. a support object held by value inside a profile class) — but a copy
/// always starts un-built, regardless of the source's state. There is no
/// way to safely transplant an already-fired std::once_flag, and a fast-path
/// check of the source's cached value outside of call_once would reintroduce
/// the exact data race call_once exists to prevent. Concretely: copying an
/// already-built Lazy<T> does not carry the built value over; the copy
/// recomputes it lazily on its own next Get().
template <class T> class Lazy {
public:
  Lazy() = default;
  Lazy(const Lazy &) {}
  Lazy &operator=(const Lazy &)
  {
    Invalidate();
    return *this;
  }

  /// Get the cached value, computing it via `p_builder` if not already built
  /// (or if invalidated since the last build).
  template <class F> const T &Get(F &&p_builder) const
  {
    std::call_once(*m_flag, [&] { m_value = std::forward<F>(p_builder)(); });
    return *m_value;
  }

  /// Discard the cached value; the next Get() call recomputes it.
  void Invalidate() const
  {
    m_flag.emplace();
    m_value.reset();
  }

  /// If a value has been built, apply `p_visitor` to it; a no-op otherwise.
  /// Useful for releasing resources the cached value owns (e.g. cascading to
  /// invalidate objects it handed out) immediately before discarding it via
  /// Invalidate(), without forcing a rebuild just to inspect the old value.
  template <class F> void IfBuilt(F &&p_visitor) const
  {
    if (m_value) {
      std::forward<F>(p_visitor)(*m_value);
    }
  }

  /// Whether the value has been computed since the last invalidation.
  bool IsBuilt() const { return m_value.has_value(); }

private:
  // std::once_flag is neither copyable nor movable, so it is wrapped in
  // std::optional and reset via emplace() (placement-construct a fresh
  // flag) rather than assignment.
  mutable std::optional<std::once_flag> m_flag{std::in_place};
  mutable std::optional<T> m_value;
};

/// @brief Like Lazy<T>, but for an idempotent setup action with no single
/// value to hand back.
///
/// Some caches are populated by a routine that mutates several existing
/// members as a side effect (e.g. numbering nodes in place) rather than
/// producing one value to store. LazyAction captures the same "run once
/// since the last invalidation" semantics for that shape, sharing Lazy<T>'s
/// concurrency guarantees and its restriction against invalidating
/// concurrently with a call to Ensure().
///
/// Copyable, with the same "a copy always starts un-built" semantics as
/// Lazy<T> and for the same reason (see above).
class LazyAction {
public:
  LazyAction() = default;
  LazyAction(const LazyAction &) {}
  LazyAction &operator=(const LazyAction &)
  {
    Invalidate();
    return *this;
  }

  /// Run `p_action` if it has not already run since construction or the
  /// last Invalidate().
  template <class F> void Ensure(F &&p_action) const
  {
    std::call_once(*m_flag, [&] {
      std::forward<F>(p_action)();
      m_built = true;
    });
  }

  /// Mark the action as needing to run again on the next Ensure() call.
  void Invalidate() const
  {
    m_flag.emplace();
    m_built = false;
  }

  /// Whether the action has run since the last invalidation.
  bool IsBuilt() const { return m_built; }

private:
  mutable std::optional<std::once_flag> m_flag{std::in_place};
  mutable bool m_built{false};
};

} // namespace Gambit

#endif // GAMBIT_CORE_LAZY_H
