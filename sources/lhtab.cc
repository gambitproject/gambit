//#
//# FILE: lhtab.cc -- Lemke-Howson tableau
//#
//# $Id$ 
//#

#include "lhtab.h"
#include "nfg.h"
#include "nfgiter.h"
#include "nfstrat.h"

//---------------------------------------------------------------------------
//                        LemkeHowson Tableau: member functions
//---------------------------------------------------------------------------

template <class T> gMatrix<T> Make_A(const Nfg<T> &, const NFSupport &S)
{
  int n1, n2, i,j;
  n1=S.NumStrats(1);
  n2=S.NumStrats(2);
  gMatrix<T> A(1,n1+n2,1,n1+n2);
  NfgIter<T> iter(&S); 
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

template <class T> gVector<T> Make_b(const Nfg<T> &, const NFSupport &S)
{
  int n1, n2, i;
  n1=S.NumStrats(1);
  n2=S.NumStrats(2);
  gVector<T> b(1,n1+n2);

  for (i = 1; i <= n1 + n2; i++) 
    b[i]=-(T)1;
//  b.Dump(gout);
  return b;
}

template <class T> LHTableau<T>::LHTableau(const Nfg<T> &N, const NFSupport &S) 
  : LTableau<T>(Make_A(N, S),Make_b(N, S))
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
template class gMatrix<double> Make_A(const Nfg<double> &, const NFSupport &);
template class gMatrix<gRational> Make_A(const Nfg<gRational> &, const NFSupport &);
template class gVector<double> Make_b(const Nfg<double> &, const NFSupport &);
template class gVector<gRational> Make_b(const Nfg<gRational> &, const NFSupport &);
#elif defined __BORLANDC__
#pragma option -Jgd
class LHTableau<double>;
class LHTableau<gRational>;
class gMatrix<double> Make_A(const Nfg<double> &, const NFSupport &);
class gMatrix<gRational> Make_A(const Nfg<gRational> &, const NFSupport &);
class gVector<double> Make_b(const Nfg<double> &, const NFSupport &);
class gVector<gRational> Make_b(const Nfg<gRational> &, const NFSupport &);
#pragma option -Jgx
#endif   // __GNUG__, __BORLANDC__





