//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: library/include/gtracer/gnmgame.h
// Definition of basic game representation class in Gametracer
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

#ifndef GAMBIT_GTRACER_GNMGAME_H
#define GAMBIT_GTRACER_GNMGAME_H

#include "cmatrix.h"

namespace Gambit {
namespace gametracer {

class gnmgame {
public:
  //actions[i] = number of actions player i has
  gnmgame(const std::vector<int> &actions);

  virtual ~gnmgame() = default;


  // Input: s[i] has integer index of player i's pure strategy
  // s is of length numPlayers
  virtual double getPurePayoff(int player, std::vector<int> &s) = 0;

  virtual void setPurePayoff(int player, std::vector<int> &, double value) = 0;

  // The actions of all players are combined in one linear array of length
  // numActions; this gives the index of a player's first action in the array.
  int firstAction(int player) const
  {
    return strategyOffset[player];
  }

  int lastAction(int player) const
  {
    return strategyOffset[player + 1];
  }

  // s is the mixed strategy profile.  It is of length numActions, and
  // s[i] is the probability that the appropriate player takes action i.
  virtual double getMixedPayoff(int player, cvector &s) const = 0;

  virtual double getSymMixedPayoff(cvector &s)
  {
    cvector fulls(getNumActions());
    int nact = getNumActions(0);
    for (int i = 0; i < getNumPlayers(); ++i) {
      for (int j = 0; j < nact; ++j) {
        fulls[j + firstAction(i)] = s[j];
      }
    }
    return getMixedPayoff(0, fulls);
  }

  // s is the mixed strategy profile, as above.  This function stores
  // the Jacobian of the payoff function G, where G(i) is the payoff to
  // the owner of action i if he deviates from s by choosing i instead.
  virtual void payoffMatrix(cmatrix &dest, cvector &s, double fuzz) const = 0;

  virtual void payoffMatrix(cmatrix &dest, cvector &s, double fuzz, bool ksym) const
  {
    if (ksym && s.getm() != getNumKSymActions()) {
      throw std::runtime_error(
        "payoffMatrix(): k-symmetric version of Jacobian not implemented for this class"
      );
    }
    payoffMatrix(dest, s, fuzz);
  }

  // store in dest the payoff function G, where G(i) is the payoff to
  // the owner of action i if he deviates from s by choosing i instead.
  virtual void getPayoffVector(cvector &dest, int player, const cvector &s) const = 0;

  // this stores the Jacobian of the retraction function in dest.
  void retractJac(cmatrix &dest, std::vector<int> &support) const;

  // This retracts z onto the nearest normalized strategy profile, according
  // to the Euclidean metric
  void retract(cvector &dest, cvector &z) const;

  void retract(cvector &dest, cvector &z, bool ksym) const;

  // This normalizes a strategy profile by scaling appropriately.
  void normalizeStrategy(cvector &s);

  int getNumPlayers() const
  { return numPlayers; }

  int getNumActions() const
  { return numActions; }

  int getNumActions(int p) const
  { return actions[p]; }

  virtual int getNumPlayerClasses() const
  { return numPlayers; }

  int getNumKSymActions() const
  { return numActions; }

  virtual int getNumKSymActions(int p) const
  { return actions[p]; }

  virtual int firstKSymAction(int p) const
  { return strategyOffset[p]; }

  virtual int lastKSymAction(int p) const
  { return strategyOffset[p + 1]; }

protected:
  std::vector<int> strategyOffset;
  int numPlayers, numStrategies, numActions;
  std::vector<int> actions;
  int maxActions;
};

}  // end namespace Gambit::gametracer
}  // end namespace Gambit
 
#endif  // GAMBIT_GTRACER_GNMGAME_H

