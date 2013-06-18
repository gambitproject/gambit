//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
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

// We build a series of functions of increasing complexity.  The
// final one, which is our goal, is the undominated support function.
// We begin by simply enumerating all subsupports.

void AllSubsupportsRECURSIVE(const Gambit::StrategySupport &s,
			     Gambit::StrategySupport *sact,
			     StrategyCursorForSupport *c,
			     Gambit::List<Gambit::StrategySupport> &p_list)
{ 
  p_list.Append(*sact);

  StrategyCursorForSupport c_copy(*c);

  do {
    Gambit::GameStrategy str_ptr = c_copy.GetStrategy();
    if (sact->Contains(str_ptr)) {
      sact->RemoveStrategy(str_ptr);
      AllSubsupportsRECURSIVE(s,sact,&c_copy,p_list);
      sact->AddStrategy(str_ptr);
    }
  } while (c_copy.GoToNext()) ;
}

Gambit::List<Gambit::StrategySupport> AllSubsupports(const Gambit::StrategySupport &S)
{
  Gambit::List<Gambit::StrategySupport> answer;
  Gambit::StrategySupport SAct(S);
  StrategyCursorForSupport cursor(S);

  AllSubsupportsRECURSIVE(S, &SAct, &cursor, answer);

  return answer;
}

// In the next two routines we only output subsupports that
// have at least one active strategy for each agent.

void AllValidSubsupportsRECURSIVE(const Gambit::StrategySupport &s,
				  Gambit::StrategySupport *sact,
				  StrategyCursorForSupport *c,
				  Gambit::List<Gambit::StrategySupport> &p_list)
{ 
  p_list.Append(*sact);

  StrategyCursorForSupport c_copy(*c);

  do {
    if (sact->NumStrategies(c_copy.PlayerIndex()) > 1) {
      Gambit::GameStrategy str_ptr = c_copy.GetStrategy();
      sact->RemoveStrategy(str_ptr); 
      AllValidSubsupportsRECURSIVE(s,sact,&c_copy,p_list);
      sact->AddStrategy(str_ptr);
    }
  } while (c_copy.GoToNext()) ;
}

Gambit::List<Gambit::StrategySupport> AllValidSubsupports(const Gambit::StrategySupport &S)
{
  Gambit::List<Gambit::StrategySupport> answer;
  Gambit::StrategySupport SAct(S);
  StrategyCursorForSupport cursor(S);

  AllValidSubsupportsRECURSIVE(S, &SAct, &cursor, answer);

  return answer;
}

void AllUndominatedSubsupportsRECURSIVE(const Gambit::StrategySupport &s,
					Gambit::StrategySupport *sact,
					StrategyCursorForSupport *c,
					const bool strong,
					const bool conditional,
					Gambit::List<Gambit::StrategySupport> &p_list)
{ 
  bool abort = false;
  bool no_deletions = true;


  Gambit::List<Gambit::GameStrategy> deletion_list;
  StrategyCursorForSupport scanner(s);

  // First we collect all the strategies that can be deleted.
  do {
    Gambit::GameStrategy this_strategy = scanner.GetStrategy();
    bool delete_this_strategy = false;
    if (sact->Contains(this_strategy)) 
      if (sact->IsDominated(this_strategy,strong) ) 
	delete_this_strategy = true;
	
    if (delete_this_strategy) {
      no_deletions = false;
      if (c->IsSubsequentTo(this_strategy)) 
	abort = true;
      else 
	deletion_list.Append(this_strategy);
    }
  } while (!abort && scanner.GoToNext());

  // Now we delete them, recurse, then restore
  if (!abort && !no_deletions) {
    Gambit::List<Gambit::GameStrategy> actual_deletions;
    for (int i = 1; !abort && i <= deletion_list.Length(); i++) {
      actual_deletions.Append(deletion_list[i]);

      sact->RemoveStrategy(deletion_list[i]); 
    }

    if (!abort)
      AllUndominatedSubsupportsRECURSIVE(s,
					 sact,
					 c,
					 strong,
					 conditional,
					 p_list);
    
    for (int i = 1; i <= actual_deletions.Length(); i++)
      sact->AddStrategy(actual_deletions[i]);
  }

  // If there are no deletions, we ask if it is consistent, then recurse.
  if (!abort && no_deletions) {
    p_list.Append(*sact);
    
    StrategyCursorForSupport c_copy(*c);
    
    do {
      if (sact->Contains(c_copy.GetStrategy()) &&
	  sact->NumStrategies(c_copy.PlayerIndex()) > 1 ) {

	Gambit::GameStrategy str_ptr = c_copy.GetStrategy();
	sact->RemoveStrategy(str_ptr); 
	AllUndominatedSubsupportsRECURSIVE(s,
					   sact,
					   &c_copy,
					   strong,
					   conditional,
					   p_list);
	sact->AddStrategy(str_ptr);
	
      }
    } while (c_copy.GoToNext()) ;
  }
}
  
Gambit::List<Gambit::StrategySupport> AllUndominatedSubsupports(const Gambit::StrategySupport &S,
					       bool strong,
					       bool conditional)
{
  Gambit::List<Gambit::StrategySupport> answer;
  Gambit::StrategySupport sact(S);
  StrategyCursorForSupport cursor(S);

  AllUndominatedSubsupportsRECURSIVE(S,
				     &sact,
				     &cursor,
				     strong,
				     conditional,
				     answer);

  return answer;
}


void PossibleNashSubsupportsRECURSIVE(const Gambit::StrategySupport &s,
				      Gambit::StrategySupport *sact,
				      StrategyCursorForSupport *c,
				      Gambit::List<Gambit::StrategySupport> &p_list)
{ 
  bool abort = false;
  bool no_deletions = true;

  Gambit::List<Gambit::GameStrategy> deletion_list;
  StrategyCursorForSupport scanner(s);

  do {
    Gambit::GameStrategy this_strategy = scanner.GetStrategy();
    bool delete_this_strategy = false;
    if (sact->Contains(this_strategy)) 
      if (sact->IsDominated(this_strategy,true) ) {
	delete_this_strategy = true;
      }
    if (delete_this_strategy) {
      no_deletions = false;
      if (c->IsSubsequentTo(this_strategy)) 
	abort = true;
      else 
	deletion_list.Append(this_strategy);
    }
  } while (!abort && scanner.GoToNext());
  
  if (!abort) {
    Gambit::List<Gambit::GameStrategy> actual_deletions;
    for (int i = 1; !abort && i <= deletion_list.Length(); i++) {
      actual_deletions.Append(deletion_list[i]);
      sact->RemoveStrategy(deletion_list[i]); 
    } 

    if (!abort && deletion_list.Length() > 0)
      PossibleNashSubsupportsRECURSIVE(s,sact,c,p_list);
    
    for (int i = 1; i <= actual_deletions.Length(); i++)
      sact->AddStrategy(actual_deletions[i]);
  }
  if (!abort && no_deletions) {
    p_list.Append(*sact);
    
    StrategyCursorForSupport c_copy(*c);
    do {
      Gambit::GameStrategy str_ptr = c_copy.GetStrategy();
      if (sact->Contains(str_ptr) &&
	  sact->NumStrategies(str_ptr->GetPlayer()->GetNumber()) > 1 ) {
	sact->RemoveStrategy(str_ptr); 
	PossibleNashSubsupportsRECURSIVE(s,sact,&c_copy,p_list);
	sact->AddStrategy(str_ptr);
      }
    } while (c_copy.GoToNext()) ;
  }
}

Gambit::List<Gambit::StrategySupport> SortSupportsBySize(Gambit::List<Gambit::StrategySupport> &p_list) 
{
  Gambit::Array<int> sizes(p_list.Length());
  for (int i = 1; i <= p_list.Length(); i++)
    sizes[i] = p_list[i].MixedProfileLength();

  Gambit::Array<int> listproxy(p_list.Length());
  for (int i = 1; i <= p_list.Length(); i++)
    listproxy[i] = i;

  int maxsize(0);
  for (int i = 1; i <= p_list.Length(); i++)
    if (sizes[i] > maxsize)
      maxsize = sizes[i];

  int cursor(1);

  for (int j = 0; j < maxsize; j++) {
    int scanner(p_list.Length());
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

  Gambit::List<Gambit::StrategySupport> answer;
  for (int i = 1; i <= p_list.Length(); i++)
    answer.Append(p_list[listproxy[i]]);

  return answer;
}
  
Gambit::List<Gambit::StrategySupport> PossibleNashSubsupports(const Gambit::StrategySupport &S)
{
  Gambit::List<Gambit::StrategySupport> answer;
  Gambit::StrategySupport sact(S);
  StrategyCursorForSupport cursor(S);
  PossibleNashSubsupportsRECURSIVE(S, &sact, &cursor, answer);

  // At this point answer has all consistent subsupports without
  // any strong dominations.  We now edit the list, removing all
  // subsupports that exhibit weak dominations, and we also eliminate
  // subsupports exhibiting domination by currently inactive strategies.

  for (int i = answer.Length(); i >= 1; i--) {
    Gambit::StrategySupport current(answer[i]);
    StrategyCursorForSupport crsr(S);
    bool remove = false;
    do {
      Gambit::GameStrategy strat = crsr.GetStrategy();
      if (current.Contains(strat))  {
	for (int j = 1; j <= strat->GetPlayer()->NumStrategies(); j++) {
	  Gambit::GameStrategy other_strat = strat->GetPlayer()->GetStrategy(j);
	  if (other_strat != strat) {
	    if (current.Contains(other_strat)) {
	      if (current.Dominates(other_strat,strat,false))  {
		remove = true;
	      }
	    }
	    else { 
	      current.AddStrategy(other_strat);
	      if (current.Dominates(other_strat,strat,false)) {
		remove = true;
	      }
	      current.RemoveStrategy(other_strat);
	    }
	  }
	}
      }
    } while (crsr.GoToNext() && !remove);
    if (remove)
      answer.Remove(i);
  }
    
  return SortSupportsBySize(answer);
  return answer;
}


//----------------------------------------------------
//                StrategyCursorForSupport
// ---------------------------------------------------

StrategyCursorForSupport::StrategyCursorForSupport(const Gambit::StrategySupport &S)
  : support(&S), pl(1), strat(1)
{}

StrategyCursorForSupport::StrategyCursorForSupport(
                  const StrategyCursorForSupport &sc)
  : support(sc.support), pl(sc.pl), strat(sc.strat)
{}

StrategyCursorForSupport::~StrategyCursorForSupport()
{}

StrategyCursorForSupport& 
StrategyCursorForSupport::operator=(const StrategyCursorForSupport &rhs)
{
  if (this != &rhs) {
    support = rhs.support;
    pl = rhs.pl;
    strat = rhs.strat;
  }
  return *this;
}

bool 
StrategyCursorForSupport::operator==(const StrategyCursorForSupport &rhs) const
{
  if (support != rhs.support ||
      pl      != rhs.pl      ||
      strat   != rhs.strat)
    return false;
  return true;
}

bool 
StrategyCursorForSupport::operator!=(const StrategyCursorForSupport &rhs) const
{
 return (!(*this==rhs));
}

bool
StrategyCursorForSupport::GoToNext()
{
  if (strat != support->NumStrategies(pl))
    { strat++; return true; }
  else if (pl != support->GetGame()->NumPlayers())
    { pl++; strat = 1; return true; }
  else return false;
}

Gambit::GameStrategy StrategyCursorForSupport::GetStrategy() const
{
  return support->GetStrategy(pl, strat);
}

int StrategyCursorForSupport::StrategyIndex() const
{
  return strat;
}

Gambit::GamePlayer StrategyCursorForSupport::GetPlayer() const
{
  return support->GetGame()->GetPlayer(pl);
}

int StrategyCursorForSupport::PlayerIndex() const
{
  return pl;
}

bool 
StrategyCursorForSupport::IsLast() const
{
  if (pl == support->GetGame()->NumPlayers())
    if (strat == support->NumStrategies(pl))
      return true;
  return false;
}

bool 
StrategyCursorForSupport::IsSubsequentTo(const Gambit::GameStrategy &s) const
{
  if (pl > s->GetPlayer()->GetNumber())
    return true; 
  else if (pl < s->GetPlayer()->GetNumber())
    return false;
  else
    if (strat > s->GetNumber())
      return true;
    else
      return false;
}
