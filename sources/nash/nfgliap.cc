//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute Nash equilibria by minimizing Liapunov function
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

#include "base/gnullstatus.h"
#include "numerical/gfuncmin.h"
#include "nfgliap.h"


class gbtMixedLiapFunction : public gbtC1Function<double>  {
private:
  mutable gbtMixedProfile<double> m_profile;

  double Value(const gbtVector<double> &) const;
  bool Gradient(const gbtVector<double> &, gbtVector<double> &) const;

  double LiapDerivValue(int, int, const gbtMixedProfile<double> &) const;
    
public:
  gbtMixedLiapFunction(const gbtMixedProfile<double> &);
  virtual ~gbtMixedLiapFunction();
};

gbtMixedLiapFunction::gbtMixedLiapFunction(const gbtMixedProfile<double> &start)
  : m_profile(start)
{ }

gbtMixedLiapFunction::~gbtMixedLiapFunction()
{ }

double 
gbtMixedLiapFunction::LiapDerivValue(int i1, int j1,
				     const gbtMixedProfile<double> &p) const
{
  int i, j;
  double x, x1, psum;
  
  x = 0.0;
  for (i = 1; i <= m_profile->GetGame()->NumPlayers(); i++)  {
    gbtGamePlayer player = m_profile->GetGame()->GetPlayer(i);
    psum = 0.0;
    for (j = 1; j <= player->NumStrategies(); j++)  {
      gbtGameStrategy strategy = player->GetStrategy(j);
      psum += p(i,j);
      x1 = p->GetPayoffDeriv(player, strategy) - p->GetPayoff(player);
      if (i1 == i) {
	if (x1 > 0.0) {
	  x -= x1 * p->GetPayoffDeriv(player, m_profile->GetGame()->GetPlayer(i1)->GetStrategy(j1));
	}
      }
      else {
	if (x1 > 0.0) {
	  x += x1 * (p->GetPayoffDeriv(player, strategy, 
				  m_profile->GetGame()->GetPlayer(i1)->GetStrategy(j1)) - 
		     p->GetPayoffDeriv(player, m_profile->GetGame()->GetPlayer(i1)->GetStrategy(j1)));
	}
      }
    }
  }
  if (p(i1, j1) < 0.0)   x += p(i1, j1);
  return 2.0 * x;
}

//
// This function projects a gradient into the plane of the simplex.
// (Actually, it works by computing the projection of 'x' onto the
// vector perpendicular to the plane, then subtracting to compute the
// component parallel to the plane.)  Also imposes binding nonnegativity
// constraints as appropriate.
//
static void Project(gbtVector<double> &grad, const gbtVector<double> &x,
		    const gbtArray<int> &lengths)
{
  int index = 1;
  for (int part = 1; part <= lengths.Length(); part++)  {
    double avg = 0.0;
    int nactive = 0;
    int j;
    for (j = 1; j <= lengths[part]; j++, index++)  {
      // If x[index] is small, assume nonnegativity is binding.
      // On the other hand, if gradient suggests that the minimizing
      // direction is towards the interior, let it go (relax constraint).
      if (x[index] > 1.0e-7 || grad[index] < 0.0) {
	avg += grad[index];
	nactive++;
      }
    }
    avg /= (double) nactive;
    index -= lengths[part];
    for (j = 1; j <= lengths[part]; j++, index++)  {
      if (x[index] > 1.0e-7 || grad[index] < 0.0) {
	grad[index] -= avg;
      }
      else {
	grad[index] = 0.0;
      }
    }
  }
}

bool gbtMixedLiapFunction::Gradient(const gbtVector<double> &v, 
				    gbtVector<double> &d) const
{
  for (int i = 1; i <= v.Length(); i++) {
    m_profile[i] = v[i];
  }
  int i1, j1, ii;
  
  for (i1 = 1, ii = 1; i1 <= m_profile->GetGame()->NumPlayers(); i1++) {
    for (j1 = 1; j1 <= m_profile->GetGame()->GetPlayer(i1)->NumStrategies(); j1++) {
      d[ii++] = LiapDerivValue(i1, j1, m_profile);
    }
  }

  // Project for constraints
  Project(d, v, m_profile->GetGame()->NumStrategies());
  return true;
}
  
double gbtMixedLiapFunction::Value(const gbtVector<double> &v) const
{
  for (int i = 1; i <= v.Length(); i++) {
    m_profile[i] = v[i];
  }

  return m_profile->GetLiapValue(false);
}

gbtList<gbtMixedProfile<double> > 
gbtNashLiapNfg(const gbtMixedProfile<double> &p_start, 
	       int p_maxitsN, double p_tolN,
	       int p_maxits1, double p_tol1)
{
  static const double ALPHA = .00000001;
  gbtMixedProfile<double> p(p_start);
  gbtMixedLiapFunction F(p);

  // if starting vector not interior, perturb it towards centroid
  int kk;
  for (kk = 1; kk <= p->StrategyProfileLength() && p[kk] > ALPHA;
       kk++);
  if (kk <= p->StrategyProfileLength()) {
    gbtMixedProfile<double> centroid = p_start->GetGame()->NewMixedProfile(0.0);
    for (int k = 1; k <= p->StrategyProfileLength(); k++) {
      p[k] = centroid[k] * ALPHA + p[k] * (1.0-ALPHA);
    }
  }

  gbtNullStatus status;

  try {
    gbtConjugatePRMinimizer minimizer(p->StrategyProfileLength());
    gbtVector<double> gradient(p->StrategyProfileLength()), dx(p->StrategyProfileLength());
    double fval;
    gbtVector<double> pvect(p->StrategyProfileLength());
    for (int i = 1; i <= pvect.Length(); i++) {
      pvect[i] = p[i];
    }
    
    minimizer.Set(F, pvect, fval, gradient, .01, p_tol1);

    try {
      for (int iter = 1; iter <= p_maxitsN; iter++) {
	if (iter % 20 == 0) {
	  status.Get();
	}
	  
	gbtMixedProfile<double> q(p);
	gbtVector<double> pvect(p->StrategyProfileLength());
	for (int i = 1; i <= pvect.Length(); i++) {
	  pvect[i] = p[i];
	}
	if (!minimizer.Iterate(F, pvect, fval, gradient, dx)) {
	  break;
	}
	
	// Guard against wasting time when we get "stuck" on a
	// boundary
	double dist = 0.0;
	for (int i = 1; i <= p->StrategyProfileLength(); i++) {
	  dist += fabs(p[i] - q[i]);
	}
	if (dist <= 1.0e-8) {
	  break;
	}

	if (sqrt(gradient.NormSquared()) < .001 && fval < p_tolN) {
	  gbtList<gbtMixedProfile<double> > results;
	  results.Append(p);
	  return results;
	}
      }
    }
    catch (gbtMinimizationException &) { }
  }
  catch (gbtInterruptException &) {
    // Just stop and return the empty list
    // Any other exceptions propagate out, assuming something Real Bad happened
  }
  return gbtList<gbtMixedProfile<double> >();
}
