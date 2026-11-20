//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/enummixed/enummixed.cc
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

#include "games.h"
#include "solvers/bimatrix/bimatrix.h"
#include "solvers/linalg/vertenum.h"
#include "solvers/enummixed/enummixed.h"

#include <algorithm>
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
Array<MixedStrategyProfile<T>>
EnumMixedStrategySolution<T>::BuildCliqueProfiles(const Biclique &p_biclique) const
{
  const auto player1 = m_game->GetPlayer(1);
  const auto player2 = m_game->GetPlayer(2);
  Array<MixedStrategyProfile<T>> profiles;
  for (const int node1 : p_biclique.left) {
    for (const int node2 : p_biclique.right) {
      MixedStrategyProfile<T> profile(m_game->NewMixedStrategyProfile(T{0}));
      for (size_t k = 1; k <= m_key1[node1].size(); k++) {
        profile[player1->GetStrategy(k)] = m_key1[node1][k];
      }
      for (size_t k = 1; k <= m_key2[node2].size(); k++) {
        profile[player2->GetStrategy(k)] = m_key2[node2][k];
      }
      profiles.push_back(profile);
    }
  }
  return profiles;
}

template <class T>
Array<Array<Array<MixedStrategyProfile<T>>>> EnumMixedStrategySolution<T>::GetComponents() const
{
  Array<Array<Array<MixedStrategyProfile<T>>>> components;
  for (const auto &bicliques : EnumerateBicliqueComponents(m_edges)) {
    Array<Array<MixedStrategyProfile<T>>> polytopes;
    for (const auto &biclique : bicliques) {
      polytopes.push_back(BuildCliqueProfiles(biclique));
    }
    components.push_back(polytopes);
  }
  return components;
}

template <class T>
std::shared_ptr<EnumMixedStrategySolution<T>>
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
EnumMixedStrategySolveDetailed(const Game &p_game, const StrategyCallbackType<T> &p_onEquilibrium,
                               const CancelToken &p_cancel)
{
  const NormalisedBimatrix<T> bimatrix = BuildNormalisedBimatrix<T>(p_game);
  std::shared_ptr<EnumMixedStrategySolution<T>> solution(new EnumMixedStrategySolution<T>(p_game));

  const Matrix<T> &A1 = bimatrix.A1;
  const Matrix<T> &A2 = bimatrix.A2;

  // Construct vectors b1, b2
  Vector<T> b1(1, p_game->GetPlayer(1)->GetStrategies().size());
  Vector<T> b2(1, p_game->GetPlayer(2)->GetStrategies().size());
  b1 = static_cast<T>(-1);
  b2 = static_cast<T>(-1);

  // enumerate vertices of A1 x + b1 <= 0 and A2 x + b2 <= 0
  const auto poly1 = EnumerateVertices(A1, b1, p_cancel);
  const auto poly2 = EnumerateVertices(A2, b2, p_cancel);

  const auto &verts1(poly1.vertices);
  const auto &verts2(poly2.vertices);
  const int numVerts1 = verts1.size();
  const int numVerts2 = verts2.size();

  Array<int> vert1id(numVerts1);
  Array<int> vert2id(numVerts2);
  std::ranges::fill(vert1id, 0);
  std::ranges::fill(vert2id, 0);

  int id1 = 0, id2 = 0;

  for (int i2 = 2; i2 <= numVerts2; i2++) {
    const BFS<T> &bfs1 = verts2[i2];
    for (int i1 = 2; i1 <= numVerts1; i1++) {
      p_cancel.Check();
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
        p_onEquilibrium(eqm);

        // The keys give the mixed strategy associated with each node.
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
        solution->m_edges.emplace_back(vert2id[i2], vert1id[i1]);
      }
    }
  }
  return solution;
}

template class EnumMixedStrategySolution<double>;
template class EnumMixedStrategySolution<Rational>;

template std::shared_ptr<EnumMixedStrategySolution<double>>
EnumMixedStrategySolveDetailed(const Game &p_game,
                               const StrategyCallbackType<double> &p_onEquilibrium,
                               const CancelToken &p_cancel);
template std::shared_ptr<EnumMixedStrategySolution<Rational>>
EnumMixedStrategySolveDetailed(const Game &p_game,
                               const StrategyCallbackType<Rational> &p_onEquilibrium,
                               const CancelToken &p_cancel);

} // end namespace Gambit::Nash
