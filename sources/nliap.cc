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
#include <limits.h>
#include "gambitio.h"
#include "normal.h"
#include "normiter.h"
#include "rational.h"
#include "gpvector.h"
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
: public gBC2FunctMin<T>, public BaseLiap, public SolutionModule {
private:
  const NormalForm<T> &rep;
  gPVector<T> p;
  LiapParams params;
public:
  LiapModule(const NormalForm<T> &N,gOutput &ofile,gOutput &efile,
	     const LiapParams &params) 
    :  SolutionModule(ofile,efile,plev),rep(N),
  p(rep.Dimensionality()), params(params),
  gBC2FunctMin<T>(N.ProfileLength()){ SetPlev(params.plev);}
  int Liap(int) ;
  virtual ~LiapModule() {}

  T Value(const gVector<T> &x);
  int Deriv(const gVector<T> &p, gVector<T> &d) {return 1;};
  int Hess(const gVector<T> &p, gMatrix<T> &d) {return 1;}
  T LiapDerivValue(int i1,int j1,const gPVector<T> &p) const;
  T LiapValue(const gPVector<T> &p) const;
  int Liap(gPVector<T> &p);
  int Nevals(void) {return nevals;}
  int Nits(void) {return nits;}
};



template <class T> int LiapModule<T>::Liap(int number)
{
  int n=0;
  T tmp,sum;
//  gPVector<T> p(rep.Centroid());
  gPVector<T> p(rep.Dimensionality());
  for(int i=1;i<=rep.NumPlayers();i++)
    for(int j= 1;j<= rep.NumStrats(i);j++)
      p(i,j)=((T)(1)/(T)(rep.NumStrats(i)));

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
	p(i,j)=tmp;
	sum+= tmp;
      }
      p(i,j)=(T)(1)-sum;
    }
    n+=(Liap(p));
  }
  return n;
};

template <class T> int LiapModule<T>::Liap(gPVector<T> &p)
{
  if(plev>=3) gout << "\np= " << p;
  T val; 
  val = MinPowell(p);
  gout << "\np= " << p << " f = " << val;
  if(val < (T) ( (T)(1) / (T)(100000) ) ) return 1;
  return 0;
};

template <class T>  
T LiapModule<T>::Value(const gVector<T> &x)
{
  assert(x.Length()==rep.ProfileLength());

  gPVector<T> m(rep.Dimensionality());
  m = x;
  return LiapValue(m);
  
};

#define BIG1 ((T) 100)
#define BIG2 ((T) 100)

template <class T>  
T LiapModule<T>::LiapValue(const gPVector<T> &p) const
{
  int i,j,num;
  gPVector<T> tmp(p);
  gPVector<T> payoff(p);
  T x,result,avg,sum;
  payoff = (T) (0);

  result= (T) 0;
  for(i=1;i<=rep.NumPlayers();i++) {
    tmp.SetRow(i,payoff.GetRow(i));
    avg=sum= (T) 0;
	// then for each strategy for that player set it to 1 and evaluate
    for(j = 1; j <= rep.NumStrats(i); j++) {
      tmp(i,j)= (T) 1;
      x=p(i,j);
      payoff(i,j) =  rep.Payoff(i,tmp);
//      gout << "\np[" << i << "][" << j << "] = " << payoff(i,j);
//      gout << "\ntmp = " << tmp;
      avg+=x*payoff(i,j);
      sum+=x;
      x= (x > ((T) 0)  ? ((T) 0)  : x);
      result += BIG1*x*x;         // add penalty for neg probabilities
      tmp(i,j) = (T) 0;
    }
    tmp.SetRow(i,p.GetRow(i));
    for(j=1;j<=rep.NumStrats(i);j++) {
      x=payoff(i,j)-avg;
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
				 *outfile, *errfile, params);
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

#if 0

double funct(double *p)
{
  get_y(p);
  return val(l_y);
}

void dfunct(double *p, double *x)
{
  int i;
  
  get_y(p);
  get_grad(l_y);
  for(i=1;i<=ndim;i++)x[i]=dp[i];
}

void get_grad(double **p)
{
  int i1,j1,ii;
  double avg;
  
  for(i1=1,ii=1;i1<=nplayers;i1++) {
    avg=(double)0.0;
    for(j1=1;j1<=nstrats[i1];j1++) {
      dp[ii]=deriv_val(i1,j1,p);
      avg+=dp[ii];
      ii++;
    }
    avg/=(double)nstrats[i1];
    ii-=nstrats[i1];
    for(j1=1;j1<=nstrats[i1];j1++) {
      dp[ii]-=avg;
      ii++;
    }
  }
}


void get_hess(double **p)
{
  int i1,i2,j1,j2,ii,iii;
  
  for(i1=1,ii=1;i1<=nplayers;i1++)
    for(j1=1;j1<=nstrats[i1];j1++) {
      for(i2=1,iii=1;i2<=nplayers;i2++)
	for(j2=1;j2<=nstrats[i2];j2++) {
	  if(iii<ii)dp2[ii][iii]=dp2[iii][ii];
	  else dp2[ii][iii]=deriv2_val(i1,j1,i2,j2,p);
	  iii++;
	}
      ii++;
    }
}


double val(double **p)
{
  int i,j;
  double v,psum,v1;

  v=(double)0.0;
  for(i=1;i<=nplayers;i++) {
    psum=(double)0.0;
    for(j=1;j<=nstrats[i];j++) {
      v1=M1(i,i,j,p)-M0(i,p);
      if(v1>(double)0.0)v+=pow(v1,(double)2.0);
      if(p[i][j]<(double)0.0)v+=pow(p[i][j],(double)2.0);
      psum+=p[i][j];
    }
    v+=pow((double)1.0-psum,(double)2.0);
  }
  return v;
}

double deriv_val(int i1, int j1, double **p)
{
  int i, j;
  double x, x1,psum;
  
  x=(double)0.0;
  for(i=1;i<=nplayers;i++) {
    psum=(double)0.0;
    for(j=1;j<=nstrats[i];j++) {
      psum+=p[i][j];
      x1=M1(i,i,j,p)-M0(i,p);
      if(x1>0)x+=x1*(M2(i,i,j,i1,j1,p)-M1(i,i1,j1,p));
    }
    if(i==i1)x+=psum-(double)1.0;
  }
  if(p[i1][j1]<(double)0.0)x+=p[i1][j1];
  return (double)2.0*x;
}

double deriv2_val(int i1, int j1, int i2, int j2, double **p)
{
  int i, j;
  double psum,x, x1;
  
  x=(double)0.0;
  psum=(double)0.0;
  for(i=1;i<=nplayers;i++)
    for(j=1;j<=nstrats[i];j++) {
      x1=M1(i,i,j,p)-M0(i,p);
      if(x1>=0)x+=(x1*(M3(i,i,j,i1,j1,i2,j2,p)-M2(i,i1,j1,i2,j2,p))
		   +(M2(i,i,j,i2,j2,p)-M1(i,i2,j2,p))
		   *(M2(i,i,j,i1,j1,p)-M1(i,i1,j1,p)));
      if(i==i1)psum+=p[i][j];
    }
  if(i1==i2)x+=(double)1.0;
  if(i1==12 && j1==j2 && p[i1][j1]<(double)0.0)x+=(double)1.0;
  return (double)2.0*x;
}




#endif




