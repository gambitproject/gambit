//#
//# FILE: enum.cc -- Nash Enum module
//#
//# $Id$
//#

#include "gwatch.h"
#include "nfg.h"
#include "nfgiter.h"

#include "enum.h"

void Epsilon_T(double &v, int i);
void Epsilon_T(gRational &v, int i);

//---------------------------------------------------------------------------
//                        EnumParams: member functions
//---------------------------------------------------------------------------

EnumParams::EnumParams(gStatus &status_) : trace(0), stopAfter(0),
tracefile(&gnull),status(status_)
{ }

//-------------------------------------------------------------------------
//                    EnumModule<T>: Member functions
//-------------------------------------------------------------------------

template <class T>
EnumModule<T>::EnumModule(const Nfg<T> &N, const EnumParams &p,
			  const NFSupport &S)
  : NF(N), support(S), params(p), rows(S.NumStrats(1)), cols(S.NumStrats(2)), 
    level(0), count(0), npivots(0)
{ 
  Epsilon_T(eps,2);
}


template <class T> int EnumModule<T>::Enum(void)
{
  if (NF.NumPlayers() != 2)   return 0;  
  int n1, n2, v1,v2,i,j,k;

  gWatch watch;

  n1=support.NumStrats(1);
  n2=support.NumStrats(2);
  NfgIter<T> iter(&support); 

  gMatrix<T> A1(1,n1,1,n2);
  gMatrix<T> A2(1,n2,1,n1);
  gVector<T> b1(1,n1);
  gVector<T> b2(1,n2);

   // compute minimum payoff  

  T min = (T) 1, x; 
  for (i=1; i<=n1; i++)   {
    for (j=1; j<=n2; j++)  {
      x = iter.Payoff(1);
      if (x < min)   min = x;
      x = iter.Payoff(2);
      if (x < min)   min = x;
      iter.Next(2);
    }
    iter.Next(1);
  }
  min-=(T)1;

   // construct A1,A2,b1,b2  

  for (i=1; i<=n1; i++)  {
    A1(i, n2) = - (T)1;
    A2(n2, i) = (T)1;
    for (j=1; j<=n2; j++)  {
      A1(i, j) = iter.Payoff(1) - min;
      A2(j, i) = iter.Payoff(2) - min;
      iter.Next(2);
    }
    iter.Next(1);
  }

  b1 = -(T)1;
  b2 = -(T)1;

  // enumerate vertices of A1 x + b1 <= 0 and A2 x + b2 <= 0

  VertEnum<T> poly1(A1,b1);
  VertEnum<T> poly2(A2,b2);

  v1=poly1.VertexList().Last();
  v2=poly2.VertexList().Last();

  BFS<T> bfs1,bfs2;
  MixedProfile<T> profile(NF,support);
  T sum;
  bool nash;

  for(i=2;i<=v2;i++) {
    bfs1 = poly2.VertexList()[i];
    sum = (T)0;
    for(k=1;k<=n1;k++) {
      profile(1,k) = (T)0;
      if(bfs1.IsDefined(k)) {
	profile(1,k) =-bfs1(k);
	sum+=profile(1,k);
      }
    } 
    for(k=1;k<=n1;k++) {
      if(bfs1.IsDefined(k)) 
	profile(1,k)/=sum;
    }
    for(j=2;j<=v1;j++) {
      bfs2 = poly1.VertexList()[j];

      // check if solution is nash 
      // need only check complementarity, since it is feasible

      nash=1;
      for(k=1;k<=n1;k++)
	if(bfs1.IsDefined(k) && bfs2.IsDefined(-k))
	  if(!EqZero(bfs1(k)*bfs2(-k)))
	    nash=0;
      for(k=1;k<=n2;k++)
	if(bfs2.IsDefined(k) && bfs1.IsDefined(-k))
	  if(!EqZero(bfs2(k)*bfs1(-k)))
	    nash=0;

      if(nash) {
	sum = (T)0;
	for(k=1;k<=n2;k++) {
	  profile(2,k) = (T)0;
	  if(bfs2.IsDefined(k)) {
	    profile(2,k) =-bfs2(k);
	    sum+=profile(2,k);
	  }
	} 
	for(k=1;k<=n2;k++) {
	  if(bfs2.IsDefined(k)) 
	    profile(2,k)/=sum;
	} 
//	gout << "\nprofile " << i << ", " << j << " " << profile << ":";
//	for(k=1;k<=n1;k++) {
//	  if(bfs1.IsDefined(k)) gout << " " << k;
//	  if(bfs2.IsDefined(-k)) gout << " " << -k;
//	}
//	gout << ";";
//	for(k=1;k<=n2;k++) {
//	  if(bfs2.IsDefined(k)) gout << " " << n1+k;
//	  if(bfs1.IsDefined(-k)) gout << " " << -n1-k;
//	}
//	gout << " Nash";
	solutions.Append(profile);
      }
    }
  }
//  gout << "\n";
  npivots = poly1.NumPivots()+poly2.NumPivots();

  time = watch.Elapsed();
  return 1;
}


template <class T> bool EnumModule<T>::EqZero(T x) const
{
  if(x <= eps && x >= -eps) return 1;
}     

template <class T> long EnumModule<T>::NumPivots(void) const
{
  return npivots;
}

template <class T> double EnumModule<T>::Time(void) const
{
  return time;
}

template <class T> EnumParams &EnumModule<T>::Parameters(void)
{
  return params;
}

template <class T>
gList<MixedProfile<T> > &EnumModule<T>::GetSolutions(void)
{
  return solutions;
}

#include "rational.h"

#ifdef __GNUG__
template class EnumModule<double>;
template class EnumModule<gRational>;
#elif defined __BORLANDC__
#pragma option -Jgd
class EnumModule<double>;
class EnumModule<gRational>;
#pragma option -Jgx
#endif   // __GNUG__, __BORLANDC__


//-------------------------------------------------------------------------
//                    Convenience functions for Enum
//-------------------------------------------------------------------------

template <class T>
int Enum(const Nfg<T> &N, const EnumParams &p,
	  gList<gPVector<T> > &/*solutions*/,
	  long &npivots, gRational &time)
{
  NFSupport S(N);
  EnumModule<T> LM(N, p, S);
  int result = LM.Enum();

  npivots = LM.NumPivots();
  time = LM.Time();
  
  LM.GetSolutions();

  return result;
}

#ifdef __GNUG__
template int Enum(const Nfg<double> &, const EnumParams &,
		   gList<gPVector<double> > &, int &, gRational &);
template int Enum(const Nfg<gRational> &, const EnumParams &,
		   gList<gPVector<gRational> > &, int &, gRational &);
#elif defined __BORLANDC__
#pragma option -Jgd
int Enum(const Nfg<double> &, const EnumParams &,
	  gList<gPVector<double> > &, int &, gRational &);
int Enum(const Nfg<gRational> &, const EnumParams &,
	  gList<gPVector<gRational> > &, int &, gRational &);
#pragma option -Jgx
#endif   // __GNUG__, __BORLANDC__










