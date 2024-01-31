//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: library/src/gtracer/gnmgame.cc
// Implementation of basic game representation class in Gametracer
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

#include <cmath>
#include <algorithm>
#include <numeric>
#include "cmatrix.h"
#include "gnmgame.h"

namespace Gambit {
namespace gametracer {

gnmgame::gnmgame(const std::vector<int> &p_actions)
  : strategyOffset(p_actions.size() + 1), numPlayers(p_actions.size()),
    numStrategies(std::accumulate(begin(p_actions), end(p_actions), 1, std::multiplies<>())),
    numActions(std::accumulate(begin(p_actions), end(p_actions), 0)), actions(p_actions),
    maxActions(*std::max_element(begin(p_actions), end(p_actions)))
{
  strategyOffset[0] = 0;
  std::partial_sum(begin(p_actions), end(p_actions), std::next(begin(strategyOffset)));
}

void gnmgame::retractJac(cmatrix &dest, const std::vector<int> &support) const
{
  int n, i, j;
  double totalk;
  for (n = 0; n < numPlayers; n++) {
    totalk = 0.0;
    for (i = firstAction(n); i < lastAction(n); i++) {
      totalk += support[i];
    }
    for (i = 0; i < numActions; i++) {
      for (j = firstAction(n); j < lastAction(n); j++) {
        if (i >= firstAction(n) && i < lastAction(n) && support[i] && support[j]) {
          if (i == j) {
            dest(i, j) = 1.0 - 1.0 / totalk;
          }
          else {
            dest(i, j) = -1.0 / totalk;
          }
        }
        else {
          dest(i, j) = 0.0;
        }
      }
    }
  }
}

int compareDouble(const void *d1, const void *d2)
{
  if (*reinterpret_cast<const double *>(d1) > *reinterpret_cast<const double *>(d2)) {
    return -1;
  }
  else if (*reinterpret_cast<const double *>(d1) < *reinterpret_cast<const double *>(d2)) {
    return 1;
  }
  else {
    return 0;
  }
}

void gnmgame::retract(cvector &dest, const cvector &z) const
{
  int n, i;
  double v, sumz;
  std::vector<double> y(numActions);
  memcpy(y.data(), z.values(), numActions * sizeof(double));
  for (n = 0; n < numPlayers; n++) {
    qsort(y.data() + firstAction(n), actions[n], sizeof(double), compareDouble);
    sumz = y[firstAction(n)];
    for (i = firstAction(n) + 1; i < lastAction(n); i++) {
      if (sumz - (i - firstAction(n)) * y[i] > 1) {
        break;
      }
      sumz += y[i];
    }
    v = (sumz - 1) / (double)(i - firstAction(n));
    for (i = firstAction(n); i < lastAction(n); i++) {
      dest[i] = z[i] - v;
      if (dest[i] < 0.0) {
        dest[i] = 0.0;
      }
    }
  }
}

void gnmgame::retract(cvector &dest, const cvector &z, bool ksym) const
{
  if (!ksym) {
    retract(dest, z);
    return;
  }
  int n, i;
  double v, sumz;
  auto *y = new double[getNumKSymActions()];
  int offs = 0;
  try {
    memcpy(y, z.values(), getNumKSymActions() * sizeof(double));
    for (n = 0; n < getNumPlayerClasses(); n++) {
      qsort(y + offs, getNumKSymActions(n), sizeof(double), compareDouble);
      sumz = y[offs];
      for (i = 1; i < getNumKSymActions(n); i++) {
        if (sumz - (i)*y[i + offs] > 1) {
          break;
        }
        sumz += y[i + offs];
      }
      v = (sumz - 1.0) / i;
      for (i = offs; i < offs + getNumKSymActions(n); i++) {
        dest[i] = z[i] - v;
        if (dest[i] < 0.0) {
          dest[i] = 0.0;
        }
      }
      offs += getNumKSymActions(n);
    }
    delete[] y;
  }
  catch (...) {
    delete[] y;
    throw;
  }
}

void gnmgame::normalizeStrategy(cvector &s) const
{
  for (int n = 0; n < numPlayers; n++) {
    double sum = 0.0;
    for (int i = firstAction(n); i < lastAction(n); i++) {
      sum += s[i];
    }
    for (int i = firstAction(n); i < lastAction(n); i++) {
      s[i] /= sum;
    }
  }
}

} // namespace gametracer
} // end namespace Gambit
