//
// $Source$
// $Date: 2006-12-15 13:29:38 -0600 (Fri, 15 Dec 2006) $
// $Revision: 6228 $
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

#include <unistd.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <libgambit/libgambit.h>
#include <libgambit/sqmatrix.h>

using namespace Gambit;


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


void QreLHS(const StrategySupportProfile &p_support, 
	    const Vector<double> &p_point,
	    Vector<double> &p_lhs)
{
  MixedStrategyProfile<double> profile(p_support), logprofile(p_support);
  for (int i = 1; i <= profile.Length(); i++) {
    profile[i] = exp(p_point[i]);
    logprofile[i] = p_point[i];
  }
  double lambda = p_point[p_point.Length()];
  
  p_lhs = 0.0;

  int rowno = 0;
  for (int pl = 1; pl <= p_support.GetGame()->NumPlayers(); pl++) {
    GamePlayer player = p_support.GetGame()->GetPlayer(pl);
    for (int st = 1; st <= player->NumStrategies(); st++) {
      rowno++;
      if (st == 1) {
	// should be st==lead: sum-to-one equation
	p_lhs[rowno] = -1.0;
	for (int j = 1; j <= player->NumStrategies(); j++) {
	  p_lhs[rowno] += profile[player->GetStrategy(j)];
	}
      }
      else {
	p_lhs[rowno] = (logprofile[player->GetStrategy(st)] - 
			logprofile[player->GetStrategy(1)] -
			lambda * (profile.GetStrategyValue(player->GetStrategy(st)) -
				  profile.GetStrategyValue(player->GetStrategy(1))));

      }
    }
  }
}

void SymmetricQreLHS(const StrategySupportProfile &p_support, 
		     const Vector<double> &p_point,
		     Vector<double> &p_lhs)
{
  MixedStrategyProfile<double> profile(p_support), logprofile(p_support);
  for (int pl = 1; pl <= p_support.GetGame()->NumPlayers(); pl++) {
    for (int i = 1; i < p_point.Length(); i++) {
      profile[(pl-1)*(p_point.Length()-1)+i] = exp(p_point[i]);
      logprofile[(pl-1)*(p_point.Length()-1)+i] = p_point[i];
    }
  }
  double lambda = p_point[p_point.Length()];
  
  p_lhs = 0.0;

  int rowno = 0;
  GamePlayer player = p_support.GetGame()->GetPlayer(1);
  for (int st = 1; st <= player->NumStrategies(); st++) {
    rowno++;
    if (st == 1) {
      // should be st==lead: sum-to-one equation
      p_lhs[rowno] = -1.0;
      for (int j = 1; j <= player->NumStrategies(); j++) {
	p_lhs[rowno] += profile[player->GetStrategy(j)];
      }
    }
    else {
      p_lhs[rowno] = (logprofile[player->GetStrategy(st)] - 
		      logprofile[player->GetStrategy(1)] -
		      lambda * (profile.GetStrategyValue(player->GetStrategy(st)) -
				profile.GetStrategyValue(player->GetStrategy(1))));
    }
  }
}

void QreJacobian(const StrategySupportProfile &p_support,
		 const Vector<double> &p_point,
		 Matrix<double> &p_matrix)
{
  MixedStrategyProfile<double> profile(p_support), logprofile(p_support);
  for (int i = 1; i <= profile.Length(); i++) {
    profile[i] = exp(p_point[i]);
    logprofile[i] = p_point[i];
  }
  double lambda = p_point[p_point.Length()];

  p_matrix = 0.0;

  int rowno = 0;
  for (int i = 1; i <= p_support.GetGame()->NumPlayers(); i++) {
    GamePlayer player = p_support.GetGame()->GetPlayer(i);

    for (int j = 1; j <= player->NumStrategies(); j++) {
      rowno++;
      if (j == 1) {
	// Should be j == lead: sum-to-one equation
	
	int colno = 0;
	for (int ell = 1; ell <= p_support.GetGame()->NumPlayers(); ell++) {
	  GamePlayer player2 = p_support.GetGame()->GetPlayer(ell);

	  for (int m = 1; m <= player2->NumStrategies(); m++) {
	    colno++;
	    
	    if (i == ell) {
	      p_matrix(colno, rowno) = profile[player2->GetStrategy(m)];
	    }
	    else {
	      p_matrix(colno, rowno) = 0.0;
	    }
	  }
	}
	
	// Derivative wrt lambda is zero
	p_matrix(p_matrix.NumRows(), rowno) = 0.0;
      }
      else {
	// This is a ratio equation

	int colno = 0;
	for (int ell = 1; ell <= p_support.GetGame()->NumPlayers(); ell++) {
	  GamePlayer player2 = p_support.GetGame()->GetPlayer(ell);

	  for (int m = 1; m <= player2->NumStrategies(); m++) {
	    colno++;

	    if (i == ell) {
	      if (m == 1) {
		// should be m==lead
		p_matrix(colno, rowno) = -1.0;
	      }
	      else if (m == j) {
		p_matrix(colno, rowno) = 1.0;
	      }
	      else {
		p_matrix(colno, rowno) = 0.0;
	      }
	    }
	    else {
	      // 1 == lead
	      p_matrix(colno, rowno) =
		-lambda * profile[player2->GetStrategy(m)] *
		(profile.GetPayoffDeriv(i, 
					p_support.GetStrategy(i, j),
					p_support.GetStrategy(ell, m)) -
		 profile.GetPayoffDeriv(i, 
					p_support.GetStrategy(i, 1),
					p_support.GetStrategy(ell, m)));
	    }
	  }

	}
	
	// column wrt lambda
	// 1 == lead
	p_matrix(p_matrix.NumRows(), rowno) =
	  (profile.GetStrategyValue(p_support.GetStrategy(i, 1)) - 
	   profile.GetStrategyValue(p_support.GetStrategy(i, j)));
      }
    }
  }

  /*
  std::cout << "Jac:\n";
  for (int i = 1; i <= p_matrix.NumColumns(); i++) {
    for (int j = 1; j <= p_matrix.NumRows(); j++) {
      std::cout << p_matrix(j, i) << " ";
    }
    std::cout << std::endl;
  }
  */
}

void SymmetricQreJacobian(const StrategySupportProfile &p_support,
			  const Vector<double> &p_point,
			  Matrix<double> &p_matrix)
{
  MixedStrategyProfile<double> profile(p_support), logprofile(p_support);
  for (int pl = 1; pl <= p_support.GetGame()->NumPlayers(); pl++) {
    for (int i = 1; i < p_point.Length(); i++) {
      profile[(pl-1)*(p_point.Length()-1)+i] = exp(p_point[i]);
      logprofile[(pl-1)*(p_point.Length()-1)+i] = p_point[i];
    }
  }

  double lambda = p_point[p_point.Length()];
  int N = p_support.GetGame()->NumPlayers();

  p_matrix = 0.0;

  int rowno = 0;
  GamePlayer player = p_support.GetGame()->GetPlayer(1);

  for (int j = 1; j <= player->NumStrategies(); j++) {
    rowno++;
    if (j == 1) {
      // Should be j == lead: sum-to-one equation
	
      int colno = 0;
      GamePlayer player2 = p_support.GetGame()->GetPlayer(2);

      for (int m = 1; m <= player2->NumStrategies(); m++) {
	colno++;
	  
	p_matrix(colno, rowno) = profile[player2->GetStrategy(m)];
      }
      
      // Derivative wrt lambda is zero
      p_matrix(p_matrix.NumRows(), rowno) = 0.0;
    }
    else {
      // This is a ratio equation
      
      int colno = 0;
      GamePlayer player2 = p_support.GetGame()->GetPlayer(2);
	
      for (int m = 1; m <= player2->NumStrategies(); m++) {
	colno++;
	 
	p_matrix(colno, rowno) = 
	  -(N-1) * lambda * profile[player2->GetStrategy(m)] *
	  (profile.GetPayoffDeriv(1, 
				  p_support.GetStrategy(1, j),
				  p_support.GetStrategy(2, m)) -
	   profile.GetPayoffDeriv(1, 
				  p_support.GetStrategy(1, 1),
				  p_support.GetStrategy(2, m)));
      
	if (m == 1) {
	  p_matrix(colno, rowno) -= 1.0;
	}
	else if (m == j) {
	  p_matrix(colno, rowno) += 1.0;
	}
      }
	
      // column wrt lambda
      // 1 == lead
      p_matrix(p_matrix.NumRows(), rowno) =
	(profile.GetStrategyValue(p_support.GetStrategy(1, 1)) - 
	 profile.GetStrategyValue(p_support.GetStrategy(1, j)));
    }
  }

  /*
  std::cout << "Jac:\n";
  for (int i = 1; i <= p_matrix.NumColumns(); i++) {
    for (int j = 1; j <= p_matrix.NumRows(); j++) {
      std::cout << p_matrix(j, i) << " ";
    }
    std::cout << std::endl;
  }
  */
}

//
// For maximum likelihood estimation
//
extern bool g_maxLike;
extern Array<double> g_obsProbs;

extern double g_targetLambda;

double LogLike(const Array<double> &p_point)
{
  double ret = 0.0;
  
  for (int i = 1; i <= g_obsProbs.Length(); i++) {
    ret += g_obsProbs[i] * log(p_point[i]);
  }

  return ret;
}

double DiffLogLike(const Array<double> &p_point,
		   const Array<double> &p_tangent)
{
  double ret = 0.0;

  for (int i = 1; i <= g_obsProbs.Length(); i++) {
    ret += g_obsProbs[i] * p_tangent[i];
  }

  return ret;
}


extern int g_numDecimals;

void PrintProfile(std::ostream &p_stream,
		  const StrategySupportProfile &p_support, const Vector<double> &x,
		  bool p_terminal = false)
{
  p_stream.setf(std::ios::fixed);
  // By convention, we output lambda first
  if (!p_terminal) {
    p_stream << std::setprecision(g_numDecimals) << x[x.Length()];
  }
  else {
    p_stream << "NE";
  }
  p_stream.unsetf(std::ios::fixed);

  for (int i = 1; i <  x.Length(); i++) {
    p_stream << "," << std::setprecision(g_numDecimals) << exp(x[i]);
  }

  if (g_maxLike) {
    MixedStrategyProfile<double> profile(p_support);
    for (int i = 1; i <= profile.Length(); i++) {
      profile[i] = exp(x[i]);
    }

    p_stream.setf(std::ios::fixed);
    p_stream << "," << std::setprecision(g_numDecimals) << LogLike(profile);
    p_stream.unsetf(std::ios::fixed);
  }

  p_stream << std::endl;
}

//
// TracePath does the real work of tracing a branch of the correspondence
//
// Strategy:
// This is the standard simple PC continuation method outlined in
// Allgower & Georg, _Numerical Continuation Methods_.
//
// The only novelty is the handling of the representation of the
// probabilities.  We deal with a correspondence in which probabilities
// often tend to zero exponentially in the lambda parameter.  However,
// negative probabilities make no sense, and cause the defining equations
// to be ill-defined.  This suggests that representing the probabilities
// as logarithms is indicated.  However, experience is that this does
// not work well when lambda is small, as the change in the probabilities
// as lambda changes in this region is closer to linear than exponential.
//
// The compromise is this: we represent probabilities below a certain
// cutoff (here set to .05) as logarithms, and probabilities above that
// as the actual probability.  Thus, we can take advantage of the
// exponential decay of small probabilities.
//


extern double g_maxDecel;
extern double g_hStart;
extern bool g_fullGraph;

void 
TraceStrategicPath(const MixedStrategyProfile<double> &p_start,
		   double p_startLambda, double p_maxLambda, double p_omega,
		   bool p_symmetric = false)
{
  const double c_tol = 1.0e-4;     // tolerance for corrector iteration
  const double c_maxDist = 0.4;    // maximal distance to curve
  const double c_maxContr = 0.6;   // maximal contraction rate in corrector
  const double c_eta = 0.1;        // perturbation to avoid cancellation
                                   // in calculating contraction rate
  double h = g_hStart;             // initial stepsize
  const double c_hmin = 1.0e-8;    // minimal stepsize

  bool newton = false;          // using Newton steplength (for MLE)
  bool restarting = false;      // flag for first restart step after MLE

  int profileLength = p_start.Length();
  if (p_symmetric) {
    profileLength = p_start.Length() / p_start.GetGame()->NumPlayers();
  }

  // When doing MLE finding, we push the data from the original path-following
  // here, and resume once we've found the local extremum.
  Vector<double> pushX(profileLength + 1);
  double pushH = h;

  Vector<double> x(profileLength + 1), u(profileLength + 1);
  for (int i = 1; i <= profileLength; i++) {
    x[i] = log(p_start[i]);
  }
  x[x.Length()] = p_startLambda;

  if (g_fullGraph) {
    PrintProfile(std::cout, p_start.GetSupport(), x);
  }

  Vector<double> t(profileLength + 1);
  Vector<double> y(profileLength);

  Matrix<double> b(profileLength + 1, profileLength);
  SquareMatrix<double> q(profileLength + 1);
  if (p_symmetric) {
    SymmetricQreJacobian(p_start.GetSupport(), x, b);
  }
  else {
    QreJacobian(p_start.GetSupport(), x, b);
  }
  QRDecomp(b, q);
  q.GetRow(q.NumRows(), t);
  
  while (x[x.Length()] >= 0.0 && x[x.Length()] < p_maxLambda) {
    bool accept = true;

    if (fabs(h) <= c_hmin) {
      // Stop.  If this occurs because we are in MLE-finding mode,
      // resume tracing the original branch
      if (newton && g_maxLike) {
	//printf("popping! %f\n", pushH);
	x = pushX;
	h = pushH;
	if (p_symmetric) {
	  SymmetricQreJacobian(p_start.GetSupport(), x, b);
	}
	else {
	  QreJacobian(p_start.GetSupport(), x, b);
	}
	QRDecomp(b, q);
	q.GetRow(q.NumRows(), t);
	newton = false;
	restarting = true;
       	continue;
      }
      else {
	// We're really done.
	return;
      }
    }

    // Predictor step
    for (int k = 1; k <= x.Length(); k++) {
      u[k] = x[k] + h * p_omega * t[k];
    }

    double decel = 1.0 / g_maxDecel;  // initialize deceleration factor
    if (p_symmetric) {
      SymmetricQreJacobian(p_start.GetSupport(), u, b);
    }
    else {
      QreJacobian(p_start.GetSupport(), u, b);
    }
    QRDecomp(b, q);

    int iter = 1;
    double disto = 0.0;
    while (true) {
      double dist;

      if (p_symmetric) {
	SymmetricQreLHS(p_start.GetSupport(), u, y);
      }
      else {
	QreLHS(p_start.GetSupport(), u, y);
      }
      NewtonStep(q, b, u, y, dist); 

      if (dist >= c_maxDist) {
	accept = false;
	break;
      }
      
      decel = max(decel, sqrt(dist / c_maxDist) * g_maxDecel);
      if (iter >= 2) {
	double contr = dist / (disto + c_tol * c_eta);
	if (contr > c_maxContr) {
	  accept = false;
	  break;
	}
	decel = max(decel, sqrt(contr / c_maxContr) * g_maxDecel);
      }

      if (dist <= c_tol) {
	// Success; break out of iteration
	break;
      }
      disto = dist;
      iter++;
    }

    if (!accept) {
      h /= g_maxDecel;   // PC not accepted; change stepsize and retry
      if (fabs(h) <= c_hmin) {
	// Stop.  If this occurs because we are in MLE-finding mode,
	// resume tracing the original branch
	if (newton && g_maxLike) {
	  //printf("popping! %f\n", pushH);
	  x = pushX;
	  h = pushH;
	  newton = false;
	  restarting = true;
	  if (p_symmetric) {
	    SymmetricQreJacobian(p_start.GetSupport(), x, b);
	  }
	  else {
	    QreJacobian(p_start.GetSupport(), x, b);
	  }
	  QRDecomp(b, q);
	  q.GetRow(q.NumRows(), t);
	  continue;
	}
	else {
	  // We're really done.
	  return;
	}
      }

      continue;
    }

    // Determine new stepsize
    if (decel > g_maxDecel) {
      decel = g_maxDecel;
    }

    if (!newton && g_maxLike) {
      // Currently, 't' is the tangent at 'x'.  We also need the
      // tangent at 'u'.
      Vector<double> newT(t);
      q.GetRow(q.NumRows(), newT); 

      if (!restarting && 
	  DiffLogLike(x, t) * DiffLogLike(u, newT) < 0.0) {
	// Store the current state, to resume later
	pushX = x;
	pushH = h;
	newton = true;
	//printf("entering newton mode\n");
      }
    }
    else if (!newton && g_targetLambda > 0.0) {
      if (!restarting &&
	  ((x[x.Length()] - g_targetLambda) *
	   (u[u.Length()] - g_targetLambda)) < 0.0) {
	// Store the current state, to resume later
	pushX = x;
	pushH = h;
	newton = true;
	//printf("entering newton mode\n");
      }
    }

    if (newton && g_maxLike) {
      // Newton-type steplength adaptation, secant method
      Vector<double> newT(t);
      q.GetRow(q.NumRows(), newT); 

      h *= -DiffLogLike(u, newT) / (DiffLogLike(u, newT) - 
				    DiffLogLike(x, t));
    }
    else if (newton && g_targetLambda > 0.0) {
      h *= -(u[u.Length()] - g_targetLambda) / (u[u.Length()] - x[x.Length()]);
    }
    else {
      // Standard steplength adaptation
      h = fabs(h / decel);
    }

    restarting = false;

    // PC step was successful; update and iterate
    x = u;

    if (g_fullGraph) {
      PrintProfile(std::cout, p_start.GetSupport(), x);
    }
    

    Vector<double> newT(t);
    q.GetRow(q.NumRows(), newT);  // new tangent
    if (t * newT < 0.0) {
      printf("Bifurcation! at %f\n", x[x.Length()]);
      // Bifurcation detected; for now, just "jump over" and continue,
      // taking into account the change in orientation of the curve.
      // Someday, we need to do more here! :)
      p_omega = -p_omega;
    }
    t = newT;

  }

  if (!g_fullGraph) {
    PrintProfile(std::cout, p_start.GetSupport(), x, true);
  }
}



