//
// FILE: hompack.cc -- Hompack code, 
//
// $Id$
//

#include <float.h>
#include <iomanip.h>
#include <iostream.h>
#include <math.h>
#include "gmisc.h"
#include "hompack.h"
#include "lapack.h"
#include "homotopy.h"

static void WritePXIHeader(gOutput &pxifile, const Nfg &N,
			   const HomQreParams &params)
{
  pxifile << "Dimensionality:\n";
  pxifile << N.NumPlayers() << " ";
  for (int pl = 1; pl <= N.NumPlayers(); pl++)
    pxifile << N.NumStrats(pl) << " ";
  pxifile << "\n";
  N.WriteNfgFile(pxifile, 6);

  pxifile << "Settings:\n" << params.minLam;
  pxifile << "\n" << params.maxLam << "\n" << params.delLam;
  pxifile << "\n" << 0 << "\n" << 1 << "\n" << params.powLam << "\n";
  
  int numcols = N.ProfileLength() + 2;

  pxifile << "DataFormat:\n" << numcols;
  
  for (int i = 1; i <= numcols; i++)
    pxifile << " " << i;
 
  pxifile << "\nData:\n";
}

/*
// Constants // "LIMITD is an upper bound on the number of steps."
  const int limitd = 10000;  //originally = 1000
// temporarily 100000 2/16/00 Vale Murthy
// This change seems to have affected the range of lambda when lambda is untransformed

// "Switch from the tolerance ???_arc_err to the (finer) tolerance
//  ???_ans_err if the curvature of any component of Y exceeds CURSW."
  const double curvature_limit = 10.0;

// The number of Newton iterations before reducing the step size.
  const int Newton_iter_limit = 4; //Originally at 4
*/  

// Constructor, destructor

template <class T> 
gHompack<T>::gHompack(const MixedProfile<gNumber> &s, const HomQreParams &p)
  : nfg(s.Game()), supp(s.Support()), start(s), params(p), limitd(10000), 
    Newton_iter_limit(4), curvature_limit(10.0) 
{
  int N = supp.TotalNumStrats()-nfg.NumPlayers();
  gVector<double> Y(N+1);
  gVector<double> A(0);
  gVector<double> ssp(8);
  double max_lambda = 1; // Vale Murthy 1/24/00
  
  int flag = -2;
  int jeval_num = 0;
  double arclength = 0;
  
  // initialize lambda to zero
  Y[1] = 0.0;
  
  // initialize Y
  int j = 1;
  for(int pl = 1;pl<=nfg.NumPlayers();pl++)
    for(int i = 1;i < supp.NumStrats(pl);i++) {
      Y[j+1] = start(pl,i);
      j++;
    }

  gout << "\nY: " << Y;  

  // Use default stepsize parameters
  ssp = 0;
  if (params.pxifile) 
    WritePXIHeader(*params.pxifile, nfg, params);
  
  fixpnf(N, Y, flag, 1., 1., .1, .1, true,
  	 A, ssp, jeval_num,arclength,max_lambda,false);
  gout << "Exiting flag: " << flag << "\n";
} 

template <class T> gHompack<T>::~gHompack(void)
{ } 


//
//  qofs: The code below is related to the  Hermite cubic predictor
//

template <class T> 
T gHompack<T>::qofs(T f0, T fp0, T f1, T fp1, T dels, T s)
{
  T dd01, dd001, dd011, dd0011;
  dd01 = (f1 - f0) / dels;
  dd001 = (dd01 - fp0) / dels;
  dd011 = (fp1 - dd01) / dels;
  dd0011 = (dd011 - dd001) / dels;
  return ((dd0011 * (s - dels) + dd001) * s + fp0) * s + f0;
}

template <class T> 
bool gHompack<T>::out_of_bounds(gVector<T> &W,int range)
{
  int i;
  
  for(i=1;i <= range;i++) {
    if ( (W[i] < 0) || (W[i] >1) ) return true;
  }
  return false;
}

template <class T> 
void gHompack<T>::build_prediction(gVector<T> &W,int N, gVector<T> &Y_old,
						gVector<T> &Ytan_old,gVector<T> &Y,
                  gVector<T> &Ytan,T old_step,T old_step_plus_step_size)
{
  for (int j = 1; j <= N+1; ++j)
    W[j] = qofs(Y_old[j], Ytan_old[j], Y[j], Ytan[j], old_step, old_step_plus_step_size);
}

 // fixpnf ///////////////////////////////////////////////////////////////
 /* Subroutine  FIXPNF  finds a fixed point or zero of the
  C N-dimensional vector function F(X), or tracks a zero curve
  C of a general homotopy map RHO(A,LAMBDA,X).  For the fixed
  C point problem F(X) is assumed to be a C2 map of some ball
  C into itself.  The equation  X = F(X)  is solved by
  C following the zero curve of the homotopy map
  C
  C  LAMBDA*(X - F(X)) + (1 - LAMBDA)*(X - A)  ,
  C
  C starting from LAMBDA = 0, X = A.  The curve is parameterized
  C by arc length S, and is followed by solving the ordinary
  C differential equation  D(HOMOTOPY MAP)/DS = 0  for
  C Y(S) = (LAMBDA(S), X(S)) using a Hermite cubic predictor and a
  C corrector which returns to the zero curve along the flow normal
  C to the Davidenko flow (which consists of the integral curves of
  C D(HOMOTOPY MAP)/DS ).
  C
  C For the zero finding problem F(X) is assumed to be a C2 map
  C such that for some R > 0,  X*F(X) >= 0  whenever NORM(X) = R.
  C The equation  F(X) = 0  is solved by following the zero curve
  C of the homotopy map
  C
  C   LAMBDA*F(X) + (1 - LAMBDA)*(X - A)
  C
  C emanating from LAMBDA = 0, X = A.
  C
  C  A  must be an interior point of the above mentioned balls.
  C
  C For the curve tracking problem RHO(A,LAMBDA,X) is assumed to
  C be a C2 map from E**M X [0,1) X E**N into E**N, which for

  C almost all parameter vectors A in some nonempty open subset
  C of E**M satisfies
  C
  C  rank [D RHO(A,LAMBDA,X)/D LAMBDA , D RHO(A,LAMBDA,X)/DX] = N
  C
  C for all points (LAMBDA,X) such that RHO(A,LAMBDA,X)=0.  It is
  C further assumed that
  C
  C           rank [ D RHO(A,0,X0)/DX ] = N  .
  C
  C With A fixed, the zero curve of RHO(A,LAMBDA,X) emanating
  C from  LAMBDA = 0, X = X0  is tracked until  LAMBDA = 1  by
  C solving the ordinary differential equation
  C D RHO(A,LAMBDA(S),X(S))/DS = 0  for  Y(S) = (LAMBDA(S), X(S)),
  C where S is arc length along the zero curve.  Also the homotopy
  C map RHO(A,LAMBDA,X) is assumed to be constructed such that
  C
  C              D LAMBDA(0)/DS > 0  .
  C
  C
  C For the fixed point and zero finding problems, the user must supply
  C a subroutine  F(X,V)  which evaluates F(X) at X and returns the
  C vector F(X) in V, and a subroutine  FJAC(X,V,K)  which returns in V
  C the Kth column of the Jacobian matrix of F(X) evaluated at X.  For
  C the curve tracking problem, the user must supply a subroutine
  C  RHO(A,LAMBDA,X,V)  which evaluates the homotopy map RHO at
  C (A,LAMBDA,X) and returns the vector RHO(A,LAMBDA,X) in V, and a
  C subroutine  RHOJAC(A,LAMBDA,X,V,K)  which returns in V the Kth
  C column of the N X (N+1) Jacobian matrix [D RHO/D LAMBDA, D RHO/DX]
  C evaluated at (A,LAMBDA,X).  FIXPNF  directly or indirectly uses
  C the subroutines  F (or  RHO ),  FJAC (or  RHOJAC ),
  C   ROOT,  ROOTNF,  STEPNF,  the LAPACK routines  DGEQPF,  DORMQR,
  C their auxiliary routines, and the BLAS routines  DCOPY,
  C   DDOT,  DGEMM,  DGEMV,  DGER,  DNRM2,  DSCAL,  DSWAP,  DTRMM,  DTRMV,
  C   IDAMAX.  The module  REAL_PRECISION  specifies 64-bit
  C real arithmetic, which the user may want to change.
  C
  C
  C ON INPUT:
  C
  C N  is the dimension of X, F(X), and RHO(A,LAMBDA,X).
  C
  C Y(:)  is an array of length  N + 1.  (Y(2),...,Y(N+1)) = A  is the
  C    starting point for the zero curve for the fixed point and
  C    zero finding problems.  (Y(2),...,Y(N+1)) = X0  for the curve
  C    tracking problem.
  C
  C IFLAG  can be -2, -1, 0, 2, or 3.  IFLAG  should be 0 on the
  C    first call to  FIXPNF  for the problem  X=F(X), -1 for the
  C    problem  F(X)=0, and -2 for the problem  RHO(A,LAMBDA,X)=0.
  C    In certain situations  IFLAG  is set to 2 or 3 by  FIXPNF,
  C    and  FIXPNF  can be called again without changing  IFLAG.
  C
  C ARCRE , ARCAE  are the relative and absolute errors, respectively,
  C    allowed the normal flow iteration along the zero curve.  If
  C    ARC?E .LE. 0.0  on input it is reset to  .5*SQRT(ANS?E) .
  C    Normally  ARC?E should be considerably larger than  ANS?E .
  C
  C ANSRE , ANSAE  are the relative and absolute error values used for
  C    the answer at LAMBDA = 1.  The accepted answer  Y = (LAMBDA, X)
  C    satisfies
  C
  C       |Y(1) - 1|  .LE.  ANSRE + ANSAE           .AND.
  C
  C       ||Z||  .LE.  ANSRE*||X|| + ANSAE          where
  C
  C    (.,Z) is the Newton step to Y.
  C
  C TRACE  is an integer specifying the logical I/O unit for
  C    intermediate output.  If  TRACE .GT. 0  the points computed on
  C    the zero curve are written to I/O unit  TRACE .
  C
  C A(:)  contains the parameter vector  A .  For the fixed point
  C    and zero finding problems, A  need not be initialized by the
  C    user, and is assumed to have length  N.  For the curve
  C    tracking problem, A  must be initialized by the user.
  C
  C SSPAR(1:8) = (LIDEAL, RIDEAL, DIDEAL, HMIN, HMAX, BMIN, BMAX, P)  is
  C    a vector of parameters used for the optimal step size estimation.
  C    If  SSPAR(J) .LE. 0.0  on input, it is reset to a default value
  C    by  FIXPNF .  Otherwise the input value of  SSPAR(J)  is used.
  C    See the comments below and in  STEPNF  for more information about
  C    these constants.
  C
  C POLY_SWITCH  is an optional logical variable used only by the driver
  C    POLSYS1H  for polynomial systems.
  C
  C
  C ON OUTPUT:
  C
  C N , TRACE , A  are unchanged.
  C
  C Y(1) = LAMBDA, (Y(2),...,Y(N+1)) = X, and Y is an approximate
  C    zero of the homotopy map.  Normally LAMBDA = 1 and X is a
  C    fixed point(zero) of F(X).  In abnormal situations LAMBDA
  C    may only be near 1 and X is near a fixed point(zero).
  C
  C IFLAG =
  C  -2   causes  FIXPNF  to initialize everything for the problem
  C       RHO(A,LAMBDA,X) = 0 (use on first call).
  C
  C  -1   causes  FIXPNF  to initialize everything for the problem
  C       F(X) = 0 (use on first call).
  C
  C   0   causes  FIXPNF  to initialize everything for the problem
  C       X = F(X) (use on first call).
  C
  C   1   Normal return.
  C
  C   2   Specified error tolerance cannot be met.  Some or all of
  C       ARCRE , ARCAE , ANSRE , ANSAE  have been increased to
  C       suitable values.  To continue, just call  FIXPNF  again
  C       without changing any parameters.
  C
  C   3   STEPNF  has been called 1000 times.  To continue, call
  C       FIXPNF  again without changing any parameters.
  C
  C   4   Jacobian matrix does not have full rank.  The algorithm
  C       has failed (the zero curve of the homotopy map cannot be
  C       followed any further).
  C
  C   5   The tracking algorithm has lost the zero curve of the
  C       homotopy map and is not making progress.  The error tolerances
  C       ARC?E  and  ANS?E  were too lenient.  The problem should be
  C       restarted by calling  FIXPNF  with smaller error tolerances
  C       and  IFLAG = 0 (-1, -2).
  C
  C   6   The normal flow Newton iteration in  STEPNF  or  ROOTNF
  C       failed to converge.  The error tolerances  ANS?E  may be too
  C       stringent.
  C
  C   7   Illegal input parameters, a fatal error.
  C
  C   8   Memory allocation error, fatal.
  C
  C ARCRE , ARCAE , ANSRE , ANSAE  are unchanged after a normal return
  C    (IFLAG = 1).  They are increased to appropriate values on the
  C    return  IFLAG = 2 .
  C
  C NFE  is the number of function evaluations (= number of
  C    Jacobian matrix evaluations).
  C
  C ARCLEN  is the length of the path followed.
  C
  C
  C Allocatable and automatic work arrays:
  C
  C YP(1:N+1)  is a work array containing the tangent vector to
  C    the zero curve at the current point  Y .
  C
  C YOLD(1:N+1)  is a work array containing the previous point found
  C    on the zero curve.
  C
  C YPOLD(1:N+1)  is a work array containing the tangent vector to
  C    the zero curve at  YOLD .
  C
  C QR(1:N,1:N+2), ALPHA(1:3*N+3), TZ(1:N+1), PIVOT(1:N+1), W(1:N+1),
  C    WP(1:N+1), Z0(1:N+1), Z1(1:N+1)  are all work arrays used by
  C    STEPNF  to calculate the tangent vectors and Newton steps.
  */

 // pointers to the vectors so they can be reallocated


static gVector<double> *Ytan_p;     // YP = tangent vector to zero curve at Y
static gVector<double> *Y_old_p;    // Yold = previous point on the zero curve
static gVector<double> *Ytan_old_p; // YPold = previous tangent vector


template <class T> 
void gHompack<T>::fixpnf(
	     int N,                     // N is the dimension
	     gVector<T> &Y,        // Y = [lambda, X]
	     int &flag,                 // IFLAG
	     T rel_arc_err,        // ARCRE = relative arc error
	     T abs_arc_err,        // ARCAE = absolute arc error
	     T rel_ans_err,        // ANSRE = relative answer error
	     T abs_ans_err,        // ANSAE = absolute answer error
	     bool trace,                // TRACE -> Print intermediate results?
	     gVector<T> &A,        // A is the parameter vector
	     gArray<T> &stepsize_params,    // SSPAR are step size parameters
	     int &jeval_num,            // NFE = number of Jacobian evaluations
	     T &arclength,         // ARCLEN is the arc length
	     T max_lambda,    // Ceiling on lambda
	     bool poly_switch  // POLY_SWITCH = ?
	     )
{
  MixedProfile<T> sol(supp);

  //cout << "Entered fixpnf()." << endl;
  //  void cleanup();
  
  // Static variables //////////////////////////////////////////////////
  // these were marked with SAVE in the Fortran
  static T abs_err;          // ABSERR
  static T current_tolerance;// CURTOL
  static T max_step;         // H was the maximum step size
  static T old_step;         // HOLD was the size of the previous step
  static T rel_err;          // RELERR
  static T total_arclength;  // S was the cumulative arc length
  
  static int iteration;           // ITER counted iterations in the main loop
  static int limit;               // LIMIT is the iteration limit
  
  static bool crash, start;

  // check the arguments
  if ( N <= 0                 // N must be positive
       || rel_ans_err <= 0    // Relative error must be positive
       || abs_ans_err < 0     // Absolute error must be nonnegative
       || Y.Length() != N+1   // dim Y must be N+1 (dim X + dim lambda)
       || ( ( flag == 0               // For the zero finding problem
	      || flag == -1 )         //   or the fixed point problems,
	    && N != A.Length()        //   dim A must be N
	    )
       ) {
    flag = 7;       // Return error 7: Illegal input parameters
    return;
  }
  
  if (flag == -2 || flag == -1 || flag == 0) {
    // initialization block ///////////////////////////////////////////
    
    // initialize the arclength to zero
    arclength = 0;
    
    // if either arc error is negative, reset it
    if (rel_arc_err <= 0)  rel_arc_err = 0.5 * sqrt(rel_ans_err);
    if (abs_arc_err <= 0)  abs_arc_err = 0.5 * sqrt(abs_ans_err);
    
    // Allocate arrays
    if (Ytan_p != NULL) delete Ytan_p;
    if (Y_old_p    != NULL)  delete Y_old_p;
    if (Ytan_old_p != NULL)  delete Ytan_old_p;
    
    Ytan_p     = new gVector<T>(N+1);
    Y_old_p    = new gVector<T>(N+1);
    Ytan_old_p = new gVector<T>(N+1);
    
    // "Set initial conditions for first call to stepnf()."
    start = true;
    crash = false;
    old_step = 0.1;      // Vale Murthy 1/28/00 used to be 1.
    // Tried 0.01 .. no noticeable effect
    max_step = 0.0001;  // Vale Murthy 1/31/00 used to be 0.01
    total_arclength = 0;
    
    // initialize Ytan and Ytan_old
    (*Ytan_p) = 0;
    (*Ytan_old_p) = 0;
    
    // set up lambda
    //Used to be Y[1] = 0.0  Vale Murthy 2/22/00
    (*Ytan_p)[1] = 1;
    (*Ytan_old_p)[1] = 1;
    
    // "Set optimal step size estimation parameters.
    //  Let Z[K] denote the Newton iterates along the flow normal to the
    //  Davidenko flow and Y their limit."
    
    // "Ideal contraction factor: ||Z[2] - Z[1]|| / ||Z[1] - Z[0]||"
    if (stepsize_params[1] <= 0)  stepsize_params[1] = 0.5;    
    
    // Ideal contraction factor used to be 0.5 2/25/00
    
    // "Ideal residual factor: ||rho(A, Z[1])|| / ||rho(A, Z[0]||"
    if (stepsize_params[2] <= 0)  stepsize_params[2] = 0.01;
    
    // "Ideal distance factor: ||Z[1] - Y|| / ||Z[0] - Y||"  was 0.5
    if (stepsize_params[3] <= 0)  stepsize_params[3] = 0.5;
    
    // "Minimum step size Hmin."
    if (stepsize_params[4] <= 0)
      // SSPAR(4)=(SQRT(N+1.0)+4.0)*EPSILON(1.0_R8)
      //            stepsize_params[4] = (sqrt(N+1.0)+4.0) * DBL_EPSILON;
      stepsize_params[4] = (sqrt(N+1.0)+4.0) * DBL_EPSILON;
    
    
    // "Maximum step size Hmax."
    
    if (stepsize_params[5] <= 0)  stepsize_params[5] = 0.001;
    
    // stepsize_params[5] = 0.0001  and limitd = 10000 reduces range of
    // lambda (as compared to stepsize_params[5] = 0.001; limitd = 10000)
    // in at least certain instances.
    
    /*
      Vale Murthy 2/22/00 stepsize_params[5]*limitd seems to control
      the range of lambda
      
      Used to be:
      (1) if (stepsize_params[5] <= 0)  stepsize_params[5] = 1;
      (2) if (stepsize_params[5] <= 0)  stepsize_params[5] = 0.01;
    */
    
    // "Minimum step size reduction factor bmin."
    if (stepsize_params[6] <= 0)  stepsize_params[6] = 0.1;
    
    // "Maximum step size reduction factor bmax."
    if (stepsize_params[7] <= 0)  stepsize_params[7] = 3;
    
    // "Assumed operation order P."
    if (stepsize_params[8] <= 0)  stepsize_params[8] = 2;
    
    // "Load A for the fixed point and zero finding problems."
    if (flag == -1 || flag == 0)
      for (int i = 1; i <= N; ++i)
	A[i] = Y[i+1];
    
    // initialize limit to the constant
    limit = limitd;
  }
  else if (flag == 3) {
    // We're continuing a call that reached the iteration limit
    // reset limit and go to the main loop
    limit = limitd;
  }
  else if (flag != 2) {
    // If we're not starting or continuing a call,
    // then flag had an illegal value
    flag = 7;       // Return error 7: Illegal input parameters
    return;
  }
  
  // main loop /////////////////////////////////////////////////////////
  for (iteration = 1; iteration <= limit; ++iteration) {
    // local variables
    gVector<T> &Ytan     = *Ytan_p;
    gVector<T> &Y_old    = *Y_old_p;
    gVector<T> &Ytan_old = *Ytan_old_p;
    
    // printf("iteration: %d",iteration); // Vale Murthy 1/25/00
    
    if (Y[1] < 0) {
      arclength = total_arclength;
      
      cleanup();
      flag = 5;   // Return error 5: Lost the zero curve
      return;
    }
    
    // "Set different error tolerance if the trajectory Y(S) has any high
    //  curvature components."
    current_tolerance = curvature_limit * old_step;
    rel_err = rel_arc_err;
    abs_err = abs_arc_err;
    
    // IF (ANY(ABS(YP-YOLD) .GT. CURTOL)) THEN
    bool areanytoobig = false;
    for (int i = 1; i <= N+1; ++i) {
      if ( fabs(Ytan[i] - Y_old[i]) > current_tolerance ) {
	areanytoobig = true;
	break;
      }
    }
    
    if (areanytoobig) {
      rel_err = rel_ans_err;
      abs_err = rel_ans_err;
    }
    
    // Debug stuff
    /*        cout << "Going into stepnf() with:" << endl;
	      cout << "old_step = " << old_step << endl;
	      cout << "step_size = " << max_step << endl;
	      cout << "rel_err = " << rel_err << endl;
	      cout << "abs_err = " << abs_err << endl;
	      cout << "Y =";
	      for (int i = 1; i <= Y.Length(); ++i)  cout << ' ' << Y[i];
	      cout << endl;
	      cout << "Ytan =";
	      for (int i = 1; i <= Ytan.Length(); ++i)  cout << ' ' << Ytan[i];
	      cout << endl;
	      cout << "Y_old =";
	      for (int i = 1; i <= Y_old.Length(); ++i)  cout << ' ' << Y_old[i];
	      cout << endl;
	      cout << "Ytan_old =";
	      for (int i = 1; i <= Ytan_old.Length(); ++i)  cout << ' ' << Ytan_old[i];
	      cout << endl;
	      cout << "A =";
	      for (int i = 1; i <= A.Length(); ++i)  cout << ' ' << A[i];
	      cout << endl;
	      cout << "ssp =";
	      for (int i = 1; i <= stepsize_params.Length(); ++i)  cout << ' ' << stepsize_params[i];
	      cout << endl;
	      cout << endl;
    */
    // "Take a step along the curve."
    
    //    show_probs("Before stepnf",Y,supp);
    
    
    stepnf(N,jeval_num, flag, start, crash, old_step, max_step, rel_err,
	   abs_err, total_arclength, Y, Ytan, Y_old, Ytan_old, A,
	   stepsize_params);
    
    //   show_probs("After stepnf",Y,supp);
    
    // 3/1/00 Vale Murthy
    
    
    //cout << "Left stepnf()." << endl;
    
    // "Print latest point on curve if requested."
    
    /*
      if (trace) {
      if (iteration == 1) {
      cout << endl << "step jeval_num arclength lambda";
      for (int i = 1; i <= N; ++i)
      cout << ' ' << setw(6) << "X" << i;
      cout << endl;
      }
      
      cout << setprecision(4);
      cout << setw(4) << iteration;
      cout << ' ' << setw(9) << jeval_num;
      cout.setf(ios::fixed);
      cout << ' ' << setw(9) << total_arclength;
      cout << ' ' << setw(6) << Y[1] << flush;
      for (int i = 2; i <= N+1; ++i)
      cout << ' ' << setw(7) << Y[i];
      cout.unsetf(ios::fixed);
      cout << endl;
      }
      
    */
    
    T tlambda = 1/(1-Y[1])-1; //Vale Murthy 2/9/00
    //          tlambda = log(1+tlambda);
    //          T tlambda = Y[1];
    
    int j = 2;
    for(int pl = 1;pl<=nfg.NumPlayers();pl++) {
      T resid = 1;
      int i;
      for(i=1;i<supp.NumStrats(pl);i++) {
	sol(pl,i) = Y[j];
	resid -= Y[j];
	j++;
      }
      sol(pl,i) =  resid;
    }

    if (params.fullGraph) {
      int index = solutions.Append(MixedSolution(sol, algorithmNfg_QRE));      
      solutions[index].SetQre(tlambda, QreValue(sol, tlambda));  // need to compute value here
      solutions[index].SetEpsilon(params.Accuracy());
    }

    if(params.pxifile) {
      (*params.pxifile).SetFloatMode();
      *params.pxifile << " " << tlambda;
      *params.pxifile << " " << QreValue(sol, tlambda);
      for(int pl = 1;pl<=nfg.NumPlayers();pl++) 
	for(int i=1;i<=supp.NumStrats(pl);i++) 
	  *params.pxifile << " " << sol(pl,i);
      *params.pxifile << "\n";

    }
    
    // "Check if the step was successful."
    if (flag > 0) {
      int index = solutions.Append(MixedSolution(sol, algorithmNfg_QRE));      
      solutions[index].SetQre(tlambda,QreValue(sol, tlambda));  // need to compute value here
      solutions[index].SetEpsilon(params.Accuracy());
      arclength = total_arclength;
      cleanup();
      return;
    }
    
    if (crash) {
      // "Change error tolerances."
      if (rel_arc_err < rel_err)  rel_arc_err = rel_err;
      if (rel_ans_err < rel_err)  rel_ans_err = rel_err;
      if (abs_arc_err < abs_err)  abs_arc_err = abs_err;
      if (abs_ans_err < abs_err)  abs_ans_err = abs_err;
      
      // "Change limit by number of iterations."
      limit -= iteration;
      
      flag = 2;   // Return error 2: Error tolerance too small.
      int index = solutions.Append(MixedSolution(sol, algorithmNfg_QRE));      
      solutions[index].SetQre(tlambda,QreValue(sol, tlambda));  // need to compute value here
      solutions[index].SetEpsilon(params.Accuracy());
      return;
    }
    
    
    if (Y[1] >= max_lambda) {
      printf("untransformed Y[1] > %f\n",max_lambda);
      // "Use hermite cubic interpolation and Newton iteration to get
      //  the answer at lambda = 1."
      
      // "Save Y_old for arc length calculation later."
      gVector<T> Z0 = Y_old;
      
      rootnf(N,jeval_num, flag, rel_ans_err, abs_ans_err,
	     Y, Ytan, Y_old, Ytan_old, A);
      
      // "Calculate final arc length."
      arclength = total_arclength - old_step + sqrt((Y - Z0).NormSquared());
      
      // Return an error if rootnf could not get the point on the zero
      // curve at lambda = 1.
      if (flag <= 0)
	flag = 1;
      cleanup();
      return;
      
    }
    
    // "For polynomial systems and the polsys1h homotopy map,
    //  d lambda/ds > 0 necessarily. This condition is forced here if
    //  the poly_switch variable is present."
    if (poly_switch) {
      if (Ytan[1] < 0) {
	// "Reverse tangent direction so d lambda/ds = Ytan[1] > 0."
	Ytan = -Ytan;
	Ytan_old = Ytan;
	
	// "Force stepnf to use the linear preditor for the next step only."
	start = true;
      }
    }
  } // END OF MAIN LOOP //
  
  // "lambda has not reached 1 in 1000 steps."
  arclength = total_arclength;
  printf("lambda has not reached 1!\n");
  flag = 3;           // Return error 3: StepNF has been called 1000 times
  int index = solutions.Append(MixedSolution(sol, algorithmNfg_QRE));      
  T tlambda = 1/(1-Y[1])-1; //Vale Murthy 2/9/00
  solutions[index].SetQre(tlambda,QreValue(sol, tlambda));  // need to compute value here
  solutions[index].SetEpsilon(params.Accuracy());
  return;
}

template <class T> 
void gHompack<T>::cleanup(void) {
  if (Ytan_p     != NULL)  delete Ytan_p;
  if (Y_old_p    != NULL)  delete Y_old_p;
  if (Ytan_old_p != NULL)  delete Ytan_old_p;
}

template <class T> 
void gHompack<T>::show_probs(char *msg,const gVector<T> &Y)
{
  MixedProfile<gNumber> sol(supp);

  int j=2;
  for(int pl = 1;pl<=nfg.NumPlayers();pl++) {
    T resid = 1;
    int i;
    for(i=1;i<supp.NumStrats(pl);i++) {
      sol(pl,i) = Y[j];
      resid -= Y[j];
      j++;
    }
    sol(pl,i) =  resid;
  }
  T tlambda = 1/(1-Y[1])-1; //Vale Murthy 2/9/00

  gout << msg << "\nlam: " << tlambda << " sol:" << sol;
  gout << "\n";
}
 
 
// stepnf ////////////////////////////////////////////////////////////////
/* STEPNF  TAKES ONE STEP ALONG THE ZERO CURVE OF THE HOMOTOPY MAP
   C USING A PREDICTOR-CORRECTOR ALGORITHM.  THE PREDICTOR USES A HERMITE
   C CUBIC INTERPOLANT, AND THE CORRECTOR RETURNS TO THE ZERO CURVE ALONG
   C THE FLOW NORMAL TO THE DAVIDENKO FLOW.  STEPNF  ALSO ESTIMATES A
   C STEP SIZE H FOR THE NEXT STEP ALONG THE ZERO CURVE.  NORMALLY
   C  STEPNF  IS USED INDIRECTLY THROUGH  FIXPNF , AND SHOULD BE CALLED
   C DIRECTLY ONLY IF IT IS NECESSARY TO MODIFY THE STEPPING ALGORITHM'S
   C PARAMETERS.
   C
   C THE FOLLOWING INTERFACE BLOCK SHOULD BE INCLUDED IN THE CALLING
   C PROGRAM:
   C
   C     INTERFACE
   C       SUBROUTINE STEPNF(N,NFE,IFLAG,START,CRASH,HOLD,H,RELERR,
   C    &    ABSERR,S,Y,YP,YOLD,YPOLD,A,QR,ALPHA,TZ,PIVOT,W,WP,
   C    &    Z0,Z1,SSPAR)
   C       USE REAL_PRECISION
   C       REAL (KIND=R8):: ABSERR,H,HOLD,RELERR,S
   C       INTEGER:: IFLAG,N,NFE
   C       LOGICAL:: CRASH,START
   C       REAL (KIND=R8):: A(:),ALPHA(3*N+3),QR(N,N+2),SSPAR(8),TZ(N+1),
   C    &    W(N+1),WP(N+1),Y(:),YOLD(N+1),YP(N+1),YPOLD(N+1),
   C    &    Z0(N+1),Z1(N+1)
   C       INTEGER:: PIVOT(N+1)
   C       END SUBROUTINE STEPNF
   C     END INTERFACE
   C
   C ON INPUT:
   C
   C N = DIMENSION OF X AND THE HOMOTOPY MAP.
   C
   C NFE = jeval_num = NUMBER OF JACOBIAN MATRIX EVALUATIONS.
   C
   C IFLAG = flag = -2, -1, OR 0, INDICATING THE PROBLEM TYPE.
   C
   C START = start = .TRUE. ON FIRST CALL TO  STEPNF , .FALSE. OTHERWISE.
   C
   C HOLD = old_step = ||Y - YOLD||; SHOULD NOT BE MODIFIED BY THE USER.
   C
   C H = step_size = UPPER LIMIT ON LENGTH OF STEP THAT WILL BE ATTEMPTED.
   C    H  MUST BE SET TO A POSITIVE NUMBER ON THE FIRST CALL TO  STEPNF .
   C    THEREAFTER  STEPNF  CALCULATES AN OPTIMAL VALUE FOR  H , AND  H
   C    SHOULD NOT BE MODIFIED BY THE USER.
   C
   C RELERR, ABSERR = rel_err, abs_err
   C    RELATIVE AND ABSOLUTE ERROR VALUES.  THE ITERATION IS
   C    CONSIDERED TO HAVE CONVERGED WHEN A POINT W=(LAMBDA,X) IS FOUND
   C    SUCH THAT
   C
   C    ||Z|| <= RELERR*||W|| + ABSERR  ,          WHERE
   C
   C    Z IS THE NEWTON STEP TO W=(LAMBDA,X).
   C
   C S = arclength
   C    (APPROXIMATE) ARC LENGTH ALONG THE HOMOTOPY ZERO CURVE UP TO
   C    Y(S) = (LAMBDA(S), X(S)).
   C
   C Y(1:N+1) = PREVIOUS POINT (LAMBDA(S), X(S)) FOUND ON THE ZERO CURVE OF
   C    THE HOMOTOPY MAP.
   C
   C YP(1:N+1) = Ytan
   C    UNIT TANGENT VECTOR TO THE ZERO CURVE OF THE HOMOTOPY MAP
   C    AT  Y .
   C
   C YOLD(1:N+1) = Y_old
   C    A POINT BEFORE  Y  ON THE ZERO CURVE OF THE HOMOTOPY MAP.
   C
   C YPOLD(1:N+1) = Ytan_old
   C    UNIT TANGENT VECTOR TO THE ZERO CURVE OF THE HOMOTOPY
   C    MAP AT  YOLD .
   C
   C A(:) = PARAMETER VECTOR IN THE HOMOTOPY MAP.
   C
   C QR(1:N,1:N+2), ALPHA(1:3*N+3), TZ(1:N+1), PIVOT(1:N+1), W(1:N+1),
   C    WP(1:N+1)  ARE WORK ARRAYS USED FOR THE QR FACTORIZATION (IN THE
   C    NEWTON STEP CALCULATION) AND THE INTERPOLATION.
   C
   C Z0(1:N+1), Z1(1:N+1)  ARE WORK ARRAYS USED FOR THE ESTIMATION OF THE
   C    NEXT STEP SIZE  H .
   C
   C SSPAR(1:8) = stepsize_params
   C    (LIDEAL, RIDEAL, DIDEAL, HMIN, HMAX, BMIN, BMAX, P)  IS
   C    A VECTOR OF PARAMETERS USED FOR THE OPTIMAL STEP SIZE ESTIMATION.
   C
   C
   C ON OUTPUT:
   C
   C N , A , SSPAR  ARE UNCHANGED.
   C
   C NFE  HAS BEEN UPDATED.
   C
   C IFLAG
   C    = -2, -1, OR 0 (UNCHANGED) ON A NORMAL RETURN.
   C
   C    = 4 IF A JACOBIAN MATRIX WITH RANK < N HAS OCCURRED.  THE
   C        ITERATION WAS NOT COMPLETED.
   C
   C    = 6 IF THE ITERATION FAILED TO CONVERGE.  W  CONTAINS THE LAST
   C        NEWTON ITERATE.
   C
   C START = .FALSE. ON A NORMAL RETURN.
   C
   C CRASH
   C    = .FALSE. ON A NORMAL RETURN.
   C
   C    = .TRUE. IF THE STEP SIZE  H  WAS TOO SMALL.  H  HAS BEEN
   C      INCREASED TO AN ACCEPTABLE VALUE, WITH WHICH  STEPNF  MAY BE
   C      CALLED AGAIN.
   C
   C    = .TRUE. IF  RELERR  AND/OR  ABSERR  WERE TOO SMALL.  THEY HAVE
   C      BEEN INCREASED TO ACCEPTABLE VALUES, WITH WHICH  STEPNF  MAY
   C      BE CALLED AGAIN.
   C
   C HOLD = ||Y - YOLD||.
   C
   C H = OPTIMAL VALUE FOR NEXT STEP TO BE ATTEMPTED.  NORMALLY  H  SHOULD
   C    NOT BE MODIFIED BY THE USER.
   C
   C RELERR, ABSERR  ARE UNCHANGED ON A NORMAL RETURN.
   C
   C S = (APPROXIMATE) ARC LENGTH ALONG THE ZERO CURVE OF THE HOMOTOPY MAP
   C    UP TO THE LATEST POINT FOUND, WHICH IS RETURNED IN  Y .
   C
   C Y, YP, YOLD, YPOLD  CONTAIN THE TWO MOST RECENT POINTS AND TANGENT
   C    VECTORS FOUND ON THE ZERO CURVE OF THE HOMOTOPY MAP.
   C
   C
   C CALLS  DNRM2 , TANGNF .
*/

template <class T> 
void gHompack<T>::stepnf( int N,
	     int &jeval_num,
	     int &flag,
	     bool &start,       bool &crash,
	     T &old_step,  T &step_size,
	     T &rel_err,   T &abs_err,
	     T &arclength,
	     gVector<T> &Y,        gVector<T> &Ytan,
	     gVector<T> &Y_old,    gVector<T> &Ytan_old,
	     const gVector<T> &A,
	     const gArray<T> &stepsize_params
	     )
{
  // top-level variables
  static T step_size_fail;
  gVector<T> W(N+1), Wtan(N+1), Z0(N+1), Z1(N+1);
  T lcalc = 0.0, rcalc = 0.0;
  int itnum;
  bool fail;
  
  // printf("Inside stepnf \n");
  
  // "The arclength must be nonnegative."
  if (arclength < 0) {
    crash = true;
    return;
  }
  
  // "If step size is too small, determine an acceptable one."
  T d = 4 * DBL_EPSILON * (arclength + 1);
  if (step_size < d) {
    step_size = d;
    crash = true;
    return;
  }
  
  // "If error tolerances are too small, increase them to acceptable values."
  T temp = sqrt(Y.NormSquared());
  if ( (rel_err*temp + abs_err) < 4 * DBL_EPSILON * temp ) {
    if (rel_err != 0) {
      rel_err = 4 * DBL_EPSILON * (4 * DBL_EPSILON + 1);
      abs_err = gmax(abs_err, 0.0);
    } else {
      abs_err = 4 * DBL_EPSILON * temp;
    }
    crash = true;
    return;
  }
  
  crash = false;
  
  if (start) {
    gVector<T> Newton_step(N+1);
    
    // "Startup Section (First step along zero curve)."
    fail = start = false;
    
    // "Determine suitable initial step size."
    step_size = gmin( gmin(step_size, .1),
		      sqrt(sqrt(rel_err * temp + abs_err))
		      );
    
    // "Use linear predictor along tangent direction to start Newton iteration."
    Ytan_old = 0;
    Ytan_old[1] = 1;
    
    // display lambda,player 1's p's, player 2's q's
    // Vale Murthy 3/1/00 The printing will scroll off the screen
    //    show_probs("Before tangnf",Y,supp);
    
    tangnf(arclength,Y, Ytan, Ytan_old, A, Newton_step, jeval_num, N, flag);
    
    //    show_probs("After tangnf",Y,supp);
    
    if (flag > 0) return;
    
    while (true) {
      Z0 = W = Y + Ytan * step_size;
      
      for (int iteration = 1; iteration <= Newton_iter_limit; ++iteration) {
	T rholen = -1;
	
	// "Calculate the Newton step at the current point W."
	
	
	// display lambda,player 1's p's, player 2's q's
	//show_probs("Before tangnf(2):",W,supp);
	
	tangnf(rholen,W, Wtan, Ytan_old, A, Newton_step, jeval_num, N, flag);
	//show_probs("After tangnf(2):",W,supp);
	if (flag > 0)
	  return;     // Return tangnf error.
	
	// "Take Newton step and check convergence."
	
	W += Newton_step;
	itnum = iteration;
	
	// "Compute quantities used for optimal step size estimation."
	if (iteration == 1) {
	  lcalc = sqrt(Newton_step.NormSquared());
	  rcalc = rholen;
	  Z1 = W;
	}
	else if (iteration == 2) {
	  lcalc = sqrt(Newton_step.NormSquared()) / lcalc;
	  rcalc = rholen / rcalc;
	}
	
	// "Go to mop-up section after convergence."
	if (sqrt(Newton_step.NormSquared()) <= rel_err * sqrt(W.NormSquared()) + abs_err)
	  goto mop_up;
      }
      
      // "No convergence in Newton_iter_limit iterations.
      //  Reduce step_size and try again."
      if (step_size <= 4 * DBL_EPSILON * (arclength+1)) {
	flag = 6;       // Return error 6: Iteration failed to converge.
	return;
      }
      
      step_size *= .5;   //0.5
    }
  }
  else { // if (!start) 
    gVector<T> Newton_step(N+1);
    
    fail = false;
    
    while (true) {
      // Predictor Section
      
      // "Compute point predicted by Hermite interpolant. Use step size
      //  computed on last call to stepnf."
      int imax = 50;
      int count = 0;
      //
      T init_step_size = step_size;
      
      build_prediction(W,N,Y_old,Ytan_old,Y,Ytan,old_step,old_step+step_size);
      
      while ( out_of_bounds(W,N+1) && (count < imax) ) {
	count ++;
	step_size /= 3.0;
	// Vale Murthy this seems to have an effect on the range of
	// lambda  3/8/00 step_size /= 4.0 to step_size /= 2.0;
	build_prediction(W,N,Y_old,Ytan_old,Y,Ytan,old_step,old_step+step_size);
      }
      
      if (count == imax) printf("Predictor extrapolation not in bounds\n");
      
      step_size = init_step_size; //reset step_size .. should this be done ?
      
      Z0 = W; // End of predictor section
      
      // display lambda,player 1's p's, player 2's q's
      // Vale Murthy 3/1/00 The printing will scroll off the screen
      // show_probs("End of Predictor section",W,supp);
      
      // Corrector section
      for (int iteration = 1; iteration <= Newton_iter_limit; ++iteration) {
	T rholen = -1;
	
	// display lambda,player 1's p's, player 2's q's
	//show_probs("Before tangnf(3):",W,supp);
	
	// "Calculate the Newton step at the curent point W."
	tangnf(rholen, W, Wtan, Ytan, A,
	       Newton_step, jeval_num, N, flag);
	// show_probs("After tangnf(3):",W,supp);
	if (flag > 0)  return;     // Return tangnf error.
	// "Take Newton step and check convergence."
	//W += Newton_step;
	//correct for probabilities (<0) or (>1)
	
	// Correcting for Newton_step out of bounds - Vale Murthy 3/3/00
	
	int imax = 50;
	int count = 0;
	gVector<T> result_step(N+1);
	result_step = Newton_step+W;
	
	while ( out_of_bounds(result_step,N+1) && (count < imax) ) {
	  count++;
	  Newton_step = Newton_step/3.0;  //tried 2.0 -- crashed 3/9/00
	  result_step = Newton_step+W;
	}
	
	if (count == imax) printf("Correction of Newton_step failed!\n");
	
	W = result_step;
	
	/*
	  int i;
	  for(i = 1; i<= N+1;i++)
	  {
	  if ( Newton_step[i]+W[i] <= 0) Newton_step[i] = -W[i]/2;
	  else
	  if (Newton_step[i]+W[i] >= 1) Newton_step[i] = 0.5 - W[i]/2.0;
	  W[i] += Newton_step[i];
	  }
	*/
	
	
	// I am not sure this bound checking works ? Vale Murthy 2/16/00
	
	itnum = iteration;
	
	// "Compute quantities used for optimal step size estimation."
	if (iteration == 1) {
	  lcalc = sqrt(Newton_step.NormSquared());
	  rcalc = rholen;
	  Z1 = W;
	}
	else if (iteration == 2) {
	  lcalc = sqrt(Newton_step.NormSquared()) / lcalc;
	  rcalc = rholen / rcalc;
	}
	
	// "Go to mop-up section after convergence."
	if (sqrt(Newton_step.NormSquared()) <= rel_err*sqrt(W.NormSquared()) + abs_err) {
	  //                    show_probs("After convergence",Y,supp);
	  goto mop_up;
	}
      }
      
      // "No convergence in Newton_iter_limit iterations.
      //  Record failure at calculated step_size,
      //  save this step size, reduce step_size and try again."
      fail = true;
      step_size_fail = step_size;
      if (step_size <= 4 * DBL_EPSILON * (arclength+1)) {
	flag = 6;       // Return error 6: Iteration failed to converge.
	return;
      }
      step_size *= .5;
    }
    // End of corrector section
  }
  
 mop_up: // Mop-up section
  
  // "Y_old and Y always contain the last two points found on the zero
  //  curve of the homotopy map. Ytan_old and Ytan contain the tangent
  //  vectors to the zero curve at Y_old and Y, respectively."
  Ytan_old = Ytan;
  Y_old = Y;
  
  // Vale Murthy 3/1/00
  // show_probs("Before Y=W",Y,supp);
  Y = W;
  // show_probs("After Y=W",Y,supp);
  Ytan = Wtan;
  
  
  // "Update arc length."
  old_step = sqrt( (Y - Y_old).NormSquared() );
  arclength += old_step;
  
  // End of mop-up section
  
  // Optimal step size estimation section
  
  // "Calculate the distance factor dcalc."
  T dcalc = sqrt( (Z0 - Y).NormSquared() );
  if (dcalc != 0)
    dcalc = sqrt( (Z1 - Y).NormSquared() )/dcalc;
  
  /* "The optimal step size Hbar is defined by:
   *    HT = Hold * [MIN(lideal/lcalc, rideal/rcalc, dideal/dcalc)]**(1/P)
   *      Hbar = MIN[ MAX(HT, Bmin*Hold, Hmin), Bmax*hold, Hmax ]
   */
  
  // "If convergence had occurred after 1 iteration, set the contraction
  //  factor lcalc to zero.
  if (itnum == 1)
    lcalc = 0;
  
  // "Formula for optimal step size."
  T optimal_step_size;
  if (lcalc + rcalc + dcalc == 0) {
    optimal_step_size = stepsize_params[7] * old_step;
  } 
  else {
    T l = lcalc/stepsize_params[1];
    T r = rcalc/stepsize_params[2];
    T d = dcalc/stepsize_params[3];
    T max = gmax( gmax(l, r), d );
    optimal_step_size = old_step * pow(1 / max, 1 / stepsize_params[8]);
  }
  
  // "optimal_step_size contains the estimated optimal step size.
  //  Now put it within reasonable bounds."
  {
    T max = gmax( gmax(optimal_step_size, stepsize_params[6]*old_step),
		       stepsize_params[4] );
    step_size = gmin( gmin(max, stepsize_params[7]*old_step), stepsize_params[5] );
  }
  
  if (itnum == 1) {
    // "If convergence had occurred after 1 iteration, don't decrease step_size."
    step_size = gmax(step_size, old_step);
  }
  else if (itnum == Newton_iter_limit) {
    // "If convergence required the maximum litgh iterations, don't increase H."
    step_size = gmin(step_size, old_step);
  }
  
  // "If convergence did not occur in the  litgh iterations for a particular
  //  H = Hfail, don't choose the new step size larger than Hfail."
  if (fail)
    step_size = gmin(step_size, step_size_fail);
  
  return;
}


// rootnf ////////////////////////////////////////////////////////////////
/* ROOTNF  FINDS THE POINT  YBAR = (1, XBAR)  ON THE ZERO CURVE OF THE
   C HOMOTOPY MAP.  IT STARTS WITH TWO POINTS YOLD=(LAMBDAOLD,XOLD) AND
   C Y=(LAMBDA,X) SUCH THAT  LAMBDAOLD < 1 <= LAMBDA , AND ALTERNATES
   C BETWEEN HERMITE CUBIC INTERPOLATION AND NEWTON ITERATION UNTIL
   C CONVERGENCE.
   C
   C ON INPUT:
   C
   C N = DIMENSION OF X AND THE HOMOTOPY MAP.
   C
   C NFE = NUMBER OF JACOBIAN MATRIX EVALUATIONS.
   C
   C IFLAG = -2, -1, OR 0, INDICATING THE PROBLEM TYPE.
   C
   C RELERR, ABSERR = RELATIVE AND ABSOLUTE ERROR VALUES.  THE ITERATION IS
   C    CONSIDERED TO HAVE CONVERGED WHEN A POINT Y=(LAMBDA,X) IS FOUND
   C    SUCH THAT
   C
   C    |Y(1) - 1| <= RELERR + ABSERR              AND
   C
   C    ||Z|| <= RELERR*||X|| + ABSERR  ,          WHERE
   C
   C    (?,Z) IS THE NEWTON STEP TO Y=(LAMBDA,X).
   C
   C Y(1:N+1) = POINT (LAMBDA(S), X(S)) ON ZERO CURVE OF HOMOTOPY MAP.
   C
   C YP(1:N+1) = UNIT TANGENT VECTOR TO THE ZERO CURVE OF THE HOMOTOPY MAP
   C    AT  Y .
   C
   C YOLD(1:N+1) = A POINT DIFFERENT FROM  Y  ON THE ZERO CURVE.
   C
   C YPOLD(1:N+1) = UNIT TANGENT VECTOR TO THE ZERO CURVE OF THE HOMOTOPY
   C    MAP AT  YOLD .
   C
   C A(1:*) = PARAMETER VECTOR IN THE HOMOTOPY MAP.
   C
   C QR(1:N,1:N+2), ALPHA(1:N), TZ(1:N+1), PIVOT(1:N+1), W(1:N+1),
   C    WP(1:N+1)  ARE WORK ARRAYS USED FOR THE QR FACTORIZATION (IN THE
   C    NEWTON STEP CALCULATION) AND THE INTERPOLATION.
   C
   C PAR(1:*) AND IPAR(1:*) ARE ARRAYS FOR (OPTIONAL) USER PARAMETERS,
   C    WHICH ARE SIMPLY PASSED THROUGH TO THE USER WRITTEN SUBROUTINES
   C    RHO, RHOJAC.
   C
   C ON OUTPUT:
   C
   C N , RELERR , ABSERR , A  ARE UNCHANGED.
   C
   C NFE  HAS BEEN UPDATED.
   C
   C IFLAG
   C    = -2, -1, OR 0 (UNCHANGED) ON A NORMAL RETURN.
   C
   C    = 4 IF A JACOBIAN MATRIX WITH RANK < N HAS OCCURRED.  THE
   C        ITERATION WAS NOT COMPLETED.
   C
   C    = 6 IF THE ITERATION FAILED TO CONVERGE.  Y  AND  YOLD  CONTAIN
   C        THE LAST TWO POINTS FOUND ON THE ZERO CURVE.
   C
   C Y  IS THE POINT ON THE ZERO CURVE OF THE HOMOTOPY MAP AT  LAMBDA = 1 .
   C
   C
   C CALLS  D1MACH , DNRM2 , ROOT , TANGNF .
*/

template <class T> 
void gHompack<T>::rootnf( int N,
	     int &jevalcount, int &flag,
	     T relerr,     T abserr,
	     gVector<T> &Y, //debugging 2/16/00 Vale Murthy
	     gVector<T> &Ytan,
	     gVector<T> &Y_old,
	     gVector<T> &Ytan_old,
	     const gVector<T> &A
	     )
{
  //cout << "Entered rootnf()." << endl;
  T rerr = gmax(relerr, DBL_EPSILON);
  T aerr = gmax(abserr, 0.);
  
  /* *****  MAIN LOOP.  ***** */
  for (int iteration = 1; iteration <= 20; ++iteration) {
    gVector<T> Newton_step = Y - Y_old;
    T dels = sqrt(Newton_step.NormSquared());
    
    // "USING TWO POINTS AND TANGENTS ON THE HOMOTOPY ZERO CURVE, CONSTRUCT
    //  THE HERMITE CUBIC INTERPOLANT Q(S).  THEN USE  ROOT  TO FIND THE S
    //  CORRESPONDING TO  LAMBDA = 1 .  THE TWO POINTS ON THE ZERO CURVE ARE
    //  ALWAYS CHOSEN TO BRACKET LAMBDA=1, WITH THE BRACKETING INTERVAL
    //  ALWAYS BEING [0, DELS]."
    T sa = 0;
    T sb = dels;
    int lcode = 1;
    
    T sout, qsout;
    root(sout, qsout, sa, sb, rerr, aerr, lcode);
    //cout << "rootnf: Left root(), #1." << endl;
    
    while (lcode <= 0) {
      qsout = qofs( Y_old[1], Ytan_old[1], Y[1], Ytan[1], dels, sout) - 1;
      root(sout, qsout, sa, sb, rerr, aerr, lcode);
      //cout << "rootnf: Left root(), #2." << endl;
    }
    
    if (lcode > 2) {
      flag = 6;
      return;
    }
    
    // "CALCULATE Q(SA) AS THE INITIAL POINT FOR A NEWTON ITERATION."
    gVector<T> W(N+1), WP(N+1);
    for (int i = 1; i <= N+1; ++i)
      W[i] = qofs( Y_old[i], Ytan_old[i], Y[i], Ytan[i], dels, sa);
    
    // display lambda,player 1's p's, player 2's q's
    // show_probs("Before tangnf(4):",W,supp);
    
    
    // "CALCULATE NEWTON STEP AT Q(SA)."
    tangnf(sa, W, WP, Ytan_old, A, Newton_step, jevalcount, N, flag);
    //cout << "rootnf: Left tangnf(), #1." << endl;
    // show_probs("After tangnf(4):",W,supp);
    
    if (flag > 0)
      return;
    
    // "NEXT POINT = CURRENT POINT + NEWTON STEP."
    W += Newton_step;
    
    // display lambda,player 1's p's, player 2's q's
    // show_probs("Before tangnf(5):",W,supp);
    
    // "GET THE TANGENT  WP  AT  W  AND THE NEXT NEWTON STEP IN Newton_step.
    tangnf(sa, W, WP, Ytan_old, A, Newton_step, jevalcount, N, flag);
    // show_probs("After tangnf(5):",W,supp);
    //cout << "rootnf: Left tangnf(), #2." << endl;
    
    if (flag > 0)
      return;
    
    // "TAKE NEWTON STEP AND CHECK CONVERGENCE.
    W += Newton_step;
    
    // Since the Fortran computes the norm starting at Newton_step[2] and
    // W[2], we need to correct for the first number.
    T ns2 = Newton_step.NormSquared() - Newton_step[1] * Newton_step[1];
    T w2  = W.NormSquared() - W[1] * W[1];
    if ( abs(W[1]-1) <= rerr + aerr
	 && sqrt(ns2) <= rerr * sqrt(w2) + aerr) {
      Y = W;
      return;
    }
    
    // "IF THE ITERATION HAS NOT CONVERGED, DISCARD ONE OF THE OLD POINTS
    //  SUCH THAT  LAMBDA = 1  IS STILL BRACKETED.
    if ( (Y_old[1] > 1 && W[1] > 1) ||
	 (Y_old[1] < 1 && W[1] < 1)    ) {
      Y_old = W;
      Ytan_old = WP;
    } 
    else {
      Y = W;
      Ytan = WP;
    }
  } // ***** END OF MAIN LOOP *****
  
  // "THE ALTERNATING OSCULATORY CUBIC INTERPOLATION AND NEWTON ITERATION
  //  HAS NOT CONVERGED IN  LIMIT  STEPS.  ERROR RETURN.
  flag = 6;
}


// root //////////////////////////////////////////////////////////////////
/*  ROOT COMPUTES A ROOT OF THE NONLINEAR EQUATION F(X)=0
    C  WHERE F(X) IS A CONTINOUS REAL FUNCTION OF A SINGLE REAL
    C  VARIABLE X.  THE METHOD USED IS A COMBINATION OF BISECTION
    C  AND THE SECANT RULE.
    C
    C  NORMAL INPUT CONSISTS OF A CONTINUOS FUNCTION F AND AN
    C  INTERVAL (B,C) SUCH THAT F(B)*F(C).LE.0.0.  EACH ITERATION
    C  FINDS NEW VALUES OF B AND C SUCH THAT THE INTERVAL(B,C) IS
    C  SHRUNK AND F(B)*F(C).LE.0.0.  THE STOPPING CRITERION IS
    C
    C          DABS(B-C).LE.2.0*(RELERR*DABS(B)+ABSERR)
    C
    C  WHERE RELERR=RELATIVE ERROR AND ABSERR=ABSOLUTE ERROR ARE
    C  INPUT QUANTITIES.  SET THE FLAG, IFLAG, POSITIVE TO INITIALIZE
    C  THE COMPUTATION.  AS B,C AND IFLAG ARE USED FOR BOTH INPUT AND
    C  OUTPUT, THEY MUST BE VARIABLES IN THE CALLING PROGRAM.
    C
    C  IF 0 IS A POSSIBLE ROOT, ONE SHOULD NOT CHOOSE ABSERR=0.0.
    C
    C  THE OUTPUT VALUE OF B IS THE BETTER APPROXIMATION TO A ROOT
    C  AS B AND C ARE ALWAYS REDEFINED SO THAT DABS(F(B)).LE.DABS(F(C)).
    C
    C  TO SOLVE THE EQUATION, ROOT MUST EVALUATE F(X) REPEATEDLY. THIS
    C  IS DONE IN THE CALLING PROGRAM.  WHEN AN EVALUATION OF F IS
    C  NEEDED AT T, ROOT RETURNS WITH IFLAG NEGATIVE.  EVALUATE FT=F(T)
    C  AND CALL ROOT AGAIN.  DO NOT ALTER IFLAG.
    C
    C  WHEN THE COMPUTATION IS COMPLETE, ROOT RETURNS TO THE CALLING
    C  PROGRAM WITH IFLAG POSITIVE=
    C
    C     IFLAG=1  IF F(B)*F(C).LT.0 AND THE STOPPING CRITERION IS MET.
    C
    C          =2  IF A VALUE B IS FOUND SUCH THAT THE COMPUTED VALUE
    C              F(B) IS EXACTLY ZERO.  THE INTERVAL (B,C) MAY NOT
    C              SATISFY THE STOPPING CRITERION.
    C
    C          =3  IF DABS(F(B)) EXCEEDS THE INPUT VALUES DABS(F(B)),
    C              DABS(F(C)).  IN THIS CASE IT IS LIKELY THAT B IS CLOSE
    C              TO A POLE OF F.
    C
    C          =4  IF NO ODD ORDER ROOT WAS FOUND IN THE INTERVAL.  A
    C              LOCAL MINIMUM MAY HAVE BEEN OBTAINED.
    C
    C          =5  IF TOO MANY FUNCTION EVALUATIONS WERE MADE.
    C              (AS PROGRAMMED, 500 ARE ALLOWED.)
    C
    C  THIS CODE IS A MODIFICATION OF THE CODE ZEROIN WHICH IS COMPLETELY
    C  EXPLAINED AND DOCUMENTED IN THE TEXT  NUMERICAL COMPUTING:  AN
    C  INTRODUCTION,  BY L. F. SHAMPINE AND R. C. ALLEN.
    C
    C  CALLS  D1MACH .
*/

template <class T> 
void gHompack<T>::root( T &t, T &F_of_t,
	   T &b, T &c,
	   T relerr, T abserr,
	   int &flag
	   )
{
  //cout << "Entered root()." << endl;
  
  /* Local variables */
  static int count, ic;
  static T a, acbs, ae, fa, fb, fc, fx, re;
  
  if (flag >= 0) {
    re = gmax(relerr, DBL_EPSILON);
    ae = gmax(abserr, (T)0  );
    ic = 0;
    acbs = fabs(b - c);
    t = a = c;
    flag = -1;
    return;
  }
  
  switch (flag) {
  case -1:
    fa = F_of_t;
    t = b;
    flag = -2;
    return;
    
  case -2:
    fb = F_of_t;
    fc = fa;
    count = 2;
    
    // "Computing MAX"
    fx = gmax( fabs(fb), fabs(fc) );
    
    break;
    
  case -3:
    fb = F_of_t;
    if (fb == 0)    { flag = 2;  return; }
    
    ++count;
    
    if ( (fb > 0 ? 1 : -1) == (fc > 0 ? 1 : -1) )
      { c = a;  fc = fa;}
    
    break;
  }
  
  if (fabs(fc) < fabs(fb))
    {
      // "INTERCHANGE B AND C SO THAT ABS(F(B)).LE.ABS(F(C))."
      a = b;  fa = fb;
      b = c;  fb = fc;
      c = a;  fc = fa;
    }
  
  T bc_midpoint = (c - b) * .5;
  T tolerance = re * fabs(b) + ae;
  
  // "TEST STOPPING CRITERION AND FUNCTION COUNT."
  if (fabs(bc_midpoint) <= tolerance) {
    // "FINISHED.  SET IFLAG."
    if ( (fb > 0 && fc > 0) || (fb < 0 && fc < 0) ) { 
      flag = 4;
      printf("Finished flag = %d\n",flag); // Vale Murthy 2/23/00
      return;
    }
    
    if (fabs(fb) > fx) {printf("fabs(fb) > fx\n"); flag = 3;  return; }
    
    flag = 1;
    return;
  }
  
  if (count >= 500)   { flag = 5;  return; }
  
  // "CALCULATE NEW ITERATE EXPLICITLY AS B+P/Q
  //  WHERE WE ARRANGE P.GE.0.  THE IMPLICIT
  //  FORM IS USED TO PREVENT OVERFLOW."
  T p = (b - a) * fb;
  T q = fa - fb;
  
  if (p < 0)  { p = -p;  q = -q; }
  
  // "UPDATE A, CHECK IF REDUCTION IN THE SIZE OF BRACKETING
  //  INTERVAL IS SATISFACTORY.  IF NOT BISECT UNTIL IT IS."
  a = b;  fa = fb;
  ++ic;
  
  if (ic >= 4) {
    if (fabs(bc_midpoint) * 8 >= acbs) {
      // "USE BISECTION."
      b = (c + b) * .5;
      
      // "HAVE COMPLETED COMPUTATION FOR NEW ITERATE B."
      t = b;
      flag = -3;
      return;
    }
    
    ic = 0;
    acbs = fabs(bc_midpoint);
  }
  
  // "TEST FOR TOO SMALL A CHANGE."
  if (p <= fabs(q) * tolerance) {
    // "INCREMENT BY TOLERANCE"
    b += fabs(tolerance) * ( bc_midpoint > 0 ? 1 : -1 );
    t = b;
    flag = -3;
    return;
  }
  
  // "ROOT OUGHT TO BE BETWEEN B AND (C+B)/2"
  if (p < bc_midpoint * q) {
    // "USE SECANT RULE."
    b += p / q;
    t = b;
    flag = -3;
    return;
  }
  
  // "USE BISECTION."
  b = (c + b) * .5;
  
  // "HAVE COMPLETED COMPUTATION FOR NEW ITERATE B."
  t = b;
  flag = -3;
  return;
}


// tangnf ////////////////////////////////////////////////////////////////
/* THIS SUBROUTINE BUILDS THE JACOBIAN MATRIX OF THE HOMOTOPY MAP,
   C COMPUTES A QR DECOMPOSITION OF THAT MATRIX, AND THEN CALCULATES THE
   C (UNIT) TANGENT VECTOR AND THE NEWTON STEP.
   C
   C THE FOLLOWING INTERFACE BLOCK SHOULD BE INCLUDED IN THE CALLING
   C PROGRAM:
   C
   C     INTERFACE
   C       SUBROUTINE TANGNF(RHOLEN,Y,YP,YPOLD,A,QR,ALPHA,TZ,PIVOT,
   C    &    NFE,N,IFLAG)
   C       USE REAL_PRECISION
   C       REAL (KIND=R8):: RHOLEN
   C       INTEGER:: IFLAG,N,NFE
   C       REAL (KIND=R8):: A(:),Y(:),YP(N+1),YPOLD(N+1)
   C       REAL (KIND=R8):: ALPHA(3*N+3),QR(N,N+2),TZ(N+1)
   C       INTEGER:: PIVOT(N+1)
   C       END SUBROUTINE TANGNF
   C     END INTERFACE
   C
   C
   C ON INPUT:
   C
   C RHOLEN < 0 IF THE NORM OF THE HOMOTOPY MAP EVALUATED AT
   C    (A, LAMBDA, X) IS TO BE COMPUTED.  IF  RHOLEN >= 0  THE NORM IS NOT
   C    COMPUTED AND  RHOLEN  IS NOT CHANGED.
   C
   C Y(1:N+1) = CURRENT POINT (LAMBDA(S), X(S)).
   C
   C YPOLD(1:N+1) = UNIT TANGENT VECTOR AT PREVIOUS POINT ON THE ZERO
   C    CURVE OF THE HOMOTOPY MAP.
   C
   C A(:) = PARAMETER VECTOR IN THE HOMOTOPY MAP.
   C
   C QR(1:N,1:N+2), ALPHA(1:3*N+3), Newton_step(1:N+1), PIVOT(1:N+1)  ARE WORK
   C    ARRAYS USED FOR THE QR FACTORIZATION.
   C
   C NFE = jevalcount = NUMBER OF JACOBIAN MATRIX EVALUATIONS =
   C    NUMBER OF HOMOTOPY FUNCTION EVALUATIONS.
   C
   C N = DIMENSION OF X.
   C
   C IFLAG = flag = -2, -1, OR 0, INDICATING THE PROBLEM TYPE.
   C
   C
   C ON OUTPUT:
   C
   C RHOLEN = ||RHO(A, LAMBDA(S), X(S)|| IF  RHOLEN < 0  ON INPUT.
   C    OTHERWISE  RHOLEN  IS UNCHANGED.
   C
   C Y, YPOLD, A, N  ARE UNCHANGED.
   C
   C YP(1:N+1) = DY/DS = UNIT TANGENT VECTOR TO INTEGRAL CURVE OF
   C    D(HOMOTOPY MAP)/DS = 0  AT  Y(S) = (LAMBDA(S), X(S)) .
   C
   C TZ = Newton_step = -(PSEUDO INVERSE OF  (D RHO(A,Y(S))/D LAMBDA ,
   C    D RHO(A,Y(S))/DX)) * RHO(A,Y(S)) .
   C
   C NFE = jevalcount  HAS BEEN INCRMENTED BY 1.
   C
   C IFLAG  IS UNCHANGED, UNLESS THE QR FACTORIZATION DETECTS A RANK < N,
   C    IN WHICH CASE THE TANGENT AND NEWTON STEP VECTORS ARE NOT COMPUTED
   C    AND  TANGNF  RETURNS WITH  IFLAG = 4 .
   C
   C
   C CALLS  DGEQPF , DNRM2 , DORMQR , F (OR  RHO ), FJAC (OR  RHOJAC ).
*/

template <class T> 
void gHompack<T>::tangnf( T &rholen,
	     const gVector<T> &Y,
	     gVector<T> &Ytan,
	     const gVector<T> &Ytan_old,
	     const gVector<T> &A,
	     gVector<T> &Newton_step,
	     int &jevalcount, int N, int &flag) 
{
  // cout << "Entered tangnf()." << endl;
  
  const T &lambda = Y[1];
  //    const T lambda = Y[1];
  gMatrix<T> QR(N, N+1);         // Split the N+2 column of QR
  gVector<T> QR_Np2(N);          // and put it in its own vector.
  gVector<T> alpha(N+1);
  gVector<int> pivot(N+1);
  
  // "jevalcount contains the number of Jacobian evaluations."
  ++jevalcount;
  
  // "Compute the Jacobian matrix, store it and homotopy map in QR."
  if (flag == -2) {
    // "QR = ( d rho(A,lambda,X)/d lambda, d rho(A,lambda,X)/dx,
    //                                              rho(a,lambda,X) ).
    
    // display lambda,player 1's p's, player 2's q's
    //    show_probs("Before rho:",Y,supp);
    
    
    // Y[1] = lambda
    // Y[2] to Y[n_rows] = p[1] ... p[n_rows-1]
    // Y[n_rows+1] to Y[n_rows+n_cols-1] = q[1] to q[n_cols-1]
    
    gVector<T> X(N);
    for (int i = 1; i <= N; ++i) {
      X[i] = Y[i+1];
    }
    
    gVector<T> V(N);
    
    for (int i = 1; i <= N+1; ++i) {
      rhojac(A, lambda, X, V, i);
      QR.SetColumn(i, V);
    }
    
    rho(A, lambda, X, QR_Np2);
    
  }
  else { // if (flag != -2) 
    gVector<T> X(N);
    for (int i = 1; i <= N; ++i)
      X[i] = Y[i+1];
    
    gVector<T> V(N);
    
    F(X, V);
    
    if (flag == 0) {
      // "QR = ( A - F(X), I - lambda * DF(X), X - A + lambda*(A - F(X)) )."
      gVector<T> temp = A - V;
      QR.SetColumn(1, temp);
      
      QR_Np2 = X - A + temp * lambda;
      
      for (int k = 1; k <= N; ++k) {
	Fjac(X, V, k);
	QR.SetColumn(k+1, V * (-lambda));
	QR(k, k+1) += 1;
      }
    }
    else { // if (flag != 0) 
      // "QR = ( F(X) - X + A, lambda * DF(X) + (1 - lambda)*I, X - A + lambda*(F(X) - X + A) )."
      gVector<T> XmA = X - A;
      QR.SetColumn(1, V - XmA);
      
      QR_Np2 = XmA + (V - XmA) * lambda;
      
      for (int i = 1; i <= N; ++i) {
	Fjac(X, V, i);
	QR.SetColumn(i+1, V * lambda);
	QR(i, i+1) += 1 - lambda;
      }
    }
  }
  
  // "Compute the norm of the homotopy map if it was requested."
  if (rholen < 0)
    rholen = sqrt( QR_Np2.NormSquared() );
  
  // "Reduce the Jacobian matrix to upper triangular form."
  pivot = 0;
  int info;
  gVector<T> tau(N);
  
  // CALL DGEQPF(N,NP1,QR,N,PIVOT,YP,ALPHA,K)
  gLapack<T> la;
  la.dgeqpf(QR, pivot, tau, info);
  
  if ( fabs(QR(N,N)) <= fabs(QR(1,1)) * DBL_MIN ) {
    flag = 4;       // Return error 4: Jacobian has rank < N
    printf("Jacobian has rank < N \n"); // Vale Murthy 2/23/00
    
    // Vale Murthy 2/24/00  Print out QR
    // QR(N,N+1)
    int i_t,j_t;
    for(i_t=1;i_t<=N;i_t++) {
      for(j_t=1;j_t<=N+1;j_t++) {
	printf("%e ",QR(i_t,j_t));
      }
      printf("\n");
    }
    printf("\n");
    
    return;
  }
  
  {
    //"call dormqr('L', 'T', N, 1, N, QR, N, YP, QR(:, NP2), N, alpha, 3*N+3, K)"
    // Need to pass QR(1:N,1:N) to dormqr as A.
    gMatrix<T> A(N, N);
    for (int j = 1; j <= N; ++j) {
      // It seems that gMatrix<T>::Column() is buggy.
      //gVector<T> temp = QR.Column(j);
      //A.SetColumn(j, temp);
      gVector<T> temp(N);
      QR.GetColumn(j, temp);
      A.SetColumn(j, temp);
    }
    gMatrix<T> C(N, 1);
    C.SetColumn(1, QR_Np2);
    gLapack<T> la;
    la.dormqr('L', 'T', N, A, tau, C, info);
    C.GetColumn(1, QR_Np2);
  }
  
  for (int i = 1; i <= N; ++i)
    alpha[i] = QR(i, i);
  
  // "Compute kernel of jacobian, which specifies Ytan = dY/dS."
  Newton_step[N+1] = 1;
  for (int i = N; i >= 1; --i) {
    //Newton_step[i] = -DOT_PRODUCT( QR[i, i+1:N+1], Newton_step[i+1:N+1] ) / alpha[i];
    T dot_product = 0;
    for (int j = i+1; j <= N+1; ++j)
      dot_product += QR(i, j) * Newton_step[j] ;
    
    Newton_step[i] = -dot_product / alpha[i];
  }
  
  T Ytan_norm = sqrt( Newton_step.NormSquared() );
  for (int i = 1; i <= N+1; ++i)
    Ytan[pivot[i]] = Newton_step[i] / Ytan_norm;
  
  if ( Ytan * Ytan_old < 0)
    Ytan = -Ytan;
  
  // "Ytan is the unit tangent vector in the correct direction."
  
  // "Compute the minimum norm solution of [d rho(Y(S))] V = -rho(Y(S)).
  //  V is given by P - (P,Q)Q, where P is any solution of
  //  [d rho] V = -rho and Q is a unit vector in the kernel of [d rho]."
  alpha[N+1] = 1;
  for (int i = N; i >= 1; --i) {
    //alpha[i] = -( DOT_PRODUCT(QR[i,i+1:N+1], alpha[i+1:N+1]) + QR[i,N+2]) / alpha[i];
    T dot_product = 0;
    for (int j = i+1; j <= N+1; ++j)
      dot_product += QR(i, j) * alpha[j];
    alpha[i] = -(dot_product + QR_Np2[i]) / alpha[i];
  }
  
  //Newton_step[pivot] = alpha[1:N+1];
  for (int i = 1; i <= N+1; ++i)
    Newton_step[pivot[i]] = alpha[i];
  
  // "Newton_step now contains a particular solution P, and Ytan contains a vector Q
  //  in the kernel (the tangent)."
  Newton_step -= Ytan * (Newton_step * Ytan);
  // "Newton_step is the Newton step from the current point Y(S) = (lambda(S), X(S))."
  /*&
    printf("NS: ");
    for (int i = 1; i<= N+1;i++)
    printf("%4.2lf ",Newton_step[i]);
    printf("\n");
  */
}

template <class T> 
void gHompack<T>::F( const gVector<T> &X, gVector<T> &V )  
{ }

template <class T> 
void gHompack<T>::Fjac(const gVector<T> &X, gVector<T> &V, int K ) 
{ }

template <class T> 
void gHompack<T>::rho(const gVector<T> &A, 
		      const T lambda, const gVector<T> &X, gVector<T> &V )
{
  T eps = 0.00;
  T tlambda = 1/(1-lambda) - 1; // switch in hompack3 as wells as here

  MixedProfile<T> sol(supp);
  int j = 1;
  for(int pl = 1;pl<=nfg.NumPlayers();pl++) {
    T resid = 1;
    int i;
    for(i=1;i<supp.NumStrats(pl);i++) {
      sol(pl,i) = X[j];
      resid -= X[j];
      j++;
    }
    sol(pl,i) =  resid;
  }

  j=1;
  for(int pl=1;pl<=nfg.NumPlayers();pl++)
    for(int i = 2; i <= supp.NumStrats(pl); i++) {
      V[j] =  my_log(sol(pl,1),eps)- my_log(sol(pl,i),eps) 
	- tlambda*(sol.Payoff(pl,pl,1)-sol.Payoff(pl,pl,i));
      j++;
    }  
}

template <class T> 
void gHompack<T>::rhojac(const gVector<T> &A, 
			 const T lambda, const gVector<T> &X, 
			 gVector<T> &V, int K )
{
  int n_strats =supp.TotalNumStrats() ;  
  int n_players = nfg.NumPlayers();
  
  gVector<T> Vxh(n_strats-n_players);
  gVector<T> Vx(n_strats-n_players);
  gVector<T> xh(n_strats-n_players);
  T dh = 1E-12;
  
  if (K == 1) {
    rho(A, lambda+dh, X, Vxh);
    rho(A, lambda, X, Vx);
    V = (Vxh-Vx)/dh;
  } // end of K == 1 i.e. w.r.t. lambda
  
  
  if ((K >= 2) && (K <= n_strats-n_players+1 )) {
    xh = X;
    xh[K-1] += dh;
    rho(A, lambda, xh,Vxh);
    rho(A, lambda, X, Vx);
    V = (Vxh-Vx)/dh;
  } // end of ((K >= 2) && (K <= n_strats-n_players+1 ))
} 

template <class T>
const gList<MixedSolution> &gHompack<T>::GetSolutions(void) const
{
  return solutions;
}

template <class T>
T gHompack<T>::QreValue(MixedProfile<T> &sol, T lambda)
{
  double val = 0.0, z;

  for (int pl = 1; pl <= nfg.NumPlayers(); pl++)  {
    for (int st = 2; st <= supp.NumStrats(pl); st++) {
      z = log(sol(pl, 1)) - log(sol(pl, st)) -
          lambda * (sol.Payoff(pl,pl,1) - sol.Payoff(pl,pl,st));
      val += z * z;
    }
  }
  return val;
}

// Instantiations  

template class gHompack<double>;

