/*-------------------------------------------------------------------
Call_Hompack.c    created 9/15/1994         last modified 9/15/1994
                             Birk Huber     (birk@math.cornell.edu
ALL RIGHTS RESERVED

  This File represents the interface between Pelican and Hompacks 
  FIXPNF path tracker. The HOMPACK routines it calls have actually 
  been translated from fortran into c with f2c and then modified a
  little so as not to require the f2c library.

  The two routines the user needs to be aware of are init_HPK 
  which takes a pelican Pvector, converts it to "tableau" format, 
  and initialies all the nescessary global variables to represent 
  the homotopy.   Call_HPK_Hom wich takes a double vector and uses 
  it as a starting point for path continuation.
--------------------------------------------------------------------*/

#include "pelclhpk.h"

#define X(i) (DVref(X,i))

int HPK_cont(Dvector X, int tweak)
{
    int i, ist, dst,N,N1;
    static int iflag,trace,nfe,*pivot,*ipar;
    static double *yold,*a,arcae;
    static double *qr, arclen, *wp, *yp, *tz,*par,*z0,*z1;
    static double ansre, *ypold, *sspar,*alpha,ansae,*w,*y,arcre;
    /* extern int fixpnf_(); IN Homotopies.h */
    
    if (Hom_defd!=1) return 0;

    /* save start of free storage space */
    ist=Itop(); dst=Dtop();

    /* init numbers of real eqs with and without hom param*/
    N=2*Hom_num_vars;
    N1=N+1;

    /* get space for local arrays from store */
    ipar=Ires(1); pivot=Ires(N1); yold=Dres(N1); a=Dres(N1); 
    alpha=Dres(N1); w=Dres(N1); y=Dres(N1); ypold=Dres(N1); 
    sspar=Dres(8); z0=Dres(N1);
    z1=Dres(N1); qr=Dres(N*(N1+1)); wp=Dres(N1); yp=Dres(N1); 
    tz=Dres(N1); par=Dres(1); 

   /* initialize parameters */
    iflag = -2; /* should not be changed switch to tell hompack to do path tracking*/
    arcre = -1.; arcae = -1.; 
         /* errors allowed during normal flow iteration will be 
                      automatically reset to appropriat values later*/
    ansre = Hom_tol;
    ansae = Hom_tol;
    trace = 1;     /* 1 keep a log , 0 dont */
    nfe = 10;      /* I am not sure what this one is for */  
    for(i=0;i<8;i++) sspar[i] = -1.; /* sspar holds a number of flags used to determine
                                       optimal step size, set to -1 will cause hompack
                                       to choose them by its own heuristics */
    Htransform(X); 
    /* print Starting point to Log File*/
    print_proj_trans();      
    for(i=1;i<=N+3;i++) 
#ifdef CHP_PRINT
fprintf(Hom_LogFile,"S %g", X(i));
    fprintf(Hom_LogFile," 0 \n")
#endif
;
    y[0]=X(N+3);/* y0 holds the initial deformation parameter */
    for(i=3;i<=N+2;i++){ /* y1 and on hold the starting coordinates */
       y[i-2]=X(i); 
    }
    fixpnf_(&N, y, &iflag, &arcre, &arcae, &ansre, &ansae, &trace,
             a, &nfe, &arclen, yp, yold, ypold, qr, alpha, tz,
             pivot, w, wp, z0, z1, sspar, par, ipar,
	    tweak);  /* tweak is used to refine step size */
    /*
#ifdef CHP_PRINT 
    fprintf(Hom_OutFile,"Done arclen=%f, LAMBDA=%f, return flag %d\n", arclen,y[0],iflag)
#endif

*/
;
    for(i=1;i<=N;i++) X(i+2)=y[i];
    X(N+3)=y[0];
    Huntransform(X);
 /* print ending point to log file */
#ifdef CHP_PRINT
 fprintf(Hom_LogFile,"E")
#endif
;
 for(i=1;i<=N+3;i++) 
#ifdef CHP_PRINT
fprintf(Hom_LogFile," %g", X(i))
#endif
;
#ifdef CHP_PRINT
 fprintf(Hom_LogFile," 4 1 %d %f %f",iflag,arclen,y[0]);
 fprintf(Hom_LogFile,"\n")
#endif
;

/*free space*/
 Ifree(ist), Dfree(dst);
return 0;
} 

#undef X
