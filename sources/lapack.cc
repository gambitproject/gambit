//
// FILE: lapack.cc -- Hompack code
//
// $Id$
//

// Includes //////////////////////////////////////////////////////////////
#include <float.h>
#include <iostream.h>
#include <math.h>
#include <stdio.h>

#include "gmisc.h"
#include "lapack.h"

template <class T> gLapack<T>::gLapack(void)
{ } 

template <class T> gLapack<T>::~gLapack(void)
{ } 



// dgeqpf ////////////////////////////////////////////////////////////////
/*  DGEQPF computes a QR factorization with column pivoting of a
    real M-by-N matrix A: A*P = Q*R.

    Arguments
    =========

    M       (input) INTEGER
            The number of rows of the matrix A. M >= 0.

    N       (input) INTEGER
            The number of columns of the matrix A. N >= 0

    A       (input/output) DOUBLE PRECISION array, dimension (LDA,N)
            On entry, the M-by-N matrix A.
            On exit, the upper triangle of the array contains the
            min(M,N)-by-N upper triangular matrix R; the elements
            below the diagonal, together with the array TAU,
            represent the orthogonal matrix Q as a product of
            min(m,n) elementary reflectors.

    LDA     (input) INTEGER
            The leading dimension of the array A. LDA >= max(1,M).

    JPVT    (input/output) INTEGER array, dimension (N)
            On entry, if JPVT(i) .ne. 0, the i-th column of A is permuted
            to the front of A*P (a leading column); if JPVT(i) = 0,
            the i-th column of A is a free column.
            On exit, if JPVT(i) = k, then the i-th column of A*P
            was the k-th column of A.

    TAU     (output) DOUBLE PRECISION array, dimension (min(M,N))
            The scalar factors of the elementary reflectors.

    WORK    (workspace) DOUBLE PRECISION array, dimension (3*N)

    INFO    (output) INTEGER
            = 0:  successful exit
            < 0:  if INFO = -i, the i-th argument had an illegal value

    Further Details
    ===============

    The matrix Q is represented as a product of elementary reflectors

       Q = H(1) H(2) . . . H(n)

    Each H(i) has the form

       H = I - tau * v * v'

    where tau is a real scalar, and v is a real vector with
    v(1:i-1) = 0 and v(i) = 1; v(i+1:m) is stored on exit in A(i+1:m,i).


    The matrix P is represented in jpvt as follows: If
       jpvt(j) = i
    then the jth column of P is the ith canonical unit vector.
*/

template <class T> void gLapack<T>::dgeqpf(gMatrix<T> &A, gArray<int> &jpvt, gVector<T> &tau, int &info)
{
  //cout << "Entered dgeqpf()." << endl;
  
  int m = A.NumRows();
  int n = A.NumColumns();
  
  info = 0;
  if (m < 0)          info = -1;
  else if (n < 0)     info = -2;
  
  if (info != 0) {
    xerbla("DGEQPF", -info);
    return;
  }
  
  int mn = gmin(m,n);
  
  // "Move initial columns up front"
  int itemp = 1;
  for (int i = 1; i <= n; ++i) {
    if (jpvt[i] != 0) {
      if (i != itemp) {
	A.SwitchColumns(i, itemp);
	jpvt[i] = jpvt[itemp];
	jpvt[itemp] = i;
      } else {
	jpvt[i] = i;
      }
      ++itemp;
    } 
    else {
      jpvt[i] = i;
    }
  }
  --itemp;
  
  // "Compute the QR factorization and update remaining columns"
  if (itemp > 0) {
    int ma = gmin(itemp,m);
    
    //dgeqr2(m, ma, &A(1,1), lda, &TAU(1), &WORK(1), info);
    // Extract apropriate members of A.
    gMatrix<T> temp(m, ma);
    for (int c = 1; c <= ma; ++c) {
      gVector<T> temp2(n);
      A.GetColumn(c, temp2);
      temp.SetColumn(c, temp2);
    }
    // Call dgeqr2
    dgeqr2(temp, tau, info);
    //cout << "dgeqpf: Left dgeqr2." << endl;
    // Put the results back in A.
    for (int c = 1; c <= ma; ++c) {
      gVector<T> temp2(n);
      temp.GetColumn(c, temp2);
      A.SetColumn(c, temp2);
    }
    
    if (ma < n) {
      //dorm2r('L', 'T', m, n - ma, ma, &A(1,1), lda,
      //     &TAU(1), &A(1,ma+1), lda, &WORK(1), info);
      // Extract the apropriate members of A.
      gMatrix<T> temp(m, ma);
      for (int c = 1; c <= ma; ++c) {
	gVector<T> temp2(n);
	A.GetColumn(c, temp2);
	temp.SetColumn(c, temp2);
      }
      gMatrix<T> C(m, n-ma);
      for (int c = 1; c <= n-ma; ++c) {
	gVector<T> temp2(n);
	A.GetColumn(ma+c, temp2);
	C.SetColumn(c, temp2);
      }
      
      // Call dorm2r
      dorm2r('L', 'T', ma, temp, tau, C, info);
      //cout << "dgeqpf: Left dorm2r." << endl;
      
      // Put the results back in A.
      for (int c = 1; c <= n-ma; ++c) {
	gVector<T> temp2(n);
	C.GetColumn(c, temp2);
	A.SetColumn(ma+c, temp2);
      }
    }
  }
  
  if (itemp < mn)
    {
      gVector<T> work(3*n);
      
      // "Initialize partial column norms. The first n elements of
      //  work store the exact column norms."
      for (int i = itemp + 1; i <= n; ++i) {
	//WORK(i) = dnrm2_(m - itemp, &A(itemp+1,i), 1);
	//WORK(n + i) = WORK(i);
	
	gVector<T> temp(m - itemp);
	for (int j = 1; j <= temp.Length(); ++j)
	  temp[j] = A(itemp+j, i);
	work[n + i] = work[i] = sqrt(temp.NormSquared());
      }
      
      // "Compute factorization"
      for (int i = itemp + 1; i <= mn; ++i) {
	// "Determine ith pivot column and swap if necessary"
	//int pvt = i - 1 + idamax_(n - i + 1, &WORK(i), 1);
	int pvt = i; T max = fabs(work[pvt]);
	for (int idx = pvt+1; idx <= n; ++idx) {
	  T tmp = fabs(work[idx]);
	  if ( tmp > max ) {
	    pvt = idx; max = tmp;
	  }
	}
	
	if (pvt != i) {
	  A.SwitchColumns(pvt, i);
	  itemp = jpvt[pvt];
	  jpvt[pvt] = jpvt[i];
	  jpvt[i] = itemp;
	  work[pvt] = work[i];
	  work[n + pvt] = work[n + i];
	}
	
	// "Generate elementary reflector H(i)"
	if (i < m) {
	  //dlarfg(m - i + 1, &A(i,i), &A(i+1,i), 1, &TAU(i));
	  // Extract the apropriate members of A
	  gVector<T> X(m-i);
	  for (int j = 1; j <= X.Length(); ++j)
	    X[j] = A(i+j, i);
	  // Call dlarfg
	  dlarfg( A(i, i), X, tau[i] );
	  //cout << "dgeqpf: Left dlarfg, #1." << endl;
	  // And put the results back in A
	  for (int j = 1; j <= X.Length(); ++j)
	    A(i+j, i) = X[j];
	} 
	else {
	  //dlarfg(1, &A(m,m), &A(m,m), 1, &TAU(m));
	  gVector<T> X(0);
	  //X = A(m, m);
	  dlarfg( A(m, m), X, tau[m] );
	  //cout << "dgeqpf: Left dlarfg, #2." << endl;
	  //A(m, m) = X[1];
	}
	
	if (i < n) {
	  // "Apply H(i) to A(i:m,i+1:n) from the left"
	  T aii = A(i,i);
	  A(i,i) = 1;
	  
	  //dlarf('L', m - i + 1, n - i, &A(i,i), 1, &TAU(i),
	  //       &A(i,i+1), lda, &WORK((n << 1) +	1));
	  gVector<T> V(m-i+1);                       // Extract
	  for (int j = 1; j <= V.Length(); ++j)
	    V[j] = A(i+j-1, i);
	  gMatrix<T> C(m-i+1, n-i);
	  for (int r = 1; r <= C.NumRows(); ++r)
	    for (int c = 1; c <= C.NumColumns(); ++c)
	      C(r, c) = A(i+r-1, i+c);
	  dlarf('L', V, tau[i], C);                       // Call
	  //cout << "dgeqpf: Left dlarfg, #3." << endl;
	  for (int r = 1; r <= C.NumRows(); ++r)          // Restore
	    for (int c = 1; c <= C.NumColumns(); ++c)
	      A(i+r-1, i+c) = C(r, c);
	  
	  A(i,i) = aii;
	}
	
	// "Update partial column norms"
	for (int j = i + 1; j <= n; ++j) {
	  if (work[j] != 0) {
	    // "Computing 2nd power"
	    T d = A(i,j) / work[j];
	    T temp = 1 - d * d;
	    if (temp < 0)
	      temp = 0;
	    
	    // "Computing 2nd power"
	    d = work[j] / work[n + j];
	    if (temp * d * d == 0) {
	      if (m - i > 0) {
		gVector<T> tempV(m - i);
		for (int k = 1; k <= tempV.Length(); ++k)
		  tempV[k] = A(i+k, j);
		work[n + j] = work[j] = sqrt(tempV.NormSquared());
	      } else {
		work[n + j] = work[j] = 0;
	      }
	    } else {
	      work[j] *= sqrt(temp);
	    }
	  }
	}
      }
    }
}

// dgeqr2 ////////////////////////////////////////////////////////////////
/*  DGEQR2 computes a QR factorization of a real m by n matrix A:
    A = Q * R.

    Arguments
    =========

    M       (input) INTEGER
            The number of rows of the matrix A.  M >= 0.

    N       (input) INTEGER
            The number of columns of the matrix A.  N >= 0.

    A       (input/output) DOUBLE PRECISION array, dimension (LDA,N)
            On entry, the m by n matrix A.
            On exit, the elements on and above the diagonal of the array

            contain the min(m,n) by n upper trapezoidal matrix R (R is
            upper triangular if m >= n); the elements below the diagonal,

            with the array TAU, represent the orthogonal matrix Q as a
            product of elementary reflectors (see Further Details).

    LDA     (input) INTEGER
            The leading dimension of the array A.  LDA >= max(1,M).

    TAU     (output) DOUBLE PRECISION array, dimension (min(M,N))
            The scalar factors of the elementary reflectors (see Further
            Details).

    WORK    (workspace) DOUBLE PRECISION array, dimension (N)

    INFO    (output) INTEGER
            = 0: successful exit
            < 0: if INFO = -i, the i-th argument had an illegal value

    Further Details
    ===============

    The matrix Q is represented as a product of elementary reflectors

       Q = H(1) H(2) . . . H(k), where k = min(m,n).

    Each H(i) has the form

       H(i) = I - tau * v * v'

    where tau is a real scalar, and v is a real vector with
    v(1:i-1) = 0 and v(i) = 1; v(i+1:m) is stored on exit in A(i+1:m,i),
    and tau in TAU(i).
*/

template <class T> void gLapack<T>::dgeqr2(gMatrix<T> &A, gArray<T> &tau, int &info)
{
  //cout << "Entered dgeqr2." << endl;
  
  int m = A.NumRows();
  int n = A.NumColumns();
  
  info = 0;
  if (m < 0)	        info = -1;
  else if (n < 0)     info = -2;
  
  if (info != 0) {
    xerbla("DGEQR2", -info);
    return;
  }
  
  int k = gmin(m,n);
  for (int i = 1; i <= k; ++i) {
    // "Generate elementary reflector H(i) to annihilate A(i+1:m,i)"
    // "Computing MIN"
    
    // First extract the proper vector from A.
    gVector<T> temp(A.NumRows() - i);
    for (int j = 1; j <= temp.Length(); ++j)
      temp[j] = A(i+j, i);
    
    // Now call dlarfg
    dlarfg( A(i, i), temp, tau[i] );
    //cout << "dgeqr2: Left dlarfg #1." << endl;
    
    // And put the results back in A.
    for (int j = 1; j <= temp.Length(); ++j)
      A(i+j, i) = temp[j];
    
    if (i < n) {
      // "Apply H(i) to A(i:m,i+1:n) from the left"
      T aii = A(i,i);
      A(i,i) = 1.;
      
      // First extract the appropriate members of A.
      gVector<T> V(m-i+1);
      for (int j = 1; j <= V.Length(); ++j)
	V[j] = A(i+j-1, i);
      gMatrix<T> temp(m-i+1, n-1);
      for (int r = 1; r <= temp.NumRows(); ++r)
	for (int c = 1; c <= temp.NumColumns(); ++c)
	  temp(r, c) = A(i+r-1, i+c);
      
      // Now call dlarf
      dlarf('L', V, tau[i], temp);
      //cout << "dgeqr2: Left dlarfg, #2." << endl;
      
      // And put the results back into A.
      for (int r = 1; r <= temp.NumRows(); ++r)
	for (int c = 1; c <= temp.NumColumns(); ++c)
	  A(i+r-1, i+c) = temp(r, c);
      
      A(i,i) = aii;
    }
  }
}

// dlarfg ////////////////////////////////////////////////////////////////
/*  DLARFG generates a real elementary reflector H of order n, such
    that

          H * ( alpha ) = ( beta ),   H' * H = I.
              (   x   )   (   0  )

    where alpha and beta are scalars, and x is an (n-1)-element real
    vector. H is represented in the form

          H = I - tau * ( 1 ) * ( 1 v' ) ,
                        ( v )

    where tau is a real scalar and v is a real (n-1)-element
    vector.

    If the elements of x are all zero, then tau = 0 and H is taken to be
    the unit matrix.

    Otherwise  1 <= tau <= 2.

    Arguments
    =========

    N       (input) INTEGER
            The order of the elementary reflector.

    ALPHA   (input/output) DOUBLE PRECISION
            On entry, the value alpha.
            On exit, it is overwritten with the value beta.

    X       (input/output) DOUBLE PRECISION array, dimension
                           (1+(N-2)*abs(INCX))
            On entry, the vector x.
            On exit, it is overwritten with the vector v.

    INCX    (input) INTEGER
            The increment between elements of X. INCX > 0.

    TAU     (output) DOUBLE PRECISION
            The value tau.
*/

template <class T> void gLapack<T>::dlarfg(T &alpha, gVector<T> &X, T &tau)
{
  //cout << "Entered dlarfg()." << endl;
  
  int n = X.Length() + 1;
  if (n <= 1) {
    tau = 0;
    return;
  }
  
  T xnorm2 = X.NormSquared();
  
  if (xnorm2 == 0)
    // "H  =  I"
    tau = 0;
  else {
    // "general case"
    T d = sqrt(alpha*alpha + xnorm2);
    T beta = (alpha > 0) ? -d : d;
    T safmin = DBL_MIN / (DBL_EPSILON / 2);
    if (d < safmin)
      {
	// "XNORM, BETA may be inaccurate; scale X and recompute them"
	T rsafmn = 1. / safmin;
	int knt = 0;
	
	do {
	  ++knt;
	  X *= rsafmn;
	  beta *= rsafmn;
	  alpha *= rsafmn;
	} 
	while ( abs(beta) < safmin );
	
	// "New BETA is at most 1, at least SAFMIN"
	xnorm2 = X.NormSquared();
	T d = sqrt(alpha*alpha + xnorm2);
	beta = (alpha > 0) ? -d : d;
	tau = (beta - alpha) / beta;
	X *= 1. / (alpha - beta);
	
	// "If ALPHA is subnormal, it may lose relative accuracy"
	alpha = beta;
	for (int j = 1; j <= knt; ++j)
	  alpha *= safmin;
      }
    else {
      tau = (beta - alpha) / beta;
      X *= 1. / (alpha - beta);
      alpha = beta;
    }
  }
}

// dormqr ////////////////////////////////////////////////////////////////
/*  DORMQR overwrites the general real M-by-N matrix C with

                    SIDE = 'L'     SIDE = 'R'
    TRANS = 'N':      Q * C          C * Q
    TRANS = 'T':      Q**T * C       C * Q**T

    where Q is a real orthogonal matrix defined as the product of k
    elementary reflectors

          Q = H(1) H(2) . . . H(k)

    as returned by DGEQRF. Q is of order M if SIDE = 'L' and of order N
    if SIDE = 'R'.

    Arguments
    =========

    SIDE    (input) CHARACTER*1
            = 'L': apply Q or Q**T from the Left;
            = 'R': apply Q or Q**T from the Right.

    TRANS   (input) CHARACTER*1
            = 'N':  No transpose, apply Q;
            = 'T':  Transpose, apply Q**T.

    M       (input) INTEGER
            The number of rows of the matrix C. M >= 0.

    N       (input) INTEGER
            The number of columns of the matrix C. N >= 0.

    K       (input) INTEGER
            The number of elementary reflectors whose product defines
            the matrix Q.
            If SIDE = 'L', M >= K >= 0;
            if SIDE = 'R', N >= K >= 0.

    A       (input) DOUBLE PRECISION array, dimension (LDA,K)
            The i-th column must contain the vector which defines the
            elementary reflector H(i), for i = 1,2,...,k, as returned by

            DGEQRF in the first k columns of its array argument A.
            A is modified by the routine but restored on exit.

    LDA     (input) INTEGER
            The leading dimension of the array A.
            If SIDE = 'L', LDA >= max(1,M);
            if SIDE = 'R', LDA >= max(1,N).

    TAU     (input) DOUBLE PRECISION array, dimension (K)
            TAU(i) must contain the scalar factor of the elementary
            reflector H(i), as returned by DGEQRF.

    C       (input/output) DOUBLE PRECISION array, dimension (LDC,N)
            On entry, the M-by-N matrix C.
            On exit, C is overwritten by Q*C or Q**T*C or C*Q**T or C*Q.


    LDC     (input) INTEGER
            The leading dimension of the array C. LDC >= max(1,M).

    WORK    (workspace/output) DOUBLE PRECISION array, dimension (LWORK)
            On exit, if INFO = 0, WORK(1) returns the optimal LWORK.

    LWORK   (input) INTEGER
            The dimension of the array WORK.
            If SIDE = 'L', LWORK >= max(1,N);
            if SIDE = 'R', LWORK >= max(1,M).
            For optimum performance LWORK >= N*NB if SIDE = 'L', and
            LWORK >= M*NB if SIDE = 'R', where NB is the optimal
            blocksize.

    INFO    (output) INTEGER
            = 0:  successful exit
            < 0:  if INFO = -i, the i-th argument had an illegal value

*/

template <class T> void gLapack<T>::dormqr(char side, char trans, int k, gMatrix<T> &A,
            gVector<T> &tau, gMatrix<T> &C, int &info)
{
  //cout << "Entered dormqr()." << endl;
  
  int nq, nw;
  int m = C.NumRows();
  int n = C.NumColumns();
  
  info = 0;
  bool left   = (side == 'L');
  bool notran = (trans == 'N');
  
  // "NQ is the order of Q and NW is the minimum dimension of WORK"
  if (left) {
    nq = m;	nw = n;
  } 
  else {
    nq = n;	nw = m;
  }
  
  if ( !left && !(side == 'R') )              info = -1;
  else if ( !notran && !(trans == 'T') )      info = -2;
  else if (m < 0)                             info = -3;
  else if (n < 0)                             info = -4;
  else if (k < 0 || k > nq)                   info = -5;
  
  if (info != 0) {
    xerbla("DORMQR", -info);
    return;
  }
  
  // "Quick return if possible"
  if (m == 0 || n == 0 || k == 0)
    return;
  
  int iinfo;
  dorm2r(side, trans, k, A, tau, C, iinfo);
  //cout << "dormqr: Left dorm2r()." << endl;
}

// dorm2r ////////////////////////////////////////////////////////////////
/*  DORM2R overwrites the general real m by n matrix C with

          Q * C  if SIDE = 'L' and TRANS = 'N', or

          Q'* C  if SIDE = 'L' and TRANS = 'T', or

          C * Q  if SIDE = 'R' and TRANS = 'N', or

          C * Q' if SIDE = 'R' and TRANS = 'T',

    where Q is a real orthogonal matrix defined as the product of k
    elementary reflectors

          Q = H(1) H(2) . . . H(k)

    as returned by DGEQRF. Q is of order m if SIDE = 'L' and of order n
    if SIDE = 'R'.

    Arguments
    =========

    SIDE    (input) CHARACTER*1
            = 'L': apply Q or Q' from the Left
            = 'R': apply Q or Q' from the Right

    TRANS   (input) CHARACTER*1
            = 'N': apply Q  (No transpose)
            = 'T': apply Q' (Transpose)

    M       (input) INTEGER
            The number of rows of the matrix C. M >= 0.

    N       (input) INTEGER
            The number of columns of the matrix C. N >= 0.

    K       (input) INTEGER
            The number of elementary reflectors whose product defines
            the matrix Q.
            If SIDE = 'L', M >= K >= 0;
            if SIDE = 'R', N >= K >= 0.

    A       (input) DOUBLE PRECISION array, dimension (LDA,K)
            The i-th column must contain the vector which defines the
            elementary reflector H(i), for i = 1,2,...,k, as returned by

            DGEQRF in the first k columns of its array argument A.
            A is modified by the routine but restored on exit.

    LDA     (input) INTEGER
            The leading dimension of the array A.
            If SIDE = 'L', LDA >= max(1,M);
            if SIDE = 'R', LDA >= max(1,N).

    TAU     (input) DOUBLE PRECISION array, dimension (K)
            TAU(i) must contain the scalar factor of the elementary
            reflector H(i), as returned by DGEQRF.

    C       (input/output) DOUBLE PRECISION array, dimension (LDC,N)
            On entry, the m by n matrix C.
            On exit, C is overwritten by Q*C or Q'*C or C*Q' or C*Q.

    LDC     (input) INTEGER
            The leading dimension of the array C. LDC >= max(1,M).

    WORK    (workspace) DOUBLE PRECISION array, dimension
                                     (N) if SIDE = 'L',
                                     (M) if SIDE = 'R'

    INFO    (output) INTEGER
            = 0: successful exit
            < 0: if INFO = -i, the i-th argument had an illegal value
*/

template <class T> void gLapack<T>::dorm2r(char side, char trans, int k, gMatrix<T> &A,
            gVector<T> &tau, gMatrix<T> &C, int &info)
{
  //cout << "Entered dorm2r()." << endl;
  
  /* Local variables */
  static int ic, jc, mi, ni;
  int m = C.NumRows();
  int n = C.NumColumns();
  
  info = 0;
  bool left   = (side  == 'L');
  bool notran = (trans == 'N');
  
  gVector<T> work(left ? n : m);
  
  // "NQ is the order of Q"
  int nq = left ? m : n;
  
  if (!left && side != 'R')           info = -1;
  else if (!notran && trans != 'T')   info = -2;
  else if (m < 0)                     info = -3;
  else if (n < 0)                     info = -4;
  else if (k < 0 || k > nq)           info = -5;
  
  if (info != 0) {
    xerbla("DORM2R", -info);
    return;
  }
  
  // "Quick return if possible"
  if (m == 0 || n == 0 || k == 0)
    return;
  
  int i1, i2, i3;
  if (left && !notran || !left && notran) {
    i1 = 1; i2 = k; i3 = 1;
  } 
  else {
    i1 = k; i2 = 1; i3 = -1;
  }
  
  if (left) {
    ni = n; jc = 1;
  } 
  else {
    mi = m; ic = 1;
  }
  
  for ( int i = i1;
	i3 < 0 ? i >= i2 : i <= i2;
	i += i3) {
    if (left) {
      // "H(i) is applied to C(i:m,1:n)"
      mi = m - i + 1;
      ic = i;
    } 
    else {
      // "H(i) is applied to C(1:m,i:n)"
      ni = n - i + 1;
      jc = i;
    }
    
    // "Apply H(i)"
    T aii = A(i,i);
    A(i,i) = 1;
    
    //dlarf(side, mi, ni, &A(i,i), 1, &tau[i], &C(ic,jc), ldc, &work[1]);
    // First extract the apropriate members of A and C
    gVector<T> V(left ? mi : ni);
    for (int j = 1; j <= V.Length(); ++j)
      V[j] = A(i+j-1, i);
    gMatrix<T> temp(mi, ni);
    for (int r = 1; r <= mi; ++r)
      for (int c = 1; c <= ni; ++c)
	temp(r, c) = C(ic+r-1, jc+c-1);
    
    // Now call dlarf
    dlarf(side, V, tau[i], temp);
    //cout << "dorm2r: Left dlarf()." << endl;
    
    // And finaly put the results back into C
    for (int r = 1; r <= mi; ++r)
      for (int c = 1; c <= ni; ++c)
	C(ic+r-1, jc+c-1) = temp(r, c);
    
    A(i,i) = aii;
  }
}

// dlarf /////////////////////////////////////////////////////////////////
/*  DLARF applies a real elementary reflector H to a real m by n matrix
    C, from either the left or the right. H is represented in the form

     H = I - tau * v * v'

    where tau is a real scalar and v is a real vector.

    If tau = 0, then H is taken to be the unit matrix.

    Arguments
    =========

    SIDE    (input) CHARACTER*1
            = 'L': form  H * C
            = 'R': form  C * H

    M       (input) INTEGER
            The number of rows of the matrix C.

    N       (input) INTEGER
            The number of columns of the matrix C.

    V       (input) DOUBLE PRECISION array, dimension
                       (1 + (M-1)*abs(INCV)) if SIDE = 'L'
                    or (1 + (N-1)*abs(INCV)) if SIDE = 'R'
            The vector v in the representation of H. V is not used if
            TAU = 0.

    INCV    (input) INTEGER
            The increment between elements of v. INCV <> 0.

    TAU     (input) DOUBLE PRECISION
            The value tau in the representation of H.

    C       (input/output) DOUBLE PRECISION array, dimension (LDC,N)
            On entry, the m by n matrix C.
            On exit, C is overwritten by the matrix H * C if SIDE = 'L',
            or C * H if SIDE = 'R'.

    LDC     (input) INTEGER
            The leading dimension of the array C. LDC >= max(1,M).

    WORK    (workspace) DOUBLE PRECISION array, dimension
            (N) if SIDE = 'L'
            or (M) if SIDE = 'R'

    */

template <class T> void gLapack<T>::dlarf(char side, gVector<T> &V, T tau, gMatrix<T> &C)
{
  //cout << "Entered dlarf()." << endl;
  
  if (side == 'L') {
    // "Form  H * C"
    if (tau != 0) {
      // "w := C' * v"
      gMatrix<T> work(C.NumColumns(), 1);
      work.SetColumn(1, C.Transpose() * V);
      gMatrix<T> Vmat(V.Length(), 1);
      Vmat.SetColumn(1, V);
      
      // "C := C - v * w'"
      // C += -tau * V * W'
      C += (Vmat * work.Transpose()) * (-tau);
    }
  }
  else // if (side != 'L')
    {
      // "Form  C * H"
      if (tau != 0) {
	// "w := C * v"
	gMatrix<T> work(C.NumRows(), 1);
	work.SetColumn(1, C * V);
	
	// "C := C - w * v'"
	// C += -tau * W * V'
	// V needs to be transposed first
	gMatrix<T> Vmat(V.Length(), 1);
	Vmat.SetColumn(1, V);
	C += (work * Vmat.Transpose()) * (-tau);
      }
    }
}

// xerbla ////////////////////////////////////////////////////////////////
/*  -- LAPACK auxiliary routine (version 2.0) --
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
       Courant Institute, Argonne National Lab, and Rice University
       September 30, 1994


    Purpose
    =======

    XERBLA  is an error handler for the LAPACK routines.
    It is called by an LAPACK routine if an input parameter has an
    invalid value.  A message is printed and execution stops.

    Installers may consider modifying the STOP statement in order to
    call system-specific exception-handling facilities.

    Arguments
    =========

    SRNAME  (input) CHARACTER*6
            The name of the routine which called XERBLA.

    INFO    (input) INTEGER
            The position of the invalid parameter in the parameter list

            of the calling routine.

   =====================================================================
*/

template <class T> void gLapack<T>::xerbla(char *srname, int info)
{
  printf("** On entry to %6s, parameter number %2i had an illegal value\n",
	 srname, info);
}

template class gLapack<double>;
