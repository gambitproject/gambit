//#
//# FILE: ngobit.cc -- Implementation of normal form Gobit algorithm
//#
//# $Id$
//#

#include <math.h>
#include "ngobit.h"
#include "gfunct.h"
#include "rational.h"

//-------------------------------------------------------------------------
//                     NFGobitParams<T>: Member functions
//-------------------------------------------------------------------------

template <class T> NFGobitParams<T>::NFGobitParams(gStatus &status_)
  : GobitParams<T>(status_)
{ }

template <class T>
NFGobitParams<T>::NFGobitParams(gOutput &out, gOutput &pxi, gStatus &status_)
  : GobitParams<T>(out, pxi, status_)
{ }

//-------------------------------------------------------------------------
//                   NFGobitFunc<T>: Class declaration
//-------------------------------------------------------------------------

template <class T>
class NFGobitFunc : public GobitFunc<T>, public gBC2FunctMin<T>  {
private:
  int niters, nevals;
  T Lambda;
  MixedProfile<T> p, pp;
  const Nfg<T> &N;
  gVector<T> **scratch1, **scratch2;
  
      // These three are inherited virtual functions from gBC2FunctMin<T>:
  int Deriv(const gVector<T> &v, gVector<T> &d);
  int Hess(const gVector<T> &v, gMatrix<T> &d);
  T Value(const gVector<T> &x);

      // Used in computation of Deriv() above
  T GobitDerivValue(int i, int j, const MixedProfile<T> &v);
  
public:
  NFGobitFunc(const Nfg<T> &NF, const GobitParams<T> &P);
  NFGobitFunc(const Nfg<T> &NF, const GobitParams<T> &P,
	      const MixedProfile<T> &s);
  virtual ~NFGobitFunc();
  
  void Init(void);
      // These two are inherited virtual functions from GobitFunc
  void Optimize(T Lambda, int &iter, T &value);
  void Output(gOutput &f, int format = 0) const;
  long NumIters(void) const;
  long NumEvals(void) const;
  const MixedProfile<T> &GetProfile(void) const;
};


//-------------------------------------------------------------------------
//               NFGobitFunc<T>: Constructor and destructor
//-------------------------------------------------------------------------

template <class T>
NFGobitFunc<T>::NFGobitFunc(const Nfg<T> &NF, const GobitParams<T> &P)
  : gBC2FunctMin<T>(NF.ProfileLength()), 
    niters(0), nevals(0), p(NF), pp(NF), N(NF)
{ 
  Init();
}

template <class T>
NFGobitFunc<T>::NFGobitFunc(const Nfg<T> &NF, const GobitParams<T> &P,
			    const MixedProfile<T> &s)
  : gBC2FunctMin<T>(NF.ProfileLength()), niters(0), nevals(0), 
    p(NF), pp(NF), N(NF)
{
  Init();
  pp = s;
}

template <class T> void NFGobitFunc<T>::Init(void)
{
  // Seems to me like this should be a parameter to the gfunct ctor?
  constrained = 1;
  scratch1 = new gVector<T> *[N.NumPlayers()] - 1;
  scratch2 = new gVector<T> *[N.NumPlayers()] - 1;
  for (int i = 1; i <= N.NumPlayers(); i++)  {
    scratch1[i] = new gVector<T>(N.NumStrats(i));
    scratch2[i] = new gVector<T>(N.NumStrats(i));
  }
}

template <class T> NFGobitFunc<T>::~NFGobitFunc()
{
  for (int i = 1; i <= N.NumPlayers(); i++)  {
    delete scratch1[i];
    delete scratch2[i];
  }

  delete (scratch1 + 1);
  delete (scratch2 + 1);
}

template <class T> const MixedProfile<T> &NFGobitFunc<T>::GetProfile(void) const
{
  return pp;
}

template <class T> long NFGobitFunc<T>::NumIters(void) const
{
  return niters;
}

template <class T> long NFGobitFunc<T>::NumEvals(void) const
{
  return nevals;
}

//-------------------------------------------------------------------------
//           NFGobitFunc<T>: Implementation of function members
//-------------------------------------------------------------------------

template <class T> int NFGobitFunc<T>::Hess(const gVector<T> &, gMatrix<T> &)
{
  return 1;
}

template <class T>
int NFGobitFunc<T>::Deriv(const gVector<T> &v, gVector<T> &d)
{
  ((gVector<T> &) p).operator=(v);
  T avg;
  
  for (int pl = 1, index = 1; pl <= N.NumPlayers(); pl++)  {
    avg = (T) 0;
    int nstrats = N.NumStrats(pl);
    for (int st = 1; st <= nstrats;
	 avg += (d[index++] = GobitDerivValue(pl, st++, p)));
    avg /= (T) nstrats;
    for (st = 1, index -= nstrats; st <= nstrats; st++, d[index++] -= avg);
  }

  return 1;
}

template <class T>
T NFGobitFunc<T>::GobitDerivValue(int i, int j, const MixedProfile<T> &v)
{
  T x((T) 0), dv;
  
  for (int pl = 1; pl <= N.NumPlayers(); pl++)  {
    gVector<T> &payoff = *scratch1[pl];
    v.Payoff(pl, pl, payoff);
    for (int st = 2; st <= N.NumStrats(pl); st++)  {
      dv =(T)log(v(pl, 1)) - (T)log(v(pl, st)) - Lambda * (payoff[1] - payoff[st]);
//      dv = log(v(pl, 1)/v(pl, st)) - Lambda * (payoff[1] - payoff[st]);
      if (pl == i)  {
	if (j == 1)              x += dv / v(pl, 1);
	else if (j == st)        x -= dv / v(pl, st);
      }
      else
	x -= dv * Lambda * (v.Payoff(pl, pl, 1, i, j) -
			    v.Payoff(pl, pl, st, i, j));
    }
  }

  return ((T) 2.0) * x; 
}

template <class T> T NFGobitFunc<T>::Value(const gVector<T> &v)
{
  nevals++;
  ((gVector<T> &) p).operator=(v);
  T val((T) 0), z;
  
  for (int pl = 1; pl <= N.NumPlayers(); pl++)  {
    gVector<T> &payoff = *scratch1[pl];
    p.Payoff(pl, pl, payoff);
    for (int st = 2; st <= N.NumStrats(pl); st++)  {
      z = (T)log(p(pl, 1)) - (T)log(p(pl, st)) - Lambda * (payoff[1] - payoff[st]);
//      z = log(p(pl, 1)/p(pl, st)) - Lambda * (payoff[1] - payoff[st]);
      val += z * z;
    }
  }

  return val;
}

//-------------------------------------------------------------------------
//            NFGobitFunc<T>: Implementation of gobit members
//-------------------------------------------------------------------------

template <class T> void NFGobitFunc<T>::Optimize(T Lam, int &iter, T &value)
{
  Lambda = Lam;
  DFP(pp, iter, value);
}


template <class T> void NFGobitFunc<T>::Output(gOutput &f,int format) const
{
      // Header information
	if(format==3) {
		f<<"Dimensionality:\n";
		N.WriteNfgFile(f);
	}
	else if(format==2) {
		int numcols = 2+N.ProfileLength();
		f<<"DataFormat:\n";
		f << numcols;
		for(int i=1;i<=numcols;i++) f << " " << i;
    f<<"\nData:\n";
		}
			// PXI output
	else if(format==1) {
		f<< " ";
		for (int pl = 1; pl <= N.NumPlayers(); pl++)
      for (int strat = 1; strat <= N.NumStrats(pl); strat++)  
	f << pp(pl,strat) << " ";
  }
  else  f << " pp = " << pp;
}


//-------------------------------------------------------------------------
//                  NFGobitModule<T>: Member functions
//-------------------------------------------------------------------------

template <class T>
NFGobitModule<T>::NFGobitModule(const Nfg<T> &NF, NFGobitParams<T> &p) 
  : GobitModule<T>(p), N(NF)
{ }

template <class T>NFGobitModule<T>
::NFGobitModule(const Nfg<T> &NF, NFGobitParams<T> &p, MixedProfile<T> &s)
  : GobitModule<T>(p,s), N(NF)
{ }

template <class T> NFGobitModule<T>::~NFGobitModule()
{ }

template <class T>
const gList<MixedProfile<T> > &NFGobitModule<T>::GetSolutions(void) const
{
  return solutions;
}

template <class T> GobitFunc<T> *NFGobitModule<T>::CreateFunc(void)
{

  if (start) {
    return new NFGobitFunc<T>(N, params, (MixedProfile<T> &) *start); 
  }
  return new NFGobitFunc<T>(N, params);

}

template <class T>
void NFGobitModule<T>::AddSolution(const GobitFunc<T> *const F)
{
  solutions.Append(MixedProfile<T>(N, ((NFGobitFunc<T> *) F)->GetProfile()));
}

#define BIGNUM 1.0e100

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class NFGobitParams<double>;
TEMPLATE class NFGobitFunc<double>;
TEMPLATE class NFGobitModule<double>;
