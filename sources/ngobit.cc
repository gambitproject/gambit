//#
//# FILE: gobit.cc -- Gobit module
//#
//# @(#)gobit.cc	1.4 10/27/94
//#

#include "gambitio.h"
#include "normal.h"
#include "normiter.h"
#include "rational.h"
#include "mixed.h"
#include "solution.h"
#include "gfunct.h"
#include "gobit.h"

class BaseGobit {
public:
  virtual int Gobit(int) = 0;
  virtual ~BaseGobit() {}
};

template <class T> class GobitModule
: public gBC2Funct_nDim<T>, public BaseGobit, public SolutionModule {
private:
  const NormalForm<T> &rep;
  T Lambda;
  MixedProfile<T> p;
  gVector<T> x;
  T minlam,maxlam,factor;
  int maxits;
  
public:
  GobitModule(const NormalForm<T> &N,gOutput &ofile,gOutput &efile,int plev)
    :  SolutionModule(ofile,efile,plev), rep(N), Lambda(0), p(rep),
  minlam((T)(.01)),maxlam(30),factor((T)(1.05)),maxits(500),
  x(rep.ProfileLength()), gBC2Funct_nDim<T>(N.ProfileLength()){ }
  virtual ~GobitModule() {}
  
  T GobitDerivValue(int i1, int j1, const MixedProfile<T> &p) const;
  T GobitValue(void) const;
  void get_x(void);
  void get_p(void);
  void get_grad(const MixedProfile<T> &p, gVector<T> &dp);
  
  T operator()(const gVector<T> &x);
  int Deriv(const gVector<T> &p, gVector<T> &d);
  int Hess(const gVector<T> &p, gMatrix<T> &d) {return 1;}
  int Gobit(int);
};

template <class T> int GobitModule<T>::Gobit(int number)
{
  p = MixedProfile<T>(rep.Centroid());
  x = gVector<T>(rep.ProfileLength());
  T ftol;
  int iter=0;

  ftol  = ((T)(1.0e-10));
  Lambda=minlam;
  T value;
  
  int nit=0;
  while(nit < maxits && Lambda<=maxlam) {
    nit++;
    get_x();
    
    DFP(x,ftol,iter,value);
    get_p();
    gout << "\nLam = " << Lambda << " nits= " << iter;
    gout << " val = " << value << " p = " << p;
    if(value>=10.0)return nit;
    Lambda*=factor;
  }
  return nit;
};

template <class T>
T GobitModule<T>::operator()(const gVector<T> &v)
{
//  gout << " in GobitModule::Operator()";
  assert(v.Length()==x.Length());
  
  x=v;
  get_p();
  return GobitValue();
};

template <class T> int GobitModule<T>::
Deriv(const gVector<T> &v, gVector<T> &d)
{
  x=v;
  get_p();
  get_grad(p,d);
//  for(i=1;i<=rep.ProfileLength();i++)x[i]=dp[i];
//  gout << "\n in Deriv()";
//  gout << "\n p = " << p;
//  gout << "\n d = " << d;

  return 1;
};

template <class T> void GobitModule<T>::
get_grad(const MixedProfile<T> &p, gVector<T> &dp)
{
  int i1,j1,ii;
  T avg;

//  gout << "\n in get_grad()";

  for(i1=1,ii=1;i1<=rep.NumPlayers();i1++) { 
    avg=(T)(0);
    for(j1=1;j1<=rep.NumStrats(i1);j1++) {
      dp[ii]=GobitDerivValue(i1,j1,p);
      avg+=dp[ii];
      ii++;
    }
    avg/=(T)rep.NumStrats(i1);
    
    ii-=rep.NumStrats(i1);
    for(j1=1;j1<=rep.NumStrats(i1);j1++) {
      dp[ii]-=avg;
      ii++;
    }
  }
}



template <class T> void GobitModule<T>::
get_x(void)
{
  int k=1;
  for(int i=1;i<=rep.NumPlayers();i++) {
    for(int j=1;j<=rep.NumStrats(i);j++) {
      x[k]=p[i][j];
      k++;
    }
  }
};

template <class T> void GobitModule<T>::
get_p(void)
{
  int k=1;
  for(int i=1;i<=rep.NumPlayers();i++) {
    for(int j=1;j<=rep.NumStrats(i);j++) {
      p[i][j]=x[k];
      k++;
    }
  }
};

#define BIG1 ((T) 100)
#define BIG2 ((T) 100)

template <class T> T GobitModule<T>::
GobitValue(void) const
{
  int i,j;
  T v,psum,z;
  
  v=(T)(0);
//  gout << "\nGobitValue(): p = " << p;
  for(i=1;i<=rep.NumPlayers();i++) {
//    psum=p[i][1];
    for(j=2;j<=rep.NumStrats(i);j++) {
      z=log(p[i][1])-log(p[i][j])
	- Lambda*(rep.Payoff(i,i,1,p)-rep.Payoff(i,i,j,p));
      v+=(z*z);
//      if(p[i][j]<(double)0.0)v+=(p[i][j]*p[i][j]);
//      psum+=p[i][j];
    }
//    z=(T)(1)-psum;
//    v+=(z*z);
  }
  return v;
}

template <class T> T GobitModule<T>::
GobitDerivValue(int i1, int j1, const MixedProfile<T> &p) const
{
  int i, j;
  T x, x1,dv;
  
  x=(T)(0);
//  gout << "\nGobitDerivValue(): p = " << p;
  for(i=1;i<=rep.NumPlayers();i++) {
    for(j=2;j<=rep.NumStrats(i);j++) {
      dv=log(p[i][1])-log(p[i][j])
	- Lambda*(rep.Payoff(i,i,1,p)-rep.Payoff(i,i,j,p));
      if(i==i1) {
	if(j1==1)  x+=dv/p[i][1];
	if(j1==j)  x-=dv/p[i][j];
      }
      if(i!=i1)
	x-=dv*Lambda*(rep.Payoff(i,i,1,i1,j1,p)-rep.Payoff(i,i,j,i1,j1,p));
    }
  }
//  if(p[i1][j1]<(T)(0))x+=p[i1][j1];

  return ((T)(2))*x;
}


int GobitSolver::Gobit(void)
// int NormalForm::Gobit(int number,int plev, gOutput &out, gOutput &err,
//		      int &nevals, int &nits)
{
  BaseGobit *T;
  gOutput *outfile = &gout, *errfile = &gerr;

  if (params.outfile != "")
    outfile = new gFileOutput((char *) params.outfile);
  if (params.errfile != "" && params.errfile != params.outfile)
    errfile = new gFileOutput((char *) params.errfile);
  if (params.errfile != "" && params.errfile == params.outfile)
    errfile = outfile;
 
  
  switch (nf.Type())  {
  case DOUBLE:
    T = new GobitModule<double>((NormalForm<double> &) nf, *outfile,
				*errfile, params.plev);
    break;
/*
 case nfRATIONAL:
    T = new GobitModule<Rational>((NFRep<Rational> &) *data, gout, gerr,0);
    break;
    */
  }
  T->Gobit(params.number);

  if (params.outfile != "")
    delete outfile;
  if (params.errfile != "" && params.errfile != params.outfile)
    delete errfile;

  delete T;
  return 1;
}


#if 0

void get_hess(double **p)
{
  int i1,i2,j1,j2,ii,iii;
  
  for(i1=1,ii=1;i1<=nplayers;i1++)
    for(j1=1;j1<=nstrats[i1];j1++)
      {
	for(i2=1,iii=1;i2<=nplayers;i2++)
	  for(j2=1;j2<=nstrats[i2];j2++)
	    {
	      if(iii<ii)dp2[ii][iii]=dp2[iii][ii];
	      else dp2[ii][iii]=deriv2_val(i1,j1,i2,j2,p);
	      iii++;
	    }
	ii++;
      }
  
}
#endif




