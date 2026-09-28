//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/path/path.h
// Interface to generic smooth path-following algorithm.
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

#ifndef GAMBIT_SOLVERS_LOGIT_PATH_H
#define GAMBIT_SOLVERS_LOGIT_PATH_H

#include <functional>
#include "core/cancel.h"

namespace Gambit {

// Function type used for determining whether to terminate the numerical continuation
using TerminationFunctionType = std::function<bool(const Vector<double> &)>;

inline bool LambdaPositiveTerminationFunction(const Vector<double> &p_point)
{
  return (p_point.back() < 0.0);
}

inline bool LambdaRangeTerminationFunction(const Vector<double> &p_point, double p_minLambda,
                                           double p_maxLambda)
{
  return (p_point.back() < p_minLambda || p_point.back() > p_maxLambda);
}

using CriterionFunctionType =
    std::function<double(const Vector<double> &, const Vector<double> &)>;

inline double NullCriterionFunction(const Vector<double> &, const Vector<double> &)
{
  return -1.0;
}

using CriterionBracketFunctionType =
    std::function<void(const Vector<double> &, const Vector<double> &)>;

inline void NullCriterionBracketFunction(const Vector<double> &, const Vector<double> &) {}

using CallbackFunctionType = std::function<void(const Vector<double> &)>;

inline void NullCallbackFunction(const Vector<double> &) {}

// Called once per detected bifurcation, with the last-accepted point and the
// rejected point where the change in tangent orientation was observed.
using BifurcationBracketFunctionType =
    std::function<void(const Vector<double> &, const Vector<double> &)>;

inline void NullBifurcationBracketFunction(const Vector<double> &, const Vector<double> &) {}

// Called when the symmetry-breaking perturbation used to traverse a suspected
// bifurcation is switched on (true) or off (false), with the current point.
using PerturbationEventFunctionType = std::function<void(bool, const Vector<double> &)>;

inline void NullPerturbationEventFunction(bool, const Vector<double> &) {}

/// @brief Counts and wall-clock timings recorded by one call to TracePath() or PolishPoint()
struct TracePathStats {
  size_t predictor_attempts{0};   ///< predictor steps tried, whether accepted or rejected
  size_t accepted_steps{0};       ///< predictor-corrector steps accepted
  size_t rejected_distance{0};    ///< steps rejected because a corrector step was too long
  size_t rejected_contraction{0}; ///< steps rejected because the corrector contracted too slowly
  size_t rejected_orientation{0}; ///< steps rejected because the tangent orientation flipped
  size_t corrector_iterations{0}; ///< Newton corrector iterations, over all attempts
  size_t function_evals{0};       ///< evaluations of the system of equations
  size_t jacobian_evals{0};       ///< evaluations of the Jacobian
  size_t factorizations{0};       ///< QR decompositions of the Jacobian

  double function_seconds{0.0};      ///< time evaluating the system of equations
  double jacobian_seconds{0.0};      ///< time evaluating the Jacobian
  double factorization_seconds{0.0}; ///< time in QR decompositions
  double newton_seconds{0.0};        ///< time in Newton corrector steps
  double terminate_seconds{0.0};     ///< time in the termination function
  double callback_seconds{0.0};      ///< time in the per-point callback
  double total_seconds{0.0};         ///< total time in the call
};

struct TracePathResult {
  Vector<double> final_point;
  bool status; // true if path tracing terminated successfully, false if it terminated due to error
  std::string message; // error message if status is false
  int steps;           // Step at which the tracing terminated
  TracePathStats stats;
};

struct PolishResult {
  Vector<double> final_point;
  bool status; // true if polishing terminated successfully, false if it terminated due to error
  std::string message; // error message if status is false
  int steps;           // Step at which the polishing terminated
  TracePathStats stats;
};
//
// This class implements a generic path-following algorithm for smooth curves.
// It is based on the ideas and codes presented in Allgower and Georg's
// _Numerical Continuation Methods_.
//
class PathTracer {
public:
  enum class TraceDirection { Positive = 1, Negative = -1 };
  PathTracer() = default;
  virtual ~PathTracer() = default;

  void SetMaxDecel(double p_maxDecel) { m_maxDecel = p_maxDecel; }
  double GetMaxDecel() const { return m_maxDecel; }

  void SetStepsize(double p_hStart) { m_hStart = p_hStart; }
  double GetStepsize() const { return m_hStart; }

  TracePathResult
  TracePath(std::function<void(const Vector<double> &, Vector<double> &)> p_function,
            std::function<void(const Vector<double> &, Matrix<double> &)> p_jacobian,
            Vector<double> &p_x, TraceDirection p_direction, size_t p_trackingIndex,
            TerminationFunctionType p_terminate,
            CallbackFunctionType p_callback = NullCallbackFunction,
            CriterionFunctionType p_criterion = NullCriterionFunction,
            CriterionBracketFunctionType p_criterionBracker = NullCriterionBracketFunction,
            const CancelToken &p_cancel = CancelToken(),
            BifurcationBracketFunctionType p_onBifurcation = NullBifurcationBracketFunction,
            PerturbationEventFunctionType p_onPerturbation = NullPerturbationEventFunction) const;

private:
  double m_maxDecel{1.1}, m_hStart{0.03};
};

// This function reduces the regret of a point that is close to an equilibrium that has been found
// by the path-following algorithm.  Fixing the value of a component of the point, it uses a
// Newton-type method to find a nearby point with lower regret.
PolishResult PolishPoint(std::function<void(const Vector<double> &, Vector<double> &)> p_function,
                         std::function<void(const Vector<double> &, Matrix<double> &)> p_jacobian,
                         Vector<double> &p_x, double fixed_value, size_t fixed_index,
                         TerminationFunctionType p_terminate, int max_iter = 100,
                         CallbackFunctionType p_callback = NullCallbackFunction);

} // end namespace Gambit

#endif // GAMBIT_SOLVERS_LOGIT_PATH_H
