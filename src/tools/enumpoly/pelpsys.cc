/* psys.c */

/*
  This file now houses all the implementation code from files beginning with 
psys in the original distribution.
*/

#include "pelpsys.h"

/**************************************************************************/
/****************** implementation code from psys_aset.c ******************/
/**************************************************************************/

aset psys_to_aset(psys P){
  aset res=nullptr,pnt=nullptr;
  int r=1,j,d,t;
  char  *s,c='a'-1;
  LOCS(2);

  PUSH_LOC(res);
  PUSH_LOC(pnt);
  d=psys_d(P);
  r=psys_r(P);

  res=aset_new(r,d+1);

  FORALL_BLOCK(P,
    psys_Bstart_poly(P,psys_bkno(P));
    t=psys_eq_size(P);
    c++;
    FORALL_MONO(P,
      s=(char *)mem_malloc(6*sizeof(char));
      s[0]=c;
      sprintf(s+1,"%d",t--);
      pnt=aset_new_pt(d+1,s);
      for(j=1;j<=d;j++) aset_pnt_set(pnt,j,*psys_exp(P,j));
      aset_pnt_set(pnt,d+1,*psys_def(P));
      aset_add(res,psys_bkno(P),pnt);
    )
  )  
     
  POP_LOCS();
  return res;
}    


 
#define T(i) (*IVref(T,i))
#define Pcoord(i) (*IVref(pnt_coords(P),i))
psys aset_to_psys(aset A,Ivector T, int seed){
 int i,j,n=0,r=0;
 int R,N,M=0,eqno=0,blkno=0;
 psys Sys;
 node Aptr,C,Cptr,P;
 double t;
 
N=aset_dim(A)-1;

 if ((R=aset_r(A))!=IVlength(T))
      bad_error("Gen_Poly: Aset and Type Vector incompatible");
 
  rand_seed(seed);
 
  /* determine dimensions of psys */
  Aptr=aset_start_cfg(A);
  while((C=aset_next_cfg(&Aptr))!=nullptr){
          r++;
          n+=T(r);
          M+=T(r)*pcfg_npts(C);
  }
  if (n!=N) bad_error("Gen_Poly: bad Type Vector");
  Sys=psys_new(N,M,R);
 
  /* set up semi-mixed structure data */
  Aptr=aset_start_cfg(A);
  while((C=aset_next_cfg(&Aptr))!=nullptr){
    blkno++;
    *psys_block_start(Sys,blkno)=eqno;
    for(i=1;i<=T(blkno);i++){
      Cptr=aset_start_pnt(C);
      P=Car(Cptr);
      eqno++;
      r=0;
      do{ 
        psys_init_mon(Sys);
        t=rand_double(0,1);
        *psys_coef_real(Sys)=cos(2*PI*t);
        *psys_coef_imag(Sys)=sin(2*PI*t);
        *psys_def(Sys)=Pcoord(N+1);
        n=0;
        for(j=1;j<=N;j++){
           *psys_exp(Sys,j)=Pcoord(j);
           n+=*psys_exp(Sys,j);
        }
        if (r<n) r=n;/* calculate actual degree of current poly */
        *psys_homog(Sys)=-n; /* save degree of current monomial */
        psys_save_mon(Sys,eqno);
      }while((P=aset_next_pnt(&Cptr))!=nullptr);
      /* Homogenize */
      psys_set_eqno(Sys,eqno);
      FORALL_MONO(Sys, (*psys_homog(Sys))+=r;)
    }
  }  
  return Sys;
}
#undef Pcoord
#undef T

/* end psys_aset.c */

/**************************************************************************/
/**************** implementation code from psys_binsolve.c ****************/
/**************************************************************************/

/*
**    copyright (c) 1995  Birk Huber
*/

/*
** psys_binsolve.c                             Birkett Huber 
**                                     Created On: 4-2-95
**                                  Last Modified: 8-6-95         
** Use Hermite normal form algorithm and gaussian elimination to 
** solve a simplicial system -- used to find initial solutions in 
** lifting homotopy.
*/

#define IMATRIX_FAST 1

#define CV_Set(V,i,j,re,im) {DMref((V),i,2*(j)-1)=(re);\
                             DMref((V),i,2*(j)  )=(im);}
#define CV_Read(V,i,j)   Complex(DMref(V,i,2*(j)-1),DMref(V,i,2*(j)))
#define CM_Set(M,i,j,re,im) {\
                DMref((M),2*(i)-1,2*(j)-1)=(re);\
                DMref((M),2*(i)  ,2*(j)  )=(re);\
                DMref((M),2*(i)-1,2*(j)  )=-1.0*(im);\
                DMref((M),2*(i)  ,2*(j)-1)=(im);}
#define S(i,j)  (*IMref(S,i,j))
#define U(i,j)  (*IMref(U,i,j))
#define Idx(i)  (*IVref(Idx,i))

/*
** psys_binsolve 
**       Input: a  polynomial vector P.
**      Output: On success a matrix of solutions.
**              On failure the null pointer (and an error message)
**              
*/
node psys_binsolve(psys sys){
 int n,i,j,k;
 int bstart=0,block_size;
 int m=1,row=0,cid;
 xpnt Dsol;
 Dmatrix Coef, Cf, Sol=nullptr;
 Imatrix S,Idx,U=nullptr;
 fcomplex ctmp;
 node Sol_List=nullptr;
 LOCS(1);
 PUSH_LOC(Sol_List);

 n=psys_d(sys);

 /* 
 ** reserve and initialize auxilary space 
 */     
 Coef=Dmatrix_new(2*n,2*n);   /* non-constant monomial coefs  */
 Cf  =Dvector_new(2*n);     /* constant term for each equation */
 S   =Imatrix_new(n,n);   /* non-constant monomials exps in rows */
 U   =Imatrix_new(n,n); /* unitary matrix putting S into HNF */
 Idx =Ivector_new(n); /*controlls enumeration of ith roots of unity */

/*
** initialize coeficient matrices to zero
*/
 for(i=1;i<=2*n;i++){
  for(j=1;j<=2*n;j++){
      DMref(Coef,i,j)=0.0;
  }
  DVref(Cf,i)=0.0;
 }

/*
** Load matrix representation of Equation Coef*X^S = C 
*/
 psys_start_block(sys);
 psys_start_poly(sys);
 do {
   block_size=psys_block_size(sys);
   for(i=1;i<=block_size;i++){
     if (psys_start_mon(sys)==FALSE){
         warning("sys not simplicial in binsys (1)");
         goto cleanup;
     }
     if (i==1) for(k=1;k<=n;k++) Idx(k)=*psys_exp(sys,k);
     CV_Set(Cf,1,bstart+i,-1.0*(*psys_coef_real(sys)),
                        -1.0*(*psys_coef_imag(sys)))
     for(j=1;j<=block_size;j++){
        if (psys_next_mon(sys)==FALSE){
             warning("sys not simplicial in binsys (2)");
             goto cleanup;
        }
        CM_Set(Coef,bstart+i,bstart+j,*psys_coef_real(sys),
                                      *psys_coef_imag(sys))
        if (i==1){
           for(k=1;k<=n;k++){
              S(bstart+j,k)=*psys_exp(sys,k)-Idx(k);
           }
        }
     }
     psys_next_poly(sys);
   }
   bstart+=block_size;
 }while (psys_next_block(sys)==TRUE);

   

 /*
 ** find equivalent binomial sytem X^S=Cf
 */
 Dmatrix_Solve(Coef,Cf,2*n);

 /* 
 ** use hermite normal form to get a triangulated system X^(US)=C^U
 **   (use first row of Coef as temporary storage for calculating C^U)
 */
 for(i=1;i<=n;i++){
   DMref(Coef,1,2*i-1)=DVref(Cf,2*i-1);
   DVref(Cf,2*i-1)=1.0;
   DMref(Coef,1,2*i  )=DVref(Cf,2*i  );
   DVref(Cf,2*i  )=0.0;
 }
 /* replace S by its hermite normal form U*S (upper triangular)*/
 Imatrix_hermite(S,U);
 /* calculate C^U*/
 for(i=1;i<=n;i++){
   for(j=1;j<=n;j++){
     ctmp=Cmul(CV_Read(Cf,1,i),Cpow(CV_Read(Coef,1,j),U(i,j)));
     CV_Set(Cf,1,i,ctmp.r,ctmp.i)
   }
 }

 /* 
 ** Calculate number of solutions to be found 
 ** (determinant of triangular matrix S)
 */
 for(i=1;i<=n;i++){
   m*=(S(i,i));
   Idx(i)=1;
 }

 /*
 ** Reserve space for solutions and initialize them to vector of constant terms
 */
 Sol=Dmatrix_new(m,2*n);
 for(i=1;i<=m;i++){
   for(j=1;j<=2*n;j++){
     DMref(Sol,i,j)=DVref(Cf,j);
   }
 }

 /*
 ** Iterate through all the appropriate roots of unity while reverse solving
 */
 while(++row<=m){
   for(j=n;j>=1;j--){
      ctmp=Cmul(Croot(CV_Read(Sol,row,j),S(j,j)),
                    RootOfOne(Idx(j),S(j,j)));
      CV_Set(Sol,row,j,ctmp.r,ctmp.i)
      for(i=j-1;i>=1;i--){
         ctmp=Cmul(CV_Read(Sol,row,i),
                Cpow(CV_Read(Sol,row,j),-1*(S(i,j))));
         CV_Set(Sol,row,i,ctmp.r,ctmp.i)
      }
   }
   cid=n;
   while(cid<=n && cid >=1){
      if (Idx(cid)<S(cid,cid)){
        (Idx(cid))++;
        cid++;
      }
      else {
        Idx(cid)=0;
         cid--;
      }
   }
   if(cid<1 && row <m) warning("binsys stoping too soon");
 }

 /*
 ** Convert complex n-vector solutions to real 2-nvectors with
 ** and two extra coordinates for the implicit(complex) homogenizing
 ** parameter. and one coordinate for the deformation parameter
 */

 for(i=1;i<=m;i++){
    Dsol=xpnt_new(psys_d(sys));
    /* homog parameter */
    xpnt_h_set(Dsol,Complex(1.0,0.0));
    /* affine coords */
    for(j=1;j<=n;j++){
      xpnt_xi_set(Dsol,j,CV_Read(Sol,i,j));
    }
   /*deformation parameter*/
   xpnt_t_set(Dsol,0.0);
   Sol_List=Cons(atom_new((char *)Dsol,DMTX),Sol_List);
 }

/* 
** clean up and leave
*/
cleanup:
  Imatrix_free(Idx);
  Imatrix_free(S);
  Imatrix_free(U);
  Dmatrix_free(Cf);
  Dmatrix_free(Coef);
  Dmatrix_free(Sol);
  
POP_LOCS();
return Sol_List;
}
#undef CV_Set
#undef CV_Read
#undef CM_Set
#undef S
#undef U
#undef Idx

/* end psys_binsolve.c */

/**************************************************************************/
/******************* implementation code from psys_cont.c ******************/
/**************************************************************************/

/*******************************************************************
**  
**  Projective Newton Path Following                    Birk Huber
**                                                      Dec 31 1995
**  This is based on a description by T.Y. Li given in a lecture
**  at the SEA-95  (Nov 23-17) at CIRM
**
**  This version is written to allow for quick change to run as 
**  C-code. (i.e. only access/change singel elts in arrays).
**  uses auxilary functions norm, and GQR which must be modified to
**  take a basis indicating which collumns to use.
**  
** Changes
**   1: Endgame strategy  use dt=(1-t) if t+dt is > final tol
**      sugested strategy seemed to prevent actual completion.
**   2:(planned) try using a history mechanism so that some number
**     of consecutive good steps are required before increasing
**     dt. (it seems we take a lot of bad steps)
**   3:(planned)use an estimate on the condition number to decide
**     when to abort a path which is becomming singular.
**   4:(possible) put some mechanism to stop infinite loops in
**     the Newton iteration.
**   4:(possible) put in a mechanism to detect and abort paths
**     going to infinity.
*****************************************************************/


/* NOW REDUNDANT
#include <math.h>
#include "psys.h"
*/

/*
**     Controll Parameters
**        
**       PN_dt0         initial stepsize
**       PN_maxdt       maximum stepsize
**       PN_mindt       minimum stepsize
**       PN_scaledt     scaling factor for step size
**       PN_cfac        initial contraction required for approx root
**       PN_NYtol       Stop Newton when |F(X)|<Ntol
**       PN_NDtol       Stop final Newton when |DX|<PN_FDtol
**       PN_Nratio      Stop Newton when |DX|/|F(X)| < PN_Nratio
**       PN_tfinal      Destination Value. (just short of 1)
**       PN_FYtol       Stop final Newton when |F(X)|<PN_FYtol 
**       PN_FDtol       Stop final Newton when |DX|<PN_FDtol
**       PN_Fratio      Stop final Newton when |DX|/|F(X)| < PN_Fratio
**       PN_maxsteps    Give up after PN_maxsteps iterations of main loop
**        
*/

double PN_dt0=.01;        
double PN_maxdt=0.1;        
double PN_mindt=1E-14;  
double PN_scaledt=2;        
double PN_cfac=10;      
double PN_NYtol=1E-8;     
double PN_NDtol=1E-12;     
double PN_Nratio=1E-4;   
double PN_tfinal=1-1E-10;
double PN_FYtol=1E-10;   
double PN_FDtol=1E-14;   
double PN_Fratio = 1E-8; 
int    PN_maxsteps=1000; 
#define Zero_Tol 1E-13
/*
**  Auxilary Macroes/Functions
**
**  Backsolve(R,QY,DX,i,j)
**     R- NxN upper triangular, QY N-vector DX n-vector, i,j integers
**     solve  R*DX=QY  (using i,j as loop counters)
*/

#define Basis(i) (*IVref(Basis,(i)))
#define Z(i)     (*IVref(    Z,(i)))
#define X(i)     (DVref(    X,(i)))
#define Y(i)     (DVref(Y,i))
#define QY(i)    (DVref(QY,i))
#define DX(i)    (DVref(DX,i))
#define XOld(i)  (DVref(XOld,i))
#define R(i,j)   (DMref(R,i,j))
#define QT(i,j)  (DMref(QT,i,j))
#define JC(i,j)  (DMref(JC,i,j))
#define A(i,j)   (DMref(A,i,j))
#define Q(i,j)   (DMref(Q,i,j))


#define Backsolve(R,QY,DX,i,j) \
    for(i=N; i>=1; i--){ \
    DX(i)=0.0;\
    for(j=N;j>i;j--){DX(i)=DX(i)+R(i,j)*DX(j);}\
    DX(i)=(QY(i)-DX(i))/(R(i,i));\
  }

   
double norm(Dvector X){
  int i;
  double abs=0.0,tmp;
  for(i=1;i<=DVlength(X);i++){
        tmp=X(i);
        abs+=(tmp*tmp);
  }
  return sqrt(abs);
}

double BGQR(Dmatrix A,Ivector Basis,int N,Dmatrix Q,Dmatrix R){
  int i,j,k;
  double s,s1,s2,t1,t2, max_e=-1,min_e=-1;
  
  for (i=1;i<=N;i++){
    for (j=1;j<=N;j++) {
       Q(i,j)=0.0;
       R(i,j)=A(i,Basis(j));
    }
    Q(i,i)=1.0;
  }
  for(i=1;i<=N-1;i++){
    for(k=i+1;k<=N;k++){
      if (R(k,i)>Zero_Tol || R(k,i)<-Zero_Tol){
         s2=R(k,i);      s1=R(i,i);
         s=sqrt(s1*s1+s2*s2);
         s1=s1/s;        
         s2=s2/s;
         for(j=1;j<=N;j++){
           t1= s1*R(i,j)+s2*R(k,j);
           t2=-s2*R(i,j)+s1*R(k,j);
           R(i,j)=t1;
           R(k,j)=t2;
           t1= s1*Q(i,j)+s2*Q(k,j);
           t2=-s2*Q(i,j)+s1*Q(k,j);
           Q(i,j)=t1;
           Q(k,j)=t2;
         }
      }
    }
    s1=fabs(R(i,i));
    if (s1>max_e||max_e<0) max_e=s1;
    else if(s1<min_e || min_e<0) min_e=s1;
  }
  return max_e/min_e;
}                        
/*
**       Variable Definition and Initialization
*/

int psys_cont(Dvector X, psys P){
Dvector Y=nullptr, QY=nullptr,DX=nullptr,XOld=nullptr;
Dmatrix JC=nullptr,QT=nullptr,R=nullptr;
Ivector Basis=nullptr,Z=nullptr;
int T;         /* total number of real variables appearing*/
int N;     /* total number of variables for 1 affine patch*/
int tsteps=0;
int i,j;
int max_idx,tmp;
int Rval=0;
double max_c,new_c;
double Nold,Nnew = 0.0;
double arclen=0.0,arcstep;
double dt=PN_dt0;
double Ndx=0.0;
double cond_num;

N=2*psys_d(P);
T=N+3;
if (DVlength(X)!=T) bad_error("Bad Starting point");

Z    =Ivector_new(2);
Basis=Ivector_new(N);
XOld =Dvector_new(T);
DX   =Dvector_new(N);
Y    =Dvector_new(N);
QY   =Dvector_new(N);
JC   =Dmatrix_new(N,T);
QT   =Dmatrix_new(N,N);
R    =Dmatrix_new(N,N);
Z(1)=1; Z(2)=2;
for(i=3;i<=T-1;i++) Basis(i-2)=i;

/*
**      Main Predictor Corrector Loop
*/
while (tsteps<PN_maxsteps && X(T)<=PN_tfinal){
  tsteps=tsteps+1;   

  /* 
  ** End game Step size (THIS DIFFERS FROM LI'S SUGGESTION)
  */
  if (X(T)+dt > PN_tfinal) dt=1-X(T);
  
  /*
  ** Choose Basis:
  ** A) Find complex coordinate with largest norm 
  */
  max_c = X(Z(1))*X(Z(1))+X(Z(2))*X(Z(2));
  max_idx=0;
  for(i=1;i<=N;i=i+2){
    new_c=X(Basis(i))*X(Basis(i))+X(Basis(i+1))*X(Basis(i+1));
    if (new_c>max_c){
      max_c=new_c;
      max_idx=i;
    }
  }
  /*
  ** B) If largest coord is not allready homog param make it so.
  */
  if (max_idx>0){
    tmp=Basis(max_idx);
    Basis(max_idx)=Z(1);
    Z(1)=tmp;
    tmp=Basis(max_idx+1);
    Basis(max_idx+1)=Z(2);
    Z(2)=tmp;
  }

  /*
  **  Store Current Solution: (To allow reset if step is rejected)
  */
  for (i=1;i<=T;i++) XOld(i)=X(i);

  /*
  ** Predictor:  Euler predictor step 
  ** A) Solve system JC(:,Basis)DX=-JC(:,T)
  */
  JC=psys_jac(P,X,JC);
  cond_num=BGQR(JC,Basis,N,QT,R);
  /* 
  ** Form RHS 
  */
  for(i=1;i<=N;i++){
    QY(i)=0.0;
    for(j=1;j<=N;j++){
      QY(i)=QY(i)-QT(i,j)*JC(j,T);
    }
  }
  /* 
  ** Backsolve
  */
  Backsolve(R,QY,DX,i,j)

  /*
  ** B) incriment X(T), and X(Basis).
  */
  X(T)=X(T)+dt;
  for(i=1;i<=N;i++) X(Basis(i))=X(Basis(i))+dt*DX(i);

  /* Corrector First Step */
  Y=psys_eval(P,X,Y);
  Nold=norm(Y);             
  JC=psys_jac(P,X,JC); 
  cond_num=BGQR(JC,Basis,N,QT,R);
  /* form RHS: */
  for(i=1;i<=N;i++){
    QY(i)=0;
    for(j=1;j<=N;j++){
        QY(i)=QY(i)-QT(i,j)*Y(j);
    }
  }
  /* Back solve */
  Backsolve(R,QY,DX,i,j)

  /* update */
  for(i=1; i<=N;i++) X(Basis(i))=X(Basis(i))+DX(i);
  Y=psys_eval(P,X,Y); 
  Nnew=norm(Y);

  /*Decide if performance is o.k.*/
  if (Nnew>(Nold/PN_cfac)&&(Nnew>PN_NYtol)){
    /*if performance is not o.k. decrease step size and try again*/
    if (dt>PN_mindt){
      dt=dt/PN_scaledt;     
      for(i=1;i<=T;i++)X(i)=XOld(i);
    }
    else {
      fprintf(stdout /* was psys_logfile */,"Minimum Step size reached\n");
      printf("Minimum Step size reached\n");
      Rval=1;
      goto cleanup;
    }
    fprintf(stdout /* was psys_logfile */,"Stp=%d, T=%g, dt=%g\n", tsteps,X(T),dt);
  }
  else{     
    /* 
    ** if performance is o.k. run Corrector loop
    ** (Maybe a good idea to limit number of Newton Steps)
    */
    Ndx=norm(DX);
    while (Nnew>PN_NYtol && Ndx>PN_NDtol && Ndx/Nnew > PN_Nratio){
    JC=psys_jac(P,X,JC); 
    cond_num=BGQR(JC,Basis,N,QT,R);
      /* form RHS*/
      for(i=1;i<=N;i++){
        QY(i)=0;
        for(j=1;j<=N;j++) QY(i)=QY(i)-QT(i,j)*Y(j);
      }
      Backsolve(R,QY,DX,i,j)

      for (i=1;i<=N;i++) X(Basis(i))=X(Basis(i))+DX(i);
      Y=psys_eval(P,X,Y); 
      Nnew=norm(Y);
      Ndx=norm(DX);
    }

    /*add size of current step to arclenth approx*/
    arcstep=0.0;
    for(i=1;i<=T;i++) arcstep+=pow(X(i)-XOld(i),(long)2);
    arclen=arclen+sqrt(arcstep);

    /* increase step size if allowed*/
    if (dt <PN_maxdt) dt=dt*PN_scaledt;      
    fprintf(stdout /* was psys_logfile */,"Stp=%d, T=%g, Y=%g, arclen=%f\n", 
           tsteps,X(T),Nnew,arclen);
  }
}

if (tsteps>PN_maxsteps){
 printf(" too many steps taken\n");
 Rval=1;
 goto cleanup;
}
  
/* RUN NEWTON's METHOD TO FINAL TOLERANCE (IF DESIRED).*/
  X(T)=1.0;
  Y=psys_eval(P,X,Y); 
  Nnew=norm(Y);
  if (Nnew>PN_FYtol){
    fprintf(stdout /* was psys_logfile */,
           "Starting final Newton iteration: T=%g,Y=%g\n",X(T),Nnew);
   do {
     JC=psys_jac(P,X,JC); 
     cond_num=BGQR(JC,Basis,N,QT,R);
     /* form RHS: */
     for(i=1;i<=N;i++){
       QY(i)=0;
       for(j=1;j<=N;j++) QY(i)=QY(i)-QT(i,j)*Y(j);
      }
      Backsolve(R,QY,DX,i,j)
      for(i=1;i<=N;i++) X(Basis(i))=X(Basis(i))+DX(i);
      Y=psys_eval(P,X,Y); 
      Nnew=norm(Y);
      Ndx=norm(DX);
      fprintf(stdout /* was psys_logfile */,
        "     Y=%g, Ndx=%g, Ndx/Y=%g cond=%g\n",Nnew,Ndx,Ndx/Nnew,cond_num);
   }while (Nnew>PN_FYtol && Ndx >PN_FDtol && Ndx/Nnew > PN_Fratio);
  }

cleanup:
printf("Stp=%d, T=%g,Y=%g, arclen=%f, FLAG=%d\n", 
           tsteps,X(T),Nnew,arclen,Rval);
Ivector_free(Z);
Ivector_free(Basis);
Dvector_free(XOld);
Dvector_free(DX);
Dvector_free(Y); 
Dvector_free(QY);
Dmatrix_free(JC);
Dmatrix_free(QT);
Dmatrix_free(R);

return Rval;}


/* end psys_cont.c */

/**************************************************************************/
/******************* implementation code from psys_def.c ******************/
/**************************************************************************/

/*
** Psys class definition. 
**    -  square system of equations, listed by blocks
*/
struct psys_t{
    int N;     /* number of variables */
    int Neq;   /* number of equations -- usually Neq==N*/
    int R;     /* number of different support sets */
    int Mmax;  /* Max number of Monomials allowed */
    int M;     /* total number of monomials used */
    int *estart;
    int *bstart;
    int *degrees;
    int *tops;  /* a vector of info desribing system */ 
    int *istore; /* an Mx(N+3) matrix of exponents for monomials*/
 double *dstore; /* and Mx2 matrix of coeficients for monomials */
    int curr_eqn;
    int curr_mon;
    int curr_block;
    int free_mon;
    void **aux;
   Dvector Trans;
};
/* integer and double  and next pointer part of ith monomial*/
#define MonI(P,i) ((P)->istore+(((i)-1)*(((P)->N)+3)))
#define MonD(P,i) ((P)->dstore+((i)-1)*2)
/* access macroes for ith monomial */
#define Mon_coefR(P,i)   ((MonD(P,i))[0])
#define Mon_coefI(P,i)   ((MonD(P,i))[1])
#define Mon_homog(P,i)   ((MonI(P,i))[0])
#define Mon_defv(P,i)    ((MonI(P,i))[1])
#define Mon_next(P,i)    ((MonI(P,i))[2])
#define Mon_exp(P,i,d)   ((MonI(P,i))[2+(d)])
#define Mon_aux(P,i)     (((P)->aux)[i-1])
#define Mon_curr(P)      ((P)->curr_mon)

/* access macroes for ith equation */
#define Eqn_start(P,i)    (((P)->estart)[(i)-1])
#define Eqn_size(P,i) (((P)->tops)[(i)-1])
#define Eqn_curr(P)   ((P)->curr_eqn)
#define Eqn_deg(P,i)    ((P)->degrees[(i)-1])
/* access macroes for the ith block */
#define Blk_start(P,i)    (((P)->bstart)[(i)-1])
#define Blk_size(P,i)     (Blk_start(sys,i+1)-Blk_start(sys,i))
#define Blk_curr(P)       ((P)->curr_block)
/* access macroes for system params */
#define Sys_M(P)       ((P)->M)
#define Sys_Mmax(P)    ((P)->Mmax)
#define Sys_Neq(P)        ((P)->Neq)
#define Sys_N(P)          ((P)->N)
#define Sys_R(P)          ((P)->R)
#define Sys_Mon_New(P)    ((P)->free_mon)

/*
** constructor/destructor functions:
*/
#define MON_SIZE (m*(n+3+4))
#define ISTORE_SIZE  ((MON_SIZE+3*n+r+1))
psys psys_new(int n, int m, int r){
    int i;
    psys res;
    res=(psys)mem_malloc(sizeof(struct psys_t));
    res->istore=(int *)mem_malloc(ISTORE_SIZE*sizeof(int));
    res->dstore=(double *)mem_malloc(2*m*sizeof(double));
    res->aux=(void **)mem_malloc(m*sizeof(void *)); 
    Sys_N(res)=n;
    Sys_Neq(res)=n;
    Sys_R(res)=r;
    Sys_M(res)=0;
    Sys_Mmax(res)=m;
    res->estart=res->istore+MON_SIZE;
    res->tops=res->estart+n;
    res->degrees=res->tops+n;
    res->bstart=res->degrees+n;
    for(i=0;i<ISTORE_SIZE;i++)res->istore[i]=0;
    Blk_start(res,r+1)=n+1;
    /* put all monomials on free list */
    Sys_Mon_New(res)=1;
    for(i=1;i<=m;i++){
        Mon_next(res,i)=i+1;
        Mon_aux(res,i)=nullptr; 
        Mon_coefR(res,i)=0.0;
        Mon_coefI(res,i)=0.0;
    }
    res->curr_mon=0;
    res->curr_eqn=0;
    res->curr_block=0;
    return res;
}

void psys_free(psys sys){
  mem_free((void *)(sys->dstore));
  mem_free((void *)(sys->istore));
  mem_free((void *)(sys->aux)); 
  mem_free((void *)(sys));
}

/*
** two functions for modifying an existing psys by adding
** monomials.
**  pysy_new_mon creates a new monomial in the
**      new_mon field, and sets curr_mon to point to it.
**      It can then be filled with data
**  once monomial has been filled with data psys_save_mon
**  will add it to a specified equation. (should not be used
**  while iterating through monomial list).
*/

int psys_init_mon(psys sys){
    if ((Mon_curr(sys)=(Sys_Mon_New(sys)))!=0) return TRUE;
    else return FALSE;
}

void psys_save_mon(psys sys, int i){
   int tmp=0,j,idx;
   /* calculate  degree of new monomial */
   for(j=1;j<=Sys_N(sys);j++) tmp+=Mon_exp(sys,Sys_Mon_New(sys),j);
  
   if (Eqn_start(sys,i)==0){ /* if first monomial set degree */
         Eqn_deg(sys,i)=tmp;
         Mon_homog(sys,Sys_Mon_New(sys))=0;
   }
   else if (Eqn_deg(sys,i)>=tmp){
        Mon_homog(sys,Sys_Mon_New(sys))=Eqn_deg(sys,i)-tmp;
   }
   else {
     Mon_homog(sys,Sys_Mon_New(sys))=0;
     idx=Eqn_start(sys,i);
     while(idx!=0){
       Mon_homog(sys,idx)+=tmp-Eqn_deg(sys,i);
       idx=Mon_next(sys,idx);
     }
     Eqn_deg(sys,i)=tmp;
   }  
   tmp=Mon_next(sys,Sys_Mon_New(sys));
   Mon_next(sys,Sys_Mon_New(sys))=Eqn_start(sys,i);
   Eqn_start(sys,i)=Sys_Mon_New(sys);
   Sys_Mon_New(sys)=tmp;
   Sys_M(sys)++; 
   Eqn_size(sys,i)++;
}

/*
** Display functions
*/
extern Pring Def_Ring;


psys psys_fprint(FILE *fout,psys sys){
 int i,pct=0,mct=0;
 Blk_curr(sys)=1;
  do {
    Eqn_curr(sys)=Blk_start(sys,Blk_curr(sys));
    do { 
      if (pct++==0)
#ifdef LOG_PRINT
 fprintf(fout,"< ")
#endif
;
      else 
#ifdef LOG_PRINT
fprintf(fout,",\n  ")
#endif
;
      Mon_curr(sys)=Eqn_start(sys,Eqn_curr(sys));
      mct=0;
      do {
        if (mct++!=0) 
#ifdef LOG_PRINT
fprintf(fout," + ")
#endif
;        if (Mon_coefR(sys,Mon_curr(sys))!=0||
             Mon_coefI(sys,Mon_curr(sys))!=0){
          if (Mon_coefI(sys,Mon_curr(sys)) != 0.0)
             fprintf(fout,"(");
          fprintf(fout,"%g", Mon_coefR(sys,Mon_curr(sys)));
          if (Mon_coefI(sys,Mon_curr(sys)) != 0.0) {
	    if (Mon_coefI(sys,Mon_curr(sys))>=0.0)fprintf(fout," + ");
	    fprintf(fout,"%g*I)", Mon_coefI(sys,Mon_curr(sys)));
	  }
          for(i=1;i<=Sys_N(sys);i++){
            if (Mon_exp(sys,Mon_curr(sys),i)!=0){
              fprintf(fout," %s^%d",ring_var(Def_Ring,i-1),
                               Mon_exp(sys,Mon_curr(sys),i));
            }
          }
          if (Mon_defv(sys,Mon_curr(sys))!=0){
             fprintf(fout," %s^%d",ring_def(Def_Ring),
                                     Mon_defv(sys,Mon_curr(sys)));
          }
          /* fprintf(fout,"\n"); */
        }
       }
       while((Mon_curr(sys)=Mon_next(sys,Mon_curr(sys)))!=0);
     }
     while(++Eqn_curr(sys)<Blk_start(sys,Blk_curr(sys)+1));
  }
  while((++Blk_curr(sys))<=Sys_R(sys));
 fprintf(fout," >\n");
return sys;
}   


/*
**  Accessors
*/

int psys_d(psys sys) {return sys->N;}


double *psys_coef_real(psys sys){
 return &(Mon_coefR(sys,Mon_curr(sys)));
}

double *psys_coef_imag(psys sys){
   return &(Mon_coefI(sys,Mon_curr(sys)));
}

int *psys_exp(psys sys,int d){ 
   return &(Mon_exp(sys,Mon_curr(sys),d));
 }

int *psys_homog(psys sys){
   return &((Mon_homog(sys,Mon_curr(sys))));
}

int *psys_def(psys sys){ 
   return &(Mon_defv(sys,Mon_curr(sys)));
}

/*
** Iteration Functions
*/

int psys_start_poly(psys sys){
      Eqn_curr(sys)=1;
      return TRUE;
}

int psys_next_poly(psys sys){ 
     if (++(Eqn_curr(sys))<=Sys_N(sys)) return TRUE;
        else return FALSE;
}

int psys_start_block(psys sys){
      Blk_curr(sys)=1;
      return TRUE;
}

int psys_next_block(psys sys){
     if (++Blk_curr(sys)<=Sys_R(sys)) return TRUE;
        else return FALSE;
}

int psys_set_eqno(psys sys, int eq){
  return (Eqn_curr(sys)=eq);
}

int psys_eqno(psys sys){
  return Eqn_curr(sys);
}

int psys_bkno(psys sys){
  return Blk_curr(sys);
}

int psys_Bstart_poly(psys sys,int i){         
      Eqn_curr(sys)=Blk_start(sys,i);       
      return TRUE;                                   
}                         
 
int psys_Bnext_poly(psys sys,int i){
     if (++(Eqn_curr(sys))<Blk_start(sys,i+1)) return TRUE;
        else return FALSE;                           
}                         

int psys_start_mon(psys sys){
    Mon_curr(sys)=Eqn_start(sys,Eqn_curr(sys));
    if (Mon_curr(sys)!=0) return TRUE;
    return FALSE;
}

int psys_next_mon(psys sys){
      if ((Mon_curr(sys)=Mon_next(sys,Mon_curr(sys)))!=0)return TRUE;
      else return FALSE;
 }

/*
** psys_type -- compute type vector for psys
**              (warning must have block structure set up)
*/
Imatrix psys_type(psys sys){
  Imatrix T;
  int i;
  T=Ivector_new(Sys_R(sys));
  for(i=1;i<=Sys_R(sys);i++) *IVref(T,i)=Blk_size(sys,i);
  return T;
}

/*
** psys_aux  find total number of monomials
*/
void **psys_aux(psys sys){
  return &(Mon_aux(sys,Mon_curr(sys)));
}

/*
** psys_size  find total number of monomials
*/
int psys_size(psys sys){
  return Sys_M(sys);
}

/* 
** finds number of consecutive equations starting with current
** equation share a common support. 
*/
int psys_r(psys sys){ return Sys_R(sys);}
int psys_block_size(psys sys){
    return Blk_size(sys,Blk_curr(sys));
}
/*
** finds number of monomials in current equation
*/
int psys_eq_size(psys sys){                 
    return Eqn_size(sys,Eqn_curr(sys));
} 

int *psys_block_start(psys sys,int i){return &(Blk_start(sys,i));}

/*
** copying a psys
*/
psys psys_copy(psys sys)
  { int i;
    psys res;
    res=psys_new(psys_d(sys),psys_size(sys),psys_r(sys));
    for(i=1;i<=psys_r(sys);i++){
       *psys_block_start(res,i)=*psys_block_start(sys,i);
    }
    FORALL_POLY(sys,
      FORALL_MONO(sys,
        psys_init_mon(res);
        *psys_aux(res)=*psys_aux(sys); 
        *psys_coef_real(res)=*psys_coef_real(sys);
        *psys_coef_imag(res)=*psys_coef_imag(sys);
        *psys_def(res)=*psys_def(sys);
        *psys_homog(res)=*psys_homog(sys);
        for(i=1;i<=psys_d(sys);i++) *psys_exp(res,i)=*psys_exp(sys,i);
        psys_save_mon(res,psys_eqno(sys));
      )
    )
    return res;
 }

/* end psys_def.c */

/**************************************************************************/
/****************** implementation code from psys_eval.c ******************/
/**************************************************************************/

/*
** psys_eval
**       input: psys sys (psys)
**              xpnt   X    (xpnt)
**             Dvector Y
**    effects: Y is resized (if nescessary) and filled with values
**             of polynomials of sys at X.
**    output: Y
**    error conditions: if X and sys are not comatable abort program
*/        
#define Y(i) (DVref(Y,i))
Dvector psys_eval(psys sys, xpnt X, Dvector Y){
  int n,i,eqno=0;
  fcomplex pval,tmp;
  n=psys_d(sys);
  if(xpnt_n(X)!=n) bad_error("bad argument in psys_eval");
  if(Y==nullptr || DVlength(Y)!=2*n) Y=Dmatrix_resize(Y,1,2*n);
  
  for(i=1;i<=2*n;i++) Y(i)=0;
  FORALL_POLY(sys,
    eqno++;
    pval=Complex(0.0,0.0);
    FORALL_MONO(sys,
      tmp=Complex(*psys_coef_real(sys),*psys_coef_imag(sys));
      for(i=1;i<=n;i++){
        tmp=Cmul(tmp,Cpow(xpnt_xi(X,i),*psys_exp(sys,i)));
      }
      tmp=Cmul(tmp,Cpow(xpnt_h(X),*psys_homog(sys)));
      tmp=RCmul(pow(xpnt_t(X),(long)(*psys_def(sys))),tmp);
      pval=Cadd(tmp,pval);
    )
    Y(2*eqno-1)=pval.r;
    Y(2*eqno)=pval.i;
  )
  return Y;
}
#undef Y

#define J(i,j) (DMref(J,i,j))
Dvector psys_jac(psys sys, xpnt X, Dmatrix J){
  int n,i,j,eqno=0;
  fcomplex tmp;
  n=psys_d(sys);
  if(xpnt_n(X)!=n) bad_error("bad argument in psys_eval");
  if(DMrows(J)!=2*n || DMcols(J)!=2*n+3) J=Dmatrix_resize(J,2*n,2*n+3);

  for(i=1;i<=2*n;i++)for(j=1;j<=2*n+3;j++) J(i,j)=0;
  
  FORALL_POLY(sys,
    eqno++;
    FORALL_MONO(sys,
      /* dM/DH */
      if (*psys_homog(sys)!=0){
        tmp=Complex(*psys_coef_real(sys),*psys_coef_imag(sys));
        for(i=1;i<=n;i++){
          tmp=Cmul(tmp,Cpow(xpnt_xi(X,i),*psys_exp(sys,i)));
        }
        tmp=RCmul(*psys_homog(sys),tmp);
        tmp=Cmul(tmp,Cpow(xpnt_h(X),*psys_homog(sys)-1));
        if (*psys_def(sys)!=0)tmp=RCmul(pow(xpnt_t(X),
					    (long)(*psys_def(sys))),tmp);
        J(2*eqno-1,1)+=tmp.r;
        J(2*eqno  ,2)+=tmp.r;
        J(2*eqno-1,2)-=tmp.i;
        J(2*eqno  ,1)+=tmp.i;
      }
     /* DM/DT */
      if (*psys_def(sys)!=0){
        tmp=Complex(*psys_coef_real(sys),*psys_coef_imag(sys));
        for(i=1;i<=n;i++){
          tmp=Cmul(tmp,Cpow(xpnt_xi(X,i),*psys_exp(sys,i)));
        }
        tmp=Cmul(tmp,Cpow(xpnt_h(X),*psys_homog(sys)));
        tmp=RCmul(pow(xpnt_t(X),(long)(*psys_def(sys)-1)),tmp);
        tmp=RCmul(*psys_def(sys),tmp);
        J(2*eqno-1,2*n+3)+=tmp.r;
        J(2*eqno  ,2*n+3)+=tmp.i;                          
      }
     /* DM/DXi*/
     for(j=1;j<=n;j++){
       if (*psys_exp(sys,j)!=0){
          tmp=Complex(*psys_coef_real(sys),*psys_coef_imag(sys));
          tmp=RCmul(*psys_exp(sys,j),tmp);
          tmp=Cmul(tmp,Cpow(xpnt_xi(X,j),(*psys_exp(sys,j))-1));
          for(i=1;i<=n;i++){
           if (i!=j) tmp=Cmul(tmp,Cpow(xpnt_xi(X,i),*psys_exp(sys,i)));
          }
          tmp=Cmul(tmp,Cpow(xpnt_h(X),*psys_homog(sys)));
          if (*psys_def(sys)!=0)tmp=RCmul(pow(xpnt_t(X),
					      (long)(*psys_def(sys))),tmp);
          J(2*eqno-1,2*j+1)+=tmp.r;
          J(2*eqno  ,2*j+2)+=tmp.r;
          J(2*eqno-1,2*j+2)-=tmp.i;
          J(2*eqno  ,2*j+1)+=tmp.i;
       }
     }
    )
  )
  return J;
}                                     
#undef J

/*
** psys_abs
**       input: psys sys (psys)
**              xpnt   X    (xpnt)
**             Dvector Y
**    effects: Y is resized (if nescessary) and filled with absolute
**             values of polynomials of sys at evaluated at X.
**    output: Y
**    error conditions: if X and sys are not comatable abort program
*/                

double psys_abs(psys sys, xpnt X){
  int n,i;
  double d;
  fcomplex pval,tmp;
  n=psys_d(sys);
  if(xpnt_n(X)!=n) bad_error("bad argument in psys_eval");
  d=0;
  FORALL_POLY(sys,
    pval=Complex(0.0,0.0);
    FORALL_MONO(sys,
      tmp=Complex(*psys_coef_real(sys),*psys_coef_imag(sys));
      for(i=1;i<=n;i++)
        tmp=Cmul(tmp,Cpow(xpnt_xi(X,i),*psys_exp(sys,i)));
      tmp=Cmul(tmp,Cpow(xpnt_h(X),*psys_homog(sys)));
      tmp=RCmul(pow(xpnt_t(X),(long)(*psys_def(sys))),tmp);
      pval=Cadd(tmp,pval);
    )
    d+=pval.r*pval.r+pval.i*pval.i;
  )
  return sqrt(d);
}

/*
** moment_sys
**      input: a pvector P
**             a xpnt X 
**                        
**    output:  a Dvector M representing a point on the newton
**             polytope of the system, the sum of the images of
**             the moment maps defined by the supporting Aset of
**             the system P.
**
**    note if M allready has enough space allocated to it will be
**         reused.
*/
#define M(i) DVref(M,i)
#define tmp(i) DVref(tmp,i)
Dvector psys_moment(psys sys,xpnt X, Dvector M){
   int i,n;
   double den,t;
   Dmatrix tmp;
   n=psys_d(sys);
   M=Dmatrix_resize(M,1,n);
   tmp=Dmatrix_new(1,n);
   for(i=1;i<=n;i++) M(i)=0.0;

   FORALL_BLOCK(sys,
     psys_Bstart_poly(sys,psys_bkno(sys));
     for(i=1;i<=n;i++) tmp(i)=0.0;
     den=1.0;
     FORALL_MONO(sys,
       t=1.0;
       for(i=1;i<=n;i++) t*=pow(Cabs(xpnt_xi(X,i)),(long)(*psys_exp(sys,i)));
       t*=pow(Cabs(xpnt_h(X)),(long)(*psys_homog(sys)));
       for(i=1;i<=n;i++) tmp(i)+=(*psys_exp(sys,i)*t);
       den+=t;
     );
     for(i=1;i<=n;i++) M(i)=tmp(i)/den;
   )
Dmatrix_free(tmp);
return M;}
#undef M
#undef tmp

/* end psys_eval.c */

/**************************************************************************/
/******************* implementation code from psys_hom.c ******************/
/**************************************************************************/

static Imatrix Norm=nullptr;

node psys_hom(psys sys, node point_list, int tweak){
  node ptr=point_list;   

#ifdef PSYS_HOM_PRINT
  fprintf(stdout /* was psys_logfile */,"S starting continuation:\n");
  fprintf(stdout /* was psys_logfile */,"N");Imatrix_fprint(stdout /* was psys_logfile */,Norm);
  fprintf(stdout /* was psys_logfile */,"P"); psys_fprint(stdout /* was psys_logfile */,sys);
#endif

  if (Cont_Alg==USE_HOMPACK){
     init_hom(sys);
     while(ptr!=nullptr){
       HPK_cont((Dmatrix)Car(Car(ptr)), tweak);
       ptr=Cdr(ptr);
     }
  }
  else {
    while(ptr!=nullptr){

#ifdef PSYS_HOM_PRINT
      fprintf(stdout /* was psys_logfile */,"P"); psys_fprint(stdout /* was psys_logfile */,sys);
      fprintf(stdout /* was psys_logfile */,"X"); Dvector_fprint(stdout /* was psys_logfile */,(Dmatrix)Car(Car(ptr)));
#endif

      psys_cont((Dmatrix)Car(Car(ptr)),sys);
      ptr=Cdr(ptr);
    }
  }

#ifdef PSYS_HOM_PRINT
  fprintf(stdout /* was psys_logfile */,"Q ending continuation\n");
#endif

  return point_list;
}

node psys_solve(psys sys, Imatrix norm, int tweak){
  psys start,hom;
  node sols=nullptr;
  Norm=norm;
  hom=psys_norm_sub(psys_copy(sys),norm);
  start=psys_lead(hom);
  sols=psys_hom(hom,psys_binsolve(start),tweak);
  Norm=nullptr;
  psys_free(start);
  psys_free(hom);
  return sols;
}

/* end psys_hom.c */

/**************************************************************************/
/******************* implementation code from psys_ops.c ******************/
/**************************************************************************/

/*
**    copyright (c) 1995  Birk Huber
*/

/*
** psys_lead(psys sys)
**      Input:  a psys.
**     Output:  (copies) of lowest terms of pys, with largest monomial
**              gcd removed.
**     side effects:
**     error conditions:
*/
psys psys_lead(psys sys)
  { int i,t,n,m;
    psys res;
    n=psys_d(sys);
    res=psys_new(n,psys_size(sys),psys_r(sys));
    for(i=1;i<=psys_r(sys);i++){ 
       *psys_block_start(res,i)=*psys_block_start(sys,i);
    }
    FORALL_POLY(sys,
    /*
    ** find lowest exponent m of deformation parameter in current
    ** polynomial of input system sys - and copy monomials having
    ** this value over to the output system res
    */
      psys_start_mon(sys);
      m=(*psys_def(sys));
      if (psys_next_mon(sys)==TRUE){
        do{
           if (m>(t=*psys_def(sys))) m=t;
        }
        while(psys_next_mon(sys)==TRUE);
      }

      FORALL_MONO(sys,
        if (*psys_def(sys)==m){
           psys_init_mon(res);
           *psys_aux(res)=*psys_aux(sys);
           *psys_coef_real(res)=*psys_coef_real(sys);
           *psys_coef_imag(res)=*psys_coef_imag(sys);
           for(i=1;i<=n;i++) *psys_exp(res,i)=*psys_exp(sys,i);
           *psys_homog(res) = *psys_homog(sys);
           *psys_def(res)=0;
           psys_save_mon(res,psys_eqno(sys));
        }
      )
    )
    return psys_saturate(res);
  }

/*
** psys_saturate
**       input: psys 
**      output: psys with largest common monomial factor removed.
**      side effects: original psys is equal to output psys.
**      error contitions:
*/
#define V(i) (*IVref(V,i))
psys psys_saturate(psys sys)
  { int i,t,n;
    Ivector V;
    n=psys_d(sys);
    V=Ivector_new(n);
    FORALL_POLY(sys,
     /*
     ** find largest common monomial factor for all
     ** monomials in res (stored by vector V of exponents)
     ** WARNING: will also need to check homog variable
     */
      psys_start_mon(sys);
      for(i=1;i<=n;i++) V(i)=(*psys_exp(sys,i));
      if (psys_next_mon(sys)==TRUE){
        do{
          for(i=1;i<=n;i++){
            if ((t=*psys_exp(sys,i))<V(i)) V(i)=t;
          }
        }
        while(psys_next_mon(sys)==TRUE);
      }
      /*
      ** devide res by common monomial factor (with exponents V)
      */
      FORALL_MONO(sys,
           for(i=1;i<=n;i++) *psys_exp(sys,i)-=V(i);
      )
    )
    Ivector_free(V);
    return sys;
 }
#undef V

/*
** psys_lift:
**    input: a system S (psys)
**           a lifting value l (int)
**    output: original system with deformation parameter for each 
**            monomial set to l
**    side effects: original system equals output system.
*/
 psys psys_lift(psys sys, int l){
  FORALL_POLY(sys,FORALL_MONO(sys,*psys_def(sys)=l;));
 return sys;
 }

/*
** psys_norm_sub
**      input: a polynomial systm sys (psys)
**             a normal           norm (Ivector)
**      output: original system after transform
**              x_i->x_i*t^n_i. (and then removing common powers of t)
**      sidefects: original system equals output system
*/
#define norm(i) (*IVref(norm,i))
psys psys_norm_sub(psys sys,Ivector norm)
  { int i,t,n;
  int m = 0;
  int tog = 0;
    n=psys_d(sys);
    FORALL_POLY(sys,
      tog=0;
      FORALL_MONO(sys,
        t=(norm(n+1))*(*psys_def(sys));
        for(i=1;i<=psys_d(sys);i++)
            t+=(*psys_exp(sys,i))*norm(i);
        *psys_def(sys)=t;
        if (tog==0) { m=t; tog=1;}
        else if (m>t) m=t;
      )
      FORALL_MONO(sys,*psys_def(sys)-=m;)
    )
    return sys;
  }
#undef norm


/* end psys_ops.c */


/**************************************************************************/
/****************** implementation code from psys_scale.c *****************/
/**************************************************************************/

/*
**    copyright (c) 1995  Birk Huber
*/

/*
** scale.c             Birk Huber                    created 4-8-1995
** All Rights Reserved
**
** Very simple program for polynomial system scaling.
**
**   Given a system F_1,...,F_n of polynomial equations
**   in variables X_1,...,X_n  calculates constants d_i
**   and c_i so that the new equations
**      10^{d_i}F_i(10^{c_1}X_1,...,10^{c_n}X_n)
**   will have coefitients as close to one as possible (i.e. the 
**   two norm of the vector of logarithms will be minimized).
**
**   The book on continuation by Morgan describes a similar algorithm,
**   in which variation among the coeficients is also minimized
**   explicitly. This code does not take speciall steps to also 
**   minimize the diferences -- It is sort of taken care of allready
**   by the least squares problem allready. (might add it later need 
**   only new rows in matrix).
*/ 

/* NOW REDUNDANT
#include <stdio.h>
#include <math.h>
#include "psys.h"
*/

/* 
** Array and Polynomial data acces macroes
*/
#define LHS(i,j) (DMref(LHS,i,j))
#define RHS(i)   (DVref(RHS,i))
/* #define   X(i)   (DVref(X,i)) REDEFINITION */
#define CEXP(i)  (*psys_exp(Sys,i))
#define CCR      (*psys_coef_real(Sys))
#define CCI      (*psys_coef_imag(Sys))

/*
** Scale
**      Input: a polynomial system Sys (n-variables n-equations).
**     Output: Vector whoose ith coordinate is the inverse 10^{-c_i}
**             of the variable scaling factor for the i-th variable
**     Side Effect: Sys gets scaled.
*/

Dvector psys_scale(psys Sys){
 int i,j,k,n,row=1,m=0,eqno=0;
 Dmatrix LHS,RHS,X;
 double t,t1,t2,s,s1,s2;

 n=psys_d(Sys);
 m=psys_size(Sys);

/*
** Set up least squares problem: find X minimizing |(LHS)(X)-(RHS)|
**
**  Under scaling 10^d_i*F_i(x_1^e_1,...,x_n^e_n) each monomial m=a*x_1^e_1*...*x_n^e_n
**   becomes 10^{d_i}*a*10^{c_1*e_1+...+c_n*e_n}x_1^{e_1}*...x_n^{e_n}
**   taking logarithms the goal that the new coeficient should be equal to one 
**   becomes a linear condition  d_i+c_1*e_1+...c_n*e_n = -log(a)
**   which gets writen out as a matrix equation for the unknown
**         X=[d_1,...,d_n,c_1,...,c_n]. 
*/
 RHS=Dvector_new(m);
 LHS=Dmatrix_new(m,2*n);
 X=Dvector_new(2*n);

 eqno=0;
 psys_start_poly(Sys);
 do {
   eqno++;
   psys_start_mon(Sys);
   do {
     for(j=1;j<=n;j++) LHS(row,j)=0;
     LHS(row,eqno)=1.0;
     for(j=1;j<=n;j++) LHS(row,n+j)=CEXP(j);
     RHS(row)=-1.0*log10(sqrt(CCR*CCR+CCI*CCI)); 
     row++;
   }
   while(psys_next_mon(Sys)==TRUE);
 }
 while(psys_next_poly(Sys)==TRUE);

/* 
** Use Givens rotations to triangularize LHS,i.e.LHS becomes Q*LHS.(triangular) 
** and also apply same givens rotations to RHS  i.e. RHS becomes Q*RHS.
*/
 for (i=1;i<=2*n;i++){
   for (k=i+1;k<=m;k++){
     if (LHS(k,i)!=0.0){
         /* compute rotation */
         s=sqrt(LHS(i,i)*LHS(i,i)+LHS(k,i)*LHS(k,i));
         s1=LHS(i,i)/s;
         s2=LHS(k,i)/s;
         /* apply rotation to LHS */
         for(j=1;j<=2*n;j++) {
             t1=LHS(i,j);
             t2=LHS(k,j);
             LHS(i,j)=s1*t1+s2*t2;
             LHS(k,j)=-s2*t1+s1*t2;
         }
         /* apply rotation to RHS */
         t1=RHS(i);
         t2=RHS(k);
         RHS(i)=s1*t1+s2*t2;
         RHS(k)=-s2*t1+s1*t2;
     }
   }
 }

/* 
** Back solve   R*X=Y
**   R=top square portion of LHS. (2nx2nupper triangular)
**   Y=top 2n entrees of RHS.
**   results in X which solves original least squares problem.
*/
for(j=2*n;j>=1;j--){
  t=RHS(j);
  for(i=j+1;i<=2*n;i++) t-=LHS(j,i)*X(i);
  t=t/LHS(j,j);
  X(j)=t;
}

/* 
** Actually perform scaling   (X=[d_1,...,d_n,c_1,...,c_n])
**  adjust coeficients to those of new system (exponents remain unchanged)
**         10^{d_i}F_i(10^{c_1}x_1,...,10^{c_n}x_n)
*/
 eqno=0;
 psys_start_poly(Sys);
 do {
   eqno++;
   psys_start_mon(Sys);
   do {
   t=X(eqno);
   for(j=1;j<=n;j++) t+=(X(n+j)*CEXP(j));  
   CCR*=pow(10.0,t);
   CCI*=pow(10.0,t);
   }
   while(psys_next_mon(Sys)==TRUE);
 }
 while(psys_next_poly(Sys)==TRUE);


/* 
** clean-up and return vector defining inverse scaling factors 
** to be used to get solutions for original problem.
*/
Dmatrix_free(LHS);
Dmatrix_free(RHS);
RHS=Dvector_new(n);
for(i=1;i<=n;i++) RHS(i)=pow(10.0,X(n+i));
Dmatrix_free(X);
return RHS;
}


/*
** Undefine Array and Polynomial data access macroes
*/
#undef LHS
#undef RHS
#undef X
#undef CEXP
#undef CCR
#undef CCI


/* end psys_scale.c */
