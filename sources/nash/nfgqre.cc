//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Computation of quantal response equilibrium correspondence for
// normal form games.
//

#include <math.h>

#include "nfgqre.h"

#include "base/gnullstatus.h"
#include "numerical/gfunc.h"
#include "math/gmath.h"
#include "math/gmatrix.h"
#include "math/gsmatrix.h"

static void WritePXIHeader(gOutput &pxifile, const Nfg &N)
{
  pxifile << "Dimensionality:\n";
  pxifile << N.NumPlayers() << " ";
  for (int pl = 1; pl <= N.NumPlayers(); pl++)
    pxifile << N.NumStrats(pl) << " ";
  pxifile << "\n";
  N.WriteNfgFile(pxifile, 6);

  pxifile << "Settings:\n" << 0.0;
  pxifile << "\n" << 1000.0 << "\n" << 1.05;
  pxifile << "\n" << 0 << "\n" << 1 << "\n" << 1 << "\n";
  
  int numcols = N.ProfileLength() + 2;

  pxifile << "DataFormat:\n" << numcols;
  
  for (int i = 1; i <= numcols; i++)
    pxifile << " " << i;
 
  pxifile << "\nData:\n";
}

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

#ifdef USE_CORRECTOR
static void NewtonStep(gMatrix<double> &q, gMatrix<double> &b,
		       gVector<double> &u, gVector<double> y,
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

static void QreLHS(const Nfg &p_nfg, const gVector<double> &p_point,
		   gVector<double> &p_lhs)
{
  MixedProfile<double> profile(p_nfg);
  for (int i = 1; i <= profile.Length(); i++) {
    profile[i] = p_point[i];
  }
  double lambda = p_point[p_point.Length()];
  
  p_lhs = 0.0;
  int rowno = 0;

  for (int pl = 1; pl <= p_nfg.NumPlayers(); pl++) {
    rowno++;
    for (int st = 1; st <= profile.Support().NumStrats(pl); st++) {
      p_lhs[rowno] += profile(pl, st);
    }
    p_lhs[rowno] -= 1.0;

    for (int st = 2; st <= profile.Support().NumStrats(pl); st++) {
      // Experimental: try fabs() to avoid sign problems, and hope that
      // jacobian will keep things OK...
      p_lhs[++rowno] = log(fabs(profile(pl, st) / profile(pl, 1)));
      p_lhs[rowno] -= (lambda * 
		       (profile.Payoff(pl, pl, st) -
			profile.Payoff(pl, pl, 1)));
      p_lhs[rowno] *= fabs(profile(pl, 1) * profile(pl, st));
    }
  }
}
#endif  // USE_CORRECTOR

static void QreJacobian(const NFSupport &p_support,
			const gVector<double> &p_point,
			gMatrix<double> &p_matrix)
{
  const Nfg &nfg = p_support.Game();
  MixedProfile<double> profile(p_support);
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

      p_matrix(p_matrix.NumRows(), rowno) = -profile(pl1, 1) * profile(pl1, st1) * (profile.Payoff(pl1, pl1, st1) - profile.Payoff(pl1, pl1, 1));
    }
  }
}

void TracePath(const MixedProfile<double> &p_start,
	       double p_startLambda, double p_maxLambda)
{
#ifdef USE_CORRECTOR
  const double c_tol = 1.0e-4;     // tolerance for corrector iteration
  const double c_maxDecel = 2.0;   // maximal deceleration factor
  const double c_maxDist = 0.4;    // maximal distance to curve
  const double c_maxContr = 0.6;   // maximal contraction rate in corrector
  const double c_eta = 0.1;        // perturbation to avoid cancellation
                                   // in calculating contraction rate
#endif  // USE_CORRECTOR
  double h = .1;                   // initial stepsize
  const double c_hmin = 1.0e-8;    // minimal stepsize

  gVector<double> x(p_start.Length() + 1), u(p_start.Length() + 1);
  for (int i = 1; i <= p_start.Length(); i++) {
    x[i] = p_start[i];
  }
  x[x.Length()] = p_startLambda;
  gVector<double> t(p_start.Length() + 1);
  gVector<double> y(p_start.Length());

  gMatrix<double> b(p_start.Length() + 1, p_start.Length());
  gSquareMatrix<double> q(p_start.Length() + 1);
  QreJacobian(p_start.Game(), x, b);
  QRDecomp(b, q);
  q.GetRow(q.NumRows(), t);
  
  double omega = 1.0;     // orientation along the curve
  
  while (x[x.Length()] >= 0.0 && x[x.Length()] < p_maxLambda) {
    gout << "Point: " << x << '\n';
#ifdef USE_CORRECTOR
    bool accept = true;

    if (fabs(h) <= c_hmin) {
      gout << "Failure at minimal stepsize\n";
      return;
    }
#endif  // USE_CORRECTOR

    // Predictor step
    for (int k = 1; k <= x.Length(); k++) {
      u[k] = x[k] + h * omega * t[k];
    }

#ifdef USE_CORRECTOR
    double decel = 1.0 / c_maxDecel;  // initialize deceleration factor
#endif  // USE_CORRECTOR
    QreJacobian(p_start.Game(), u, b);
    QRDecomp(b, q);

#ifdef USE_CORRECTOR
    int iter = 1;
    double disto = 0.0;
    while (true) {
      double dist;

      QreLHS(p_nfg, u, y);
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
      continue;
    }

    // Determine new stepsize
    if (decel > c_maxDecel) {
      decel = c_maxDecel;
    }
    h = fabs(h / decel);
#endif   // USE_CORRECTOR

    gout << x[x.Last()] << ": ";
    for (int i = 1; i < x.Length(); i++) {
      gout << (x[x.Last()] * (u[i] - x[i]) / h) << ' ';
    }
    gout << '\n';

    gout << x[x.Last()] << ": ";
    for (int i = 1; i < x.Length(); i++) {
      gout << ((u[i] - x[i]) / h) << ' ';
    }
    gout << '\n';

    MixedProfile<double> foo(p_start), bar(p_start);
    for (int i = 1; i <= foo.Length(); i++) {
      foo[i] = x[i];
      bar[i] = u[i];
    }
    gout << x[x.Last()] << ": ";
    for (int pl = 1; pl <= p_start.Game().NumPlayers(); pl++) {
      for (int st = 1; st <= p_start.Support().NumStrats(pl); st++) {
	gout << (foo(pl, st) * 
		 (foo.Payoff(pl, pl, st) - foo.Payoff(pl, pl, 1)));
	gout << ' ';
      }
    }
    gout << '\n';

    // PC step was successful; update and iterate
    for (int i = 1; i <= x.Length(); i++) {
      if (u[i] < 1.0e-25) {
	x[i] = 1.0e-25;
      }
      else {
	x[i] = u[i];
      }
    }
    q.GetRow(q.NumRows(), t);  // new tangent
  }
}

nfgQre::nfgQre(void)
  : m_maxLam(30.0), m_stepSize(0.0001), m_fullGraph(false)
{ }

void nfgQre::SolveStep(MixedProfile<double> &p_profile, double &p_nu,
		       double p_initialSign, double p_stepsize) const
{
}

gList<MixedSolution> nfgQre::Solve(const NFSupport &p_support,
				   gStatus &p_status)
{
  gList<MixedSolution> solutions;
  MixedProfile<double> profile(p_support);
  WritePXIHeader(gnull, p_support.Game());

  try {
    TracePath(profile, 0.0, m_maxLam);

#ifdef UNUSED
      // Write out the QreValue as 0 in the PXI file; not generally
      // going to be the case, but QreValue is suspect for large lambda 
      p_pxiFile << "\n" << (nu / (1.0-nu)) << " " << 0.0 << " ";
      for (int pl = 1; pl <= p_nfg.NumPlayers(); pl++) {
	for (int st = 1; st <= profile.Support().NumStrats(pl); st++) {
	  p_pxiFile << profile(pl, st) << " ";
	}
      }

      if (m_fullGraph) { 
	p_corresp.Append(1, nu / (1.0-nu),
			 MixedSolution(profile, algorithmNfg_QRE));
      }

      p_status.Get();
      p_status.SetProgress(nu * (1.0 + m_maxLam) / m_maxLam,
			   gText("Current lambda: ") + ToText(nu / (1.0-nu)));
#endif // UNUSED
  }
  catch (...) {
    //    p_corresp.Append(1, nu / (1.0-nu), MixedSolution(profile, algorithmNfg_QRE));
    throw;
  }
  
  if (!m_fullGraph) { 
    //    p_corresp.Append(1, nu / (1.0-nu), MixedSolution(profile, algorithmNfg_QRE));
  }

  return solutions;
}

