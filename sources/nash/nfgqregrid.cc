//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation to grid solve method for computing LQRE
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
#include "math/rational.h"
#include "math/gmath.h"
#include "math/gsmatrix.h"
#include "game/game.h"
#include "nfgqregrid.h"

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

class MixedProfileIterator {
private:
  gbtMixedProfile<double> m_profile;
  int m_staticPlayer;
  double m_step;
  gbtMixedProfile<double> m_minVal, m_maxVal;
  gbtArray<double> m_sums;

  bool Next(int row);

public:
  MixedProfileIterator(const gbtMixedProfile<double> &p_base,
		       double p_step, double p_size, int p_staticPlayer);

  bool Next(void);

  gbtMixedProfile<double> &GetProfile(void) { return m_profile; }
};

MixedProfileIterator::MixedProfileIterator(const gbtMixedProfile<double> &p_base,
					   double p_step, double p_size,
					   int p_staticPlayer)
  : m_profile(p_base),
    m_staticPlayer(p_staticPlayer), m_step(p_step),
    m_minVal(p_base), m_maxVal(p_base),
    m_sums(p_base->GetGame()->NumPlayers())
{
  for (int i = 1; i <= m_sums.Length(); i++) {
    m_sums[i] = 0;

    // Precalc these to save time.  Memory is cheap.
    for (int j = 1; j <= m_profile->GetGame()->GetPlayer(i)->NumStrategies() - 1; j++) { 
      if (m_minVal(i,j) >= p_size) {
	m_minVal(i,j) -= p_size;
      }
      else {
	m_minVal(i,j) = 0.0;
      }

      m_profile(i,j) = m_minVal(i,j);	// start at the minimum
      m_sums[i] += m_profile(i,j);
 
      if (m_maxVal(i,j) <= 1.0 - p_size) {
	m_maxVal(i,j) += p_size;
      }
      else {
	m_maxVal(i,j) = 1.0;
      }
    }

    m_profile(i, m_profile->GetGame()->GetPlayer(i)->NumStrategies()) = 1.0 - m_sums[i];
  }
}

bool MixedProfileIterator::Next(int row)
{
  const double eps = 1e-8;

  int dim = m_profile->GetGame()->GetPlayer(row)->NumStrategies();
  // dim==1 is an annoying special case
  if (dim != 1) {
    double &sum = m_sums[row];
    do {
      for (int i = 1; i <= dim-1; i++) {
	if (m_profile(row,i) < m_maxVal(row,i) - m_step + eps) {
	  m_profile(row,i) += m_step;
	  sum += m_step;
	  break;
	}
	else {
	  sum -= (m_profile(row,i) - m_minVal(row,i));
	  m_profile(row,i) = m_minVal(row,i);
	  if (i == dim-1) {
	    m_profile(row,dim) = 1.0 - sum;
	    return false;
	  }
	}
      }
    } while (sum > 1.0 + eps);
    m_profile(row,dim) = 1.0 - sum;
    return true;
  }
  else	{
    // dim == 1
    return false;
  }
}

bool MixedProfileIterator::Next(void)
{
  for (int i = 1; i <= m_profile->GetGame()->NumPlayers(); i++) {
    if (i == m_staticPlayer) continue;
    if (Next(i)) {
      return true;
    }
    else {
      if (i == m_profile->GetGame()->NumPlayers()) {
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

void QreNfgGrid::OutputHeader(const gbtGame &p_game, std::ostream &out) const
{
  out<< "Dimensionality:\n";
  out<< p_game->NumPlayers()<<' ';
  for (int pl = 1; pl <= p_game->NumPlayers(); pl++) {
    out << p_game->GetPlayer(pl)->NumStrategies() << ' ';
  }
  out << '\n';
 
  out << "Settings:\n";
  out << m_minLam << '\n' << m_maxLam << '\n' << m_delLam << '\n';
  out << 0 << '\n' << 1 << '\n' << m_powLam << '\n';
  
  out << "Extra:\n";
  out << 1 << '\n' << m_tol1 << '\n' << m_delp1 << '\n';
  
  out << "DataFormat:\n";
  int numcols = p_game->StrategyProfileLength() + 2;
  out << numcols <<' '; 
  for (int i = 1; i <= numcols; i++) out << i << ' ';
  
  out << '\n';
  
  out << "Data:\n";
}

void QreNfgGrid::OutputResult(std::ostream &p_file,
			      const gbtMixedProfile<double> &p_profile,
			      double p_lambda, double p_objFunc) const
{
  p_file << p_lambda << ' ' << p_objFunc << ' ';
  for (int i = 1; i <= p_profile->StrategyProfileLength(); i++) {
    p_file << p_profile[i] << ' ';
  }
  p_file << '\n';
}

double QreNfgGrid::Distance(const gbtVector<gbtRational> &a,
			    const gbtVector<double> &b) const
{
  double dist = 0.0;
  for (int i = 1; i <= a.Length(); i++) {
    if (fabs((double) a[i] - b[i]) > dist) {
      dist = fabs((double) a[i] - b[i]);
    }
  }
  return dist;
}

double QreNfgGrid::Distance(const gbtVector<double> &a,
			    const gbtVector<double> &b) const
{
  double dist = 0.0;
  for (int i = 1; i <= a.Length(); i++) {
    if (fabs(a[i] - b[i]) > dist) {
      dist = fabs(a[i] - b[i]);
    }
  }
  return dist;
}

double QreNfgGrid::Distance(const gbtMixedProfile<double> &a,
			    const gbtMixedProfile<double> &b) const
{
  double dist = 0.0;
  for (int i = 1; i <= a->StrategyProfileLength(); i++) {
    if (fabs(a[i] - b[i]) > dist) {
      dist = fabs(a[i] - b[i]);
    }
  }
  return dist;
}


gbtVector<double>
QreNfgGrid::UpdateFunc(const gbtMixedProfile<double> &p_profile,
		       int p_player, double p_lambda) const
{
  gbtVector<double> r(p_profile->GetGame()->GetPlayer(p_player)->NumStrategies());
  gbtVector<double> tmp(p_profile->GetGame()->GetPlayer(p_player)->NumStrategies());
  double denom = 0.0;
  for (int st = 1; st <= r.Length(); st++) {
    double p = p_profile->GetPayoffDeriv(p_profile->GetGame()->GetPlayer(p_player), p_profile->GetGame()->GetPlayer(p_player)->GetStrategy(st));
    tmp[st] = exp(p_lambda * p);
    denom += tmp[st];
  }
  for (int st = 1; st <= r.Length(); st++) {
    r[st] = tmp[st] / denom;
  }
  return r;
}


// Note: static_player just refers to the player w/ the greatest # of strats.
bool QreNfgGrid::CheckEqu(gbtMixedProfile<double> &p_profile,
			  double p_lambda, int p_staticPlayer,
			  double p_tol) const
{
  gbtArray<double> array = UpdateFunc(p_profile, p_staticPlayer, p_lambda);
  for (int st = 1; st <= p_profile->GetGame()->GetPlayer(p_staticPlayer)->NumStrategies(); st++) {
    p_profile(p_staticPlayer, st) = array[st];
  }
  
  gbtMixedProfile<double> newProfile(p_profile);
  for (int pl = 1; pl <= p_profile->GetGame()->NumPlayers(); pl++) {
    if (pl != p_staticPlayer) {
      ((gbtPVector<double> &) newProfile).SetRow(pl, UpdateFunc(p_profile, pl, p_lambda));
      if (Distance(((gbtPVector<double> &) newProfile).GetRow(pl), ((gbtPVector<double> &) p_profile).GetRow(pl)) > p_tol) {
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
static void Jacobian(gbtVector<double> &p_vector, 
		     gbtSquareMatrix<double> &p_matrix,
		     const gbtMixedProfile<double> &p_profile, double p_lambda)
{
  gbtPVector<double> logitterms(p_profile->GetGame()->NumStrategies());
  for (int pl = 1; pl <= p_profile->GetGame()->NumPlayers(); pl++) {
    for (int st = 1; st <= p_profile->GetGame()->GetPlayer(pl)->NumStrategies();
	 st++) {
      logitterms(pl, st) = exp(p_lambda * 
			       p_profile->GetPayoffDeriv(p_profile->GetGame()->GetPlayer(pl), p_profile->GetGame()->GetPlayer(pl)->GetStrategy(st)));
    }
  }

  int rowno = 0;

  for (int pl1 = 1; pl1 <= p_profile->GetGame()->NumPlayers(); pl1++) {
    double logitsum = 0.0;
    for (int st = 1; st <= p_profile->GetGame()->GetPlayer(pl1)->NumStrategies(); st++) {
      logitsum += logitterms(pl1, st);
    }

    for (int st1 = 1; st1 <= p_profile->GetGame()->GetPlayer(pl1)->NumStrategies(); st1++) {
      rowno++;

      p_vector[rowno] = p_profile(pl1, st1) * logitsum - logitterms(pl1, st1);

      int colno = 0;
      for (int pl2 = 1; pl2 <= p_profile->GetGame()->NumPlayers(); pl2++) {
	for (int st2 = 1; st2 <= p_profile->GetGame()->GetPlayer(pl2)->NumStrategies(); st2++) {
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
	    for (int st = 1; st <= p_profile->GetGame()->GetPlayer(pl1)->NumStrategies(); st++) {
	      p_matrix(rowno, colno) += p_profile->GetPayoffDeriv(p_profile->GetGame()->GetPlayer(pl1), p_profile->GetGame()->GetPlayer(pl1)->GetStrategy(st), p_profile->GetGame()->GetPlayer(pl2)->GetStrategy(st2)) * logitterms(pl1, st);
	    }
	    p_matrix(rowno, colno) *= p_profile(pl1, st1);
	    p_matrix(rowno, colno) -= p_profile->GetPayoffDeriv(p_profile->GetGame()->GetPlayer(pl1), p_profile->GetGame()->GetPlayer(pl1)->GetStrategy(st1), p_profile->GetGame()->GetPlayer(pl2)->GetStrategy(st2)) * logitterms(pl1, st1);
	    p_matrix(rowno, colno) *= p_lambda;
	  }
	}
      }
    }
  }
}

static double Norm(const gbtVector<double> &p_vector)
{
  double norm = 0.0;

  for (int i = 1; i <= p_vector.Length(); i++) {
    norm += p_vector[i] * p_vector[i];
  }

  return sqrt(norm);
}

static double Norm(const gbtMixedProfile<double> &p_vector)
{
  double norm = 0.0;

  for (int i = 1; i <= p_vector->StrategyProfileLength(); i++) {
    norm += p_vector[i] * p_vector[i];
  }

  return sqrt(norm);
}

static bool Polish(gbtMixedProfile<double> &p_profile, double p_lambda)
{
  gbtVector<double> f(p_profile->GetGame()->StrategyProfileLength());
  gbtSquareMatrix<double> J(p_profile->GetGame()->StrategyProfileLength());

  for (int iter = 1; iter <= 100; iter++) {
    try {
      Jacobian(f, J, p_profile, p_lambda);

      gbtVector<double> step = J.Inverse() * f;
      for (int i = 1; i <= p_profile->StrategyProfileLength(); i++) {
	p_profile[i] -= step[i];
      }

      if (Norm(step) <= .0000001 * (1.0 + Norm(p_profile))) {
	for (int i = 1; i <= p_profile->StrategyProfileLength(); i++) {
	  if (p_profile[i] < 0.0) {
	    return false;
	  }
	}
	return true;
      }
    }
    catch (gbtSingularMatrixException &) {
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

void QreNfgGrid::Solve(const gbtGame &p_game, std::ostream &p_pxifile,
		       gbtStatus &p_status,
		       gbtList<gbtMixedProfile<double> > &p_solutions)
{
  p_solutions.Flush();

  // find the player w/ the most strategies and make it static
  int staticPlayer = 1;
  for (int i = 2; i <= p_game->NumPlayers(); i++) {
    if (p_game->GetPlayer(i)->NumStrategies() > 
	p_game->GetPlayer(staticPlayer)->NumStrategies())  {
      staticPlayer = i;
    }
  }

  OutputHeader(p_game, p_pxifile);

  int step = 0, numSteps;
  if (m_powLam == 0) {
    numSteps = (int) ((m_maxLam - m_minLam) / m_delLam);
  }
  else {
    numSteps = (int) (log(m_maxLam/m_minLam) / log(m_delLam + 1.0));
  }

  gbtMixedProfile<double> centroid = p_game->NewMixedProfile(0.0);
  double lambda = m_minLam;
  while (lambda <= m_maxLam) {
    step++;
    gbtList<gbtMixedProfile<double> > cursolns;

    p_status.Get();
    MixedProfileIterator iter1(centroid, m_delp1, 1.0, staticPlayer);

    do {
      p_status.Get();
      if (CheckEqu(iter1.GetProfile(), lambda, staticPlayer, m_tol1)) {
	MixedProfileIterator iter2(iter1.GetProfile(), m_delp2, m_delp1 / 2.0,
				   staticPlayer);
	do {
	  p_status.Get();
	  if (CheckEqu(iter2.GetProfile(), lambda, staticPlayer, m_tol2)) {
	    gbtMixedProfile<double> candidate(iter2.GetProfile());
	    if (Polish(candidate, lambda)) {
	      bool newsoln = true;
	      for (int j = 1; j <= cursolns.Length(); j++) {
		if (Distance(cursolns[j], candidate) < .00001) {
		  newsoln = false;
		}
	      }

	      if (newsoln) {
		OutputResult(p_pxifile, candidate, lambda, 0.0);
		cursolns.Append(candidate);
	      }
	    }
	  }
	} while (iter2.Next());
      }
    } while (iter1.Next());

    p_status.SetProgress((double) step / (double) numSteps,
			 std::string("Lambda = ") + ToText(lambda));

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
