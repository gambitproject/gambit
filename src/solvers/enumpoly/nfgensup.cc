//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/nfgensup.cc
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

#include "nfgensup.h"

using namespace Gambit;

namespace {

List<StrategySupportProfile> SortSupportsBySize(List<StrategySupportProfile> &p_list)
{
  Array<int> sizes(p_list.Length());
  for (int i = 1; i <= p_list.Length(); i++) {
    sizes[i] = p_list[i].MixedProfileLength();
  }

  Array<int> listproxy(p_list.Length());
  for (int i = 1; i <= p_list.Length(); i++) {
    listproxy[i] = i;
  }

  int maxsize = 0;
  for (int i = 1; i <= p_list.Length(); i++) {
    if (sizes[i] > maxsize) {
      maxsize = sizes[i];
    }
  }

  int cursor = 1;

  for (int j = 0; j < maxsize; j++) {
    int scanner(p_list.Length());
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

  List<StrategySupportProfile> answer;
  for (int i = 1; i <= p_list.Length(); i++) {
    answer.push_back(p_list[listproxy[i]]);
  }

  return answer;
}

void PossibleNashSubsupports(const StrategySupportProfile &s, StrategySupportProfile &sact,
                             StrategySupportProfile::iterator &c,
                             List<StrategySupportProfile> &p_list)
{
  bool abort = false;
  bool no_deletions = true;

  List<GameStrategy> deletion_list;
  StrategySupportProfile::iterator scanner(s);

  do {
    GameStrategy this_strategy = scanner.GetStrategy();
    bool delete_this_strategy = false;
    if (sact.Contains(this_strategy)) {
      if (sact.IsDominated(this_strategy, true)) {
        delete_this_strategy = true;
      }
    }
    if (delete_this_strategy) {
      no_deletions = false;
      if (c.IsSubsequentTo(this_strategy)) {
        abort = true;
      }
      else {
        deletion_list.push_back(this_strategy);
      }
    }
  } while (!abort && scanner.GoToNext());

  if (!abort) {
    List<GameStrategy> actual_deletions;
    for (int i = 1; !abort && i <= deletion_list.Length(); i++) {
      actual_deletions.push_back(deletion_list[i]);
      sact.RemoveStrategy(deletion_list[i]);
    }

    if (!abort && deletion_list.Length() > 0) {
      PossibleNashSubsupports(s, sact, c, p_list);
    }

    for (int i = 1; i <= actual_deletions.Length(); i++) {
      sact.AddStrategy(actual_deletions[i]);
    }
  }
  if (!abort && no_deletions) {
    p_list.push_back(sact);

    StrategySupportProfile::iterator c_copy(c);
    do {
      GameStrategy str_ptr = c_copy.GetStrategy();
      if (sact.Contains(str_ptr) && sact.NumStrategies(str_ptr->GetPlayer()->GetNumber()) > 1) {
        sact.RemoveStrategy(str_ptr);
        PossibleNashSubsupports(s, sact, c_copy, p_list);
        sact.AddStrategy(str_ptr);
      }
    } while (c_copy.GoToNext());
  }
}

} // end anonymous namespace

List<StrategySupportProfile> PossibleNashSubsupports(const StrategySupportProfile &S)
{
  List<StrategySupportProfile> answer;
  StrategySupportProfile sact(S);
  StrategySupportProfile::iterator cursor(S);
  PossibleNashSubsupports(S, sact, cursor, answer);

  // At this point answer has all consistent subsupports without
  // any strong dominations.  We now edit the list, removing all
  // subsupports that exhibit weak dominations, and we also eliminate
  // subsupports exhibiting domination by currently inactive strategies.

  for (int i = answer.Length(); i >= 1; i--) {
    StrategySupportProfile current(answer[i]);
    StrategySupportProfile::iterator crsr(S);
    bool remove = false;
    do {
      GameStrategy strat = crsr.GetStrategy();
      if (current.Contains(strat)) {
        for (int j = 1; j <= strat->GetPlayer()->NumStrategies(); j++) {
          GameStrategy other_strat = strat->GetPlayer()->GetStrategy(j);
          if (other_strat != strat) {
            if (current.Contains(other_strat)) {
              if (current.Dominates(other_strat, strat, false)) {
                remove = true;
              }
            }
            else {
              current.AddStrategy(other_strat);
              if (current.Dominates(other_strat, strat, false)) {
                remove = true;
              }
              current.RemoveStrategy(other_strat);
            }
          }
        }
      }
    } while (crsr.GoToNext() && !remove);
    if (remove) {
      answer.Remove(i);
    }
  }

  return SortSupportsBySize(answer);
  return answer;
}
