//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of QR algorithm for computing eigenvalues
// Based on Numerical Recipes
//

#include <math.h>
#include "math/gsmatrix.h"

void Balance(gSquareMatrix<double> &p_matrix)
{
  const double RADIX = 2.0;

  double sqrdx = RADIX * RADIX;
  bool last = false;

  while (!last) {
    last = true;
    for (int i = 1; i <= p_matrix.NumRows(); i++) {  
      // calculate row and column norms
      double r = 0.0, c = 0.0;
      for (int j = 1; j <= p_matrix.NumRows(); j++) {
	if (j != i) {
	  c += fabs(p_matrix(j, i));
	  r += fabs(p_matrix(i, j));
	}
      }

      if (c && r) {
	double g = r / RADIX;
	double f = 1.0;
	double s = c + r;

	while (c < g) {
	  // find the integer power of the machine radix that
	  // comes closest to balancing the matrix
	  f *= RADIX;
	  c *= sqrdx;
	}
	g = r * RADIX;
	while (c > g) {
	  f /= RADIX;
	  c /= sqrdx;
	}

	if ((c + r) / f < 0.95 * s) {
	  last = false;
	  g = 1.0 / f;
	  /* apply similarity transformation */
	  for (int j = 1; j <= p_matrix.NumRows();
	       p_matrix(i, j++) *= g);
	  for (int j = 1; j <= p_matrix.NumRows(); 
	       p_matrix(j++, i) *= f);
	}
      }
    }
  }
}

inline void swap(double &x, double &y)
{
  double tmp = x;
  x = y;
  y = tmp;
}

void MakeHessenberg(gSquareMatrix<double> &p_matrix)
{
  for (int m = 2; m < p_matrix.NumRows(); m++) {
    double x = 0.0;
    int i = m;
    for (int j = m; j <= p_matrix.NumRows(); j++) {
      // Find the pivot
      if (fabs(p_matrix(j, m-1)) > fabs(x)) {
	x = p_matrix(j, m-1);
	i = j;
      }
    }

    if (i != m) {
      // interchange rows and columns
      for (int j = m - 1; j <= p_matrix.NumRows(); j++) {
	swap(p_matrix(i, j), p_matrix(m, j));
      }

      for (int j = 1; j <= p_matrix.NumRows(); j++) {
	swap(p_matrix(j, i), p_matrix(j, m));
      }
    }

    if (x) {
      // carry out the elimination
      for (int i = m + 1; i <= p_matrix.NumRows(); i++) {
	double y = p_matrix(i, m-1);
	if (y != 0.0) {
	  y /= x;
	  p_matrix(i, m-1) = y;
	  for (int j = m; j <= p_matrix.NumRows(); j++) {
	    p_matrix(i, j) -= y * p_matrix(m, j);
	  }
	  for (int j = 1; j <= p_matrix.NumRows(); j++) {
	    p_matrix(j, m) += y * p_matrix(j, i);
	  }
	}
      }
    }
  }
}

inline double ifsign(double p, double q)
{
  return (q >= 0.0) ? fabs(p) : -fabs(p);
}

void Eigenvalues(const gSquareMatrix<double> &p_matrix, 
		 gArray<double> &p_realParts, gArray<double> &p_complexParts)
{
  gSquareMatrix<double> M(p_matrix.NumRows());
  for (int i = 1; i <= M.NumRows(); i++) {
    for (int j = 1; j <= M.NumRows(); j++) {
      M(i, j) = p_matrix(i, j);
    }
  }
  Balance(M);
  MakeHessenberg(M);

  double anorm = 0.0;
  double z, y, x, w, v, u, t, s, r, q, p;
  int nn, m, l, k, j, its, i, mmin;
  
  for (i = 1; i <= M.NumRows(); i++) {
    for (j = (i > 1) ? (i - 1) : 1; j <= M.NumRows(); j++) {
      anorm += fabs(M(i, j));
    }
  }

  nn = M.NumRows();
  t = 0.0;
  while (nn >= 1) {
    its = 0;

    do {
      for (l = nn; l >= 2; l--) {
	// look for single small subdiagonal element
	s = fabs(M(l-1, l-1)) + fabs(M(l, l));
	if (s == 0.0) {
	  s = anorm;
	}
	if ((double) fabs((double) M(l, l-1) + (double) s) == (double) s) {
	  break;
	}
      }

      x = M(nn, nn);
      if (l == nn) {
	// one root found
	p_realParts[nn] = x + t;
	p_complexParts[nn--] = 0.0;
      }
      else {
	y = M(nn-1, nn-1);
	w = M(nn, nn-1) * M(nn-1, nn);
	if (l == nn-1) {
	  // two roots found
	  p = 0.5 * (y - x);
	  q = p*p + w;
	  z = sqrt(fabs(q));
	  x += t;
	  if (q >= 0.0) {
	    // a real pair
	    z = p + ifsign(z, p);
	    p_realParts[nn-1] = x + z;
	    p_realParts[nn] = x + z;
	    if (z) {
	      p_realParts[nn] = x - w / z;
	    }
	    p_complexParts[nn-1] = 0.0;
	    p_complexParts[nn] = 0.0;
	  }
	  else {
	    // a complex pair
	    p_realParts[nn-1] = x + p;
	    p_realParts[nn] = x + p;
	    p_complexParts[nn-1] = -z;
	    p_complexParts[nn] = z;
	  }
	  nn -= 2;
	}
	else {
	  // no roots found; continue iteration
	  if (its == 30) {
	    // give up
	    gout << "giving up\n";
	    return;
	  }

	  if ((its + 1) % 10 == 0) {
	    // form "exceptional shift"
	    t += x;
	    for (i = 1; i <= nn; i++) {
	      M(i, i) -= x;
	    }
	    s = fabs(M(nn, nn-1)) + fabs(M(nn-1, nn-2));
	    y = x = 0.75 * s;
	    w = -0.4375 * s * s;
	  }
	  ++its;

	  for (m = nn-2; m >= l; m--) {
	    // form shift, look for 2 consecutive small subdiagonal elements
	    z = M(m, m);
	    r = x - z;
	    s = y - z;
	    p = (r * s - w) / M(m+1, m) + M(m, m+1);
	    q = M(m+1, m+1) - z - r - s;
	    r = M(m+2, m+1);
	    // scale to prevent underflow/overflow
	    s = fabs(p) + fabs(q) + fabs(r);
	    p /= s;
	    q /= s;
	    r /= s;
	    if (m == l) {
	      break;
	    }
	    u = fabs(M(m, m-1)) * (fabs(q) + fabs(r));
	    v = fabs(p) * (fabs(M(m-1, m-1)) + fabs(z) + fabs(M(m+1, m+1)));
	    if ((double) (u + v) == v) {
	      break;
	    }
	  }

	  for (i = m + 2; i <= nn; i++) {
	    M(i, i-2) = 0.0;
	    if (i != m + 2) {
	      M(i, i-3) = 0.0;
	    }
	  }

	  for (k = m; k <= nn - 1; k++) {
	    // double QR step on rows 1 to nn and columns m to nn
	    if (k != m) {
	      // begin setup of Householder vector
	      p = M(k, k-1);
	      q = M(k+1, k-1);
	      r = 0.0;
	      if (k != nn - 1) {
		r = M(k+2, k-1);
	      }
	      if ((x = fabs(p) + fabs(q) + fabs(r)) != 0.0) {
		// scale to prevent overflow or underflow
		p /= x;
		q /= x;
		r /= x;
	      }
	    }

	    if ((s = ifsign(sqrt(p*p+q*q+r*r), p)) != 0.0) {
	      if (k == m) {
		if (l != m) {
		  M(k, k-1) = -M(k, k-1);
		}
	      }
	      else {
		M(k, k-1) = -s * x;
	      }
	      p += s;
	      x = p / s;
	      y = q / s;
	      z = r / s;
	      q /= p;
	      r /= p;
	      for (j = k; j <= nn; j++) {
		// row modification
		p = M(k, j) + q*M(k+1, j);
		if (k != nn - 1) {
		  p += r * M(k+2, j);
		  M(k+2, j) -= p * z;
		}
		M(k+1, j) -= p*y;
		M(k, j) -= p*x;
	      }
	      
	      mmin = (nn < k + 3) ? nn : k+3;
	      for (i = 1; i <= mmin; i++) {
		// column modification
		p = x * M(i, k) + y * M(i, k+1);
		if (k != nn-1) {
		  p += z * M(i, k+2);
		  M(i, k+2) -= p * r;
		}
		M(i, k+1) -= p*q;
		M(i, k) -= p;
	      }
	    }
	  }
	}
      }
    } while (l < nn - 1);
  }
}

