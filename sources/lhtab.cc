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

template <class T> gMatrix<T> Make_A(const NormalForm<T> &N)
{
  int n1, n2, i,j;
  n1=N.NumStrats(1);
  n2=N.NumStrats(2);
  gMatrix<T> A(1,n1+n2,1,n1+n2);
  NormalIter<T> iter(N); 
  T min = (T) 0, x; 
  
  for (i = 1; i <= n1; i++)   {
    for (j = 1; j <= n2; j++)  {
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
    for (j = 1; j <= n1; j++) 
      A(i, j) = (T)0;
  
  for (i = n1 + 1; i <= n1 + n2; i++)
    for (j = n1 + 1; j <= n1 + n2; j++)
      A(i, j) = (T)0;
  
  for (i = 1; i <= n1; i++)  {
    for (j = 1; j <= n2; j++)  {
      A(i, n1 + j) = iter.Payoff(1) - min;
      A(n1 + j, i) = iter.Payoff(2) - min;
      iter.Next(2);
    }
    iter.Next(1);
  }
//  A.Dump(gout);
  return A;
}

template <class T> gVector<T> Make_b(const NormalForm<T> &N)
{
  int n1, n2, i;
  n1=N.NumStrats(1);
  n2=N.NumStrats(2);
  gVector<T> b(1,n1+n2);

  for (i = 1; i <= n1 + n2; i++) 
    b[i]=-(T)1;
//  b.Dump(gout);
  return b;
}

template <class T> LHTableau<T>::LHTableau(const NormalForm<T> &N) 
  : LTableau<T>(Make_A(N),Make_b(N))
{ 
//  Refactor(); 
}


template <class T> LHTableau<T>::LHTableau(Tableau<T> &tab)
  : LTableau<T>(tab) 
{ }

template <class T> LHTableau<T>::~LHTableau(void) 
{ }



#ifdef __GNUG__
template class LHTableau<double>;
template class LHTableau<gRational>;
template class gMatrix<double> Make_A(const NormalForm<double> &);
template class gMatrix<gRational> Make_A(const NormalForm<gRational> &);
template class gVector<double> Make_b(const NormalForm<double> &);
template class gVector<gRational> Make_b(const NormalForm<gRational> &);
#elif defined __BORLANDC__
#pragma option -Jgd
class LHTableau<double>;
class LHTableau<gRational>;
class gMatrix<double> Make_A(const NormalForm<double> &);
class gMatrix<gRational> Make_A(const NormalForm<gRational> &);
class gVector<double> Make_b(const NormalForm<double> &);
class gVector<gRational> Make_b(const NormalForm<gRational> &);
#pragma option -Jgx
#endif   // __GNUG__, __BORLANDC__





