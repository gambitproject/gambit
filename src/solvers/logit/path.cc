//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/tools/logit/path.cc
// Implementation of generic smooth path-following algorithm.
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
#include <algorithm> // for std::max

#include "gambit.h"
#include "path.h"

namespace Gambit {

//----------------------------------------------------------------------------
//                       PathTracer: Auxiliary functions
//----------------------------------------------------------------------------

namespace {

inline double sqr(double x) { return x * x; }

void Givens(Matrix<double> &b, Matrix<double> &q, double &c1, double &c2, int l1, int l2, int l3)
{
  if (std::abs(c1) + std::abs(c2) == 0.0) {
    return;
  }

  double sn;
  if (std::abs(c2) >= std::abs(c1)) {
    sn = std::sqrt(1.0 + sqr(c1 / c2)) * std::abs(c2);
  }
  else {
    sn = std::sqrt(1.0 + sqr(c2 / c1)) * std::abs(c1);
  }
  const double s1 = c1 / sn;
  const double s2 = c2 / sn;

  for (size_t k = 1; k <= q.NumColumns(); k++) {
    const double sv1 = q(l1, k);
    const double sv2 = q(l2, k);
    q(l1, k) = s1 * sv1 + s2 * sv2;
    q(l2, k) = -s2 * sv1 + s1 * sv2;
  }

  for (size_t k = l3; k <= b.NumColumns(); k++) {
    const double sv1 = b(l1, k);
    const double sv2 = b(l2, k);
    b(l1, k) = s1 * sv1 + s2 * sv2;
    b(l2, k) = -s2 * sv1 + s1 * sv2;
  }

  c1 = sn;
  c2 = 0.0;
}

void SetAsIdentity(Matrix<double> &M)
{
  M = 0.0;
  for (int i = M.MinRow(); i <= M.MaxRow(); ++i) {
    M(i, i) = 1.0;
  }
}

void QRDecomp(Matrix<double> &b, Matrix<double> &q)
{
  SetAsIdentity(q);
  for (size_t m = 1; m <= b.NumColumns(); m++) {
    for (size_t k = m + 1; k <= b.NumRows(); k++) {
      Givens(b, q, b(m, m), b(k, m), m, k, m + 1);
    }
  }
}

void NewtonStep(Matrix<double> &q, Matrix<double> &b, Vector<double> &u, Vector<double> &y,
                double &d)
{
  for (size_t k = 1; k <= b.NumColumns(); k++) {
    for (size_t l = 1; l <= k - 1; l++) {
      y[k] -= b(l, k) * y[l];
    }
    y[k] /= b(k, k);
  }

  d = 0.0;
  for (size_t k = 1; k <= b.NumRows(); k++) {
    double s = 0.0;
    for (size_t l = 1; l <= b.NumColumns(); l++) {
      s += q(l, k) * y[l];
    }
    u[k] -= s;
    d += s * s;
  }
  d = std::sqrt(d);
}

} // end anonymous namespace

//----------------------------------------------------------------------------
//             PathTracer: Implementation of path-following engine
//----------------------------------------------------------------------------

// To handle possible (simple) bifurcations in the graph, TracePath detects a
// change in orientation of the curve, and, if one is found, implements a
// perturbation on the first equation.  This perturbation is maintained only
// long enough to traverse past the apparent bifurcation.
// Preliminary experience suggests this works fairly well (when applied to QRE).
// It is possible for the path-following to land sufficiently close to the
// bifurcation point that the tracing gets stuck there as it is not possible
// to find a small enough step size to avoid stepping over the bifurcation
// point.
TracePathResult
PathTracer::TracePath(std::function<void(const Vector<double> &, Vector<double> &)> p_function,
                      std::function<void(const Vector<double> &, Matrix<double> &)> p_jacobian,
                      Vector<double> &x, TraceDirection p_direction, size_t p_trackingIndex,
                      TerminationFunctionType p_terminate, CallbackFunctionType p_callback,
                      CriterionFunctionType p_criterion,
                      CriterionBracketFunctionType p_criterionBracket) const
{
  const double c_tol = 1.0e-4;       // tolerance for corrector iteration
  const double c_maxDist = 0.4;      // maximal distance to curve
  const double c_maxContr = 0.6;     // maximal contraction rate in corrector
  const double c_eta = 0.1;          // perturbation to avoid cancellation
                                     // in calculating contraction rate
  double h = m_hStart;               // initial stepsize
  const double c_hmin = 1.0e-8;      // minimal stepsize
  const int c_maxIter = 100;         // maximum iterations in corrector
  const double c_newtonTol = 1.0e-8; // tolerance for Newton convergence

  bool newton = false;             // using Newton steplength (for zero-finding)
  const double c_pert = 0.0000001; // The size of perturbation to apply to avoid bifurcation traps
  double pert = 0.0;               // The current version of the perturbation being applied
  double pert_countdown = 0.0;     // How much longer (in arclength) to apply perturbation
  const double c_orientTol = 1.0e-8; // tolerance for detecting change in orientation

  Vector<double> u(x.size());
  // t is current tangent at x; newT is tangent at u, which is the next point.
  Vector<double> t(x.size()), newT(x.size());
  Vector<double> y(x.size() - 1);
  Matrix<double> b(x.size(), x.size() - 1);
  Matrix<double> q(x.size(), x.size());

  p_jacobian(x, b);
  QRDecomp(b, q);
  q.GetRow(q.NumRows(), t);
  p_callback(x);
  int steps = 0;

  bool first_step = true;
  double omega = (p_direction == TraceDirection::Positive) ? 1.0 : -1.0;

  if (p_trackingIndex > x.size() || p_trackingIndex < 1) {
    return {x, false, "Tracking index exceeds dimension of point vector.", steps};
  }

  while (!p_terminate(x)) {
    bool accept = true;

    if (std::abs(h) <= c_hmin) {
      if (newton && std::abs(p_criterion(x, t)) < c_newtonTol) {
        return {x, true,
                "Path following terminated successfully at point satisfying criterion function.",
                steps};
      }
      else {
        return {x, false, "Stepsize fell below minimum threshold.", steps};
      }
    }

    if (first_step) {
      if (std::abs(t[p_trackingIndex]) <= c_orientTol) {
        return {x, false, "Initial tangent vector is orthogonal to path-following direction.",
                steps};
      }
      // Ensure that the tangent is oriented in the same direction as
      // the path-following direction.
      else if (t[p_trackingIndex] < -c_orientTol) {
        omega *= -1.0;
      }
      first_step = false;
    }

    // Predictor step
    for (size_t k = 1; k <= x.size(); k++) {
      u[k] = x[k] + h * omega * t[k];
    }

    double decel = 1.0 / m_maxDecel; // initialize deceleration factor
    p_jacobian(u, b);
    QRDecomp(b, q);

    int iter = 1;
    double disto = 0.0;
    while (true) {
      double dist;

      p_function(u, y);
      y[1] += pert;
      NewtonStep(q, b, u, y, dist);

      if (dist >= c_maxDist) {
        accept = false;
        break;
      }

      decel = std::max(decel, std::sqrt(dist / c_maxDist) * m_maxDecel);
      if (iter >= 2) {
        const double contr = dist / (disto + c_tol * c_eta);
        if (contr > c_maxContr) {
          accept = false;
          break;
        }
        decel = std::max(decel, std::sqrt(contr / c_maxContr) * m_maxDecel);
      }

      if (dist <= c_tol) {
        // Success; break out of iteration
        break;
      }
      disto = dist;
      iter++;
      if (iter > c_maxIter) {
        return {x, false, "Maximum iterations exceeded.", steps};
      }
    }

    // Obtain the tangent at the next step
    q.GetRow(q.NumRows(), newT);
    const double omega_flip = (t * newT < 0.0) ? -1.0 : 1.0;

    if (omega_flip == -1.0) {
      // The orientation of the curve has changed, indicating a bifurcation.
      // Switch on perturbation and attempt to continue following the branch that
      // is oriented in the same direction as we were originally following
      if (pert_countdown == 0.0) {
        pert = c_pert;
        pert_countdown = std::abs(2 * h);
      }
      accept = false;
    }

    if (!accept) {
      h /= m_maxDecel; // PC not accepted; change stepsize and retry
      if (std::abs(h) <= c_hmin) {
        if (newton && std::abs(p_criterion(x, t)) < c_newtonTol) {
          return {x, true,
                  "Path following terminated successfully at point satisfying criterion function.",
                  steps};
        }
        else {
          return {x, false, "Stepsize fell below minimum threshold.", steps};
        }
      }
      continue;
    }

    // Determine new stepsize
    decel = std::min(decel, m_maxDecel);

    // If we are at a bifurcation point, the orientation of the tangent
    // will flip.  This will confuse many criterion functions, especially
    // those which are using derivatives to maximize or minimize an objective.
    // This ensures the criterion function is called with both the old and
    // new tangent oriented in the same sense.
    if (!newton && p_criterion(x, t) * p_criterion(u, newT * omega_flip) < 0.0) {
      newton = true;
      p_criterionBracket(x, u);
    }

    if (newton) {
      // Newton-type steplength adaptation, secant method
      h *= -p_criterion(u, newT) / (p_criterion(u, newT) - p_criterion(x, t));
    }
    else {
      // Standard steplength adaptation
      h = std::abs(h / decel);
    }

    // PC step was successful; update and iterate
    x = u;
    t = newT;
    p_callback(x);
    steps++;

    if (pert_countdown > 0.0) {
      // If we are currently perturbing in the neighborhood of a bifurcation, check to see
      // whether we think we are likely past it, and switch off if we are.
      pert_countdown -= abs(h);
      if (pert_countdown < 0.0) {
        pert = 0.0;
        pert_countdown = 0.0;
      }
    }
  }
  return {x, true, "Path tracing terminated successfully.", steps};
}

PolishResult PolishPoint(std::function<void(const Vector<double> &, Vector<double> &)> p_function,
                         std::function<void(const Vector<double> &, Matrix<double> &)> p_jacobian,
                         Vector<double> &x, double fixed_value, size_t fixed_index,
                         TerminationFunctionType p_terminate, int max_iter,
                         CallbackFunctionType p_callback)
{
  x[fixed_index] = fixed_value;

  const size_t N = x.size() - 1;
  Vector<double> y(N);               // Equations results
  Matrix<double> jac_full(N + 1, N); // Full Jacobian matrix (N+1 unknowns, N equations)
  Matrix<double> jac_square(N, N);   // Jacobian matrix with fixed_index row removed
  Matrix<double> Q(N, N);            // Orthogonal matrix from QR decomposition
  Vector<double> x_reduced(N);       // Reduced x vector with fixed_index removed

  int steps = 0;
  double dist = 0.0;

  while (!p_terminate(x)) {
    if (steps >= max_iter) {
      return {x, false, "Polishing exceeded maximum iterations.", steps};
    }

    p_function(x, y);
    p_jacobian(x, jac_full);

    size_t row_index = 1;
    for (size_t i = 1; i <= N + 1; ++i) { // Newton step expects the transposed Jacobian
      if (i != fixed_index) {
        for (size_t j = 1; j <= N; ++j) {
          jac_square(row_index, j) = jac_full(i, j);
        }
        row_index++;
      }
    }

    // Reduced x vector removing fixed_index
    size_t temp_idx = 1;
    for (size_t i = 1; i <= N + 1; ++i) {
      if (i != fixed_index) {
        x_reduced[temp_idx++] = x[i];
      }
    }

    QRDecomp(jac_square, Q);

    // Solve jac_square * x_reduced = -y
    NewtonStep(Q, jac_square, x_reduced, y, dist);

    // Update x, keeping fixed_index constant
    temp_idx = 1;
    for (size_t i = 1; i <= N + 1; ++i) {
      if (i != fixed_index) {
        x[i] = x_reduced[temp_idx++];
      }
    }

    steps++;

    if (p_callback) {
      p_callback(x);
    }
  }

  return {x, true, "Polishing terminated successfully.", steps};
}
} // end namespace Gambit
