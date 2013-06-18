//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/logit/path.cc
// Implementation of generic smooth path-following algorithm.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include <cmath>
#include <algorithm>   // for std::max
#include <iostream>

#include <libgambit/libgambit.h>
#include <libgambit/sqmatrix.h>
using namespace Gambit;

#include "path.h"

//----------------------------------------------------------------------------
//                       PathTracer: Auxiliary functions
//----------------------------------------------------------------------------

inline double sqr(double x) { return x*x; }

static void Givens(Matrix<double> &b, Matrix<double> &q,
		   double &c1, double &c2, int l1, int l2, int l3)
{
  if (fabs(c1) + fabs(c2) == 0.0) {
    return;
  }

  double sn;
  if (fabs(c2) >= fabs(c1)) {
    sn = sqrt(1.0 + sqr(c1/c2)) * fabs(c2);
  }
  else {
    sn = sqrt(1.0 + sqr(c2/c1)) * fabs(c1);
  }
  double s1 = c1/sn;
  double s2 = c2/sn;

  for (int k = 1; k <= q.NumColumns(); k++) {
    double sv1 = q(l1, k);
    double sv2 = q(l2, k);
    q(l1, k) = s1 * sv1 + s2 * sv2;
    q(l2, k) = -s2 * sv1 + s1 * sv2;
  }

  for (int k = l3; k <= b.NumColumns(); k++) {
    double sv1 = b(l1, k);
    double sv2 = b(l2, k);
    b(l1, k) = s1 * sv1 + s2 * sv2;
    b(l2, k) = -s2 * sv1 + s1 * sv2;
  }

  c1 = sn;
  c2 = 0.0;
}

static void QRDecomp(Matrix<double> &b, Matrix<double> &q)
{
  q.MakeIdent();
  for (int m = 1; m <= b.NumColumns(); m++) {
    for (int k = m + 1; k <= b.NumRows(); k++) {
      Givens(b, q, b(m, m), b(k, m), m, k, m + 1);
    }
  }
}

static void NewtonStep(Matrix<double> &q, Matrix<double> &b,
		       Vector<double> &u, Vector<double> &y,
		       double &d)
{
  for (int k = 1; k <= b.NumColumns(); k++) {
    for (int l = 1; l <= k - 1; l++) {
      y[k] -= b(l, k) * y[l];
    }
    y[k] /= b(k, k);
  }

  d = 0.0;
  for (int k = 1; k <= b.NumRows(); k++) {
    double s = 0.0;
    for (int l = 1; l <= b.NumColumns(); l++) {
      s += q(l, k) * y[l];
    }
    u[k] -= s;
    d += s * s;
  }
  d = sqrt(d);
}


//----------------------------------------------------------------------------
//             PathTracer: Implementation of path-following engine
//----------------------------------------------------------------------------

void 
PathTracer::TracePath(Vector<double> &x,
		      double p_maxLambda, double &p_omega)
{
  const double c_tol = 1.0e-4;     // tolerance for corrector iteration
  const double c_maxDist = 0.4;    // maximal distance to curve
  const double c_maxContr = 0.6;   // maximal contraction rate in corrector
  const double c_eta = 0.1;        // perturbation to avoid cancellation
                                   // in calculating contraction rate
  double h = m_hStart;             // initial stepsize
  const double c_hmin = 1.0e-8;    // minimal stepsize
  const int c_maxIter = 100;       // maximum iterations in corrector
  
  bool newton = false;             // using Newton steplength (for zero-finding)

  Vector<double> u(x.Length()), restart(x.Length());
  // t is current tangent at x; newT is tangent at u, which is the next point.
  Vector<double> t(x.Length()), newT(x.Length());
  Vector<double> y(x.Length() - 1);
  Matrix<double> b(x.Length(), x.Length() - 1);
  SquareMatrix<double> q(x.Length());

  OnStep(x, false);
  GetJacobian(x, b);
  QRDecomp(b, q);
  q.GetRow(q.NumRows(), t);
  
  while (x[x.Length()] >= 0.0 && x[x.Length()] < p_maxLambda) {
    bool accept = true;

    if (fabs(h) <= c_hmin) {
      if (newton) {
	// Restore the place to restart if desired
	x = restart;
      }
      return;
    }

    // Predictor step
    for (int k = 1; k <= x.Length(); k++) {
      u[k] = x[k] + h * p_omega * t[k];
    }

    double decel = 1.0 / m_maxDecel;  // initialize deceleration factor
    GetJacobian(u, b);
    QRDecomp(b, q);

    int iter = 1;
    double disto = 0.0;
    while (true) {
      double dist;

      GetLHS(u, y);
      NewtonStep(q, b, u, y, dist); 

      if (dist >= c_maxDist) {
	accept = false;
	break;
      }
      
      decel = std::max(decel, sqrt(dist / c_maxDist) * m_maxDecel);
      if (iter >= 2) {
	double contr = dist / (disto + c_tol * c_eta);
	if (contr > c_maxContr) {
	  accept = false;
	  break;
	}
	decel = std::max(decel, sqrt(contr / c_maxContr) * m_maxDecel);
      }

      if (dist <= c_tol) {
	// Success; break out of iteration
	break;
      }
      disto = dist;
      iter++;
      if (iter > c_maxIter) {
	OnStep(x, true);
	if (newton) {
	  // Restore the place to restart if desired
	  x = restart;
	}
	return;
      }
    }

    if (!accept) {
      h /= m_maxDecel;   // PC not accepted; change stepsize and retry
      if (fabs(h) <= c_hmin) {
	OnStep(x, true);
	if (newton) {
	  // Restore the place to restart if desired
	  x = restart;
	}
	return;
      }

      continue;
    }

    // Determine new stepsize
    if (decel > m_maxDecel) {
      decel = m_maxDecel;
    }

    // Obtain the tangent at the next step
    q.GetRow(q.NumRows(), newT); 

    if (!newton &&
	Criterion(x, t) * Criterion(u, newT) < 0.0) {
      newton = true;
      restart = u;
    }

    if (newton) {
      // Newton-type steplength adaptation, secant method
      h *= -Criterion(u, newT) / (Criterion(u, newT) - Criterion(x, t));
    }
    else {
      // Standard steplength adaptation
      h = fabs(h / decel);
    }

    // PC step was successful; update and iterate
    x = u;
    OnStep(x, false);

    if (t * newT < 0.0) {
      // Bifurcation detected; for now, just "jump over" and continue,
      // taking into account the change in orientation of the curve.
      // Someday, we need to do more here!
      p_omega = -p_omega;
    }
    t = newT;
  }

  OnStep(x, true);
  if (newton) {
    x = restart;
  }
}

