/*  SIMPDIV.CC    Mixed strategy Algorithm for GAMBIT       2/14/91
*/

/*
simpdiv is a simplicial subdivision algorithm with restart, for finding
mixed strategy solutions to general finite n-person games.  It is based on
van Der Laan, Talman and van Der Heyden, Math in Oper Res, 1987,
code was written by R. McKelvey
*/

#define TOL 1.0e-10

#include "gambitio.h"
#include "normal.h"
#include "nfrep.h"
#include "normiter.h"
#include "rational.h"
#include "gmatrix.h"
#include "gmatrix1.h"
#include "mixed.h"
#include "solution.h"

class BaseSimpdiv {
public:
  virtual int Simpdiv(int) = 0;
  virtual int Nevals(void) = 0;
  virtual ~BaseSimpdiv() {}
};

template <class T> class SimpdivModule
: public BaseSimpdiv, public SolutionModule {
private:
  const NFRep<T> &rep;
  long leash;
  int t, nplayers, ibar,nevals, plev,ndivs;
  T pay,d,maxz,bestz,mingrid;
  gVector<int> strat,temp,nstrats,ylabel,temp1;
  gVector<T> M;
  gMatrix1<int> labels,pi;
  gNRMatrix1<int> U,TT;
  MixedProfile<T> ab,y,besty,v;
public:
  SimpdivModule(const NFRep<T> &N,gOutput &ofile,gOutput &efile,int plev,
	      int ndivs, int leashlength)
    :  SolutionModule(ofile,efile,plev), rep(N), plev(plev),ab(rep),y(rep),
  leash(leashlength),ndivs(ndivs), nevals(0), besty(rep),v(rep){ }
  
  int Nevals(void) { return nevals;}
  int Simpdiv(int) ;
  virtual ~SimpdivModule() {}
  
  T simplex(void);
  T getlabel(MixedProfile<T> &yy);
  void update(int j, int i);
  void getY(MixedProfile<T> &x,int k);
  void getnexty(MixedProfile<T> &x,int i);
  int get_c(int j, int h);
  int get_b(int j, int h);
  
};

template <class T> int SimpdivModule<T>::Simpdiv(int number)
{
  int qf,soln,i,j,k,maxt,ii;
  

//  leash=10;
  if(leash==0)leash=32000;
  bestz=(T)1.0e30;
//  mingrid = (T) ( (T)(1) / (T)(2) );
  mingrid=(T)(2);
  for(i=1;i<=ndivs;i++)mingrid=mingrid*(T)(2);
  mingrid = ((T)(1))/mingrid;
  gout << "\nleash = " << leash << " ndivs = " << ndivs;
  gout << " mingrid = " << mingrid;

//  mingrid=(T)(1)/(T)ndivs;

  nplayers=rep.NumPlayers();
  
  nstrats = gVector<int>(1,nplayers);
  
  for(i=1,maxt=0;i<=nplayers;i++)
    {
      nstrats[i]=rep.NumStrats(i);
      maxt+=j;
    }
  
  temp = gVector<int>(2);
//  v = MixedProfile<T>(rep);
  ylabel = gVector<int>(2);
  
  for(i=1,maxt=0;i<=nplayers;i++)
    maxt+=nstrats[i];
  
//  gVector<int> temp1(maxt+1);
//  temp1 = 2;
  pi = gMatrix1<int>(1,maxt,1,2);
  labels = gMatrix1<int>(1,maxt+1,1,2);
  U = gNRMatrix1<int>(1,nplayers,1,nstrats);
  TT = gNRMatrix1<int>(1,nplayers,1,nstrats);
//  ab = MixedProfile<T>(rep);
//  y = MixedProfile<T>(rep);
//  besty = MixedProfile<T>(rep);

  y = 0;
//  gout << "\nnplayers =" << nplayers;
//  gout << "\nnstrats = " << nstrats;
//  gout << "\ny = " << y;
  
  for(soln=0;soln<number;soln++)
    {
      k=1;
      d=(T)(1.0/(T)(k));
      for(i=1;i<=nplayers;i++)
	{
//	  gout << "\n i = " << i;
	  y[i][1]=(T)1.0;
	  for(j=1;j<=nstrats[i];j++)
	    if(j>1)y[i][j]=(T)0.0;
	}
      
      for(qf=0;qf!=1 && d > mingrid;)
	{
	  ii=0;
	  d=(T)(d/(T)2.0);
	  for(i=1;i<=nplayers;i++)
	    for(j=1;j<=nstrats[i];j++)
	      v[i][j]=y[i][j];
	  maxz=simplex();
	  
	  if(plev >= 2) { 
	    gout << "\ngrid size = " << d << " maxz = " << maxz;
	    gout << " nevals = " <<nevals;
	  } 
	  if(maxz<TOL)qf=1;
	}
/*			fclose(dumpfile);
*/
      gout << "\nSimDiv solution # " << soln+1 << " : " << y;
    }
}

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
	  if(v[j][h]==(T)0.0)U(j,h)=1;
	  ab[j][h]=(T)0.0;
	  y[j][h]=v[j][h];
	}
    }
  
 step1:;
  
  maxz=getlabel(y);
  j=ylabel[1];
  h=ylabel[2];
  labels(ibar,1)=j;
  labels(ibar,2)=h;
  
 case1a:;
  if(TT(j,h)==0 && U(j,h)==0)
    {
      
      for(hh=1,tot=0;hh<=nstrats[j];hh++)
	if(TT(j,hh)==1 || U(j,hh)==1)tot++;
      if(tot==nstrats[j]-1)goto end;
      else
	{
	  i=t+1;
	  goto step2;
	}
    }
      /* case1b */
  else if(TT(j,h))
    {
      i=1;
      
      while(labels(i,1)!=j || labels(i,2)!=h || i==ibar)
	{
	  i++;
	}
      goto step3;
    }
      /* case1c */
  else if(U(j,h))
    {
      k=h;
      while(U(j,k)){k++;if(k>nstrats[j])k=1;}
      if(TT(j,k)==0)i=t+1;
      else
	{
	  i=1;
	  while((pi(i,1)!=j || pi(i,2)!=k) && i<=t)i++;
	}
      goto step2;
    }
  
 step2:;
  getY(y,i);
  pi.RotateUp(i,t+1);
//  temp=pi[t+1];
//  for(k=t;k>=i;k--)
//    {
//      pi[k+1]=pi[k];
//    }
//  pi[i]=temp;
  pi(i,1)=j;
  pi(i,2)=h;
  labels.RotateUp(i+1,t+2);
//  temp=labels[t+2];
//  for(k=t+1;k>i;k--)
//    labels[k+1]=labels[k];
//  labels[i+1]=temp;
  ibar=i+1;
  t++;
  getnexty(y,i);
  TT(j,h)=1;
  U(j,h)=0;
  goto step1;
  
 step3:;
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
  if(i==1 && (y[j][k]<=0 || (v[j][k]-y[j][k])>=(T)leash*d))
    {
      for(hh=1,tot=0;hh<=nstrats[j];hh++)
	if(TT(j,hh)==1 || U(j,hh)==1)tot++;
      if(tot==nstrats[j]-1)
	{
	  U(j,k)=1;
	  goto end;
	}
      else
	{
	  update(j,i);
	  U(j,k)=1;
	  getnexty(y,t);
	  goto step1;
	}
    }
      /* case3b */
  else if(i>=2 && i<=t && (y[j][k]<=0 || (v[j][k]-y[j][k])>=(T)leash*d))
    {
      goto step4;
    }
      /* case3c */
  else if(i==t+1 && ab[j][kk]==0)
    {
      if(y[j][h]<=0 || (v[j][h]-y[j][h])>=(T)leash*d)goto step4;
      else
	{
	  k=0;
	  while(ab[j][kk]==0 && k==0)
	    {
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
  else
    {
      if(i==1)
	{
	  getnexty(y,1);
	}
      else if(i<=t)
	{
	  getnexty(y,i);
	}
      else if(i==t+1)
	{
	  j=pi(t,1);
	  h=pi(t,2);
	  hh=get_b(j,h);
	  y[j][h]-=d;
	  y[j][hh]+=d;
	}
      update(j,i);
    }
  goto step1;
 step4:;
  getY(y,1);
  j=pi(i-1,1);
  h=pi(i-1,2);
  TT(j,h)=0;
  if(y[j][h]<=0 || (v[j][h]-y[j][h])>=(T)leash*d)U(j,h)=1;
  labels.RotateDown(i,t+1);
//  temp=labels[i];
//  for(ii=i;ii<=t;ii++)
//    labels[ii]=labels[ii+1];
//  labels[t+1]=temp;
  pi.RotateDown(i-1,t);
//  temp=pi[i-1];
//  for(ii=i-1;ii<=t-1;ii++)
//    pi[ii]=pi[ii+1];
//  pi[t]=temp;
  t--;
  ii=1;
  while(labels(ii,1)!=j || labels(ii,2)!=h)
    {
      ii++;
    }
  i=ii;
  goto step3;
 step5:;
  k=kk;

  labels.RotateUp(1,t+1);
//  temp=labels[t+1];
//  for(i=t+1;i>=2;i--)labels[i]=labels[i-1];
//  labels[1]=temp;
  ibar=1;
  pi.RotateUp(1,t);
//  temp=pi[t];
//  for(i=t;i>=2;i--)pi[i]=pi[i-1];
//  pi[1]=temp;
  
  U(j,h)=0;

  jj=pi(1,1);
  hh=pi(1,2);
  kk=get_b(jj,hh);
  y[jj][hh]-=d;
  y[jj][kk]+=d;
  
  k=get_c(j,h);
  kk=1;
  while(kk)
    {
      if(k==h)kk=0;
      ab[j][k]--;
      k++;
      if(k>nstrats[j])k=1;
    }
  goto step1;
 end:;
  
  for(i=1;i<=nplayers;i++)
    for(j=1;j<=nstrats[i];j++)
      y[i][j]=besty[i][j];
  maxz=bestz;
  return maxz;
}

template <class T> void SimpdivModule<T>::update(int j, int i)
{
  int jj, hh, k,f;
  
  f=1;
  if(i>=2 && i<=t)
    {
      pi.SwapRows(i,i-1);
//      temp=pi[i];
//      pi[i]=pi[i-1];
//      pi[i-1]=temp;
      ibar=i;
    }
  else if(i==1)
    {
      labels.RotateDown(1,t+1);
//      temp=labels[1];
//      for(k=1;k<=t;k++)
//	labels[k]=labels[k+1];
//      labels[t+1]=temp;
      ibar=t+1;
      jj=pi(1,1);
      hh=pi(1,2);
      if(jj==j)
	{
	  k=get_c(jj,hh);
	  while(f)
	    {
	      
	      if(k==hh)f=0;
	      ab[j][k]++;
	      k++;
	      if(k>nstrats[jj])k=1;
	    }
	  pi.RotateDown(1,t);
//	  temp=pi[1];
//	  for(k=1;k<t;k++)
//	    pi[k]=pi[k+1];
//	  pi[t]=temp;
	}
    }
  else if(i==t+1)
    {
      labels.RotateUp(1,t+1);
//      temp=labels[t+1];
//      for(k=t;k>=1;k--)
//	labels[k+1]=labels[k];
//      labels[1]=temp;
      ibar=1;
      jj=pi(t,1);
      hh=pi(t,2);
      if(jj==j)
	{
	  k=get_c(jj,hh);
	  while(f)
	    {
	      if(k==hh)f=0;
	      ab[j][k]--;
	      k++;
	      if(k>nstrats[jj])k=1;
	    }
	  pi.RotateUp(1,t);
//	  temp=pi[t];
//	  for(k=t;k>=2;k--)
//	    pi[k]=pi[k-1];
//	  pi[1]=temp;
	}
    }
}

template <class T> void SimpdivModule<T>
::getY(MixedProfile<T> &x,int k)
{
  int j, h, i,hh;
  
  for(j=1;j<=nplayers;j++)
    for(h=1;h<=nstrats[j];h++)
      x[j][h]=v[j][h];
  for(j=1;j<=nplayers;j++)
    for(h=1;h<=nstrats[j];h++)
      if(TT(j,h)==1 || U(j,h)==1)
	{
	  x[j][h]+=d*ab[j][h];
	  hh=h-1;
	  if(hh==0)hh=nstrats[j];
	  x[j][hh]-=d*ab[j][h];
	}
  
  i=2;
  while(i<=k)
    {
      getnexty(x,i-1);
      i++;
    }
}

template <class T> void SimpdivModule<T>
::getnexty(MixedProfile<T> &x,int i)
{
  int j,h,hh;
  
  assert(i>=1);
  j=pi(i,1);
  h=pi(i,2);
  x[j][h]+=d;
  hh=get_b(j,h);
  x[j][hh]-=d;
}

template <class T> int SimpdivModule<T>::get_b(int j, int h)
{
  int hh;
  
  hh=h-1;
  if(hh==0)hh=nstrats[j];
  while(U(j,hh))
    {
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

template <class T> T SimpdivModule<T>::getlabel(MixedProfile<T> &yy)
{
  int i,j,ii,jj;
  T maxz,payoff,maxval;
  
  nevals++;

  maxz=(T)(-1000000.0);
  
  ylabel[1]=1;
  ylabel[2]=1;
  
  for(i=1;i<=rep.NumPlayers();i++)
    {
      payoff=(T)0.0;
      maxval=(T)(-1000000.0);
      for(j=1;j<=rep.NumStrats(i);j++)
	{
	  pay=rep.Payoff(i,i,j,yy);
	  payoff+=yy[i][j]*pay;
	  if(pay>maxval)
	    {
	      maxval=pay;
	      jj=j;
	    }
	}
      if((maxval-payoff)>maxz)
	{
	  maxz=maxval-payoff;
	  ylabel[1]=i;
	  ylabel[2]=jj;
	}
    }
	if(maxz<bestz)
	  {
	    bestz=maxz;
	    for(i=1;i<=nplayers;i++)
	      for(j=1;j<=nstrats[i];j++)
		besty[i][j]=yy[i][j];
	  }
  return maxz;
}


int NormalForm::Simpdiv(int number, int plev,gOutput &out,gOutput &err,
			int ndivs, int leashlength,int &nevals)
{
  BaseSimpdiv *T;
  
  switch (type)  {
  case DOUBLE:
    T = new SimpdivModule<double>((NFRep<double> &) *data, out, err,plev,
				  ndivs,leashlength);
    break;
/*
 case nfRATIONAL:
    T = new LiapModule<Rational>((NFRep<Rational> &) *data, gout, gerr,plev);
    break;
    */
  }
  T->Simpdiv(number);
  nevals=T->Nevals();
  return 1;
  
}



