//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation to grid solve method for computing LQRE
//

/*
Here is how it should be done for three person games
(extension to more players is done in an analagous fashion.)

Suppose p,q,and r are the probability vectors for players 1, 2, and 3,
respectively.  Then the following equations give the updating rules
for any one of these, given the other two.

p = f_p(q,r)
q = f_q(p,r)
r = f_r(p,q)

Here, f_p(q,r), etc.  are determined by the exponential rules similar
to the two player case.  So, if p = (p_1,p_2,p_3), then

p_j = (f_p(q,r))_j =
			exp(lam*v_1j)/(exp(lam*v_11)+exp(lam*v_12)+exp(lam*v_13))

where v_1j is the value of strategy j for player 1, when plugging in
the probabilities q and r for players 2 and 3.  Similarly for the
other players.

To find the whole QRE correspondence, then pick n-1 players (say 1 and
2), and then use a grid over their strategy spaces.  For any p,q on
the grid, evaluate the following objective function:

v(p,q) = | p - f_p(q,f_r(p,q)) | + | q - f_q(p,f_r(p,q)) |

Then keep any values of (p,q,f_r(p,q)) where v(p,q) < tolerance.

NEW:  To speed up the grid searches on large games, we introduce a two step
search.  The initial search uses a rough grid (params.delp1) and a
corresponding large tolerance (params.tol1).  If an EQU is found, a new
search is started on a hypercube centered around the point found in previous
step, with a finer grid (params.delp2) and a smaller tolerance (params.tol2).
To enable this two step process, the the params.multi_grid to nonzero.  Note
that this technique is easily extended to n steps.
*/

#include <math.h>
#include "base/base.h"
#include "math/gmath.h"
#include "math/gsmatrix.h"
#include "game/mixed.h"
#include "nfgqregrid.h"
#include "game/nfg.h"

//==========================================================================
//                    class MixedProfileIterator
//==========================================================================

// 
// MixedProfileIterator generates a uniform grid of mixed strategy profiles
// with grid size 'p_step', with width 'p_size' around the specified
// "base" profile 'p_base'.
//
// Note that the grid can be generated for (n-1), not n players but setting
// the static_player member to the nonzero integer corresponding to the
// player whose probability space is NOT to be iterated over.
//
// This class is similar to the MixedProfileGrid.  It generates a grid of
// stepsize step in the region around the point defined by M with a radius of
// size.  It is used to generate a 'finer' grid around a potential EQU point
// in GridSolve.  In that case, size=rough_grid/2.
// In this example, * is the point of interest, step=1, rough_grid=4
// .    .    .    .
// .    .  .....  .
// .    .  .....  .
// .    .  ..*..  .
// .    .  .....  .
// .    .  .....  .
// .    .    .    .

class MixedProfileIterator : public MixedProfile<double> {
private:
  int m_staticPlayer;
  double m_step;
  MixedProfile<double> m_minVal, m_maxVal;
  gArray<double> m_sums;

  bool Next(int row);

public:
  MixedProfileIterator(const MixedProfile<double> &p_base,
		       double p_step, double p_size, int p_staticPlayer);

  bool Next(void);
};

MixedProfileIterator::MixedProfileIterator(const MixedProfile<double> &p_base,
					   double p_step, double p_size,
					   int p_staticPlayer)
  : MixedProfile<double>(p_base),
    m_staticPlayer(p_staticPlayer), m_step(p_step),
    m_minVal(p_base), m_maxVal(p_base),
    m_sums(p_base.Lengths().Length())
{
  for (int i = 1; i <= m_sums.Length(); i++) {
    m_sums[i] = 0;

    // Precalc these to save time.  Memory is cheap.
    for (int j = 1; j <= svlen[i] - 1; j++) { 
      if (m_minVal(i,j) >= p_size) {
	m_minVal(i,j) -= p_size;
      }
      else {
	m_minVal(i,j) = 0.0;
      }

      (*this)(i,j) = m_minVal(i,j);	// start at the minimum
      m_sums[i] += (*this)(i,j);
 
      if (m_maxVal(i,j) <= 1.0 - p_size) {
	m_maxVal(i,j) += p_size;
      }
      else {
	m_maxVal(i,j) = 1.0;
      }
    }

    (*this)(i, svlen[i]) = 1.0 - m_sums[i];
  }
}

bool MixedProfileIterator::Next(int row)
{
  const double eps = 1e-8;

  int dim = svlen[row];
  // dim==1 is an annoying special case
  if (dim != 1) {
    double &sum = m_sums[row];
    do {
      for (int i = 1; i <= dim-1; i++) {
	if ((*this)(row,i) < m_maxVal(row,i) - m_step + eps) {
	  (*this)(row,i) += m_step;
	  sum += m_step;
	  break;
	}
	else {
	  sum -= ((*this)(row,i) - m_minVal(row,i));
	  (*this)(row,i) = m_minVal(row,i);
	  if (i == dim-1) {
	    (*this)(row,dim) = 1.0 - sum;
	    return false;
	  }
	}
      }
    } while (sum > 1.0 + eps);
    (*this)(row,dim) = 1.0 - sum;
    return true;
  }
  else	{
    // dim == 1
    return false;
  }
}

bool MixedProfileIterator::Next(void)
{
  for (int i = 1; i <= svlen.Length(); i++) {
    if (i == m_staticPlayer) continue;
    if (Next(i)) {
      return true;
    }
    else {
      if (i == svlen.Length()) {
	return false;
      }
    }
  }
  return false; // return from here only when static_player==svlen.Length
}


//==========================================================================
//                          class QreNfgGrid
//==========================================================================

//--------------------------------------------------------------------------
//                        QreNfgGrid: Lifecycle
//--------------------------------------------------------------------------

QreNfgGrid::QreNfgGrid(void)
  : m_minLam(.01), m_maxLam(30), m_delLam(.01), m_delp1(.01), m_delp2(0.01), 
    m_tol1(.01), m_tol2(0.01), m_powLam(1), m_fullGraph(false)
{ }

//--------------------------------------------------------------------------
//               QreNfgGrid: Private auxiliary functions
//--------------------------------------------------------------------------

void QreNfgGrid::OutputHeader(const NFSupport &p_support, gOutput &out) const
{
  out<< "Dimensionality:\n";
  out<< p_support.Game().NumPlayers()<<' ';
  for (int pl = 1; pl <= p_support.Game().NumPlayers(); pl++) {
    out << p_support.NumStrats(pl) << ' ';
  }
  out << '\n';
 
  out << "Settings:\n";
  out << m_minLam << '\n' << m_maxLam << '\n' << m_delLam << '\n';
  out << 0 << '\n' << 1 << '\n' << m_powLam << '\n';
  
  out << "Extra:\n";
  out << 1 << '\n' << m_tol1 << '\n' << m_delp1 << '\n';
  
  out << "DataFormat:\n";
  int numcols = p_support.TotalNumStrats() + 2;
  out << numcols <<' '; 
  for (int i = 1; i <= numcols; i++) out << i << ' ';
  
  out << '\n';
  
  out << "Data:\n";
}

void QreNfgGrid::OutputResult(gOutput &p_file,
			      const MixedProfile<double> &p_profile,
			      double p_lambda, double p_objFunc) const
{
  p_file << p_lambda << ' ' << p_objFunc << ' ';
  for (int i = 1; i <= p_profile.Length(); i++) {
    p_file << p_profile[i] << ' ';
  }
  p_file << '\n';
}

double QreNfgGrid::Distance(const gVector<gNumber> &a,
			    const gVector<double> &b) const
{
  double dist = 0.0;
  for (int i = 1; i <= a.Length(); i++) {
    if (abs((double) a[i] - b[i]) > dist) {
      dist = abs((double) a[i] - b[i]);
    }
  }
  return dist;
}

double QreNfgGrid::Distance(const gVector<double> &a,
			    const gVector<double> &b) const
{
  double dist = 0.0;
  for (int i = 1; i <= a.Length(); i++) {
    if (abs(a[i] - b[i]) > dist) {
      dist = abs(a[i] - b[i]);
    }
  }
  return dist;
}


gVector<double> QreNfgGrid::UpdateFunc(const MixedProfile<double> &p_profile,
				       int p_player, double p_lambda) const
{
  gVector<double> r(p_profile.Support().NumStrats(p_player));
  gVector<double> tmp(p_profile.Support().NumStrats(p_player));
  double denom = 0.0;
  for (int st = 1; st <= r.Length(); st++) {
    double p = p_profile.Payoff(p_player, p_player, st);
    tmp[st] = exp(p_lambda * p);
    denom += tmp[st];
  }
  for (int st = 1; st <= r.Length(); st++) {
    r[st] = tmp[st] / denom;
  }
  return r;
}


// Note: static_player just refers to the player w/ the greatest # of strats.
bool QreNfgGrid::CheckEqu(MixedProfile<double> &p_profile,
			  double p_lambda, int p_staticPlayer,
			  double p_tol) const
{
  p_profile.SetRow(p_staticPlayer, 
		   UpdateFunc(p_profile, p_staticPlayer, p_lambda));
  
  MixedProfile<double> newProfile(p_profile);
  for (int pl = 1; pl <= p_profile.Game().NumPlayers(); pl++) {
    if (pl != p_staticPlayer) {
      newProfile.SetRow(pl, UpdateFunc(p_profile, pl, p_lambda));
      if (Distance(newProfile.GetRow(pl), p_profile.GetRow(pl)) > p_tol) {
	return false;
      }
    }
  }
  
  return true;
}

//
// Use Newton's method to attempt to "polish" the profile to be close
// to an LQRE.
//
static void Jacobian(gVector<double> &p_vector, 
		     gSquareMatrix<double> &p_matrix,
		     const MixedProfile<double> &p_profile, double p_lambda)
{
  gPVector<double> logitterms(p_profile.Lengths());
  for (int pl = 1; pl <= p_profile.Game().NumPlayers(); pl++) {
    for (int st = 1; st <= p_profile.Support().NumStrats(pl); st++) {
      logitterms(pl, st) = exp(p_lambda * p_profile.Payoff(pl, pl, st));
    }
  }

  int rowno = 0;

  for (int pl1 = 1; pl1 <= p_profile.Game().NumPlayers(); pl1++) {
    double logitsum = 0.0;
    for (int st = 1; st <= p_profile.Support().NumStrats(pl1); st++) {
      logitsum += logitterms(pl1, st);
    }

    for (int st1 = 1; st1 <= p_profile.Support().NumStrats(pl1); st1++) {
      rowno++;

      p_vector[rowno] = p_profile(pl1, st1) * logitsum - logitterms(pl1, st1);

      int colno = 0;
      for (int pl2 = 1; pl2 <= p_profile.Game().NumPlayers(); pl2++) {
	for (int st2 = 1; st2 <= p_profile.Support().NumStrats(pl2); st2++) {
	  colno++;

	  if (pl1 == pl2) {
	    if (st1 == st2) {
	      p_matrix(rowno, colno) = logitsum;
	    }
	    else {
	      p_matrix(rowno, colno) = 0.0;
	    }
	  }
	  else {
	    p_matrix(rowno, colno) = 0.0;
	    for (int st = 1; st <= p_profile.Support().NumStrats(pl1); st++) {
	      p_matrix(rowno, colno) += p_profile.Payoff(pl1, pl1, st, pl2, st2) * logitterms(pl1, st);
	    }
	    p_matrix(rowno, colno) *= p_profile(pl1, st1);
	    p_matrix(rowno, colno) -= p_profile.Payoff(pl1, pl1, st1, pl2, st2) * logitterms(pl1, st1);
	    p_matrix(rowno, colno) *= p_lambda;
	  }
	}
      }
    }
  }
}

static double Norm(const gVector<double> &p_vector)
{
  double norm = 0.0;

  for (int i = 1; i <= p_vector.Length(); i++) {
    norm += p_vector[i] * p_vector[i];
  }

  return sqrt(norm);
}

static bool Polish(MixedProfile<double> &p_profile, double p_lambda)
{
  gVector<double> f(p_profile.Length());
  gSquareMatrix<double> J(p_profile.Length());

  for (int iter = 1; iter <= 100; iter++) {
    try {
      Jacobian(f, J, p_profile, p_lambda);

      gVector<double> step = J.Inverse() * f;
      for (int i = 1; i <= p_profile.Length(); i++) {
	p_profile[i] -= step[i];
      }

      if (Norm(step) <= .0000001 * (1.0 + Norm(p_profile))) {
	for (int i = 1; i <= p_profile.Length(); i++) {
	  if (p_profile[i] < 0.0) {
	    return false;
	  }
	}
	return true;
      }
    }
    catch (gSquareMatrix<double>::MatrixSingular &) {
      // check to see if maybe by dumb luck we have a solution
      for (int i = 1; i <= f.Length(); i++) {
	if (f[i] > .0001) {
	  return false;
	}
      }
      return true;
    }
  }

  return false;
}

void QreNfgGrid::Solve(const NFSupport &p_support, gOutput &p_pxifile,
		       gStatus &p_status, gList<MixedSolution> &p_solutions)
{
  p_solutions.Flush();

  // find the player w/ the most strategies and make it static
  int staticPlayer = 1;
  for (int i = 2; i <= p_support.Game().NumPlayers(); i++) {
    if (p_support.NumStrats(i) > p_support.NumStrats(staticPlayer))  {
      staticPlayer = i;
    }
  }

  OutputHeader(p_support, p_pxifile);

  int step = 0, numSteps;
  if (m_powLam == 0) {
    numSteps = (int) ((m_maxLam - m_minLam) / m_delLam);
  }
  else {
    numSteps = (int) (log(m_maxLam/m_minLam) / log(m_delLam + 1.0));
  }

  MixedProfile<double> centroid(p_support);
  double lambda = m_minLam;
  while (lambda <= m_maxLam) {
    step++;
    gList<MixedSolution> cursolns;

    p_status.Get();
    MixedProfileIterator iter1(centroid, m_delp1, 1.0, staticPlayer);

    do {
      p_status.Get();
      if (CheckEqu(iter1, lambda, staticPlayer, m_tol1)) {
	MixedProfileIterator iter2(iter1, m_delp2, m_delp1 / 2.0,
				   staticPlayer);
	do {
	  p_status.Get();
	  if (CheckEqu(iter2, lambda, staticPlayer, m_tol2)) {
	    MixedProfile<double> candidate(iter2);
	    if (Polish(candidate, lambda)) {
	      bool newsoln = true;
	      for (int j = 1; j <= cursolns.Length(); j++) {
		if (Distance(*cursolns[j].Profile(), candidate) < .00001) {
		  newsoln = false;
		}
	      }

	      if (newsoln) {
		OutputResult(p_pxifile, candidate, lambda, 0.0);
		int i = cursolns.Append(MixedSolution(candidate,
						      algorithmNfg_QREALL));
		cursolns[i].SetQre(lambda, 0.0);
	      }
	    }
	  }
	} while (iter2.Next());
      }
    } while (iter1.Next());

    p_status.SetProgress((double) step / (double) numSteps,
			 gText("Lambda = ") + ToText(lambda));

    if (m_fullGraph || step == numSteps) {
      p_solutions += cursolns;
    }

    if (m_powLam == 0)  {
      lambda += m_delLam;
    } 
    else  {
      lambda *= (m_delLam + 1.0);
    }
  }
  
}

//==========================================================================
//                            Logit Dynamics
//==========================================================================

static gVector<double> UpdateFunc(const MixedProfile<double> &p_profile,
				  int p_player, double p_lambda) 
{
  gVector<double> r(p_profile.Support().NumStrats(p_player));
  gVector<double> tmp(p_profile.Support().NumStrats(p_player));
  double denom = 0.0;
  for (int st = 1; st <= r.Length(); st++) {
    double p = p_profile.Payoff(p_player, p_player, st);
    tmp[st] = exp(p_lambda * p);
    denom += tmp[st];
  }
  for (int st = 1; st <= r.Length(); st++) {
    r[st] = tmp[st] / denom;
  }
  return r;
}

