//#
//# FILE: lemke.cc -- Lemke-Howson module
//#
//# $Id$
//#

#include "gambitio.h"
#include "normal.h"
#include "nfrep.h"
#include "gtableau.h"
#include "rational.h"

int NormalForm::Lemke(int dup_strat)
{
  int i, j;
  StrategyProfile s(2);

  if (NumPlayers() != 2)   return 0;
  int n1 = strategies[1]->NumStrats();
  int n2 = strategies[2]->NumStrats();
  int n = n1+n2;

  for (i = 1; i <= strategies.Length(); i++)
    s.SetStrategy(strategies[i]->GetStrategy(1));

  switch (array->Type())   {
    case nfDOUBLE:   {
      double min = 0.0, x;
      gTableau<double> T(n);
      NFRep<double> *N = (NFRep<double> *) array;

//      N->WriteNfgFile(gout);

      for (i = 1; i <= n1; i++)
 	for (j = 1; j <= n2; j++)  {
	  s.SetStrategy(strategies[1]->GetStrategy(i));
	  s.SetStrategy(strategies[2]->GetStrategy(j));
	  x = (*N)(s,1);
//	  gout << i << ' ' << j << ' ' << s << " -> " << x << '\n';
	  if (x < min)  min = x;
	  x = (*N)(s,2);
//	  gout << i << ' ' << j << ' ' << s << " -> " << x << '\n';
	  if (x < min)  min = x;
	}

      for (i = 1; i <= n1; i++) 
	for (j = 1; j <= n1; j++)
	  T.Entry(i,j) = 0.0;
      
      for (i = n1 + 1; i <= n; i++)  
	for (j = n1 + 1; j <= n; j++) 
	  T.Entry(i,j) = 0.0;

      for (i = 1; i <= n1; i++)  
	for (j = 1; j <= n2; j++)  {
	  s.SetStrategy(strategies[1]->GetStrategy(i));
	  s.SetStrategy(strategies[2]->GetStrategy(j));
	  T.Entry(i, n1 + j) = (*N)(s,1) - min;
	  T.Entry(n1 + j, i) = (*N)(s,2) - min;
	}

      for (i = 1; i <= n; T.Entry(i++, 0) = -1.0);
      for (i = 1; i <= n; T.Entry(i++, n + 1) = 0.0);
      T.Lemke((dup_strat <= 0 || dup_strat > n) ? 0 : dup_strat); 
      return 1;
    }

    case nfRATIONAL:   {
      Rational min = 0, x;
      gTableau<Rational> T(n);
      NFRep<Rational> *N = (NFRep<Rational> *) array;

      for (i = 1; i <= n1; i++)
	for (j = 1; j <= n2; j++)  {
	  s.SetStrategy(strategies[1]->GetStrategy(i));
	  s.SetStrategy(strategies[2]->GetStrategy(j));
	  x = (*N)(s,1);
	  if (x < min)  min = x;
	  x = (*N)(s,2);
	  if (x < min)  min = x;
	}

      for (i = 1; i <= n1; i++) 
	for (j = 1; j <= n1; j++)
	  T.Entry(i,j) = 0.0;
      
      for (i = n1 + 1; i <= n; i++)  
	for (j = n1 + 1; j <= n; j++) 
	  T.Entry(i,j) = 0.0;

      for (i = 1; i <= n1; i++)  
	for (j = 1; j <= n2; j++)  {
	  s.SetStrategy(strategies[1]->GetStrategy(i));
	  s.SetStrategy(strategies[2]->GetStrategy(j));
	  T.Entry(i, n1 + j) = (*N)(s,1) - min;
	  T.Entry(n1 + j, i) = (*N)(s,2) - min;
	}

      for (i = 1; i <= n; T.Entry(i++, 0) = -1.0);
      for (i = 1; i <= n; T.Entry(i++, n + 1) = 0.0);
      T.Lemke((dup_strat <= 0 || dup_strat > n) ? 0 : dup_strat); 
      return 1;
    }
  }
}




