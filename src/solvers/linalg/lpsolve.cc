//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/linalg/lpsolve.cc
// Instantiation of common LP solvers
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

#include <optional>

#include "lpsolve.h"
#include "core/rational.h"

namespace Gambit::linalg {

namespace {

template <class T> struct LPSolveState {
  bool wellFormed{true};
  bool feasible{true};
  bool bounded{true};
  int flag{0};
  int nvars{0}, neqns{0}, nequals{0};
  T totalCost, eps1, eps2, eps3, tmin;
  BFS<T> optBfs, dualBfs;
  LPTableau<T> tab;
  Array<bool> UB, LB;
  Array<T> ub, lb;
  Vector<T> xx, cost;
  Vector<T> y, x, d;
  CancelToken cancel;
};

template <class T> T ComputeEpsilon(int i);
template <> double ComputeEpsilon<double>(int i)
{
  return std::pow(10.0, -static_cast<double>(i));
}
template <> Rational ComputeEpsilon<Rational>(int /*i*/) { return Rational(0); }

template <class T> Array<int> MakeArtificials(const Vector<T> &b)
{
  Array<int> ret;
  for (int i = b.front_index(); i <= b.back_index(); i++) {
    if (b[i] < T{0}) {
      ret.push_back(i);
    }
  }
  return ret;
}

template <class T> int Enter(LPSolveState<T> &state)
{
  int in = 0;
  T test{0};
  for (int i = 1; i <= state.nvars + state.neqns; i++) {
    int lab = i;
    if (i > state.nvars) {
      lab = state.nvars - i;
    }
    if (!state.tab.IsMember(lab)) {
      T rc = state.tab.ComputeRelativeCost(lab);
      if (rc > test + state.eps1) {
        if (!state.UB[i] || (state.UB[i] && state.xx[i] - state.ub[i] < -state.eps1)) {
          test = rc;
          in = lab;
          state.flag = -1;
        }
      }
      if (-rc > test + state.eps1) {
        if (!state.LB[i] || (state.LB[i] && state.xx[i] - state.lb[i] > state.eps1)) {
          test = -rc;
          in = lab;
          state.flag = 1;
        }
      }
    }
  }
  return in;
}

template <class T> int Exit(LPSolveState<T> &state, int in)
{
  int out = 0;
  // nullopt means no row has yet imposed a binding constraint; avoids
  // assuming a magnitude no real ratio can reach.
  std::optional<T> best;

  for (int j = 1; j <= state.neqns; j++) {
    const int lab = state.tab.GetLabel(j);
    int col = lab;
    if (lab < 0) {
      col = state.nvars - lab;
    }
    std::optional<T> t;
    if (state.flag == -1) {
      if (state.d[j] > state.eps2 && state.LB[col]) {
        t = (state.xx[col] - state.lb[col]) / state.d[j];
      }
      if (state.d[j] < -state.eps2 && state.UB[col]) {
        t = (state.xx[col] - state.ub[col]) / state.d[j];
      }
    }
    if (state.flag == 1) {
      if (state.d[j] > state.eps2 && state.UB[col]) {
        t = (state.ub[col] - state.xx[col]) / state.d[j];
      }
      if (state.d[j] < -state.eps2 && state.LB[col]) {
        t = (state.lb[col] - state.xx[col]) / state.d[j];
      }
    }
    if (t.has_value() && *t >= -state.eps2 && (!best.has_value() || *t < *best - state.eps2)) {
      best = t;
      out = j;
    }
  }

  int col = in;
  if (in < 0) {
    col = state.nvars - in;
  }
  std::optional<T> t;
  if (state.flag == -1 && state.UB[col]) {
    t = state.ub[col] - state.xx[col];
  }
  if (state.flag == 1 && state.LB[col]) {
    t = state.xx[col] - state.lb[col];
  }
  if (t.has_value() && *t > state.eps2 && (!best.has_value() || *t < *best - state.eps2)) {
    best = t;
    out = -1;
  }

  state.tmin = best.value_or(state.tmin);
  return out;
}

template <class T> void Solve(LPSolveState<T> &state, int phase)
{
  int in, xlab;
  int outlab = 0;
  int out = 0;

  do {
    state.cancel.Check();
    // step 1: Solve y B = c_B
    do {
      in = Enter(state); // step 2: Choose entering variable
      if (in) {
        state.tab.SolveColumn(in, state.d); // step 3: Solve B d = a, where a col #in of A
        out = Exit(state, in);              // step 4: Choose leaving variable
        if (out == 0) {
          state.bounded = false;
          return;
        }
        else if (out < 0) {
          outlab = in;
        }
        else {
          outlab = state.tab.GetLabel(out);
        }
        // update xx
        for (size_t i = 1; i <= state.x.size(); i++) {
          xlab = state.tab.GetLabel(i);
          if (xlab < 0) {
            xlab = state.nvars - xlab;
          }
          state.xx[xlab] = state.xx[xlab] + static_cast<T>(state.flag) * state.tmin * state.d[i];
        }
        if (in > 0) {
          state.xx[in] -= static_cast<T>(state.flag) * state.tmin;
        }
        if (in < 0) {
          state.xx[state.nvars - in] -= static_cast<T>(state.flag) * state.tmin;
        }
      }
    } while (outlab == in && outlab != 0);
    if (in) {
      state.tab.Pivot(out, in);
      state.tab.GetBasisVector(state.x);
      if (phase == 1 && state.tab.ComputeTotalCost() >= -state.eps1) {
        return;
      }
    }
  } while (in);
}

} // end anonymous namespace

template <class T>
LPSolveResult<T> SolveLP(const Matrix<T> &A, const Vector<T> &b, const Vector<T> &c, int nequals,
                         const CancelToken &p_cancel)
{
  const int nvars0 = static_cast<int>(c.size());
  const int neqns = static_cast<int>(b.size());
  const Array<int> artificials = MakeArtificials(b);
  const int nvars = nvars0 + static_cast<int>(artificials.size());
  const int n = nvars + neqns;

  LPSolveState<T> state{
      .nvars = nvars,
      .neqns = neqns,
      .nequals = nequals,
      .totalCost = T{0},
      .eps1 = ComputeEpsilon<T>(5),
      .eps2 = ComputeEpsilon<T>(8),
      .eps3 = ComputeEpsilon<T>(6),
      .tmin = T{0},
      .tab = LPTableau<T>(A, artificials, b),
      .UB = Array<bool>(n),
      .LB = Array<bool>(n),
      .ub = Array<T>(n),
      .lb = Array<T>(n),
      .xx = Vector<T>(n),
      .cost = Vector<T>(n),
      .y = Vector<T>(b.size()),
      .x = Vector<T>(b.size()),
      .d = Vector<T>(b.size()),
      .cancel = p_cancel,
  };

  // Check dimensions
  if (A.NumRows() != b.size() || A.NumColumns() != c.size()) {
    state.wellFormed = false;
    return {state.wellFormed,    state.feasible,          state.bounded,       state.totalCost,
            std::move(state.xx), std::move(state.optBfs), std::move(state.tab)};
  }

  // initialize data
  int xlab;
  const int num_inequals = A.NumRows() - nequals;

  std::fill(state.UB.begin(), state.UB.end(), false);
  std::fill(state.LB.begin(), state.LB.end(), false);
  std::fill(state.ub.begin(), state.ub.end(), T{0});
  std::fill(state.lb.begin(), state.lb.end(), T{0});

  // Define Phase I upper and lower bounds
  for (int i = 1; i <= state.nvars; i++) {
    state.LB[i] = true; // original and artificial variables have lower bounds of 0
  }
  // for slack variables
  for (int i = 1; i <= state.neqns; i++) {
    if (b[i] >= T{0}) {
      state.LB[state.nvars + i] = true;
    }
    else {
      state.UB[state.nvars + i] = true;
    }
  }
  // define Phase 1 unit cost vector
  state.cost = T{0};
  for (int i = 1; i <= state.neqns; i++) {
    state.cost[state.nvars + i] = T{0};
    if (state.UB[state.nvars + i]) {
      state.cost[state.nvars + i] = T{1};
    }
    else if (i > num_inequals) {
      state.cost[state.nvars + i] = T{-1};
    }
  }

  // Initialize the tableau
  state.tab.SetCost(state.cost);

  // set xx to be initial feasible solution to phase II
  for (size_t i = 1; i <= state.xx.size(); i++) {
    if (state.LB[i]) {
      state.xx[i] = state.lb[i];
    }
    else if (state.UB[i]) {
      state.xx[i] = state.ub[i];
    }
    else {
      state.xx[i] = T{0};
    }
  }
  state.tab.GetBasisVector(state.x);
  for (size_t i = 1; i <= state.x.size(); i++) {
    xlab = state.tab.GetLabel(i);
    if (xlab < 0) {
      xlab = state.nvars - xlab;
    }
    state.xx[xlab] = state.x[i];
  }

  Solve(state, 1);

  state.totalCost = state.tab.ComputeTotalCost();

  // which eps should be used here?
  if (state.totalCost < -state.eps1) {
    state.feasible = false;
    return {state.wellFormed,    state.feasible,          state.bounded,       state.totalCost,
            std::move(state.xx), std::move(state.optBfs), std::move(state.tab)};
  }

  // Define Phase II upper and lower bounds for slack variables

  for (int i = num_inequals + 1; i <= state.neqns; i++) {
    state.UB[state.nvars + i] = true;
  }
  for (int i = 1; i <= state.neqns; i++) {
    if (b[i] < T{0}) {
      state.LB[state.nvars + i] = true;
    }
  }

  // install Phase II unit cost vector
  state.cost = T{0};
  std::copy(c.begin(), c.end(), state.cost.begin());

  state.tab.SetCost(state.cost);
  Solve(state, 2);

  state.totalCost = state.tab.ComputeTotalCost();
  state.y = state.tab.GetDualVector();
  state.optBfs = state.tab.GetColumnBFS();
  state.dualBfs = state.tab.GetDualBFS();

  for (int i = 1; i <= state.neqns; i++) {
    if (state.dualBfs.count(-i)) {
      state.optBfs.insert(-i, state.dualBfs[-i]);
    }
  }

  return {state.wellFormed,    state.feasible,          state.bounded,       state.totalCost,
          std::move(state.xx), std::move(state.optBfs), std::move(state.tab)};
}

template struct LPSolveResult<double>;
template struct LPSolveResult<Rational>;

template LPSolveResult<double> SolveLP(const Matrix<double> &, const Vector<double> &,
                                       const Vector<double> &, int, const CancelToken &);
template LPSolveResult<Rational> SolveLP(const Matrix<Rational> &, const Vector<Rational> &,
                                         const Vector<Rational> &, int, const CancelToken &);

} // end namespace Gambit::linalg
