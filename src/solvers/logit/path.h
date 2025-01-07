//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/tools/logit/path.h
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

#ifndef PATH_H
#define PATH_H

#include <functional>

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

//
// This class implements a generic path-following algorithm for smooth curves.
// It is based on the ideas and codes presented in Allgower and Georg's
// _Numerical Continuation Methods_.
//
class PathTracer {
public:
  PathTracer() : m_maxDecel(1.1), m_hStart(0.03) {}
  virtual ~PathTracer() = default;

  void SetMaxDecel(double p_maxDecel) { m_maxDecel = p_maxDecel; }
  double GetMaxDecel() const { return m_maxDecel; }

  void SetStepsize(double p_hStart) { m_hStart = p_hStart; }
  double GetStepsize() const { return m_hStart; }

  void
  TracePath(std::function<void(const Vector<double> &, Vector<double> &)> p_function,
            std::function<void(const Vector<double> &, Matrix<double> &)> p_jacobian,
            Vector<double> &p_x, double &p_omega, TerminationFunctionType p_terminate,
            CallbackFunctionType p_callback = NullCallbackFunction,
            CriterionFunctionType p_criterion = NullCriterionFunction,
            CriterionBracketFunctionType p_criterionBracker = NullCriterionBracketFunction) const;

private:
  double m_maxDecel, m_hStart;
};

} // end namespace Gambit

#endif // PATH_H
