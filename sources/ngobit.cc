//#
//# FILE: gobit.cc -- Gobit module
//#
//# $Id$
//#

#ifdef __GNUG__
#pragma implementation "ngobit.h"
#endif   // __GNUG__

#include "gambitio.h"
#include "normal.h"
#include "normiter.h"
#include "rational.h"
#include "gmatrix.h"
#include "gpvector.h"
#include "solution.h"
#include "gfunct.h"
#include "gwatch.h"
#include "ngobit.h"

GobitParams::GobitParams(void) 
  : plev(0), nequilib(1), minLam(.01), maxLam(30),delLam(1.01), 
    tolDFP(1.0e-10)
{ }

class BaseGobit {
public:
  virtual int Gobit(int) = 0;
  virtual int Nevals(void) = 0;
  virtual int Nits(void) = 0;
  virtual ~BaseGobit() {}
};

template <class T> class GobitModule
: public gBC2FunctMin<T>, public BaseGobit, public SolutionModule {
private:
  const NormalForm<T> &N;
  T Lambda;
  gPVector<T> p;
  gTuple<gVector<T> *> scratch;
  GobitParams params;
  int maxits;
  
public:
  GobitModule(const NormalForm<T> &NF,gOutput &ofile,gOutput &efile,
	      const GobitParams &params)
    :  SolutionModule(ofile,efile,params.plev), N(NF), Lambda(0),
  p(NF.Dimensionality()), params(params),
  maxits(500), gBC2FunctMin<T>(NF.ProfileLength()),
  scratch(NF.NumPlayers())  { constrained=1;}
  virtual ~GobitModule() {}
  
  T Value(const gVector<T> &x);
  int Deriv(const gVector<T> &p, gVector<T> &d);
  int Hess(const gVector<T> &p, gMatrix<T> &d) {return 1;}
  int Gobit(int);

  T GobitDerivValue(int i1, int j1, const gPVector<T> &p) const;
  int Nevals(void) {return nevals;}
  int Nits(void) {return nits;}

};

template <class T> int GobitModule<T>::Gobit(int number)
{
  gPVector<T> pp(N.Dimensionality());
  N.Centroid(pp);

  for (int pl = 1; pl <= N.NumPlayers(); pl++)
    scratch[pl] = new gVector<T>(N.NumStrats(pl));

  int iter=0;
  
  Lambda=(T) (params.minLam);
  T value;
  
  int nit=0;
  while(nit < maxits && Lambda<=(T)(params.maxLam)) {
    nit++;
    DFP(pp, (T)(params.tolDFP), iter, value);
    output << "\nLam = " << Lambda << " nits= " << iter;
    output << " val = " << value << " p = " << p;
    output << " evals = " << Nevals();
    if(value>=10.0)return nit;
    Lambda *= ((T)(params.delLam));
  }

  for (pl = 1; pl <= N.NumPlayers(); pl++)
    delete scratch[pl];

  return nit;
};


template <class T> T GobitModule<T>::
GobitDerivValue(int i1, int j1, const gPVector<T> &p) const
{
  int i, j;
  T x((T) 0), x1,dv;
  
  for(i=1;i<=N.NumPlayers();i++) {
    gVector<T> *payoffs = scratch[i];
    N.Payoff(i, i, p, *payoffs);

    for(j=2;j<=N.NumStrats(i);j++) {
      dv=log(p(i,1))-log(p(i,j))
//	- Lambda*(N.Payoff(i, i, 1, p)-N.Payoff(i,i,j,p));
	- Lambda * ((*payoffs)[1] - (*payoffs)[j]);
      if(i==i1) {
	if(j1==1)  x+=dv/p(i,1);
	if(j1==j)  x-=dv/p(i,j);
      }
      if(i!=i1)
	x-=dv*Lambda*(N.Payoff(i,i,1,i1,j1,p)-N.Payoff(i,i,j,i1,j1,p));

    }
  }
//  if(p(i1,j1)<(T)(0))x+=p(i1,j1);
  
  return ((T)(2))*x;
}


template <class T> T GobitModule<T>::Value(const gVector<T> &v)
{
  p=v;
  int i,j;
  T val,psum,z;
  
  val=(T)(0);
//  gout << "\nValue(): p = " << p;
  for(i=1;i<=N.NumPlayers();i++) {
//    psum=p(i,1);
    gVector<T> *payoffs = scratch[i];
    N.Payoff(i, i, p, *payoffs);

    for(j=2;j<=N.NumStrats(i);j++) {
      z=log(p(i,1))-log(p(i,j))
//	- Lambda*(N.Payoff(i,i,1,p)-N.Payoff(i,i,j,p));
	- Lambda*((*payoffs)[1] - (*payoffs)[j]);
      val+=(z*z);
//      if(p(i,j)<(double)0.0)v+=(p(i,j)*p(i,j));
//      psum+=p(i,j);
    }
//    z=(T)(1)-psum;
//    val+=(z*z);
  }
  return val;
};

template <class T>
int GobitModule<T>::Deriv(const gVector<T> &v, gVector<T> &d)
{
  p = v;

  T avg;
  
  for (int pl = 1, index = 1; pl <= N.NumPlayers(); pl++)  {
    avg = (T) 0;
    int nstrats = N.NumStrats(pl);
    for (int st = 1; st <= nstrats;
	 avg += (d[index++] = GobitDerivValue(pl, st++, p)));
    avg /= (T) nstrats;
    for (st = 1, index -= nstrats; st <= nstrats;
	 st++, d[index++] -= avg);
  }
  return 1;
};

template class GobitModule<double>;

int GobitSolver::Gobit(void)
{
  BaseGobit *T;
  gOutput *outfile = &gout, *errfile = &gerr;
  
  if (params.outfile != "")
    outfile = new gFileOutput((char *) params.outfile);
  if (params.errfile != "" && params.errfile != params.outfile)
    errfile = new gFileOutput((char *) params.errfile);
  if (params.errfile != "" && params.errfile == params.outfile)
    errfile = outfile;
  
  gWatch watch;
  
  switch (nf.Type())  {
  case DOUBLE:
    T = new GobitModule<double>((NormalForm<double> &) nf, *outfile,
				*errfile, params);
    break;
/*
 case RATIONAL:
    T = new GobitModule<Rational>((NFRep<Rational> &) *data, gout, gerr,0);
		break;
    */
  }
  T->Gobit(params.nequilib);
  
  time = watch.Elapsed();
  
  if (params.outfile != "")
    delete outfile;
  if (params.errfile != "" && params.errfile != params.outfile)
		delete errfile;
  
  delete T;
  return 1;
}


#include "gblock.imp"
#include "gtuple.imp"

template class gBlock<gVector<double> *>;
template class gTuple<gVector<double> *>;
