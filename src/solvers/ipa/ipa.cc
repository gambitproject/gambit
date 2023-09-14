//
// This file is part of Gambit
// Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/ipa/ipa.cc
// Compute Nash equilibria via iterated polymatrix approximation
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
#include "solvers/ipa/ipa.h"
#include "solvers/gtracer/gtracer.h"

using namespace Gambit::gametracer;

namespace Gambit {
namespace Nash {

List<MixedStrategyProfile<double> >
NashIPAStrategySolver::Solve(const Game &p_game) const
{
  Array<double> pert(p_game->MixedProfileLength());
  for (int i = 1; i <= pert.Length(); i++) {
    pert[i] = 1.0;
  }
  return Solve(p_game, pert);
}
  
List<MixedStrategyProfile<double> >
NashIPAStrategySolver::Solve(const Game &p_game,
			     const Array<double> &p_pert) const
{
  if (!p_game->IsPerfectRecall()) {
    throw UndefinedException("Computing equilibria of games with imperfect recall is not supported.");
  }

  std::shared_ptr<gnmgame> A;
  List<MixedStrategyProfile<double> > solutions;
  
  if (p_game->IsAgg()) {
    A = std::make_shared<aggame>(dynamic_cast<GameAggRep &>(*p_game));
  }
  else {
    std::vector<int> actions(p_game->NumPlayers());
    int veclength = p_game->NumPlayers();
    for (int pl = 1; pl <= p_game->NumPlayers(); pl++) {
      actions[pl-1] = p_game->GetPlayer(pl)->NumStrategies();
      veclength *= p_game->GetPlayer(pl)->NumStrategies();
    }
    cvector payoffs(veclength);
  
    A = std::make_shared<nfgame>(p_game->NumPlayers(), actions, payoffs);
  
    std::vector<int> profile(p_game->NumPlayers());
    for (StrategyProfileIterator iter(p_game); !iter.AtEnd(); iter++) {
      for (int pl = 1; pl <= p_game->NumPlayers(); pl++) {
	profile[pl-1] = (*iter)->GetStrategy(pl)->GetNumber() - 1;
      }

      for (int pl = 1; pl <= p_game->NumPlayers(); pl++) {
	A->setPurePayoff(pl-1, profile, (*iter)->GetPayoff(pl));
      }
    }
  }

  cvector g(A->getNumActions()); // perturbation ray
  int numEq;

  cvector ans(A->getNumActions());
  cvector zh(A->getNumActions(),1.0);
  do {
    const double ALPHA = 0.2;
    const double EQERR = 1e-6;

    for (int i = 0; i < A->getNumActions(); i++) {
      g[i] = p_pert[i+1];
    }
    g /= g.norm(); // normalized
    numEq = IPA(*A, g, zh, ALPHA, EQERR, ans);
  } while(numEq == 0);

  MixedStrategyProfile<double> eqm = p_game->NewMixedStrategyProfile(0.0);
  for (int i = 1; i <= eqm.MixedProfileLength(); i++) {
    eqm[i] = ans[i-1];
  }
  m_onEquilibrium->Render(eqm);
  solutions.push_back(eqm);
  return solutions;
}

}  // end namespace Gambit::Nash
}  // end namespace Gambit


    
