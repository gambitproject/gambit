//#
//# FILE: liap.cc -- Liapunov module
//#
//# $Id$
//#

#define MAXIT 10
#define TOL (T)(.00000001)

#include <stdlib.h>
#include "gambitio.h"
#include "normal.h"
#include "normiter.h"
#include "rational.h"
#include "gpvector.h"
#include "gfunct.h"
#include "gwatch.h"
#include "grandom.h"
#include "liap.h"

//-------------------------------------------------------------------------
//                     LiapParams<T>: Member functions
//-------------------------------------------------------------------------

template <class T> LiapParams<T>::LiapParams(void)
  : nequilib(1), plev(0), tolDFP((T) 1.0e-10)
{ }

#ifdef __GNUG__
template class LiapParams<double>;
template class LiapParams<gRational>;
#elif defined __BORLANDC__
#pragma option -Jgd
class LiapParams<double>;
class LiapParams<gRational>;
#pragma option -Jgx
#endif   // __GNUG__, __BORLANDC__


//-------------------------------------------------------------------------
//                       BaseLiap: Class definition
//-------------------------------------------------------------------------

class BaseLiap {
  public:
    virtual int Nevals(void) = 0;
    virtual int Nits(void) = 0;
    virtual ~BaseLiap();
};

BaseLiap::~BaseLiap()
{ }

//-------------------------------------------------------------------------
//                    LiapModule<T>: Class definition
//-------------------------------------------------------------------------

template <class T> class LiapModule : public gBC2FunctMin<T>, public BaseLiap {
private:
  const NormalForm<T> &rep;
  gPVector<T> p;
  LiapParams<T> params;
  gOutput &output, &errors;
  int printlevel;

public:
  LiapModule(const NormalForm<T> &N,gOutput &ofile,gOutput &efile,
	     const LiapParams<T> &params) 
    : rep(N), output(ofile), errors(efile), printlevel(params.plev),
  p(rep.Dimensionality()), params(params),
  gBC2FunctMin<T>(N.ProfileLength()){ SetPlev(params.plev);}
  virtual ~LiapModule() {}

  T Value(const gVector<T> &x);
  int Deriv(const gVector<T> &p, gVector<T> &d);
  int Hess(const gVector<T> &p, gMatrix<T> &d) {return 1;}
  int Liap(int, gList<gPVector<T> > &);

  int Liap(gPVector<T> &p);
  T LiapDerivValue(int i1,int j1,const gPVector<T> &p) const;
//  T LiapValue(const gPVector<T> &p) const;
  int Nevals(void) {return nevals;}
  int Nits(void) {return nits;}
};



template <class T> int LiapModule<T>::Liap(int number,
					   gList<gPVector<T> > &sol)
{
  sol.Flush();

  int n = 0;
  T tmp,sum;
  gPVector<T> pp(rep.Dimensionality());
  rep.Centroid(pp);

  if (Liap(pp))  {
    n++;
    sol.Append(pp);
  }

  if (plev > 0)
    gout << " n = " << n;
  
  for (int it = 0; n < number && it < MAXIT; it++)  {
    for (int i = 1; i <= rep.NumPlayers(); i++)  {
      sum = (T) 0;
      for (int j = 1; j < rep.NumStrats(i); j++)  {
	do
	  tmp = (T) Uniform();
	while (tmp + sum > (T) 1);
	pp(i,j) = tmp;
	sum += tmp;
      }
      pp(i,j) = (T) 1 - sum;
    }
    if (Liap(pp))   {
      n++;
      sol.Append(pp);
    }
  }

  if (plev > 0)
    gout << '\n';

  return n;
};

template <class T> int LiapModule<T>::Liap(gPVector<T> &p)
{
  if (plev >= 3)
    gout << "\np= " << p;
  T val = (T) 0;
  int iter = 0;
  DFP(p, params.tolDFP, iter, val);
//  val = MinPowell(p);
  if (plev > 0)
    gout << "\np= " << p << " f = " << val;
  return (val < (T) ((T) 1 / (T) 100000));
};

#define BIG1 ((T) 100)
#define BIG2 ((T) 100)

template <class T> T LiapModule<T>::Value(const gVector<T> &x)
{
  assert(x.Length()==p.Length());

  p=x;
  int i,j,num;
  gPVector<T> tmp(p);
  gPVector<T> payoff(p);
  T x,result,avg,sum;
  payoff = (T) (0);

  result= (T) 0;
  for(i=1;i<=rep.NumPlayers();i++) {
    tmp.CopyRow(i, payoff);
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
    tmp.CopyRow(i, p);
    for(j=1;j<=rep.NumStrats(i);j++) {
      x=payoff(i,j)-avg;
      x = (x > 0 ? x : 0);
      result += x*x;          // add penalty if not best response
    }
    x=sum - ((T) 1);
    result += BIG2*x*x ;          // add penalty for sum not equal to 1
  }
  return result;

//  return LiapValue(m);
  
};

template <class T> int LiapModule<T>::Deriv(const gVector<T> &v, gVector<T> &d)
{
  p=v;
  int i1,j1,ii;
  T avg;
  
  for(i1=1,ii=1;i1<=rep.NumPlayers();i1++) {
    avg=(T)0;
    for(j1=1;j1<=rep.NumStrats(i1);j1++) {
      d[ii]=LiapDerivValue(i1,j1,p);
      avg+=d[ii];
      ii++;
    }
    avg/=(T)rep.NumStrats(i1);
    ii-=rep.NumStrats(i1);
    for(j1=1;j1<=rep.NumStrats(i1);j1++) {
      d[ii]-=avg;
      ii++;
    }
  }
};

template <class T> T LiapModule<T>::
LiapDerivValue(int i1, int j1, const gPVector<T> &p) const
{
  int i, j;
  T x, x1,psum;
  
  x=(T)0;
  for(i=1;i<=rep.NumPlayers();i++) {
    psum=(T)0.0;
    for(j=1;j<=rep.NumStrats(i);j++) {
      psum+=p(i,j);
      x1=rep.Payoff(i,i,j,p)-rep.Payoff(i,p);
      if(i1==i) {
	if(x1>0)x-=x1*(rep.Payoff(i,i1,j1,p));
      }
      else {
	if(x1>0)x+=x1*(rep.Payoff(i,i,j,i1,j1,p)-rep.Payoff(i,i1,j1,p));
      }
    }
    if(i==i1)x+=psum-(T)1.0;
  }
  if(p(i1,j1)<(T)0.0)x+=p(i1,j1);
  return (T)2.0*x;
};

#ifdef __GNUG__
template class LiapModule<double>;
template class LiapModule<gRational>;
#elif defined __BORLANDC__
#pragma option -Jgd
class LiapModule<double>;
class LiapModule<gRational>;
#pragma option -Jgx
#endif   // __GNUG__, __BORLANDC__

//------------------------------------------------------------------------
//                     LiapSolver<T>: Member functions
//------------------------------------------------------------------------

template <class T> LiapSolver<T>::LiapSolver(const NormalForm<T> &N,
					     const LiapParams<T> &p)
  : nf(N), params(p), nevals(0), nits(0)
{ }

template <class T> int LiapSolver<T>::NumEvals(void) const
{
  return nevals;
}

template <class T> int LiapSolver<T>::NumIters(void) const
{
  return nits;
}

template <class T> gRational LiapSolver<T>::Time(void) const
{
  return time;
}

template <class T> LiapParams<T> &LiapSolver<T>::Parameters(void)
{
  return params;
}

template <class T>
const gList<gPVector<T> > &LiapSolver<T>::GetSolutions(void) const
{
  return solutions;
}

template <class T> int LiapSolver<T>::Liap(void)
{
  gOutput *outfile = &gout, *errfile = &gerr;

  if (params.outfile != "")
    outfile = new gFileOutput((char *) params.outfile);
  if (params.errfile != "" && params.errfile != params.outfile)
    errfile = new gFileOutput((char *) params.errfile);
  if (params.errfile != "" && params.errfile == params.outfile)
    errfile = outfile;

  gWatch watch;
  LiapModule<T> LT(nf, *outfile, *errfile, params);

  LT.Liap(params.nequilib, solutions);
  time = watch.Elapsed();
  nits = LT.Nits();
  nevals = LT.Nevals();
  

  if (params.outfile != "")
    delete outfile;
  if (params.errfile != "" && params.errfile != params.outfile)
    delete errfile;

  return 1;
}


#ifdef __GNUG__
template class LiapSolver<double>;
template class LiapSolver<gRational>;
#elif defined __BORLANDC__
#pragma option -Jgd
class LiapSolver<double>;
class LiapSolver<gRational>;
#pragma option -Jgx
#endif   // __GNUG__, __BORLANDC__


#ifdef UNUSED

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




