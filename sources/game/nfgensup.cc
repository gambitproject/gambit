//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Enumerate undominated subsupports of a support
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
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
#include "nfdom.h"

// We build a series of functions of increasing complexity.  The
// final one, which is our goal, is the undominated support function.
// We begin by simply enumerating all subsupports.

void AllSubsupportsRECURSIVE(const gbtNfgSupport *s,
			           gbtNfgSupport *sact,
			           gbtStrategyIterator *c,
			           gList<const gbtNfgSupport> *list)
{ 
  (*list) += *sact;

  gbtStrategyIterator c_copy(*c);

  do {
    Strategy *str_ptr = (Strategy *)c_copy.GetStrategy();
    if (sact->Contains(str_ptr) ) {
      sact->RemoveStrategy(str_ptr);
      AllSubsupportsRECURSIVE(s,sact,&c_copy,list);
      sact->AddStrategy(str_ptr);
    }
  } while (c_copy.GoToNext()) ;
}

gList<const gbtNfgSupport> AllSubsupports(const gbtNfgSupport &S)
{
  gList<const gbtNfgSupport> answer;
  gbtNfgSupport SAct(S);
  gbtStrategyIterator cursor(S);

  AllSubsupportsRECURSIVE(&S,&SAct,&cursor,&answer);

  return answer;
}

// In the next two routines we only output subsupports that
// have at least one active strategy for each agent.

void AllValidSubsupportsRECURSIVE(const gbtNfgSupport *s,
                                         gbtNfgSupport *sact,
					 gbtStrategyIterator *c,
					 gList<const gbtNfgSupport> *list)
{ 
  (*list) += *sact;

  gbtStrategyIterator c_copy(*c);

  do {
    if ( sact->NumStrats(c_copy.GetPlayerId()) > 1 ) {
      Strategy *str_ptr = (Strategy *)c_copy.GetStrategy();
      sact->RemoveStrategy(str_ptr); 
      AllValidSubsupportsRECURSIVE(s,sact,&c_copy,list);
      sact->AddStrategy(str_ptr);
    }
  } while (c_copy.GoToNext()) ;
}

gList<const gbtNfgSupport> AllValidSubsupports(const gbtNfgSupport &S)
{
  gList<const gbtNfgSupport> answer;
  gbtNfgSupport SAct(S);
  gbtStrategyIterator cursor(S);

  AllValidSubsupportsRECURSIVE(&S,&SAct,&cursor,&answer);

  return answer;
}

void AllUndominatedSubsupportsRECURSIVE(const gbtNfgSupport *s,
					      gbtNfgSupport *sact,
					      gbtStrategyIterator *c,
					const bool strong,
					const bool conditional,
					      gList<const gbtNfgSupport> *list,
					gStatus &status)
{ 
  bool abort = false;
  bool no_deletions = true;


  gList<Strategy *> deletion_list;
  gbtStrategyIterator scanner(*s);

  // First we collect all the strategies that can be deleted.
  do {
    Strategy *this_strategy = (Strategy *)scanner.GetStrategy();
    bool delete_this_strategy = false;
    if ( sact->Contains(this_strategy) ) 
      if (sact->IsDominated(this_strategy,strong) ) 
	delete_this_strategy = true;
	
    if (delete_this_strategy) {
      no_deletions = false;
      if (c->IsSubsequentTo(this_strategy)) 
	abort = true;
      else 
	deletion_list += this_strategy;
    }
  } while (!abort && scanner.GoToNext());

  // Now we delete them, recurse, then restore
  if (!abort && !no_deletions) {
    gList<Strategy *> actual_deletions;
    for (int i = 1; !abort && i <= deletion_list.Length(); i++) {
      actual_deletions += deletion_list[i];

      sact->RemoveStrategy(deletion_list[i]); 
    }

    if (!abort)
      AllUndominatedSubsupportsRECURSIVE(s,
					 sact,
					 c,
					 strong,
					 conditional,
					 list,
					 status);
    
    for (int i = 1; i <= actual_deletions.Length(); i++)
      sact->AddStrategy(actual_deletions[i]);
  }

  // If there are no deletions, we ask if it is consistent, then recurse.
  if (!abort && no_deletions) {
    (*list) += *sact;
    
    gbtStrategyIterator c_copy(*c);
    
    do {
      if ( sact->Contains((Strategy *)c_copy.GetStrategy()) &&
	   sact->NumStrats(c_copy.GetPlayerId()) > 1 ) {

	Strategy *str_ptr = (Strategy *)c_copy.GetStrategy();
	sact->RemoveStrategy(str_ptr); 
	AllUndominatedSubsupportsRECURSIVE(s,
					   sact,
					   &c_copy,
					   strong,
					   conditional,
					   list,
					   status);
	sact->AddStrategy(str_ptr);
	
      }
    } while (c_copy.GoToNext()) ;
  }
}
  
gList<const gbtNfgSupport> AllUndominatedSubsupports(const gbtNfgSupport &S,
						 const bool strong,
						 const bool conditional,
						 gStatus &status)
{
  gList<const gbtNfgSupport> answer;
  gbtNfgSupport sact(S);
  gbtStrategyIterator cursor(S);

  AllUndominatedSubsupportsRECURSIVE(&S,
				     &sact,
				     &cursor,
				     strong,
				     conditional,
				     &answer,
				     status);

  return answer;
}


void PossibleNashSubsupportsRECURSIVE(const gbtNfgSupport *s,
					    gbtNfgSupport *sact,
				            gbtStrategyIterator *c,
				            gList<const gbtNfgSupport> *list,
				      gStatus &status)
{ 
  status.Get();

  bool abort = false;
  bool no_deletions = true;

  gList<Strategy *> deletion_list;
  gbtStrategyIterator scanner(*s);

  do {
    Strategy *this_strategy = (Strategy *)scanner.GetStrategy();
    bool delete_this_strategy = false;
    if ( sact->Contains(this_strategy) ) 
      if (sact->IsDominated(this_strategy,true) ) {
	delete_this_strategy = true;
      }
    if (delete_this_strategy) {
      no_deletions = false;
      if (c->IsSubsequentTo(this_strategy)) 
	abort = true;
      else 
	deletion_list += this_strategy;
    }
  } while (!abort && scanner.GoToNext());
  
  if (!abort) {
    gList<Strategy *> actual_deletions;
    for (int i = 1; !abort && i <= deletion_list.Length(); i++) {
      actual_deletions += deletion_list[i];
      sact->RemoveStrategy(deletion_list[i]); 
    } 

    if (!abort && deletion_list.Length() > 0)
      PossibleNashSubsupportsRECURSIVE(s,sact,c,list,status);
    
    for (int i = 1; i <= actual_deletions.Length(); i++)
      sact->AddStrategy(actual_deletions[i]);
  }
  if (!abort && no_deletions) {
    (*list) += *sact;
    
    gbtStrategyIterator c_copy(*c);
    do {
      Strategy *str_ptr = (Strategy *)c_copy.GetStrategy();
      if (sact->Contains(str_ptr) &&
	  sact->NumStrats(str_ptr->GetPlayer()) > 1 ) {
	sact->RemoveStrategy(str_ptr); 
	PossibleNashSubsupportsRECURSIVE(s,sact,&c_copy,list,status);
	sact->AddStrategy(str_ptr);
      }
    } while (c_copy.GoToNext()) ;
  }
}

gList<const gbtNfgSupport> SortSupportsBySize(gList<const gbtNfgSupport> &list) 
{
  gArray<int> sizes(list.Length());
  for (int i = 1; i <= list.Length(); i++) {
    sizes[i] = list[i].ProfileLength();
  }

  gArray<int> listproxy(list.Length());
  for (int i = 1; i <= list.Length(); i++)
    listproxy[i] = i;

  int maxsize(0);
  for (int i = 1; i <= list.Length(); i++)
    if (sizes[i] > maxsize)
      maxsize = sizes[i];

  int cursor(1);

  for (int j = 0; j < maxsize; j++) {
    int scanner(list.Length());
    while (cursor < scanner)
      if (sizes[scanner] != j)
	scanner--;
      else {
	while (sizes[cursor] == j)
	  cursor++;
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

  gList<const gbtNfgSupport> answer;
  for (int i = 1; i <= list.Length(); i++)
    answer += list[listproxy[i]];

  return answer;
}
  
gList<const gbtNfgSupport> PossibleNashSubsupports(const gbtNfgSupport &S,
					       gStatus &status)
{
  gList<const gbtNfgSupport> answer;
  gbtNfgSupport sact(S);
  gbtStrategyIterator cursor(S);
  status.SetProgress(0);
  PossibleNashSubsupportsRECURSIVE(&S,&sact,&cursor,&answer,status);
  status.SetProgress(.5);

  // At this point answer has all consistent subsupports without
  // any strong dominations.  We now edit the list, removing all
  // subsupports that exhibit weak dominations, and we also eliminate
  // subsupports exhibiting domination by currently inactive strategies.

  for (int i = answer.Length(); i >= 1; i--) {
    status.SetProgress((2.0-((double)i/(double)answer.Length()))/2.0);
    status.Get();
    gbtNfgSupport current(answer[i]);
    gbtStrategyIterator crsr(S);
    bool remove = false;
    do {
      Strategy *strat = (Strategy *)crsr.GetStrategy();
      if (current.Contains(strat)) 
	for (int j = 1; j <= strat->GetPlayer().NumStrategies(); j++) {
	  Strategy *other_strat = strat->GetPlayer().GetStrategy(j);
	  if (other_strat != strat)
	    if (current.Contains(other_strat)) {
	      if (current.Dominates(other_strat,strat,false)) 
		remove = true;
	    }
	    else { 
	      current.AddStrategy(other_strat);
	      if (current.Dominates(other_strat,strat,false)) {
		remove = true;
	      }
	      current.RemoveStrategy(other_strat);
	    }
      }
    } while (crsr.GoToNext() && !remove);
    if (remove)
      answer.Remove(i);
  }
    
  return SortSupportsBySize(answer);
  return answer;
}


