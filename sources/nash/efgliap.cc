//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute Nash equilibria via Lyapunov function minimization
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

#include "efgliap.h"
#include "base/gstatus.h"
#include "math/gmatrix.h"
#include "numerical/gfuncmin.h"

class gbtBehavLiapFunction : public gbtC1Function<double>  {
private:
  mutable gbtBehavProfile<double> m_profile;

  double Value(const gbtVector<double> &x) const;
  bool Gradient(const gbtVector<double> &, gbtVector<double> &) const;

public:
  gbtBehavLiapFunction(const gbtBehavProfile<double> &);
  virtual ~gbtBehavLiapFunction();
};


gbtBehavLiapFunction::gbtBehavLiapFunction(const gbtBehavProfile<double> &start)
  : m_profile(start)
{ }

gbtBehavLiapFunction::~gbtBehavLiapFunction()
{ }


double gbtBehavLiapFunction::Value(const gbtVector<double> &v) const
{
  for (int i = 1; i <= v.Length(); i++) {
    m_profile[i] = v[i];
  }
  // Don't impose penalties in Lyapunov function; avoid this as
  // we go out of the feasible set in numerically computing the
  // derivative of the function.
  return m_profile->GetLiapValue(false);
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

bool gbtBehavLiapFunction::Gradient(const gbtVector<double> &x,
			  gbtVector<double> &grad) const
{
  const double DELTA = .00001;

  for (int i = 1; i <= x.Length(); i++) {
    m_profile[i] = x[i];
  }

  for (int i = 1; i <= x.Length(); i++) {
    m_profile[i] += DELTA;
    double value = m_profile->GetLiapValue(false);
    m_profile[i] -= 2.0 * DELTA;
    value -= m_profile->GetLiapValue(false);
    m_profile[i] += DELTA;
    grad[i] = value / (2.0 * DELTA);
  }

  // Project for constraints
  Project(grad, x, m_profile->GetGame()->NumActions());

  return true;
}

gbtList<gbtBehavProfile<double> >
gbtNashLiapEfg(const gbtBehavProfile<double> &p_start, 
	       int p_maxitsN, double p_tolN,
	       int p_maxits1, double p_tol1)
{
  static const double ALPHA = .00000001;

  gbtBehavProfile<double> p(p_start);
  gbtBehavLiapFunction F(p);

  // if starting vector not interior, perturb it towards centroid
  int kk = 0;
  for (int kk = 1; kk <= p->BehaviorProfileLength() && p[kk] > ALPHA; kk++);
  if (kk <= p->BehaviorProfileLength()) {
    gbtBehavProfile<double> c = p_start->GetGame()->NewBehavProfile(0.0);
    for (int k = 1; k <= p->BehaviorProfileLength(); k++) {
      p[k] = c[k]*ALPHA + p[k]*(1.0-ALPHA);
    }
  }

  gbtMatrix<double> xi(p->BehaviorProfileLength(), p->BehaviorProfileLength());
  gbtNullStatus status;

  try {
    gbtConjugatePRMinimizer minimizer(p->BehaviorProfileLength());
    gbtVector<double> gradient(p->BehaviorProfileLength());
    gbtVector<double> dx(p->BehaviorProfileLength());
    double fval;
    minimizer.Set(F, p, fval, gradient, .01, p_tol1);

    try {
      for (int iter = 1; iter <= p_maxitsN; iter++) {
	if (iter % 20 == 0) {
	  status.Get();
	}
	  
	gbtVector<double> pvect(p->BehaviorProfileLength());
	for (int i = 1; i <= pvect.Length(); i++) {
	  pvect[i] = p[i];
	}
	if (!minimizer.Iterate(F, pvect, fval, gradient, dx)) {
	  break;
	}
	for (int i = 1; i <= pvect.Length(); i++) {
	  p[i] = pvect[i];
	}

	if (sqrt(gradient.NormSquared()) < .001 &&
	    fval < p_tolN) {
	  gbtList<gbtBehavProfile<double> > results;
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
  return gbtList<gbtBehavProfile<double> >();
}
