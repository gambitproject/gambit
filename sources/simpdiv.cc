//#
//# FILE: simpdiv.cc -- Mixed strategy algorithm for Gambit
//#
//# $Id$
//#

//
// simpdiv is a simplicial subdivision algorithm with restart, for finding
// mixed strategy solutions to general finite n-person games.  It is based on
// van Der Laan, Talman and van Der Heyden, Math in Oper Res, 1987,
// code was written by R. McKelvey
//

#define TOL 1.0e-10
#define MAXIT 10000

#include "gambitio.h"
#include "normal.h"
#include "normiter.h"
#include "rational.h"
#include "grarray.h"
#include "gpvector.h"
#include "gwatch.h"
#include "simpdiv.h"

SimpdivParams::SimpdivParams(void) 
  : number(1), plev(0), ndivs(20), leash(0), output(&gnull)
{ }

//-------------------------------------------------------------------------
//               SimpdivModule<T>: Constructor and destructor
//-------------------------------------------------------------------------

template <class T> SimpdivModule<T>::SimpdivModule(const NormalForm<T> &n,
						   const SimpdivParams &p)
  : rep(n), params(p), ab(n.Dimensionality()), nstrats(n.Dimensionality()),
    y(n.Dimensionality()), pi(n.ProfileLength(), 2),
    labels(n.ProfileLength(), 2), ylabel(2), nevals(0), nits(0),
    U(n.Dimensionality()), TT(n.Dimensionality()),
    besty(n.Dimensionality()), v(n.Dimensionality())
{ }

template <class T> SimpdivModule<T>::~SimpdivModule()
{ }


//-------------------------------------------------------------------------
//               SimpdivModule<T>: Private member functions
//-------------------------------------------------------------------------

template <class T> T SimpdivModule<T>::simplex(void)
{
  int i, j, k, h, jj, hh,ii, kk,tot;
  T maxz;
  
 step0:;
  ibar=1;
  t=0;
  for(j=1;j<=nplayers;j++)
    {
      for(h=1;h<=nstrats[j];h++)
	{
	  TT(j,h)=0;
	  U(j,h)=0;
	  if(v(j,h)==(T)0.0)U(j,h)=1;
	  ab(j,h)=(T)(0);
	  y(j,h)=v(j,h);
	}
    }
  
 step1:;
  
  maxz=getlabel(y);
  j=ylabel[1];
  h=ylabel[2];
  labels(ibar,1)=j;
  labels(ibar,2)=h;
//  if(nits>=MAXIT)goto end;
  if(params.plev>=4) {
    *params.output << "\n Step 1, j = " << j << " h= " << h; 
    *params.output << " maxz = " << maxz; 
  }
  
 case1a:;
  if(TT(j,h)==0 && U(j,h)==0)
    {
      if(params.plev>=4) *params.output << " Case 1a "; 
      for(hh=1,tot=0;hh<=nstrats[j];hh++)
	if(TT(j,hh)==1 || U(j,hh)==1)tot++;
      if(tot==nstrats[j]-1)goto end;
      else {
	i=t+1;
	goto step2;
      }
    }
      /* case1b */
  else if(TT(j,h))
    {
      if(params.plev>=4) *params.output << " Case 1b "; 
      i=1;
      while(labels(i,1)!=j || labels(i,2)!=h || i==ibar) i++;
      goto step3;
    }
      /* case1c */
  else if(U(j,h))
    {
      if(params.plev>=4) *params.output << " Case 1c "; 
      k=h;
      while(U(j,k)){k++;if(k>nstrats[j])k=1;}
      if(TT(j,k)==0)i=t+1;
      else {
	i=1;
	while((pi(i,1)!=j || pi(i,2)!=k) && i<=t)i++;
      }
      goto step2;
    }
  
 step2:;
  if(params.plev>=4) *params.output << "  Step 2 "; 
  getY(y,i);
  pi.RotateDown(i,t+1);
  pi(i,1)=j;
  pi(i,2)=h;
  labels.RotateDown(i+1,t+2);
  ibar=i+1;
  t++;
  getnexty(y,i);
  TT(j,h)=1;
  U(j,h)=0;
  goto step1;
  
 step3:;
  if(params.plev>=4) *params.output << "  Step 3 "; 
  if(i==t+1)ii=t;
  else ii=i;
  j=pi(ii,1);
  h=pi(ii,2);
  k=h;
  if(i<t+1)k=get_b(j,h);
  kk=get_c(j,h);
  if(i==1)ii=t+1;
  else if(i==t+1)ii=1;
  else ii=i-1;
  getY(y,ii);
  
      /* case3a */
  if(i==1 && (y(j,k)<=(T)0 || (v(j,k)-y(j,k))>=((T)(leash))*d)) {
    if(params.plev>=4) *params.output << " Case 3a "; 
    for(hh=1,tot=0;hh<=nstrats[j];hh++)
      if(TT(j,hh)==1 || U(j,hh)==1)tot++;
    if(tot==nstrats[j]-1) {
      U(j,k)=1;
      goto end;
    }
    else {
      update(j,i);
      U(j,k)=1;
      getnexty(y,t);
      goto step1;
    }
  }
      /* case3b */
  else if(i>=2 && i<=t &&
	  (y(j,k)<=(T)(0) || (v(j,k)-y(j,k))>=((T)(leash))*d)) {
    if(params.plev>=4) *params.output << " Case 3b "; 
    goto step4;
  }
      /* case3c */
  else if(i==t+1 && ab(j,kk)==(T)(0)) {
    if(params.plev>=4) *params.output << " Case 3c "; 
    if(y(j,h)<=(T)(0) || (v(j,h)-y(j,h))>=((T)(leash))*d)goto step4;
    else {
      k=0;
      while(ab(j,kk)==(T)(0) && k==0) {
	if(kk==h)k=1;
	kk++;
	if(kk>nstrats[j])kk=1;
      }
      kk--;
      if(kk==0)kk=nstrats[j];
      if(kk==h)goto step4;
      else goto step5;
    }
  }
  else {
      if(i==1) getnexty(y,1);
      else if(i<=t) getnexty(y,i);
      else if(i==t+1) {
	j=pi(t,1);
	h=pi(t,2);
	hh=get_b(j,h);
	y(j,h)-=d;
	y(j,hh)+=d;
      }
      update(j,i);
    }
  goto step1;
 step4:;
  if(params.plev>=4) *params.output << "  Step 4 "; 
  getY(y,1);
  j=pi(i-1,1);
  h=pi(i-1,2);
  TT(j,h)=0;
  if(y(j,h)<=(T)(0) || (v(j,h)-y(j,h))>=((T)(leash))*d)U(j,h)=1;
  labels.RotateUp(i,t+1);
  pi.RotateUp(i-1,t);
  t--;
  ii=1;
  while(labels(ii,1)!=j || labels(ii,2)!=h) {ii++;}
  i=ii;
  goto step3;
 step5:;
  if(params.plev>=4) *params.output << "  Step 5 "; 
  k=kk;

  labels.RotateDown(1,t+1);
  ibar=1;
  pi.RotateDown(1,t);
  U(j,k)=0;
  jj=pi(1,1);
  hh=pi(1,2);
  kk=get_b(jj,hh);
  y(jj,hh)-=d;
  y(jj,kk)+=d;
  
  k=get_c(j,h);
  kk=1;
  while(kk){
    if(k==h)kk=0;
    ab(j,k)=(ab(j,k)-((T)(1)));
    k++;
    if(k>nstrats[j])k=1;
  }
  goto step1;

 end:;
  maxz=bestz;
  nevals+=nits;
  if(params.plev >= 2) { 
    *params.output << "\ngrid = " << d << " maxz = " << maxz;
    if(params.plev>= 3) {
      *params.output << " j = " << j << " h = " << h;  
      *params.output << " nits = " <<nits;
    }
  } 
  for(i=1;i<=nplayers;i++)
    for(j=1;j<=nstrats[i];j++)
      y(i,j)=besty(i,j);
  nits=0;
  return maxz;
}

template <class T> void SimpdivModule<T>::update(int j, int i)
{
  int jj, hh, k,f;
  
  f=1;
  if(i>=2 && i<=t) {
    pi.SwitchRows(i,i-1);
    ibar=i;
  }
  else if(i==1) {
    labels.RotateUp(1,t+1);
    ibar=t+1;
    jj=pi(1,1);
    hh=pi(1,2);
    if(jj==j) {
      k=get_c(jj,hh);
      while(f) {
	if(k==hh)f=0;
	ab(j,k)=ab(j,k) + ((T)(1));
	k++;
	if(k>nstrats[jj])k=1;
      }
      pi.RotateUp(1,t);
    }
  }
  else if(i==t+1) {
    labels.RotateDown(1,t+1);
    ibar=1;
    jj=pi(t,1);
    hh=pi(t,2);
    if(jj==j) {
      k=get_c(jj,hh);
      while(f) {
	if(k==hh)f=0;
	ab(j,k)= ab(j,k)-((T)(1));
	k++;
	if(k>nstrats[jj])k=1;
      }
      pi.RotateDown(1,t);
    }
  }
}

template <class T> void SimpdivModule<T>::getY(gPVector<T> &x,int k)
{
  int j, h, i,hh;
  
  for(j=1;j<=nplayers;j++)
    for(h=1;h<=nstrats[j];h++)
      x(j,h)=v(j,h);
  for(j=1;j<=nplayers;j++)
    for(h=1;h<=nstrats[j];h++)
      if(TT(j,h)==1 || U(j,h)==1) {
	x(j,h)+=(d*ab(j,h));
	hh=h-1;
	if(hh==0)hh=nstrats[j];
	x(j,hh)-=(d*ab(j,h));
      }
  i=2;
  while(i<=k) {
    getnexty(x,i-1);
    i++;
  }
}

template <class T> void SimpdivModule<T>::getnexty(gPVector<T> &x,int i)
{
  int j,h,hh;
  
  assert(i>=1);
  j=pi(i,1);
  h=pi(i,2);
  x(j,h)+=d;
  hh=get_b(j,h);
  x(j,hh)-=d;
}

template <class T> int SimpdivModule<T>::get_b(int j, int h)
{
  int hh;
  
  hh=h-1;
  if(hh==0)hh=nstrats[j];
  while(U(j,hh)) {
    hh--;
    if(hh==0)hh=nstrats[j];
  }
  return hh;
}

template <class T> int SimpdivModule<T>::get_c(int j, int h)
{
  int hh;
  
  hh=get_b(j,h);
  hh++;
  if(hh>nstrats[j])hh=1;
  return hh;
}

template <class T> T SimpdivModule<T>::getlabel(gPVector<T> &yy)
{
  int i,j,ii,jj;
  T maxz,payoff,maxval;
  
  nits++;

  maxz=((T)(-1000000));
  
  ylabel[1]=1;
  ylabel[2]=1;
  
  for(i=1;i<=rep.NumPlayers();i++) {
    payoff=(T)(0);
    maxval=((T)(-1000000));
    for(j=1;j<=rep.NumStrats(i);j++) {
      pay=rep.Payoff(i,i,j,yy);
      payoff+=(yy(i,j)*pay);
      if(pay>maxval) {
	maxval=pay;
	jj=j;
      }
    }
    if((maxval-payoff)>maxz) {
      maxz=maxval-payoff;
      ylabel[1]=i;
      ylabel[2]=jj;
    }
  }
  if(maxz<bestz) {
    bestz=maxz;
    for(i=1;i<=nplayers;i++)
      for(j=1;j<=nstrats[i];j++)
	besty(i,j)=yy(i,j);
  }
  return maxz;
}


//-------------------------------------------------------------------------
//               SimpdivModule<T>: Main solution algorithm
//-------------------------------------------------------------------------

template <class T> int SimpdivModule<T>::Simpdiv(gList<gPVector<T> > &solns)
{
  int qf,soln,i,j,k,ii;

  if(leash==0)leash=32000;
  bestz=(T)1.0e30;
  mingrid=(T)(2);
  for(i=1;i<=params.ndivs;i++)mingrid=mingrid*(T)(2);
  mingrid = ((T)(1))/mingrid;
//  *params.output << "\nleash = " << leash << " ndivs = " << ndivs;
//  *params.output << " mingrid = " << mingrid;
  
  nplayers=rep.NumPlayers();
  
  y = (T)(0);
//  *params.output << "\nnplayers =" << nplayers;
//  *params.output << "\nnstrats = " << nstrats;
//  *params.output << "\ny = " << y;
  
  solns.Flush();
  for(soln=0;soln<params.number;soln++)
    {
      k=1;
      d=(T) 1.0 / (T) k;
      for(i=1;i<=nplayers;i++)
	{
//	  *params.output << "\n i = " << i;
	  y(i,1)=(T)(1);
	  for(j=1;j<=nstrats[i];j++)
	    if(j>1)y(i,j)=(T)(0);
	}
      
      for(qf=0;qf!=1 && d > mingrid;)
	{
	  ii=0;
	  d=(T)(d/(T)2.0);
	  for(i=1;i<=nplayers;i++)
	    for(j=1;j<=nstrats[i];j++)
	      v(i,j)=y(i,j);
	  maxz=simplex();
	  
//	  if(maxz<(T)(TOL) || nevals>=MAXIT)qf=1;
	  if(maxz<(T)(TOL))qf=1;
	}
      *params.output << "\nSimpDiv solution # " << soln+1 << " : " << y;
      *params.output << " maxz = " << maxz; 
      solns.Append(y);
    }
}


#include "rational.h"

#ifdef __GNUG__
template class SimpdivModule<double>;
template class SimpdivModule<gRational>;
#elif defined __BORLANDC__
#pragma option -Jgd
class SimpdivModule<double>;
class SimpdivModule<gRational>;
#endif   // __GNUG__, __BORLANDC__
