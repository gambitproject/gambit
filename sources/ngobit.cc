//#
//# FILE: ngobit.cc -- Implementation of normal form Gobit algorithm
//#
//# $Id$
//#

#include <math.h>
#include "ngobit.h"
#include "gfunct.h"

template <class T> NFGobitParams<T>::NFGobitParams(void)
{ }


//-------------------------------------------------------------------------
//                   NFGobitFunc<T>: Class declaration
//-------------------------------------------------------------------------

template <class T>
class NFGobitFunc : public GobitFunc<T>, public gBC2FunctMin<T>  {
  private:
    T Lambda, tolDFP;
    gPVector<T> p, pp;
    const NormalForm<T> &N;
    gVector<T> **scratch1, **scratch2;

    // These three are inherited virtual functions from gBC2FunctMin<T>:
    int Deriv(const gVector<T> &v, gVector<T> &d);
    int Hess(const gVector<T> &v, gMatrix<T> &d);
    T Value(const gVector<T> &x);

    // Used in computation of Deriv() above
    T GobitDerivValue(int i, int j, const gPVector<T> &v);

  public:
    NFGobitFunc(const NormalForm<T> &NF, const T &tol);
    virtual ~NFGobitFunc();

    // These two are inherited virtual functions from GobitFunc
    void Optimize(T Lambda, int &iter, T &value);
    void Output(gOutput &f) const;
};


//-------------------------------------------------------------------------
//               NFGobitFunc<T>: Constructor and destructor
//-------------------------------------------------------------------------

template <class T>
NFGobitFunc<T>::NFGobitFunc(const NormalForm<T> &NF, const T &tol)
  : gBC2FunctMin<T>(NF.ProfileLength()), N(NF), p(NF.Dimensionality()),
    pp(NF.Dimensionality()), tolDFP(tol)
{
  // Seems to me like this should be a parameter to the gfunct ctor?
  constrained = 1;
  N.Centroid(pp);
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
  p = v;
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
T NFGobitFunc<T>::GobitDerivValue(int i, int j, const gPVector<T> &v)
{
  T x((T) 0), dv;
  
  for (int pl = 1; pl <= N.NumPlayers(); pl++)  {
    gVector<T> &payoff = *scratch1[pl];
    N.Payoff(pl, pl, v, payoff);
    for (int st = 2; st <= N.NumStrats(pl); st++)  {
      dv = log(v(pl, 1)) - log(v(pl, st)) - Lambda * (payoff[1] - payoff[st]);
      if (pl == i)  {
	if (j == 1)              x += dv / v(pl, 1);
	else if (j == st)        x -= dv / v(pl, st);
      }
      else
	x -= dv * Lambda * (N.Payoff(pl, pl, 1, i, j, v) -
			    N.Payoff(pl, pl, st, i, j, v));
    }
  }

  return ((T) 2.0) * x; 
}

template <class T> T NFGobitFunc<T>::Value(const gVector<T> &v)
{
  p = v;
  T val((T) 0), z;
  
  for (int pl = 1; pl <= N.NumPlayers(); pl++)  {
    gVector<T> &payoff = *scratch1[pl];
    N.Payoff(pl, pl, p, payoff);
    for (int st = 2; st <= N.NumStrats(pl); st++)  {
      z = log(p(pl, 1)) - log(p(pl, st)) - Lambda * (payoff[1] - payoff[st]);
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
  DFP(pp, tolDFP, iter, value);
}


template <class T> void NFGobitFunc<T>::Output(gOutput &f) const
{
  f << " p = " << p;
}


//-------------------------------------------------------------------------
//                  NFGobitModule<T>: Member functions
//-------------------------------------------------------------------------

template <class T>
NFGobitModule<T>::NFGobitModule(const NormalForm<T> &NF, NFGobitParams<T> &p)
  : GobitModule<T>(p), N(NF)
{ }

template <class T> NFGobitModule<T>::~NFGobitModule()
{ }

template <class T> GobitFunc<T> *NFGobitModule<T>::CreateFunc(void)
{
  return new NFGobitFunc<T>(N, params.tolOpt);
}


#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class NFGobitParams<double>;
TEMPLATE class NFGobitFunc<double>;
TEMPLATE class NFGobitModule<double>;
