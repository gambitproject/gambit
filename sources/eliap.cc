//#
//# FILE: eliap.cc -- Extensive Form Liapunov module
//#
//# $Id$
//#

#define MAXIT 10
#define TOL (T)(.00000001)

#include <math.h>
#include "eliap.h"
#include "gfunct.h"
#include "gdpvect.h"
#include "gmatrix.h"
#include "rational.h"

#include "behav.h"
#include "player.h"
#include "infoset.h"

//-------------------------------------------------------------------------
//                     EFLiapParams<T>: Member functions
//-------------------------------------------------------------------------

template <class T> EFLiapParams<T>::EFLiapParams(void)
{ }

//-------------------------------------------------------------------------
//                      EFLiapFunc<T>: Class definition
//-------------------------------------------------------------------------

template <class T>
class EFLiapFunc : public LiapFunc<T>, public gBFunctMin<T>   {
  private:
    int niters, nevals;
    const ExtForm<T> &E;
    BehavProfile<T> p, pp;
    gDPVector<T> cpay;
    gMatrix<T> xi;

    T Value(const gVector<T> &x);

  public:
    EFLiapFunc(const ExtForm<T> &EF, const LiapParams<T> &P); 
    EFLiapFunc(const ExtForm<T> &EF, const LiapParams<T> &P,
	     const BehavProfile<T> &s); 
    virtual ~EFLiapFunc();

    int Optimize(int &iter, T &value);
    void Output(gOutput &f) const;

    void Randomize(void);
    int NumIters(void) const;
    int NumEvals(void) const;

    const BehavProfile<T> &GetProfile(void) const;
};

//----------------------------------------------------------------------
//               EFLiapFunct<T>: Constructor and destructor
//----------------------------------------------------------------------

template <class T>EFLiapFunc<T>
::EFLiapFunc(const ExtForm<T> &EF, const LiapParams<T> &P)
  : gBFunctMin<T>(EF.ProfileLength()), E(EF), p(EF, false),
		  pp(EF, false), cpay(EF.Dimensionality()),
		  xi(p.Length(),p.Length()),
		  niters(0), nevals(0)
{
  xi.MakeIdent();
  E.Centroid(pp);
}

template <class T>EFLiapFunc<T>
::EFLiapFunc(const ExtForm<T> &EF, const LiapParams<T> &P,
			  const BehavProfile<T> &s)
  : gBFunctMin<T>(EF.ProfileLength()), E(EF), p(EF, false),
		  pp(EF,false), cpay(EF.Dimensionality()),
		  xi(p.Length(),p.Length()),
		  niters(0), nevals(0)
{
  xi.MakeIdent();
  pp = s;
}

template <class T> EFLiapFunc<T>::~EFLiapFunc()
{ }


template <class T> const BehavProfile<T> &EFLiapFunc<T>::GetProfile(void) const
{
  return pp;
}

template <class T> void EFLiapFunc<T>::Output(gOutput &f) const
{
  f << "\np = " << pp;
 }

template <class T> int EFLiapFunc<T>::NumIters(void) const
{
  return niters;
}

template <class T> int EFLiapFunc<T>::NumEvals(void) const
{
  return nevals;
}

template <class T> int EFLiapFunc<T>::Optimize(int &iter, T &value)
{
  return Powell(pp, xi, iter, value);
}

template <class T> void EFLiapFunc<T>::Randomize(void)
{
  T sum, tmp;

  for (int i = 1; i <= E.NumPlayers(); i++) {
    Player *player = E.PlayerList()[i];
    for(int j=1;j<=player->NumInfosets();j++) {
      sum = (T) 0;
      Infoset *s = player->InfosetList()[j];
      for (int k = 1; k < s->NumActions(); k++)  {
	do
	  tmp = (T) Uniform();
	while (tmp + sum > (T) 1);
	pp(i,j,k) = tmp;
	sum += tmp;
      }
      
      pp(i,j,k) = (T) 1 - sum;
    }
  }
}

template <class T> T EFLiapFunc<T>::Value(const gVector<T> &v)
{
  static const T BIG1 = (T) 10000.0;
  static const T BIG2 = (T) 100.0;

  nevals++;

  (gVector<T> &) p = v;
  T x, result((T) 0), avg, sum;

      // Ted -- only reason for this is because you 
      // got rid of CondPayoff ( . , . )
  gPVector<T> probs(E.Dimensionality().Lengths());  
  E.CondPayoff(p,cpay,probs);

//  gout << "\nv = " << v << "\np = " << p << "\ncpay = " << cpay;
  
  for(int i = 1; i <= E.NumPlayers(); i++) {
    Player *player = E.PlayerList()[i];
    for(int j=1;j<=player->NumInfosets();j++) {
      avg = sum = (T) 0;
      Infoset *s = player->InfosetList()[j];
      for (int k = 1; k <= s->NumActions(); k++) {
	x = p(i, j, k); 
	avg += (x * cpay(i, j, k));
	sum += x;
	if(x > (T)0 ) x = (T)0;
	result += BIG1*x*x;         // add penalty for neg probabilities
      }
      for(k=1; k<=s->NumActions(); k++) {
	x=cpay(i,j,k)-avg;
	if(x < (T)0 ) x = (T)0;
	result += x*x;          // add penalty if not best response
      }
      x=sum - ((T) 1);
      result += BIG2*x*x ;          // add penalty for sum not equal to 1
    }
  }
  return result;

}

//------------------------------------------------------------------------
//                    EFLiapModule<T>: Member functions
//------------------------------------------------------------------------

template <class T> EFLiapModule<T>
::EFLiapModule(const ExtForm<T> &E, EFLiapParams<T> &p)
  : LiapModule<T>(p), E(E)
{ }

template <class T> EFLiapModule<T>
::EFLiapModule(const ExtForm<T> &E, EFLiapParams<T> &p,gDPVector<T> &s)
  : LiapModule<T>(p, s), E(E)
{ }

template <class T> EFLiapModule<T>::~EFLiapModule()
{ }

template <class T>
const gList<BehavProfile<T> > &EFLiapModule<T>::GetSolutions(void) const
{
  return solutions;
}

template <class T> LiapFunc<T> *EFLiapModule<T>::CreateFunc(void)
{
  if(start) {
    BehavProfile<T> s(E, (gDPVector<T> &) *start);
//    s=*start;
    return new EFLiapFunc<T>(E, params, s); 
  }
  return new EFLiapFunc<T>(E, params);
}

template <class T>
void EFLiapModule<T>::AddSolution(const LiapFunc<T> *const F)
{
  solutions.Append(((EFLiapFunc<T> *) F)->GetProfile());
}

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class EFLiapParams<double>;
TEMPLATE class EFLiapModule<double>;
TEMPLATE class EFLiapFunc<double>;


