//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/gnm/gnm.cc
// Compute Nash equilibria via the global Newton method
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
#include "solvers/gnm/gnm.h"
#include "solvers/gtracer/gtracer.h"

using namespace Gambit::gametracer;

namespace Gambit {
namespace Nash {

MixedStrategyProfile<double>
NashGNMStrategySolver::ToProfile(const Game &p_game,
				 const cvector &p_pert)
{
  MixedStrategyProfile<double> msp = p_game->NewMixedStrategyProfile(0.0);
  for (size_t i = 1; i <= msp.MixedProfileLength(); i++) {
    msp[i] = p_pert[i-1];
  }
  return msp;
}

List<MixedStrategyProfile<double> >
NashGNMStrategySolver::Solve(const Game &p_game,
			     const std::shared_ptr<gnmgame> &p_rep,
			     const cvector &p_pert) const
{
  const int STEPS = 100;
  const double FUZZ = 1e-12;
  const int LNMFREQ = 3;
  const int LNMMAX = 10;
  const double LAMBDAMIN = -10.0;
  const bool WOBBLE = false;
  const double THRESHOLD = 1e-2;

  List<MixedStrategyProfile<double> > eqa;

  if (m_verbose) {
    m_onEquilibrium->Render(ToProfile(p_game, p_pert), "pert");
  }
  cvector norm_pert = p_pert / p_pert.norm();
  std::list<cvector> answers;
  std::string return_message;
  GNM(*p_rep, norm_pert, answers,
      STEPS, FUZZ, LNMFREQ, LNMMAX, LAMBDAMIN, WOBBLE, THRESHOLD,
      m_verbose, return_message);
  for (auto answer: answers) {
    eqa.push_back(ToProfile(p_game, answer));
    m_onEquilibrium->Render(eqa.back());
  }
  return eqa;
}

std::shared_ptr<gnmgame>
NashGNMStrategySolver::BuildRepresentation(const Game &p_game) const
{
  if (p_game->IsAgg()) {
    return std::shared_ptr<gnmgame>(new aggame(dynamic_cast<GameAGGRep &>(*p_game)));
  }
  else {
    Rational maxPay = p_game->GetMaxPayoff();
    Rational minPay = p_game->GetMinPayoff();
    double scale = 1.0 / (maxPay - minPay);

    std::vector<int> actions(p_game->NumPlayers());
    int veclength = p_game->NumPlayers();
    for (int pl = 1; pl <= p_game->NumPlayers(); pl++) {
      actions[pl-1] = p_game->GetPlayer(pl)->NumStrategies();
      veclength *= p_game->GetPlayer(pl)->NumStrategies();
    }
    cvector payoffs(veclength);
  
    std::shared_ptr<gnmgame> A(new nfgame(actions, payoffs));
  
    std::vector<int> profile(p_game->NumPlayers());
    for (StrategyProfileIterator iter(p_game); !iter.AtEnd(); iter++) {
      for (int pl = 1; pl <= p_game->NumPlayers(); pl++) {
        profile[pl-1] = (*iter)->GetStrategy(pl)->GetNumber() - 1;
      }

      for (int pl = 1; pl <= p_game->NumPlayers(); pl++) {
        A->setPurePayoff(pl-1, profile,
		       (double) ((*iter)->GetPayoff(pl) - minPay) *
		       scale);
      }
    }
    return A;
  }
}
 
List<MixedStrategyProfile<double> >
NashGNMStrategySolver::Solve(const Game &p_game) const
{
  if (!p_game->IsPerfectRecall()) {
    throw UndefinedException("Computing equilibria of games with imperfect recall is not supported.");
  }

  std::shared_ptr<gnmgame> A = BuildRepresentation(p_game);
  cvector g(A->getNumActions()); 
  g[0] = 1.0;
  for (int i = 1; i < A->getNumActions(); i++) {
    g[i] = 0.0;
  }
  return Solve(p_game, A, g);
}

List<MixedStrategyProfile<double> >
NashGNMStrategySolver::Solve(const Game &p_game,
			     const MixedStrategyProfile<double> &p_pert) const
{
  if (!p_game->IsPerfectRecall()) {
    throw UndefinedException("Computing equilibria of games with imperfect recall is not supported.");
  }

  List<MixedStrategyProfile<double> > solutions;
  std::shared_ptr<gnmgame> A = BuildRepresentation(p_game);
  cvector g(A->getNumActions());
  for (int i = 0; i < A->getNumActions(); i++) {
    g[i] = p_pert[i+1];
  }
  g /= g.norm();
  return Solve(p_game, A, g);
}

}  // end namespace Gambit::Nash
}  // end namespace Gambit
