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

#include "nfdom.h"

bool gbtNfgSupport::Dominates(gbtNfgStrategy s, gbtNfgStrategy t,
			      bool strong) const
{
  const Nfg &n = Game();

  NfgContIter A(*this), B(*this);

  A.Freeze(s.GetPlayer());
  A.Set(s);
  B.Freeze(s.GetPlayer());
  B.Set(t);  

  if (strong)  {
    do  {
      gNumber ap = ((!A.GetOutcome().IsNull()) ? 
		    n.Payoff(A.GetOutcome(), s.GetPlayer()) : gNumber(0));
      gNumber bp = ((!B.GetOutcome().IsNull()) ? 
		    n.Payoff(B.GetOutcome(), s.GetPlayer()) : gNumber(0));

      if (ap <= bp)  {
	return false;
      }
      A.NextContingency();
    } while (B.NextContingency());
	
    return true;
  }

  bool equal = true;
  
  do   {
    gNumber ap = ((!A.GetOutcome().IsNull()) ? 
		  n.Payoff(A.GetOutcome(), s.GetPlayer()) : gNumber(0));
    gNumber bp = ((!B.GetOutcome().IsNull()) ? 
		  n.Payoff(B.GetOutcome(), s.GetPlayer()) : gNumber(0));

    if (ap < bp)   { 
      return false;
    }
    else if (ap > bp) 
      equal = false;
    A.NextContingency();
  } while (B.NextContingency());

  return (!equal);
}

bool gbtNfgSupport::IsDominated(gbtNfgStrategy s, bool strong) const
{
  for (int i = 1; i <= NumStrats(s.GetPlayer().GetId()); i++) {
    if (i != s.GetId()) {
      if (Dominates(GetStrategy(s.GetPlayer().GetId(), i), s, strong)) {
	return true;
      }
    }
  }
  return false;
}

bool gbtNfgSupport::Undominated(gbtNfgSupport &newS, int pl, bool strong,
				gOutput &tracefile, gStatus &status) const
{
  gArray<int> set(NumStrats(pl));
  int i;
  for (i = 1; i <= set.Length(); i++)
    set[i] = i;

  int min, dis;
  double d1,d2;
  d1 = (double)(pl-1) / (double) Game().NumPlayers();
  d2 = (double)pl / (double) Game().NumPlayers();
  for (min = 0, dis = NumStrats(pl) - 1; min <= dis; )  {
    status.Get();
    int pp;
    double s1 = (double)min/(double)(dis+1);
    status.SetProgress((1.0-s1)*d1 + s1*d2);
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
	  tracefile << GetStrategy(pl, set[dis+1]).GetId() << " dominated by " << GetStrategy(pl, set[min+1]).GetId() << '\n';
	  dis--;
	}
	else if (Dominates(GetStrategy(pl, set[dis+1]),
			   GetStrategy(pl, set[min+1]), strong)) { 
	  tracefile << GetStrategy(pl, set[min+1]).GetId() << " dominated by " << GetStrategy(pl, set[dis+1]).GetId() << '\n';
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
    for (i = min + 1; i <= NumStrats(pl); i++) {
      newS.RemoveStrategy(GetStrategy(pl, set[i]));
    }
    
    return true;
  }
  else
    return false;
}

gbtNfgSupport gbtNfgSupport::Undominated(bool strong,
					 const gArray<int> &players,
					 gOutput &tracefile, 
					 gStatus &status) const
{
  gbtNfgSupport newS(*this);
  
  for (int i = 1; i <= players.Length(); i++)   {
    status.Get();
    status.SetProgress(0, (gText("Eliminating strategies for player ") +
			   ToText(players[i])));
    int pl = players[i];
    tracefile << "Dominated strategies for player " << pl << ":\n";
    Undominated(newS, pl, strong, tracefile, status);
  }

  return newS;
}



