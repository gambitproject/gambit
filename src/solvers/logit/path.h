//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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

//
// This class implements a generic path-following algorithm for smooth curves.
// It is based on the ideas and codes presented in Allgower and Georg's
// _Numerical Continuation Methods_.
//
class PathTracer {
public:
  //
  // Encapsulates the system of equations to be traversed.
  //
  class EquationSystem {
  public:
    virtual ~EquationSystem() = default;
    // Compute the value of the system of equations at the specified point.
    virtual void GetValue(const Vector<double> &p_point, Vector<double> &p_lhs) const = 0;
    // Compute the Jacobian matrix at the specified point.
    virtual void GetJacobian(const Vector<double> &p_point, Matrix<double> &p_matrix) const = 0;
  };

  //
  // Encapsulates a function to find a zero of when tracing a path.
  //
  class CriterionFunction {
  public:
    virtual ~CriterionFunction() = default;
    virtual double operator()(const Vector<double> &p_point,
                              const Vector<double> &p_tangent) const = 0;
  };

  //
  // A criterion function to pass when not finding a zero of a function.
  //
  class NullCriterionFunction : public CriterionFunction {
  public:
    ~NullCriterionFunction() override = default;
    double operator()(const Vector<double> &, const Vector<double> &) const override
    {
      return -1.0;
    }
  };

  //
  // A function to call on each accepted step of the tracing process.
  //
  class CallbackFunction {
  public:
    virtual ~CallbackFunction() = default;
    virtual void operator()(const Vector<double> &p_point, bool p_isTerminal) const = 0;
  };

  //
  // A callback function to pass when no action required on each step.
  //
  class NullCallbackFunction : public CallbackFunction {
  public:
    ~NullCallbackFunction() override = default;
    void operator()(const Vector<double> &p_point, bool p_isTerminal) const override {}
  };

  void SetMaxDecel(double p_maxDecel) { m_maxDecel = p_maxDecel; }
  double GetMaxDecel() const { return m_maxDecel; }

  void SetStepsize(double p_hStart) { m_hStart = p_hStart; }
  double GetStepsize() const { return m_hStart; }

protected:
  PathTracer() : m_maxDecel(1.1), m_hStart(0.03) {}
  virtual ~PathTracer() = default;

  void TracePath(const EquationSystem &p_system, Vector<double> &p_x, double &p_omega,
                 TerminationFunctionType p_terminate,
                 const CallbackFunction &p_callback = NullCallbackFunction(),
                 const CriterionFunction &p_criterion = NullCriterionFunction()) const;

private:
  double m_maxDecel, m_hStart;
};

} // end namespace Gambit

#endif // PATH_H
