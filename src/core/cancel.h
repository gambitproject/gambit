//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/core/cancel.h
// Cooperative cancellation of long-running computations
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

#ifndef GAMBIT_CORE_CANCEL_H
#define GAMBIT_CORE_CANCEL_H

#include <atomic>
#include <memory>
#include <stdexcept>

namespace Gambit {

/// @brief Thrown by CancelToken::Check() to unwind out of a computation when
/// cancellation has been requested.
class ComputationCanceledException : public std::runtime_error {
public:
  ComputationCanceledException() : std::runtime_error("Computation canceled") {}
};

/// @brief A cooperative cancellation flag, shared between the thread running
/// a long computation and the thread (e.g. a GUI) that may want to stop it
/// early.
///
/// A computation takes a CancelToken by const reference and calls Check() at
/// its own natural iteration points (once per candidate/pivot/step, not tied
/// to how often it happens to produce a result); Check() throws
/// ComputationCanceledException once RequestCancel() has been called from
/// another thread. A default-constructed token is never canceled, so
/// passing one costs nothing for callers that don't need cancellation.
class CancelToken {
public:
  void RequestCancel() const { m_canceled->store(true); }
  bool IsCanceled() const { return m_canceled->load(); }
  void Check() const
  {
    if (IsCanceled()) {
      throw ComputationCanceledException();
    }
  }

private:
  std::shared_ptr<std::atomic<bool>> m_canceled = std::make_shared<std::atomic<bool>>(false);
};

} // namespace Gambit

#endif // GAMBIT_CORE_CANCEL_H
