//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/linalg/vertenum.cc
// Implementation of vertex enumerator
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

#include "vertenum.h"

namespace Gambit::linalg {

namespace {

template <class T> struct VertexEnumerationState {
  const Matrix<T> &A;
  const Vector<T> &b;
  Vector<T> artificialConstraint;
  bool multipleOptima{false};
  size_t depth{0};
  Array<BFS<T>> vertexList, dualVertexList;
  Array<long> visits, branches;
  CancelToken cancel;
};

template <class T> void Deeper(VertexEnumerationState<T> &state)
{
  state.depth++;
  if (state.visits.size() < state.depth) {
    state.visits.push_back(0);
    state.branches.push_back(0);
  }
  state.visits[state.depth] += 1;
}

template <class T> void Search(VertexEnumerationState<T> &state, LPTableau<T> &tab)
{
  state.cancel.Check();
  Deeper(state);
  if (tab.IsLexMin()) {
    state.vertexList.push_back(tab.GetFullBFS());
    state.dualVertexList.push_back(tab.GetDualBFS());
  }
  auto pivot_list = tab.ComputeReversePivots();
  if (!pivot_list.empty()) {
    state.branches[state.depth] += pivot_list.size();
    LPTableau<T> tab2(tab);
    for (auto pivot : pivot_list) {
      tab2 = tab;
      tab2.Pivot(pivot[1], pivot[2]);
      Search(state, tab2);
    }
  }
  state.depth--;
}

template <class T> void DualSearch(VertexEnumerationState<T> &state, LPTableau<T> &tab)
{
  state.cancel.Check();
  Deeper(state);
  state.branches[state.depth] += 1;

  if (state.multipleOptima) {
    tab.SetConst(state.artificialConstraint); // install artificial constraint vector
    LPTableau<T> tab2(tab);
    for (int i = state.b.front_index(); i <= state.b.back_index(); i++) {
      if (state.b[i] == T{0}) {
        for (int j = -state.b.back_index(); j <= state.A.MaxCol(); j++) {
          if (j && !tab.IsMember(j)) {
            if (tab.IsDualReversePivot(i, j)) {
              state.branches[state.depth] += 1;
              tab2 = tab;
              tab2.Pivot(i, j);
              DualSearch(state, tab2);
            }
          }
        }
      }
    }
  }
  tab.SetConst(state.b); // install original constraint vector
  Search(state, tab);    // do primal search
  state.depth--;
}

} // end anonymous namespace

template <class T>
VertexEnumerationResult<T> EnumerateVertices(const Matrix<T> &A, const Vector<T> &b,
                                             const CancelToken &p_cancel)
{
  VertexEnumerationState<T> state{
      .A = A,
      .b = b,
      .artificialConstraint = b,
      .multipleOptima = std::any_of(b.begin(), b.end(), [](const T &v) { return v == T{0}; }),
      .cancel = p_cancel,
  };
  state.artificialConstraint = T{-1}; // NOLINT(cppcoreguidelines-prefer-member-initializer)

  LPTableau<T> tab(A, b);
  Vector<T> c(A.MinCol(), A.MaxCol());
  c = T{1};
  tab.SetCost(c);

  DualSearch(state, tab);

  return {state.vertexList, state.dualVertexList, static_cast<int>(A.NumColumns())};
}

template <class T> std::list<Vector<T>> VertexEnumerationResult<T>::GetVertices() const
{
  std::list<Vector<T>> verts;
  for (int i = 1; i <= vertices.size(); i++) {
    Vector<T> vert(numColumns);
    vert = T{0};
    for (int j = 1; j <= vert.size(); j++) {
      if (vertices[i].count(j)) {
        vert[j] = -vertices[i][j];
      }
    }
    verts.push_back(vert);
  }
  return verts;
}

template struct VertexEnumerationResult<double>;
template struct VertexEnumerationResult<Rational>;

template VertexEnumerationResult<double>
EnumerateVertices(const Matrix<double> &, const Vector<double> &, const CancelToken &);
template VertexEnumerationResult<Rational>
EnumerateVertices(const Matrix<Rational> &, const Vector<Rational> &, const CancelToken &);

} // end namespace Gambit::linalg
