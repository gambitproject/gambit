//
// FILE: stab.cc
// DESCRIPTION:
// Implementation of R. Wilson's algorithm to compute simply
// stable sets.
//
//
// This program finds simply stable sets to a general
// two person normal form game using the algorithm described in
// "Computing Simply Stable Equilibria," by Robert Wilson (1991).
//
// The will find either primal or dual simply stable sets,
// and can also be used to find all simply stable sets that are
// accessible from the extraneous solution under Wilson's algorithm.
// To do this, at each new simply stable set found (with the ith
// rotation of the lex-order), one searches recursively on all
// j-paths (with j /= i) for new simply stable sets.
//
// To run the program, type
//
// stab game.nfg
//
// where game.nfg is a Gambit normal form game file.
//
//
// ALGORITHM PARAMETERS
//
// The behavior of the algorithm may be modified by the following
// parameters (currently, these are hardcoded into the source
// file; you'll have to modify them and recompile to change them):
//
// typesol:
// 0 = simply stable (Beware!! Doesn't work yet)
// 1 = primal simply stable (default)
// 2 = dual simply stable (Beware!! Doesn't work yet)
//
// maxnum: 	Maximum number of solutions to compute
// 0 = all accessible(default)
// i = stop after i
//
// maxlev: 	Maximum level to recurse for additional solutions
// 0 = all (default)
// i = stop at level i
//
// whichkeep:
// 0 = keep all equilibria found (default)
// 1 = keep only solutions of type typesol
//
// printlev: 	Level of output
// 0 = just output final solutions(default)
// 1 = output normal form and solutions
// 4 = output entire route
// 5 = output tableaus after each pivot
// 6 = output tableaus before and after each pivot
//
// OUTPUT
// The solutions are output in the following format
//
// num direc ipath M level setnum sstab value_1 value_2 (p_1, p_2, ... ) (q_1, q_2, ... )
//
// where
// num: equilibrium number
// direc:
// ipath:
// M:
// level:
// setnum:
// sstab:
// value_1: value to player 1
// value_2: value to player 2
// p_i:
//
// HISTORY AND ACKNOWLEDGEMENTS:
//
// The code for finding simply stable sets was translated
// from an APL version provided by R. Wilson.
//
// The translation to C was done by R. McKelvey, who also made some
// modifications to enhance speed and added some recursive calls
// to allow for computation of all accessible stable sets.
// This version was written to compile under Turbo C, Version 1.01,
// for DOS 3.0 or above. (!!!)
//
// The current version was produced in December 2004 by T. Turocy, who
// modified the version dated 12 September 1991 by R. McKelvey.
// The code was updated (somewhat) to C++, cleaned up to use some
// more modern idioms, and adapted to use the modern Gambit
// API for accessing normal form game payoffs to construct the
// tableaux.
//
// This version is dated December 14, 2004.
//
// This file is Copyright (C) 2004 by The Gambit Project, and
// is distributed freely under the terms of the GNU GPL.
//

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#include <fstream>
#include <libgambit/libgambit.h>


int st_typesol, st_maxnum, st_maxlev, st_whichkeep, st_printlev;

int *ivector(int nl,int nh);
int **imatrix(int nrl,int nrh,int ncl,int nch);
double *dvector(int nl,int nh);
double **dmatrix(int nrl,int nrh,int ncl,int nch);
void free_ivector(int *v,int nl,int nh);
void free_imatrix(int **m,int nrl,int nrh,int ncl,int nch);
void free_dvector(double *v,int nl,int nh);
void free_dmatrix(double **m,int nrl,int nrh,int ncl,int nch);
void nrerror(char *error_text);


#define SIMP_STABLE 0
#define PRIMAL_STABLE 1
#define DUAL_STABLE 2

FILE *outfl = stdout;


int **lbl0;
double ***T0;
double *D0;

int **lbl;
double ***T;
double *D;

double eps=DBL_EPSILON * (double)256.0;
double *xx, *yy;
double *xS, *rr;
Gambit::Array<int> S, iS;
int *BZ, *bz, *NG, *j, *J0;
int **jvec;

int _m,_n;
int setnum, setnum1,setnum2,level, maxmn,dimlbl,dimlex,dimx,MMM;
int rows[3],cols[3];
int ifirst, ilast;

int count;  /**/

void reorder(double *x, int *j);
double ***dtableau(int nrl,int nrh,int ncl,int nch);
void free_dtableau(double ***m, int nrl,int nrh,int ncl,int nch);


int new_component(void);
void print_tableau(int k);


//--------------------------------------------------------------------
//                General utility routines
//--------------------------------------------------------------------

bool approx(double d1, double d2)
{
  return (((d1 && d2) ? fabs((d2-d1)/d2) : fabs(d2-d1)) < eps);
}

void shell_sort(int n, int *arr, int *brr)
{
  for (int gap = n/2; gap > 0; gap /= 2) {
    for (int i = gap; i < n; i++) {
      for (int j = i-gap+1; j >=1 && arr[j] > arr[j+gap]; j -= gap) {
	int temp = arr[j];
	arr[j] = arr[j+gap];
	arr[j+gap] = temp;
	temp = brr[j];
	brr[j] = brr[j+gap];
	brr[j+gap] = temp;
      }
    }
  }
}

bool elementof(int x, int *y, int n)
{
  int i;

  for(i=1;i<=n;i++)if(y[i]==x)return true;

  return false;
}

bool elementof(int x, const Gambit::Array<int> &y, int n)
{
  for (int i = 1; i <= n; i++) {
    if (y[i] == x) {
      return true;
    }
  }

  return false;
}

class Equilibrium {
  /* just store non zero components  */
public:
  int equinumber;
  Equilibrium *nextequi;

  int stable, level,setnum,direction,ipath,M;
  double value[3];
  /* data for mixed strategy representation */
  int nstrats[3];          /* # non-zero components by [plyr#] */
  int **basis;    		/* basis by [plyr#][non-zero#] */
  double **prob;

  void Print(FILE *) const;
};

// Print this equilibrium, and all subsequent ones in linked list
void Equilibrium::Print(FILE *f) const
{
  for (const Equilibrium *e = this; e; e = e->nextequi) {
    fprintf(f, "\n%3d%3d%3d %3d%3d%3d%2d%8.3f%8.3f  ",
	    e->equinumber,
	    e->direction,
	    e->ipath,
	    e->M,
	    e->level,
	    e->setnum,
	    e->stable,
	    e->value[1],
	    e->value[2]);
    for (int i = 1; i <= _m; i++) {
      int i1 = 1;
      while (i1 <= e->nstrats[1] && e->basis[1][i1] != i) {
	i1++;
      }

      if (i1 <= e->nstrats[1]) {
	fprintf(f, "%6.3f", e->prob[1][i1]);
      }
      else {
	fprintf(f, "%6.3f", 0.0);
      }
    }
    fprintf(f, "  ");

    for (int i = MMM+1; i <= MMM+_n; i++) {
      int i1 = 1;
      while (i1 <= e->nstrats[2] && e->basis[2][i1] != i) {
	i1++;
      }
      if (i1 <= e->nstrats[2]) {
	fprintf(f, "%6.3f", e->prob[2][i1]);
      }
      else {
	fprintf(f, "%6.3f", 0.0);
      }
    }
  }
  fprintf(f, "\n");
}


bool AreSameEquilibrium(Equilibrium *e1, Equilibrium *e2)
{
  for (int k = 1; k <= 2; k++) {
    if (e1->nstrats[k] != e2->nstrats[k]) return false;

    for (int i = 1; i <= e1->nstrats[k]; i++) {
      if (e1->basis[k][i] != e2->basis[k][i]) {
	return false;
      }
    }
  }
  return true;
}



class AlgorithmState {
public:
  int delp, del;
  Gambit::Array<int> lex0;
  double dA[3];

  // List of computed equilibria
  int nequi;
  Equilibrium *firstequi, *lastequi;

  AlgorithmState(void)
    : nequi(0), firstequi(NULL), lastequi(NULL)
  { }

  Equilibrium *NewEquilibrium(void);
  void FreeEquilibrium(Equilibrium *);
  Equilibrium *PriorEquilibrium(Equilibrium *ee);
};

Equilibrium *AlgorithmState::NewEquilibrium(void)
{
  Equilibrium *ee = new Equilibrium;

  if (firstequi != NULL) {
    Equilibrium *e;
    int i;

    for (e = firstequi, i = 1;
	 e->nextequi != NULL; e = e->nextequi, i++) {
      e->equinumber = i;
    }
    e->equinumber = i;
    e->nextequi = ee;
    ee->equinumber = i+1;
  }
  else {
    firstequi = ee;
    ee->equinumber = 1;
  }

  ee->nextequi = NULL;
  nequi++;

  ee->level = 0;
  ee->setnum = 0;
  ee->stable = 0;

  ee->value[1] = ee->value[2] = 0.0;
  ee->nstrats[1] = ee->nstrats[2] = 0;
  ee->basis = NULL;
  ee->prob = NULL;

  return ee;
}

void AlgorithmState::FreeEquilibrium(Equilibrium *ee)
{
  free_imatrix(ee->basis,1,2,1,1);
  free_dmatrix(ee->prob,1,2,1,1);
  if (firstequi == ee) {
    firstequi = ee->nextequi;
  }
  else {
    (PriorEquilibrium(ee))->nextequi = ee->nextequi;
  }

  delete ee;
  nequi--;
}

/* find previous equilibrium to ee */
Equilibrium *AlgorithmState::PriorEquilibrium(Equilibrium *ee)
{
  Equilibrium *e;
  for (e = firstequi; e && e->nextequi != ee; e = e->nextequi);
  return e;
}


int Ipath(AlgorithmState &, Gambit::Array<int> &);
int findpr(AlgorithmState &, Gambit::Array<int> &, int k, int pc);
void rlex(AlgorithmState &, Gambit::Array<int> &lx, int dir);
void lexorder(AlgorithmState &, Gambit::Array<int> &, int *j, int k);
void Pivot(AlgorithmState &, int k, int pr,int pc);


int IsOldComponent(AlgorithmState &p_state, Equilibrium *ee)
{
  for (Equilibrium *e = p_state.firstequi; e != ee; e = e->nextequi) {
    if (AreSameEquilibrium(e, ee) ||
	(ee->nstrats[1]==0 && ee->nstrats[2]==0)) {
      return true;
    }
  }

  return false;
}

 /* checks each newly found component, tossing
    old components, marking and saving new ones.
    Returns 1 if new stable set, 0 otherwise */
int new_component(AlgorithmState &p_state)
{
  Equilibrium *e,*ee, *e1;
  Equilibrium *lastnew,*firstnew;

  if(p_state.firstequi==NULL)return 0;
  /* get first and last new cquilibria */
  firstnew=p_state.firstequi;

  if(p_state.lastequi!=NULL) {
    firstnew=p_state.lastequi->nextequi;
  }

  for(e=firstnew;e && e->nextequi!=NULL;e=e->nextequi);
  lastnew=e;

  if(!lastnew || lastnew==p_state.lastequi)return 0;

  /* for each component . . . */
  for(e=firstnew;e!=NULL;) {
    /* get next component */

    for(ee=e;ee!=NULL && ee->setnum==e->setnum;ee=ee->nextequi);

    /* toss if already found  */
    if (IsOldComponent(p_state, e) ||
	(e->setnum!=lastnew->setnum && st_whichkeep==1)) {
      while(e!=ee) {
	e1=e->nextequi;
	p_state.FreeEquilibrium(e);
	e=e1;
      }
      e=ee;
    }
    else {
      /* mark and save if new */
      setnum2++;
      for(e1=e;e1!=ee;e1=e1->nextequi) {
	if(e1->setnum==lastnew->setnum)
	  e1->stable=1;
	e1->setnum=setnum2;
	p_state.lastequi=e1;
	if(p_state.lastequi==lastnew) {
	  if(e1->stable==1)setnum1++;
	  return 1;
	}
      }
    }
    e=ee;
  }
  return 0;
}

int _M, _N, I,Ps, dr;


void output(AlgorithmState &p_state, int it, int dr, int I)
{
  double p, pay;
  int dim,i, k,kalt,ii;
  int **so, **soi;

  so=imatrix(1,3,1,_m+_n);
  soi=imatrix(1,3,1,_m+_n);
  Gambit::Array<double> xo(1,_m+_n);

  Equilibrium *e = p_state.NewEquilibrium();

  for(i=1;i<=_m;i++)xo[i]=-T[1][i][_n+1];
  for(i=1;i<=_n;i++)xo[_m+i]=-T[2][i][_m+1];

  for(k=1;k<=2;k++)
    {
      kalt=2;
      if(k==2)kalt=1;
      ii=0;
      for(i=1;i<=rows[kalt];i++)if(lbl[kalt][i]>0)
	{
	  ii++;
	  so[k][ii]=lbl[kalt][i];
	  soi[k][ii]=i;
	  if(k==1)soi[k][ii]+=_m;
	}
      e->nstrats[k]=ii;
      shell_sort(ii,so[k],soi[k]);
    }


  e->direction=dr;
  e->ipath=I;

  e->level=level;
  e->M=0;
  if(I==_M){e->M=1;setnum++;}
  e->setnum=setnum;

  dim=e->nstrats[1];
  if(e->nstrats[2]>dim)dim=e->nstrats[2];
  if(dim==0)dim=1;

  e->basis=imatrix(1,2,1, dim);
  e->prob=dmatrix(1,2,1,dim);

  for(k=1;k<=2;k++)
    {
      p=T[k][rows[k]+1][cols[k]+1];
      if(p==0.0)pay=0.0;
      else pay=-(D[k]/p)-p_state.dA[k];
      e->value[k]=pay;
    }

  for(k=1;k<=2;k++)
    {
      pay=0.0;
      for(i=1;i<=e->nstrats[k];i++)
	{
	  e->basis[k][i]=so[k][i];
	  e->prob[k][i]=xo[soi[k][i]];
	  pay+=e->prob[k][i];
	}

      if(pay!=0.0)
	for(i=1;i<=e->nstrats[k];i++)
	  e->prob[k][i]/=pay;
    }

  free_imatrix(so,1,3,1,_m+1);
  free_imatrix(soi,1,3,1,_m+1);

}


int get_sol(AlgorithmState &p_state, Gambit::Array<int> &p_lex)
{
  int i,i1,i2,it;
  int k,r,jj;
  Equilibrium *e;
  ifirst=1;
  ilast=_m+_n;
  if(st_typesol==0)ilast*=2;
  setnum++;
  _M=p_lex[ifirst]; _N=p_lex[ilast];

  Ps=0;dr=-1;I=_N;

  it=0;
  lexorder(p_state, p_lex, jvec[1],1);
  lexorder(p_state, p_lex, jvec[2],2);

  /**/
  if(st_printlev>=5)
    {
      if(st_printlev>=4)fprintf(outfl,"\nlbl = \n");

      for(i1=1;i1<=2;i1++)
	{
	  for(i2=1;i2<=dimlbl;i2++)
	    if(st_printlev>=4)fprintf(outfl,"%4d",lbl[i1][i2]);
	  if(st_printlev>=4)fprintf(outfl,"\n");
	}

      if(st_printlev>=4)fprintf(outfl,"\nm= %d, n= %d, lex = ",_m,_n);
      for(i1=0;i1<=dimlex;i1++)
	if(st_printlev>=4)fprintf(outfl,"%4d",p_lex[i1]);
      if(st_printlev>=4)fprintf(outfl,"\n");

      if(st_printlev>=4)fprintf(outfl,"\njvec[1]= \n");
      for(i=1;i<=_m+_n+1;i++)
	if(st_printlev>=4)fprintf(outfl,"%4d",jvec[1][i]);

      if(st_printlev>=4)fprintf(outfl,"\njvec[2]= \n");
      for(i=1;i<=_m+_n+1;i++)
	if(st_printlev>=4)fprintf(outfl,"%4d",jvec[2][i]);

    }

  /**/
 Rotate:
  it=it+1;
  if(it>=2)output(p_state, it, dr, I);

  p_state.delp=0;

  if(it==1 && 0.0== T[1][_m+1][_n+1] && 0.0== T[2][_n+1][_m+1])
    {
      rlex(p_state, p_lex, 1);
      if(st_printlev>=4)fprintf(outfl,"\nBase level (I=%d); iter=%d",_M,it); /* R4 */
      if(Ipath(p_state, p_lex))goto Rotate;
      else goto Error;
    }
										/* for each lex-order i, */
  for(;;)
    {								/*   R1   */
      /* check for i-feasibility */

      k=1;							/* for each row r */
      for(r=1;r<=rows[k];r++)
	{
	  /* check if row r is lex positive */
	  for(i=1;i<=dimx;i++)
	    {
	      jj=jvec[k][i];
	      if(jj>_m && T[k][r][jj-_m]!=0.0)
		{
		  if(T[k][r][jj-_m]>0.0) goto R2;
		  else goto quit1;
		}
	      if(jj==r) goto R2;
	    }
	quit1:;
	}

      k=2;
      for(r=1;r<=rows[k];r++)
	{
	  /* check if row r is lex positive */
	  for(i=1;i<=dimx;i++)
	    {
	      jj=jvec[k][i];
	      if(jj==_m+_n+1 && T[k][r][_m+1]!=0.0)
		{
		  if(T[k][r][_m+1]>0.0) goto R2;
		  else goto quit2;
		}
	      if(jj<=_m && T[k][r][jj]!=0.0)
		{
		  if(T[k][r][jj]>0.0) goto R2;
		  else goto quit2;
		}
	      if(jj==_m+r) goto R2;
	    }
	quit2:;

	}


      if(p_lex[ilast]==_M && dr==-1)
	{
	  if(st_printlev>=4)fprintf(outfl,"\nBase level (I=%d); iter=%d",_M,it); /* R4 */
	  if(Ipath(p_state, p_lex))goto Rotate;
	  else goto Error;
	}

      if(p_lex[ilast]==_N && dr==1)
	{
	  if(st_printlev>=4)fprintf(outfl,"\nEnd of route %d.", _N);          /* REnd */
	  Ps+=p_state.delp;
	  goto End;
	}

      p_state.delp+=dr;
      rlex(p_state, p_lex, dr);
    }

 R2:													/*  R2  */
  if(p_state.delp!=0)
    {
      p_state.delp-=dr;                                /*  R3 */
      rlex(p_state, p_lex, -dr);
      if(Ipath(p_state, p_lex))goto Rotate;
      else goto Error;
    }

  if(st_printlev>=4)fprintf(outfl,"\nTableau infeasible");

 End:
  e = p_state.firstequi;
  if (p_state.lastequi != NULL) {
    e = p_state.lastequi->nextequi;
  }
  /*if(st_printlev>=4) */ e->Print(outfl);
  return 1;

 Error:
  return 0;
}

int Ipath(AlgorithmState &p_state, Gambit::Array<int> &p_lex)
{
  int i, k,pr, pc,jj;
  int ib[3],nrows,ncols,dimBZ,dimNG;
  /* IPath  */
  Ps+=p_state.delp;
  I=p_lex[_m+_n];
  if(dr>0) {
    /*  NPvt  */
    pc=p_lex[1];
    I=pc;
    k=1+(elementof(pc,lbl[2],dimlbl));
    rlex(p_state, p_lex, dr);
    Ps++;
    p_state.del=-1;
    if(st_printlev>=4)fprintf(outfl,"\nNPvt:Ps,k,pc= %d, %d, %d;pr,pc",Ps,k,pc);
  }
  else {
    dr=1;
    /*  PPvt */
    k=1;
    i=rows[k]+1;
    jj=abs(I);
    while(jj!= abs(lbl[k][i]) && i <= _m+_n) i++;
    if(_m+_n>=i) goto IP1;

    k=2;
    i=rows[k]+1;
    jj=abs(I);
    while(jj!= abs(lbl[k][i]) && i <= _m+_n ) i++;
    if(_m+_n>=i) goto IP1;
  IP1:
    pc=lbl[k][i];
    p_state.del=1;
    if(st_printlev>=4)fprintf(outfl,"\nPPvt:Ps,k,pc=%d,%d,%d;pr,pc",Ps,k,pc);
  }
 Findpr:
  pr=findpr(p_state, p_lex, k,pc);

 Pivot:
  if(pr == 0 || pr==-pc) {
    printf("\nEq# %d, level %d, lex # %d, pass %d.  Accuracy lost: Can't find pivot row",
	    setnum1,level,p_lex[1],count);
    return 0;
  }
  Pivot(p_state, k, pr, pc);

 Check:
  pc=-pr;

  k++;
  if(k==3)k=1;
  /* check these statements */
  if(abs(I)==abs(pc))return 1;
  if(_M==I)goto Findpr;

 Boundary:
  i=1;
  while(lbl[k][i]!=I && i<=dimlbl)i++;
  ib[1]=i;
  i=1;
  while(lbl[k][i]!=pc && i<=dimlbl)i++;
  ib[2]=i;

  jj=0;
  nrows=rows[k];
  ncols=cols[k];
  for(i=1;i<=nrows;i++) {
    bz[i]=0;
    if(T[k][i][ncols+1]==0.0) {
      bz[i]=1;
      jj++;
      BZ[jj]=i;
    }
  }
  dimBZ=jj;
  jj=0;
  for(i=1;i<=nrows;i++) {
    xx[i]=0.0;
    yy[i]=0.0;
    if(T[k][i][ib[2]-nrows]>0.0)xx[i]=1.0;
    if(T[k][i][ib[2]-nrows]<0.0) {
      jj++;
      yy[i]=1.0;
      NG[jj]=i;
    }
  }
  dimNG=jj;

 Bdy1:
  i=1;
  while(xx[i]*bz[i]!=1 && i<=nrows)i++;
  if( ( !( elementof(ib[1],BZ,dimBZ)&&elementof(ib[1],NG,dimNG) ) )
      || i<=nrows)goto Findpr;
  if(st_printlev>=4) {
    fprintf(outfl, "\nBdy:k,I,pc=%d,%d,%d",k,I,pc);
  }

  /*	RevNPvt  */
  dr=-1;
  rlex(p_state, p_lex, dr);
  S[1]=I;
  pr=I;
  p_state.del=-1;
  Ps--;
  if (st_printlev >= 4) {
    fprintf(outfl,"\nRvNPvt:Ps,k,pr,pc=%d,%d,%d,%d",Ps,k,pr,pc);
  }
  goto Pivot;
}


void Pivot(AlgorithmState &p_state, int k, int pr, int pc)
{
  int ix,iy;
  double p,pvt,d,d1,d2;

  if(st_printlev>=6)print_tableau(k);
  p_state.del=1;
  /* get index of pivot row = ix
     pivot col = iy */

  if(st_printlev>=4)fprintf(outfl,"=%d,%d",pr,pc);
  ix=1;iy=1;
  while(lbl[k][ix]!=pr && ix <=_m+_n)ix++;
  while(lbl[k][iy]!=pc && iy <=_m+_n)iy++;
  iy-=rows[k];

  if(st_printlev>=5)fprintf(outfl,":%d,%d",ix,iy);
  /* get pivot row = xx
     pivot col = yy, and pivot = pvt */

  for (int i = 1; i <= cols[k]+1; i++) {
    xx[i] = T[k][ix][i];
  }
  for (int i = 1; i <= rows[k]+1; i++) {
    yy[i] = T[k][i][iy];
  }
  pvt=xx[iy];
  p=1.0;
  if(pvt<0.0)p=-1.0;
  if(pvt==0.0)p=0.0;

  d=D[k]*p;
										/* do pivot */
  for (int i1 = 1; i1 <= rows[k]+1; i1++) {
    for (int i2 = 1; i2 <= cols[k]+1; i2++) {
      d1=T[k][i1][i2]*pvt;
      d2=yy[i1]*xx[i2];
      if (approx(d1, d2)) {
	T[k][i1][i2] = 0.0;
      }
      else {
	T[k][i1][i2] = (d1-d2)/d;
      }
     }
  }

  D[k]=fabs(pvt);
  for (int i = 1; i <= cols[k]+1; i++) {
    T[k][ix][i] = xx[i]*p;
  }
  for (int i = 1; i <= rows[k]+1; i++) {
    T[k][i][iy] =- yy[i]*p;
  }
  T[k][ix][iy]=d;

  /* update labels and jvec */
  lbl[k][ix]=pc;
  lbl[k][rows[k]+iy]=pr;

  if (k==1) {
    int i1=1, i2=1;
    while(jvec[k][i1]!=ix)i1++;
    while(jvec[k][i2]!=(rows[k]+iy))i2++;
    jvec[k][i1]=rows[k]+iy;jvec[k][i2]=ix;
  }
  else if (k==2) {
    int i1=1, i2=1;
    while(jvec[k][i1]!=iy)i1++;
    while(jvec[k][i2]!=(cols[k]+ix))i2++;
    jvec[k][i1]=cols[k]+ix;jvec[k][i2]=iy;
  }

  if (st_printlev >= 5) {
    print_tableau(k);
  }
}

void print_tableau(int k)
{
  int i1,i2;
  fprintf(outfl,"\n");
  for(i2=rows[k]+1;i2<=_m+_n;i2++)
    fprintf(outfl,"         %4d   ",lbl[k][i2]);

  for(i1=1;i1<=rows[k]+1;i1++)
    {
      if(i1<=rows[k])
	fprintf(outfl,"\n%4d",lbl[k][i1]);
      else
	fprintf(outfl,"\n    ",lbl[k][i1]);
      for(i2=1;i2<=cols[k]+1;i2++)
	if(st_printlev>=4)fprintf(outfl,"%16.8e",T[k][i1][i2]);
    }
  fprintf(outfl,"\n");
}

int findpr(AlgorithmState &p_state, Gambit::Array<int> &p_lex, int k, int pc)
{
  int l,jj,dimy,dimS,L;
  double maxr;

/* get pivot column */
  int m = 1;
  while (lbl[k][m] != pc && m <= dimlbl) m++;
  jj = m-rows[k];

  for (int i = 1; i <= rows[k]; i++) {
    yy[i] = T[k][i][jj] * p_state.del;
  }

  dimy=rows[k];
  /* get positive elements of pivot column */
 IP2:
  dimS=0;
  for (int i = 1; i <= dimy; i++) {
    if (yy[i] > 0.0) {
      dimS++;
      S[dimS]=lbl[k][i];
      iS[dimS]=i;
    }
  }
  l=0;

 IP3:
  if(dimS==1) return S[1];
  if(dimS==0) return 0;
  l++;
  L=p_lex[l-1];
  if (elementof(L,S,dimS)) {
    S[1]=L;
    dimS=1;
    return S[1];
  }

 IP4:
  if (elementof(L,lbl[k],dimy))  goto IP3;
  jj=1;
  while(L!=lbl[k][jj] && jj<=dimlbl)jj++;
  if(jj>dimlbl)goto IP3;
  for (int i = 1; i <= dimS; i++) {
    xS[i] = T[k][iS[i]][jj-rows[k]];
  }

 IP5:
  maxr=-100.0;
  for (int i = 1; i <= dimS; i++) {
    rr[i]=xS[i]/yy[iS[i]];
    if(rr[i]>=maxr)maxr=rr[i];
  }
  jj=0;
  for (int i = 1; i <= dimS; i++) {
    if (rr[i] == maxr) {
      jj++;
      S[jj]=S[i];
      iS[jj]=iS[i];
    }
  }
  dimS=jj;
  goto IP3;
}

void rlex(AlgorithmState &p_state, Gambit::Array<int> &lx, int dir)
{
  int k,is,ns;

  k=2;
  ns=rows[1];

  if (dir == 1) {
    is=lx[1];
    for (int i = 1; i < ilast; i++) lx[i] = lx[i+1];
    lx[ilast]=is;

    if(is>=MMM){k=1;ns=rows[2];}
    is=jvec[k][2];
    for (int i = 2; i <= ns; i++)  jvec[k][i] = jvec[k][i+1];
    jvec[k][ns+1]=is;
  }
  else if (dir == -1) {
    is=lx[ilast];
    for (int i = ilast; i > 1; i--)  lx[i] = lx[i-1];
    lx[1]=is;

    if(is>=MMM){k=1;ns=rows[2];}
    is=jvec[k][ns+1];
    for(int i = ns+1; i > 2; i--)  jvec[k][i] = jvec[k][i-1];
    jvec[k][2]=is;
  }
}

void lexorder(AlgorithmState &p_state,
	      Gambit::Array<int> &p_lex, int *j, int k)
{
  int i1;

  if (k == 1) {
    for (int i = 1; i <= _m+_n+1; i++) {
      i1=0;
      while(p_lex[i1]!=lbl[k][i])i1++;
      J0[i]=i1;j[i]=i;
    }
  }
  else if (k == 2) {
    for (int i = 1; i <= _m; i++) {
      i1=0;
      while(p_lex[i1]!=lbl[k][i+_n])i1++;
      J0[i]=i1;j[i]=i;
    }
    for (int i = 1; i <= _n; i++) {
      i1=0;
      while(p_lex[i1]!=lbl[k][i])i1++;
      J0[i+_m]=i1;j[i+_m]=i+_m;
    }
    i1=0;
    int i=_m+_n+1;
    while(p_lex[i1]!=lbl[k][i])i1++;
    J0[i]=i1;j[i]=i;
  }
  shell_sort(_m+_n+1, J0,j);
}


void setup(AlgorithmState &p_state)
{
  int i,k,i1,i2;

  for(i=1;i<=_m;i++)
    {
      T0[1][i][_n+1]=-1.0;
      T0[2][_n+1][i]=-1.0;
      lbl0[1][i]=-i;
      lbl0[2][_n+i]=i;
      p_state.lex0[i]=i;
      p_state.lex0[_m+_n+i]=-i;
    }
  for(i=1;i<=_n;i++)
    {
      T0[1][_m+1][i]=-1.0;
      T0[2][i][_m+1]=-1.0;
      lbl0[1][_m+i]=MMM+i;
      lbl0[2][i]=-(MMM+i);
      p_state.lex0[_m+i]=MMM+i;
      p_state.lex0[2*_m+_n+i]=-(MMM+i);
    }

  lbl0[1][_m+_n+1]=0;
  lbl0[2][_m+_n+1]=0;
  T0[1][_m+1][_n+1]=0.0;
  T0[2][_n+1][_m+1]=0.0;
  p_state.lex0[0]=0;
  dimlbl=_m+_n+1;
  dimlex=2*(_m+_n);
  dimx=_m+_n+1;
  D0[1]=D0[2]=1.0;

  if(st_printlev>=4)
    {
      fprintf(outfl,"\nT0 = \n");

      for(k=1;k<=2;k++)
	{
	  for(i1=1;i1<=rows[k]+1;i1++)
	    {
	      for(i2=1;i2<=cols[k]+1;i2++)
		fprintf(outfl,"%11.3e",T0[k][i1][i2]);
	      fprintf(outfl,"\n");
	    }
	  fprintf(outfl,"\n");
	}

      fprintf(outfl,"\nlbl0 = \n");
      for(k=1;k<=2;k++)
	{
	  for(i1=1;i1<=dimlbl;i1++)
	    fprintf(outfl,"%4d",lbl0[k][i1]);
	  fprintf(outfl,"\n");
	}

      fprintf(outfl,"\nm= %d, n= %d, lex0 = ",_m,_n);
      for(i1=0;i1<=dimlex;i1++)
	fprintf(outfl,"%4d",p_state.lex0[i1]);
      fprintf(outfl,"\n");

    }
}


void GetMinPayoffs(const Gambit::Game &p_game, double &p_minA, double &p_minB)
{
  p_minA = 1000000.0;
  p_minB = 1000000.0;

  Gambit::GamePlayer p1 = p_game->GetPlayer(1);
  Gambit::GamePlayer p2 = p_game->GetPlayer(2);

  for (Gambit::StrategyIterator iter(p_game); !iter.AtEnd(); iter++) {
    if (p_minA > iter->GetOutcome()->GetPayoff<double>(1)) {
      p_minA = iter->GetOutcome()->GetPayoff<double>(1);
    }

    if (p_minB > iter->GetOutcome()->GetPayoff<double>(2)) {
      p_minB = iter->GetOutcome()->GetPayoff<double>(2);
    }
  }
}

int *ivector(int nl,int nh)
{
  int *v;

  v=(int *)malloc((long) ((nh-nl+1)*(long)sizeof(int)));
  if (!v) nrerror("allocation failure in ivector()");
  return v-nl;
}

int **imatrix(int nrl,int nrh,int ncl,int nch)
{
  int i;
  int **m;

  m=(int **)malloc((long) ((nrh-nrl+1)*(long)sizeof(int*)));
  if (!m) nrerror("allocation failure 1 in imatrix()");
  m -= nrl;

  for(i=nrl;i<=nrh;i++)
    {
      m[i]=(int *)malloc((long)( (nch-ncl+1)*(long)sizeof(int)));
      if (!m[i]) nrerror("allocation failure 2 in imatrix()");
      m[i] -= ncl;
    }
  return m;
}

double *dvector(int nl,int nh)
{
  double *v;

  v=(double *)malloc((long)( (nh-nl+1)*(long)sizeof(double)));
  if (!v) nrerror("allocation failure in dvector()");
  return v-nl;
}

double **dmatrix(int nrl,int nrh,int ncl,int nch)
{
  int i;
  double **m;

  m=(double **) malloc((long)( (nrh-nrl+1)*(long)sizeof(double*)));
  if (!m) nrerror("allocation failure 1 in dmatrix()");
  m -= nrl;

  for(i=nrl;i<=nrh;i++)
    {
      m[i]=(double *) malloc((long) ((nch-ncl+1)*(long)sizeof(double)));
      if (!m[i]) nrerror("allocation failure 2 in dmatrix()");
      m[i] -= ncl;
    }
  return m;
}

void free_ivector(int *v,int nl,int nh)
{
  (void)free((char*) (v+nl));
}

void free_imatrix(int **m,int nrl,int nrh,int ncl,int nch)
{
  int i;

  for(i=nrh;i>=nrl;i--) (void)free((char*) (m[i]+ncl));
  (void)free((char*) (m+nrl));
}

void free_dvector(double *v,int nl,int nh)
{
  (void)free((char*) (v+nl));
}

void free_dmatrix(double **m,int nrl,int nrh,int ncl,int nch)
{
  int i;

  for(i=nrh;i>=nrl;i--) free((char*) (m[i]+ncl));
  (void)free((char*) (m+nrl));
}

void nrerror(char *error_text)
{

  fprintf(stderr,"Gambit run-time error...\n");
  fprintf(stderr,"%s\n",error_text);
  fprintf(stderr,"...now exiting to system...\n");
  exit(1);
}

double ***dtableau(int nrl,int nrh,int ncl,int nch)
{
  double ***m;

  m=(double ***) malloc((long)( 2*(long)sizeof(double**)));
  if (!m) nrerror("allocation failure 1 in dmatrix()");
  m -= 1;

  m[1]=dmatrix(nrl,nrh,ncl,nch);
  m[2]=dmatrix(ncl,nch,nrl,nrh);

  return m;
}

void free_dtableau(double ***m,int nrl,int nrh,int ncl,int nch)
{
  free_dmatrix(m[1],nrl,nrh,ncl,nch);
  free_dmatrix(m[2],ncl,nch,nrl,nrh);
  (void)free((char*) (m+1));

}

void allocate(AlgorithmState &p_state)
{
  int maxmn;

  maxmn=_m;
  if(_n>_m)maxmn=_n;
  p_state.lex0 = Gambit::Array<int>(0,2*(_m+_n));
  lbl0=imatrix(1,2,1,_m+_n+1);

  D0=dvector(1,2);
  S = Gambit::Array<int>(1, maxmn);
  iS = Gambit::Array<int>(1, maxmn);
  BZ=ivector(1,maxmn);
  bz=ivector(1,maxmn);
  NG=ivector(1,maxmn);
  j=ivector(1,_m+_n+1);
  J0=ivector(1,_m+_n+1);
  xx=dvector(1,_m+_n+1);
  yy=dvector(1,_m+_n+1);
  xS=dvector(1,_m+_n);
  rr=dvector(1,_m+_n);
  T0=dtableau(1,_m+1,1,_n+1);
  jvec=imatrix(1,2,1,_m+_n+1);
}


void de_allocate(AlgorithmState &p_state)
{
  free_dvector(D0,1,2);
  free_imatrix(lbl0,1,2,1,_m+_n+1);
  free_ivector(BZ,1,maxmn);
  free_ivector(bz,1,maxmn);
  free_ivector(NG,1,maxmn);
  free_ivector(j,1,_m+_n+1);
  free_ivector(J0,1,_m+_n+1);
  free_dvector(xx,1,_m+_n+1);
  free_dvector(yy,1,_m+_n+1);
  free_dvector(xS,1,_m+_n);
  free_dvector(rr,1,_m+_n);
  free_dtableau(T0,1,_m+1,1,_n+1);
  free_imatrix(jvec,1,2,1,_m+_n+1);
}

void get_data(AlgorithmState &p_state, const Gambit::Game &p_game)
{
  double minA, minB;
  int n1, n2,i1,i2;

  GetMinPayoffs(p_game, minA, minB);

  _m = p_game->GetPlayer(1)->NumStrategies();
  _n = p_game->GetPlayer(2)->NumStrategies();
  rows[1] = cols[2] = _m;
  rows[2] = cols[1] = _n;

  p_state.dA[1]=0.0;
  p_state.dA[2]=0.0;
  if (minA < 1.0) {
    p_state.dA[1] = 1.0 - minA;
  }

  if (minB < 1.0) {
    p_state.dA[2] = 1.0 - minB;
  }
  if (st_printlev >= 4) {
    fprintf(outfl,
	    "\nAdded %8.4f %8.4f to A B to form tableau",
	    p_state.dA[1], p_state.dA[2]);
  }

  MMM=10*(1+(int)((1+_m)/10));

  allocate(p_state);

  for (Gambit::StrategyIterator iter(p_game); !iter.AtEnd(); iter++) {
    int i1 = iter->GetStrategy(1)->GetNumber();
    int i2 = iter->GetStrategy(2)->GetNumber();
    printf("%d,%d,%f,%f\n", i1, i2,
	   iter->GetOutcome()->GetPayoff<double>(1),
	   iter->GetOutcome()->GetPayoff<double>(2));

    T0[1][i1][i2] = iter->GetOutcome()->GetPayoff<double>(1) + p_state.dA[1];
    T0[2][i2][i1] = iter->GetOutcome()->GetPayoff<double>(2) + p_state.dA[2];
  }
}

void RecursiveSolve(AlgorithmState &p_state,
		    const Gambit::Array<int> &lex1,int **lbl1,double ***T1,double *D1)
{
  int i,i1,i2;
  int **lbl2;
  double ***T2;
  double *D2;

  Gambit::Array<int> lex2(0, 2*(_m+_n));
  lbl2=imatrix(1,2,1,_m+_n+1);
  T2=dtableau(1,_m+1,1,_n+1);
  D2=dvector(1,2);

  lbl=lbl2;
  T=T2;
  D=D2;

  D[1]=D1[1];
  D[2]=D1[2];


  for(i=0;i<=2*(_m+_n);i++)
    lex2[i]=lex1[i];

  for(i=1;i<=2;i++)
    for(i1=1;i1<=_m+_n+1;i1++)
      lbl[i][i1]=lbl1[i][i1];

  for(i=1;i<=2;i++)
    for(i1=1;i1<=rows[i]+1;i1++)
      for(i2=1;i2<=cols[i]+1;i2++)
	T[i][i1][i2]=T1[i][i1][i2];

  get_sol(p_state, lex2);

  if(st_printlev>=3)
    {
      fprintf(outfl,"\n");
      i=1;
      while(i<level){fprintf(outfl,"  ");i++;}
      fprintf(outfl,"level=%d, lex[1]=%d",level,lex2[1]);
    }

  printf("\nEq# %d, level %d, lex # %d, pass %d. ",
	  setnum1, level, lex2[1],count);

  count++;
  /*	if(count>=13)st_printlev=5;
   */
  if(new_component(p_state))
    for(i=1;i<_m+_n;i++)
      {
	rlex(p_state, lex2,1);
	if(level<st_maxlev && setnum1<st_maxnum) {
	  level++;
	  RecursiveSolve(p_state, lex2,lbl2,T2,D2);
	  level--;
	}
      }
  free_dvector(D2,1,2);
  free_dtableau(T2,1,_m+1,1,_n+1);
  free_imatrix(lbl2,1,2,1,_m+_n+1);
}


void ComputeStable(AlgorithmState &p_state)
{
  int i;
  Equilibrium *e;

  setnum=0; setnum1=0; setnum2=0;
  level=0; count=1;

  if(st_maxnum==1) {
    lbl=lbl0; T=T0; D=D0;
    get_sol(p_state, p_state.lex0);
    new_component(p_state);
  }
  else {
    for (i=1;i<=_n+_m;i++) {
      if(level<st_maxlev && setnum1<st_maxnum) {
	level++;
	RecursiveSolve(p_state, p_state.lex0,lbl0,T0,D0);
	level--;
      }
      rlex(p_state, p_state.lex0,1);
    }
  }

  for(e=p_state.firstequi,i=1;e!=NULL;e=e->nextequi,i++)e->equinumber=i;

  fprintf(outfl,"\nSolutions\n");
  p_state.firstequi->Print(outfl);

  de_allocate(p_state);
  //  fclose(outfl);
}


int main(int argc, char *argv[])
{
  int i;

  st_typesol=PRIMAL_STABLE;
  st_maxnum=0; st_maxlev=0; st_printlev=0;
  st_whichkeep=0;

  fprintf(outfl,"\ntypesol:%3d, maxnum:%3d, maxlev:%3d, whichkeep:%3d",
	  st_typesol, st_maxnum, st_maxlev,st_whichkeep);
  fprintf(outfl,"\nprintlev:%3d", st_printlev);

  if(st_maxnum==0)st_maxnum=10000;
  if(st_maxlev==0)st_maxlev=10000;

  try {
    Gambit::Game game = Gambit::ReadGame(std::cin);

    AlgorithmState state;

    get_data(state, game);   /* get lex0, lbl0, T0, and D0 */
    setup(state);

    ComputeStable(state);
    return 0;
  }
  catch (Gambit::InvalidFileException) {
    std::cerr << "Error: Game not in a recognized format.\n";
    return 1;
  }
  catch (...) {
    std::cerr << "Error: An internal error occurred.\n";
    return 1;
  }
}
