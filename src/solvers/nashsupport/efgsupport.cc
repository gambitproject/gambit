//
// This file is part of Gambit
// Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
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

namespace { // to keep the recursive function private

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

void PossibleNashBehaviorSupports(const BehaviorSupportProfile &p_support,
                                  const std::list<GameAction>::iterator &p_cursor,
                                  const std::list<GameAction>::iterator &p_end,
                                  std::list<BehaviorSupportProfile> &p_list)
{
  auto copy = std::next(p_cursor);
  if (copy == p_end) {
    if (AllActionsReachable(p_support)) {
      p_list.push_back(p_support);
    }

    BehaviorSupportProfile copySupport(p_support);
    copySupport.RemoveAction(*p_cursor);
    if (AllActionsReachable(copySupport)) {
      p_list.push_back(copySupport);
    }
  }
  else {
    PossibleNashBehaviorSupports(p_support, copy, p_end, p_list);

    BehaviorSupportProfile copySupport(p_support);
    copySupport.RemoveAction(*p_cursor);
    PossibleNashBehaviorSupports(copySupport, copy, p_end, p_list);
  }
}

} // namespace

//
// TODO: This is a naive implementation that does not take into account
//       that removing actions from a support profile can (and often does) lead
//       to information sets becoming unreachable.
//
std::shared_ptr<PossibleNashBehaviorSupportsResult>
PossibleNashBehaviorSupports(const Game &p_game)
{
  BehaviorSupportProfile support(p_game);
  auto result = std::make_shared<PossibleNashBehaviorSupportsResult>();

  std::list<GameAction> actions;
  for (const auto &player : p_game->GetPlayers()) {
    for (const auto &infoset : player->GetInfosets()) {
      for (const auto &action : infoset->GetActions()) {
        actions.push_back(action);
      }
    }
  }

  PossibleNashBehaviorSupports(support, actions.begin(), actions.end(), result->m_supports);
  return result;
}
