//#
//# FILE: purenash.cc -- Find all pure strategy Nash equilibria
//#
//# @(#)purenash.cc	1.1 10/19/94
//#
#include "rational.h"
#include "gambitio.h"
#include "normal.h"
#include "normiter.h"
#include "purenash.h"
#include "integer.h"

/*****************************************************/
template <class T> int PureNashModule<T>::PureNash(void)
{
  int k;
  long index=1;
 
  long i;

  for(i=0;i<sindex[players];i++)
    {
      SingleNash(i);
    }
  gout << Number_Nash();
  /**********print payoffs**************/
 
  return 0;
}

/******************************************/

template <class T> PureNashModule<T>::PureNashModule(const NormalForm<T> &r)
  : rep(r)
{
  int v;
  long m;
  int i;
  long indeks=1;
  players=r.NumPlayers();

  for(i=1;i<=players;i++)
    {
      Num_strats.Append(v);
      Num_strats[i]=r.NumStrats(i);
      sindex.Append(m);
      sindex[i]=indeks*Num_strats[i];
      indeks*=Num_strats[i];
    }
}
/***********************************************/
template <class T> PureNashModule<T>::~PureNashModule()
{

}
/*******************index*****************************/
template <class T> int PureNashModule<T>::Reverse(long index, int player)
{
  if(player==1)
    {
      return(index-(index/sindex[1])*Num_strats[1]+1);
    }
  else
    {
      return((index-(index/sindex[player])*sindex[player])/sindex[player-1]+1); 
    }
}
/*****************************************************/

/****************************************************/
//Check if the given index is a nash equilibria
//if so add it index and SP to the gblock of the solutions
//
template <class T> void PureNashModule<T>::SingleNash(long i)
{
  
  int s,counter,flag=1;
  NormalIter<T> a(rep);
  T current_payoff;

  /**set normiter to the current strategy***/
  for(s=1;s<=players;s++)
    {

      a.Set(s,Reverse(i,s));
    }
/*checs if it a Nash eq*/
  for(s=1;s<=players && flag;s++)
    {
      current_payoff=a.Evaluate(s);

      counter=1;

      while(counter<=Num_strats[s])
	{
	  a.Next(s);
	  
	  if(a.Evaluate(s)<current_payoff)
	    {
	      /****add that index to the NON_NASH set***/
	    }
	  else{if(a.Evaluate(s)>current_payoff)
		 {flag=0;}
	     }
	  counter++;
	}
    }
  /*adding to the gblock of the solution*/
  if(flag)
    {
    /*****that index is a nash eq****/
      solution.Append(i);sol.Append(players);
      
      gout << "Nash Eq:\n";
      for(int h=1;h<=players;h++)
	{
	  sol[sol.Length()].SetStrategy(Strategy(h,Reverse(i,h)));
	  gout<<h<<a.Evaluate(h);
	}
      gout << "\n";
    }
}
	    
/**************************************************/
//
//initializes the PureNashModule
//
int NashSolver::Solve(void)
{
  PureNashModule<double> *M;
  PureNashModule<Rational> *N;
  
  switch (nf.Type())   {
    case DOUBLE:   {
     M=new  PureNashModule<double>((NormalForm<double> &) nf);
      M->PureNash();
      return 1;
     delete M;
   }
    
    case RATIONAL:  {
      N=new PureNashModule<Rational>((NormalForm<Rational> &) nf);
      N->PureNash();
      return 1;
      delete N;
    }

    default:
      return 0;
  }

}
















