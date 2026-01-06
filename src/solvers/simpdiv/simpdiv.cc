//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/tools/simpdiv/simpdiv.cc
// Compute Nash equilibria via simplicial subdivision on the normal form
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

#include <numeric>
#include "gambit.h"
#include "solvers/simpdiv/simpdiv.h"

namespace Gambit::Nash {

template <class T> class PVector {
private:
  Vector<T> m_values;
  Array<size_t> m_offsets;
  Array<size_t> m_shape;

public:
  explicit PVector(const Array<size_t> &p_shape)
    : m_values(std::accumulate(p_shape.begin(), p_shape.end(), 0)), m_offsets(p_shape.size()),
      m_shape(p_shape)
  {
    for (size_t index = 0, i = 1; i <= m_shape.size(); i++) {
      m_offsets[i] = index;
      index += m_shape[i];
    }
  }
  PVector(const PVector<T> &v) = default;
  ~PVector() = default;

  size_t size() const { return m_values.size(); }
  T &operator[](const int a) { return m_values[a]; }
  T &operator()(const int a, const int b) { return m_values[m_offsets[a] + b]; }
  const T &operator()(const int a, const int b) const { return m_values[m_offsets[a] + b]; }

  PVector<T> &operator=(const PVector<T> &v) = default;
  PVector<T> &operator=(const Vector<T> &v)
  {
    m_values = v;
    return *this;
  }
  PVector<T> &operator=(const T &c)
  {
    m_values = c;
    return *this;
  }

  const Array<size_t> &GetShape() const { return m_shape; }
  explicit operator const Vector<T> &() const { return m_values; }
};

class NashSimpdivStrategySolver {
public:
  explicit NashSimpdivStrategySolver(
      int p_gridResize = 2, int p_leashLength = 0,
      const Rational &p_maxregret = Rational(1, 1000000),
      StrategyCallbackType<Rational> p_onEquilibrium = NullStrategyCallback<Rational>)
    : m_gridResize(p_gridResize), m_leashLength((p_leashLength > 0) ? p_leashLength : 32000),
      m_maxregret(p_maxregret), m_onEquilibrium(p_onEquilibrium)
  {
  }
  ~NashSimpdivStrategySolver() = default;

  std::list<MixedStrategyProfile<Rational>>
  Solve(const MixedStrategyProfile<Rational> &p_start) const;
  std::list<MixedStrategyProfile<Rational>> Solve(const Game &p_game) const;

private:
  int m_gridResize, m_leashLength;
  Rational m_maxregret;
  StrategyCallbackType<Rational> m_onEquilibrium;

  class State;

  Rational Simplex(MixedStrategyProfile<Rational> &, const Rational &d) const;
  static void update(State &, RectArray<int> &, RectArray<int> &, PVector<Rational> &,
                     const PVector<int> &, int j, int i);
  static void getY(const State &, MixedStrategyProfile<Rational> &x, PVector<Rational> &,
                   const PVector<int> &, const PVector<int> &, const PVector<Rational> &,
                   const RectArray<int> &, int k);
  static void getnexty(const State &, MixedStrategyProfile<Rational> &x, const RectArray<int> &,
                       const PVector<int> &, int i);
  static int get_c(int j, int h, int nstrats, const PVector<int> &);
  static int get_b(int j, int h, int nstrats, const PVector<int> &);
};

//-------------------------------------------------------------------------
//          NashSimpdivStrategySolver: Private member functions
//-------------------------------------------------------------------------

inline GameStrategy GetStrategy(const Game &game, int pl, int st)
{
  return game->GetPlayer(pl)->GetStrategy(st);
}

class NashSimpdivStrategySolver::State {
public:
  int m_leashLength;
  int t{0}, ibar{1};
  Rational d, pay, maxz, bestz;

  explicit State(int p_leashLength) : m_leashLength(p_leashLength), bestz(1.0e30) {}
  Rational getlabel(MixedStrategyProfile<Rational> &yy, Array<int> &, PVector<Rational> &);

  /* Check whether the distance p_dist is "too far" given the leash length, if set. */
  bool CheckLeashOK(const Rational &p_dist) const
  {
    if (m_leashLength == 0) {
      return true;
    }
    return p_dist < m_leashLength * d;
  }
};

/// @brief Implementation of the piecewise path-following algorithm
/// @returns The maximum regret of any player at the terminal profile
Rational NashSimpdivStrategySolver::Simplex(MixedStrategyProfile<Rational> &y,
                                            const Rational &d) const
{
  const Game game = y.GetGame();
  State state(m_leashLength);
  state.d = d;
  Array<size_t> nstrats(game->GetStrategies().shape_array());
  Array<int> ylabel(2);
  RectArray<int> labels(y.MixedProfileLength(), 2), pi(y.MixedProfileLength(), 2);
  PVector<int> U(nstrats), TT(nstrats);
  PVector<Rational> ab(nstrats), besty(nstrats), v(nstrats);
  for (size_t i = 1; i <= v.size(); i++) {
    v[i] = y[i];
  }
  besty = y.GetProbVector();
  int i = 0;
  int j, k, h, jj, hh, ii, kk, tot;
  Rational maxz;

  // Label step0 not currently used, hence commented
  // step0:
  TT = 0;
  U = 0;
  ab = Rational(0);
  for (j = 1; j <= static_cast<int>(game->NumPlayers()); j++) {
    const GamePlayer player = game->GetPlayer(j);
    for (h = 1; h <= nstrats[j]; h++) {
      if (v(j, h) == Rational(0)) {
        U(j, h) = 1;
      }
      y[player->GetStrategy(h)] = v(j, h);
    }
  }

step1:
  maxz = state.getlabel(y, ylabel, besty);
  j = ylabel[1];
  h = ylabel[2];
  labels(state.ibar, 1) = j;
  labels(state.ibar, 2) = h;

  // Label case1a not currently used, hence commented
  // case1a:
  if (TT(j, h) == 0 && U(j, h) == 0) {
    for (hh = 1, tot = 0; hh <= nstrats[j]; hh++) {
      if (TT(j, hh) == 1 || U(j, hh) == 1) {
        tot++;
      }
    }
    if (tot == nstrats[j] - 1) {
      goto end;
    }
    else {
      i = state.t + 1;
      goto step2;
    }
  }
  /* case1b */
  else if (TT(j, h)) {
    i = 1;
    while (labels(i, 1) != j || labels(i, 2) != h || i == state.ibar) {
      i++;
    }
    goto step3;
  }
  /* case1c */
  else if (U(j, h)) {
    k = h;
    while (U(j, k)) {
      k++;
      if (k > nstrats[j]) {
        k = 1;
      }
    }
    if (TT(j, k) == 0) {
      i = state.t + 1;
    }
    else {
      i = 1;
      while ((pi(i, 1) != j || pi(i, 2) != k) && i <= state.t) {
        i++;
      }
    }
    goto step2;
  }

step2:
  getY(state, y, v, U, TT, ab, pi, i);
  pi.RotateDown(i, state.t + 1);
  pi(i, 1) = j;
  pi(i, 2) = h;
  labels.RotateDown(i + 1, state.t + 2);
  state.ibar = i + 1;
  state.t++;
  getnexty(state, y, pi, U, i);
  TT(j, h) = 1;
  U(j, h) = 0;
  goto step1;

step3:
  ii = (i == state.t + 1) ? state.t : i;
  j = pi(ii, 1);
  h = pi(ii, 2);
  k = h;
  if (i < state.t + 1) {
    k = get_b(j, h, nstrats[j], U);
  }
  kk = get_c(j, h, nstrats[j], U);
  if (i == 1) {
    ii = state.t + 1;
  }
  else if (i == state.t + 1) {
    ii = 1;
  }
  else {
    ii = i - 1;
  }
  getY(state, y, v, U, TT, ab, pi, ii);

  /* case3a */
  if (i == 1 && (y[GetStrategy(game, j, k)] <= Rational(0) ||
                 !state.CheckLeashOK(v(j, k) - y[GetStrategy(game, j, k)]))) {
    for (hh = 1, tot = 0; hh <= nstrats[j]; hh++) {
      if (TT(j, hh) == 1 || U(j, hh) == 1) {
        tot++;
      }
    }
    if (tot == nstrats[j] - 1) {
      U(j, k) = 1;
      goto end;
    }
    else {
      update(state, pi, labels, ab, U, j, i);
      U(j, k) = 1;
      getnexty(state, y, pi, U, state.t);
      goto step1;
    }
  }
  /* case3b */
  else if (i >= 2 && i <= state.t &&
           (y[GetStrategy(game, j, k)] <= Rational(0) ||
            !state.CheckLeashOK(v(j, k) - y[GetStrategy(game, j, k)]))) {
    goto step4;
  }
  /* case3c */
  else if (i == state.t + 1 && ab(j, kk) == Rational(0)) {
    if (y[GetStrategy(game, j, h)] <= Rational(0) ||
        !state.CheckLeashOK(v(j, h) - y[GetStrategy(game, j, h)])) {
      goto step4;
    }
    else {
      k = 0;
      while (ab(j, kk) == Rational(0) && k == 0) {
        if (kk == h) {
          k = 1;
        }
        kk++;
        if (kk > nstrats[j]) {
          kk = 1;
        }
      }
      kk--;
      if (kk == 0) {
        kk = nstrats[j];
      }
      if (kk == h) {
        goto step4;
      }
      else {
        goto step5;
      }
    }
  }
  else {
    if (i == 1) {
      getnexty(state, y, pi, U, 1);
    }
    else if (i <= state.t) {
      getnexty(state, y, pi, U, i);
    }
    else if (i == state.t + 1) {
      j = pi(state.t, 1);
      h = pi(state.t, 2);
      hh = get_b(j, h, nstrats[j], U);
      y[GetStrategy(game, j, h)] -= state.d;
      y[GetStrategy(game, j, hh)] += state.d;
    }
    update(state, pi, labels, ab, U, j, i);
  }
  goto step1;

step4:
  getY(state, y, v, U, TT, ab, pi, 1);
  j = pi(i - 1, 1);
  h = pi(i - 1, 2);
  TT(j, h) = 0;
  if (y[GetStrategy(game, j, h)] <= Rational(0) ||
      !state.CheckLeashOK(v(j, h) - y[GetStrategy(game, j, h)])) {
    U(j, h) = 1;
  }
  labels.RotateUp(i, state.t + 1);
  pi.RotateUp(i - 1, state.t);
  state.t--;
  ii = 1;
  while (labels(ii, 1) != j || labels(ii, 2) != h) {
    ii++;
  }
  i = ii;
  goto step3;

step5:
  k = kk;
  labels.RotateDown(1, state.t + 1);
  state.ibar = 1;
  pi.RotateDown(1, state.t);
  U(j, k) = 0;
  jj = pi(1, 1);
  hh = pi(1, 2);
  kk = get_b(jj, hh, nstrats[jj], U);
  y[GetStrategy(game, jj, hh)] -= state.d;
  y[GetStrategy(game, jj, kk)] += state.d;

  k = get_c(j, h, nstrats[j], U);
  kk = 1;
  while (kk) {
    if (k == h) {
      kk = 0;
    }
    ab(j, k) -= Rational(1);
    k++;
    if (k > nstrats[j]) {
      k = 1;
    }
  }
  goto step1;

end:
  maxz = state.bestz;
  for (i = 1; i <= static_cast<int>(game->NumPlayers()); i++) {
    for (j = 1; j <= nstrats[i]; j++) {
      y[GetStrategy(game, i, j)] = besty(i, j);
    }
  }
  return maxz;
}

void NashSimpdivStrategySolver::update(State &state, RectArray<int> &pi, RectArray<int> &labels,
                                       PVector<Rational> &ab, const PVector<int> &U, int j, int i)
{
  int jj, hh, k, f = 1;

  if (i >= 2 && i <= state.t) {
    pi.SwitchRows(i, i - 1);
    state.ibar = i;
  }
  else if (i == 1) {
    labels.RotateUp(1, state.t + 1);
    state.ibar = state.t + 1;
    jj = pi(1, 1);
    hh = pi(1, 2);
    if (jj == j) {
      k = get_c(jj, hh, ab.GetShape()[jj], U);
      while (f) {
        if (k == hh) {
          f = 0;
        }
        ab(j, k) += Rational(1);
        k++;
        if (k > ab.GetShape()[jj]) {
          k = 1;
        }
      }
      pi.RotateUp(1, state.t);
    }
  }
  else if (i == state.t + 1) {
    labels.RotateDown(1, state.t + 1);
    state.ibar = 1;
    jj = pi(state.t, 1);
    hh = pi(state.t, 2);
    if (jj == j) {
      k = get_c(jj, hh, ab.GetShape()[jj], U);
      while (f) {
        if (k == hh) {
          f = 0;
        }
        ab(j, k) -= Rational(1);
        k++;
        if (k > ab.GetShape()[jj]) {
          k = 1;
        }
      }
      pi.RotateDown(1, state.t);
    }
  }
}

void NashSimpdivStrategySolver::getY(const State &state, MixedStrategyProfile<Rational> &x,
                                     PVector<Rational> &v, const PVector<int> &U,
                                     const PVector<int> &TT, const PVector<Rational> &ab,
                                     const RectArray<int> &pi, int k)
{
  x = static_cast<const Vector<Rational> &>(v);
  for (int j = 1; j <= static_cast<int>(x.GetGame()->NumPlayers()); j++) {
    const GamePlayer player = x.GetGame()->GetPlayer(j);
    for (size_t h = 1; h <= player->GetStrategies().size(); h++) {
      if (TT(j, h) == 1 || U(j, h) == 1) {
        x[player->GetStrategy(h)] += state.d * ab(j, h);
        const int hh = (h > 1) ? h - 1 : player->GetStrategies().size();
        x[player->GetStrategy(hh)] -= state.d * ab(j, h);
      }
    }
  }
  for (int i = 2; i <= k; i++) {
    getnexty(state, x, pi, U, i - 1);
  }
}

void NashSimpdivStrategySolver::getnexty(const State &state, MixedStrategyProfile<Rational> &x,
                                         const RectArray<int> &pi, const PVector<int> &U, int i)
{
  const int j = pi(i, 1);
  const GamePlayer player = x.GetGame()->GetPlayer(j);
  const int h = pi(i, 2);
  x[player->GetStrategy(h)] += state.d;
  const int hh = get_b(j, h, player->GetStrategies().size(), U);
  x[player->GetStrategy(hh)] -= state.d;
}

int NashSimpdivStrategySolver::get_b(int j, int h, int nstrats, const PVector<int> &U)
{
  int hh = (h > 1) ? h - 1 : nstrats;
  while (U(j, hh)) {
    hh--;
    if (hh == 0) {
      hh = nstrats;
    }
  }
  return hh;
}

int NashSimpdivStrategySolver::get_c(int j, int h, int nstrats, const PVector<int> &U)
{
  const int hh = get_b(j, h, nstrats, U) + 1;
  return (hh > nstrats) ? 1 : hh;
}

Rational NashSimpdivStrategySolver::State::getlabel(MixedStrategyProfile<Rational> &yy,
                                                    Array<int> &ylabel, PVector<Rational> &besty)
{
  Rational maxz(-1000000);
  ylabel[1] = 1;
  ylabel[2] = 1;

  for (int i = 1; i <= static_cast<int>(yy.GetGame()->NumPlayers()); i++) {
    const GamePlayer player = yy.GetGame()->GetPlayer(i);
    Rational payoff(0);
    Rational maxval(-1000000);
    int jj = 0;
    for (size_t j = 1; j <= player->GetStrategies().size(); j++) {
      pay = yy.GetPayoff(player->GetStrategy(j));
      payoff += yy[player->GetStrategy(j)] * pay;
      if (pay > maxval) {
        maxval = pay;
        jj = j;
      }
    }
    if (maxval - payoff > maxz) {
      maxz = maxval - payoff;
      ylabel[1] = i;
      ylabel[2] = jj;
    }
  }
  if (maxz < bestz) {
    bestz = maxz;
    for (int i = 1; i <= static_cast<int>(yy.GetGame()->NumPlayers()); i++) {
      const GamePlayer player = yy.GetGame()->GetPlayer(i);
      for (size_t j = 1; j <= player->GetStrategies().size(); j++) {
        besty(i, j) = yy[player->GetStrategy(j)];
      }
    }
  }
  return maxz;
}

//-------------------------------------------------------------------------
//           NashSimpdivStrategySolver: Main solution algorithm
//-------------------------------------------------------------------------

inline Integer find_lcd(const Vector<Rational> &vec)
{
  Integer lcd(1);
  for (int i = vec.front_index(); i <= vec.back_index(); i++) {
    lcd = lcm(vec[i].denominator(), lcd);
  }
  return lcd;
}

std::list<MixedStrategyProfile<Rational>>
NashSimpdivStrategySolver::Solve(const MixedStrategyProfile<Rational> &p_start) const
{
  if (!p_start.GetGame()->IsPerfectRecall()) {
    throw UndefinedException(
        "Computing equilibria of games with imperfect recall is not supported.");
  }
  Rational d(Integer(1), find_lcd(p_start.GetProbVector()));
  const Rational scale = p_start.GetGame()->GetMaxPayoff() - p_start.GetGame()->GetMinPayoff();

  MixedStrategyProfile<Rational> y(p_start);
  m_onEquilibrium(y, "start");

  while (true) {
    d /= Rational(m_gridResize);
    const Rational regret = Simplex(y, d);
    m_onEquilibrium(y, std::to_string(d));
    if (regret <= m_maxregret * scale) {
      break;
    }
  }

  m_onEquilibrium(y, "NE");
  std::list<MixedStrategyProfile<Rational>> sol;
  sol.push_back(y);
  return sol;
}

///
/// Compute an equilibrium using the default starting point.
///
/// This computes the equilibrium reached from the starting point profile
/// in which all players put probability one on their first strategy.
/// This is a not-unreasonable default in that it starts with a very
/// coarse grid and, if the game has an equilibrium in pure strategies,
/// or in mixed strategies with small denominators, it will find it quickly.
/// Starting with a strategy profile with a smaller denominator can lead
/// to a long initial search before reaching a candidate neighborhood
/// for an equilibrium.
///
std::list<MixedStrategyProfile<Rational>>
NashSimpdivStrategySolver::Solve(const Game &p_game) const
{
  MixedStrategyProfile<Rational> start = p_game->NewMixedStrategyProfile(Rational(0));
  start = Rational(0);
  for (const auto &player : p_game->GetPlayers()) {
    start[player->GetStrategies().front()] = Rational(1);
  }
  return Solve(start);
}

std::list<MixedStrategyProfile<Rational>>
SimpdivStrategySolve(const MixedStrategyProfile<Rational> &p_start, const Rational &p_maxregret,
                     int p_gridResize, int p_leashLength,
                     StrategyCallbackType<Rational> p_onEquilibrium)
{
  return NashSimpdivStrategySolver(p_gridResize, p_leashLength, p_maxregret, p_onEquilibrium)
      .Solve(p_start);
}

} // end namespace Gambit::Nash
