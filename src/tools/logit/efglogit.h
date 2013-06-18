//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/logit/efglogit.h
// Computation of agent quantal response equilibrium correspondence for
// extensive games.
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

#ifndef EFGLOGIT_H
#define EFGLOGIT_H

#include "path.h"

class Equation;

class AgentQREPathTracer : public PathTracer {
public:
  AgentQREPathTracer(const MixedBehavProfile<double> &p_start);
  virtual ~AgentQREPathTracer();

  void 
  TraceAgentPath(const MixedBehavProfile<double> &p_start,
		 double p_startLambda, double p_maxLambda, double p_omega);

  void SetFullGraph(bool p_fullGraph) { m_fullGraph = p_fullGraph; }
  bool GetFullGraph(void) const { return m_fullGraph; }

  void SetDecimals(int p_decimals) { m_decimals = p_decimals; }
  int GetDecimals(void) const { return m_decimals; }

protected:
  virtual void OnStep(const Vector<double> &, bool);

  virtual double Criterion(const Vector<double> &, const Vector<double> &);

  // Compute the LHS of the system of equations at the specified point.
  virtual void GetLHS(const Vector<double> &p_point, Vector<double> &p_lhs);
  // Compute the Jacobian matrix at the specified point.
  virtual void GetJacobian(const Vector<double> &p_point, Matrix<double> &p_matrix);

private:
  MixedBehavProfile<double> m_start;
  Array<Equation *> m_equations;
  bool m_fullGraph;
  int m_decimals;

  void PrintProfile(std::ostream &p_stream, const Vector<double> &x,
		    bool p_isTerminal);
};


#endif  // EFGLOGIT_H
