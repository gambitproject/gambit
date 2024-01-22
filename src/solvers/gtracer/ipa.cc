//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: library/src/gtracer/gnm.cc
// Implementation of Global Newton Method from Gametracer
// This file is based on GameTracer v0.2, which is
// Copyright (c) 2002, Ben Blum and Christian Shelton
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

#include "gtracer.h"

namespace Gambit {
namespace gametracer {

int indexOf(const std::vector<int> &list, int target)
{
  for (size_t i = 0; i < list.size(); i++) {
    if (list[i] == target) {
      return i;
    }
  }
  return -1;
}

int Pivot(cmatrix &T, int pr, int pc, std::vector<int> &row, std::vector<int> &col, double &D,
          int numActions, int numPlayers)
{
  double pivot = T(pr, pc);
  int i0, j0, p, sgn = pivot < 0 ? -1 : 1;

  for (i0 = 0; i0 < numActions + numPlayers; i0++) {
    if (i0 != pr) {
      for (j0 = 0; j0 < numActions + numPlayers + 2; j0++) {
        if (j0 != pc) {
          T(i0, j0) *= pivot;
          T(i0, j0) -= T(i0, pc) * T(pr, j0);
          T(i0, j0) /= D * sgn;
        }
      }
    }
  }
  if (sgn == 1) {
    for (i0 = 0; i0 < numActions + numPlayers; i0++) {
      T(i0, pc) = -T(i0, pc);
    }
  }
  else {
    for (j0 = 0; j0 < numActions + numPlayers + 2; j0++) {
      T(pr, j0) = -T(pr, j0);
    }
  }
  T(pr, pc) = sgn * D;
  D = pivot < 0 ? -pivot : pivot;
  p = row[pr];
  row[pr] = col[pc];
  col[pc] = p;
  if (D > 1e6) {
    T /= D;
    D = 1;
  }
  return p;
}

void LemkeHowson(const gnmgame &game, cvector &dest, cmatrix &T, std::vector<int> &Im)
{
  const double BIGFLOAT = 3.0e+28F;
  int numActions = game.getNumActions(), numPlayers = game.getNumPlayers();
  double D = 1;
  int cg = numActions + numPlayers;
  int K = cg + 1;
  int pc, pr, p;
  double m;
  std::vector<int> col(numActions + numPlayers + 2);
  std::vector<int> row(numActions + numPlayers);
  for (int n = 0; n < numActions + numPlayers + 2; n++) {
    col[n] = n + 1;
  }
  for (int n = 0; n < numActions + numPlayers; n++) {
    row[n] = -n - 1;
  }
  for (int n = 0; n < numPlayers; n++) {
    pc = indexOf(col, Im[n] + 1);
    pr = indexOf(row, -numActions - n - 1);
    Pivot(T, pr, pc, row, col, D, numActions, numPlayers);
    pc = indexOf(col, numActions + n + 1);
    pr = indexOf(row, -Im[n] - 1);
    Pivot(T, pr, pc, row, col, D, numActions, numPlayers);
  }
  pc = indexOf(col, cg + 1);
  m = -BIGFLOAT;
  pr = -1;
  for (int n = 0; n < numPlayers + numActions; n++) {
    if (T(n, pc) < 0) {
      if (T(n, K) / T(n, pc) > m) {
        m = T(n, K) / T(n, pc);
        pr = n;
      }
    }
  }

  if (m > 0) {
    p = Pivot(T, pr, pc, row, col, D, numActions, numPlayers);
    do {
      pc = indexOf(col, -p);
      m = BIGFLOAT;
      pr = -1;
      for (int n = 0; n < numPlayers + numActions; n++) {
        if (T(n, pc) > 0 && (row[n] <= numActions || row[n] > numActions + numPlayers)) {
          if (T(n, K) / T(n, pc) < m) {
            m = T(n, K) / T(n, pc);
            pr = n;
          }
        }
      }
      p = Pivot(T, pr, pc, row, col, D, numActions, numPlayers);
    } while (p != cg + 1);
  }
  for (int n = 0; n < numActions; n++) {
    pr = indexOf(row, n + 1);
    if (pr == -1) {
      dest[n] = 0.0;
    }
    else {
      dest[n] = T(pr, K) / D;
    }
  }
}

// Initialize the Lemke-Howson tableau
void InitialiseLHTableau(cmatrix &T, const gnmgame &game, const cmatrix &DG, const cvector &g)
{
  int N = game.getNumPlayers(), M = game.getNumActions();

  for (int n = 0; n < N; n++) {
    for (int i = 0; i < M + N; i++) {
      if (i >= game.firstAction(n) && i < game.lastAction(n)) {
        T(M + n, i) = 1;
        T(i, M + n) = -1;
      }
      else {
        T(M + n, i) = T(i, M + n) = 0;
      }
    }
  }
  for (int n = 0; n < M; n++) {
    T(n, M + N) = g[n];
    T(n, M + N + 1) = 0.0;
  }
  for (int n = 0; n < N; n++) {
    T(M + n, M + N + 1) = 1.0;
    T(M + n, M + N) = 0.0;
  }
  for (int i = 0; i < M; i++) {
    for (int j = 0; j < M; j++) {
      T(i, j) = DG(i, j);
    }
  }
}

/// @brief Find the best response strategies when the game is highly perturbed
/// @details Given a game A, a perturbed game is defined by taking a vector
///          g which maps the strategies of the game to real numbers,
///          and adding lambda * g[s] to each payoff associated with strategy s.
///          If lambda is large enough, then this ensures that the player's
///          strategy s with the largest magnitude in g will be the unique
///          best response.
/// @param A    The target game
/// @param g    The perturbation vector, with length equal to the number of
///             strategies in the game
/// @param Im   The vector of best response indices, one for each player
/// @exception std::domain_error  If the perturbation vector \p g is not generic,
///                               that is, if each player does not have a unique
///                               maximizing strategy
void FindPerturbedBR(const gnmgame &A, const cvector &g, std::vector<int> &Im)
{
  for (int n = 0; n < A.getNumPlayers(); n++) {
    double bestPayoff = g[A.firstAction(n)];
    bool isTie = false;
    Im[n] = A.firstAction(n);
    for (int j = A.firstAction(n) + 1; j < A.lastAction(n); j++) {
      if (g[j] > bestPayoff) {
        bestPayoff = g[j];
        Im[n] = j;
        isTie = false;
      }
      else if (g[j] == bestPayoff) {
        isTie = true;
      }
    }
    if (isTie) {
      throw std::domain_error("Perturbation vector does not have unique maximizer for player " +
                              std::to_string(n + 1));
    }
  }
}

int IPA(const gnmgame &A, const cvector &g, cvector &zh, double alpha, double fuzz, cvector &ans,
        unsigned int maxiter, bool p_verbose)
{
  int N = A.getNumPlayers(),
      M = A.getNumActions(); // For easy reference
  std::vector<int> Im(N);    // best actions in perturbed game

  cmatrix DG(M, M), O(N, N, 0), // matrix of zeroes
      S(N, M, 0),               //
      I(M + N, M + N, 1, true), // identity
      T(M + N, M + N + 2, 0),   // tableau for Lemke-Howson
      T2(M + N, M + N, 0);      // submatrix of tableau used if Lemke-Howson is unnecessary

  cvector d(M), // diff
      u(M),
      y(M),      // old z
      yh(M),     // old zh
      s(M),      // current strategy
      so(M),     // old strategy
      sh(M),     // approximate strategy
      sho(M, 0), // old approximate strategy
      z(M),      // current point in game-space
      zt(M),     // next approximating point
      ym1(M),    // utility vars
      ym2(M), ymn1(M + N), ymn2(M + N);

  FindPerturbedBR(A, g, Im);
  A.retract(sh, zh);
  so = sh;

  for (unsigned int iter = 1; iter <= maxiter; iter++) {
    A.payoffMatrix(DG, sh, 0.0);
    DG /= (double)(N - 1); // find the Jacobian of the approximating bimatrix game

    InitialiseLHTableau(T, A, DG, g);

    // copy the tableau to T2
    for (int i = 0; i < M + N; i++) {
      for (int j = 0; j < M + N; j++) {
        T2(i, j) = T(i, j);
      }
    }

    // zero out columns not in the support
    for (int i = 0; i < M; i++) {
      if (so[i] <= 0.0) {
        for (int j = 0; j < M + N; j++) {
          T2(j, i) = (i == j) ? 1.0 : 0.0;
        }
      }
    }

    for (int i = 0; i < M + N; i++) {
      ymn1[i] = (i < M) ? 0.0 : 1.0;
    }

    // find equilibrium assuming current support
    T2.solve(ymn1, ymn2);
    for (int i = 0; i < M; i++) {
      s[i] = ymn2[i];
    }

    if (s.min() < 0.0) {
      // update support and solve
      LemkeHowson(A, s, T, Im);
    }
    else {
      // limit to current support
      for (int i = 0; i < M; i++) {
        if (so[i] <= 0.0) {
          s[i] = 0.0;
        }
      }
    }
    DG.multiply(s, z);
    z += s;

    // see if the support has changed
    int B = 1;
    for (int i = 0; i < M; i++) {
      if ((s[i] > 0.0) != (so[i] > 0.0)) {
        B = 0;
        break;
      }
    }

    // see if angle between z-sh and zh-sh is acute; if so, scale zh.
    u = zh;
    u -= sh;

    ym1 = z;
    ym1 -= sh;
    double ell = (ym1 * u) / u.norm2(); // dot product
    if (ell <= 0.0 || B) {
      zh = u;
      zh *= ell;
      zh += sh;
      yh -= sho;
      yh *= ell;
      yh += sho;
    }

    ym1 = z;
    ym1 -= zh;
    ym2 = s;
    ym2 -= sh;
    if (p_verbose) {
      std::cerr << "iter " << iter << "\tz diff " << ym1.norm() << "\ts diff " << ym2.norm()
                << std::endl;
    }
    if (!std::isfinite(ym1.norm()) || !std::isfinite(ym2.norm())) {
      throw std::out_of_range("encountered infinite probabilities in computed strategy profile");
    }
    // if z and zh or s and sh are close enough,
    // we've got an approximate equilibrium, so we can quit
    if (N <= 2 || (ym1.norm() < fuzz || ym2.norm() < fuzz)) {
      ans = s;
      A.payoffMatrix(DG, s, 0.0);
      return 1;
    }
    ym1 = z;
    // do a first-order approximation on the first iteration,
    // and subsequently do a second-order approximation
    if (iter > 1) {
      d = z;
      d -= zh;
      d -= y;
      d += yh;
      // Rule of false position
      for (int i = 0; i < M; i++) {
        if (B && d[i] > fuzz) {
          ym1[i] = (yh[i] * z[i] - zh[i] * y[i]) / d[i];
        }
        else {
          // Only do a first-order approximation
          ym1[i] = z[i];
        }
      }
    }
    else {
      ym1 = z;
    }
    zt = ym1;
    zt *= alpha / (1 - alpha);
    zt += zh;
    zt *= (1 - alpha);

    // Update values
    so = s;
    y = z;
    sho = sh;
    yh = zh;
    zh = zt;
    A.retract(sh, zh);

    sh.unfuzz(fuzz);
    A.normalizeStrategy(sh);
  }
  // Max iterations reached
  return 0;
}

} // namespace gametracer
} // end namespace Gambit
