//#
//# FILE: liap.cc -- Liapunov module
//#
//# $Id$
//#

#include "gambitio.h"
#include "normal.h"
#include "nfrep.h"
#include "normiter.h"
#include "rational.h"
#include "mixed.h"
#include "solution.h"
#include "gfunct.h"

class BaseLiap {
public:
  virtual int Liap(int) = 0;  
  virtual int Nevals(void) = 0;
  virtual int Nits(void) = 0;
  virtual ~BaseLiap() {}
};

template <class T> class LiapModule 
: public gBFunct_nDim<T>, public BaseLiap, public SolutionModule {
private:
  const NFRep<T> &rep;
public:
  LiapModule(const NFRep<T> &N,gOutput &ofile,gOutput &efile,int plev) 
    :  SolutionModule(ofile,efile,plev),rep(N),
  gBFunct_nDim<T>(N.ProfileLength()){ SetPlev(plev);}
  int Liap(int) ;
  virtual ~LiapModule() {}
  T operator()(const gVector<T> &x) const;
  int Liap(MixedProfile<T> &p);
  int Nevals(void) {return nevals;}
  int Nits(void) {return nits;}
};



template <class T> int LiapModule<T>::Liap(int number)
{
  int n=0;
  MixedProfile<T> p(rep.Centroid());
  n+=Liap(p);
/*
  for(int i=1;i<number;i++)
    {
      p.Randomize();
      n+=Liap(p);
    }
  */
  return n;
};

template <class T> int LiapModule<T>::Liap(MixedProfile<T> &p)
{
// Ted -- we need a constructor in gvector to construct v from p,
// so we can replace all of this with dVector v(p);
// I wasn't sure where it would belong, since we don't want to include
// mixed.h in gvector.h
  
  gVector<T> v(rep.ProfileLength());
  int k=1;
  
  for(int i=1;i<=rep.NumPlayers();i++)
    {
      for(int j=1;j<=rep.NumStrats(i);j++) {
	v[k]=p[i][j];
	k++;
      }
    }
  if(plev>=3)
    gout << "\nv= " << v;
  MinPowell(v);
  gout << "\nv= " << v;
  return 1;
};

template <class T>  
T LiapModule<T>::operator()(const gVector<T> &x) const
{
  assert(x.Length()==rep.ProfileLength());

//  gout << "\nloc 7: x= " << x;
  
  MixedProfile<T> m(rep);
  
//  gout << "\nloc 8: x= " << x;
  int k=1;
  for(int i=1;i<=rep.NumPlayers();i++)
    {
      m[i] = gVector<T>(rep.NumStrats(i));
      for(int j=1;j<=rep.NumStrats(i);j++){
	m[i][j]=x[k];
	k++;
      }
    }
//  gout << "\nloc 9: m= " << m;
  return rep.LiapValue(m);
  
};

#define BIG1 ((T) 100)
#define BIG2 ((T) 100)

template <class T>  
T NFRep<T>::LiapValue(const MixedProfile<T> &p) const
{
  int i,j,num;
  MixedProfile<T> tmp(p);
  gVector<T> payoff;
  T x,result,avg,sum;
  
  result= (T) 0;
  for(i=1;i<=NumPlayers();i++) {
    payoff = gVector<T>(NumStrats(i));
    payoff=(T) 0;
    tmp[i]=(T) 0;
    avg=sum= (T) 0;
	// then for each strategy for that player set it to 1 and evaluate
    for(j = 1; j <= NumStrats(i); j++) {
      tmp[i][j]= (T) 1;
      x=p[i][j];
      payoff[j] = Payoff(i,tmp);
//      gout << "\np[" << i << "][" << j << "] = " << payoff[j];
//      gout << "\ntmp = " << tmp;
      avg+=x*payoff[j];
      sum+=x;
      x= (x > ((T) 0)  ? ((T) 0)  : x);
      result += BIG1*x*x;         // add penalty for neg probabilities
      tmp[i][j]= (T) 0;
    }
    tmp[i]=p[i];
    for(j=1;j<=NumStrats(i);j++) {
      x=payoff[j]-avg;
      x = (x > 0 ? x : 0);
      result += x*x;          // add penalty if not best response
    }
    x=sum - ((T) 1);
    result += BIG2*x*x ;          // add penalty for sum not equal to 1
  }
  return result;
}

int NormalForm::Liap(int number, int plev, gOutput &out, gOutput &err,
		   int &nevals, int &nits)
{
  BaseLiap *T;

  switch (type)  {
    case DOUBLE:   
      T = new LiapModule<double>((NFRep<double> &) *data, out, err, plev);
      break;
/*
    case RATIONAL:  
      T = new LiapModule<Rational>((NFRep<Rational> &) *data, out, err, plev);
      break; 
*/
  }
  T->Liap(number);
  nits=T->Nits();
  nevals= T->Nevals();
  return 1;

}






