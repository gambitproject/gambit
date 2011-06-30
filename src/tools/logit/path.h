//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
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

using namespace Gambit;

//
// This class implements a generic path-following algorithm for smooth curves.
// It is based on the ideas and codes presented in Allgower and Georg's
// _Numerical Continuation Methods_.
//
class PathTracer {
public:
  void SetMaxDecel(double p_maxDecel) { m_maxDecel = p_maxDecel; }
  double GetMaxDecel(void) const { return m_maxDecel; }

  void SetStepsize(double p_hStart) { m_hStart = p_hStart; }
  double GetStepsize(void) const { return m_hStart; }

  void SetTargetParam(double p_targetParam) { m_targetParam = p_targetParam; }
  double GetTargetParam(void) const { return m_targetParam; }

protected:
  PathTracer(void) : m_maxDecel(1.1), m_hStart(0.03), m_targetParam(0.0) 
    { } 
  virtual ~PathTracer() { }

  void TracePath(Vector<double> &p_x, double p_maxLambda, double &p_omega);

  // Criterion function: path tracer attempts to compute a zero of this function.
  virtual double Criterion(const Vector<double> &p_point, 
			   const Vector<double> &p_tangent) { return -1.0; }

  // Called on each accepted step of the tracing process.
  virtual void OnStep(const Vector<double> &, bool) = 0;

  // Compute the LHS of the system of equations at the specified point.
  virtual void GetLHS(const Vector<double> &p_point, Vector<double> &p_lhs) = 0;
  // Compute the Jacobian matrix at the specified point.
  virtual void GetJacobian(const Vector<double> &p_point, Matrix<double> &p_matrix) = 0;

private:
  double m_maxDecel, m_hStart, m_targetParam;
};

#endif  // PATH_H
