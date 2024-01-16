//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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

#include <iostream>
#include "efgensup.h"

// We build a series of functions of increasing complexity.  The
// final one, which is our goal, is the undominated support function.
// We begin by simply enumerating all subsupports.

void AllSubsupportsRECURSIVE(const Gambit::BehaviorSupportProfile &s,
                             Gambit::BehaviorSupportProfile *sact, ActionCursorForSupport *c,
                             Gambit::List<Gambit::BehaviorSupportProfile> &list)
{
  list.push_back(*sact);

  ActionCursorForSupport c_copy(*c);

  do {
    if (sact->Contains(c_copy.GetAction())) {
      sact->RemoveAction(c_copy.GetAction());
      AllSubsupportsRECURSIVE(s, sact, &c_copy, list);
      sact->AddAction(c_copy.GetAction());
    }
  } while (c_copy.GoToNext());
}

Gambit::List<Gambit::BehaviorSupportProfile>
AllSubsupports(const Gambit::BehaviorSupportProfile &S)
{
  Gambit::List<Gambit::BehaviorSupportProfile> answer;
  Gambit::BehaviorSupportProfile SAct(S);
  ActionCursorForSupport cursor(S);

  AllSubsupportsRECURSIVE(S, &SAct, &cursor, answer);

  return answer;
}

bool HasActiveActionsAtActiveInfosets(const Gambit::BehaviorSupportProfile &p_support)
{
  for (auto player : p_support.GetGame()->GetPlayers()) {
    for (auto infoset : player->GetInfosets()) {
      if (p_support.IsReachable(infoset) && !p_support.HasAction(infoset)) {
        return false;
      }
    }
  }
  return true;
}

bool HasActiveActionsAtActiveInfosetsAndNoOthers(const Gambit::BehaviorSupportProfile &p_support)
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

// Subsupports of a given support are _path equivalent_ if they
// agree on every infoset that can be reached under either, hence both,
// of them.  The next routine outputs one support for each equivalence
// class.  It is not for use in solution routines, but is instead a
// prototype of the eventual path enumerator, which will also perform
// dominance elimination.

void AllInequivalentSubsupportsRECURSIVE(const Gambit::BehaviorSupportProfile &s,
                                         Gambit::BehaviorSupportProfile *sact,
                                         ActionCursorForSupport *c,
                                         Gambit::List<Gambit::BehaviorSupportProfile> &list)
{
  if (HasActiveActionsAtActiveInfosetsAndNoOthers(*sact)) {
    list.push_back(*sact);
  }

  ActionCursorForSupport c_copy(*c);

  do {
    if (sact->Contains(c_copy.GetAction())) {

      Gambit::List<Gambit::GameInfoset> deactivated_infosets;
      sact->RemoveAction(c_copy.GetAction(), deactivated_infosets);

      if (!c_copy.DeletionsViolateActiveCommitments(sact, &deactivated_infosets)) {
        AllInequivalentSubsupportsRECURSIVE(s, sact, &c_copy, list);
      }
      sact->AddAction(c_copy.GetAction());
    }
  } while (c_copy.GoToNext());
}

Gambit::List<Gambit::BehaviorSupportProfile>
AllInequivalentSubsupports(const Gambit::BehaviorSupportProfile &S)
{
  Gambit::List<Gambit::BehaviorSupportProfile> answer;
  Gambit::BehaviorSupportProfile SAct(S);
  ActionCursorForSupport cursor(S);

  AllInequivalentSubsupportsRECURSIVE(S, &SAct, &cursor, answer);

  return answer;
}

void AllUndominatedSubsupportsRECURSIVE(const Gambit::BehaviorSupportProfile &s,
                                        Gambit::BehaviorSupportProfile *sact,
                                        ActionCursorForSupport *c, bool strong, bool conditional,
                                        Gambit::List<Gambit::BehaviorSupportProfile> &list)
{
  bool abort = false;
  bool no_deletions = true;
  bool check_domination = false;
  if (HasActiveActionsAtActiveInfosets(*sact)) {
    check_domination = true;
  }
  Gambit::List<Gambit::GameAction> deletion_list;
  ActionCursorForSupport scanner(s);

  // First we collect all the actions that can be deleted.
  do {
    Gambit::GameAction this_action = scanner.GetAction();
    bool delete_this_action = false;

    if (sact->Contains(this_action)) {
      if (!sact->IsReachable(this_action->GetInfoset())) {
        delete_this_action = true;
      }
      else {
        if (check_domination) {
          if (sact->IsDominated(this_action, strong, conditional)) {
            delete_this_action = true;
          }
        }
      }
    }

    if (delete_this_action) {
      no_deletions = false;
      if (c->IsSubsequentTo(this_action)) {
        abort = true;
      }
      else {
        deletion_list.push_back(this_action);
      }
    }
  } while (!abort && scanner.GoToNext());

  // Now we delete them, recurse, then restore
  if (!abort && !no_deletions) {
    Gambit::List<Gambit::GameAction> actual_deletions;
    for (int i = 1; !abort && i <= deletion_list.Length(); i++) {
      actual_deletions.push_back(deletion_list[i]);
      Gambit::List<Gambit::GameInfoset> deactivated_infosets;

      sact->RemoveAction(deletion_list[i], deactivated_infosets);

      if (c->DeletionsViolateActiveCommitments(sact, &deactivated_infosets)) {
        abort = true;
      }
    }

    if (!abort) {
      AllUndominatedSubsupportsRECURSIVE(s, sact, c, strong, conditional, list);
    }

    for (int i = 1; i <= actual_deletions.Length(); i++) {
      sact->AddAction(actual_deletions[i]);
    }
  }

  // If there are no deletions, we ask if it is consistent, then recurse.
  if (!abort && no_deletions) {
    if (HasActiveActionsAtActiveInfosetsAndNoOthers(*sact)) {
      list.push_back(*sact);
    }

    ActionCursorForSupport c_copy(*c);

    do {
      if (sact->Contains(c_copy.GetAction())) {

        Gambit::List<Gambit::GameInfoset> deactivated_infosets;
        sact->RemoveAction(c_copy.GetAction(), deactivated_infosets);

        if (!c_copy.DeletionsViolateActiveCommitments(sact, &deactivated_infosets)) {
          AllUndominatedSubsupportsRECURSIVE(s, sact, &c_copy, strong, conditional, list);
        }
        sact->AddAction(c_copy.GetAction());
      }
    } while (c_copy.GoToNext());
  }
}

Gambit::List<Gambit::BehaviorSupportProfile>
AllUndominatedSubsupports(const Gambit::BehaviorSupportProfile &S, bool strong, bool conditional)
{
  Gambit::List<Gambit::BehaviorSupportProfile> answer;
  Gambit::BehaviorSupportProfile sact(S);
  ActionCursorForSupport cursor(S);

  AllUndominatedSubsupportsRECURSIVE(S, &sact, &cursor, strong, conditional, answer);

  return answer;
}

void PossibleNashSubsupportsRECURSIVE(const Gambit::BehaviorSupportProfile &s,
                                      Gambit::BehaviorSupportProfile *sact,
                                      ActionCursorForSupport *c,
                                      Gambit::List<Gambit::BehaviorSupportProfile> &list)
{
  bool abort = false;
  bool add_support = true;

  bool check_domination = false;
  if (HasActiveActionsAtActiveInfosets(*sact)) {
    check_domination = true;
  }
  Gambit::List<Gambit::GameAction> deletion_list;
  ActionCursorForSupport scanner(s);

  do {
    Gambit::GameAction this_action = scanner.GetAction();
    bool delete_this_action = false;

    if (sact->Contains(this_action)) {
      if (!sact->IsReachable(this_action->GetInfoset())) {
        delete_this_action = true;
      }
      else {
        if (check_domination) {
          if (sact->IsDominated(this_action, true, true) ||
              sact->IsDominated(this_action, true, false)) {
            add_support = false;
            if (c->InfosetGuaranteedActiveByPriorCommitments(sact, this_action->GetInfoset())) {
              delete_this_action = true;
            }
          }
        }
      }
    }

    if (delete_this_action) {
      if (c->IsSubsequentTo(this_action)) {
        abort = true;
      }
      else {
        deletion_list.push_back(this_action);
      }
    }
  } while (!abort && scanner.GoToNext());

  std::cout << "length of deletion list = " << deletion_list.Length() << std::endl;
  if (!abort) {
    Gambit::List<Gambit::GameAction> actual_deletions;
    for (int i = 1; !abort && i <= deletion_list.Length(); i++) {
      Gambit::List<Gambit::GameInfoset> deactivated_infosets;
      sact->RemoveAction(deletion_list[i], deactivated_infosets);
      if (c->DeletionsViolateActiveCommitments(sact, &deactivated_infosets)) {
        abort = true;
      }
      actual_deletions.push_back(deletion_list[i]);
    }

    if (!abort && deletion_list.Length() > 0) {
      PossibleNashSubsupportsRECURSIVE(s, sact, c, list);
    }

    for (int i = 1; i <= actual_deletions.Length(); i++) {
      sact->AddAction(actual_deletions[i]);
    }
  }

  if (!abort && deletion_list.Length() == 0) {

    if (add_support && HasActiveActionsAtActiveInfosetsAndNoOthers(*sact)) {
      list.push_back(*sact);
    }
    ActionCursorForSupport c_copy(*c);
    do {
      if (sact->Contains(c_copy.GetAction())) {
        Gambit::List<Gambit::GameInfoset> deactivated_infosets;
        sact->RemoveAction(c_copy.GetAction(), deactivated_infosets);
        if (!c_copy.DeletionsViolateActiveCommitments(sact, &deactivated_infosets)) {
          PossibleNashSubsupportsRECURSIVE(s, sact, &c_copy, list);
        }

        sact->AddAction(c_copy.GetAction());
      }
    } while (c_copy.GoToNext());
  }
}

Gambit::List<Gambit::BehaviorSupportProfile>
SortSupportsBySize(Gambit::List<Gambit::BehaviorSupportProfile> &list)
{
  Gambit::Array<int> sizes(list.Length());
  for (int i = 1; i <= list.Length(); i++) {
    sizes[i] = list[i].BehaviorProfileLength();
  }

  Gambit::Array<int> listproxy(list.Length());
  for (int i = 1; i <= list.Length(); i++) {
    listproxy[i] = i;
  }

  int maxsize(0);
  for (int i = 1; i <= list.Length(); i++) {
    if (sizes[i] > maxsize) {
      maxsize = sizes[i];
    }
  }

  int cursor(1);

  for (int j = 0; j < maxsize; j++) {
    int scanner(list.Length());
    while (cursor < scanner) {
      if (sizes[scanner] != j) {
        scanner--;
      }
      else {
        while (sizes[cursor] == j) {
          cursor++;
        }
        if (cursor < scanner) {
          int tempindex = listproxy[cursor];
          listproxy[cursor] = listproxy[scanner];
          listproxy[scanner] = tempindex;
          int tempsize = sizes[cursor];
          sizes[cursor] = sizes[scanner];
          sizes[scanner] = tempsize;
          cursor++;
        }
      }
    }
  }

  Gambit::List<Gambit::BehaviorSupportProfile> answer;
  for (int i = 1; i <= list.Length(); i++) {
    answer.push_back(list[listproxy[i]]);
  }

  return answer;
}

namespace { // to keep the recursive function private

using namespace Gambit;

#ifdef UNUSED
void PrintSupport(std::ostream &p_stream, const std::string &p_label,
                  const Gambit::BehaviorSupportProfile &p_support)
{
  p_stream << p_label;

  for (int pl = 1; pl <= p_support.GetGame()->NumPlayers(); pl++) {
    GamePlayer player = p_support.GetGame()->GetPlayer(pl);

    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      GameInfoset infoset = player->GetInfoset(iset);

      p_stream << ",";

      for (int act = 1; act <= infoset->NumActions(); act++) {
        if (p_support.Contains(infoset->GetAction(act))) {
          p_stream << "1";
        }
        else {
          p_stream << "0";
        }
      }
    }
  }
  p_stream << std::endl;
}
#endif // UNUSED

void PossibleNashSubsupports(const Gambit::BehaviorSupportProfile &p_support,
                             const ActionCursorForSupport &p_cursor,
                             Gambit::List<Gambit::BehaviorSupportProfile> &p_list)
{
  ActionCursorForSupport copy(p_cursor);
  if (!copy.GoToNext()) {
    if (HasActiveActionsAtActiveInfosetsAndNoOthers(p_support)) {
      p_list.push_back(p_support);
    }

    Gambit::BehaviorSupportProfile copySupport(p_support);
    copySupport.RemoveAction(p_cursor.GetAction());
    if (HasActiveActionsAtActiveInfosetsAndNoOthers(copySupport)) {
      p_list.push_back(copySupport);
    }
    return;
  }
  PossibleNashSubsupports(p_support, copy, p_list);

  Gambit::BehaviorSupportProfile copySupport(p_support);
  copySupport.RemoveAction(p_cursor.GetAction());
  PossibleNashSubsupports(copySupport, copy, p_list);
}

} // namespace

//
// This is a naive version of the real thing.  The original one
// (see below, commented out) no longer works after underlying API
// changes.  I suspect the problem is that the behavior support class
// was hacked to make this work, and recent refactorings have undone
// the hacks it depended on.  So, for now, we do a really simple
// implementation that works (but may be inefficient), and we will
// go from there later.
//
Gambit::List<Gambit::BehaviorSupportProfile>
PossibleNashSubsupports(const Gambit::BehaviorSupportProfile &p_support)
{
  ActionCursorForSupport cursor(p_support);
  Gambit::List<Gambit::BehaviorSupportProfile> supports;

  PossibleNashSubsupports(p_support, cursor, supports);
  return supports;
}

/*
Gambit::List<Gambit::BehaviorSupportProfile> PossibleNashSubsupports(const
Gambit::BehaviorSupportProfile &S)
{
  Gambit::List<Gambit::BehaviorSupportProfile> answer;
  Gambit::BehaviorSupportProfile sact(S);
  ActionCursorForSupport cursor(S);

  PossibleNashSubsupportsRECURSIVE(S, &sact, &cursor, answer);

  // At this point answer has all consistent subsupports without
  // any strong dominations.  We now edit the list, removing all
  // subsupports that exhibit weak dominations, and we also eliminate
  // subsupports exhibiting domination by currently inactive actions.

  for (int i = answer.Length(); i >= 1; i--) {
    Gambit::BehaviorSupportProfile current(answer[i]);
    ActionCursorForSupport crsr(S);
    bool remove = false;
    do {
      Gambit::GameAction act = crsr.GetAction();
      if (current.Contains(act))
        for (int j = 1; j <= act->GetInfoset()->NumActions(); j++) {
          Gambit::GameAction other_act = act->GetInfoset()->GetAction(j);
          if (other_act != act)
            if (current.Contains(other_act)) {
              if (current.Dominates(other_act,act,false,true) ||
                  current.Dominates(other_act,act,false,false))
                remove = true;
            }
            else {
              current.AddAction(other_act);
              if (current.HasActiveActionsAtActiveInfosetsAndNoOthers())
                if (current.Dominates(other_act,act,false,true) ||
                    current.Dominates(other_act,act,false,false)) {
                  remove = true;
                }
              current.RemoveAction(other_act);
            }
      }
    } while (crsr.GoToNext() && !remove);
    if (remove)
      answer.Remove(i);
  }

  return SortSupportsBySize(answer);
}
*/

//----------------------------------------------------
//                ActionCursorForSupport
// ---------------------------------------------------

ActionCursorForSupport::ActionCursorForSupport(const Gambit::BehaviorSupportProfile &S)
  : support(S), pl(1), iset(1), act(1)
{
  Gambit::Game efg = S.GetGame();

  // Make sure that (pl,iset) points to a valid information set:
  // when solving by subgames, some players may not have any information
  // sets at all.
  for (pl = 1; pl <= efg->NumPlayers(); pl++) {
    if (efg->GetPlayer(pl)->NumInfosets() > 0) {
      for (iset = 1; iset <= efg->GetPlayer(pl)->NumInfosets(); iset++) {
        if (S.NumActions(pl, iset) > 0) {
          return;
        }
      }
    }
  }
}

ActionCursorForSupport::ActionCursorForSupport(const ActionCursorForSupport &ac)

    = default;

ActionCursorForSupport::~ActionCursorForSupport() = default;

ActionCursorForSupport &ActionCursorForSupport::operator=(const ActionCursorForSupport &rhs)
{
  if (this != &rhs) {
    support = rhs.support;
    pl = rhs.pl;
    iset = rhs.iset;
    act = rhs.act;
  }
  return *this;
}

bool ActionCursorForSupport::operator==(const ActionCursorForSupport &rhs) const
{
  if (support != rhs.support || pl != rhs.pl || iset != rhs.iset || act != rhs.act) {
    return false;
  }
  return true;
}

bool ActionCursorForSupport::operator!=(const ActionCursorForSupport &rhs) const
{
  return (!(*this == rhs));
}

bool ActionCursorForSupport::GoToNext()
{
  if (act != support.NumActions(pl, iset)) {
    act++;
    return true;
  }

  int temppl(pl);
  int tempiset(iset);
  tempiset++;

  while (temppl <= support.GetGame()->NumPlayers()) {
    while (tempiset <= support.GetGame()->GetPlayer(temppl)->NumInfosets()) {
      if (support.NumActions(temppl, tempiset) > 0) {
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

Gambit::GameAction ActionCursorForSupport::GetAction() const
{
  return support.GetAction(pl, iset, act);
}

int ActionCursorForSupport::ActionIndex() const { return act; }

Gambit::GameInfoset ActionCursorForSupport::GetInfoset() const
{
  return support.GetGame()->GetPlayer(pl)->GetInfoset(iset);
}

int ActionCursorForSupport::InfosetIndex() const { return iset; }

Gambit::GamePlayer ActionCursorForSupport::GetPlayer() const
{
  return support.GetGame()->GetPlayer(pl);
}

int ActionCursorForSupport::PlayerIndex() const { return pl; }

bool ActionCursorForSupport::IsLast() const
{
  if (pl == support.GetGame()->NumPlayers()) {
    if (iset == support.GetGame()->GetPlayer(pl)->NumInfosets()) {
      if (act == support.NumActions(pl, iset)) {
        return true;
      }
    }
  }
  return false;
}

bool ActionCursorForSupport::IsSubsequentTo(const Gambit::GameAction &a) const
{
  if (pl > a->GetInfoset()->GetPlayer()->GetNumber()) {
    return true;
  }
  else if (pl < a->GetInfoset()->GetPlayer()->GetNumber()) {
    return false;
  }
  else if (iset > a->GetInfoset()->GetNumber()) {
    return true;
  }
  else if (iset < a->GetInfoset()->GetNumber()) {
    return false;
  }
  else if (act > a->GetNumber()) {
    return true;
  }
  else {
    return false;
  }
}

bool ActionCursorForSupport::DeletionsViolateActiveCommitments(
    const Gambit::BehaviorSupportProfile *S, const Gambit::List<Gambit::GameInfoset> *infosetlist)
{
  for (int i = 1; i <= infosetlist->Length(); i++) {
    Gambit::GameInfoset infoset = (*infosetlist)[i];
    if (infoset->GetPlayer()->GetNumber() < PlayerIndex() ||
        (infoset->GetPlayer()->GetNumber() == PlayerIndex() &&
         infoset->GetNumber() < InfosetIndex())) {
      if (S->NumActions(infoset->GetPlayer()->GetNumber(), infoset->GetNumber()) > 0) {
        return true;
      }
    }
    if (infoset->GetPlayer()->GetNumber() == PlayerIndex() &&
        infoset->GetNumber() == InfosetIndex()) {
      for (int action = 1; action < ActionIndex(); action++) {
        if (S->Contains(infoset->GetAction(action))) {
          return true;
        }
      }
    }
  }
  return false;
}

bool ActionCursorForSupport::InfosetGuaranteedActiveByPriorCommitments(
    const Gambit::BehaviorSupportProfile *S, const Gambit::GameInfoset &infoset)
{
  Gambit::List<Gambit::GameNode> members;
  for (int i = 1; i <= infoset->NumMembers(); i++) {
    members.push_back(infoset->GetMember(i));
  }

  for (int i = 1; i <= members.Length(); i++) {
    Gambit::GameNode current = members[i];
    if (current == S->GetGame()->GetRoot()) {
      return true;
    }
    else {
      while (S->Contains(current->GetPriorAction()) && IsSubsequentTo(current->GetPriorAction())) {
        current = current->GetParent();
        if (current == S->GetGame()->GetRoot()) {
          return true;
        }
      }
    }
  }
  return false;
}
