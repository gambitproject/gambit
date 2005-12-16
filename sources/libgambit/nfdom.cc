//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute dominated strategies on normal forms
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

#include <iostream>
#include "libgambit.h"

bool gbtNfgSupport::Dominates(gbtNfgStrategy s, gbtNfgStrategy t, 
			      bool strong) const
{
  gbtNfgGame *n = GetGame();

  gbtNfgContingencyIterator A(*this,
			      s->GetPlayer()->GetNumber(), GetIndex(s));
  gbtNfgContingencyIterator B(*this,
			      t->GetPlayer()->GetNumber(), GetIndex(t));

  if (strong)  {
    do  {
      gbtRational ap = ((A.GetOutcome()) ? 
		      A.GetOutcome()->GetPayoff(s->GetPlayer()->GetNumber()) : gbtRational(0));
      gbtRational bp = ((B.GetOutcome()) ? 
		      B.GetOutcome()->GetPayoff(s->GetPlayer()->GetNumber()) : gbtRational(0));

      if (ap <= bp)  {
	return false;
      }
      A.NextContingency();
    } while (B.NextContingency());
	
    return true;
  }

  bool equal = true;
  
  do   {
    gbtRational ap = ((A.GetOutcome()) ? 
		    A.GetOutcome()->GetPayoff(s->GetPlayer()->GetNumber()) : gbtRational(0));
    gbtRational bp = ((B.GetOutcome()) ? 
		    B.GetOutcome()->GetPayoff(s->GetPlayer()->GetNumber()) : gbtRational(0));

    if (ap < bp) { 
      return false;
    }
    else if (ap > bp) {
      equal = false;
    }
    A.NextContingency();
  } while (B.NextContingency());

  return (!equal);
}


bool gbtNfgSupport::IsDominated(gbtNfgStrategy s, bool strong) const
{
  for (int i = 1; i <= NumStrats(s->GetPlayer()->GetNumber()); i++) {
    if (GetStrategy(s->GetPlayer()->GetNumber(), i) != s &&
	Dominates(GetStrategy(s->GetPlayer()->GetNumber(), i), s, strong)) {
      return true;
    }
  }
  return false;
}

bool gbtNfgSupport::Undominated(gbtNfgSupport &newS, int pl, bool strong,
			    std::ostream &tracefile) const
{
  gbtArray<int> set(NumStrats(pl));
  int i;
  for (i = 1; i <= set.Length(); i++)
    set[i] = i;

  int min, dis;
  double d1,d2;
  d1 = (double)(pl-1) / (double) GetGame()->NumPlayers();
  d2 = (double)pl / (double) GetGame()->NumPlayers();
  for (min = 0, dis = NumStrats(pl) - 1; min <= dis; )  {
    int pp;
    double s1 = (double)min/(double)(dis+1);
    for (pp = 0;
	 pp < min && !Dominates(GetStrategy(pl, set[pp+1]),
				GetStrategy(pl, set[dis+1]), strong); 
	 pp++);
    if (pp < min)
      dis--;
    else  {
      int foo = set[dis+1];
      set[dis+1] = set[min+1];
      set[min+1] = foo;

      for (int inc = min + 1; inc <= dis; )  {
	if (Dominates(GetStrategy(pl, set[min+1]),
		      GetStrategy(pl, set[dis+1]), strong)) { 
	  tracefile << GetStrategy(pl, set[dis+1])->GetNumber() << " dominated by " << GetStrategy(pl, set[min+1])->GetNumber() << '\n';
	  dis--;
	}
	else if (Dominates(GetStrategy(pl, set[dis+1]),
			   GetStrategy(pl, set[min+1]), strong)) { 
	  tracefile << GetStrategy(pl, set[min+1])->GetNumber() << " dominated by " << GetStrategy(pl, set[dis+1])->GetNumber() << '\n';
	  foo = set[dis+1];
	  set[dis+1] = set[min+1];
	  set[min+1] = foo;
	  dis--;
	}
	else  {
	  foo = set[dis+1];
	  set[dis+1] = set[inc+1];
	  set[inc+1] = foo;
	  inc++;
	}
      }
      min++;
    }
  }
    
  if (min + 1 <= NumStrats(pl))   {
    for (i = min + 1; i <= NumStrats(pl); i++)
      newS.RemoveStrategy(GetStrategy(pl, set[i]));
    
    return true;
  }
  else
    return false;
}

gbtNfgSupport gbtNfgSupport::Undominated(bool strong, const gbtArray<int> &players,
				 std::ostream &tracefile) const
{
  gbtNfgSupport newS(*this);
  
  for (int i = 1; i <= players.Length(); i++)   {
    int pl = players[i];
    tracefile << "Dominated strategies for player " << pl << ":\n";
    Undominated(newS, pl, strong, tracefile);
  }

  return newS;
}



