//
// FILE: nfdom.cc -- Implementation of dominance functions of nfdom.h
//
// $Id$
//

#include "nfdom.h"

bool Dominates(const Nfg &,const NFSupport &S, int pl, int a, int b, 
	       const gRectArray<gNumber> *paytable, bool strong)
{
  //DEBUG - In order to cross check with pointer version of Dominates(..)
  //  Strategy *s = S.GetStrategy(pl,a);
  //  Strategy *t = S.GetStrategy(pl,b);
  //  bool answer = Dominates(S,s,t,strong);

  NfgContIter A(S), B(S);

  A.Freeze(pl);
  A.Set(pl, a);
  B.Freeze(pl);
  B.Set(pl, b);

  if (strong) {
    do {
      gNumber ap = (*paytable)(A.GetOutcome()->GetNumber(), pl);
      gNumber bp = (*paytable)(B.GetOutcome()->GetNumber(), pl);
      if (ap <= bp)  return false;
      A.NextContingency();
    } while (B.NextContingency());

    return true;
  }

  bool equal = true;

  do {
    gNumber ap = (*paytable)(A.GetOutcome()->GetNumber(), pl);
    gNumber bp = (*paytable)(B.GetOutcome()->GetNumber(), pl);
    if (ap < bp)   return false;
    else if (ap > bp)  equal = false;
    A.NextContingency();
  } while (B.NextContingency());

  return (!equal);
}

bool Dominates(const Nfg &,const NFSupport &S, int pl, int a, int b, 
	       const bool strong)
{
  gRectArray<gNumber> *paytable;
  paytable = new gRectArray<gNumber>(S.Game().NumOutcomes(), 
				     S.Game().NumPlayers());
  InitializePaytable(S.Game(),paytable);
  bool answer = Dominates(S.Game(),S,pl,a,b,paytable,strong);
  delete paytable;
  return answer;
}

bool Dominates(const NFSupport &S, Strategy *s, Strategy *t, bool strong,
	       const gStatus &status)
{
  return Dominates(S,s,t,strong);
}

bool Dominates(const NFSupport &S, Strategy *s, Strategy *t, bool strong)
{
  //DEBUG
  bool answer = Dominates(S.Game(),S,s->Player()->GetNumber(), 
			  S.GetNumber(s), S.GetNumber(t),strong);

  const Nfg* n = S.GamePtr();

  NfgContIter A(S), B(S);

  A.Freeze(s->Player());
  A.Set(s);
  B.Freeze(s->Player());
  B.Set(t);  

  if (strong)  {
    do  {

      //DEBUG
      if (!A.GetOutcome() || !B.GetOutcome())
	{ gout << "Weirdness!!\n"; exit(0); }

      gNumber ap = (A.GetOutcome()) ? 
      n->Payoff(A.GetOutcome(), s->Player()) : gNumber(0);
      gNumber bp = (B.GetOutcome()) ? 
      n->Payoff(B.GetOutcome(), s->Player()) : gNumber(0);

      if (ap <= bp)  { if (answer) { gout << "E1.\n"; exit(0);}
      return false;
      }
      A.NextContingency();
    } while (B.NextContingency());
	
    if (!answer) {gout << "E2.\n"; exit(0);}
    return true;
  }

  bool equal = true;
  
  do   {
    gNumber ap = (A.GetOutcome()) ? 
    n->Payoff(A.GetOutcome(), s->Player()) : gNumber(0);
    gNumber bp = (B.GetOutcome()) ? 
    n->Payoff(B.GetOutcome(), s->Player()) : gNumber(0);

    if (ap < bp)   {if (answer) {gout << "E3.\n"; exit(0);}
    return false;
    }
    else if (ap > bp)  equal = false;
    A.NextContingency();
  } while (B.NextContingency());

  if (answer == equal) {gout << "E4.\n"; exit(0);}
  return (!equal);
}


bool IsDominated(const NFSupport &S, Strategy *s, bool strong, 
		 const gStatus &status)
{
  for (int i = 1; i <= S.NumStrats(s->Player()->GetNumber()); i++) {
    if (i != s->Number()) 
      if (Dominates(S,S.Strategies(s->Player()->GetNumber())[i],s,
		    strong,status)) {
	return true;
      }
  }
  return false;
}

bool ComputeDominated(const Nfg &N, const NFSupport &S, NFSupport &newS,
		      int pl, const gRectArray<gNumber> *paytable, bool strong,
		      gOutput &tracefile, gStatus &status)
{
  gArray<int> set(S.NumStrats(pl));
  int i;
  for (i = 1; i <= set.Length(); i++)
    set[i] = i;

  int min, dis;
  double d1,d2;
  d1 = (double)(pl-1)/(double)S.Game().NumPlayers();
  d2 = (double)pl/(double)S.Game().NumPlayers();
  for (min = 0, dis = S.NumStrats(pl) - 1; min <= dis; )  {
    status.Get();
    int pp;
    double s1 = (double)min/(double)(dis+1);
    status.SetProgress((1.0-s1)*d1 + s1*d2);
    for (pp = 0;
	 pp < min && !Dominates(N, S, pl, set[pp+1], set[dis+1], 
				paytable, strong);
	 pp++);
    if (pp < min)
      dis--;
    else  {
      int foo = set[dis+1];
      set[dis+1] = set[min+1];
      set[min+1] = foo;

      for (int inc = min + 1; inc <= dis; )  {
	if (Dominates(N, S, pl, set[min+1], set[dis+1], 
		      paytable, strong))  {
	  tracefile << S.Strategies(pl)[set[dis+1]]->Number() << " dominated by " << S.Strategies(pl)[set[min+1]]->Number() << '\n';
	  dis--;
	}
	else if (Dominates(N, S, pl, set[dis+1], set[min+1], 
			   paytable, strong))  {
	  tracefile << S.Strategies(pl)[set[min+1]]->Number() << " dominated by " << S.Strategies(pl)[set[dis+1]]->Number() << '\n';
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
  
  
  if (min + 1 <= S.NumStrats(pl))   {
    for (i = min + 1; i <= S.NumStrats(pl); i++)
      newS.RemoveStrategy(S.Strategies(pl)[set[i]]);
    
    return true;
  }
  else
    return false;
}

void InitializePaytable(const Nfg &N, gRectArray<gNumber> *paytable) 
{
  for (int outc = 1; outc <= N.NumOutcomes(); outc++)
    for (int pl = 1; pl <= N.NumPlayers(); pl++)
      (*paytable)(outc, pl) = N.Payoff(N.Outcomes()[outc], pl);
}


NFSupport *ComputeDominated(const Nfg &N, NFSupport &S, bool strong,
			    const gArray<int> &players,
			    gOutput &tracefile, gStatus &status)
{
  NFSupport *newS = new NFSupport(S);
  bool any = false;
  
  gRectArray<gNumber> *paytable;
  paytable = new gRectArray<gNumber>(N.NumOutcomes(), N.NumPlayers());
  InitializePaytable(N,paytable);

  for (int i = 1; i <= players.Length(); i++)   {
    status.Get();
    int pl = players[i];
    tracefile << "Dominated strategies for player " << pl << ":\n";
    any |= ComputeDominated(N, S, *newS, pl, paytable,
			    strong, tracefile, status);
// status.SetProgress((double)i/players.Length());
  }

  delete paytable;

  if (!any)  {
    delete newS;
    return 0;
  }
  return newS;
}



