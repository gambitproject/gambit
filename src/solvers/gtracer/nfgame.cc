//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: library/src/gtracer/nfgame.cc
// Implementation of normal form game class for Gametracer
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

#include <vector>
#include "cmatrix.h"
#include "nfgame.h"

namespace Gambit {
namespace gametracer {

nfgame::nfgame(const std::vector<int> &actions, const cvector &payoffs)
  : gnmgame(actions), payoffs(payoffs),
    blockSize(actions.size() + 1)
{
  blockSize[0] = 1;
  for (int i = 1; i <= numPlayers; i++) {
    blockSize[i] = blockSize[i - 1] * actions[i - 1];
  }
}

int nfgame::findIndex(int player, std::vector<int> &s) const
{
  int i, retIndex = player * blockSize[numPlayers];
  for (i = 0; i < numPlayers; i++) {
    retIndex += blockSize[i] * s[i];
  }
  return retIndex;
}

double nfgame::getMixedPayoff(int player, cvector &s) const
{
  std::vector<double> m(blockSize[numPlayers]);
  memcpy(m.data(), payoffs.values() + player * blockSize[numPlayers],
         blockSize[numPlayers] * sizeof(double));
  return localPayoff(s, m.data(), numPlayers - 1);
}

void nfgame::getPayoffVector(cvector &dest, int player, const cvector &s) const
{
  std::vector<double> t(payoffs.values() + player * blockSize[numPlayers],
                        payoffs.values() + (player + 1) * blockSize[numPlayers]);

  localPayoffVector(dest.values(), player, const_cast<cvector &>(s), &(t[0]), numPlayers - 1);
}

void nfgame::payoffMatrix(cmatrix &dest, cvector &s, double fuzz) const
{
  int rown, coln, rowi, coli;
  double fuzzcount;
  std::vector<double> m(blockSize[numPlayers]);
  std::vector<double> local(maxActions * maxActions);
  for (rown = 0; rown < numPlayers; rown++) {
    for (coln = 0; coln < numPlayers; coln++) {
      if (rown == coln) {
        fuzzcount = fuzz;
        for (rowi = firstAction(rown); rowi < lastAction(rown); rowi++) {
          for (coli = firstAction(coln); coli < lastAction(coln); coli++) {
            dest(rowi, coli) = fuzzcount;
            fuzzcount += fuzz;
          }
        }
      }
      else {
        // set m to be the payoffs for player rown
        memcpy(m.data(), payoffs.values() + rown * blockSize[numPlayers],
               blockSize[numPlayers] * sizeof(double));
        localPayoffMatrix(local.data(), rown, coln, s, m.data(), numPlayers - 1);
        for (rowi = firstAction(rown); rowi < lastAction(rown); rowi++) {
          for (coli = firstAction(coln); coli < lastAction(coln); coli++) {
            if (rown > coln) {
              dest(rowi, coli) = *(local.data() + (rowi - firstAction(rown)) * actions[coln] +
                                   (coli - firstAction(coln)));
            }
            else {
              dest(rowi, coli) = *(local.data() + (coli - firstAction(coln)) * actions[rown] +
                                   (rowi - firstAction(rown)));
            }
          }
        }
      }
    }
  }
}


//assumes m = memcpy(m, payoffs + blockSize[numPlayers] * player1, blockSize[numPlayers]*sizeof(double)), player1 != player2
//i.e. m points to payoff cmatrix for the desired player

void nfgame::localPayoffMatrix(double *dest, int player1, int player2, cvector &s, double *m, int n) const
{
  int i;
  if (player1 == n) {
    for (i = 0; i < actions[player1]; i++) {
      localPayoffVector(dest + i * actions[player2], player2, s, m + i * blockSize[player1], n - 1);
    }
  }
  else if (player2 == n) {
    for (i = 0; i < actions[player2]; i++) {
      localPayoffVector(dest + i * actions[player1], player1, s, m + i * blockSize[player2], n - 1);
    }
  }
  else {
    m = scaleMatrix(s, m, n);
    localPayoffMatrix(dest, player1, player2, s, m, n - 1);
  }
}

double *nfgame::scaleMatrix(cvector &s, double *m, int n) const
{
  int i, j, curbase, newbase = -1;
  double scale;
  for (i = 0; i < actions[n]; i++) {
    if (s[i + firstAction(n)] > 0.0) {
      scale = s[i + firstAction(n)];
      curbase = i * blockSize[n];
      if (newbase < 0) {
        newbase = curbase;
        for (j = curbase; j < curbase + blockSize[n]; j++) {
          m[j] *= scale;
        }
      }
      else {
        for (j = 0; j < blockSize[n]; j++) {
          m[newbase + j] += scale * m[curbase + j];
        }
      }
    }
  }
  return m + newbase;
}

void nfgame::localPayoffVector(double *dest, int player, cvector &s, double *m, int n) const
{
  if (player == n) {
    for (int i = 0; i < actions[player]; i++) {
      dest[i] = localPayoff(s, m + i * blockSize[player], n - 1);
    }
  }
  else {
    m = scaleMatrix(s, m, n);
    localPayoffVector(dest, player, s, m, n - 1);
  }
}

double nfgame::localPayoff(cvector &s, double *m, int n) const
{
  if (n < 0) {
    return *m;
  }
  else {
    m = scaleMatrix(s, m, n);
    return localPayoff(s, m, n - 1);
  }
}

}  // end namespace Gambit::gametracer
}  // end namespace Gambit
