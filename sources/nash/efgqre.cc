//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Trace a branch of the agent QRE correspondence
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
#include "base/gnullstatus.h"
#include "math/gmath.h"
#include "math/gsmatrix.h"
#include "efgqre.h"

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
// Many of these functions are duplicated in efgqre.cc and nfgqre.cc.
// This should be fixed in future!
//

inline double sqr(double x) { return x*x; }

static void Givens(gMatrix<double> &b, gMatrix<double> &q,
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

static void QRDecomp(gMatrix<double> &b, gMatrix<double> &q)
{
  q.MakeIdent();
  for (int m = 1; m <= b.NumColumns(); m++) {
    for (int k = m + 1; k <= b.NumRows(); k++) {
      Givens(b, q, b(m, m), b(k, m), m, k, m + 1);
    }
  }
}

static void NewtonStep(gMatrix<double> &q, gMatrix<double> &b,
		       gVector<double> &u, gVector<double> &y,
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

static void QreLHS(const EFSupport &p_support, const gVector<double> &p_point,
		   gVector<double> &p_lhs)
{
  BehavProfile<double> profile(p_support);
  for (int i = 1; i <= profile.Length(); i++) {
    profile[i] = p_point[i];
  }
  double lambda = p_point[p_point.Length()];

  p_lhs = 0.0;
  int rowno = 0;

  for (int pl = 1; pl <= p_support.GetGame().NumPlayers(); pl++) {
    EFPlayer *player = p_support.GetGame().Players()[pl];
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      rowno++;
      for (int act = 1; act <= p_support.NumActions(pl, iset); act++) {
	p_lhs[rowno] += profile(pl, iset, act);
      }
      p_lhs[rowno] -= 1.0;

      for (int act = 2; act <= p_support.NumActions(pl, iset); act++) {
	p_lhs[++rowno] = log(profile(pl, iset, act) / profile(pl, iset, 1));
	p_lhs[rowno] -= (lambda *
			 (profile.GetActionValue(p_support.Actions(pl, iset)[act]) -
			  profile.GetActionValue(p_support.Actions(pl, iset)[1])));
	p_lhs[rowno] *= profile(pl, iset, 1) * profile(pl, iset, act);
      }
    }
  }
}

static void QreJacobian(const EFSupport &p_support,
			const gVector<double> &p_point,
			gMatrix<double> &p_matrix)
{
  const efgGame &efg = p_support.GetGame();
  BehavProfile<double> profile(p_support);
  for (int i = 1; i <= profile.Length(); i++) {
    profile[i] = p_point[i];
  }
  double lambda = p_point[p_point.Length()];

  int rowno = 0; 
  for (int pl1 = 1; pl1 <= efg.NumPlayers(); pl1++) {
    EFPlayer *player1 = efg.Players()[pl1];
    for (int iset1 = 1; iset1 <= player1->NumInfosets(); iset1++) {
      Infoset *infoset1 = player1->Infosets()[iset1];
      rowno++;
      // First, do the "sum to one" equation
      int colno = 0;
      for (int pl2 = 1; pl2 <= efg.NumPlayers(); pl2++) {
	EFPlayer *player2 = efg.Players()[pl2];
	for (int iset2 = 1; iset2 <= player2->NumInfosets(); iset2++) {
	  for (int act2 = 1; act2 <= p_support.NumActions(pl2, iset2); act2++) {
	    colno++;
	    if (pl1 == pl2 && iset1 == iset2) {
	      p_matrix(colno, rowno) = 1.0;
	    }
	    else {
	      p_matrix(colno, rowno) = 0.0;
	    }
	  }
	}
      }
      p_matrix(p_matrix.NumRows(), rowno) = 0.0;
					    
      for (int act1 = 2; act1 <= p_support.NumActions(pl1, iset1); act1++) {
	rowno++;
	int colno = 0;

	for (int pl2 = 1; pl2 <= efg.NumPlayers(); pl2++) {
	  EFPlayer *player2 = efg.Players()[pl2];
	  for (int iset2 = 1; iset2 <= player2->NumInfosets(); iset2++) {
	    Infoset *infoset2 = player2->Infosets()[iset2];

	    for (int act2 = 1; act2 <= p_support.NumActions(pl2, iset2); act2++) {
	      colno++;
	      if (infoset1 == infoset2) {
		if (act2 == 1) {
		  p_matrix(colno, rowno) = -profile(pl1, iset1, act1);
		}
		else if (act1 == act2) {
		  p_matrix(colno, rowno) = profile(pl1, iset1, 1);
		}
		else {
		  p_matrix(colno, rowno) = 0.0;
		}
	      }
	      else {   // infoset1 != infoset2
		p_matrix(colno, rowno) = -lambda * profile(pl1, iset1, 1) * profile(pl1, iset1, act1) * (profile.DiffActionValue(p_support.Actions(pl1, iset1)[act1], p_support.Actions(pl2, iset2)[act2]) - profile.DiffActionValue(p_support.Actions(pl1, iset1)[1], p_support.Actions(pl2, iset2)[act2]));
	      }
	    }
	  }
	}

	p_matrix(p_matrix.NumRows(), rowno) = -profile(pl1, iset1, 1) * profile(pl1, iset1, act1) * (profile.GetActionValue(p_support.Actions(pl1, iset1)[act1]) - profile.GetActionValue(p_support.Actions(pl1, iset1)[1]));
      }
    }
  }
}

static void TracePath(const BehavProfile<double> &p_start,
		      double p_startLambda, double p_maxLambda, double p_omega,
		      gStatus &p_status,
		      gList<BehavSolution> &p_solutions)
{
  const double c_tol = 1.0e-4;     // tolerance for corrector iteration
  const double c_maxDecel = 1.1;   // maximal deceleration factor
  const double c_maxDist = 0.4;    // maximal distance to curve
  const double c_maxContr = 0.6;   // maximal contraction rate in corrector
  const double c_eta = 0.1;        // perturbation to avoid cancellation
                                   // in calculating contraction rate
  double h = .03;                  // initial stepsize
  const double c_hmin = 1.0e-5;    // minimal stepsize

  gVector<double> x(p_start.Length() + 1), u(p_start.Length() + 1);
  for (int i = 1; i <= p_start.Length(); i++) {
    x[i] = p_start[i];
  }
  x[x.Length()] = p_startLambda;
  gVector<double> t(p_start.Length() + 1);
  gVector<double> y(p_start.Length());

  gMatrix<double> b(p_start.Length() + 1, p_start.Length());
  gSquareMatrix<double> q(p_start.Length() + 1);
  QreJacobian(p_start.Support(), x, b);
  QRDecomp(b, q);
  q.GetRow(q.NumRows(), t);
  
  int niters = 0;

  while (x[x.Length()] >= 0.0 && x[x.Length()] < p_maxLambda) {
    if (niters++ % 25 == 0) {
      p_status.Get();
      p_status.SetProgress(x[x.Length()] / p_maxLambda,
			   gText("Lambda = ") + ToText(x[x.Length()]));
    }

    bool accept = true;

    if (fabs(h) <= c_hmin) {
      return;
    }

    // Predictor step
    for (int k = 1; k <= x.Length(); k++) {
      u[k] = x[k] + h * p_omega * t[k];
      if (k < x.Length() && u[k] < 0.0) {
	accept = false;
	break;
      }
    }

    if (!accept) {
      h *= 0.5;
      continue;
    }

    double decel = 1.0 / c_maxDecel;  // initialize deceleration factor
    QreJacobian(p_start.Support(), u, b);
    QRDecomp(b, q);

    int iter = 1;
    double disto = 0.0;
    while (true) {
      double dist;

      QreLHS(p_start.Support(), u, y);
      NewtonStep(q, b, u, y, dist); 
      if (dist >= c_maxDist) {
	accept = false;
	break;
      }
      for (int i = 1; i < u.Length(); i++) {
	if (u[i] < 0.0) {
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
	double contr = dist / (disto + c_tol * c_eta);
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
      if (u[i] < 1.0e-10) {
	// Drop this strategy from the support, then recursively call
	// to continue tracing
	EFSupport newSupport(p_start.Support());
	int index = 1;
	for (int pl = 1; pl <= newSupport.GetGame().NumPlayers(); pl++) {
	  EFPlayer *player = newSupport.GetGame().Players()[pl];
	  for (int iset = 1; iset <= player->NumInfosets(); iset++) {
	    for (int act = 1; act <= newSupport.NumActions(pl, iset); act++) {
	      if (index++ == i) {
		newSupport.RemoveAction(newSupport.Actions(pl, iset)[act]);
	      }
	    }
	  }
	}

	BehavProfile<double> newProfile(newSupport);
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

    BehavProfile<double> foo(p_start);
    for (int i = 1; i <= foo.Length(); i++) {
      foo[i] = x[i];
    }
    p_solutions.Append(BehavSolution(foo, algorithmEfg_QRE_EFG));
    p_solutions[p_solutions.Length()].SetQre(x[x.Last()], 0);
    
    gVector<double> newT(t);
    q.GetRow(q.NumRows(), newT);  // new tangent
    if (t * newT < 0.0) {
      // Bifurcation detected; for now, just "jump over" and continue,
      // taking into account the change in orientation of the curve.
      // Someday, we need to do more here! :)
      p_omega = -p_omega;
    }
    t = newT;
  }
}

efgQre::efgQre(void)
  : m_maxLam(30.0), m_stepSize(0.0001), m_fullGraph(false)
{ }

gList<BehavSolution> efgQre::Solve(const EFSupport &p_support,
				   gStatus &p_status)
{
  gList<BehavSolution> solutions;
  BehavProfile<double> start(p_support);

  try {
    TracePath(start, 0.0, m_maxLam, 1.0, p_status, solutions);
  }
  catch (...) { }

  if (!m_fullGraph) { 
    while (solutions.Length() > 1) {
      solutions.Remove(1);
    }
  }

  return solutions;
}

