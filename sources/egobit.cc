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
#include "efplayer.h"
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
  long niters, nevals;
  T Lambda;
  gPVector<T> probs;
  BehavProfile<T> p, pp, cpay;
  gMatrix<T> xi;
  const Efg<T> &E;
  gVector<T> ***scratch;
    
      // Inherited virtual function from gBFunctMin
  T Value(const gVector<T> &x);
  
public:
  EFGobitFunc(const Efg<T> &EF, const GobitParams<T> &P,
	      const BehavProfile<T> &s);
    virtual ~EFGobitFunc();
  
  void Init(void);
      // These two are inherited virtual functions from GobitFunc
  void Optimize(T Lambda, int &iter, T &value);
  void Output(gOutput &f, int format = 0) const;
  long NumIters(void) const;
  long NumEvals(void) const;
  
  const BehavProfile<T> &GetProfile(void) const;
};

//-------------------------------------------------------------------------
//               EFGobitFunc<T>: Constructor and destructor
//-------------------------------------------------------------------------

template <class T>
EFGobitFunc<T>::EFGobitFunc(const Efg<T> &EF, const GobitParams<T> &,
			    const BehavProfile<T> &start)
  :gBFunctMin<T>(EF.ProfileLength(true)), niters(0), nevals(0), 
   probs(EF.Dimensionality().Lengths()),
   p(EF, true), pp(EF, true), cpay(EF),
   xi(p.Length(), p.Length()), E(EF)
{
  Init();
  for (int pl = 1; pl <= E.NumPlayers(); pl++)  {
    EFPlayer *p = E.PlayerList()[pl];
    for (int iset = 1; iset <= p->NumInfosets(); iset++)  {
      Infoset *s = p->InfosetList()[iset];
      for (int act = 1; act < s->NumActions(); act++)  {
	pp(pl, iset, act) = start(pl, iset, act);
      }
	 }
  }
}

template <class T> void EFGobitFunc<T>::Init(void)
{
  // Seems to me like this should be a parameter to the gfunct ctor?
  constrained = 1;
  xi.MakeIdent();
  scratch = new gVector<T> **[E.NumPlayers()] -1;
  for(int i=1;i<=E.NumPlayers();i++) {
	 int nisets = (E.PlayerList()[i])->NumInfosets();
	 scratch[i] = new gVector<T> *[nisets+1] -1 ;
	 for(int j=1;j<=nisets;j++) {
		scratch[i][j] = new gVector<T>(pp.GetEFSupport().NumActions(i,j));
	 }
  }
}

template <class T> EFGobitFunc<T>::~EFGobitFunc()
{
  for(int i=1;i<=E.NumPlayers();i++) {
	 int nisets = (E.PlayerList()[i])->NumInfosets();
	 for(int j=1;j<=nisets;j++) {
		delete scratch[i][j];
	 }
	 delete [] (scratch[i]+1);
  }
  delete [] (scratch + 1);
}

//-------------------------------------------------------------------------
//           EFGobitFunc<T>: Implementation of function members
//-------------------------------------------------------------------------

template <class T> T EFGobitFunc<T>::Value(const gVector<T> &v)
{
  static const T PENALTY1 = (T) 10000.0;

  nevals++;
  ((gVector<T> &) p).operator=(v);
  T val((T) 0), prob, psum, z;
//  gVector<T> &payoff = *scratch
  p.CondPayoff(cpay, probs);

  for (int pl = 1; pl <= E.NumPlayers(); pl++)  {
    EFPlayer *player = E.PlayerList()[pl];
    for (int iset = 1; iset <= player->NumInfosets(); iset++)  {
      prob = (T) 0;
      psum = (T) 0;
      
      Infoset *s = player->InfosetList()[iset];
      int act;
      for (act = 1; act <= s->NumActions(); act++)  {
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

template <class T> const BehavProfile<T> &EFGobitFunc<T>::GetProfile(void) const
{
  return pp;
}

template <class T> long  EFGobitFunc<T>::NumIters(void) const
{
  return niters;
}

template <class T> long EFGobitFunc<T>::NumEvals(void) const
{
  return nevals;
}


//------------------------------------------------------------------------
//                  EFGobitModule<T>: Member functions
//------------------------------------------------------------------------

template <class T>
EFGobitModule<T>::EFGobitModule(const Efg<T> &EF, EFGobitParams<T> &p,
				BehavProfile<T> &s)
  : GobitModule<T>(p), E(EF), start(s)
{ }

template <class T> EFGobitModule<T>::~EFGobitModule()
{ }

template <class T>
const gList<BehavProfile<T> > &EFGobitModule<T>::GetSolutions(void) const
{
  return solutions;
}

template <class T> GobitFunc<T> *EFGobitModule<T>::CreateFunc(void)
{
  if(E.ProfileLength(true)) 
    return new EFGobitFunc<T>(E, params, start);
  return 0;
}

template <class T>
void EFGobitModule<T>::AddSolution(const GobitFunc<T> *const F)
{
  BehavProfile<T> foo(((EFGobitFunc<T> *) F)->GetProfile());

  BehavProfile<T> bar(E);
  for (int pl = 1; pl <= E.NumPlayers(); pl++)   {
    for (int iset = 1; iset <= E.PlayerList()[pl]->NumInfosets(); iset++)  {
      T accum = (T) 0;
      int act;
      for (act = 1; act < E.PlayerList()[pl]->InfosetList()[iset]->NumActions(); act++)  {
	bar(pl, iset, act) = foo(pl, iset, act);
	accum += foo(pl, iset, act);
      }
      bar(pl, iset, act) = (T) 1 - accum;
    }
  }

  solutions.Append(bar);
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

