//#
//# FILE: lemke.cc -- Lemke-Howson module
//#
//# $Id$
//#

#include "gambitio.h"
#include "normal.h"
#include "nfrep.h"
#include "gtableau.h"

// This sets what datatype Lemke works on, for the moment.  This will be
// superfluous when multiple datatype support comes into being.
#define LemkeType    Rational

int NormalForm::Lemke(int dup_strat)
{
  LemkeType min,x;
  int i,j;
  StrategyProfile s;

  if (NumPlayers() != 2)   return 0;
  int n1 = strategies[1]->NumStrats();
  int n2 = strategies[2]->NumStrats();
  int n=n1+n2;

  gTableau<LemkeType> T(n);

  for (i = 1; i <= strategies.Length(); i++)
    s.SetStrategy(strategies[i]->GetStrategy(1));

  for(i=1;i<=n1;i++) { 
    for(j=1;j<=n2;j++) {
      s.SetStrategy(strategies[1]->GetStrategy(i));
      s.SetStrategy(strategies[2]->GetStrategy(j));
      x=(LemkeType)((NFRep<double> &) *array)(s,1);
      if(x<min)min=x;
      x=(LemkeType)((NFRep<double> &) *array)(s,2);
      if(x<min)min=x;
    }
  }

  for(i=1;i<=n1;i++) 
    for(j=1;j<=n1;j++) {T.Entry(i,j)=(LemkeType)0;}

  for(i=n1+1;i<=n;i++)  
    for(j=n1+1;j<=n;j++) {T.Entry(i,j)=(LemkeType)0;}


  for(i=1;i<=n1;i++) { 
    for(j=1;j<=n2;j++) {
      s.SetStrategy(strategies[1]->GetStrategy(i));
      s.SetStrategy(strategies[2]->GetStrategy(j));
      x=(LemkeType)((NFRep<double> &) *array)(s,1)-min;
      T.Entry(i,n1+j)=x;
      x=(LemkeType)((NFRep<double> &) *array)(s,2)-min;
      T.Entry(n1+j,i)=x;
    }
  }
  for(i=1;i<=n;i++)
    T.Entry(i,0)=(LemkeType)(-1);
  for(i=1;i<=n;i++)
    T.Entry(i,n+1)=(LemkeType)(0);
  j=dup_strat; if(dup_strat<=0 || dup_strat > n)j=0;
  T.Lemke(j);
}



