//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Computation of quantal response equilibrium correspondence for
// normal form games.
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
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

#include <math.h>

#include "nfglogit.h"
#include "base/gnullstatus.h"
#include "math/gmath.h"
#include "math/gmatrix.h"
#include "math/gsmatrix.h"

//=========================================================================
//             QRE Correspondence Computation via Homotopy
//=========================================================================

//
// The following code implements a homotopy approach to computing
// the logistic QRE correspondence.  This implementation is a basic
// Euler-Newton approach with adaptive step size, based upon the
// ideas and codes presented in Allgower and Georg's
// _Numerical Continuation Methods_.
//

inline double sqr(double x) { return x*x; }

#if GBT_WITH_MP_FLOAT
inline double sqr(const gbtMPFloat &x) { return x*x; }
#endif  // GBT_WITH_MP_FLOAT

template <class T>
static void Givens(gbtMatrix<T> &b, gbtMatrix<T> &q,
		   T &c1, T &c2, int l1, int l2, int l3)
{
  if (fabs(c1) + fabs(c2) == 0.0) {
    return;
  }

  T sn;
  if (fabs(c2) >= fabs(c1)) {
    sn = sqrt(1.0 + sqr(c1/c2)) * fabs(c2);
  }
  else {
    sn = sqrt(1.0 + sqr(c2/c1)) * fabs(c1);
  }
  T s1 = c1/sn;
  T s2 = c2/sn;

  for (int k = 1; k <= q.NumColumns(); k++) {
    T sv1 = q(l1, k);
    T sv2 = q(l2, k);
    q(l1, k) = s1 * sv1 + s2 * sv2;
    q(l2, k) = -s2 * sv1 + s1 * sv2;
  }

  for (int k = l3; k <= b.NumColumns(); k++) {
    T sv1 = b(l1, k);
    T sv2 = b(l2, k);
    b(l1, k) = s1 * sv1 + s2 * sv2;
    b(l2, k) = -s2 * sv1 + s1 * sv2;
  }

  c1 = sn;
  c2 = 0.0;
}

template static void Givens(gbtMatrix<double> &, gbtMatrix<double> &,
			    double &, double &, int, int, int);
#if GBT_WITH_MP_FLOAT
template static void Givens(gbtMatrix<gbtMPFloat> &, gbtMatrix<gbtMPFloat> &,
			    gbtMPFloat &, gbtMPFloat &, int, int, int);
#endif  // GBT_WITH_MP_FLOAT


template <class T>
static void QRDecomp(gbtMatrix<T> &b, gbtMatrix<T> &q)
{
  q.MakeIdent();
  for (int m = 1; m <= b.NumColumns(); m++) {
    for (int k = m + 1; k <= b.NumRows(); k++) {
      Givens(b, q, b(m, m), b(k, m), m, k, m + 1);
    }
  }
}

template static void QRDecomp(gbtMatrix<double> &, gbtMatrix<double> &);
#if GBT_WITH_MP_FLOAT
template static void QRDecomp(gbtMatrix<gbtMPFloat> &, 
			      gbtMatrix<gbtMPFloat> &);
#endif  // GBT_WITH_MP_FLOAT

template <class T>
static void NewtonStep(gbtMatrix<T> &q, gbtMatrix<T> &b,
		       gbtVector<T> &u, gbtVector<T> &y,
		       T &d)
{
  for (int k = 1; k <= b.NumColumns(); k++) {
    for (int l = 1; l <= k - 1; l++) {
      y[k] -= b(l, k) * y[l];
    }
    y[k] /= b(k, k);
  }

  d = 0.0;
  for (int k = 1; k <= b.NumRows(); k++) {
    T s = 0.0;
    for (int l = 1; l <= b.NumColumns(); l++) {
      s += q(l, k) * y[l];
    }
    u[k] -= s;
    d += s * s;
  }
  d = sqrt(d);
}

template static void NewtonStep(gbtMatrix<double> &, gbtMatrix<double> &,
				gbtVector<double> &, gbtVector<double> &,
				double &);
#if GBT_WITH_MP_FLOAT
template static void NewtonStep(gbtMatrix<gbtMPFloat> &,
				gbtMatrix<gbtMPFloat> &,
				gbtVector<gbtMPFloat> &,
				gbtVector<gbtMPFloat> &,
				gbtMPFloat &);
#endif // GBT_WITH_MP_FLOAT 

template <class T>
static void QreLHS(const gbtNfgSupport &p_support,
		   const gbtVector<T> &p_point,
		   gbtVector<T> &p_lhs)
{
  gbtMixedProfile<T> profile(p_support);
  for (int i = 1; i <= profile.Length(); i++) {
    profile[i] = p_point[i];
  }
  T lambda = p_point[p_point.Length()];
  
  p_lhs = (T) 0.0;
  int rowno = 0;

  for (int pl = 1; pl <= p_support.GetGame().NumPlayers(); pl++) {
    rowno++;
    for (int st = 1; st <= p_support.NumStrats(pl); st++) {
      p_lhs[rowno] += profile(pl, st);
    }
    p_lhs[rowno] -= 1.0;

    for (int st = 2; st <= p_support.NumStrats(pl); st++) {
      p_lhs[++rowno] = log(profile(pl, st) / profile(pl, 1));
      p_lhs[rowno] -= (lambda * 
		       (profile.Payoff(pl, p_support.GetStrategy(pl, st)) -
			profile.Payoff(pl, p_support.GetStrategy(pl, 1))));
      p_lhs[rowno] *= profile(pl, 1) * profile(pl, st);
    }
  }
}

template static void QreLHS(const gbtNfgSupport &, const gbtVector<double> &,
			    gbtVector<double> &);
#if GBT_WITH_MP_FLOAT
template static void QreLHS(const gbtNfgSupport &, 
			    const gbtVector<gbtMPFloat> &,
			    gbtVector<gbtMPFloat> &);
#endif // GBT_WITH_MP_FLOAT

template <class T>
static void QreJacobian(const gbtNfgSupport &p_support,
			const gbtVector<T> &p_point,
			gbtMatrix<T> &p_matrix)
{
  gbtNfgGame nfg = p_support.GetGame();
  gbtMixedProfile<T> profile(p_support);
  for (int i = 1; i <= profile.Length(); i++) {
    profile[i] = p_point[i];
  }
  double lambda = p_point[p_point.Length()];

  int rowno = 0;
  for (int pl1 = 1; pl1 <= nfg.NumPlayers(); pl1++) {
    rowno++;
    // First, do the "sum to one" equation
    int colno = 0;
    for (int pl2 = 1; pl2 <= nfg.NumPlayers(); pl2++) {
      for (int st2 = 1; st2 <= p_support.NumStrats(pl2); st2++) {
	colno++;
	if (pl1 == pl2) {
	  p_matrix(colno, rowno) = 1.0;
	}
	else {
	  p_matrix(colno, rowno) = 0.0;
	}
      }
    }
    p_matrix(p_matrix.NumRows(), rowno) = 0.0;

    for (int st1 = 2; st1 <= p_support.NumStrats(pl1); st1++) {
      rowno++;
      int colno = 0;

      for (int pl2 = 1; pl2 <= nfg.NumPlayers(); pl2++) {
	for (int st2 = 1; st2 <= p_support.NumStrats(pl2); st2++) {
	  colno++;
	  if (pl1 == pl2) {
	    if (st2 == 1) {
	      p_matrix(colno, rowno) = -profile(pl1, st1);
	    }
	    else if (st1 == st2) {
	      p_matrix(colno, rowno) = profile(pl1, 1);
	    }
	    else {
	      p_matrix(colno, rowno) = 0.0;
	    }
	  }
	  else {
	    p_matrix(colno, rowno) = -lambda * profile(pl1, 1) * profile(pl1, st1) * (profile.Payoff(pl1, pl1, st1, pl2, st2) - profile.Payoff(pl1, pl1, 1, pl2, st2));
	  }
	}
      }

      p_matrix(p_matrix.NumRows(), rowno) = -profile(pl1, 1) * profile(pl1, st1) *
	(profile.Payoff(pl1, p_support.GetStrategy(pl1, st1)) -
	 profile.Payoff(pl1, p_support.GetStrategy(pl1, 1)));
    }
  }
}

template static void QreJacobian(const gbtNfgSupport &p_support,
				 const gbtVector<double> &p_point,
				 gbtMatrix<double> &p_matrix);
#if GBT_WITH_MP_FLOAT
template static void QreJacobian(const gbtNfgSupport &p_support,
				 const gbtVector<gbtMPFloat> &p_point,
				 gbtMatrix<gbtMPFloat> &p_matrix);
#endif // GBT_WITH_MP_FLOAT

//
// TracePath does the real work of tracing a branch of the correspondence
//
// Strategy:
// This is the standard simple PC continuation method outlined in
// Allgower & Georg, _Numerical Continuation Methods_.
// The only modification is to deal with the machine limitation
// in computing the log in the homotopy equations (in QreLHS() above).
// When the probability for a strategy drops below 10^-10, it is
// removed from the support, and TracePath() is called recursively from
// that point.  
//
// In a quantal response equilibrium (for finite lambda, at least),
// no strategy is ever truly played with zero probability, as this method
// approximates.  However, strictly inferior strategies are played
// with probability that is exponentially decreasing in lambda.  So,
// approximating these as zero gives a good approximation to the true
// equilibrium, at least to floating point precision.  If one is
// using this method to compute Nash equilibria, this should be harmless;
// if one is actually interested in the quantal response equilibria,
// then this should be happening at lambda that are so large that they
// will not be observed in lab data.
//
// The algorithm could be improved to approximate the exponential decay
// of dropped strategies explicitly, but, again, these probabilities are
// already going to display as zero on plots or in floating point output
// with standard precisions, so it isn't a priority.
//
// Really, the algorithm should check that omitted strategies remain
// truly inferior after they drop out; in practice, this is so unlikely
// to happen that the code should only be rewritten once a reasonable
// example of a strategy that drops out by this criterion winding up with
// positive probability.
//

template <class T>
static void TracePath(const gbtMixedProfile<T> &p_start,
		      T p_startLambda, T p_maxLambda, T p_omega,
		      gbtStatus &p_status,
		      gbtList<MixedSolution> &p_solutions)
{
  const int c_maxIters = 5000;     // maximum number of iterations
  const T c_tol = 1.0e-4;     // tolerance for corrector iteration
  const T c_maxDecel = 1.1;   // maximal deceleration factor
  const T c_maxDist = 0.4;    // maximal distance to curve
  const T c_maxContr = 0.6;   // maximal contraction rate in corrector
  const T c_eta = 0.1;        // perturbation to avoid cancellation
                                   // in calculating contraction rate
  T h = .03;                  // initial stepsize
  const T c_hmin = 1.0e-5;    // minimal stepsize

  gbtVector<T> x(p_start.Length() + 1), u(p_start.Length() + 1);
  for (int i = 1; i <= p_start.Length(); i++) {
    x[i] = p_start[i];
  }
  x[x.Length()] = p_startLambda;
  gbtVector<T> t(p_start.Length() + 1);
  gbtVector<T> y(p_start.Length());

  gbtMatrix<T> b(p_start.Length() + 1, p_start.Length());
  gbtSquareMatrix<T> q(p_start.Length() + 1);
  QreJacobian(p_start.Support(), x, b);
  QRDecomp(b, q);
  q.GetRow(q.NumRows(), t);
  
  int niters = 0;

  while (x[x.Length()] >= (T) 0.0 && x[x.Length()] < p_maxLambda) {
    p_status.Get();
    if (niters > c_maxIters) {
      // Give up
      return;
    }

    if (niters++ % 25 == 0) {
      p_status.SetProgress(x[x.Length()] / p_maxLambda,
			   gbtText("Lambda = ") + ToText(x[x.Length()]));
    }

    bool accept = true;

    if (fabs(h) <= c_hmin) {
      return;
    }

    // Predictor step
    for (int k = 1; k <= x.Length(); k++) {
      u[k] = x[k] + h * p_omega * t[k];
      if (k < x.Length() && u[k] < (T) 0.0) {
	accept = false;
	break;
      }
    }

    if (!accept) {
      h *= 0.5;
      continue;
    }

    T decel = 1.0 / c_maxDecel;  // initialize deceleration factor
    QreJacobian(p_start.Support(), u, b);
    QRDecomp(b, q);

    int iter = 1;
    T disto = 0.0;
    while (true) {
      T dist;

      QreLHS(p_start.Support(), u, y);
      NewtonStep(q, b, u, y, dist); 
      if (dist >= c_maxDist) {
	accept = false;
	break;
      }
      for (int i = 1; i < u.Length(); i++) {
	if (u[i] < (T) 0.0) {
	  // don't go negative
	  accept = false;
	  break;
	}
      }
      if (!accept) {
	break;
      }
      
      decel = gmax(decel, sqrt(dist / c_maxDist) * c_maxDecel);
      if (iter >= 2) {
	T contr = dist / (disto + c_tol * c_eta);
	if (contr > c_maxContr) {
	  accept = false;
	  break;
	}
	decel = gmax(decel, sqrt(contr / c_maxContr) * c_maxDecel);
      }

      if (dist <= c_tol) {
	// Success; break out of iteration
	break;
      }
      disto = dist;
      iter++;
    }

    if (!accept) {
      h /= c_maxDecel;   // PC not accepted; change stepsize and retry
      if (fabs(h) <= c_hmin) {
	return;
      }

      continue;
    }

    // Determine new stepsize
    if (decel > c_maxDecel) {
      decel = c_maxDecel;
    }
    h = fabs(h / decel);

    // PC step was successful; update and iterate
    for (int i = 1; i < x.Length(); i++) {
      if (u[i] < (T) 1.0e-10) {
	// Drop this strategy from the support, then recursively call
	// to continue tracing
	gbtNfgSupport newSupport(p_start.Support());
	int index = 1;
	for (int pl = 1; pl <= newSupport.GetGame().NumPlayers(); pl++) {
	  for (int st = 1; st <= newSupport.NumStrats(pl); st++) {
	    if (index++ == i) {
	      newSupport.RemoveStrategy(newSupport.GetStrategy(pl, st));
	    }
	  }
	}

	gbtMixedProfile<T> newProfile(newSupport);
	for (int j = 1; j <= newProfile.Length(); j++) {
	  if (j < i) {
	    newProfile[j] = u[j];
	  }
	  else if (j >= i) {
	    newProfile[j] = u[j+1];
	  }
	}

	TracePath(newProfile, u[u.Length()], p_maxLambda, p_omega,
		  p_status, p_solutions);
	return;
      }
      else {
	x[i] = u[i];
      }
    }

    x[x.Length()] = u[u.Length()];

    gbtMixedProfile<T> foo(p_start);
    for (int i = 1; i <= foo.Length(); i++) {
      foo[i] = x[i];
    }
    p_solutions.Append(MixedSolution(foo, "Qre[NFG]"));
    p_solutions[p_solutions.Length()].SetQre((double) x[x.Last()], 0);
    
    gbtVector<T> newT(t);
    q.GetRow(q.NumRows(), newT);  // new tangent
    if (t * newT < (T) 0.0) {
      // Bifurcation detected; for now, just "jump over" and continue,
      // taking into account the change in orientation of the curve.
      // Someday, we need to do more here! :)
      p_omega = -p_omega;
    }
    t = newT;
  }
}

template static void TracePath(const gbtMixedProfile<double> &p_start,
			       double p_startLambda, double p_maxLambda, 
			       double p_omega,
			       gbtStatus &p_status,
			       gbtList<MixedSolution> &p_solutions);
#if GBT_WITH_MP_FLOAT
template static void TracePath(const gbtMixedProfile<gbtMPFloat> &p_start,
			       gbtMPFloat p_startLambda,
			       gbtMPFloat p_maxLambda, 
			       gbtMPFloat p_omega,
			       gbtStatus &p_status,
			       gbtList<MixedSolution> &p_solutions);
#endif // GBT_WITH_MP_FLOAT

gbtNfgNashLogit::gbtNfgNashLogit(void)
  : m_maxLam(30.0), m_stepSize(0.0001), m_fullGraph(false)
{ }

gbtList<MixedSolution> gbtNfgNashLogit::Solve(const gbtNfgSupport &p_support,
					      gbtStatus &p_status)
{
  gbtList<MixedSolution> solutions;

#if GBT_WITH_MP_FLOAT
  gbtMixedProfile<gbtMPFloat> start(p_support);
#else
  gbtMixedProfile<double> start(p_support);
#endif  // GBT_WITH_MP_FLOAT

  try {
#if GBT_WITH_MP_FLOAT
    TracePath(start, gbtMPFloat(0.0), gbtMPFloat(m_maxLam), 
	      gbtMPFloat(1.0), p_status, solutions);
#else
    TracePath(start, 0.0, m_maxLam, 1.0, p_status, solutions);
#endif // GBT_WITH_MP_FLOAT
  }
  catch (...) { }

  if (!m_fullGraph) { 
    while (solutions.Length() > 1) {
      solutions.Remove(1);
    }
  }

  return solutions;
}
