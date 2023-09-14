//
// This file is part of Gambit
// Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/liap/efgliap.cc
// Compute Nash equilibria via Lyapunov function minimization
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

#include "gambit.h"
#include "core/function.h"
#include "liap.h"

using namespace Gambit;

//------------------------------------------------------------------------
//                      class AgentLyapunovFunction
//------------------------------------------------------------------------

class AgentLyapunovFunction : public FunctionOnSimplices {
public:
  explicit AgentLyapunovFunction(const MixedBehaviorProfile<double> &p_start)
    : m_game(p_start.GetGame()), m_profile(p_start)
  { }
  ~AgentLyapunovFunction() override = default;

private:
  Game m_game;
  mutable MixedBehaviorProfile<double> m_profile;

  double Value(const Vector<double> &x) const override;
  bool Gradient(const Vector<double> &, Vector<double> &) const override;
};


double AgentLyapunovFunction::Value(const Vector<double> &x) const
{
  static_cast<Vector<double> &>(m_profile).operator=(x);
  return m_profile.GetLiapValue();
}

bool AgentLyapunovFunction::Gradient(const Vector<double> &x,
                                     Vector<double> &grad) const
{
  const double DELTA = .00001;
  static_cast<Vector<double> &>(m_profile).operator=(x);
  for (int i = 1; i <= x.Length(); i++) {
    m_profile[i] += DELTA;
    double value = m_profile.GetLiapValue();
    m_profile[i] -= 2.0 * DELTA;
    value -= m_profile.GetLiapValue();
    m_profile[i] += DELTA;
    grad[i] = value / (2.0 * DELTA);
  }
  Project(grad, m_game->NumInfosets());
  return true;
}

//------------------------------------------------------------------------
//                    class NashLiapBehaviorSolver
//------------------------------------------------------------------------

List<MixedBehaviorProfile<double> >
NashLiapBehaviorSolver::Solve(const MixedBehaviorProfile<double> &p_start) const
{
  if (!p_start.GetGame()->IsPerfectRecall()) {
    throw UndefinedException("Computing equilibria of games with imperfect recall is not supported.");
  }

  static const double ALPHA = .00000001;
  List<MixedBehaviorProfile<double> > solutions;

  MixedBehaviorProfile<double> p(p_start);
  if (m_verbose) {
    this->m_onEquilibrium->Render(p, "start");
  }

  // if starting vector not interior, perturb it towards centroid
  int kk = 1;
  for (; kk <= p.Length() && p[kk] > ALPHA; kk++);
  if (kk <= p.Length()) {
    MixedBehaviorProfile<double> c(p_start.GetGame());
    for (int k = 1; k <= p.Length(); k++) {
      p[k] = c[k]*ALPHA + p[k]*(1.0-ALPHA);
    }
  }

  AgentLyapunovFunction F(p);
  Matrix<double> xi(p.Length(), p.Length());
  ConjugatePRMinimizer minimizer(p.Length());
  Vector<double> gradient(p.Length()), dx(p.Length());
  double fval;
  minimizer.Set(F, p, fval, gradient, .01, .0001);

  for (int iter = 1; iter <= m_maxitsN; iter++) {
    if (!minimizer.Iterate(F, p, fval, gradient, dx)) {
      break;
    }
    
    if (sqrt(gradient.NormSquared()) < .001) {
      break;
    }
  }

  if (fval < .00001) {
    this->m_onEquilibrium->Render(p, "NE");
    solutions.push_back(p);
  }
  else if (m_verbose) {
    this->m_onEquilibrium->Render(p, "end");
  }

  return solutions;
}

