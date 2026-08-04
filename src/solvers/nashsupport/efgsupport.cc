//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/efgensup.cc
// Enumerate undominated subsupports
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

#include "nashsupport.h"

namespace { // to keep this helper private

using namespace Gambit;

bool AllActionsReachable(const BehaviorSupportProfile &p_support)
{
  for (auto player : p_support.GetGame()->GetPlayers()) {
    for (auto infoset : player->GetInfosets()) {
      if (p_support.IsReachable(infoset) != p_support.HasAction(infoset)) {
        return false;
      }
    }
  }
  return true;
}

} // namespace

// Candidates are the 2^n ways of choosing, for each action in the game (in player/infoset/
// action order), whether to keep it or remove it from the full support.  This is walked as
// a binary odometer over `m_removeFlags`, with the last action in the list as the
// fastest-changing (rightmost) digit.
class PossibleNashBehaviorSupports::Impl {
public:
  explicit Impl(const Game &p_game) : m_fullSupport(p_game)
  {
    for (const auto &player : p_game->GetPlayers()) {
      for (const auto &infoset : player->GetInfosets()) {
        for (const auto &action : infoset->GetActions()) {
          m_actions.push_back(action);
        }
      }
    }
    m_removeFlags.assign(m_actions.size(), false);
  }

  std::optional<BehaviorSupportProfile> Next()
  {
    while (Advance()) {
      BehaviorSupportProfile candidate(m_fullSupport);
      for (size_t i = 0; i < m_actions.size(); i++) {
        if (m_removeFlags[i]) {
          candidate.RemoveAction(m_actions[i]);
        }
      }
      if (AllActionsReachable(candidate)) {
        return candidate;
      }
    }
    return std::nullopt;
  }

private:
  // Moves to the next combination of remove-flags.  The first call leaves the
  // all-kept (full-support) combination in place; later calls increment the odometer.
  // Returns false once every combination has been produced.
  bool Advance()
  {
    if (m_exhausted) {
      return false;
    }
    if (m_first) {
      m_first = false;
      return true;
    }
    for (size_t i = m_removeFlags.size(); i-- > 0;) {
      if (!m_removeFlags[i]) {
        m_removeFlags[i] = true;
        return true;
      }
      m_removeFlags[i] = false;
    }
    m_exhausted = true;
    return false;
  }

  BehaviorSupportProfile m_fullSupport;
  std::vector<GameAction> m_actions;
  std::vector<bool> m_removeFlags;
  bool m_first = true;
  bool m_exhausted = false;
};

PossibleNashBehaviorSupports::PossibleNashBehaviorSupports(const Game &p_game)
  : m_impl(std::make_unique<Impl>(p_game))
{
}

PossibleNashBehaviorSupports::~PossibleNashBehaviorSupports() = default;
PossibleNashBehaviorSupports::PossibleNashBehaviorSupports(
    PossibleNashBehaviorSupports &&) noexcept = default;
PossibleNashBehaviorSupports &
PossibleNashBehaviorSupports::operator=(PossibleNashBehaviorSupports &&) noexcept = default;

std::optional<BehaviorSupportProfile> PossibleNashBehaviorSupports::Next()
{
  return m_impl->Next();
}
