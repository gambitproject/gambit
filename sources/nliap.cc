//#
//# FILE: liap.cc -- Liapunov module
//#
//# $Id$
//#

#define MAXIT 10
#define TOL (T)(.00000001)

#ifdef __GNUG__
#pragma implementation "liap.h"
#endif   // __GNUG__

#include <stdlib.h>
#include "gambitio.h"
#include "normal.h"
#include "normiter.h"
#include "rational.h"
#include "mixed.h"
#include "solution.h"
#include "gfunct.h"
#include "liap.h"

LiapParams::LiapParams(void) : nequilib(1), plev(2)
{ }

class BaseLiap {
public:
  virtual int Liap(int) = 0;  
  virtual int Nevals(void) = 0;
  virtual int Nits(void) = 0;
  virtual ~BaseLiap() {}
};

template <class T> class LiapModule 
: public gBFunct_nDim<T>, public BaseLiap, public SolutionModule {
private:
  const NormalForm<T> &rep;
public:
  LiapModule(const NormalForm<T> &N,gOutput &ofile,gOutput &efile,int plev) 
    :  SolutionModule(ofile,efile,plev),rep(N),
  gBFunct_nDim<T>(N.ProfileLength()){ SetPlev(plev);}
  int Liap(int) ;
  virtual ~LiapModule() {}

  T LiapValue(const MixedProfile<T> &p) const;
  T operator()(const gVector<T> &x);
  int Liap(MixedProfile<T> &p);
  int Nevals(void) {return nevals;}
  int Nits(void) {return nits;}
};



template <class T> int LiapModule<T>::Liap(int number)
{
  int n=0;
  T tmp,sum;
  MixedProfile<T> p(rep.Centroid());
  n+=(Liap(p));
  gout << " n = " << n;
  int it = 0;
  while(n<number && it < MAXIT) {
    it++;
    for(int i=1;i<rep.NumPlayers();i++) {
      sum = (T)(0);
      for(int j=1;j<rep.NumStrats(i);j++) {
	tmp=(T)(2);
	while (tmp + sum > ((T)(1)) ) tmp = (T)((T)rand()/(T)INT_MAX);
	p[i][j]=tmp;
	sum+= tmp;
      }
      p[i][j]=(T)(1)-sum;
    }
    n+=(Liap(p));
  }
  return n;
};

template <class T> int LiapModule<T>::Liap(MixedProfile<T> &p)
{
  gVector<T> v(rep.ProfileLength());
  int k=1;

  
  for(int i=1;i<=rep.NumPlayers();i++)
    {
      for(int j=1;j<=rep.NumStrats(i);j++) {
	v[k]=p[i][j];
	k++;
      }
    }
  if(plev>=3)
    gout << "\nv= " << v;
  T val; 
  val = MinPowell(v);
  gout << "\nv= " << v << " f = " << val;
  if(val < (T) ( (T)(1) / (T)(100000) ) ) return 1;
  return 0;
};

template <class T>  
T LiapModule<T>::operator()(const gVector<T> &x)
{
  assert(x.Length()==rep.ProfileLength());

  MixedProfile<T> m(rep);

  int k=1;
  for(int i=1;i<=rep.NumPlayers();i++)
    {
      m[i] = gVector<T>(rep.NumStrats(i));
      for(int j=1;j<=rep.NumStrats(i);j++){
	m[i][j]=x[k];
	k++;
      }
    }
  return LiapValue(m);
  
};

#define BIG1 ((T) 100)
#define BIG2 ((T) 100)

template <class T>  
T LiapModule<T>::LiapValue(const MixedProfile<T> &p) const
{
  int i,j,num;
  MixedProfile<T> tmp(p);
  gVector<T> payoff;
  T x,result,avg,sum;
  
  result= (T) 0;
  for(i=1;i<=rep.NumPlayers();i++) {
    payoff = gVector<T>(rep.NumStrats(i));
    payoff=(T) 0;
    tmp[i]=(T) 0;
    avg=sum= (T) 0;
	// then for each strategy for that player set it to 1 and evaluate
    for(j = 1; j <= rep.NumStrats(i); j++) {
      tmp[i][j]= (T) 1;
      x=p[i][j];
      payoff[j] = rep.Payoff(i,tmp);
//      gout << "\np[" << i << "][" << j << "] = " << payoff[j];
//      gout << "\ntmp = " << tmp;
      avg+=x*payoff[j];
      sum+=x;
      x= (x > ((T) 0)  ? ((T) 0)  : x);
      result += BIG1*x*x;         // add penalty for neg probabilities
      tmp[i][j]= (T) 0;
    }
    tmp[i]=p[i];
    for(j=1;j<=rep.NumStrats(i);j++) {
      x=payoff[j]-avg;
      x = (x > 0 ? x : 0);
      result += x*x;          // add penalty if not best response
    }
    x=sum - ((T) 1);
    result += BIG2*x*x ;          // add penalty for sum not equal to 1
  }
  return result;
}

int LiapSolver::Liap(void)
//int LiapModule::Liap(int number, int plev, gOutput &out, gOutput &err,
//		   int &nevals, int &nits)
{
  BaseLiap *T;
  gOutput *outfile = &gout, *errfile = &gerr;

  gout << "\nLiapSolver::Liap() Loc 1";
  if (params.outfile != "")
    outfile = new gFileOutput((char *) params.outfile);
  if (params.errfile != "" && params.errfile != params.outfile)
    errfile = new gFileOutput((char *) params.errfile);
  if (params.errfile != "" && params.errfile == params.outfile)
    errfile = outfile;

  gout << "\nLiapSolver::Liap() Loc 2";
  switch (nf.Type())   {
    case DOUBLE:   
      T = new LiapModule<double>((NormalForm<double> &) nf, 
				 *outfile, *errfile, params.plev);
      break;
/*
    case RATIONAL:  
      T = new LiapModule<Rational>((NFRep<Rational> &) *data, out, err, plev);
      break; 
*/
  }
  gout << "\nLiapSolver::Liap() Loc 3";

  T->Liap(params.nequilib);
  gout << "\nLiapSolver::Liap() Loc 4";
  nits=T->Nits();
  nevals= T->Nevals();

  if (params.outfile != "")
    delete outfile;
  if (params.errfile != "" && params.errfile != params.outfile)
    delete errfile;

  delete T;
  return 1;

}






