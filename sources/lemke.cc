
#include "normal.h"
#include "gtableau.h"
#include "gambitio.h" 
//#include "rational.h"
#define Rational double

NormalForm::Lemke(int dup_strat)
{
  Rational min,x;
  int i,j;
  StrategyProfile s;

  if(NumberOfPlayers()>2)return 0;
  int n1 = strategies[1]->GetNumberOfStrategies();
  int n2= strategies[2]->GetNumberOfStrategies();
  int n=n1+n2;

  gTableau<Rational> T(n);

  s=FirstProfile(); 
  for(i=1;i<=n1;i++) { 
    for(j=1;j<=n2;j++) {
      s.SetStrategy(strategies[1]->GetStrategy(i));
      s.SetStrategy(strategies[2]->GetStrategy(j));
      x=(Rational)(*array)(s,1);
      if(x<min)min=x;
      x=(Rational)(*array)(s,2);
      if(x<min)min=x;
    }
  }

  for(i=1;i<=n1;i++) 
    for(j=1;j<=n1;j++) {T.Entry(i,j)=(Rational)0;}

  for(i=n1+1;i<=n;i++)  
    for(j=n1+1;j<=n;j++) {T.Entry(i,j)=(Rational)0;}


  for(i=1;i<=n1;i++) { 
    for(j=1;j<=n2;j++) {
      s.SetStrategy(strategies[1]->GetStrategy(i));
      s.SetStrategy(strategies[2]->GetStrategy(j));
      x=(Rational)(*array)(s,1)-min;
      T.Entry(i,n1+j)=x;
      x=(Rational)(*array)(s,2)-min;
      T.Entry(n1+j,i)=x;
    }
  }
  for(i=1;i<=n;i++)
    T.Entry(i,0)=(Rational)(-1);
  j=dup_strat; if(dup_strat<=0 || dup_strat > n)j=0;
  T.Lemke(j);
}

