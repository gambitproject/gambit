//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/liap/nfgliap.cc
// Compute Nash equilibria by minimizing Liapunov function
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
//                    class StrategicLyapunovFunction
//------------------------------------------------------------------------

class StrategicLyapunovFunction : public FunctionOnSimplices {
public:
  explicit StrategicLyapunovFunction(const MixedStrategyProfile<double> &p_start)
    : m_game(p_start.GetGame()), m_profile(p_start)
  { }
  ~StrategicLyapunovFunction() override = default;

private:
  Game m_game;
  mutable MixedStrategyProfile<double> m_profile;

  double Value(const Vector<double> &) const override;
  bool Gradient(const Vector<double> &, Vector<double> &) const override;

  double LiapDerivValue(int, int, const MixedStrategyProfile<double> &) const;
};

double 
StrategicLyapunovFunction::LiapDerivValue(int i1, int j1,
					  const MixedStrategyProfile<double> &p) const
{
  GameStrategy wrt_strategy = m_game->GetPlayer(i1)->GetStrategies()[j1];
  double x = 0.0;
  for (int i = 1; i <= m_game->NumPlayers(); i++) {
    double psum = 0.0;
    GamePlayer player = m_game->GetPlayer(i);
    for (int j = 1; j <= player->NumStrategies(); j++) {
      GameStrategy strategy = player->GetStrategies()[j];
      psum += p[strategy];
      double x1 = p.GetPayoff(strategy) - p.GetPayoff(i);
      if (i1 == i) {
        if (x1 > 0.0)
          x -= x1 * p.GetPayoffDeriv(i, wrt_strategy);
      }
      else if (x1 > 0.0) {
        x += x1 * (p.GetPayoffDeriv(i, strategy, wrt_strategy) -
                   p.GetPayoffDeriv(i, wrt_strategy));
      }
    }
    if (i == i1) {
      x += 100.0 * (psum - 1.0);
    }
  }
  if (p[wrt_strategy] < 0.0) {
    x += p[wrt_strategy];
  }
  return 2.0 * x;
}

bool 
StrategicLyapunovFunction::Gradient(const Vector<double> &v, Vector<double> &d) const
{
  m_profile = v;
  for (int pl = 1, ii = 1; pl <= m_game->NumPlayers(); pl++) {
    for (int st = 1; st <= m_game->GetPlayer(pl)->GetStrategies().size(); st++) {
      d[ii++] = LiapDerivValue(pl, st, m_profile);
    }
  }
  Project(d, m_game->NumStrategies());
  return true;
}
  
double StrategicLyapunovFunction::Value(const Vector<double> &v) const
{
  m_profile = v;
  return m_profile.GetLiapValue();
}

//------------------------------------------------------------------------
//                     class NashLiapStrategySolver
//------------------------------------------------------------------------

List<MixedStrategyProfile<double> > 
NashLiapStrategySolver::Solve(const MixedStrategyProfile<double> &p_start) const
{
  if (!p_start.GetGame()->IsPerfectRecall()) {
    throw UndefinedException("Computing equilibria of games with imperfect recall is not supported.");
  }

  static const double ALPHA = .00000001;
  List<MixedStrategyProfile<double> > solutions;

  MixedStrategyProfile<double> p(p_start);
  if (m_verbose) {
    this->m_onEquilibrium->Render(p, "start");
  }

  // if starting vector not interior, perturb it towards centroid
  size_t kk;
  for (kk = 1; kk <= p.MixedProfileLength() && p[kk] > ALPHA; kk++);
  if (kk <= p.MixedProfileLength()) {
    MixedStrategyProfile<double> centroid(p.GetGame()->NewMixedStrategyProfile(0.0));
    for (size_t k = 1; k <= p.MixedProfileLength(); k++) {
      p[k] = centroid[k] * ALPHA + p[k] * (1.0-ALPHA);
    }
  }

  StrategicLyapunovFunction F(p);
  ConjugatePRMinimizer minimizer(p.MixedProfileLength());
  Vector<double> gradient(p.MixedProfileLength()), dx(p.MixedProfileLength());
  double fval;
  minimizer.Set(F, static_cast<const Vector<double> &>(p),
		fval, gradient, .01, .0001);

  for (int iter = 1; iter <= m_maxitsN; iter++) {
    Vector<double> point(p);
    if (!minimizer.Iterate(F, point, fval, gradient, dx)) {
      break;
    }
    p = point;
    if (sqrt(gradient.NormSquared()) < .001) {
      break;
    }
  }

  if (fval < 0.0001) {
    this->m_onEquilibrium->Render(p, "NE");
    solutions.push_back(p);
  }
  else if (m_verbose) {
    this->m_onEquilibrium->Render(p, "end");
  }

  return solutions;
}




