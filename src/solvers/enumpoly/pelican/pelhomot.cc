/* Homotopies.c */

/*
  This is the implementation code from the various files that used 
to reside in the Cont subdirectory of Pelican0.80/source.
*/

#include "pelhomot.h"

/**********************************************************************/
/*************** implementation code from Hom_params.c ****************/
/**********************************************************************/

double Hom_tol=.000001;  /* tolerence for path tracking */

void print_Hom_params(FILE *outfile){
#ifdef HOM_PRINT
 fprintf(outfile," Hominuation Parameters:\n");
 fprintf(outfile,"      Hom_tol=%g\n",Hom_tol)
#endif
;
}

/* end Hom_params.c */

/************************************************************************/
/***************************** code from fixpnf.c ***********************/
/************************************************************************/

#define TRUE_ (1)
#define FALSE_ (0)
double get_abs_homog();
double coord_r, coord_i;

/* Table of constant values */

static integer c_4 = 4;
static integer c_1 = 1;

/* Subroutine */ int fixpnf_0_(int     n_, 
			       int    *n, 
			       double *y, 
			       int    *iflag, 
			       double *arcre, 
			       double *arcae, 
			       double *ansre, 
			       double *ansae, 
			       int    *trace, 
			       double *a, 
			       int    *nfe, 
			       double *arclen, 
			       double *yp, 
			       double *yold, 
			       double *ypold, 
			       double *qr, 
			       double *alpha, 
			       double *tz, 
			       int    *pivot, 
			       double *w, 
			       double *wp, 
			       double *z0, 
			       double *z1, 
			       double *sspar, 
			       double *par, 
			       int    *ipar,

			       int    tweak) /* added to adjust step size */
{
    /* System generated locals */
    int qr_dim1, qr_offset, i_1, i_2;
    double d_1 /* ,abx UNUSED */;

    /* Builtin functions */
    /*    double sqrt(double); CANT DECLARE BUILTINS UNDER C++ */
    // integer s_wsfe(), do_fio(), e_wsfe(); /* in Hom_params.c these are int's */

    /* Local variables */
    static int nfec;
    static double hold;
    static int iter;
    extern double dnrm2_(integer    *n, 
			 doublereal *dx, 
			 integer    *incx);
    static double h, s;
    static long int crash;
    static int limit;
    extern double d1mach_(integer *);
    static long int start;
    static int nc, iflagc, jw;
    static double abserr, relerr;
    extern /* Subroutine */ int stepnf_(integer    *n, 
					integer    *nfe, 
					integer    *iflag, 
					logical    *start, 
					logical    *crash, 
					doublereal *hold, 
					doublereal *h, 
					doublereal *relerr, 
					doublereal *abserr, 
					doublereal *s, 
					doublereal *y, 
					doublereal *yp, 
					doublereal *yold, 
					doublereal *ypold, 
					doublereal *a, 
					doublereal *qr, 
					doublereal *alpha, 
					doublereal *tz, 
					integer    *pivot, 
					doublereal *w, 
					doublereal *wp, 
					doublereal *z0, 
					doublereal *z1,
					doublereal *sspar, 
					doublereal *par, 
					integer    *ipar);

    static double curtol;

    extern /* Subroutine */ int rootnf_(int    *n, 
					int    *nfe, 
					int    *iflag, 
					double *relerr,
					double *abserr,
					double *y,
					double *yp, 
					double *yold, 
					double *ypold, 
					double *a,
					double *qr, 
					double *alpha, 
					double *tz, 
					int    *pivot, 
					double *w, 
					double *wp, 
					double *par, 
					int    *ipar);
    static int np1;
    static long int polsys;



/* SUBROUTINE  FIXPNF  FINDS A FIXED POINT OR ZERO OF THE */
/* N-DIMENSIONAL VECTOR FUNCTION F(X), OR TRACKS A ZERO CURVE */
/* OF A GENERAL HOMOTOPY MAP RHO(A,LAMBDA,X).  FOR THE FIXED */
/* POINT PROBLEM F(X) IS ASSUMED TO BE A C2 MAP OF SOME BALL */
/* INTO ITSELF.  THE EQUATION  X = F(X)  IS SOLVED BY */
/* FOLLOWING THE ZERO CURVE OF THE HOMOTOPY MAP */

/*  LAMBDA*(X - F(X)) + (1 - LAMBDA)*(X - A)  , */

/* STARTING FROM LAMBDA = 0, X = A.  THE CURVE IS PARAMETERIZED */
/* BY ARC LENGTH S, AND IS FOLLOWED BY SOLVING THE ORDINARY */
/* DIFFERENTIAL EQUATION  D(HOMOTOPY MAP)/DS = 0  FOR */
/* Y(S) = (LAMBDA(S), X(S)) USING A HERMITE CUBIC PREDICTOR AND A */
/* CORRECTOR WHICH RETURNS TO THE ZERO CURVE ALONG THE FLOW NORMAL */
/* TO THE DAVIDENKO FLOW (WHICH CONSISTS OF THE INTEGRAL CURVES OF */
/* D(HOMOTOPY MAP)/DS ). */

/* FOR THE ZERO FINDING PROBLEM F(X) IS ASSUMED TO BE A C2 MAP */
/* SUCH THAT FOR SOME R > 0,  X*F(X) >= 0  WHENEVER NORM(X) = R. */
/* THE EQUATION  F(X) = 0  IS SOLVED BY FOLLOWING THE ZERO CURVE */
/* OF THE HOMOTOPY MAP */

/*   LAMBDA*F(X) + (1 - LAMBDA)*(X - A) */

/* EMANATING FROM LAMBDA = 0, X = A. */

/*  A  MUST BE AN INTERIOR POINT OF THE ABOVE MENTIONED BALLS. */

/* FOR THE CURVE TRACKING PROBLEM RHO(A,LAMBDA,X) IS ASSUMED TO */
/* BE A C2 MAP FROM E**M X [0,1) X E**N INTO E**N, WHICH FOR */
/* ALMOST ALL PARAMETER VECTORS A IN SOME NONEMPTY OPEN SUBSET */
/* OF E**M SATISFIES */

/*  RANK [D RHO(A,LAMBDA,X)/D LAMBDA , D RHO(A,LAMBDA,X)/DX] = N */

/* FOR ALL POINTS (LAMBDA,X) SUCH THAT RHO(A,LAMBDA,X)=0.  IT IS */
/* FURTHER ASSUMED THAT */

/*           RANK [ D RHO(A,0,X0)/DX ] = N  . */

/* WITH A FIXED, THE ZERO CURVE OF RHO(A,LAMBDA,X) EMANATING */
/* FROM  LAMBDA = 0, X = X0  IS TRACKED UNTIL  LAMBDA = 1  BY */
/* SOLVING THE ORDINARY DIFFERENTIAL EQUATION */
/* D RHO(A,LAMBDA(S),X(S))/DS = 0  FOR  Y(S) = (LAMBDA(S), X(S)), */
/* WHERE S IS ARC LENGTH ALONG THE ZERO CURVE.  ALSO THE HOMOTOPY */
/* MAP RHO(A,LAMBDA,X) IS ASSUMED TO BE CONSTRUCTED SUCH THAT */

/*              D LAMBDA(0)/DS > 0  . */


/* FOR THE FIXED POINT AND ZERO FINDING PROBLEMS, THE USER MUST SUPPLY */
/* A SUBROUTINE  F(X,V)  WHICH EVALUATES F(X) AT X AND RETURNS THE */
/* VECTOR F(X) IN V, AND A SUBROUTINE  FJAC(X,V,K)  WHICH RETURNS IN V */
/* THE KTH COLUMN OF THE JACOBIAN MATRIX OF F(X) EVALUATED AT X.  FOR */
/* THE CURVE TRACKING PROBLEM, THE USER MUST SUPPLY A SUBROUTINE */
/*  RHO(A,LAMBDA,X,V,PAR,IPAR)  WHICH EVALUATES THE HOMOTOPY MAP RHO AT */
/* (A,LAMBDA,X) AND RETURNS THE VECTOR RHO(A,LAMBDA,X) IN V, AND A */
/* SUBROUTINE  RHOJAC(A,LAMBDA,X,V,K,PAR,IPAR)  WHICH RETURNS IN V THE KTH
 */
/* COLUMN OF THE N X (N+1) JACOBIAN MATRIX [D RHO/D LAMBDA, D RHO/DX] */
/* EVALUATED AT (A,LAMBDA,X).  FIXPNF  DIRECTLY OR INDIRECTLY USES */
/* THE SUBROUTINES  STEPNF , TANGNF , ROOTNF , ROOT , F (OR  RHO ), */
/* FJAC (OR  RHOJAC ), D1MACH , AND THE BLAS FUNCTIONS  DDOT  AND */
/* DNRM2 .  ONLY  D1MACH  CONTAINS MACHINE DEPENDENT CONSTANTS. */
/* NO OTHER MODIFICATIONS BY THE USER ARE REQUIRED. */


/* ON INPUT: */

/* N  IS THE DIMENSION OF X, F(X), AND RHO(A,LAMBDA,X). */

/* Y  IS AN ARRRAY OF LENGTH  N + 1.  (Y(2),...,Y(N+1)) = A  IS THE */
/*    STARTING POINT FOR THE ZERO CURVE FOR THE FIXED POINT AND */
/*    ZERO FINDING PROBLEMS.  (Y(2),...,Y(N+1)) = X0  FOR THE CURVE */
/*    TRACKING PROBLEM. */

/* IFLAG  CAN BE -2, -1, 0, 2, OR 3.  IFLAG  SHOULD BE 0 ON THE */
/*    FIRST CALL TO  FIXPNF  FOR THE PROBLEM  X=F(X), -1 FOR THE */
/*    PROBLEM  F(X)=0, AND -2 FOR THE PROBLEM  RHO(A,LAMBDA,X)=0. */
/*    IN CERTAIN SITUATIONS  IFLAG  IS SET TO 2 OR 3 BY  FIXPNF, */
/*    AND  FIXPNF  CAN BE CALLED AGAIN WITHOUT CHANGING  IFLAG. */

/* ARCRE , ARCAE  ARE THE RELATIVE AND ABSOLUTE ERRORS, RESPECTIVELY, */
/*    ALLOWED THE NORMAL FLOW ITERATION ALONG THE ZERO CURVE.  IF */
/*    ARC?E .LE. 0.0  ON INPUT IT IS RESET TO  .5*SQRT(ANS?E) . */
/*    NORMALLY  ARC?E SHOULD BE CONSIDERABLY LARGER THAN  ANS?E . */

/* ANSRE , ANSAE  ARE THE RELATIVE AND ABSOLUTE ERROR VALUES USED FOR */
/*    THE ANSWER AT LAMBDA = 1.  THE ACCEPTED ANSWER  Y = (LAMBDA, X) */
/*    SATISFIES */

/*       |Y(1) - 1|  .LE.  ANSRE + ANSAE           .AND. */

/*       ||Z||  .LE.  ANSRE*||X|| + ANSAE          WHERE */

/*    (.,Z) IS THE NEWTON STEP TO Y. */

/* TRACE  IS AN INTEGER SPECIFYING THE LOGICAL I/O UNIT FOR */
/*    INTERMEDIATE OUTPUT.  IF  TRACE .GT. 0  THE POINTS COMPUTED ON */
/*    THE ZERO CURVE ARE WRITTEN TO I/O UNIT  TRACE . */

/* A(1:*)  CONTAINS THE PARAMETER VECTOR  A .  FOR THE FIXED POINT */
/*    AND ZERO FINDING PROBLEMS, A  NEED NOT BE INITIALIZED BY THE */
/*    USER, AND IS ASSUMED TO HAVE LENGTH  N.  FOR THE CURVE */
/*    TRACKING PROBLEM, A  MUST BE INITIALIZED BY THE USER. */

/* YP(1:N+1)  IS A WORK ARRAY CONTAINING THE TANGENT VECTOR TO */
/*    THE ZERO CURVE AT THE CURRENT POINT  Y . */

/* YOLD(1:N+1)  IS A WORK ARRAY CONTAINING THE PREVIOUS POINT FOUND */
/*    ON THE ZERO CURVE. */

/* YPOLD(1:N+1)  IS A WORK ARRAY CONTAINING THE TANGENT VECTOR TO */
/*    THE ZERO CURVE AT  YOLD . */

/* QR(1:N,1:N+2), ALPHA(1:N), TZ(1:N+1), PIVOT(1:N+1) , W(1:N+1) , */
/*    WP(1:N+1) , Z0(1:N+1) , Z1(1:N+1)  ARE ALL WORK ARRAYS USED BY */
/*    STEPNF  TO CALCULATE THE TANGENT VECTORS AND NEWTON STEPS. */

/* SSPAR(1:8) = (LIDEAL, RIDEAL, DIDEAL, HMIN, HMAX, BMIN, BMAX, P)  IS */
/*    A VECTOR OF PARAMETERS USED FOR THE OPTIMAL STEP SIZE ESTIMATION. */
/*    IF  SSPAR(J) .LE. 0.0  ON INPUT, IT IS RESET TO A DEFAULT VALUE */
/*    BY  FIXPNF .  OTHERWISE THE INPUT VALUE OF  SSPAR(J)  IS USED. */
/*    SEE THE COMMENTS BELOW AND IN  STEPNF  FOR MORE INFORMATION ABOUT */
/*    THESE CONSTANTS. */

/* PAR(1:*) AND IPAR(1:*) ARE ARRAYS FOR (OPTIONAL) USER PARAMETERS, */
/*    WHICH ARE SIMPLY PASSED THROUGH TO THE USER WRITTEN SUBROUTINES */
/*    RHO, RHOJAC. */


/* ON OUTPUT: */

/* N , TRACE , A  ARE UNCHANGED. */

/* Y(1) = LAMBDA, (Y(2),...,Y(N+1)) = X, AND Y IS AN APPROXIMATE */
/*    ZERO OF THE HOMOTOPY MAP.  NORMALLY LAMBDA = 1 AND X IS A */
/*    FIXED POINT(ZERO) OF F(X).  IN ABNORMAL SITUATIONS LAMBDA */
/*    MAY ONLY BE NEAR 1 AND X IS NEAR A FIXED POINT(ZERO). */

/* IFLAG = */
/*  -2   CAUSES  FIXPNF  TO INITIALIZE EVERYTHING FOR THE PROBLEM */
/*       RHO(A,LAMBDA,X) = 0 (USE ON FIRST CALL). */

/*  -1   CAUSES  FIXPNF  TO INITIALIZE EVERYTHING FOR THE PROBLEM */
/*       F(X) = 0 (USE ON FIRST CALL). */

/*   0   CAUSES  FIXPNF  TO INITIALIZE EVERYTHING FOR THE PROBLEM */
/*       X = F(X) (USE ON FIRST CALL). */

/*   1   NORMAL RETURN. */

/*   2   SPECIFIED ERROR TOLERANCE CANNOT BE MET.  SOME OR ALL OF */
/*       ARCRE , ARCAE , ANSRE , ANSAE  HAVE BEEN INCREASED TO */
/*       SUITABLE VALUES.  TO CONTINUE, JUST CALL  FIXPNF  AGAIN */
/*       WITHOUT CHANGING ANY PARAMETERS. */

/*   3   STEPNF  HAS BEEN CALLED 1000 TIMES.  TO CONTINUE, CALL */
/*       FIXPNF  AGAIN WITHOUT CHANGING ANY PARAMETERS. */

/*   4   JACOBIAN MATRIX DOES NOT HAVE FULL RANK.  THE ALGORITHM */
/*       HAS FAILED (THE ZERO CURVE OF THE HOMOTOPY MAP CANNOT BE */
/*       FOLLOWED ANY FURTHER). */

/*   5   THE TRACKING ALGORITHM HAS LOST THE ZERO CURVE OF THE */
/*       HOMOTOPY MAP AND IS NOT MAKING PROGRESS.  THE ERROR TOLERANCES */
/*       ARC?E  AND  ANS?E  WERE TOO LENIENT.  THE PROBLEM SHOULD BE */
/*       RESTARTED BY CALLING  FIXPNF  WITH SMALLER ERROR TOLERANCES */
/*       AND  IFLAG = 0 (-1, -2). */

/*   6   THE NORMAL FLOW NEWTON ITERATION IN  STEPNF  OR  ROOTNF */
/*       FAILED TO CONVERGE.  THE ERROR TOLERANCES  ANS?E  MAY BE TOO */
/*       STRINGENT. */

/*   7   ILLEGAL INPUT PARAMETERS, A FATAL ERROR. */

/* ARCRE , ARCAE , ANSRE , ANSAE  ARE UNCHANGED AFTER A NORMAL RETURN */
/*    (IFLAG = 1).  THEY ARE INCREASED TO APPROPRIATE VALUES ON THE */
/*    RETURN  IFLAG = 2 . */

/* NFE  IS THE NUMBER OF FUNCTION EVALUATIONS (= NUMBER OF */
/*    JACOBIAN EVALUATIONS). */

/* ARCLEN  IS THE LENGTH OF THE PATH FOLLOWED. */




/* ***** ARRAY DECLARATIONS. ***** */


/* ***** END OF DIMENSIONAL INFORMATION. ***** */


/* LIMITD  IS AN UPPER BOUND ON THE NUMBER OF STEPS.  IT MAY BE */
/* CHANGED BY CHANGING THE FOLLOWING PARAMETER STATEMENT: */

/* SWITCH FROM THE TOLERANCE  ARC?E  TO THE (FINER) TOLERANCE  ANS?E  IF 
*/
/* THE CURVATURE OF ANY COMPONENT OF  Y  EXCEEDS  CURSW. */



/* :  :  :  :  :  :  :  :  :  :  :  :  :  :  :  :  :  :  :  :  :  :  :  : 
*/
/* SET LOGICAL SWITCH TO REFLECT ENTRY POINT. */
    /* Parameter adjustments */
    --y;
    --a;
    --yp;
    --yold;
    --ypold;
    qr_dim1 = *n;
    qr_offset = qr_dim1 + 1;
    qr -= qr_offset;
    --alpha;
    --tz;
    --pivot;
    --w;
    --wp;
    --z0;
    --z1;
    --sspar;
    --par;
    --ipar;

    /* Function Body */
    switch(n_) {
	case 1: goto L_polynf;
	}

    polsys = FALSE_;
    goto L11;

L_polynf:
    polsys = TRUE_;
L11:

    if (*n <= 0 || *ansre <= (float)0. || *ansae < (float)0.) {
	*iflag = 7;
    }
    if (*iflag >= -2 && *iflag <= 0) {
	goto L20;
    }
    if (*iflag == 2) {
	goto L120;
    }
    if (*iflag == 3) {
	goto L90;
    }
/* ONLY VALID INPUT FOR  IFLAG  IS -2, -1, 0, 2, 3. */
    *iflag = 7;
    return 0;

/* *****  INITIALIZATION BLOCK.  ***** */

L20:
    *arclen = (float)0.;
    if (*arcre <= (float)0.) {
	*arcre = sqrt(*ansre) * (float).5;
    }
    if (*arcae <= (float)0.) {
	*arcae = sqrt(*ansae) * (float).5;
    }
    nc = *n;
    nfec = 0;
    iflagc = *iflag;
    np1 = *n + 1;
/* SET INITIAL CONDITIONS FOR FIRST CALL TO  STEPNF . */
    start = TRUE_;
    crash = FALSE_;
    hold = (float)1.;
    h = (float).1;
    s = (float)0.;
    ypold[1] = (float)1.;
    yp[1] = (float)1.;
    y[1] = (float)0.;
    i_1 = np1;
    for (jw = 2; jw <= i_1; ++jw) {
	ypold[jw] = (float)0.;
	yp[jw] = (float)0.;
/* L40: */
    }
/* SET OPTIMAL STEP SIZE ESTIMATION PARAMETERS. */
/* LET Z[K] DENOTE THE NEWTON ITERATES ALONG THE FLOW NORMAL TO THE */
/* DAVIDENKO FLOW AND Y THEIR LIMIT. */
/* IDEAL CONTRACTION FACTOR:  ||Z[2] - Z[1]|| / ||Z[1] - Z[0]|| */
    if (sspar[1] <= (float)0.) {
	sspar[1] = (float).5;
    }
/* IDEAL RESIDUAL FACTOR:  ||RHO(A, Z[1])|| / ||RHO(A, Z[0])|| */
    if (sspar[2] <= (float)0.) {
	sspar[2] = (float).01;
    }
/* IDEAL DISTANCE FACTOR:  ||Z[1] - Y|| / ||Z[0] - Y|| */
    if (sspar[3] <= (float)0.) {
	sspar[3] = (float).5;
    }
/* MINIMUM STEP SIZE  HMIN . */
    if (sspar[4] <= (float)0.) {
	sspar[4] = (sqrt(*n + (float)1.) + (float)4.) * d1mach_(&c_4);
    }
/* MAXIMUM STEP SIZE  HMAX . */
    if (sspar[5] <= (float)0.) {
	sspar[5] = (float)1.;
    }
/* MINIMUM STEP SIZE REDUCTION FACTOR  BMIN . */
    if (sspar[6] <= (float)0.) {
	sspar[6] = (float).1;
    }
/* MAXIMUM STEP SIZE EXPANSION FACTOR  BMAX . */
    if (sspar[7] <= (float)0.) {
	sspar[7] = (float)3.;
    }
/* ASSUMED OPERATING ORDER  P . */
    if (sspar[8] <= (float)0.) {
	sspar[8] = (float)2.;
    }

    /* Adjustments of step size */
    if (tweak == 1)
      sspar[5] = (float).1; 
    else if (tweak == 2)
      sspar[5] = (float).01;
    else if (tweak == 3)
      sspar[5] = (float).001; 


/* LOAD  A  FOR THE FIXED POINT AND ZERO FINDING PROBLEMS. */
    if (iflagc >= -1) {
	i_1 = np1;
	for (jw = 2; jw <= i_1; ++jw) {
	    a[jw - 1] = y[jw];
/* L60: */
	}
    }
L90:
    limit = 1000;

    /* Adjustment of maximum number of steps for smaller step size */
    if (tweak == 1)
      limit = 10000;
    else if (tweak == 2)
      limit = 100000;
    else if (tweak == 3)
      limit = 1000000;

/* *****  END OF INITIALIZATION BLOCK.  ***** */


/* *****  MAIN LOOP.  ***** */

L120:
    i_1 = limit;
    for (iter = 1; iter <= i_1; ++iter) {
	if (y[1] < (float)0.) {
	    *arclen = s;
	    *iflag = 5;
	    return 0;
	}

/* SET DIFFERENT ERROR TOLERANCE IF THE TRAJECTORY Y(S) HAS ANY HIGH 
*/
/* CURVATURE COMPONENTS. */
/* L140: */
	curtol = hold * 10.;
	relerr = *arcre;
	abserr = *arcae;
	i_2 = np1;
	for (jw = 1; jw <= i_2; ++jw) {
	    if ((d_1 = yp[jw] - ypold[jw], abs((int)d_1)) > curtol) {
		relerr = *ansre;
		abserr = *ansae;
		goto L200;
	    }
/* L160: */
	}

/* TAKE A STEP ALONG THE CURVE. */
L200:
	stepnf_((integer *)&nc, (integer *)&nfec, (integer *)&iflagc, &start, 
		&crash, &hold, &h, &relerr, &abserr, 
		&s, &y[1], &yp[1], &yold[1], &ypold[1], &a[1], &qr[
		qr_offset], &alpha[1], &tz[1], (integer *)&pivot[1], 
		&w[1], &wp[1], &z0[
		1], &z1[1], &sspar[1], &par[1], (integer *)&ipar[1]);
/* PRINT LATEST POINT ON CURVE IF REQUESTED. */
	if (*trace > 0) { 
     print_homog(y+2,&coord_r, &coord_i);
#ifdef HOM_PRINT    
 fprintf(Hom_LogFile,"C %g %g",coord_r,coord_i)
#endif
; 
     for(jw=2; jw<=np1; jw++){
      #ifdef HOM_PRINT
   fprintf(Hom_LogFile," %d = %g",jw,y[jw])
#endif
;
     }
#ifdef HOM_PRINT  
   fprintf(Hom_LogFile," %g",y[1])
#endif
;
     } 
#ifdef HOM_PRINT
     fprintf(Hom_LogFile," 4 0 %d %d %f\n",iter,nfec,s)
#endif
;
	*nfe = nfec;
/* CHECK IF THE STEP WAS SUCCESSFUL. */
	if (iflagc > 0) {
	    *arclen = s;
	    *iflag = iflagc;
	    return 0;
	}
	if (crash) {
/* RETURN CODE FOR ERROR TOLERANCE TOO SMALL. */
	    *iflag = 2;
/* CHANGE ERROR TOLERANCES. */
	    if (*arcre < relerr) {
		*arcre = relerr;
	    }
	    if (*ansre < relerr) {
		*ansre = relerr;
	    }
	    if (*arcae < abserr) {
		*arcae = abserr;
	    }
	    if (*ansae < abserr) {
		*ansae = abserr;
	    }
/* CHANGE LIMIT ON NUMBER OF ITERATIONS. */
	    limit -= iter;
	    return 0;
	}

	if (y[1] >= (float)1.) {

/* USE HERMITE CUBIC INTERPOLATION AND NEWTON ITERATION TO GET THE
 */
/* ANSWER AT LAMBDA = 1.0 . */

/* SAVE  YOLD  FOR ARC LENGTH CALCULATION LATER. */
	    i_2 = np1;
	    for (jw = 1; jw <= i_2; ++jw) {
		z0[jw] = yold[jw];
/* L260: */
	    }
	    rootnf_(&nc, &nfec, &iflagc, ansre, ansae, &y[1], &yp[1], &yold[1],
		    &ypold[1], &a[1], &qr[qr_offset], &alpha[1], &tz[1],
		    &pivot[1], &w[1], &wp[1], &par[1], &ipar[1]);

	    *nfe = nfec;
	    *iflag = 1;
/* SET ERROR FLAG IF  ROOTNF  COULD NOT GET THE POINT ON THE ZERO 
*/
/* CURVE AT  LAMBDA = 1.0  . */
	    if (iflagc > 0) {
		*iflag = iflagc;
	    }
/* CALCULATE FINAL ARC LENGTH. */
	    i_2 = np1;
	    for (jw = 1; jw <= i_2; ++jw) {
		w[jw] = y[jw] - z0[jw];
/* L290: */
	    }
	    *arclen = s - hold + dnrm2_((integer *)&np1, &w[1], &c_1);
	    return 0;
	}

/* FOR POLYNOMIAL SYSTEMS AND THE  POLSYS  HOMOTOPY MAP, */
/* D LAMBDA/DS .GE. 0 NECESSARILY.  THIS CONDITION IS FORCED HERE IF 
*/
/* THE ENTRY POINT WAS  POLYNF . */

/*	if (polsys) {   */
	    if (yp[1] < (float)0.) {
/* REVERSE TANGENT DIRECTION SO D LAMBDA/DS = YP(1) > 0 . */
		i_2 = np1;
		for (jw = 1; jw <= i_2; ++jw) {
		    yp[jw] = -yp[jw];
		    ypold[jw] = yp[jw];
/* L310: */
/*		} */
/* FORCE  STEPNF  TO USE THE LINEAR PREDICTOR FOR THE NEXT STE
P ONLY. */
		start = TRUE_;
	    }
	}

/* L400: */
    }

/* *****  END OF MAIN LOOP.  ***** */

/* LAMBDA HAS NOT REACHED 1 IN 1000 STEPS. */
    *iflag = 3;
    *arclen = s;
    return 0;

} /* fixpnf_ */

/* Subroutine */ int fixpnf_(int    *n, 
			     double *y, 
			     int    *iflag, 
			     double *arcre, 
			     double *arcae, 
			     double *ansre, 
			     double *ansae, 
			     int    *trace, 
			     double *a, 
			     int    *nfe, 
			     double *arclen, 
			     double *yp, 
			     double *yold, 
			     double *ypold, 
			     double *qr, 
			     double *alpha, 
			     double *tz, 
			     int    *pivot, 
			     double *w, 
			     double *wp, 
			     double *z0, 
			     double *z1, 
			     double *sspar, 
			     double *par, 
			     int    *ipar,

			     int    tweak)
{
    return fixpnf_0_(0, n, y, iflag, arcre, arcae, ansre, ansae, trace, a, 
	    nfe, arclen, yp, yold, ypold, qr, alpha, tz, pivot, w, wp, z0, z1,
	     sspar, par, ipar, tweak);
    }

/* Subroutine */ int polynf_(int    *n, 
			     double *y, 
			     int    *iflag, 
			     double *arcre, 
			     double *arcae, 
			     double *ansre, 
			     double *ansae, 
			     int    *trace, 
			     double *a, 
			     int    *nfe, 
			     double *arclen, 
			     double *yp, 
			     double *yold, 
			     double *ypold, 
			     double *qr, 
			     double *alpha, 
			     double *tz, 
			     int    *pivot, 
			     double *w, 
			     double *wp, 
			     double *z0, 
			     double *z1, 
			     double *sspar, 
			     double *par, 
			     int    *ipar,

			     int    tweak)
{
    return fixpnf_0_(1, n, y, iflag, arcre, arcae, ansre, ansae, trace, a, 
	    nfe, arclen, yp, yold, ypold, qr, alpha, tz, pivot, w, wp, z0, z1,
	     sspar, par, ipar, tweak);
    }



/* end fixpnf.c */

/**********************************************************************/
/***************** implementation code from Hom_Mem.c *****************/
/**********************************************************************/

/*--------------------------------------------------------------------
 Workspace and Workspace management macros
-------------------------------------------------------------------*/
#define TOPN 20       /*Assume no more than 20 variables*/
#define TOPM 500      /* Assume no more than 500 monomials total*/
static int Istore[TOPM*(TOPN+1)+3*TOPN+2+TOPM];
static double Dstore[TOPM*2+12*(TOPN+1)+8+TOPN*(TOPN+2)+1];
static int didx=0,iidx=0;

/* initialization for storage  currently static should be
   made dynamic*/

/* access funtions to double storage */
double *Dres(int sz){ int v=didx; didx+=sz; return Dstore+v;}
int Dtop(){return didx;}
int Dfree(int ntop){ didx=ntop; return 0;}

/* access funtions to int storage */
int *Ires(int sz){ int v=iidx; iidx+=sz; return Istore+v;}
int Itop(){return iidx;}
int Ifree(int ntop){ iidx=ntop; return 0;}

/* end Hom_Mem.c */

/**********************************************************************/
/***************** implementation code from d1mach.c ******************/
/**********************************************************************/

/* Table of constant values */

doublereal d1mach_(integer *i)
{
    /* Initialized data */

    static integer sc = 987;
    static struct {
	integer e_1[10];
	doublereal e_2;
    } equiv_4 = { { 0, 1048576, -1, 2146435071, 0, 1017118720, 0, 
		    1018167296, 1352628735, 1070810131 }, 0.0 };



    /* System generated locals */
    doublereal ret_val = 0.0;

    /* Builtin functions */
    /* Subroutine */ // int s_stop();
    // integer s_wsfe(), do_fio(), e_wsfe();

    /* Local variables */
#define log10 ((integer *)&equiv_4 + 8)
#define dmach ((doublereal *)&equiv_4)
#define large ((integer *)&equiv_4 + 2)
#define small ((integer *)&equiv_4)
#define diver ((integer *)&equiv_4 + 6)
#define right ((integer *)&equiv_4 + 4)




/*  DOUBLE-PRECISION MACHINE CONSTANTS */

/*  D1MACH( 1) = B**(EMIN-1), THE SMALLEST POSITIVE MAGNITUDE. */

/*  D1MACH( 2) = B**EMAX*(1 - B**(-T)), THE LARGEST MAGNITUDE. */

/*  D1MACH( 3) = B**(-T), THE SMALLEST RELATIVE SPACING. */

/*  D1MACH( 4) = B**(1-T), THE LARGEST RELATIVE SPACING. */

/*  D1MACH( 5) = LOG10(B) */

/*  TO ALTER THIS FUNCTION FOR A PARTICULAR ENVIRONMENT, */
/*  THE DESIRED SET OF DATA STATEMENTS SHOULD BE ACTIVATED BY */
/*  REMOVING THE C FROM COLUMN 1. */
/*  ON RARE MACHINES A STATIC STATEMENT MAY NEED TO BE ADDED. */
/*  (BUT PROBABLY MORE SYSTEMS PROHIBIT IT THAN REQUIRE IT.) */

/*  FOR IEEE-ARITHMETIC MACHINES (BINARY STANDARD), ONE OF THE FIRST */
/*  TWO SETS OF CONSTANTS BELOW SHOULD BE APPROPRIATE.  IF YOU DO NOT */
/*  KNOW WHICH SET TO USE, TRY BOTH AND SEE WHICH GIVES PLAUSIBLE */
/*  VALUES. */

/*  WHERE POSSIBLE, DECIMAL, OCTAL OR HEXADECIMAL CONSTANTS ARE USED */
/*  TO SPECIFY THE CONSTANTS EXACTLY.  SOMETIMES THIS REQUIRES USING */
/*  EQUIVALENT INTEGER ARRAYS.  IF YOUR COMPILER USES HALF-WORD */
/*  INTEGERS BY DEFAULT (SOMETIMES CALLED INTEGER*2), YOU MAY NEED TO */
/*  CHANGE INTEGER TO INTEGER*4 OR OTHERWISE INSTRUCT YOUR COMPILER */
/*  TO USE FULL-WORD INTEGERS IN THE NEXT 5 DECLARATIONS. */

/*  COMMENTS JUST BEFORE THE END STATEMENT (LINES STARTING WITH *) */
/*  GIVE C SOURCE FOR D1MACH. */




/*     MACHINE CONSTANTS FOR BIG-ENDIAN IEEE ARITHMETIC (BINARY FORMAT) */
/*     MACHINES IN WHICH THE MOST SIGNIFICANT BYTE IS STORED FIRST, */
/*     SUCH AS THE AT&T 3B SERIES, MOTOROLA 68000 BASED MACHINES (E.G. */
/*     SUN 3), AND MACHINES THAT USE SPARC, HP, OR IBM RISC CHIPS. */

/*      DATA SMALL(1),SMALL(2) /    1048576,          0 / */
/*      DATA LARGE(1),LARGE(2) / 2146435071,         -1 / */
/*      DATA RIGHT(1),RIGHT(2) / 1017118720,          0 / */
/*      DATA DIVER(1),DIVER(2) / 1018167296,          0 / */
/*      DATA LOG10(1),LOG10(2) / 1070810131, 1352628735 /, SC/987/ */

/*     MACHINE CONSTANTS FOR LITTLE-ENDIAN (BINARY) IEEE ARITHMETIC */
/*     MACHINES IN WHICH THE LEAST SIGNIFICANT BYTE IS STORED FIRST, */
/*     E.G. IBM PCS AND OTHER MACHINES THAT USE INTEL 80X87 OR DEC */
/*     ALPHA CHIPS. */


/*     MACHINE CONSTANTS FOR AMDAHL MACHINES. */

/*      DATA SMALL(1),SMALL(2) /    1048576,          0 / */
/*      DATA LARGE(1),LARGE(2) / 2147483647,         -1 / */
/*      DATA RIGHT(1),RIGHT(2) /  856686592,          0 / */
/*      DATA DIVER(1),DIVER(2) /  873463808,          0 / */
/*      DATA LOG10(1),LOG10(2) / 1091781651, 1352628735 /, SC/987/ */

/*     MACHINE CONSTANTS FOR THE BURROUGHS 1700 SYSTEM. */

/*      DATA SMALL(1) / ZC00800000 / */
/*      DATA SMALL(2) / Z000000000 / */

/*      DATA LARGE(1) / ZDFFFFFFFF / */
/*      DATA LARGE(2) / ZFFFFFFFFF / */

/*      DATA RIGHT(1) / ZCC5800000 / */
/*      DATA RIGHT(2) / Z000000000 / */

/*      DATA DIVER(1) / ZCC6800000 / */
/*      DATA DIVER(2) / Z000000000 / */

/*      DATA LOG10(1) / ZD00E730E7 / */
/*      DATA LOG10(2) / ZC77800DC0 /, SC/987/ */

/*     MACHINE CONSTANTS FOR THE BURROUGHS 5700 SYSTEM. */

/*      DATA SMALL(1) / O1771000000000000 / */
/*      DATA SMALL(2) / O0000000000000000 / */

/*      DATA LARGE(1) / O0777777777777777 / */
/*      DATA LARGE(2) / O0007777777777777 / */

/*      DATA RIGHT(1) / O1461000000000000 / */
/*      DATA RIGHT(2) / O0000000000000000 / */

/*      DATA DIVER(1) / O1451000000000000 / */
/*      DATA DIVER(2) / O0000000000000000 / */

/*      DATA LOG10(1) / O1157163034761674 / */
/*      DATA LOG10(2) / O0006677466732724 /, SC/987/ */

/*     MACHINE CONSTANTS FOR THE BURROUGHS 6700/7700 SYSTEMS. */

/*      DATA SMALL(1) / O1771000000000000 / */
/*      DATA SMALL(2) / O7770000000000000 / */

/*      DATA LARGE(1) / O0777777777777777 / */
/*      DATA LARGE(2) / O7777777777777777 / */

/*      DATA RIGHT(1) / O1461000000000000 / */
/*      DATA RIGHT(2) / O0000000000000000 / */

/*      DATA DIVER(1) / O1451000000000000 / */
/*      DATA DIVER(2) / O0000000000000000 / */

/*      DATA LOG10(1) / O1157163034761674 / */
/*      DATA LOG10(2) / O0006677466732724 /, SC/987/ */

/*     MACHINE CONSTANTS FOR FTN4 ON THE CDC 6000/7000 SERIES. */

/*      DATA SMALL(1) / 00564000000000000000B / */
/*      DATA SMALL(2) / 00000000000000000000B / */

/*      DATA LARGE(1) / 37757777777777777777B / */
/*      DATA LARGE(2) / 37157777777777777774B / */

/*      DATA RIGHT(1) / 15624000000000000000B / */
/*      DATA RIGHT(2) / 00000000000000000000B / */

/*      DATA DIVER(1) / 15634000000000000000B / */
/*      DATA DIVER(2) / 00000000000000000000B / */

/*      DATA LOG10(1) / 17164642023241175717B / */
/*      DATA LOG10(2) / 16367571421742254654B /, SC/987/ */

/*     MACHINE CONSTANTS FOR FTN5 ON THE CDC 6000/7000 SERIES. */

/*      DATA SMALL(1) / O"00564000000000000000" / */
/*      DATA SMALL(2) / O"00000000000000000000" / */

/*      DATA LARGE(1) / O"37757777777777777777" / */
/*      DATA LARGE(2) / O"37157777777777777774" / */

/*      DATA RIGHT(1) / O"15624000000000000000" / */
/*      DATA RIGHT(2) / O"00000000000000000000" / */

/*      DATA DIVER(1) / O"15634000000000000000" / */
/*      DATA DIVER(2) / O"00000000000000000000" / */

/*      DATA LOG10(1) / O"17164642023241175717" / */
/*      DATA LOG10(2) / O"16367571421742254654" /, SC/987/ */

/*     MACHINE CONSTANTS FOR CONVEX C-1 */

/*      DATA SMALL(1),SMALL(2) / '00100000'X, '00000000'X / */
/*      DATA LARGE(1),LARGE(2) / '7FFFFFFF'X, 'FFFFFFFF'X / */
/*      DATA RIGHT(1),RIGHT(2) / '3CC00000'X, '00000000'X / */
/*      DATA DIVER(1),DIVER(2) / '3CD00000'X, '00000000'X / */
/*      DATA LOG10(1),LOG10(2) / '3FF34413'X, '509F79FF'X /, SC/987/ */

/*     MACHINE CONSTANTS FOR THE CRAY 1, XMP, 2, AND 3. */

/*      DATA SMALL(1) / 201354000000000000000B / */
/*      DATA SMALL(2) / 000000000000000000000B / */

/*      DATA LARGE(1) / 577767777777777777777B / */
/*      DATA LARGE(2) / 000007777777777777776B / */

/*      DATA RIGHT(1) / 376434000000000000000B / */
/*      DATA RIGHT(2) / 000000000000000000000B / */

/*      DATA DIVER(1) / 376444000000000000000B / */
/*      DATA DIVER(2) / 000000000000000000000B / */

/*      DATA LOG10(1) / 377774642023241175717B / */
/*      DATA LOG10(2) / 000007571421742254654B /, SC/987/ */

/*     MACHINE CONSTANTS FOR THE DATA GENERAL ECLIPSE S/200 */

/*     SMALL, LARGE, RIGHT, DIVER, LOG10 SHOULD BE DECLARED */
/*     INTEGER SMALL(4), LARGE(4), RIGHT(4), DIVER(4), LOG10(4) */

/*     NOTE - IT MAY BE APPROPRIATE TO INCLUDE THE FOLLOWING LINE - */
/*     STATIC DMACH(5) */

/*      DATA SMALL/20K,3*0/,LARGE/77777K,3*177777K/ */
/*      DATA RIGHT/31420K,3*0/,DIVER/32020K,3*0/ */
/*      DATA LOG10/40423K,42023K,50237K,74776K/, SC/987/ */

/*     MACHINE CONSTANTS FOR THE HARRIS SLASH 6 AND SLASH 7 */

/*      DATA SMALL(1),SMALL(2) / '20000000, '00000201 / */
/*      DATA LARGE(1),LARGE(2) / '37777777, '37777577 / */
/*      DATA RIGHT(1),RIGHT(2) / '20000000, '00000333 / */
/*      DATA DIVER(1),DIVER(2) / '20000000, '00000334 / */
/*      DATA LOG10(1),LOG10(2) / '23210115, '10237777 /, SC/987/ */

/*     MACHINE CONSTANTS FOR THE HONEYWELL DPS 8/70 SERIES. */

/*      DATA SMALL(1),SMALL(2) / O402400000000, O000000000000 / */
/*      DATA LARGE(1),LARGE(2) / O376777777777, O777777777777 / */
/*      DATA RIGHT(1),RIGHT(2) / O604400000000, O000000000000 / */
/*      DATA DIVER(1),DIVER(2) / O606400000000, O000000000000 / */
/*      DATA LOG10(1),LOG10(2) / O776464202324, O117571775714 /, SC/987/ 
*/

/*     MACHINE CONSTANTS FOR THE IBM 360/370 SERIES, */
/*     THE XEROX SIGMA 5/7/9 AND THE SEL SYSTEMS 85/86. */

/*      DATA SMALL(1),SMALL(2) / Z00100000, Z00000000 / */
/*      DATA LARGE(1),LARGE(2) / Z7FFFFFFF, ZFFFFFFFF / */
/*      DATA RIGHT(1),RIGHT(2) / Z33100000, Z00000000 / */
/*      DATA DIVER(1),DIVER(2) / Z34100000, Z00000000 / */
/*      DATA LOG10(1),LOG10(2) / Z41134413, Z509F79FF /, SC/987/ */

/*     MACHINE CONSTANTS FOR THE INTERDATA 8/32 */
/*     WITH THE UNIX SYSTEM FORTRAN 77 COMPILER. */

/*     FOR THE INTERDATA FORTRAN VII COMPILER REPLACE */
/*     THE Z'S SPECIFYING HEX CONSTANTS WITH Y'S. */

/*      DATA SMALL(1),SMALL(2) / Z'00100000', Z'00000000' / */
/*      DATA LARGE(1),LARGE(2) / Z'7EFFFFFF', Z'FFFFFFFF' / */
/*      DATA RIGHT(1),RIGHT(2) / Z'33100000', Z'00000000' / */
/*      DATA DIVER(1),DIVER(2) / Z'34100000', Z'00000000' / */
/*      DATA LOG10(1),LOG10(2) / Z'41134413', Z'509F79FF' /, SC/987/ */

/*     MACHINE CONSTANTS FOR THE PDP-10 (KA PROCESSOR). */

/*      DATA SMALL(1),SMALL(2) / "033400000000, "000000000000 / */
/*      DATA LARGE(1),LARGE(2) / "377777777777, "344777777777 / */
/*      DATA RIGHT(1),RIGHT(2) / "113400000000, "000000000000 / */
/*      DATA DIVER(1),DIVER(2) / "114400000000, "000000000000 / */
/*      DATA LOG10(1),LOG10(2) / "177464202324, "144117571776 /, SC/987/ 
*/

/*     MACHINE CONSTANTS FOR THE PDP-10 (KI PROCESSOR). */

/*      DATA SMALL(1),SMALL(2) / "000400000000, "000000000000 / */
/*      DATA LARGE(1),LARGE(2) / "377777777777, "377777777777 / */
/*      DATA RIGHT(1),RIGHT(2) / "103400000000, "000000000000 / */
/*      DATA DIVER(1),DIVER(2) / "104400000000, "000000000000 / */
/*      DATA LOG10(1),LOG10(2) / "177464202324, "047674776746 /, SC/987/ 
*/

/*     MACHINE CONSTANTS FOR PDP-11 FORTRANS SUPPORTING */
/*     32-BIT INTEGERS (EXPRESSED IN INTEGER AND OCTAL). */

/*      DATA SMALL(1),SMALL(2) /    8388608,           0 / */
/*      DATA LARGE(1),LARGE(2) / 2147483647,          -1 / */
/*      DATA RIGHT(1),RIGHT(2) /  612368384,           0 / */
/*      DATA DIVER(1),DIVER(2) /  620756992,           0 / */
/*      DATA LOG10(1),LOG10(2) / 1067065498, -2063872008 /, SC/987/ */

/*      DATA SMALL(1),SMALL(2) / O00040000000, O00000000000 / */
/*      DATA LARGE(1),LARGE(2) / O17777777777, O37777777777 / */
/*      DATA RIGHT(1),RIGHT(2) / O04440000000, O00000000000 / */
/*      DATA DIVER(1),DIVER(2) / O04500000000, O00000000000 / */
/*      DATA LOG10(1),LOG10(2) / O07746420232, O20476747770 /, SC/987/ */

/*     MACHINE CONSTANTS FOR PDP-11 FORTRANS SUPPORTING */
/*     16-BIT INTEGERS (EXPRESSED IN INTEGER AND OCTAL). */

/*     SMALL, LARGE, RIGHT, DIVER, LOG10 SHOULD BE DECLARED */
/*     INTEGER SMALL(4), LARGE(4), RIGHT(4), DIVER(4), LOG10(4) */

/*      DATA SMALL(1),SMALL(2) /    128,      0 / */
/*      DATA SMALL(3),SMALL(4) /      0,      0 / */

/*      DATA LARGE(1),LARGE(2) /  32767,     -1 / */
/*      DATA LARGE(3),LARGE(4) /     -1,     -1 / */

/*      DATA RIGHT(1),RIGHT(2) /   9344,      0 / */
/*      DATA RIGHT(3),RIGHT(4) /      0,      0 / */

/*      DATA DIVER(1),DIVER(2) /   9472,      0 / */
/*      DATA DIVER(3),DIVER(4) /      0,      0 / */

/*      DATA LOG10(1),LOG10(2) /  16282,   8346 / */
/*      DATA LOG10(3),LOG10(4) / -31493, -12296 /, SC/987/ */

/*      DATA SMALL(1),SMALL(2) / O000200, O000000 / */
/*      DATA SMALL(3),SMALL(4) / O000000, O000000 / */

/*      DATA LARGE(1),LARGE(2) / O077777, O177777 / */
/*      DATA LARGE(3),LARGE(4) / O177777, O177777 / */

/*      DATA RIGHT(1),RIGHT(2) / O022200, O000000 / */
/*      DATA RIGHT(3),RIGHT(4) / O000000, O000000 / */

/*      DATA DIVER(1),DIVER(2) / O022400, O000000 / */
/*      DATA DIVER(3),DIVER(4) / O000000, O000000 / */

/*      DATA LOG10(1),LOG10(2) / O037632, O020232 / */
/*      DATA LOG10(3),LOG10(4) / O102373, O147770 /, SC/987/ */

/*     MACHINE CONSTANTS FOR THE PRIME 50 SERIES SYSTEMS */
/*     WITH 32-BIT INTEGERS AND 64V MODE INSTRUCTIONS, */
/*     SUPPLIED BY IGOR BRAY. */

/*      DATA SMALL(1),SMALL(2) / :10000000000, :00000100001 / */
/*      DATA LARGE(1),LARGE(2) / :17777777777, :37777677775 / */
/*      DATA RIGHT(1),RIGHT(2) / :10000000000, :00000000122 / */
/*      DATA DIVER(1),DIVER(2) / :10000000000, :00000000123 / */
/*      DATA LOG10(1),LOG10(2) / :11504046501, :07674600177 /, SC/987/ */

/*     MACHINE CONSTANTS FOR THE SEQUENT BALANCE 8000 */

/*      DATA SMALL(1),SMALL(2) / $00000000,  $00100000 / */
/*      DATA LARGE(1),LARGE(2) / $FFFFFFFF,  $7FEFFFFF / */
/*      DATA RIGHT(1),RIGHT(2) / $00000000,  $3CA00000 / */
/*      DATA DIVER(1),DIVER(2) / $00000000,  $3CB00000 / */
/*      DATA LOG10(1),LOG10(2) / $509F79FF,  $3FD34413 /, SC/987/ */

/*     MACHINE CONSTANTS FOR THE UNIVAC 1100 SERIES. */

/*      DATA SMALL(1),SMALL(2) / O000040000000, O000000000000 / */
/*      DATA LARGE(1),LARGE(2) / O377777777777, O777777777777 / */
/*      DATA RIGHT(1),RIGHT(2) / O170540000000, O000000000000 / */
/*      DATA DIVER(1),DIVER(2) / O170640000000, O000000000000 / */
/*      DATA LOG10(1),LOG10(2) / O177746420232, O411757177572 /, SC/987/ 
*/

/*     MACHINE CONSTANTS FOR THE VAX UNIX F77 COMPILER */

/*      DATA SMALL(1),SMALL(2) /        128,           0 / */
/*      DATA LARGE(1),LARGE(2) /     -32769,          -1 / */
/*      DATA RIGHT(1),RIGHT(2) /       9344,           0 / */
/*      DATA DIVER(1),DIVER(2) /       9472,           0 / */
/*      DATA LOG10(1),LOG10(2) /  546979738,  -805796613 /, SC/987/ */

/*     MACHINE CONSTANTS FOR THE VAX-11 WITH */
/*     FORTRAN IV-PLUS COMPILER */

/*      DATA SMALL(1),SMALL(2) / Z00000080, Z00000000 / */
/*      DATA LARGE(1),LARGE(2) / ZFFFF7FFF, ZFFFFFFFF / */
/*      DATA RIGHT(1),RIGHT(2) / Z00002480, Z00000000 / */
/*      DATA DIVER(1),DIVER(2) / Z00002500, Z00000000 / */
/*      DATA LOG10(1),LOG10(2) / Z209A3F9A, ZCFF884FB /, SC/987/ */

/*     MACHINE CONSTANTS FOR VAX/VMS VERSION 2.2 */

/*      DATA SMALL(1),SMALL(2) /       '80'X,        '0'X / */
/*      DATA LARGE(1),LARGE(2) / 'FFFF7FFF'X, 'FFFFFFFF'X / */
/*      DATA RIGHT(1),RIGHT(2) /     '2480'X,        '0'X / */
/*      DATA DIVER(1),DIVER(2) /     '2500'X,        '0'X / */
/*      DATA LOG10(1),LOG10(2) / '209A3F9A'X, 'CFF884FB'X /, SC/987/ */

/*  ***  ISSUE STOP 779 IF ALL DATA STATEMENTS ARE COMMENTED... */
    if (sc != 987) {
/*	s_stop("779", 3L); */exit(0);
    }
/*  ***  ISSUE STOP 778 IF ALL DATA STATEMENTS ARE OBVIOUSLY WRONG... */
    if (dmach[3] >= 1.) {
/*	s_stop("778", 3L); */ exit(0);
    }
    if (*i < 1 || *i > 5) {
	goto L999;
    }
    ret_val = dmach[*i - 1];
    return ret_val;
L999:
/*    s_wsfe(&io___8);
    do_fio(&c_1, (char *)&(*i), (ftnlen)sizeof(integer));
    e_wsfe();
    s_stop("", 0L); */
    return ret_val;
} /* d1mach_ */

#undef right
#undef diver
#undef small
#undef large
#undef dmach
#undef log10

/* end d1mach.c */

/**********************************************************************/
/***************** implementation code from daxpy.c *******************/
/**********************************************************************/

/* Subroutine */ int daxpy_(integer    *n, 
			    doublereal *da, 
			    doublereal *dx, 
			    integer    *incx, 
			    doublereal *dy, 
			    integer    *incy)
{
    /* System generated locals */
    integer i_1;

    /* Local variables */
    static integer i, m, ix, iy, mp1;


/*     constant times a vector plus a vector. */
/*     uses unrolled loops for increments equal to one. */
/*     jack dongarra, linpack, 3/11/78. */


    /* Parameter adjustments */
    --dy;
    --dx;

    /* Function Body */
    if (*n <= 0) {
	return 0;
    }
    if (*da == 0.) {
	return 0;
    }
    if (*incx == 1 && *incy == 1) {
	goto L20;
    }

/*        code for unequal increments or equal increments */
/*          not equal to 1 */

    ix = 1;
    iy = 1;
    if (*incx < 0) {
	ix = (-(*n) + 1) * *incx + 1;
    }
    if (*incy < 0) {
	iy = (-(*n) + 1) * *incy + 1;
    }
    i_1 = *n;
    for (i = 1; i <= i_1; ++i) {
	dy[iy] += *da * dx[ix];
	ix += *incx;
	iy += *incy;
/* L10: */
    }
    return 0;

/*        code for both increments equal to 1 */


/*        clean-up loop */

L20:
    m = *n % 4;
    if (m == 0) {
	goto L40;
    }
    i_1 = m;
    for (i = 1; i <= i_1; ++i) {
	dy[i] += *da * dx[i];
/* L30: */
    }
    if (*n < 4) {
	return 0;
    }
L40:
    mp1 = m + 1;
    i_1 = *n;
    for (i = mp1; i <= i_1; i += 4) {
	dy[i] += *da * dx[i];
	dy[i + 1] += *da * dx[i + 1];
	dy[i + 2] += *da * dx[i + 2];
	dy[i + 3] += *da * dx[i + 3];
/* L50: */
    }
    return 0;
} /* daxpy_ */

/* end daxpy.c */

/**********************************************************************/
/***************** implementation code from dcopy.c *******************/
/**********************************************************************/

/* Subroutine */ int dcopy_(integer    *n, 
			    doublereal *dx, 
			    integer    *incx, 
			    doublereal *dy, 
			    integer    *incy)
{
    /* System generated locals */
    integer i_1;

    /* Local variables */
    static integer i, m, ix, iy, mp1;


/*     copies a vector, x, to a vector, y. */
/*     uses unrolled loops for increments equal to one. */
/*     jack dongarra, linpack, 3/11/78. */


    /* Parameter adjustments */
    --dy;
    --dx;

    /* Function Body */
    if (*n <= 0) {
	return 0;
    }
    if (*incx == 1 && *incy == 1) {
	goto L20;
    }

/*        code for unequal increments or equal increments */
/*          not equal to 1 */

    ix = 1;
    iy = 1;
    if (*incx < 0) {
	ix = (-(*n) + 1) * *incx + 1;
    }
    if (*incy < 0) {
	iy = (-(*n) + 1) * *incy + 1;
    }
    i_1 = *n;
    for (i = 1; i <= i_1; ++i) {
	dy[iy] = dx[ix];
	ix += *incx;
	iy += *incy;
/* L10: */
    }
    return 0;

/*        code for both increments equal to 1 */


/*        clean-up loop */

L20:
    m = *n % 7;
    if (m == 0) {
	goto L40;
    }
    i_1 = m;
    for (i = 1; i <= i_1; ++i) {
	dy[i] = dx[i];
/* L30: */
    }
    if (*n < 7) {
	return 0;
    }
L40:
    mp1 = m + 1;
    i_1 = *n;
    for (i = mp1; i <= i_1; i += 7) {
	dy[i] = dx[i];
	dy[i + 1] = dx[i + 1];
	dy[i + 2] = dx[i + 2];
	dy[i + 3] = dx[i + 3];
	dy[i + 4] = dx[i + 4];
	dy[i + 5] = dx[i + 5];
	dy[i + 6] = dx[i + 6];
/* L50: */
    }
    return 0;
} /* dcopy_ */

/* end dcopy.c */

/**********************************************************************/
/***************** implementation code from dcpose.c ******************/
/**********************************************************************/

/* Table of constant values */

/* static integer c_1 = 1; NOW REDUNDANT */

/* Subroutine */ int dcpose_(integer    *ndim, 
			     integer    *n, 
			     doublereal *qr, 
			     doublereal *alpha, 
			     integer    *pivot, 
			     integer    *ierr, 
			     doublereal *y, 
			     doublereal *sum)
{
    /* System generated locals */
    integer qr_dim1, qr_offset, i_1, i_2, i_3;
    doublereal d_1;

    /* Builtin functions */
    /*     double sqrt(double);  CANT DECLARE BUILTINS UNDER C++ */

    /* Local variables */
    static doublereal beta;
    static integer jbar;
    extern doublereal ddot_(integer    *n,
		 doublereal *dx,
		 integer    *incx,
		 doublereal *dy,
		 integer    *incy);
    static doublereal qrkk;
    static integer i, j, k;
    static doublereal sigma, alphak;
    static integer kp1, np1;


/* SUBROUTINE  DCPOSE  IS A MODIFICATION OF THE ALGOL PROCEDURE */
/* DECOMPOSE  IN P. BUSINGER AND G. H. GOLUB, LINEAR LEAST */
/* SQUARES SOLUTIONS BY HOUSEHOLDER TRANSFORMATIONS, */
/* NUMER. MATH. 7 (1965) 269-276. */

    /* Parameter adjustments */
    qr_dim1 = *ndim;
    qr_offset = qr_dim1 + 1;
    qr -= qr_offset;
    --alpha;
    --pivot;
    --y;
    --sum;

    /* Function Body */
    *ierr = 0;
    np1 = *n + 1;
    i_1 = np1;
    for (j = 1; j <= i_1; ++j) {
	sum[j] = ddot_(n, &qr[j * qr_dim1 + 1], &c_1, &qr[j * qr_dim1 + 1], &
		c_1);
/* L20: */
	pivot[j] = j;
    }
    i_1 = *n;
    for (k = 1; k <= i_1; ++k) {
	sigma = sum[k];
	jbar = k;
	kp1 = k + 1;
	i_2 = np1;
	for (j = kp1; j <= i_2; ++j) {
	    if (sigma >= sum[j]) {
		goto L40;
	    }
	    sigma = sum[j];
	    jbar = j;
L40:
	    ;
	}
	if (jbar == k) {
	    goto L70;
	}
	i = pivot[k];
	pivot[k] = pivot[jbar];
	pivot[jbar] = i;
	sum[jbar] = sum[k];
	sum[k] = sigma;
	i_2 = *n;
	for (i = 1; i <= i_2; ++i) {
	    sigma = qr[i + k * qr_dim1];
	    qr[i + k * qr_dim1] = qr[i + jbar * qr_dim1];
	    qr[i + jbar * qr_dim1] = sigma;
/* L50: */
	}
/*   END OF COLUMN INTERCHANGE. */
L70:
	i_2 = *n - k + 1;
	sigma = ddot_(&i_2, &qr[k + k * qr_dim1], &c_1, &qr[k + k * qr_dim1]
		, &c_1);
	if (sigma != (float)0.) {
	    goto L60;
	}
	*ierr = 1;
	return 0;
L60:
	if (k == *n) {
	    goto L500;
	}
	qrkk = qr[k + k * qr_dim1];
	alphak = -sqrt(sigma);
	if (qrkk < (float)0.) {
	    alphak = -alphak;
	}
	alpha[k] = alphak;
	beta = (float)1. / (sigma - qrkk * alphak);
	qr[k + k * qr_dim1] = qrkk - alphak;
	i_2 = np1;
	for (j = kp1; j <= i_2; ++j) {
/* L80: */
	    i_3 = *n - k + 1;
	    y[j] = beta * ddot_(&i_3, &qr[k + k * qr_dim1], &c_1, &qr[k + j 
		    * qr_dim1], &c_1);
	}
	i_3 = np1;
	for (j = kp1; j <= i_3; ++j) {
	    i_2 = *n;
	    for (i = k; i <= i_2; ++i) {
		qr[i + j * qr_dim1] -= qr[i + k * qr_dim1] * y[j];
/* L90: */
	    }
/* Computing 2nd power */
	    d_1 = qr[k + j * qr_dim1];
	    sum[j] -= d_1 * d_1;
/* L100: */
	}
L500:
	;
    }
    alpha[*n] = qr[*n + *n * qr_dim1];
    return 0;
} /* dcpose_ */

/* end dcpose.c */

/**********************************************************************/
/****************** implementation code from ddot.c *******************/
/**********************************************************************/

doublereal ddot_(integer    *n,
		 doublereal *dx,
		 integer    *incx,
		 doublereal *dy,
		 integer    *incy)
{
    /* System generated locals */
    integer i_1;
    doublereal ret_val;

    /* Local variables */
    static integer i, m;
    static doublereal dtemp;
    static integer ix, iy, mp1;


/*     forms the dot product of two vectors. */
/*     uses unrolled loops for increments equal to one. */
/*     jack dongarra, linpack, 3/11/78. */


    /* Parameter adjustments */
    --dy;
    --dx;

    /* Function Body */
    ret_val = 0.;
    dtemp = 0.;
    if (*n <= 0) {
	return ret_val;
    }
    if (*incx == 1 && *incy == 1) {
	goto L20;
    }

/*        code for unequal increments or equal increments */
/*          not equal to 1 */

    ix = 1;
    iy = 1;
    if (*incx < 0) {
	ix = (-(*n) + 1) * *incx + 1;
    }
    if (*incy < 0) {
	iy = (-(*n) + 1) * *incy + 1;
    }
    i_1 = *n;
    for (i = 1; i <= i_1; ++i) {
	dtemp += dx[ix] * dy[iy];
	ix += *incx;
	iy += *incy;
/* L10: */
    }
    ret_val = dtemp;
    return ret_val;

/*        code for both increments equal to 1 */


/*        clean-up loop */

L20:
    m = *n % 5;
    if (m == 0) {
	goto L40;
    }
    i_1 = m;
    for (i = 1; i <= i_1; ++i) {
	dtemp += dx[i] * dy[i];
/* L30: */
    }
    if (*n < 5) {
	goto L60;
    }
L40:
    mp1 = m + 1;
    i_1 = *n;
    for (i = mp1; i <= i_1; i += 5) {
	dtemp = dtemp + dx[i] * dy[i] + dx[i + 1] * dy[i + 1] + dx[i + 2] * 
		dy[i + 2] + dx[i + 3] * dy[i + 3] + dx[i + 4] * dy[i + 4];
/* L50: */
    }
L60:
    ret_val = dtemp;
    return ret_val;
} /* ddot_ */

/* end ddot.c */

/**********************************************************************/
/******************* implementation code from divp.c *********************/
/**********************************************************************/

/* Table of constant values */

static integer c_2 = 2;

/* Subroutine */ int divp_(doublereal *xxxx, 
			   doublereal *yyyy, 
			   doublereal *zzzz, 
			   integer    *ierr)
{
    static doublereal xnum, denom;
    extern doublereal d1mach_(integer *i);


/* THIS SUBROUTINE PERFORMS DIVISION  OF COMPLEX NUMBERS: */
/* ZZZZ = XXXX/YYYY */

/* ON INPUT: */

/* XXXX  IS AN ARRAY OF LENGTH TWO REPRESENTING THE FIRST COMPLEX */
/*       NUMBER, WHERE XXXX(1) = REAL PART OF XXXX AND XXXX(2) = */
/*       IMAGINARY PART OF XXXX. */

/* YYYY  IS AN ARRAY OF LENGTH TWO REPRESENTING THE SECOND COMPLEX */
/*       NUMBER, WHERE YYYY(1) = REAL PART OF YYYY AND YYYY(2) = */
/*       IMAGINARY PART OF YYYY. */

/* ON OUTPUT: */

/* ZZZZ  IS AN ARRAY OF LENGTH TWO REPRESENTING THE RESULT OF */
/*       THE DIVISION, ZZZZ = XXXX/YYYY, WHERE ZZZZ(1) = */
/*       REAL PART OF ZZZZ AND ZZZZ(2) = IMAGINARY PART OF ZZZZ. */

/* IERR = */
/*  1   IF DIVISION WOULD HAVE CAUSED OVERFLOW.  IN THIS CASE, THE */
/*      APPROPRIATE PARTS OF ZZZZ ARE SET EQUAL TO THE LARGEST */
/*      FLOATING POINT NUMBER, AS GIVEN BY FUNCTION  D1MACH . */

/*  0   IF DIVISION DOES NOT CAUSE OVERFLOW. */

/* DECLARATION OF INPUT */

/* DECLARATION OF OUTPUT */

/* DECLARATION OF VARIABLES */

    /* Parameter adjustments */
    --zzzz;
    --yyyy;
    --xxxx;

    /* Function Body */
    *ierr = 0;
    denom = yyyy[1] * yyyy[1] + yyyy[2] * yyyy[2];
    xnum = xxxx[1] * yyyy[1] + xxxx[2] * yyyy[2];
    if (dblabs(denom) >= (float)1. || (dblabs(denom) < (float)1. && dblabs(xnum) / 
	    d1mach_(&c_2) < dblabs(denom)) ) {
	zzzz[1] = xnum / denom;
    } else {
	zzzz[1] = d1mach_(&c_2);
	*ierr = 1;
    }
    xnum = xxxx[2] * yyyy[1] - xxxx[1] * yyyy[2];
    if (dblabs(denom) >= (float)1. || (dblabs(denom) < (float)1. && dblabs(xnum) / 
	    d1mach_(&c_2) < dblabs(denom))) {
	zzzz[2] = xnum / denom;
    } else {
	zzzz[2] = d1mach_(&c_2);
	*ierr = 1;
    }
    return 0;
} /* divp_ */

/* end divp.c */

/**********************************************************************/
/****************** implementation code from dnrm2.c ******************/
/**********************************************************************/

doublereal dnrm2_(integer    *n, 
		  doublereal *dx, 
		  integer    *incx)
{
    /* Initialized data */

    static doublereal zero = 0.;
    static doublereal one = 1.;
    static doublereal cutlo = 8.232e-11;
    static doublereal cuthi = 1.304e19;

    /* Format strings */
    static char fmt_30[] = "";
    static char fmt_50[] = "";
    static char fmt_70[] = "";
    static char fmt_110[] = "";

    /* System generated locals */
    integer i_1;
    doublereal ret_val, d_1;

    /* Builtin functions */
    /*    double sqrt(double); CANT DECLARE BUILTINS UNDER C++ */

    /* Local variables */
    static doublereal xmax;
    static integer next, i, j, ix;
    static doublereal hitest, sum;

    /* Assigned format variables */
    char *next_fmt;

    /* Parameter adjustments */
    --dx;

    /* Function Body */

/*     euclidean norm of the n-vector stored in dx() with storage */
/*     increment incx . */
/*     if    n .le. 0 return with result = 0. */
/*     if n .ge. 1 then incx must be .ge. 1 */

/*           c.l.lawson, 1978 jan 08 */
/*     modified to correct failure to update ix, 1/25/92. */
/*     modified 3/93 to return if incx .le. 0. */

/*     four phase method     using two built-in constants that are */
/*     hopefully applicable to all machines. */
/*         cutlo = maximum of  dsqrt(u/eps)  over all known machines. */
/*         cuthi = minimum of  dsqrt(v)      over all known machines. */
/*     where */
/*         eps = smallest no. such that eps + 1. .gt. 1. */
/*         u   = smallest positive no.   (underflow limit) */
/*         v   = largest  no.            (overflow  limit) */

/*     brief outline of algorithm.. */

/*     phase 1    scans zero components. */
/*     move to phase 2 when a component is nonzero and .le. cutlo */
/*     move to phase 3 when a component is .gt. cutlo */
/*     move to phase 4 when a component is .ge. cuthi/m */
/*     where m = n for x() real and m = 2*n for complex. */

/*     values for cutlo and cuthi.. */
/*     from the environmental parameters listed in the imsl converter */
/*     document the limiting values are as follows.. */
/*     cutlo, s.p.   u/eps = 2**(-102) for  honeywell.  close seconds are 
*/
/*                   univac and dec at 2**(-103) */
/*                   thus cutlo = 2**(-51) = 4.44089e-16 */
/*     cuthi, s.p.   v = 2**127 for univac, honeywell, and dec. */
/*                   thus cuthi = 2**(63.5) = 1.30438e19 */
/*     cutlo, d.p.   u/eps = 2**(-67) for honeywell and dec. */
/*                   thus cutlo = 2**(-33.5) = 8.23181d-11 */
/*     cuthi, d.p.   same as s.p.  cuthi = 1.30438d19 */
/*     data cutlo, cuthi / 8.232d-11,  1.304d19 / */
/*     data cutlo, cuthi / 4.441e-16,  1.304e19 / */

    if (*n > 0 && *incx > 0) {
	goto L10;
    }
    ret_val = zero;
    goto L300;

L10:
    next = 0;
    next_fmt = fmt_30;
    sum = zero;
    i = 1;
    ix = 1;
/*                                                 begin main loop */
L20:
    switch ((int)next) {
	case 0: goto L30;
	case 1: goto L50;
	case 2: goto L70;
	case 3: goto L110;
    }
L30:
    if ((d_1 = dx[i], dblabs(d_1)) > cutlo) {
	goto L85;
    }
    next = 1;
    next_fmt = fmt_50;
    xmax = zero;

/*                        phase 1.  sum is zero */

L50:
    if (dx[i] == zero) {
	goto L200;
    }
    if ((d_1 = dx[i], dblabs(d_1)) > cutlo) {
	goto L85;
    }

/*                                prepare for phase 2. */
    next = 2;
    next_fmt = fmt_70;
    goto L105;

/*                                prepare for phase 4. */

L100:
    ix = j;
    next = 3;
    next_fmt = fmt_110;
    sum = sum / dx[i] / dx[i];
L105:
    xmax = (d_1 = dx[i], dblabs(d_1));
    goto L115;

/*                   phase 2.  sum is small. */
/*                             scale to avoid destructive underflow. */

L70:
    if ((d_1 = dx[i], dblabs(d_1)) > cutlo) {
	goto L75;
    }

/*                     common code for phases 2 and 4. */
/*                     in phase 4 sum is large.  scale to avoid overflow. 
*/

L110:
    if ((d_1 = dx[i], dblabs(d_1)) <= xmax) {
	goto L115;
    }
/* Computing 2nd power */
    d_1 = xmax / dx[i];
    sum = one + sum * (d_1 * d_1);
    xmax = (d_1 = dx[i], dblabs(d_1));
    goto L200;

L115:
/* Computing 2nd power */
    d_1 = dx[i] / xmax;
    sum += d_1 * d_1;
    goto L200;


/*                  prepare for phase 3. */

L75:
    sum = sum * xmax * xmax;


/*     for real or d.p. set hitest = cuthi/n */
/*     for complex      set hitest = cuthi/(2*n) */

L85:
    hitest = cuthi / (real) (*n);

/*                   phase 3.  sum is mid-range.  no scaling. */

    i_1 = *n;
    for (j = ix; j <= i_1; ++j) {
	if ((d_1 = dx[i], dblabs(d_1)) >= hitest) {
	    goto L100;
	}
/* Computing 2nd power */
	d_1 = dx[i];
	sum += d_1 * d_1;
	i += *incx;
/* L95: */
    }
    ret_val = sqrt(sum);
    goto L300;

L200:
    ++ix;
    i += *incx;
    if (ix <= *n) {
	goto L20;
    }

/*              end of main loop. */

/*              compute square root and adjust for scaling. */

    ret_val = xmax * sqrt(sum);
L300:
    return ret_val;
} /* dnrm2_ */

/* end dnrm2.c */

/**********************************************************************/
/***************** implementation code from dscal.c *******************/
/**********************************************************************/

/* Subroutine */ int dscal_(integer    *n, 
			    doublereal *da, 
			    doublereal *dx, 
			    integer    *incx)
{
    /* System generated locals */
    integer i_1, i_2;

    /* Local variables */
    static integer i, m, nincx, mp1;


/*     scales a vector by a constant. */
/*     uses unrolled loops for increment equal to one. */
/*     jack dongarra, linpack, 3/11/78. */
/*     modified 3/93 to return if incx .le. 0. */


    /* Parameter adjustments */
    --dx;

    /* Function Body */
    if (*n <= 0 || *incx <= 0) {
	return 0;
    }
    if (*incx == 1) {
	goto L20;
    }

/*        code for increment not equal to 1 */

    nincx = *n * *incx;
    i_1 = nincx;
    i_2 = *incx;
    for (i = 1; i_2 < 0 ? i >= i_1 : i <= i_1; i += i_2) {
	dx[i] = *da * dx[i];
/* L10: */
    }
    return 0;

/*        code for increment equal to 1 */


/*        clean-up loop */

L20:
    m = *n % 5;
    if (m == 0) {
	goto L40;
    }
    i_2 = m;
    for (i = 1; i <= i_2; ++i) {
	dx[i] = *da * dx[i];
/* L30: */
    }
    if (*n < 5) {
	return 0;
    }
L40:
    mp1 = m + 1;
    i_2 = *n;
    for (i = mp1; i <= i_2; i += 5) {
	dx[i] = *da * dx[i];
	dx[i + 1] = *da * dx[i + 1];
	dx[i + 2] = *da * dx[i + 2];
	dx[i + 3] = *da * dx[i + 3];
	dx[i + 4] = *da * dx[i + 4];
/* L50: */
    }
    return 0;
} /* dscal_ */

/* end dscal.c */

/**********************************************************************/
/******************* implementation code from f.c *********************/
/**********************************************************************/

/* Subroutine */ int f_(doublereal *x, 
			doublereal *v)
{

/* EVALUATE  F(X)  AND RETURN IN THE VECTOR  V . */

    /* Parameter adjustments */
    --v;
    --x;

    /* Function Body */
    return 0;
} /* f_ */

/* end f.c */

/**********************************************************************/
/***************** implementation code from fjac.c ********************/
/**********************************************************************/

/* Subroutine */ int fjac_(doublereal *x, 
			   doublereal *v, 
			   integer    *k)
{

/* RETURN IN  V  THE KTH COLUMN OF THE JACOBIAN MATRIX OF */
/* F(X) EVALUATED AT  X . */

    /* Parameter adjustments */
    --v;
    --x;

    /* Function Body */
    return 0;
} /* fjac_ */

/* end fjac.c */

/**********************************************************************/
/***************** implementation code from idamax.c ******************/
/**********************************************************************/

integer idamax_(integer    *n, 
		doublereal *dx, 
		integer    *incx)
{
    /* System generated locals */
    integer ret_val, i_1;
    doublereal d_1;

    /* Local variables */
    static doublereal dmax_;
    static integer i, ix;


/*     finds the index of element having max. absolute value. */
/*     jack dongarra, linpack, 3/11/78. */
/*     modified 3/93 to return if incx .le. 0. */


    /* Parameter adjustments */
    --dx;

    /* Function Body */
    ret_val = 0;
    if (*n < 1 || *incx <= 0) {
	return ret_val;
    }
    ret_val = 1;
    if (*n == 1) {
	return ret_val;
    }
    if (*incx == 1) {
	goto L20;
    }

/*        code for increment not equal to 1 */

    ix = 1;
    dmax_ = dblabs(dx[1]);
    ix += *incx;
    i_1 = *n;
    for (i = 2; i <= i_1; ++i) {
	if ((d_1 = dx[ix], dblabs(d_1)) <= dmax_) {
	    goto L5;
	}
	ret_val = i;
	dmax_ = (d_1 = dx[ix], dblabs(d_1));
L5:
	ix += *incx;
/* L10: */
    }
    return ret_val;

/*        code for increment equal to 1 */

L20:
    dmax_ = dblabs(dx[1]);
    i_1 = *n;
    for (i = 2; i <= i_1; ++i) {
	if ((d_1 = dx[i], dblabs(d_1)) <= dmax_) {
	    goto L30;
	}
	ret_val = i;
	dmax_ = (d_1 = dx[i], dblabs(d_1));
L30:
	;
    }
    return ret_val;
} /* idamax_ */

/* end idamax.c */

/**********************************************************************/
/****************** implementation code from root.c *******************/
/**********************************************************************/

double d_sign(double *arg1, double *arg2){
   if (*arg2>=0.0) return dblabs(*arg1);
   else return -1.0*dblabs(*arg1);
}

/* Table of constant values */

/* static integer c_4 = 4; NOW REDUNDANT */
static doublereal c_b17 = 1.;

/* Subroutine */ int root_(doublereal *t, 
			   doublereal *ft, 
			   doublereal *b, 
			   doublereal *c, 
			   doublereal *relerr, 
			   doublereal *abserr, 
			   integer    *iflag)
{
    /* System generated locals */
    doublereal d_1, d_2;

    /* Builtin functions */
    double d_sign(double *arg1, double *arg2);

    /* Local variables */
    static doublereal acmb, acbs, a, p, q, u;
    extern doublereal d1mach_(integer *i);
    static integer kount;
    static doublereal ae, fa, fb, fc;
    static integer ic;
    static doublereal re, fx, cmb, tol;


/*  ROOT COMPUTES A ROOT OF THE NONLINEAR EQUATION F(X)=0 */
/*  WHERE F(X) IS A CONTINOUS REAL FUNCTION OF A SINGLE REAL */
/*  VARIABLE X.  THE METHOD USED IS A COMBINATION OF BISECTION */
/*  AND THE SECANT RULE. */

/*  NORMAL INPUT CONSISTS OF A CONTINUOS FUNCTION F AND AN */
/*  INTERVAL (B,C) SUCH THAT F(B)*F(C).LE.0.0.  EACH ITERATION */
/*  FINDS NEW VALUES OF B AND C SUCH THAT THE INTERVAL(B,C) IS */
/*  SHRUNK AND F(B)*F(C).LE.0.0.  THE STOPPING CRITERION IS */

/*          DABS(B-C).LE.2.0*(RELERR*DABS(B)+ABSERR) */

/*  WHERE RELERR=RELATIVE ERROR AND ABSERR=ABSOLUTE ERROR ARE */
/*  INPUT QUANTITIES.  SET THE FLAG, IFLAG, POSITIVE TO INITIALIZE */
/*  THE COMPUTATION.  AS B,C AND IFLAG ARE USED FOR BOTH INPUT AND */
/*  OUTPUT, THEY MUST BE VARIABLES IN THE CALLING PROGRAM. */

/*  IF 0 IS A POSSIBLE ROOT, ONE SHOULD NOT CHOOSE ABSERR=0.0. */

/*  THE OUTPUT VALUE OF B IS THE BETTER APPROXIMATION TO A ROOT */
/*  AS B AND C ARE ALWAYS REDEFINED SO THAT DABS(F(B)).LE.DABS(F(C)). */

/*  TO SOLVE THE EQUATION, ROOT MUST EVALUATE F(X) REPEATEDLY. THIS */
/*  IS DONE IN THE CALLING PROGRAM.  WHEN AN EVALUATION OF F IS */
/*  NEEDED AT T, ROOT RETURNS WITH IFLAG NEGATIVE.  EVALUATE FT=F(T) */
/*  AND CALL ROOT AGAIN.  DO NOT ALTER IFLAG. */

/*  WHEN THE COMPUTATION IS COMPLETE, ROOT RETURNS TO THE CALLING */
/*  PROGRAM WITH IFLAG POSITIVE= */

/*     IFLAG=1  IF F(B)*F(C).LT.0 AND THE STOPPING CRITERION IS MET. */

/*          =2  IF A VALUE B IS FOUND SUCH THAT THE COMPUTED VALUE */
/*              F(B) IS EXACTLY ZERO.  THE INTERVAL (B,C) MAY NOT */
/*              SATISFY THE STOPPING CRITERION. */

/*          =3  IF DABS(F(B)) EXCEEDS THE INPUT VALUES DABS(F(B)), */
/*              DABS(F(C)).  IN THIS CASE IT IS LIKELY THAT B IS CLOSE */
/*              TO A POLE OF F. */

/*          =4  IF NO ODD ORDER ROOT WAS FOUND IN THE INTERVAL.  A */
/*              LOCAL MINIMUM MAY HAVE BEEN OBTAINED. */

/*          =5  IF TOO MANY FUNCTION EVALUATIONS WERE MADE. */
/*              (AS PROGRAMMED, 500 ARE ALLOWED.) */

/*  THIS CODE IS A MODIFICATION OF THE CODE ZEROIN WHICH IS COMPLETELY */
/*  EXPLAINED AND DOCUMENTED IN THE TEXT  NUMERICAL COMPUTING:  AN */
/*  INTRODUCTION,  BY L. F. SHAMPINE AND R. C. ALLEN. */

/*  CALLS  D1MACH . */


    if (*iflag >= 0) {
	goto L100;
    }
    *iflag = dblabs(*iflag);
    switch ((int)*iflag) {
	case 1:  goto L200;
	case 2:  goto L300;
	case 3:  goto L400;
    }
L100:
    u = d1mach_(&c_4);
    re = max(*relerr,u);
    ae = max(*abserr,0.);
    ic = 0;
    acbs = (d_1 = *b - *c, dblabs(d_1));
    a = *c;
    *t = a;
    *iflag = -1;
    return 0;
L200:
    fa = *ft;
    *t = *b;
    *iflag = -2;
    return 0;
L300:
    fb = *ft;
    fc = fa;
    kount = 2;
/* Computing MAX */
    d_1 = dblabs(fb), d_2 = dblabs(fc);
    fx = max(d_1,d_2);
L1:
    if (dblabs(fc) >= dblabs(fb)) {
	goto L2;
    }

/*  INTERCHANGE B AND C SO THAT ABS(F(B)).LE.ABS(F(C)). */

    a = *b;
    fa = fb;
    *b = *c;
    fb = fc;
    *c = a;
    fc = fa;
L2:
    cmb = (*c - *b) * (float).5;
    acmb = dblabs(cmb);
    tol = re * dblabs(*b) + ae;

/*  TEST STOPPING CRITERION AND FUNCTION COUNT. */

    if (acmb <= tol) {
	goto L8;
    }
    if (kount >= 500) {
	goto L12;
    }

/*  CALCULATE NEW ITERATE EXPLICITLY AS B+P/Q */
/*  WHERE WE ARRANGE P.GE.0.  THE IMPLICIT */
/*  FORM IS USED TO PREVENT OVERFLOW. */

    p = (*b - a) * fb;
    q = fa - fb;
    if (p >= (float)0.) {
	goto L3;
    }
    p = -p;
    q = -q;

/*  UPDATE A, CHECK IF REDUCTION IN THE SIZE OF BRACKETING */
/*  INTERVAL IS SATISFACTORY.  IF NOT BISECT UNTIL IT IS. */

L3:
    a = *b;
    fa = fb;
    ++ic;
    if (ic < 4) {
	goto L4;
    }
    if (acmb * (float)8. >= acbs) {
	goto L6;
    }
    ic = 0;
    acbs = acmb;

/*  TEST FOR TOO SMALL A CHANGE. */

L4:
    if (p > dblabs(q) * tol) {
	goto L5;
    }

/*  INCREMENT BY TOLERANCE */

/* sign gives sign(arg2)*|arg1| */
    *b += d_sign(&tol, &cmb);
    goto L7;

/*  ROOT OUGHT TO BE BETWEEN B AND (C+B)/2 */

L5:
    if (p >= cmb * q) {
	goto L6;
    }

/*  USE SECANT RULE. */

    *b += p / q;
    goto L7;

/*  USE BISECTION. */

L6:
    *b = (*c + *b) * (float).5;

/*  HAVE COMPLETED COMPUTATION FOR NEW ITERATE B. */

L7:
    *t = *b;
    *iflag = -3;
    return 0;
L400:
    fb = *ft;
    if (fb == (float)0.) {
	goto L9;
    }
    ++kount;
    if (d_sign(&c_b17, &fb) != d_sign(&c_b17, &fc)) {
	goto L1;
    }
    *c = a;
    fc = fa;
    goto L1;

/* FINISHED.  SET IFLAG. */

L8:
    if (d_sign(&c_b17, &fb) == d_sign(&c_b17, &fc)) {
	goto L11;
    }
    if (dblabs(fb) > fx) {
	goto L10;
    }
    *iflag = 1;
    return 0;
L9:
    *iflag = 2;
    return 0;
L10:
    *iflag = 3;
    return 0;
L11:
    *iflag = 4;
    return 0;
L12:
    *iflag = 5;
    return 0;
} /* root_ */

/* end root.c */

/**********************************************************************/
/***************** implementation code from rootnf.c ******************/
/**********************************************************************/

/* Table of constant values */

/*
static int c_4 = 4;
static int c_1 = 1;
*/

/* Subroutine */ int rootnf_(int    *n, 
			     int    *nfe, 
			     int    *iflag, 
			     double *relerr,
			     double *abserr,
			     double *y,
			     double *yp, 
			     double *yold, 
			     double *ypold, 
			     double *a,
			     double *qr, 
			     double *alpha, 
			     double *tz, 
			     int    *pivot, 
			     double *w, 
			     double *wp, 
			     double *par, 
			     int    *ipar)
{
    /* System generated locals */
    int qr_dim1, qr_offset, i_1;
    double d_1;

    /* Local variables */
    static double dels, aerr, rerr;
    static int judy;
    extern /* Subroutine */ int root_(doublereal *t, 
				      doublereal *ft, 
				      doublereal *b, 
				      doublereal *c, 
				      doublereal *relerr, 
				      doublereal *abserr, 
				      integer    *iflag);
    static double sout;
    extern double dnrm2_(integer    *n, 
			 doublereal *dx, 
			 integer    *incx);
    static double u;
    static int lcode;
    extern double d1mach_(integer *);
    static double qsout, sa, sb;
    static int jw;
    /* extern */ /* Subroutine */ /* int tangnf_(); IN pelutils.h */
    static int np1;


/* ROOTNF  FINDS THE POINT  YBAR = (1, XBAR)  ON THE ZERO CURVE OF THE */
/* HOMOTOPY MAP.  IT STARTS WITH TWO POINTS YOLD=(LAMBDAOLD,XOLD) AND */
/* Y=(LAMBDA,X) SUCH THAT  LAMBDAOLD < 1 <= LAMBDA , AND ALTERNATES */
/* BETWEEN HERMITE CUBIC INTERPOLATION AND NEWTON ITERATION UNTIL */
/* CONVERGENCE. */

/* ON INPUT: */

/* N = DIMENSION OF X AND THE HOMOTOPY MAP. */

/* NFE = NUMBER OF JACOBIAN MATRIX EVALUATIONS. */

/* IFLAG = -2, -1, OR 0, INDICATING THE PROBLEM TYPE. */

/* RELERR, ABSERR = RELATIVE AND ABSOLUTE ERROR VALUES.  THE ITERATION IS 
*/
/*    CONSIDERED TO HAVE CONVERGED WHEN A POINT Y=(LAMBDA,X) IS FOUND */
/*    SUCH THAT */

/*    |Y(1) - 1| <= RELERR + ABSERR              AND */

/*    ||Z|| <= RELERR*||X|| + ABSERR  ,          WHERE */

/*    (?,Z) IS THE NEWTON STEP TO Y=(LAMBDA,X). */

/* Y(1:N+1) = POINT (LAMBDA(S), X(S)) ON ZERO CURVE OF HOMOTOPY MAP. */

/* YP(1:N+1) = UNIT TANGENT VECTOR TO THE ZERO CURVE OF THE HOMOTOPY MAP 
*/
/*    AT  Y . */

/* YOLD(1:N+1) = A POINT DIFFERENT FROM  Y  ON THE ZERO CURVE. */

/* YPOLD(1:N+1) = UNIT TANGENT VECTOR TO THE ZERO CURVE OF THE HOMOTOPY */
/*    MAP AT  YOLD . */

/* A(1:*) = PARAMETER VECTOR IN THE HOMOTOPY MAP. */

/* QR(1:N,1:N+2), ALPHA(1:N), TZ(1:N+1), PIVOT(1:N+1), W(1:N+1), */
/*    WP(1:N+1)  ARE WORK ARRAYS USED FOR THE QR FACTORIZATION (IN THE */
/*    NEWTON STEP CALCULATION) AND THE INTERPOLATION. */

/* PAR(1:*) AND IPAR(1:*) ARE ARRAYS FOR (OPTIONAL) USER PARAMETERS, */
/*    WHICH ARE SIMPLY PASSED THROUGH TO THE USER WRITTEN SUBROUTINES */
/*    RHO, RHOJAC. */

/* ON OUTPUT: */

/* N , RELERR , ABSERR , A  ARE UNCHANGED. */

/* NFE  HAS BEEN UPDATED. */

/* IFLAG */
/*    = -2, -1, OR 0 (UNCHANGED) ON A NORMAL RETURN. */

/*    = 4 IF A JACOBIAN MATRIX WITH RANK < N HAS OCCURRED.  THE */
/*        ITERATION WAS NOT COMPLETED. */

/*    = 6 IF THE ITERATION FAILED TO CONVERGE.  Y  AND  YOLD  CONTAIN */
/*        THE LAST TWO POINTS FOUND ON THE ZERO CURVE. */

/* Y  IS THE POINT ON THE ZERO CURVE OF THE HOMOTOPY MAP AT  LAMBDA = 1 . 
*/


/* CALLS  D1MACH , DNRM2 , ROOT , TANGNF . */


/* ***** ARRAY DECLARATIONS. ***** */


/* ***** END OF DIMENSIONAL INFORMATION. ***** */

/* THE LIMIT ON THE NUMBER OF ITERATIONS ALLOWED MAY BE CHANGED BY */
/* CHANGING THE FOLLOWING PARAMETER STATEMENT: */

/* DEFINITION OF HERMITE CUBIC INTERPOLANT VIA DIVIDED DIFFERENCES. */



    /* Parameter adjustments */
    --wp;
    --w;
    --pivot;
    --tz;
    --alpha;
    qr_dim1 = *n;
    qr_offset = qr_dim1 + 1;
    qr -= qr_offset;
    --a;
    --ypold;
    --yold;
    --yp;
    --y;
    --par;
    --ipar;

    /* Function Body */
    u = d1mach_(&c_4);
    rerr = max(*relerr,u);
    aerr = max(*abserr,0.);
    np1 = *n + 1;

/* *****  MAIN LOOP.  ***** */

/* L100: */
    for (judy = 1; judy <= 20; ++judy) {
	i_1 = np1;
	for (jw = 1; jw <= i_1; ++jw) {
	    tz[jw] = y[jw] - yold[jw];
/* L110: */
	}
	dels = dnrm2_((integer *)&np1, &tz[1], &c_1);

/* USING TWO POINTS AND TANGENTS ON THE HOMOTOPY ZERO CURVE, CONSTRUCT
 */
/* THE HERMITE CUBIC INTERPOLANT Q(S).  THEN USE  ROOT  TO FIND THE S 
*/
/* CORRESPONDING TO  LAMBDA = 1 .  THE TWO POINTS ON THE ZERO CURVE AR
E */
/* ALWAYS CHOSEN TO BRACKET LAMBDA=1, WITH THE BRACKETING INTERVAL */
/* ALWAYS BEING [0, DELS]. */

	sa = (float)0.;
	sb = dels;
	lcode = 1;
L130:
	root_(&sout, &qsout, &sa, &sb, &rerr, &aerr, (integer *)&lcode);
	if (lcode > 0) {
	    goto L140;
	}
	qsout = ((((yp[1] - (y[1] - yold[1]) / dels) / dels - ((y[1] - yold[1]
		) / dels - ypold[1]) / dels) / dels * (sout - dels) + ((y[1] 
		- yold[1]) / dels - ypold[1]) / dels) * sout + ypold[1]) * 
		sout + yold[1] - (float)1.;
	goto L130;
/* IF LAMBDA = 1 WERE BRACKETED,  ROOT  CANNOT FAIL. */
L140:
	if (lcode > 2) {
	    *iflag = 6;
	    return 0;
	}

/* CALCULATE Q(SA) AS THE INITIAL POINT FOR A NEWTON ITERATION. */
	i_1 = np1;
	for (jw = 1; jw <= i_1; ++jw) {
	    w[jw] = ((((yp[jw] - (y[jw] - yold[jw]) / dels) / dels - ((y[jw] 
		    - yold[jw]) / dels - ypold[jw]) / dels) / dels * (sa - 
		    dels) + ((y[jw] - yold[jw]) / dels - ypold[jw]) / dels) * 
		    sa + ypold[jw]) * sa + yold[jw];
/* L150: */
	}
/* CALCULATE NEWTON STEP AT Q(SA). */
	tangnf_(&sa, &w[1], &wp[1], &ypold[1], &a[1], &qr[qr_offset], &alpha[
		1], &tz[1], &pivot[1], nfe, n, iflag, &par[1], &ipar[1]);
	if (*iflag > 0) {
	    return 0;
	}
/* NEXT POINT = CURRENT POINT + NEWTON STEP. */
	i_1 = np1;
	for (jw = 1; jw <= i_1; ++jw) {
	    w[jw] += tz[jw];
/* L160: */
	}
/* GET THE TANGENT  WP  AT  W  AND THE NEXT NEWTON STEP IN  TZ . */
	tangnf_(&sa, &w[1], &wp[1], &ypold[1], &a[1], &qr[qr_offset], &alpha[
		1], &tz[1], &pivot[1], nfe, n, iflag, &par[1], &ipar[1]);
	if (*iflag > 0) {
	    return 0;
	}
/* TAKE NEWTON STEP AND CHECK CONVERGENCE. */
	i_1 = np1;
	for (jw = 1; jw <= i_1; ++jw) {
	    w[jw] += tz[jw];
/* L170: */
	}
	if ((d_1 = w[1] - (float)1., dblabs(d_1)) <= rerr + aerr && dnrm2_((integer *)n, &
		tz[2], &c_1) <= rerr * dnrm2_((integer*)n, &w[2], &c_1) + aerr) {
	    i_1 = np1;
	    for (jw = 1; jw <= i_1; ++jw) {
		y[jw] = w[jw];
/* L180: */
	    }
	    return 0;
	}
/* IF THE ITERATION HAS NOT CONVERGED, DISCARD ONE OF THE OLD POINTS 
*/
/* SUCH THAT  LAMBDA = 1  IS STILL BRACKETED. */
	if ((yold[1] - (float)1.) * (w[1] - (float)1.) > (float)0.) {
	    i_1 = np1;
	    for (jw = 1; jw <= i_1; ++jw) {
		yold[jw] = w[jw];
		ypold[jw] = wp[jw];
/* L200: */
	    }
	} else {
	    i_1 = np1;
	    for (jw = 1; jw <= i_1; ++jw) {
		y[jw] = w[jw];
		yp[jw] = wp[jw];
/* L210: */
	    }
	}
/* L300: */
    }

/* ***** END OF MAIN LOOP. ***** */

/* THE ALTERNATING OSCULATORY CUBIC INTERPOLATION AND NEWTON ITERATION */
/* HAS NOT CONVERGED IN  LIMIT  STEPS.  ERROR RETURN. */
    *iflag = 6;
    return 0;
} /* rootnf_ */

/* end rootnf.c */

/**********************************************************************/
/******************* implementation code from stepnf.c *********************/
/**********************************************************************/

/* Table of constant values */

/*
static integer c_4 = 4;
static integer c_1 = 1;
*/

/* Subroutine */ int stepnf_(integer    *n, 
			     integer    *nfe, 
			     integer    *iflag, 
			     logical    *start, 
			     logical    *crash, 
			     doublereal *hold, 
			     doublereal *h, 
			     doublereal *relerr, 
			     doublereal *abserr, 
			     doublereal *s, 
			     doublereal *y, 
			     doublereal *yp, 
			     doublereal *yold, 
			     doublereal *ypold, 
			     doublereal *a, 
			     doublereal *qr, 
			     doublereal *alpha, 
			     doublereal *tz, 
			     integer    *pivot, 
			     doublereal *w, 
			     doublereal *wp, 
			     doublereal *z0, 
			     doublereal *z1,
			     doublereal *sspar, 
			     doublereal *par, 
			     integer    *ipar)
{
    /* System generated locals */
    integer qr_dim1, qr_offset, i_1;
    doublereal d_1, d_2, d_3, d_4;

    /* Builtin functions */
    /*    double sqrt(double), pow(double,double); CANT DECLARE BUILTINS UNDER C++ */

    /* Local variables */
    static logical fail;
    static doublereal temp;
    static integer judy;
    static doublereal twou;
    extern doublereal dnrm2_(integer    *n, 
			     doublereal *dx, 
			     integer    *incx);
    static doublereal dcalc;
    static integer j;
    static doublereal lcalc, hfail, rcalc;
    static integer itnum;
    extern doublereal d1mach_(integer *i);
    static doublereal fouru, ht;
    /*extern */ /* Subroutine */ /* int tangnf_(); IN pelutils.h */
    static doublereal rholen;
    static integer np1;


/*  STEPNF  TAKES ONE STEP ALONG THE ZERO CURVE OF THE HOMOTOPY MAP */
/* USING A PREDICTOR-CORRECTOR ALGORITHM.  THE PREDICTOR USES A HERMITE */
/* CUBIC INTERPOLANT, AND THE CORRECTOR RETURNS TO THE ZERO CURVE ALONG */
/* THE FLOW NORMAL TO THE DAVIDENKO FLOW.  STEPNF  ALSO ESTIMATES A */
/* STEP SIZE H FOR THE NEXT STEP ALONG THE ZERO CURVE.  NORMALLY */
/*  STEPNF  IS USED INDIRECTLY THROUGH  FIXPNF , AND SHOULD BE CALLED */
/* DIRECTLY ONLY IF IT IS NECESSARY TO MODIFY THE STEPPING ALGORITHM'S */
/* PARAMETERS. */

/* ON INPUT: */

/* N = DIMENSION OF X AND THE HOMOTOPY MAP. */

/* NFE = NUMBER OF JACOBIAN MATRIX EVALUATIONS. */

/* IFLAG = -2, -1, OR 0, INDICATING THE PROBLEM TYPE. */

/* START = .TRUE. ON FIRST CALL TO  STEPNF , .FALSE. OTHERWISE. */

/* HOLD = ||Y - YOLD||; SHOULD NOT BE MODIFIED BY THE USER. */

/* H = UPPER LIMIT ON LENGTH OF STEP THAT WILL BE ATTEMPTED.  H  MUST BE 
*/
/*    SET TO A POSITIVE NUMBER ON THE FIRST CALL TO  STEPNF . */
/*    THEREAFTER  STEPNF  CALCULATES AN OPTIMAL VALUE FOR  H , AND  H */
/*    SHOULD NOT BE MODIFIED BY THE USER. */

/* RELERR, ABSERR = RELATIVE AND ABSOLUTE ERROR VALUES.  THE ITERATION IS 
*/
/*    CONSIDERED TO HAVE CONVERGED WHEN A POINT W=(LAMBDA,X) IS FOUND */
/*    SUCH THAT */

/*    ||Z|| <= RELERR*||W|| + ABSERR  ,          WHERE */

/*    Z IS THE NEWTON STEP TO W=(LAMBDA,X). */

/* S = (APPROXIMATE) ARC LENGTH ALONG THE HOMOTOPY ZERO CURVE UP TO */
/*    Y(S) = (LAMBDA(S), X(S)). */

/* Y(1:N+1) = PREVIOUS POINT (LAMBDA(S), X(S)) FOUND ON THE ZERO CURVE OF 
*/
/*    THE HOMOTOPY MAP. */

/* YP(1:N+1) = UNIT TANGENT VECTOR TO THE ZERO CURVE OF THE HOMOTOPY MAP 
*/
/*    AT  Y . */

/* YOLD(1:N+1) = A POINT BEFORE  Y  ON THE ZERO CURVE OF THE HOMOTOPY MAP.
 */

/* YPOLD(1:N+1) = UNIT TANGENT VECTOR TO THE ZERO CURVE OF THE HOMOTOPY */
/*    MAP AT  YOLD . */

/* A(1:*) = PARAMETER VECTOR IN THE HOMOTOPY MAP. */

/* QR(1:N,1:N+2), ALPHA(1:N), TZ(1:N+1), PIVOT(1:N+1), W(1:N+1), */
/*    WP(1:N+1)  ARE WORK ARRAYS USED FOR THE QR FACTORIZATION (IN THE */
/*    NEWTON STEP CALCULATION) AND THE INTERPOLATION. */

/* Z0(1:N+1), Z1(1:N+1)  ARE WORK ARRAYS USED FOR THE ESTIMATION OF THE */
/*    NEXT STEP SIZE  H . */

/* SSPAR(1:8) = (LIDEAL, RIDEAL, DIDEAL, HMIN, HMAX, BMIN, BMAX, P)  IS */
/*    A VECTOR OF PARAMETERS USED FOR THE OPTIMAL STEP SIZE ESTIMATION. */

/* PAR(1:*) AND IPAR(1:*) ARE ARRAYS FOR (OPTIONAL) USER PARAMETERS, */
/*    WHICH ARE SIMPLY PASSED THROUGH TO THE USER WRITTEN SUBROUTINES */
/*    RHO, RHOJAC. */

/* ON OUTPUT: */

/* N , A , SSPAR  ARE UNCHANGED. */

/* NFE  HAS BEEN UPDATED. */

/* IFLAG */
/*    = -2, -1, OR 0 (UNCHANGED) ON A NORMAL RETURN. */

/*    = 4 IF A JACOBIAN MATRIX WITH RANK < N HAS OCCURRED.  THE */
/*        ITERATION WAS NOT COMPLETED. */

/*    = 6 IF THE ITERATION FAILED TO CONVERGE.  W  CONTAINS THE LAST */
/*        NEWTON ITERATE. */

/* START = .FALSE. ON A NORMAL RETURN. */

/* CRASH */
/*    = .FALSE. ON A NORMAL RETURN. */

/*    = .TRUE. IF THE STEP SIZE  H  WAS TOO SMALL.  H  HAS BEEN */
/*      INCREASED TO AN ACCEPTABLE VALUE, WITH WHICH  STEPNF  MAY BE */
/*      CALLED AGAIN. */

/*    = .TRUE. IF  RELERR  AND/OR  ABSERR  WERE TOO SMALL.  THEY HAVE */
/*      BEEN INCREASED TO ACCEPTABLE VALUES, WITH WHICH  STEPNF  MAY */
/*      BE CALLED AGAIN. */

/* HOLD = ||Y - YOLD||. */

/* H = OPTIMAL VALUE FOR NEXT STEP TO BE ATTEMPTED.  NORMALLY  H  SHOULD 
*/
/*    NOT BE MODIFIED BY THE USER. */

/* RELERR, ABSERR  ARE UNCHANGED ON A NORMAL RETURN. */

/* S = (APPROXIMATE) ARC LENGTH ALONG THE ZERO CURVE OF THE HOMOTOPY MAP 
*/
/*    UP TO THE LATEST POINT FOUND, WHICH IS RETURNED IN  Y . */

/* Y, YP, YOLD, YPOLD  CONTAIN THE TWO MOST RECENT POINTS AND TANGENT */
/*    VECTORS FOUND ON THE ZERO CURVE OF THE HOMOTOPY MAP. */


/* CALLS  D1MACH , DNRM2 , TANGNF . */


/* ***** ARRAY DECLARATIONS. ***** */


/* ***** END OF DIMENSIONAL INFORMATION. ***** */

/* THE LIMIT ON THE NUMBER OF NEWTON ITERATIONS ALLOWED BEFORE REDUCING */
/* THE STEP SIZE  H  MAY BE CHANGED BY CHANGING THE FOLLOWING PARAMETER */
/* STATEMENT: */

/* DEFINITION OF HERMITE CUBIC INTERPOLANT VIA DIVIDED DIFFERENCES. */



    /* Parameter adjustments */
    --z1;
    --z0;
    --wp;
    --w;
    --pivot;
    --tz;
    --alpha;
    qr_dim1 = *n;
    qr_offset = qr_dim1 + 1;
    qr -= qr_offset;
    --a;
    --ypold;
    --yold;
    --yp;
    --y;
    --sspar;
    --par;
    --ipar;

    /* Function Body */
    twou = d1mach_(&c_4) * (float)2.;
    fouru = twou + twou;
    np1 = *n + 1;
    *crash = TRUE_;
/* THE ARCLENGTH  S  MUST BE NONNEGATIVE. */
    if (*s < (float)0.) {
	return 0;
    }
/* IF STEP SIZE IS TOO SMALL, DETERMINE AN ACCEPTABLE ONE. */
    if (*h < fouru * (*s + (float)1.)) {
	*h = fouru * (*s + (float)1.);
	return 0;
    }
/* IF ERROR TOLERANCES ARE TOO SMALL, INCREASE THEM TO ACCEPTABLE VALUES. 
*/
    temp = dnrm2_(&np1, &y[1], &c_1);
    if ((*relerr * temp + *abserr) * (float).5 >= twou * temp) {
	goto L40;
    }
    if (*relerr != (float)0.) {
	*relerr = fouru * (fouru + (float)1.);
	*abserr = max(*abserr,0.);
    } else {
	*abserr = fouru * temp;
    }
    return 0;
L40:
    *crash = FALSE_;
    if (! (*start)) {
	goto L300;
    }

/* *****  STARTUP SECTION(FIRST STEP ALONG ZERO CURVE.  ***** */

    fail = FALSE_;
    *start = FALSE_;
/* DETERMINE SUITABLE INITIAL STEP SIZE. */
/* Computing MIN */
    d_1 = min(*h,.1), d_2 = sqrt(sqrt(*relerr * temp + *abserr));
    *h = min(d_1,d_2);
/* USE LINEAR PREDICTOR ALONG TANGENT DIRECTION TO START NEWTON ITERATION.
 */
    ypold[1] = (float)1.;
    i_1 = np1;
    for (j = 2; j <= i_1; ++j) {
	ypold[j] = (float)0.;
/* L50: */
    }
    tangnf_(s, &y[1], &yp[1], &ypold[1], &a[1], &qr[qr_offset], &alpha[1], 
	    &tz[1], (int *)&pivot[1], (int *)nfe, 
	    (int *)n, (int *)iflag, &par[1], (int *)&ipar[1]);
    if (*iflag > 0) {
	return 0;
    }
L70:
    i_1 = np1;
    for (j = 1; j <= i_1; ++j) {
	temp = y[j] + *h * yp[j];
	w[j] = temp;
	z0[j] = temp;
/* L80: */
    }
    for (judy = 1; judy <= 4; ++judy) {
	rholen = (float)-1.;
/* CALCULATE THE NEWTON STEP  TZ  AT THE CURRENT POINT  W . */
	tangnf_(&rholen, &w[1], &wp[1], &ypold[1], &a[1], &qr[qr_offset], 
		&alpha[1], &tz[1], (int *)&pivot[1], (int *)nfe, 
		(int *)n, (int *)iflag, &par[1], (int *)&ipar[1])
		;
	if (*iflag > 0) {
	    return 0;
	}

/* TAKE NEWTON STEP AND CHECK CONVERGENCE. */
	i_1 = np1;
	for (j = 1; j <= i_1; ++j) {
	    w[j] += tz[j];
/* L90: */
	}
	itnum = judy;
/* COMPUTE QUANTITIES USED FOR OPTIMAL STEP SIZE ESTIMATION. */
	if (judy == 1) {
	    lcalc = dnrm2_(&np1, &tz[1], &c_1);
	    rcalc = rholen;
	    i_1 = np1;
	    for (j = 1; j <= i_1; ++j) {
		z1[j] = w[j];
/* L110: */
	    }
	} else if (judy == 2) {
	    lcalc = dnrm2_(&np1, &tz[1], &c_1) / lcalc;
	    rcalc = rholen / rcalc;
	}
/* GO TO MOP-UP SECTION AFTER CONVERGENCE. */
	if (dnrm2_(&np1, &tz[1], &c_1) <= *relerr * dnrm2_(&np1, &w[1], &
		c_1) + *abserr) {
	    goto L600;
	}

/* L200: */
    }

/* NO CONVERGENCE IN  LITFH  ITERATIONS.  REDUCE  H  AND TRY AGAIN. */
    if (*h <= fouru * (*s + (float)1.)) {
	*iflag = 6;
	return 0;
    }
    *h *= (float).5;
    goto L70;

/* ***** END OF STARTUP SECTION. ***** */

/* ***** PREDICTOR SECTION. ***** */

L300:
    fail = FALSE_;
/* COMPUTE POINT PREDICTED BY HERMITE INTERPOLANT.  USE STEP SIZE  H */
/* COMPUTED ON LAST CALL TO  STEPNF . */
L320:
    i_1 = np1;
    for (j = 1; j <= i_1; ++j) {
	d_1 = *hold + *h;
	temp = ((((yp[j] - (y[j] - yold[j]) / *hold) / *hold - ((y[j] - yold[
		j]) / *hold - ypold[j]) / *hold) / *hold * (d_1 - *hold) + ((
		y[j] - yold[j]) / *hold - ypold[j]) / *hold) * d_1 + ypold[j]
		) * d_1 + yold[j];
	w[j] = temp;
	z0[j] = temp;
/* L330: */
    }

/* ***** END OF PREDICTOR SECTION. ***** */

/* ***** CORRECTOR SECTION. ***** */

    for (judy = 1; judy <= 4; ++judy) {
	rholen = (float)-1.;
/* CALCULATE THE NEWTON STEP  TZ  AT THE CURRENT POINT  W . */
	tangnf_(&rholen, &w[1], &wp[1], &yp[1], &a[1], &qr[qr_offset], 
		&alpha[1], &tz[1], (int *)&pivot[1], (int *)nfe, 
		(int *)n, (int *)iflag, &par[1], (int *)&ipar[1]);
	if (*iflag > 0) {
	    return 0;
	}

/* TAKE NEWTON STEP AND CHECK CONVERGENCE. */
	i_1 = np1;
	for (j = 1; j <= i_1; ++j) {
	    w[j] += tz[j];
/* L420: */
	}
	itnum = judy;
/* COMPUTE QUANTITIES USED FOR OPTIMAL STEP SIZE ESTIMATION. */
	if (judy == 1) {
	    lcalc = dnrm2_(&np1, &tz[1], &c_1);
	    rcalc = rholen;
	    i_1 = np1;
	    for (j = 1; j <= i_1; ++j) {
		z1[j] = w[j];
/* L440: */
	    }
	} else if (judy == 2) {
	    lcalc = dnrm2_(&np1, &tz[1], &c_1) / lcalc;
	    rcalc = rholen / rcalc;
	}
/* GO TO MOP-UP SECTION AFTER CONVERGENCE. */
	if (dnrm2_(&np1, &tz[1], &c_1) <= *relerr * dnrm2_(&np1, &w[1], &
		c_1) + *abserr) {
	    goto L600;
	}

/* L500: */
    }

/*NO CONVERGENCE IN  LITFH  ITERATIONS.  RECORD FAILURE AT CALCULATED  H ,
*/
/* SAVE THIS STEP SIZE, REDUCE  H  AND TRY AGAIN. */
    fail = TRUE_;
    hfail = *h;
    if (*h <= fouru * (*s + (float)1.)) {
	*iflag = 6;
	return 0;
    }
    *h *= (float).5;
    goto L320;

/* ***** END OF CORRECTOR SECTION. ***** */

/* ***** MOP-UP SECTION. ***** */

/* YOLD  AND  Y  ALWAYS CONTAIN THE LAST TWO POINTS FOUND ON THE ZERO */
/* CURVE OF THE HOMOTOPY MAP.  YPOLD  AND  YP  CONTAIN THE TANGENT */
/* VECTORS TO THE ZERO CURVE AT  YOLD  AND  Y , RESPECTIVELY. */

L600:
    i_1 = np1;
    for (j = 1; j <= i_1; ++j) {
	yold[j] = y[j];
	ypold[j] = yp[j];
	y[j] = w[j];
	yp[j] = wp[j];
	w[j] = y[j] - yold[j];
/* L620: */
    }
/* UPDATE ARC LENGTH. */
    *hold = dnrm2_(&np1, &w[1], &c_1);
    *s += *hold;

/* ***** END OF MOP-UP SECTION. ***** */

/* ***** OPTIMAL STEP SIZE ESTIMATION SECTION. ***** */

/* CALCULATE THE DISTANCE FACTOR  DCALC . */
/* L700: */
    i_1 = np1;
    for (j = 1; j <= i_1; ++j) {
	tz[j] = z0[j] - y[j];
	w[j] = z1[j] - y[j];
/* L710: */
    }
    dcalc = dnrm2_(&np1, &tz[1], &c_1);
    if (dcalc != (float)0.) {
	dcalc = dnrm2_(&np1, &w[1], &c_1) / dcalc;
    }

/* THE OPTIMAL STEP SIZE HBAR IS DEFINED BY */

/*   HT=HOLD * [MIN(LIDEAL/LCALC, RIDEAL/RCALC, DIDEAL/DCALC)]**(1/P) */

/*     HBAR = MIN [ MAX(HT, BMIN*HOLD, HMIN), BMAX*HOLD, HMAX ] */

/* IF CONVERGENCE HAD OCCURRED AFTER 1 ITERATION, SET THE CONTRACTION */
/* FACTOR  LCALC  TO ZERO. */
    if (itnum == 1) {
	lcalc = (float)0.;
    }
/* FORMULA FOR OPTIMAL STEP SIZE. */
    if (lcalc + rcalc + dcalc == (float)0.) {
	ht = sspar[7] * *hold;
    } else {
/* Computing MAX */
	d_2 = lcalc / sspar[1], d_3 = rcalc / sspar[2], d_2 = max(d_2,
		d_3), d_3 = dcalc / sspar[3];
	d_1 = (float)1. / max(d_2,d_3);
	d_4 = (float)1. / sspar[8];
 /*	ht = pow_dd(&d_1, &d_4) * *hold; */ ht=pow(d_1,d_4) * *hold; 
    }
/*  HT  CONTAINS THE ESTIMATED OPTIMAL STEP SIZE.  NOW PUT IT WITHIN */
/* REASONABLE BOUNDS. */
/* Computing MIN */
/* Computing MAX */
    d_3 = ht, d_4 = sspar[6] * *hold, d_3 = max(d_3,d_4);
    d_1 = max(d_3,sspar[4]), d_2 = sspar[7] * *hold, d_1 = min(d_1,d_2);
    *h = min(d_1,sspar[5]);
    if (itnum == 1) {
/* IF CONVERGENCE HAD OCCURRED AFTER 1 ITERATION, DON'T DECREASE  H . 
*/
	*h = max(*h,*hold);
    } else if (itnum == 4) {
/* IF CONVERGENCE REQUIRED THE MAXIMUM  LITFH  ITERATIONS, DON'T */
/* INCREASE  H . */
	*h = min(*h,*hold);
    }
/* IF CONVERGENCE DID NOT OCCUR IN  LITFH  ITERATIONS FOR A PARTICULAR */
/* H = HFAIL , DON'T CHOOSE THE NEW STEP SIZE LARGER THAN  HFAIL . */
    if (fail) {
	*h = min(*h,hfail);
    }


    return 0;
} /* stepnf_ */

/* end stepnf.c */

/**********************************************************************/
/***************** implementation code from Pmatrix.c *****************/
/**********************************************************************/

#define min(i,j) ((i) < (j) ? (i): (j))
#define FALSE 0
#define TRUE 1
/*--------------------------------------------------------------- 
  vector/matrix type  a linear array of int, whith auxilary info.
       *) the number of elements that can be stored is in elt[0]
       *) the current number of rows is in elt[1]
       *) the current number of collumbs is in elt[2]
The actual data are then stored in row major order from elt[3] on
---------------------------------------------------------------*/
struct Pmatrix_t {
    int store;           /* maximum number of helts reserved */
    int topc;            /* effective number of columbs */
    int topr;            /* effective number of rows */
    int ncols;           /* number of elts between first elts of rows */
    polynomial1 *coords;
};

/*
   **  a couple of functions which are defined in files that
   **  I don't want to include right now, when things stableize
   **      bad_error -- used mainly to stop things when a function
   **                   is passed bad input, as things stablizes
   **                   less catastrophic reactions may be introduced.
   **      mem_alloc,
   **      mem_free -- call malloc and free with some extra book-keeping.
 */
void bad_error(const char *);


/* WEIRD - The first pair of declarations below are the global one,
which are now (6/13/99) included from pelutils.h.  They replace the 
second pair, which had been declared at this point, specifically
for this file, EVEN THOUGH THE ARGUMENT LIST IS DIFFERENT.
I do not understand. - AMM */
/*
void  *mem_malloc(int); 
void   mem_free(void *);
*/
/*
char *mem_malloc(size_t);
void mem_free(char *);
*/



/*-------------------------------------------------------------
 vector access macroes (which ignore any rows except for first)
-------------------------------------------------------------*/
#define Vstore(V)  ((V->store))	/* maximum #elts available */
#define Vlength(V) ((V->topr))	/* actual #elts stored      */
#define Vref1(V,i) (&(((V->coords)[i-1])))	/*acces ith elt (starting at 1) */
#define Vref0(V,i)  (&(((V->coords)[i])))	/*acces ith elt (starting at 0) */
#define Vref(V,i)  Vref1(V,i)

/*------------------------------------------------------------
 matrix access macroes
-------------------------------------------------------------*/
#define Mstore(V)  ((V->store))	/* maximum #elts available */
#define MMrows(V)  ((V->store/V->ncols))	/* maximum #rows          */
#define Mrows(V) ((V->topr))	/* number rows stored */
#define Mcols(V) ((V->topc))	/* number cols stored */
#define MNcols(V) ((V->ncols))  /* number cols stored */

#define Mref1(V,i,j) (&(((V->coords)[(i-1)*(V->ncols)+j-1])))
#define Mref0(V,i,j) (&(((V->coords)[i*(V->ncols)+j])))
#define Mref(V,i,j)  Mref1((V),i,j)



int PMstore(Pmatrix M)
{
    return Mstore(M);
}
int PMMrows(Pmatrix M)
{
    return MMrows(M);
}
int PMrows(Pmatrix M)
{
    return Mrows(M);
}
int PMcols(Pmatrix M)
{
    return Mcols(M);
}
polynomial1 *PMref1(Pmatrix M, int i, int j)
{
    return Mref1(M, i, j);
}



/*
   **   Constructor/Destructors for Pmatrixes
   ** 
   ** Pmatrix Pmatrix_free(int r, int c); 
   **       New Pmatrix cabable of holding r rows, and c collumbs.
   ** Pmatrix Pmatrix_new(Pmatrix V);
 */

Pmatrix Pmatrix_new(int r, int c)
{
    Pmatrix V;
    int i, j;
    V = (Pmatrix) mem_malloc(sizeof(struct Pmatrix_t));
    if (!V)
	bad_error("allocation failure in Pmatrix_new()");
    V->coords = (polynomial1 *) mem_malloc(r * c * sizeof(polynomial1));
    if (!V)
	bad_error("allocation failure 2 in Pmatrix_new()");
    Mstore(V) = r * c;
    Mrows(V) = r;
    Mcols(V) = c;
    MNcols(V) = c;
    for (i = 1; i <= r; i++) {
	for (j = 1; j <= c; j++)
	    *Mref(V, i, j)=nullptr;
    }
    return V;
}

void Pmatrix_free(Pmatrix V)
{
    int i, j;

    if (V != nullptr && V->coords != nullptr) {
	for (i = 1; i <= Mrows(V); i++)
	    for (j = 1; j <= Mcols(V); j++)
		freeP(*Mref(V, i, j));
	mem_free((char *) (V->coords));
    }
    if (V != nullptr)
	mem_free((char *) (V));
}


/*
   ** Pmatrix_resize(R,r,c)
   **   Reset R to hold an r,by  c matrix.
   **   if R has enough storage to hold an rxc matrix resets
   **   row and columb entrees of r to r and c. otherwise
   **   frees R and reallocates an rxc matrix
   ** DOES NOT PRESERVE INDECIES OF EXISTING DATA
 */
Pmatrix Pmatrix_resize(Pmatrix R, int r, int c)
{

    if (R == nullptr || Mstore(R) < (r * c)) {
        if (R != nullptr) Pmatrix_free(R);
        R = Pmatrix_new(r, c);
    } else {
        Mrows(R) = r;
        Mcols(R) = c;
        MNcols(R) = c;
    }
    return R;
}

Pmatrix Pmatrix_submat(Pmatrix R, int r, int c)
{

    if (R == nullptr || c > Mcols(R) || r > Mrows(R) * MNcols(R)) {
        bad_error("bad subscripts or zero matrix in Pmatrix_submat()");
    } else {
        Mrows(R) = r;
        Mcols(R) = c;
    }
    return R;
}

/*
   **  Pmatrix_print(M):  print an Pmatrix
   **    if M is null print <<>> and return fail.
   **    otherwise print matrix and return true.
 */
Pmatrix Pmatrix_print(Pmatrix M)
{
    int i, j;

    if (M == nullptr) {
	printf("<<>>");
	return nullptr;
    }
    printf("<");
    for (i = 1; i <= Mrows(M); i++) {
	printf("<");
	for (j = 1; j <= Mcols(M); j++) {
	    printP(*Mref(M, i, j));
	    if (j < Mcols(M))
		printf(", ");
	}
	printf(">");
	if (i < Mrows(M))
	    printf("\n");
    }
    printf(">");
    return M;
}

Pmatrix Pmatrix_copy(Pmatrix P){
  Pmatrix C;
 int i,j;
  C=Pmatrix_new(Mrows(P),Mcols(P));
  for(i=1;i<=Mrows(P);i++){
    for(j=1;j<=Mcols(P);j++){
      *Mref(C,i,j)=copyP(*Mref(P,i,j));
    }
  }
 return C;
}


/* end Pmatrix.c */


/**********************************************************************/
/********* implementation code from the original Homotopies.c *********/
/**********************************************************************/

/*------------------------------------------------------------------
Homotopies.c       created 9/15/1994         last modified 9/15/994
                               Birk Huber     (birk@math.cornell.edu
ALL RIGHTS RESERVED

Store and evaluate Homotopies for Continuation module of Pelican.
the functions rho and rhojac are defined to be usable by the 
hompack code (originaly in fortran translated with the help of f2c).

The representation of Homotopies and implementation is very similar
to that used by hompack and described in Morgans book.
-------------------------------------------------------------------*/

/*------------------------------------------------------------------
 globals representing the homotopy and macroes for accessing them
 The system of complex polynomial1s is represented in real form 
-----------------------------------------------------------------*/

/* parameters affecting the homotopy */
int Hom_defd = 0;         /* 0 no homotopy initialized , 1 else*/
int Hom_use_proj = 1;     /* 0 dont use proj trans, 1 else*/
int Hom_num_vars = 0;     /* number of complex vars in curr hom*/

/* private variables defining homotopy */
static int NV,N,N1,M; 
static int *Starting_Monomial; 
static int *Number_of_Monomials;
static int *Exponents; 
static int *Hdegree;
static int *Edegree;
static double *Coefitients; 
static int *Deformation; 
static double *Proj_Trans;

/* index in monomial list (starting at 0) of equation i*/
#define monst(i,j) ((Starting_Monomial[(i)-1])+(j)-1)

/* number of monomials in the ith equation */
#define Nmon(i) (Number_of_Monomials[(i)-1])

/* exponent of variable k in monomial j of equation i */
#define Exp(i,j,k) Exponents[(monst(i,j))*NV+(k)-1]

/* get the exponent of the deformation parameter in the jth monomial
    of the ith equation */
#define Def(i,j) Deformation[monst(i,j)]

/* get the real and imaginary part of jth monomial of ith equation */
#define RCoef(i,j) Coefitients[2*(monst(i,j))]
#define ICoef(i,j) Coefitients[2*(monst(i,j))+1]

/* real and imaginary parts of coordinates defining the 
   projective transformation */
#define RPtrans(j) Proj_Trans[2*(j)-2]
#define IPtrans(j) Proj_Trans[2*(j)-1]

/* exponent of homogenizing variable in jthmonomial of ith equation*/
#define Hdeg(i,j) Hdegree[monst(i,j)]

/* degree of equation i */
#define Edeg(i) Edegree[i-1]

extern Pring Def_Ring;
Pvector psys_to_Pvec(psys sys){
 polynomial1 tmpm,tmpp;
 int j;
 Pvector PV;
 PV=Pvector_new(psys_d(sys));
 FORALL_POLY(sys,
   tmpp=nullptr;
   FORALL_MONO(sys,
     tmpm=makeP(Def_Ring);
     *poly_coef(tmpm)=Complex(*psys_coef_real(sys),
                              *psys_coef_imag(sys));
     *poly_def(tmpm)=*psys_def(sys);
     *poly_homog(tmpm)=*psys_homog(sys);
     for(j=1;j<=psys_d(sys);j++) *poly_exp(tmpm,j)=*psys_exp(sys,j);
     tmpp=addPPP(tmpp,tmpm,tmpp);
     freeP(tmpm);
   )
   *PVref(PV,psys_eqno(sys))=tmpp;
 )
 return PV;
}

/*-------------------------------------------------------------------
 init_hom  takes a Pvector and loads the above data structures 
           to hold a representation of the system.
-------------------------------------------------------------------*/
int init_hom(psys PS){
Pvector P;
polynomial1 ptr;
int i,j,k;
double t;
int seed=12;

P=psys_to_Pvec(PS);
NV=poly_dim(*PMref(P,1,1)); /* number of complex variables*/
N=2*NV;             /* number of real coordinates */
N1=N+1;             /* number of realcoords incl lift coord*/


/* check that P has n elts, and that Ring has NV vars */
if ( NV != PMcols(P)) {
      printf("warning nonsquare homotopy in init_HPK\n");
      Hom_defd=0;
}

/* Set number of vars for external use */
Hom_num_vars=NV;

/* free all work space */
Dfree(0); Ifree(0);

/* count monomials in equations to initialize monst and Nmon*/
Starting_Monomial=Ires(NV+1);
Number_of_Monomials=Ires(NV);

Starting_Monomial[0]=0;
for(i=0;i<NV;i++){
    Number_of_Monomials[i]=0; ptr=*PMref(P,1,i+1);
    while(ptr!=nullptr){Number_of_Monomials[i]++; ptr=poly_next(ptr);}
    Starting_Monomial[i+1]=
              Starting_Monomial[i]+Number_of_Monomials[i];
}
M=Starting_Monomial[NV];

Exponents=Ires(NV*M);
Coefitients=Dres(M*2);
Deformation=Ires(M);
Hdegree=Ires(M);
Edegree=Ires(NV);
Proj_Trans=Dres(2*NV+2);

#if defined(HAVE_SRAND48)
srand48(seed);
#else
srand(seed);
#endif  /* defined(HAVE_SRAND48) */

for(i=1;i<=NV;i++){                             
    j=1; ptr=*PMref(P,1,i); Edeg(i)=0;
    while(ptr!=nullptr){
      Hdeg(i,j)=0;
      RCoef(i,j)=(*poly_coef(ptr)).r; 
      ICoef(i,j)=(*poly_coef(ptr)).i; 
      for(k=1;k<=NV;k++){
                  Exp(i,j,k)=*poly_exp(ptr,k);
                  Hdeg(i,j)+=Exp(i,j,k);
                }
      Edeg(i)=max(Edeg(i),Hdeg(i,j));
      Def(i,j)=*poly_def(ptr); 
      j++; ptr=poly_next(ptr);
    }
    for(j=1;j<=Nmon(i);j++) Hdeg(i,j)=Edeg(i)-Hdeg(i,j); 
}      

/*Define Projective transformation */
for(j=1;j<=NV+1;j++){
#if defined(HAVE_DRAND48)
  t=drand48()*2*PI;
#else
  t=rand()*2*PI;
#endif  /* defined(HAVE_DRAND48) */
  RPtrans(j)=cos(t);
  IPtrans(j)=sin(t); 
 }
Hom_defd=1;   
Pvector_free(P);
return Hom_defd;
}

void print_homog(double *x,double *coord_r,double *coord_i){
 int i;
 fcomplex PN;
 PN=Complex(RPtrans((NV+1)),IPtrans((NV+1)));
 for(i=1;i<=NV;i++)
       PN=Cadd(PN,Cmul(Complex(RPtrans(i),IPtrans(i)),
                       Complex(x[2*i-2],x[2*i-1])));
  *coord_r=PN.r;
  *coord_i=PN.i;
 }

void print_proj_trans(){
 int i;
#ifdef HOM_PRINT
 fprintf(Hom_LogFile,"T %d ", NV)
#endif
; 
   for(i=1;i<=NV+1;i++) {
#ifdef HOM_PRINT
      fprintf(Hom_LogFile,"%10g %10g",RPtrans(i),IPtrans(i))
#endif
;
   }
#ifdef HOM_PRINT
 fprintf(Hom_LogFile,"\n")
#endif
; 
 }

 
/* 
** Htransform takes a vector in RP^(2*NV) and scales it so that
**            it lies on the hyperplane defining the projective 
**            transformation. (Using the scaling factor
**                             L=pn1/(Z-(p1X1+...+pnXn)).)
** Huntransform takes a 2*NV+2 dvector whoose first 2*NV coords
**          define a point in the affine chart defined by the 
**          projective transformation and fills in the last two
**          coords with the value Z=pn1+p1X1+...+pnXn. 
**
*/
#define X(i) (DVref(X,i))
void Htransform(Dvector X){
  int i;
  fcomplex C,L;
  
   L=Complex(0.0,0.0);
   for(i=1;i<=NV;i++){
     L=Cadd(L,Cmul(Complex(RPtrans(i),IPtrans(i)),
                   Complex(X(2*i+1),X(2*i+2))));
   }
   L=Cdiv(Complex(RPtrans(NV+1),IPtrans(NV+1)),
          Csub(Complex(X(1),X(2)),L));
  for(i=1;i<=NV+1;i++) {
      C=Cmul(L,Complex(X(2*i-1),X(2*i)));
      X(2*i-1)=C.r;
      X(2*i)  =C.i;
  }
 }

void Huntransform(Dvector X){
  int i;
  fcomplex L;
   L=Complex(RPtrans(NV+1),IPtrans(NV+1));
   for(i=1;i<=NV;i++) {
     L=Cadd(L,Cmul(Complex(RPtrans(i),IPtrans(i)),
                   Complex(X(2*i+1),X(2*i+2))));
   }
   X(1)=L.r;
   X(2)=L.i;
 }
#undef X

fcomplex Hpath(double t){ return Complex(t,0.0);}

fcomplex DHpath(double t){return Complex(1.0,0.0);}

int rho_(double *a, 
	 double *lambda, 
	 double *x, 
	 double *v, 
	 double *par, 
	 int    *ipar)
{int i,j,h;
 fcomplex c,Hpath(double),PN;
 if (*lambda < 0.0) *lambda = 0.;

 /* calculate projective coordinate */
if (Hom_use_proj==1){
 PN=Complex(RPtrans(NV+1),IPtrans(NV+1));
 for(i=1;i<=NV;i++) PN=Cadd(PN,Cmul(Complex(RPtrans(i),IPtrans(i)),
                                    Complex(x[2*i-2],x[2*i-1])));
}
 /* loop over equations */
 for(i=1;i<=NV;i++){
   v[2*i-2]=0.0;
   v[2*i-1]=0.0;
   for(j=1;j<=Nmon(i);j++){
      c=Cmul(Complex(RCoef(i,j),ICoef(i,j)),
             Cpow(Hpath(*lambda),Def(i,j)));
      for(h=1;h<=NV;h++) 
            c=Cmul(c,Cpow(Complex(x[2*h-2],x[2*h-1]),Exp(i,j,h)));
if (Hom_use_proj==1)   c=Cmul(c,Cpow(PN,Hdeg(i,j))); 
      v[2*i-2]+=c.r;
      v[2*i-1]+=c.i; 
   }
 }
 return 0;
} 

int rhojac_(double *a, 
	    double *lambda, 
	    double *x, 
	    double *v, 
	    int    *k, 
	    double *par, 
	    int    *ipar)
{
 int i,j,h,d;
 fcomplex c,Hpath(double), DHpath(double),PN;  
 double t;

 if (*lambda < 0.) *lambda = 0.;
 if(Hom_use_proj==1){
   PN=Complex(RPtrans((NV+1)),IPtrans((NV+1)));
   for(i=1;i<=NV;i++) 
       PN=Cadd(PN,Cmul(Complex(RPtrans(i),IPtrans(i)),
                       Complex(x[2*i-2],x[2*i-1])));
 }
 if (*k>1){
   d=(*k)/2;     /* determine the complex var to diff by.*/
   for(i=1;i<=NV;i++){
     v[2*i-2]=0.0;
     v[2*i-1]=0.0;
     if (Hom_use_proj==1){
       for(j=1;j<=Nmon(i);j++){
         if (Hdeg(i,j)!=0){
          c=Cmul(Complex(RCoef(i,j),ICoef(i,j)),
                 Cpow(Hpath(*lambda),Def(i,j)));
          for(h=1;h<=NV;h++)
                c=Cmul(c,Cpow(Complex(x[2*h-2],x[2*h-1]),Exp(i,j,h)));
          c=Cmul(c,Cpow(PN,Hdeg(i,j)-1));
          c=Cmul(c,Complex(t=Hdeg(i,j),0.0));
          c=Cmul(c,Complex(RPtrans(d),IPtrans(d)));
          v[2*i-2]+=c.r;
          v[2*i-1]+=c.i;
         }
       }
     }
         
     for(j=1;j<=Nmon(i);j++){
      if (Exp(i,j,d)!=0){
        c=Cmul(Complex(RCoef(i,j),ICoef(i,j)),
               Cpow(Hpath(*lambda),Def(i,j)));
        for(h=1;h<d;h++)
              c=Cmul(c,Cpow(Complex(x[2*h-2],x[2*h-1]),Exp(i,j,h)));
        c=Cmul(c,Cpow(Complex(x[2*d-2],x[2*d-1]),Exp(i,j,d)-1));
        c=Cmul(c,Complex(t=Exp(i,j,d),0.0));
        for(h=d+1;h<=NV;h++)
              c=Cmul(c,Cpow(Complex(x[2*h-2],x[2*h-1]),Exp(i,j,h)));
if(Hom_use_proj==1)  c=Cmul(c,Cpow(PN,Hdeg(i,j)));
        v[2*i-2]+=c.r;
        v[2*i-1]+=c.i;
      }
     }
     if(((*k)%2)==1) {
      t=v[2*i-2];
      v[2*i-2]=-1.0*v[2*i-1];
      v[2*i-1]=t;
     }
   }
 }
 else {
   for(i=1;i<=NV;i++){
     v[2*i-2]=0.0;
     v[2*i-1]=0.0;
     for(j=1;j<=Nmon(i);j++){
      if(Def(i,j)!=0){
        c=Cmul(Complex(RCoef(i,j),ICoef(i,j)),
               Cpow(Hpath(*lambda),Def(i,j)-1));
        c=Cmul(c,Cmul(Complex(t=Def(i,j),0.0),DHpath(*lambda)));
        for(h=1;h<=NV;h++)
              c=Cmul(c,Cpow(Complex(x[2*h-2],x[2*h-1]),Exp(i,j,h)));
if(Hom_use_proj==1) c=Cmul(c,Cpow(PN,Hdeg(i,j)));
        v[2*i-2]+=c.r;
        v[2*i-1]+=c.i;
      }
     }
   }
 }
return 0;
}

/* end, original Homotopies.c */


/**************************************************************************/
/********************* implementations from tangnf.c **********************/
/**************************************************************************/

/* tangnf.f -- translated by f2c (version 19940305).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/


/* Table of constant values */

/* Subroutine */ int tangnf_(double *rholen, 
			     double *y, 
			     double *yp, 
			     double *ypold, 
			     double *a, 
			     double *qr, 
			     double *alpha, 
			     double *tz, 
			     int    *pivot, 
			     int    *nfe, 
			     int    *n, 
			     int    *iflag, 
			     double *par, 
			     int    *ipar)
{
    /* System generated locals */
    int qr_dim1, qr_offset, i_1, i_2, i_3;
    double d_1;

    /* Builtin functions */
    /*    double sqrt(); CANT DECLARE BUILTINS UNDER C++ */

    /* Local variables */
    /*    extern */ /* Subroutine */ /* int fjac_(); */
    static double beta;
    static int jbar;
    /*     extern double ddot_(); */
    static double qrkk;
    /*    extern double dnrm2_(); */
    /*    extern */ /* Subroutine */ /* int f_(); */
    static int i, j, k;
    static double sigma, lambda, alphak;
    /*    extern */ /* Subroutine */ /* int rhojac_(); */
    static int kp1, np1, np2;
    static double ypnorm;
    /*    extern */ /* Subroutine */ /* int rho_(); */
    static double sum;


/* THIS SUBROUTINE BUILDS THE JACOBIAN MATRIX OF THE HOMOTOPY MAP, */
/* COMPUTES A QR DECOMPOSITION OF THAT MATRIX, AND THEN CALCULATES THE */
/* (UNIT) TANGENT VECTOR AND THE NEWTON STEP. */

/* ON INPUT: */

/* RHOLEN < 0 IF THE NORM OF THE HOMOTOPY MAP EVALUATED AT */
/*    (A, LAMBDA, X) IS TO BE COMPUTED.  IF  RHOLEN >= 0  THE NORM IS NOT 
*/
/*    COMPUTED AND  RHOLEN  IS NOT CHANGED. */

/* Y(1:N+1) = CURRENT POINT (LAMBDA(S), X(S)). */

/* YPOLD(1:N+1) = UNIT TANGENT VECTOR AT PREVIOUS POINT ON THE ZERO */
/*    CURVE OF THE HOMOTOPY MAP. */

/* A(1:*) = PARAMETER VECTOR IN THE HOMOTOPY MAP. */

/* QR(1:N,1:N+2), ALPHA(1:N), TZ(1:N+1), PIVOT(1:N+1)  ARE WORK ARRAYS */
/*    USED FOR THE QR FACTORIZATION. */

/* NFE = NUMBER OF JACOBIAN MATRIX EVALUATIONS = NUMBER OF HOMOTOPY */
/*    FUNCTION EVALUATIONS. */

/* N = DIMENSION OF X. */

/* IFLAG = -2, -1, OR 0, INDICATING THE PROBLEM TYPE. */

/* PAR(1:*) AND IPAR(1:*) ARE ARRAYS FOR (OPTIONAL) USER PARAMETERS, */
/*    WHICH ARE SIMPLY PASSED THROUGH TO THE USER WRITTEN SUBROUTINES */
/*    RHO, RHOJAC. */

/* ON OUTPUT: */

/* RHOLEN = ||RHO(A, LAMBDA(S), X(S)|| IF  RHOLEN < 0  ON INPUT. */
/*    OTHERWISE  RHOLEN  IS UNCHANGED. */

/* Y, YPOLD, A, N  ARE UNCHANGED. */

/* YP(1:N+1) = DY/DS = UNIT TANGENT VECTOR TO INTEGRAL CURVE OF */
/*    D(HOMOTOPY MAP)/DS = 0  AT  Y(S) = (LAMBDA(S), X(S)) . */

/* TZ = THE NEWTON STEP = -(PSEUDO INVERSE OF  (D RHO(A,Y(S))/D LAMBDA , 
*/
/*    D RHO(A,Y(S))/DX)) * RHO(A,Y(S)) . */

/* NFE  HAS BEEN INCRMENTED BY 1. */

/* IFLAG  IS UNCHANGED, UNLESS THE QR FACTORIZATION DETECTS A RANK < N, */
/*    IN WHICH CASE THE TANGENT AND NEWTON STEP VECTORS ARE NOT COMPUTED 
*/
/*    AND  TANGNF  RETURNS WITH  IFLAG = 4 . */


/* CALLS  DDOT , DNRM2 , F (OR  RHO ), FJAC (OR  RHOJAC ). */


/* *****  ARRAY DECLARATIONS.  ***** */


/* ARRAYS FOR COMPUTING THE JACOBIAN MATRIX AND ITS KERNEL. */

/* *****  END OF DIMENSIONAL INFORMATION.  ***** */


    /* Parameter adjustments */
    --pivot;
    --tz;
    --alpha;
    qr_dim1 = *n;
    qr_offset = qr_dim1 + 1;
    qr -= qr_offset;
    --a;
    --ypold;
    --yp;
    --y;
    --par;
    --ipar;

    /* Function Body */
    lambda = y[1];
    np1 = *n + 1;
    np2 = *n + 2;
    ++(*nfe);
/* NFE CONTAINS THE NUMBER OF JACOBIAN EVALUATIONS. */
/*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   * */

/* COMPUTE THE JACOBIAN MATRIX, STORE IT AND HOMOTOPY MAP IN QR. */

    if (*iflag == -2) {

/*  QR = ( D RHO(A,LAMBDA,X)/D LAMBDA , D RHO(A,LAMBDA,X)/DX , */
/*                                              RHO(A,LAMBDA,X) )  . 
*/

	i_1 = np1;
	for (k = 1; k <= i_1; ++k) {
	    rhojac_(&a[1], &lambda, &y[2], &qr[k * qr_dim1 + 1], &k, &par[1], 
		    &ipar[1]);
/* L30: */
	}
	rho_(&a[1], &lambda, &y[2], &qr[np2 * qr_dim1 + 1], &par[1], &ipar[1])
		;
    } else {
	f_(&y[2], &tz[1]);
	if (*iflag == 0) {

/*      QR = ( A - F(X), I - LAMBDA*DF(X) , */
/*                                 X - A + LAMBDA*(A - F(X)) )  . 
*/

	    i_1 = *n;
	    for (j = 1; j <= i_1; ++j) {
		sigma = a[j];
		beta = sigma - tz[j];
		qr[j + qr_dim1] = beta;
/* L100: */
		qr[j + np2 * qr_dim1] = y[j + 1] - sigma + lambda * beta;
	    }
	    i_1 = *n;
	    for (k = 1; k <= i_1; ++k) {
		fjac_(&y[2], &tz[1], (integer *)&k);
		kp1 = k + 1;
		i_2 = *n;
		for (j = 1; j <= i_2; ++j) {
/* L110: */
		    qr[j + kp1 * qr_dim1] = -lambda * tz[j];
		}
/* L120: */
		qr[k + kp1 * qr_dim1] += (float)1.;
	    }
	} else {

/*   QR = ( F(X) - X + A, LAMBDA*DF(X) + (1 - LAMBDA)*I , */
/*                                  X - A + LAMBDA*(F(X) - X + A) 
)  . */

/* L140: */
	    i_1 = *n;
	    for (j = 1; j <= i_1; ++j) {
		sigma = y[j + 1] - a[j];
		beta = tz[j] - sigma;
		qr[j + qr_dim1] = beta;
/* L150: */
		qr[j + np2 * qr_dim1] = sigma + lambda * beta;
	    }
	    i_1 = *n;
	    for (k = 1; k <= i_1; ++k) {
		fjac_(&y[2], &tz[1], (integer *)&k);
		kp1 = k + 1;
		i_2 = *n;
		for (j = 1; j <= i_2; ++j) {
/* L160: */
		    qr[j + kp1 * qr_dim1] = lambda * tz[j];
		}
/* L170: */
		qr[k + kp1 * qr_dim1] = (float)1. - lambda + qr[k + kp1 * 
			qr_dim1];
	    }
	}
    }

/*   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   * */
/* COMPUTE THE NORM OF THE HOMOTOPY MAP IF IT WAS REQUESTED. */
    if (*rholen < (float)0.) {
	*rholen = dnrm2_((integer *)n, &qr[np2 * qr_dim1 + 1], &c_1);
    }

/* REDUCE THE JACOBIAN MATRIX TO UPPER TRIANGULAR FORM. */

/* THE FOLLOWING CODE IS A MODIFICATION OF THE ALGOL PROCEDURE */
/* DECOMPOSE  IN P. BUSINGER AND G. H. GOLUB, LINEAR LEAST */
/* SQUARES SOLUTIONS BY HOUSEHOLDER TRANSFORMATIONS, */
/* NUMER. MATH. 7 (1965) 269-276. */

    i_1 = np1;
    for (j = 1; j <= i_1; ++j) {
	yp[j] = ddot_((integer *)n, &qr[j * qr_dim1 + 1], 
		      &c_1, &qr[j * qr_dim1 + 1], &c_1);
/* L220: */
	pivot[j] = j;
    }
    i_1 = *n;
    for (k = 1; k <= i_1; ++k) {
	sigma = yp[k];
	jbar = k;
	kp1 = k + 1;
	i_2 = np1;
	for (j = kp1; j <= i_2; ++j) {
	    if (sigma >= yp[j]) {
		goto L240;
	    }
	    sigma = yp[j];
	    jbar = j;
L240:
	    ;
	}
	if (jbar == k) {
	    goto L260;
	}
	i = pivot[k];
	pivot[k] = pivot[jbar];
	pivot[jbar] = i;
	yp[jbar] = yp[k];
	yp[k] = sigma;
	i_2 = *n;
	for (i = 1; i <= i_2; ++i) {
	    sigma = qr[i + k * qr_dim1];
	    qr[i + k * qr_dim1] = qr[i + jbar * qr_dim1];
	    qr[i + jbar * qr_dim1] = sigma;
/* L250: */
	}
/*   END OF COLUMN INTERCHANGE. */
L260:
	i_2 = *n - k + 1;
	sigma = ddot_((integer *)&i_2, &qr[k + k * qr_dim1], 
		      &c_1, &qr[k + k * qr_dim1], &c_1);
	if (sigma == (float)0.) {
	    *iflag = 4;
	    return 0;
	}
/* L270: */
	if (k == *n) {
	    goto L300;
	}
	qrkk = qr[k + k * qr_dim1];
	alphak = -sqrt(sigma);
	if (qrkk < (float)0.) {
	    alphak = -alphak;
	}
	alpha[k] = alphak;
	beta = (float)1. / (sigma - qrkk * alphak);
	qr[k + k * qr_dim1] = qrkk - alphak;
	i_2 = np2;
	for (j = kp1; j <= i_2; ++j) {
	    i_3 = *n - k + 1;
	    sigma = beta * ddot_((integer *)&i_3, &qr[k + k * qr_dim1], 
				 &c_1, &qr[k + j * qr_dim1], &c_1);
	    i_3 = *n;
	    for (i = k; i <= i_3; ++i) {
		qr[i + j * qr_dim1] -= qr[i + k * qr_dim1] * sigma;
/* L280: */
	    }
	    if (j < np2) {
/* Computing 2nd power */
		d_1 = qr[k + j * qr_dim1];
		yp[j] -= d_1 * d_1;
	    }
/* L290: */
	}
L300:
	;
    }
    alpha[*n] = qr[*n + *n * qr_dim1];


/* COMPUTE KERNEL OF JACOBIAN, WHICH SPECIFIES YP=DY/DS. */
    tz[np1] = (float)1.;
    for (i = *n; i >= 1; --i) {
	sum = (float)0.;
	i_1 = np1;
	for (j = i + 1; j <= i_1; ++j) {
/* L330: */
	    sum += qr[i + j * qr_dim1] * tz[j];
	}
/* L340: */
	tz[i] = -sum / alpha[i];
    }
    ypnorm = dnrm2_((integer *)&np1, &tz[1], &c_1);
    i_1 = np1;
    for (k = 1; k <= i_1; ++k) {
/* L360: */
	yp[pivot[k]] = tz[k] / ypnorm;
    }
    if (ddot_((integer *)&np1, &yp[1], &c_1, &ypold[1], &c_1) >= (float)0.) {
	goto L380;
    }
    i_1 = np1;
    for (i = 1; i <= i_1; ++i) {
/* L370: */
	yp[i] = -yp[i];
    }
/* YP  IS THE UNIT TANGENT VECTOR IN THE CORRECT DIRECTION. */

/* COMPUTE THE MINIMUM NORM SOLUTION OF [D RHO(Y(S))] V = -RHO(Y(S)). */
/* V IS GIVEN BY  P - (P,Q)Q  , WHERE P IS ANY SOLUTION OF */
/* [D RHO] V = -RHO  AND Q IS A UNIT VECTOR IN THE KERNEL OF [D RHO]. */

L380:
    for (i = *n; i >= 1; --i) {
	sum = qr[i + np1 * qr_dim1] + qr[i + np2 * qr_dim1];
	i_1 = *n;
	for (j = i + 1; j <= i_1; ++j) {
/* L430: */
	    sum += qr[i + j * qr_dim1] * alpha[j];
	}
/* L440: */
	alpha[i] = -sum / alpha[i];
    }
    i_1 = *n;
    for (k = 1; k <= i_1; ++k) {
/* L450: */
	tz[pivot[k]] = alpha[k];
    }
    tz[pivot[np1]] = (float)1.;
/* TZ NOW CONTAINS A PARTICULAR SOLUTION P, AND YP CONTAINS A VECTOR Q */
/* IN THE KERNEL(THE TANGENT). */
    sigma = ddot_((integer *)&np1, &tz[1], &c_1, &yp[1], &c_1);
    i_1 = np1;
    for (j = 1; j <= i_1; ++j) {
	tz[j] -= sigma * yp[j];
/* L470: */
    }
/* TZ IS THE NEWTON STEP FROM THE CURRENT POINT Y(S) = (LAMBDA(S), X(S)). 
*/
    return 0;
} /* tangnf_ */

/* end tangnf.c */
