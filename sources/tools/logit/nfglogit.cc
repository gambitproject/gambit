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

#include <unistd.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include "libgambit/libgambit.h"

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

static void Givens(gbtMatrix<double> &b, gbtMatrix<double> &q,
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

static void QRDecomp(gbtMatrix<double> &b, gbtMatrix<double> &q)
{
  q.MakeIdent();
  for (int m = 1; m <= b.NumColumns(); m++) {
    for (int k = m + 1; k <= b.NumRows(); k++) {
      Givens(b, q, b(m, m), b(k, m), m, k, m + 1);
    }
  }
}

static void NewtonStep(gbtMatrix<double> &q, gbtMatrix<double> &b,
		       gbtVector<double> &u, gbtVector<double> &y,
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

static void QreLHS(const gbtNfgSupport &p_support, 
		   const gbtVector<double> &p_point,
		   gbtVector<double> &p_lhs)
{
  gbtMixedProfile<double> profile(p_support);
  for (int i = 1; i <= profile.Length(); i++) {
    profile[i] = p_point[i];
  }
  double lambda = p_point[p_point.Length()];
  
  p_lhs = 0.0;
  int rowno = 0;

  for (int pl = 1; pl <= p_support.GetGame()->NumPlayers(); pl++) {
    rowno++;
    for (int st = 1; st <= p_support.NumStrats(pl); st++) {
      p_lhs[rowno] += profile(pl, st);
    }
    p_lhs[rowno] -= 1.0;

    for (int st = 2; st <= p_support.NumStrats(pl); st++) {
      p_lhs[++rowno] = log(profile(pl, st) / profile(pl, 1));
      p_lhs[rowno] -= (lambda * 
		       (profile.GetPayoff(pl, pl, st) -
			profile.GetPayoff(pl, pl, 1)));
      p_lhs[rowno] *= profile(pl, 1) * profile(pl, st);
    }
  }
}

static void QreJacobian(const gbtNfgSupport &p_support,
			const gbtVector<double> &p_point,
			gbtMatrix<double> &p_matrix)
{
  const gbtNfgGame &nfg = *p_support.GetGame();
  gbtMixedProfile<double> profile(p_support);
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
	    p_matrix(colno, rowno) = -lambda * profile(pl1, 1) * profile(pl1, st1) * (profile.GetPayoff(pl1, pl1, st1, pl2, st2) - profile.GetPayoff(pl1, pl1, 1, pl2, st2));
	  }
	}
      }

      p_matrix(p_matrix.NumRows(), rowno) = -profile(pl1, 1) * profile(pl1, st1) * (profile.GetPayoff(pl1, pl1, st1) - profile.GetPayoff(pl1, pl1, 1));
    }
  }
}

//
// For maximum likelihood estimation
//
bool g_maxLike = false;
gbtArray<double> g_obsProbs;

double LogLike(const gbtMixedProfile<double> &p_point)
{
  double ret = 0.0;
  
  for (int i = 1; i <= g_obsProbs.Length(); i++) {
    ret += ((double) g_obsProbs[i]) * log(p_point[i]);
  }

  return ret;
}

double DiffLogLike(const gbtMixedProfile<double> &p_point,
		   const gbtArray<double> &p_tangent)
{
  double ret = 0.0;

  int i = 1, index = 1;

  for (int pl = 1; pl <= p_point.GetGame()->NumPlayers(); pl++) {
    gbtNfgPlayer *player = p_point.GetGame()->GetPlayer(pl);

    int maxstrat = 1;
    for (int st = 2; st <= player->NumStrats(); st++) {
      if (p_point(pl, st) > p_point(pl, maxstrat)) {
	maxstrat = st;
      }
    }

    for (int st = 1; st <= player->NumStrats(); st++) {
      if (p_point(pl, st) < 1.0e-10) {
	// Approximation mode; this isn't in the tangent vector
	ret += ((double) g_obsProbs[i++]) * 
	  (p_point.GetPayoff(pl, pl, st) -
	   p_point.GetPayoff(pl, pl, maxstrat));
      }
      else {
	// This does have an entry in the tangent vector
	ret += ((double) g_obsProbs[i]) * p_tangent[index] / p_point[i];
	i++;
	index++;
      }
    }
  }

  return ret;
}


int g_numDecimals = 6;

gbtMixedProfile<double> 
AdjustZeroes(const gbtVector<double> &p_x, 
	     const gbtNfgSupport &p_support,
	     double p_lambda)
{
  gbtMixedProfile<double> profile(p_support.GetGame());

  int index = 1;
  for (int pl = 1; pl <= p_support.GetGame()->NumPlayers(); pl++) {
    gbtNfgPlayer *player = p_support.GetGame()->GetPlayer(pl);
    for (int st = 1; st <= player->NumStrats(); st++) {
      if (p_support.Contains(player->GetStrategy(st))) {
	profile(pl, st) = p_x[index++];
      }
      else {
	profile(pl, st) = 0.0;
      }
    }
  }

  for (int pl = 1; pl <= p_support.GetGame()->NumPlayers(); pl++) {
    gbtNfgPlayer *player = p_support.GetGame()->GetPlayer(pl);
    int maxstrat = 1;
    for (int st = 2; st <= player->NumStrats(); st++) {
      if (profile(pl, st) > profile(pl, maxstrat)) {
	maxstrat = st;
      }
    }

    for (int st = 1; st <= player->NumStrats(); st++) {
      if (profile(pl, st) < 1.0e-10) {
	profile(pl, st) = (profile(pl, maxstrat) *
			   exp(p_lambda * 
			       (profile.GetPayoff(pl, pl, st) -
				profile.GetPayoff(pl, pl, maxstrat))));
      }
    }
  }

  return profile;
}

void PrintProfile(std::ostream &p_stream,
		  const gbtNfgSupport &p_support, const gbtVector<double> &x,
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

  gbtMixedProfile<double> profile = AdjustZeroes(x, p_support,
						 x[x.Length()]);

  for (int i = 1; i <= profile.Length(); i++) {
    p_stream << "," << std::setprecision(g_numDecimals) << profile[i];
  }

  if (g_maxLike) {
    p_stream << "," << std::setprecision(g_numDecimals) << LogLike(profile);
  }

  p_stream << std::endl;
}

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

double g_maxDecel = 1.1;
double g_hStart = .03;
bool g_fullGraph = true;

static void TracePath(const gbtMixedProfile<double> &p_start,
		      double p_startLambda, double p_maxLambda, double p_omega)	
{
  const double c_tol = 1.0e-4;     // tolerance for corrector iteration
  const double c_maxDist = 0.4;    // maximal distance to curve
  const double c_maxContr = 0.6;   // maximal contraction rate in corrector
  const double c_eta = 0.1;        // perturbation to avoid cancellation
                                   // in calculating contraction rate
  double h = g_hStart;             // initial stepsize
  const double c_hmin = 1.0e-5;    // minimal stepsize

  bool newton = false;          // using Newton steplength (for MLE)
  bool restarting = false;      // flag for first restart step after MLE
  // When doing MLE finding, we push the data from the original path-following
  // here, and resume once we've found the local extremum.
  gbtVector<double> pushX(p_start.Length() + 1);
  double pushH = h;

  gbtVector<double> x(p_start.Length() + 1), u(p_start.Length() + 1);
  for (int i = 1; i <= p_start.Length(); i++) {
    x[i] = p_start[i];
  }
  x[x.Length()] = p_startLambda;
  gbtVector<double> t(p_start.Length() + 1);
  gbtVector<double> y(p_start.Length());

  gbtMatrix<double> b(p_start.Length() + 1, p_start.Length());
  gbtSquareMatrix<double> q(p_start.Length() + 1);
  QreJacobian(p_start.GetSupport(), x, b);
  QRDecomp(b, q);
  q.GetRow(q.NumRows(), t);
  
  int niters = 0;

  while (x[x.Length()] >= 0.0 && x[x.Length()] < p_maxLambda) {
    bool accept = true;

    if (fabs(h) <= c_hmin) {
      // Stop.  If this occurs because we are in MLE-finding mode,
      // resume tracing the original branch
      if (newton) {
	//printf("popping! %f\n", pushH);
	x = pushX;
	h = pushH;
	QreJacobian(p_start.GetSupport(), x, b);
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
      if (k < x.Length() && u[k] < 0.0) {
	accept = false;
	break;
      }
    }

    if (!accept) {
      h *= 0.5;
      continue;
    }

    double decel = 1.0 / g_maxDecel;  // initialize deceleration factor
    QreJacobian(p_start.GetSupport(), u, b);
    QRDecomp(b, q);

    int iter = 1;
    double disto = 0.0;
    while (true) {
      double dist;

      QreLHS(p_start.GetSupport(), u, y);
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
      
      decel = gmax(decel, sqrt(dist / c_maxDist) * g_maxDecel);
      if (iter >= 2) {
	double contr = dist / (disto + c_tol * c_eta);
	if (contr > c_maxContr) {
	  accept = false;
	  break;
	}
	decel = gmax(decel, sqrt(contr / c_maxContr) * g_maxDecel);
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
	if (newton) {
	  //printf("popping! %f\n", pushH);
	  x = pushX;
	  h = pushH;
	  newton = false;
	  restarting = true;
	  QreJacobian(p_start.GetSupport(), x, b);
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

    if (g_maxLike) {
      // Currently, 't' is the tangent at 'x'.  We also need the
      // tangent at 'u'.
      gbtVector<double> newT(t);
      q.GetRow(q.NumRows(), newT); 

      gbtMixedProfile<double> xProfile = AdjustZeroes(x, p_start.GetSupport(),
						      x[x.Length()]);
      gbtMixedProfile<double> uProfile = AdjustZeroes(u, p_start.GetSupport(),
						      u[u.Length()]);

      if (!restarting && 
	  DiffLogLike(xProfile, t) * DiffLogLike(uProfile, newT) < 0.0) {
	// Store the current state, to resume later
	pushX = x;
	pushH = h;
	newton = true;
	//printf("entering newton mode\n");
      }
    }

    if (newton) {
      // Newton-type steplength adaptation, secant method
      gbtVector<double> newT(t);
      q.GetRow(q.NumRows(), newT); 

      gbtMixedProfile<double> xProfile = AdjustZeroes(x, p_start.GetSupport(),
						      x[x.Length()]);
      gbtMixedProfile<double> uProfile = AdjustZeroes(u, p_start.GetSupport(),
						      u[u.Length()]);

      h *= -DiffLogLike(uProfile, newT) / (DiffLogLike(uProfile, newT) - 
					   DiffLogLike(xProfile, t));
    }
    else {
      // Standard steplength adaptation
      h = fabs(h / decel);
    }

    restarting = false;

    // PC step was successful; update and iterate
    for (int i = 1; i < x.Length(); i++) {
      if (u[i] < 1.0e-10) {
	// Drop this strategy from the support, then recursively call
	// to continue tracing
	gbtNfgSupport newSupport(p_start.GetSupport());
	int index = 1;
	for (int pl = 1; pl <= newSupport.GetGame()->NumPlayers(); pl++) {
	  for (int st = 1; st <= newSupport.NumStrats(pl); st++) {
	    if (index++ == i) {
	      newSupport.RemoveStrategy(newSupport.GetStrategy(pl, st));
	    }
	  }
	}

	gbtMixedProfile<double> newProfile(newSupport);
	for (int j = 1; j <= newProfile.Length(); j++) {
	  if (j < i) {
	    newProfile[j] = u[j];
	  }
	  else if (j >= i) {
	    newProfile[j] = u[j+1];
	  }
	}

	TracePath(newProfile, u[u.Length()], p_maxLambda, p_omega);
	return;
      }
      else {
	x[i] = u[i];
      }
    }

    x[x.Length()] = u[u.Length()];

    if (g_fullGraph) {
      PrintProfile(std::cout, p_start.GetSupport(), x);
    }
    
    gbtVector<double> newT(t);
    q.GetRow(q.NumRows(), newT);  // new tangent
    if (t * newT < 0.0) {
      //printf("Bifurcation! at %f\n", x[x.Length()]);
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

void PrintBanner(std::ostream &p_stream)
{
  p_stream << "Compute a branch of the logit equilibrium correspondence\n";
  p_stream << "Gambit version " VERSION ", Copyright (C) 2005, The Gambit Project\n";
  p_stream << "This is free software, distributed under the GNU GPL\n\n";
}

void PrintHelp(char *progname)
{
  PrintBanner(std::cerr);
  std::cerr << "Usage: " << progname << " [OPTIONS]\n";
  std::cerr << "Accepts strategic game on standard input.\n";

  std::cerr << "Options:\n";
  std::cerr << "  -d DECIMALS      show equilibria as floating point with DECIMALS digits\n";
  std::cerr << "  -s STEP          initial stepsize (default is .03)\n";
  std::cerr << "  -a ACCEL         maximum acceleration (default is 1.1)\n";
  std::cerr << "  -m MAXLAMBDA     stop when reaching MAXLAMBDA (default is 1000000)\n";
  std::cerr << "  -h               print this help message\n";
  std::cerr << "  -q               quiet mode (suppresses banner)\n";
  std::cerr << "  -e               print only the terminal equilibrium\n";
  std::cerr << "                   (default is to print the entire branch)\n";
  exit(1);
}

//
// Read in a comma-separated values list of observed data values
//
bool ReadProfile(std::istream &p_stream, gbtArray<double> &p_profile)
{
  for (int i = 1; i <= p_profile.Length(); i++) {
    if (p_stream.eof() || p_stream.bad()) {
      return false;
    }

    p_stream >> p_profile[i];
    if (i < p_profile.Length()) {
      char comma;
      p_stream >> comma;
    }
  }
  // Read in the rest of the line and discard
  std::string foo;
  std::getline(p_stream, foo);
  return true;
}

int main(int argc, char *argv[])
{
  opterr = 0;

  bool quiet = false;
  double maxLambda = 1000000.0;
  std::string mleFile = "";

  int c;
  while ((c = getopt(argc, argv, "d:s:a:m:qehL:")) != -1) {
    switch (c) {
    case 'q':
      quiet = true;
      break;
    case 'd':
      g_numDecimals = atoi(optarg);
      break;
    case 's':
      g_hStart = atof(optarg);
      break;
    case 'a':
      g_maxDecel = atof(optarg);
      break;
    case 'm':
      maxLambda = atof(optarg);
      break;
    case 'e':
      g_fullGraph = false;
      break;
    case 'h':
      PrintHelp(argv[0]);
      break;
    case 'L':
      mleFile = optarg;
      break;
    case '?':
      if (isprint(optopt)) {
	std::cerr << argv[0] << ": Unknown option `-" << ((char) optopt) << "'.\n";
      }
      else {
	std::cerr << argv[0] << ": Unknown option character `\\x" << optopt << "`.\n";
      }
      return 1;
    default:
      abort();
    }
  }

  if (!quiet) {
    PrintBanner(std::cerr);
  }


  gbtNfgGame *nfg;

  try {
    nfg = ReadNfg(std::cin);
  }
  catch (...) {
    return 1;
  }

  if (mleFile != "") {
    g_obsProbs = gbtArray<double>(nfg->ProfileLength());
    std::ifstream mleData(mleFile.c_str());
    ReadProfile(mleData, g_obsProbs);
    g_maxLike = true;
  }
  

  gbtMixedProfile<double> start(nfg);

  try {
    TracePath(start, 0.0, maxLambda, 1.0);
    return 0;
  }
  catch (...) {
    return 1;
  }
}


