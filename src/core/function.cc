//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/function.cc
// Implementation of function and function minimization routines
//
// These routines derive from the N-dimensional function minimization
// routines in the GNU Scientific Library, version 1.2, which is
// Copyright (C) 1996, 1997, 1998, 1999, 2000 Fabrice Rossi
// and is released under the terms of the GNU General Public License.
// Modifications consist primarily of converting the routines to
// use Gambit's vector and function classes.
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
#include "gambit.h"
#include "function.h"

using namespace Gambit;

//========================================================================
//                      class FunctionOnSimplices
//========================================================================

//
// Project a gradient into the plane of the product of simplices.
// (Actually, it works by computing the projection of 'x' onto the
// vector perpendicular to the plane, then subtracting to compute the
// component parallel to the plane.)
//
void FunctionOnSimplices::Project(Vector<double> &x, const Array<int> &lengths) const
{
  int index = 1;
  for (int part = 1; part <= lengths.Length(); part++) {
    double avg = 0.0;
    int j;
    for (j = 1; j <= lengths[part]; j++, index++) {
      avg += x[index];
    }
    avg /= (double)lengths[part];
    index -= lengths[part];
    for (j = 1; j <= lengths[part]; j++, index++) {
      x[index] -= avg;
    }
  }
}

//========================================================================
//               Conjugate gradient Polak-Ribiere algorithm
//========================================================================

//
// These routines are based on ones found in
// multimin/conjugate_pr.c in the GSL 1.2 distribution

void ConjugatePRMinimizer::AlphaXPlusY(double alpha, const Vector<double> &x, Vector<double> &y)
{
  for (int i = 1; i <= y.Length(); i++) {
    y[i] += alpha * x[i];
  }
}

// These routines are drawn from comparably-named ones in
// multimin/directional_minimize.c in GSL.

void ConjugatePRMinimizer::TakeStep(const Vector<double> &x, const Vector<double> &p, double step,
                                    double lambda, Vector<double> &x1, Vector<double> &dx)
{
  dx = 0.0;
  AlphaXPlusY(-step * lambda, p, dx);
  x1 = x;
  AlphaXPlusY(1.0, dx, x1);
}

void ConjugatePRMinimizer::IntermediatePoint(const Function &fdf, const Vector<double> &x,
                                             const Vector<double> &p, double lambda, double pg,
                                             double stepa, double stepc, double fa, double fc,
                                             Vector<double> &x1, Vector<double> &dx,
                                             Vector<double> &gradient, double &step, double &f)
{
  double stepb, fb;

trial:
  double u = fabs(pg * lambda * stepc);
  if ((fc - fa) + u == 0) {
    // TLT: Added this check 2002/08/31 due to floating point exceptions
    // under MSW.  Not really sure how to handle this correctly.
    throw FunctionMinimizerError();
  }
  stepb = 0.5 * stepc * u / ((fc - fa) + u);

  TakeStep(x, p, stepb, lambda, x1, dx);

  fb = fdf.Value(x1);

  if (fb >= fa && stepb > 0.0) {
    /* downhill step failed, reduce step-size and try again */
    fc = fb;
    stepc = stepb;
    goto trial;
  }

  step = stepb;
  f = fb;
  fdf.Gradient(x1, gradient);
}

void ConjugatePRMinimizer::Minimize(const Function &fdf, const Vector<double> &x,
                                    const Vector<double> &p, double lambda, double stepa,
                                    double stepb, double stepc, double fa, double fb, double fc,
                                    double tol, Vector<double> &x1, Vector<double> &dx1,
                                    Vector<double> &x2, Vector<double> &dx2,
                                    Vector<double> &gradient, double &step, double &f,
                                    double &gnorm)
{
  /* Starting at (x0, f0) move along the direction p to find a minimum
     f(x0 - lambda * p), returning the new point x1 = x0-lambda*p,
     f1=f(x1) and g1 = grad(f) at x1.  */

  double u = stepb;
  double v = stepa;
  double w = stepc;
  double fu = fb;
  double fv = fa;
  double fw = fc;

  double old2 = fabs(w - v);
  double old1 = fabs(v - u);

  double stepm, fm, pg, gnorm1;

  double iter = 0;

  x2 = x1;
  dx2 = dx1;

  f = fb;
  step = stepb;
  gnorm = std::sqrt(gradient.NormSquared());

mid_trial:
  iter++;

  if (iter > 10) {
    return; /* MAX ITERATIONS */
  }

  double dw = w - u;
  double dv = v - u;
  double du = 0.0;

  double e1 = ((fv - fu) * dw * dw + (fu - fw) * dv * dv);
  double e2 = 2.0 * ((fv - fu) * dw + (fu - fw) * dv);

  if (e2 != 0.0) {
    du = e1 / e2;
  }

  if (du > 0 && du < (stepc - stepb) && fabs(du) < 0.5 * old2) {
    stepm = u + du;
  }
  else if (du < 0 && du > (stepa - stepb) && fabs(du) < 0.5 * old2) {
    stepm = u + du;
  }
  else if ((stepc - stepb) > (stepb - stepa)) {
    stepm = 0.38 * (stepc - stepb) + stepb;
  }
  else {
    stepm = stepb - 0.38 * (stepb - stepa);
  }

  TakeStep(x, p, stepm, lambda, x1, dx1);

  fm = fdf.Value(x1);

  if (fm > fb) {
    if (fm < fv) {
      w = v;
      v = stepm;
      fw = fv;
      fv = fm;
    }
    else if (fm < fw) {
      w = stepm;
      fw = fm;
    }

    if (stepm < stepb) {
      stepa = stepm;
      fa = fm;
    }
    else {
      stepc = stepm;
      fc = fm;
    }
    goto mid_trial;
  }
  else if (fm <= fb) {
    old2 = old1;
    old1 = fabs(u - stepm);
    w = v;
    v = u;
    u = stepm;
    fw = fv;
    fv = fu;
    fu = fm;

    x2 = x1;
    dx2 = dx1;

    fdf.Gradient(x1, gradient);
    pg = p * gradient;
    gnorm1 = std::sqrt(gradient.NormSquared());

    f = fm;
    step = stepm;
    gnorm = gnorm1;

    if (gnorm1 == 0.0) {
      return;
    }

    if (fabs(pg * lambda / gnorm1) < tol) {
      return; /* SUCCESS */
    }

    if (stepm < stepb) {
      stepc = stepb;
      fc = fb;
      stepb = stepm;
      fb = fm;
    }
    else {
      stepa = stepb;
      fa = fb;
      stepb = stepm;
      fb = fm;
    }
    goto mid_trial;
  }
}

ConjugatePRMinimizer::ConjugatePRMinimizer(int n) : x1(n), dx1(n), x2(n), p(n), g0(n) {}

void ConjugatePRMinimizer::Set(const Function &fdf, const Vector<double> &x, double &f,
                               Vector<double> &gradient, double step_size, double p_tol)
{
  iter = 0;
  step = step_size;
  max_step = step_size;
  m_tol = p_tol;

  f = fdf.Value(x);
  fdf.Gradient(x, gradient);

  /* Use the gradient as the initial direction */
  p = gradient;
  g0 = gradient;

  double gnorm = std::sqrt(gradient.NormSquared());
  pnorm = gnorm;
  g0norm = gnorm;
}

void ConjugatePRMinimizer::Restart() { iter = 0; }

bool ConjugatePRMinimizer::Iterate(const Function &fdf, Vector<double> &x, double &f,
                                   Vector<double> &gradient, Vector<double> &dx)
{
  double fa = f, fb, fc;
  double dir;
  double stepa = 0.0, stepb, stepc = step, tol = m_tol;

  double g1norm;
  double pg;

  if (pnorm == 0.0 || g0norm == 0.0) {
    dx = 0.0;
    return false;
  }

  /* Determine which direction is downhill, +p or -p */
  pg = p * gradient;
  dir = (pg >= 0.0) ? +1.0 : -1.0;

  /* Compute new trial point at x_c= x - step * p, where p is the
     current direction */
  TakeStep(x, p, stepc, dir / pnorm, x1, dx);

  /* Evaluate function and gradient at new point xc */
  fc = fdf.Value(x1);

  if (fc < fa) {
    /* Success, reduced the function value */
    step = stepc * 2.0;
    f = fc;
    x = x1;
    fdf.Gradient(x1, gradient);
    g0norm = std::sqrt(gradient.NormSquared());
    return true;
  }

  /* Do a line minimisation in the region (xa,fa) (xc,fc) to find an
     intermediate (xb,fb) satisifying fa > fb < fc.  Choose an initial
     xb based on parabolic interpolation */
  IntermediatePoint(fdf, x, p, dir / pnorm, pg, stepa, stepc, fa, fc, x1, dx1, gradient, stepb,
                    fb);

  if (stepb == 0.0) {
    return false;
  }

  Minimize(fdf, x, p, dir / pnorm, stepa, stepb, stepc, fa, fb, fc, tol, x1, dx1, x2, dx, gradient,
           step, f, g1norm);
  x = x2;

  /* Choose a new conjugate direction for the next step */
  iter = (iter + 1) % x.Length();

  if (iter == 0) {
    p = gradient;
    pnorm = g1norm;
  }
  else {
    /* p' = g1 - beta * p */
    double g0g1, beta;
    AlphaXPlusY(-1.0, gradient, g0); /* g0' = g0 - g1 */
    g0g1 = g0 * gradient;            /* g1g0 = (g0-g1).g1 */
    beta = g0g1 / (g0norm * g0norm); /* beta = -((g1 - g0).g1)/(g0.g0) */

    p *= -beta;
    AlphaXPlusY(1.0, gradient, p);
    pnorm = std::sqrt(p.NormSquared());
  }

  g0norm = g1norm;
  g0 = gradient;

  return true;
}
