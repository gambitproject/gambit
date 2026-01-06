//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/tools/enummixed/enummixed.cc
// Compute Nash equilibria via Mangasarian's algorithm
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
#include "solvers/linalg/vertenum.imp"
#include "solvers/enummixed/enummixed.h"
#include "clique.h"

namespace Gambit::Nash {

using namespace Gambit::linalg;

bool EqZero(const double &x)
{
  const double eps = ::pow(10.0, -15.0);
  return (x <= eps && x >= -eps);
}

bool EqZero(const Rational &x) { return x == Rational(0); }

template <class T>
Array<Array<MixedStrategyProfile<T>>> EnumMixedStrategySolution<T>::GetCliques() const
{
  if (m_cliques1.empty()) {
    // Cliques are generated on demand
    auto n = m_node1.size();
    if (m_node2.size() != n) {
      throw DimensionException();
    }

    Array<CliqueEnumerator::Edge> edgelist;
    edgelist.reserve(n);
    std::transform(m_node1.begin(), m_node1.end(), m_node2.begin(), std::back_inserter(edgelist),
                   [](const int a, const int b) { return CliqueEnumerator::Edge(a, b); });

    const CliqueEnumerator clique(edgelist, m_v2 + 1, m_v1 + 1);
    m_cliques1 = clique.GetCliques1();
    m_cliques2 = clique.GetCliques2();
  }

  Array<Array<MixedStrategyProfile<T>>> solution;
  for (size_t cl = 1; cl <= m_cliques1.size(); cl++) {
    solution.push_back(Array<MixedStrategyProfile<T>>());
    for (size_t i = 1; i <= m_cliques1[cl].size(); i++) {
      for (size_t j = 1; j <= m_cliques2[cl].size(); j++) {
        MixedStrategyProfile<T> profile(m_game->NewMixedStrategyProfile(static_cast<T>(0)));

        for (size_t k = 1; k <= m_key1[m_cliques1[cl][i]].size(); k++) {
          profile[k] = m_key1[m_cliques1[cl][i]][k];
        }
        for (size_t k = 1; k <= m_key2[m_cliques2[cl][j]].size(); k++) {
          profile[k + m_key1[m_cliques1[cl][i]].size()] = m_key2[m_cliques2[cl][j]][k];
        }
        solution[cl].push_back(profile);
      }
    }
  }
  return solution;
}

template <class T>
std::shared_ptr<EnumMixedStrategySolution<T>>
EnumMixedStrategySolveDetailed(const Game &p_game, StrategyCallbackType<T> p_onEquilibrium)
{
  if (p_game->NumPlayers() != 2) {
    throw UndefinedException("Method only valid for two-player games.");
  }
  if (!p_game->IsPerfectRecall()) {
    throw UndefinedException(
        "Computing equilibria of games with imperfect recall is not supported.");
  }
  std::shared_ptr<EnumMixedStrategySolution<T>> solution(new EnumMixedStrategySolution<T>(p_game));

  const PureStrategyProfile profile = p_game->NewPureStrategyProfile();

  Rational min = p_game->GetMinPayoff();
  if (min > Rational(0)) {
    min = Rational(0);
  }
  min -= Rational(1);

  Rational max = p_game->GetMaxPayoff();
  if (max < Rational(0)) {
    max = Rational(0);
  }

  const Rational fac(1, max - min);

  // Construct matrices A1, A2
  Matrix<T> A1(1, p_game->GetPlayer(1)->GetStrategies().size(), 1,
               p_game->GetPlayer(2)->GetStrategies().size());
  Matrix<T> A2(1, p_game->GetPlayer(2)->GetStrategies().size(), 1,
               p_game->GetPlayer(1)->GetStrategies().size());

  for (size_t i = 1; i <= p_game->GetPlayer(1)->GetStrategies().size(); i++) {
    profile->SetStrategy(p_game->GetPlayer(1)->GetStrategy(i));
    for (size_t j = 1; j <= p_game->GetPlayer(2)->GetStrategies().size(); j++) {
      profile->SetStrategy(p_game->GetPlayer(2)->GetStrategy(j));
      A1(i, j) = fac * (profile->GetPayoff(p_game->GetPlayer(1)) - min);
      A2(j, i) = fac * (profile->GetPayoff(p_game->GetPlayer(2)) - min);
    }
  }

  // Construct vectors b1, b2
  Vector<T> b1(1, p_game->GetPlayer(1)->GetStrategies().size());
  Vector<T> b2(1, p_game->GetPlayer(2)->GetStrategies().size());
  b1 = (T)-1;
  b2 = (T)-1;

  // enumerate vertices of A1 x + b1 <= 0 and A2 x + b2 <= 0
  const VertexEnumerator<T> poly1(A1, b1);
  const VertexEnumerator<T> poly2(A2, b2);

  const auto &verts1(poly1.VertexList());
  const auto &verts2(poly2.VertexList());
  solution->m_v1 = verts1.size();
  solution->m_v2 = verts2.size();

  Array<int> vert1id(solution->m_v1);
  Array<int> vert2id(solution->m_v2);
  std::fill(vert1id.begin(), vert1id.end(), 0);
  std::fill(vert2id.begin(), vert2id.end(), 0);

  int id1 = 0, id2 = 0;

  for (int i2 = 2; i2 <= solution->m_v2; i2++) {
    const BFS<T> &bfs1 = verts2[i2];
    for (int i1 = 2; i1 <= solution->m_v1; i1++) {
      const BFS<T> &bfs2 = verts1[i1];

      // check if solution is nash
      // need only check complementarity, since it is feasible
      bool nash = true;
      for (size_t k = 1; nash && k <= p_game->GetPlayer(1)->GetStrategies().size(); k++) {
        if (bfs1.count(k) && bfs2.count(-k)) {
          nash = EqZero(bfs1[k] * bfs2[-k]);
        }
      }
      for (size_t k = 1; nash && k <= p_game->GetPlayer(2)->GetStrategies().size(); k++) {
        if (bfs2.count(k) && bfs1.count(-k)) {
          nash = EqZero(bfs2[k] * bfs1[-k]);
        }
      }

      if (nash) {
        MixedStrategyProfile<T> eqm(p_game->NewMixedStrategyProfile(static_cast<T>(0)));
        eqm = static_cast<T>(0);
        for (size_t k = 1; k <= p_game->GetPlayer(1)->GetStrategies().size(); k++) {
          if (bfs1.count(k)) {
            eqm[p_game->GetPlayer(1)->GetStrategy(k)] = -bfs1[k];
          }
        }
        for (size_t k = 1; k <= p_game->GetPlayer(2)->GetStrategies().size(); k++) {
          if (bfs2.count(k)) {
            eqm[p_game->GetPlayer(2)->GetStrategy(k)] = -bfs2[k];
          }
        }
        eqm = eqm.Normalize();
        solution->m_extremeEquilibria.push_back(eqm);
        p_onEquilibrium(eqm, "NE");

        // note: The keys give the mixed strategy associated with each node.
        //       The keys should also keep track of the basis
        //       As things stand now, two different bases could lead to
        //       the same key... BAD!
        if (vert1id[i1] == 0) {
          id1++;
          vert1id[i1] = id1;
          solution->m_key2.push_back(eqm.GetStrategy(p_game->GetPlayer(2)));
        }
        if (vert2id[i2] == 0) {
          id2++;
          vert2id[i2] = id2;
          solution->m_key1.push_back(eqm.GetStrategy(p_game->GetPlayer(1)));
        }
        solution->m_node1.push_back(vert2id[i2]);
        solution->m_node2.push_back(vert1id[i1]);
      }
    }
  }
  return solution;
}

template class EnumMixedStrategySolution<double>;
template class EnumMixedStrategySolution<Rational>;

template std::shared_ptr<EnumMixedStrategySolution<double>>
EnumMixedStrategySolveDetailed(const Game &p_game, StrategyCallbackType<double> p_onEquilibrium);
template std::shared_ptr<EnumMixedStrategySolution<Rational>>
EnumMixedStrategySolveDetailed(const Game &p_game, StrategyCallbackType<Rational> p_onEquilibrium);

} // end namespace Gambit::Nash
