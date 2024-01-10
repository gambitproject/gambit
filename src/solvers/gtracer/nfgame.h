//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: library/include/gtracer/nfgame.h
// Definition of normal form game class for Gametracer
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

#ifndef GAMBIT_GTRACER_NFGAME_H
#define GAMBIT_GTRACER_NFGAME_H

#include "gnmgame.h"
#include "cmatrix.h"

namespace Gambit {
namespace gametracer {

class nfgame : public gnmgame {
public:
  friend std::ostream &operator<<(std::ostream &s, nfgame &g);

  nfgame(const std::vector<int> &actions, const cvector &payoffs);

  ~nfgame() override = default;

  // Input: s[i] has integer index of player i's pure strategy
  // s is of length numPlayers
  double getPurePayoff(int player, std::vector<int> &s) override
  {
    return payoffs[findIndex(player, s)];
  }

  void setPurePayoff(int player, std::vector<int> &s, double value) override
  {
    payoffs[findIndex(player, s)] = value;
  }

  double getMixedPayoff(int player, cvector &s) const override;

  void payoffMatrix(cmatrix &dest, cvector &s, double fuzz) const override;

  void getPayoffVector(cvector &dest, int player, const cvector &s) const override;


private:
  int findIndex(int player, std::vector<int> &s) const;

  void localPayoffMatrix(double *dest, int player1, int player2, cvector &s, double *m, int n) const;

  void localPayoffVector(double *dest, int player, cvector &s, double *m, int n) const;

  double localPayoff(cvector &s, double *m, int n) const;

  double *scaleMatrix(cvector &s, double *m, int n) const;

  cvector payoffs;
  std::vector<int> blockSize;
};

inline std::ostream &operator<<(std::ostream &s, nfgame &g)
{
  s << g.numPlayers << std::endl;
  for (int i = 0; i < g.numPlayers; i++) {
    if (i > 0) { s << " "; }
    s << g.actions[i];
  }
  s << std::endl;
  s << g.payoffs;
  s << std::endl;
  return s;
}

}  // end namespace Gambit::gametracer
}  // end namespace Gambit

#endif  // GAMBIT_GTRACER_NFGAME_H
