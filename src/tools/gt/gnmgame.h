/* Copyright 2002 Ben Blum, Christian Shelton
 *
 * This file is part of GameTracer.
 *
 * GameTracer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GameTracer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GameTracer; if not, write to the Free Software Foundation, 
 * Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __GNMGAME_H
#define __GNMGAME_H

#include "cmatrix.h"
#define BIGFLOAT 3.0e+28F

class gnmgame {
 public:
  
  //actions[i] = number of actions player i has
  gnmgame(int numplayers, int *actions);
  virtual ~gnmgame();
  
  
  
  // Input: s[i] has integer index of player i's pure strategy
  // s is of length numPlayers
  virtual double getPurePayoff(int player, int *s) = 0; 

  virtual void setPurePayoff(int player, int *s, double value) = 0;

  // The actions of all players are combined in one linear array of length
  // numActions; this gives the index of a player's first action in the array.
  inline int firstAction(int player) {
    return strategyOffset[player];
  }

  inline int lastAction(int player) {
    return strategyOffset[player+1];
  }

  // s is the mixed strategy profile.  It is of length numActions, and
  // s[i] is the probability that the appropriate player takes action i.
  virtual double getMixedPayoff(int player, cvector &s) = 0;

  // s is the mixed strategy profile, as above.  This function stores
  // the Jacobian of the payoff function G, where G(i) is the payoff to
  // the owner of action i if he deviates from s by choosing i instead.
  virtual void payoffMatrix(cmatrix &dest, cvector &s, double fuzz) = 0;

  // this stores the Jacobian of the retraction function in dest.  
  void retractJac(cmatrix &dest, std::vector<int> &support);

  // This retracts z onto the nearest normalized strategy profile, according
  // to the Euclidean metric
  void retract(cvector &dest, cvector &z);

  // LNM runs the local Newton method on z to attempt to bring it closer to
  // the image of the graph of the equilibrium correspondence above the ray,
  // under the homeomorphism.  In order to prevent costly memory allocation,
  // a number of scratch vectors are passed in.

  double LNM(cvector &z, const cvector &g, double det, cmatrix &J, cmatrix &DG,  cvector &s, int MaxLNM, double fuzz, cvector &del, cvector &scratch, cvector &backup);

  // This normalizes a strategy profile by scaling appropriately.
  void normalizeStrategy(cvector &s);

  void LemkeHowson(cvector &dest, cmatrix &T, std::vector<int> &Im);


  inline int getNumPlayers() { return numPlayers; }
  inline int getNumActions() { return numActions; }
  inline int getNumActions(int p) { return actions[p]; }
  inline int getMaxActions() { return maxActions; }

 protected:
  
  int Pivot(cmatrix &T, int pr, int pc, std::vector<int> &row, std::vector<int> &col, 
	    double &D);

  int *strategyOffset;
  int numPlayers, numStrategies, numActions;
  int *actions;
  int maxActions;
};

#endif
