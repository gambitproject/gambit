//#
//# FILE: csum.cc -- Constant Sum Game Solution Module
//#
//# $Id$
//#

#include "rational.h"
#include "gwatch.h"
#include "gpvector.h"

#include "normal.h"
#include "normiter.h"

#include "csum.h"

//---------------------------------------------------------------------------
//                        ZSumParams: member functions
//---------------------------------------------------------------------------

ZSumParams::ZSumParams(void) 
  :  trace(0), stopAfter(0), tracefile(&gnull)
{ }


//-------------------------------------------------------------------------
//                    ZSumModule<T>: Member functions
//-------------------------------------------------------------------------

template <class T>
ZSumModule<T>::ZSumModule(const NormalForm<T> &N, const ZSumParams &p)
  : NF(N), params(p), A(0), b(0), c(0), npivots(0)
{ }

template <class T> ZSumModule<T>::~ZSumModule()
{ if(A) delete A; if(b) delete b; if(c) delete c;}

template <class T> bool ZSumModule<T>::IsConstSum()
{ 
// useless as written 
  return 1;
}

template <class T> void ZSumModule<T>::Make_Abc()
{
  int i,j,m,k;
  T pay;
  
  m = NF.NumStrats(1);
  k = NF.NumStrats(2);
  A = new gMatrix<T>(1,k+1,1,m+1);
  b = new gVector<T>(1,k+1);
  c = new gVector<T>(1,m+1);
  NormalIter<T> iter(NF);

      // Ted -- this should be in the normal form.  
  minpay = (T)0;
  for (i = 1; i <= k; i++)  {
    for (j = 1; j <= m; j++)  {
      pay = iter.Payoff(1);
      if(pay < minpay)minpay = pay;
      iter.Next(1);
    }
    (*A)(i,m+1) = (T)1;
    iter.Next(2);
  }

  minpay=minpay-(T)1;

  for (i = 1; i <= k; i++)  {
    for (j = 1; j <= m; j++)  {
      (*A)(i, j) = minpay-iter.Payoff(1);
      iter.Next(1);
    }
    (*A)(i,m+1) = (T)1;
    iter.Next(2);
  }
  for (j = 1;j<=m;j++) (*A)(k+1,j)= (T)1;
  (*A)(k+1,m+1) = (T)0;

  (*b) = (T)0;
  (*b)[k+1] = (T)1;
  (*c) = (T)0;
  (*c)[m+1] = (T)1;
//  gout << "\nA = \n";
//  A->Dump(gout);
//  gout << "\nb = ";
//  b->Dump(gout);
//  gout << "\nc = ";
//  c->Dump(gout);
}

template <class T> int ZSumModule<T>::ZSum(int dup)
{
  BFS<T> cbfs((T) 0);
  
  if (NF.NumPlayers() != 2 || !params.tracefile)   return 0;
  if(!IsConstSum()) return 0;;
  gWatch watch;
  
  List = BFS_List();  // Ted -- is this necessary?
  
  Make_Abc();
  LPSolve<T> LP(*A,*b,*c,1);
  Add_BFS(LP); 
  
  npivots = LP.NumPivots();
  time = watch.Elapsed();
  return List.Length();
}

template <class T> int ZSumModule<T>::Add_BFS(const LPSolve<T> &lp)
{
  BFS<T> cbfs((T) 0);
  T value;

  lp.OptBFS(cbfs);
  value=cbfs(NF.NumStrats(1)+1)+minpay;
  cbfs.Remove(NF.NumStrats(1)+1);
  cbfs.Remove(-NF.NumStrats(2)-1);
  cbfs.Dump(gout);
  gout << "\nvalue =" << value << "\n\n";
  if (List.Contains(cbfs))  return 0;
//  if(params.trace >=2) (*params.tracefile) << "\nFound CBFS";
//  (*params.tracefile)  << "\nB = ";
//  B.Dump(*params.tracefile);
//  (*params.tracefile)  << "\ncbfs = ";
//  cbfs.Dump(*params.tracefile );
  List.Append(cbfs);
  return 1;
}

template <class T>
void ZSumModule<T>::GetSolutions(gList<MixedProfile<T> > &solutions) const
{
  int n1=NF.NumStrats(1);
  int n2=NF.NumStrats(2);
  solutions.Flush();

  for (int i = 1; i <= List.Length(); i++)    {
    MixedProfile<T> profile(NF);
    for (int j = 1; j <= n1; j++) 
      if (List[i].IsDefined(j))   
	profile(1, j) = List[i](j);
      else  profile(1, j) = (T) 0;

    for (j = 1; j <= n2; j++)
      if (List[i].IsDefined(-j))
	profile(2, j) = List[i](-j);
      else
	profile(2, j) = (T) 0;

    solutions.Append(profile);
  }
}

template <class T> long ZSumModule<T>::NumPivots(void) const
{
  return npivots;
}

template <class T> double ZSumModule<T>::Time(void) const
{
  return time;
}

#ifdef __GNUG__
template class ZSumModule<double>;
template class ZSumModule<gRational>;
#elif defined __BORLANDC__
#pragma option -Jgd
class ZSumModule<double>;
class ZSumModule<gRational>;
#pragma option -Jgx
#endif   // __GNUG__, __BORLANDC__


//-------------------------------------------------------------------------
//                    Convenience functions for ZSum
//-------------------------------------------------------------------------

template <class T>
int ZSum(const NormalForm<T> &N, const ZSumParams &p,
	  gList<MixedProfile<T> > &solutions,
	  long &npivots, gRational &time)
{
  ZSumModule<T> LM(N, p);
  int result = LM.ZSum();

  npivots = LM.NumPivots();
  time = LM.Time();
  
  LM.GetSolutions(solutions);

  return result;
}

#ifdef __GNUG__
template int ZSum(const NormalForm<double> &, const ZSumParams &,
		   gList<MixedProfile<double> > &, long &, gRational &);
template int ZSum(const NormalForm<gRational> &, const ZSumParams &,
		   gList<MixedProfile<gRational> > &, long &, gRational &);
#elif defined __BORLANDC__
#pragma option -Jgd
int ZSum(const NormalForm<double> &, const ZSumParams &,
	  gList<MixedProfile<double> > &, long &, gRational &);
int ZSum(const NormalForm<gRational> &, const ZSumParams &,
	  gList<MixedProfile<gRational> > &, long &, gRational &);
#pragma option -Jgx
#endif   // __GNUG__, __BORLANDC__




