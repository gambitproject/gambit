//#
//# FILE: lhtab.cc -- Lemke-Howson tableau
//#
//# $Id$ 
//#

#include "lhtab.h"
#include "normal.h"
#include "normiter.h"

//---------------------------------------------------------------------------
//                        LemkeHowson Tableau: member functions
//---------------------------------------------------------------------------
 
template <class T> LHTableau<T>::LHTableau(const NormalForm<T> &N) 
  : LTableau<T>(1, N.NumStrats(1) + N.NumStrats(2), 1, N.NumStrats(1) +
	   N.NumStrats(2))
{ 
  int n1, n2;
  n1=N.NumStrats(1);
  n2=N.NumStrats(2);
  NormalIter<T>
    iter(N); T min = (T) 0, x; int i;
  
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
}


template <class T> LHTableau<T>::LHTableau(Tableau<T> &tab)
  : LTableau<T>(tab) 
{ }

template <class T> LHTableau<T>::~LHTableau(void) 
{ }



#ifdef __GNUG__
template class LHTableau<double>;
template class LHTableau<gRational>;
#elif defined __BORLANDC__
#pragma option -Jgd
class LHTableau<double>;
class LHTableau<gRational>;
#pragma option -Jgx
#endif   // __GNUG__, __BORLANDC__





