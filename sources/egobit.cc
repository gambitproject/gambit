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

#include "behav.h"
#include "player.h"
#include "infoset.h"

//-------------------------------------------------------------------------
//                     EFGobitParams<T>: Member functions
//-------------------------------------------------------------------------

template <class T> EFGobitParams<T>::EFGobitParams(gStatus &status_)
  : GobitParams<T>(status_)
{ }

template <class T>
EFGobitParams<T>::EFGobitParams(gOutput &out, gOutput &pxi, gStatus &status_)
  : GobitParams<T>(out, pxi, status_)
{ }

//-------------------------------------------------------------------------
//                   EFGobitFunc<T>: Class declaration
//-------------------------------------------------------------------------

template <class T>
class EFGobitFunc : public GobitFunc<T>, public gBFunctMin<T>  {
  private:
    T Lambda;
    gPVector<T> probs;
    BehavProfile<T> p, pp, cpay;
    gMatrix<T> xi;
    const ExtForm<T> &E;
    
    // Inherited virtual function from gBFunctMin
    T Value(const gVector<T> &x);

  public:
    EFGobitFunc(const ExtForm<T> &EF, const GobitParams<T> &P);
    EFGobitFunc(const ExtForm<T> &EF, const GobitParams<T> &P,
	      const BehavProfile<T> &s);
    virtual ~EFGobitFunc();

    void Init(void);
    // These two are inherited virtual functions from GobitFunc
    void Optimize(T Lambda, int &iter, T &value);
    void Output(gOutput &f, int format = 0) const;
};

//-------------------------------------------------------------------------
//               EFGobitFunc<T>: Constructor and destructor
//-------------------------------------------------------------------------

template <class T>
EFGobitFunc<T>::EFGobitFunc(const ExtForm<T> &EF, const GobitParams<T> &P)
  :gBFunctMin<T>(EF.ProfileLength(true)),
                 probs(EF.Dimensionality().Lengths()),
		 p(EF, true), pp(EF, true), cpay(EF),
		 xi(p.Length(), p.Length()), E(EF)
{
  Init();
  E.Centroid(pp);
}

template <class T>
EFGobitFunc<T>::EFGobitFunc(const ExtForm<T> &EF, const GobitParams<T> &P,
			    const BehavProfile<T> &s)
  :gBFunctMin<T>(EF.ProfileLength(true)),
		 probs(EF.Dimensionality().Lengths()),
		 p(EF, true), pp(EF, true), cpay(EF),
		 xi(p.Length(), p.Length()), E(EF)
{
  Init();
  pp = s;
}

template <class T> void EFGobitFunc<T>::Init(void)
{
  // Seems to me like this should be a parameter to the gfunct ctor?
  constrained = 1;
  xi.MakeIdent();
}

template <class T> EFGobitFunc<T>::~EFGobitFunc()
{ }

//-------------------------------------------------------------------------
//           EFGobitFunc<T>: Implementation of function members
//-------------------------------------------------------------------------

template <class T> T EFGobitFunc<T>::Value(const gVector<T> &v)
{
  static const T PENALTY1 = (T) 10000.0;

  (gVector<T> &) p = v;
  T val((T) 0), prob, psum, z;

  E.CondPayoff(p, cpay, probs);

  for (int pl = 1; pl <= E.NumPlayers(); pl++)  {
    Player *player = E.PlayerList()[pl];
    for (int iset = 1; iset <= player->NumInfosets(); iset++)  {
      prob = (T) 0;
      psum = (T) 0;
      
      Infoset *s = player->InfosetList()[iset];
      for (int act = 1; act <= s->NumActions(); act++)  {
	z = Lambda * cpay(pl, iset, act);
	z = exp(z);
	psum += z;
	cpay(pl, iset, act) = z;
      }
      
      for (act = 1; act < s->NumActions(); act++)  {
	z = p(pl, iset, act);
	prob += z;
	if (z < (T) 0)
	  val += PENALTY1 * z * z;
	z -= cpay(pl, iset, act) / psum;
	val += z * z;
      }
      
      z = (T) 1 - prob;
      if (z < (T) 0)
	val += PENALTY1 * z * z;
      z -= cpay(pl, iset, act) / psum;
      val += z * z;

/*  This is obsoleted by the constraint that sum(prob) = 1  */
/*      z = (T) 1 - prob;
      val += PENALTY2 * z * z;  */
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

template <class T> void EFGobitFunc<T>::Output(gOutput &f, int format) const
{
	int pl, iset, act, nisets;

			// Header information
	if (format==3) {
		nisets=0;
		f<<"Dimensionality:\n";
		for (pl = 1; pl <= E.NumPlayers(); pl++)
			nisets += E.PlayerList()[pl]->NumInfosets();
		f << nisets;
		for (pl = 1; pl <= E.NumPlayers(); pl++)
			for (iset = 1; iset <= E.PlayerList()[pl]->NumInfosets(); iset++)
	f << " " << E.PlayerList()[pl]->InfosetList()[iset]->NumActions();
		f << "\n";
	}
	else if (format==2) {
		int numcols = 2+E.ProfileLength();
		f<<"DataFormat:";
		f << "\n" << numcols;
		for(int i=1;i<=numcols;i++) f << ' ' << i;
		f<<"\nData:\n";
	}
			// PXI output
	else if (format==1) {
		f<< " ";
		for (pl = 1; pl <= E.NumPlayers(); pl++)
			for (iset = 1; iset <= E.PlayerList()[pl]->NumInfosets(); iset++)  {
	T prob = (T) 0.0;
	for (act = 1; act < E.PlayerList()[pl]->InfosetList()[iset]->NumActions(); prob += pp(pl, iset, act++))
		f << pp(pl,iset,act) << ' ';
	f << ((T) 1 - prob) << ' ';
			}
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

template <class T>
EFGobitModule<T>::EFGobitModule(const ExtForm<T> &EF, EFGobitParams<T> &p,
				gDPVector<T> &s)
  : GobitModule<T>(p,s), E(EF)
{ }

template <class T> EFGobitModule<T>::~EFGobitModule()
{ }

template <class T> GobitFunc<T> *EFGobitModule<T>::CreateFunc(void)
{
  if (start) {
    BehavProfile<T> s(E, (gDPVector<T> &) *start);
    return new EFGobitFunc<T>(E, params, s); 
  }
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

