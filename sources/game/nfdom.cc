//
// FILE: nfdom.cc -- Implementation of dominance functions
//
// $Id$
//

#include "nfdom.h"

bool NFSupport::Dominates(Strategy *s, Strategy *t, bool strong) const
{
  const Nfg &n = Game();

  NfgContIter A(*this), B(*this);

  A.Freeze(s->Player());
  A.Set(s);
  B.Freeze(s->Player());
  B.Set(t);  

  if (strong)  {
    do  {
      gNumber ap = ((A.GetOutcome()) ? 
		    n.Payoff(A.GetOutcome(), s->Player()) : gNumber(0));
      gNumber bp = ((B.GetOutcome()) ? 
		    n.Payoff(B.GetOutcome(), s->Player()) : gNumber(0));

      if (ap <= bp)  {
	return false;
      }
      A.NextContingency();
    } while (B.NextContingency());
	
    return true;
  }

  bool equal = true;
  
  do   {
    gNumber ap = ((A.GetOutcome()) ? 
		  n.Payoff(A.GetOutcome(), s->Player()) : gNumber(0));
    gNumber bp = ((B.GetOutcome()) ? 
		  n.Payoff(B.GetOutcome(), s->Player()) : gNumber(0));

    if (ap < bp)   { 
      return false;
    }
    else if (ap > bp) 
      equal = false;
    A.NextContingency();
  } while (B.NextContingency());

  return (!equal);
}


bool NFSupport::IsDominated(Strategy *s, bool strong) const
{
  for (int i = 1; i <= NumStrats(s->Player()->GetNumber()); i++) {
    if (i != s->Number()) {
      if (Dominates(Strategies(s->Player()->GetNumber())[i], s, strong)) {
	return true;
      }
    }
  }
  return false;
}

bool NFSupport::Undominated(NFSupport &newS, int pl, bool strong,
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
	 pp < min && !Dominates(Strategies(pl)[set[pp+1]],
				Strategies(pl)[set[dis+1]], strong); 
	 pp++);
    if (pp < min)
      dis--;
    else  {
      int foo = set[dis+1];
      set[dis+1] = set[min+1];
      set[min+1] = foo;

      for (int inc = min + 1; inc <= dis; )  {
	if (Dominates(Strategies(pl)[set[min+1]],
		      Strategies(pl)[set[dis+1]], strong)) { 
	  tracefile << Strategies(pl)[set[dis+1]]->Number() << " dominated by " << Strategies(pl)[set[min+1]]->Number() << '\n';
	  dis--;
	}
	else if (Dominates(Strategies(pl)[set[dis+1]],
			   Strategies(pl)[set[min+1]], strong)) { 
	  tracefile << Strategies(pl)[set[min+1]]->Number() << " dominated by " << Strategies(pl)[set[dis+1]]->Number() << '\n';
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
      newS.RemoveStrategy(Strategies(pl)[set[i]]);
    
    return true;
  }
  else
    return false;
}

NFSupport *NFSupport::Undominated(bool strong, const gArray<int> &players,
				 gOutput &tracefile, gStatus &status) const
{
  NFSupport *newS = new NFSupport(*this);
  bool any = false;
  
  for (int i = 1; i <= players.Length(); i++)   {
    status.Get();
    status.SetProgress(0, (gText("Eliminating strategies for player ") +
			   ToText(players[i])));
    int pl = players[i];
    tracefile << "Dominated strategies for player " << pl << ":\n";
    any |= Undominated(*newS, pl, strong, tracefile, status);
  }

  if (!any)  {
    delete newS;
    return 0;
  }
  return newS;
}



