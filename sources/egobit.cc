//#
//# FILE: egobit.cc -- Implementation of extensive form Gobit algorithm
//#
//# $Id$
//#

#include <math.h>
#include "egobit.h"
#include "gfunct.h"
#include "gdpvect.h"
#include "gmatrix.h"

template <class T> EFGobitParams<T>::EFGobitParams(void)
{ }


//-------------------------------------------------------------------------
//                   EFGobitFunc<T>: Class declaration
//-------------------------------------------------------------------------

template <class T>
class EFGobitFunc : public GobitFunc<T>, public gBFunctMin<T>  {
  private:
    T Lambda;
    gPVector<T> probs;
    gDPVector<T> p, pp, cpay;
    gMatrix<T> xi;
    const ExtForm<T> &E;
    
    // Inherited virtual function from gBFunctMin
    T Value(const gVector<T> &x);

  public:
    EFGobitFunc(const ExtForm<T> &EF, const GobitParams<T> &P);
    virtual ~EFGobitFunc();

    // These two are inherited virtual functions from GobitFunc
    void Optimize(T Lambda, int &iter, T &value);
    void Output(gOutput &f,int format) const;
};

//-------------------------------------------------------------------------
//               EFGobitFunc<T>: Constructor and destructor
//-------------------------------------------------------------------------

template <class T>
EFGobitFunc<T>::EFGobitFunc(const ExtForm<T> &EF, const GobitParams<T> &P)
  :gBFunctMin<T>(EF.ProfileLength(),P.tolOpt,P.maxitsOpt,
		 P.tolBrent,P.maxitsBrent), E(EF), p(EF.Dimensionality()),
		 pp(EF.Dimensionality()), cpay(EF.Dimensionality()),
		 probs(EF.Dimensionality().Lengths()),
		 xi(p.Length(), p.Length())
{
  constrained = 1;
  
  // This can later be replaced by ExtForm<T>::Centroid(), when written
  for (int i = 1; i <= E.NumPlayers(); i++)
    for (int j = 1; j <= E.NumInfosets(1, i); j++)
      for (int k = 1; k <= E.NumActions(1, i, j); k++)
	pp(i, j, k) = ((T) 1 / (T) E.NumActions(1, i, j));

  for (i = 1; i <= p.Length(); i++)
    for (int j = 1; j <= p.Length(); j++)  {
      xi(i, j) = (T) 0;
      if (i == j)    xi(i, j) = (T) 1;
    }
}

template <class T> EFGobitFunc<T>::~EFGobitFunc()
{ }

//-------------------------------------------------------------------------
//           EFGobitFunc<T>: Implementation of function members
//-------------------------------------------------------------------------

template <class T> T EFGobitFunc<T>::Value(const gVector<T> &v)
{
  static const T PENALTY1 = (T) 10000.0;
  static const T PENALTY2 = (T) 100.0;

  p = v;
  T val((T) 0), prob, psum, z;

  E.CondPayoff(p, cpay,probs);
  
  for (int pl = 1; pl <= E.NumPlayers(); pl++)  {
    for (int iset = 1; iset <= E.NumInfosets(1, pl); iset++)  {
      prob = (T) 0;
      psum = (T) 0;
      
      for (int act = 1; act <= E.NumActions(1, pl, iset); act++)  {
	z = probs(pl,iset) * Lambda * cpay(pl, iset, act);
	assert(z < (T) 600);
	z = exp(z);
	psum += z;
	cpay(pl, iset, act) = z;
      }
      
      for (act = 1; act <= E.NumActions(1, pl, iset); act++)  {
	z = p(pl, iset, act);
	prob += z;
	if (z < (T) 0)
	  val += PENALTY1 * z * z;
	z -= cpay(pl, iset, act) / psum;
	val += z * z;
      }
      
      z = (T) 1 - prob;
      val += PENALTY2 * z * z;
    }
  }

  return val;
};
  

//------------------------------------------------------------------------
//            EFGobitFunc<T>: Implementation of gobit members
//------------------------------------------------------------------------

template <class T> void EFGobitFunc<T>::Optimize(T Lam, int &iter, T &value)
{
  Lambda = Lam;
  Powell(pp, xi, iter, value);
}

template <class T> void EFGobitFunc<T>
::Output(gOutput &f, int format = 0) const
{
  int pl, iset, act,nisets;

      // Header information
  if(format==3) {
    nisets=0;
    for (pl = 1; pl <= E.NumPlayers(); pl++)  
      for (iset = 1; iset <= E.NumInfosets(1, pl); iset++)
	nisets++;
    f << nisets;
    for (pl = 1; pl <= E.NumPlayers(); pl++)  
      for (iset = 1; iset <= E.NumInfosets(1, pl); iset++)  
	f << " " << E.NumActions(1,pl,iset);
    f << "\n";
  }
  else if(format==2) {
    int numcols = 2+E.ProfileLength();
    f << "\n" << numcols;
    for(int i=1;i<=numcols;i++) f << " " << i;
  }
      // PXI output
  else if(format==1) {
    f<< " ";
    for (pl = 1; pl <= E.NumPlayers(); pl++)  
      for (iset = 1; iset <= E.NumInfosets(1, pl); iset++)  
	for(act = 1;act <= E.NumActions(1,pl,iset);act++)
	  f << pp(pl,iset,act) << " ";
  }
  else  f << " pp = " << pp;

}

//------------------------------------------------------------------------
//                  EFGobitModule<T>: Member functions
//------------------------------------------------------------------------

template <class T>
EFGobitModule<T>::EFGobitModule(const ExtForm<T> &EF, EFGobitParams<T> &p)
  : GobitModule<T>(p), E(EF)
{ }

template <class T> EFGobitModule<T>::~EFGobitModule()
{ }

template <class T> GobitFunc<T> *EFGobitModule<T>::CreateFunc(void)
{
  return new EFGobitFunc<T>(E, params);
}


#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class EFGobitParams<double>;
TEMPLATE class EFGobitFunc<double>;
TEMPLATE class EFGobitModule<double>;


