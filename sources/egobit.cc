//#
//# FILE: egobit.cc -- Gobit module
//#
//# $Id$
//#

#ifdef __GNUG__
#pragma implementation "egobit.h"
#endif   // __GNUG__

#include "gambitio.h"
#include "extform.h"
#include "rational.h"
#include "gmatrix.h"
#include "gpvector.h"
#include "gdpvect.h"
#include "solution.h"
#include "gfunct.h"
#include "egobit.h"

#define PENALTY1 10000.0
#define PENALTY2 100.0

ExtGobitParams::ExtGobitParams(void) 
  : plev(0), nequilib(1), minLam(.01), maxLam(30),delLam(1.01), 
    tolPOW(1.0e-10)
{ }

class BaseExtGobit {
public:
  virtual int Gobit(int) = 0;
  virtual int Nevals(void) = 0;
  virtual int Nits(void) = 0;
  virtual ~BaseExtGobit() {}
};

template <class T> class ExtGobitModule
: public gBFunctMin<T>, public BaseExtGobit, public SolutionModule {
private:
  const ExtForm<T> &rep;
  T Lambda;
  gDPVector<T> p;
  ExtGobitParams params;
  int maxits,liap_its;
  
public:
  ExtGobitModule(const ExtForm<T> &N,gOutput &ofile,gOutput &efile,
	      const ExtGobitParams &params)
    :  SolutionModule(ofile,efile,params.plev), rep(N), Lambda(0),
  p(rep.Dimensionality()), params(params), liap_its(0),
  maxits(500), gBFunctMin<T>(N.ProfileLength()){ constrained=0;
					       SetPlev(params.plev);}
  virtual ~ExtGobitModule() {}
  
  T Value(const gVector<T> &x);
  void Foo(void);
//  int Deriv(const gVector<T> &p, gVector<T> &d);
//  int Hess(const gVector<T> &p, gMatrix<T> &d) {return 1;}
  int Gobit(int);

//  T GobitDerivValue(int i1, int j1, int k1, const gDPVector<T> &p) const;
  int Nevals(void) {return nevals;}
  int Nits(void) {return nits;}

};

template <class T> int ExtGobitModule<T>::Gobit(int number)
{
  int i,j,k,ndim=p.Length();
  gMatrix<T> xi(ndim,ndim);
  gDPVector<T> pp(rep.Dimensionality());

  for(i=1;i<=rep.NumPlayers();i++)
    for(j=1;j<=rep.NumInfosets(1,i);j++)
      for(k=1;k<=rep.NumActions(1,i,j);k++)
	pp(i,j,k)=((T)(1)/(T)(rep.NumActions(1,i,j)));
  
  for(i=1;i<=ndim;i++)
    for(j=1;j<=ndim;j++) {
      xi(i,j)=(T)(0);
      if(i==j)xi(i,j)=(T)(1);
    }
  
  int iter=0;
  
  Lambda=(T) (params.minLam);
  T value;
  
  int nit=0;
  while(nit < maxits && Lambda<=(T)(params.maxLam)) {
    nit++;
    Powell(pp,xi,(T)(params.tolPOW),iter,value);
//    DFP(p, (T)(params.tolDFP), iter, value);

    gout << "\nLam = " << Lambda << " nits= " << iter;
    gout << " val = " << value << " pp = " << pp;
    gout << " evals = " << liap_its;
    if(value>=10.0)return nit;
    Lambda = Lambda * ((T)(params.delLam));
  }
  return nit;
};


#define BIG1 ((T) 100)
#define BIG2 ((T) 100)


template <class T> inline
T ExtGobitModule<T>::Value(const gVector<T> &v)
{
//  gout << " in ExtGobitModule::Operator()";
  assert(v.Length()==p.Length());
  
  nevals++;
  p=v;
  int i,j;
  T val,psum,prob,z;
  
  gDPVector<T> c_pay(rep.Dimensionality());
  rep.CondPayoff(p,c_pay);

  liap_its++;

  val=(T)(0);
  for(i=1;i<=rep.NumPlayers();i++) 
    for(int k=1;k<=rep.NumInfosets(1,i);k++) {
      prob=(T)(0);
      psum=(T)(0);
      for(j=1;j<=rep.NumActions(1,i,k);j++) {
	z=Lambda*c_pay(i,k,j);
	assert(z < (T)(600));
	z=exp(z);
	psum+=z;
	c_pay(i,k,j)=z;
      }
      for(j=1;j<=rep.NumActions(1,i,k);j++) {
	z=p(i,k,j);
	prob+=z;
	if(z<(T)(0))
	  val+=(PENALTY1*z*z);
	z-=(c_pay(i,k,j)/psum);
	val+=(z*z);
      }
      
/*
  psum=p(i,k,1);
      z=psum;
      if(z<(T)(0))val+=(PENALTY1*z*z);
      for(j=2;j<=rep.NumActions(1,i,k);j++) {
	z=log(p(i,k,1))-log(p(i,k,j))
	  - Lambda*(c_pay(i,k,1)-c_pay(i,k,j));
	val+=(z*z);
	z=p(i,k,j);
	if(z<(T)(0))val+=(PENALTY1*z*z);
	psum+=z;
      }
      */
      
      z=(T)(1)-prob;
      val+=(PENALTY2*z*z);
    }

//  gout << "\np =     " << p << "\n c_pay = " << c_pay << " val = " << val;

  return val;
};

#ifdef __GNUG__
template class ExtGobitModule<double>;
template class ExtGobitModule<gRational>;
#elif defined __BORLANDC__
#pragma option -Jgd
class ExtGobitModule<double>;
class ExtGobitModule<gRational>;
#pragma option -Jgx
#endif   // __GNUG__, __BORLANDC__

#include "gwatch.h"

int ExtGobitSolver::Gobit(void)
{
  BaseExtGobit *T;
  gOutput *outfile = &gout, *errfile = &gerr;
  
  if (params.outfile != "")
    outfile = new gFileOutput((char *) params.outfile);
  if (params.errfile != "" && params.errfile != params.outfile)
    errfile = new gFileOutput((char *) params.errfile);
  if (params.errfile != "" && params.errfile == params.outfile)
    errfile = outfile;
  
  gWatch watch;
  
  switch (ef.Type())  {
    case DOUBLE:
      T = new ExtGobitModule<double>((ExtForm<double> &) ef, *outfile,
				     *errfile, params);
      break;
    case RATIONAL:
      T = new ExtGobitModule<gRational>((ExtForm<gRational> &) ef,
					*outfile, *errfile, params);
      break;
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



