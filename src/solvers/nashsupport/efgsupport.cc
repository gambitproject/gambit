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

class ActionCursor {
protected:
  BehaviorSupportProfile m_support;
  int pl, iset, act;

public:
  // Lifecycle
  explicit ActionCursor(const BehaviorSupportProfile &S);
  ~ActionCursor() = default;

  // Operators
  ActionCursor &operator=(const ActionCursor &) = default;

  // Manipulation
  bool GoToNext();

  // State
  GameAction GetAction() const { return m_support.GetAction(pl, iset, act); }
};


ActionCursor::ActionCursor(const BehaviorSupportProfile &p_support)
  : m_support(p_support), pl(1), iset(1), act(1)
{
  Game efg = p_support.GetGame();

  // Make sure that (pl, iset) points to a valid information set.
  // It is permitted to have a game where a player has no information sets.
  for (; pl <= efg->NumPlayers(); pl++) {
    if (efg->GetPlayer(pl)->NumInfosets() > 0) {
      for (iset = 1; iset <= efg->GetPlayer(pl)->NumInfosets(); iset++) {
        if (m_support.NumActions(pl, iset) > 0) {
          return;
        }
      }
    }
  }
}


bool ActionCursor::GoToNext()
{
  if (act != m_support.NumActions(pl, iset)) {
    act++;
    return true;
  }

  int temppl = pl, tempiset = iset + 1;
  while (temppl <= m_support.GetGame()->NumPlayers()) {
    while (tempiset <= m_support.GetGame()->GetPlayer(temppl)->NumInfosets()) {
      if (m_support.NumActions(temppl, tempiset) > 0) {
        pl = temppl;
        iset = tempiset;
        act = 1;
        return true;
      }
      else {
        tempiset++;
      }
    }
    tempiset = 1;
    temppl++;
  }
  return false;
}

bool HasActiveActionsAtActiveInfosetsAndNoOthers(const BehaviorSupportProfile &p_support)
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


void
PossibleNashBehaviorSupports(const BehaviorSupportProfile &p_support,
                             const ActionCursor &p_cursor,
                             std::list<BehaviorSupportProfile> &p_list)
{
  ActionCursor copy(p_cursor);
  if (!copy.GoToNext()) {
    if (HasActiveActionsAtActiveInfosetsAndNoOthers(p_support)) {
      p_list.push_back(p_support);
    }

    BehaviorSupportProfile copySupport(p_support);
    copySupport.RemoveAction(p_cursor.GetAction());
    if (HasActiveActionsAtActiveInfosetsAndNoOthers(copySupport)) {
      p_list.push_back(copySupport);
    }
    return;
  }
  PossibleNashBehaviorSupports(p_support, copy, p_list);

  BehaviorSupportProfile copySupport(p_support);
  copySupport.RemoveAction(p_cursor.GetAction());
  PossibleNashBehaviorSupports(copySupport, copy, p_list);
}

}


//
// TODO: This is a naive implementation that does not take into account
//       that removing actions from a support profile can (and often does) lead
//       to information sets becoming unreachable.
//
std::shared_ptr<PossibleNashBehaviorSupportsResult> PossibleNashBehaviorSupports(const Game &p_game)
{
  BehaviorSupportProfile support(p_game);
  ActionCursor cursor(support);
  auto result = std::make_shared<PossibleNashBehaviorSupportsResult>();

  PossibleNashBehaviorSupports(support, cursor, result->m_supports);
  return result;
}
 
