//#
//# FILE: sftab.cc -- Sequence form tableau
//#
//# $Id$ 
//#

#include "sftab.h"
#include "extform.h"
#include "infoset.h"
#include "player.h"
#include "rational.h"


template <class T> int NumSequences(int j, const ExtForm<T> &E)
{
  gArray<Infoset *> isets;
  isets = E.PlayerList()[j]->InfosetList();
  int num = 1; 
  for(int i = isets.First();i<= isets.Last();i++)
    num+=(isets[i])->NumActions();
  return num;
}

template <class T> int NumInfosets(int j, const ExtForm<T> &E) 
{
  return E.PlayerList()[j]->InfosetList().Length();
}

//---------------------------------------------------------------------------
//                        LemkeHowson Tableau: member functions
//---------------------------------------------------------------------------
 
template <class T> SFTableau<T>::SFTableau(Tableau<T> &tab)
  : LTableau<T>(tab) 
{ }

template <class T> SFTableau<T>::SFTableau(const ExtForm<T> &E) 
  : LTableau<T>(1, NumSequences(1,E) + NumSequences(2,E) 
		 + 2*NumInfosets(1,E) + 2*NumInfosets(2,E), 
		 0, NumSequences(1,E) + NumSequences(2,E)  
		 + 2*NumInfosets(1,E) + 2*NumInfosets(2,E)) 
{ 
  int ns1, ns2, ni1, ni2;
  ns1=NumSequences(1,E);
  ns2=NumSequences(2,E);
  ni1=NumInfosets(1,E);
  ni2=NumInfosets(2,E);
  T prob = (T)1;
  FillTableau(E,E.RootNode(),prob);
  
/*
  for (i = 1; i <= n1; i++)   {
    for (int j = 1; j <= n2; j++)  {
      x = iter.Payoff(1);
      if (x < min)   min = x;
      x = iter.Payoff(2);
      if (x < min)   min = x;
      iter.Next(2);
    }
    iter.Next(1);
  }

  min-=(T)1;
  
  for (i = 1; i <= n1; i++) 
    for (int j = 1; j <= n1; j++) 
      dtab->Set_A(i, j) = (T)0;
  
  for (i = n1 + 1; i <= n1 + n2; i++)
    for (int j = n1 + 1; j <= n1 + n2; j++)
      dtab->Set_A(i, j) = (T)0;
  
  for (i = 1; i <= n1; i++)  {
    for (int j = 1; j <= n2; j++)  {
      dtab->Set_A(i, n1 + j) = iter.Payoff(1) - min;
      dtab->Set_A(n1 + j, i) = iter.Payoff(2) - min;
      iter.Next(2);
    }
    iter.Next(1);
  }
  for (i = 1; i <= n1 + n2; i++) 
    dtab->Set_b(i)=-1.0;
  Refactor();
  */
}

template <class T> SFTableau<T>::~SFTableau(void) 
{ }

template <class T> void SFTableau<T>
::FillTableau(const ExtForm<T> &E, const Node *n, T prob)
{ } 

#ifdef __GNUG__
template int NumSequences(int, const ExtForm<double> &);
template int NumSequences(int, const ExtForm<gRational> &);
template int NumInfosets(int, const ExtForm<double> &);
template int NumInfosets(int, const ExtForm<gRational> &);
template class SFTableau<double>;
template class SFTableau<gRational>;
#elif defined __BORLANDC__
#pragma option -Jgd
int NumSequences(int, const ExtForm<double> &);
int NumSequences(int, const ExtForm<gRational> &);
int NumInfosets(int, const ExtForm<double> &);
int NumInfosets(int, const ExtForm<gRational> &);
class SFTableau<double>;
class SFTableau<gRational>;
#pragma option -Jgx
#endif   // __GNUG__, __BORLANDC__





