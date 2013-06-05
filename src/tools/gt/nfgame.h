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

#ifndef __NFGAME_H
#define __NFGAME_H

#include "gnmgame.h"
#include "cmatrix.h"

class nfgame : public gnmgame {
 public:
  friend ostream& operator<< (ostream& s, nfgame& g);
  nfgame(int numPlayers, int *actions, const cvector &payoffs);
  ~nfgame();

  // Input: s[i] has integer index of player i's pure strategy
  // s is of length numPlayers
  inline double getPurePayoff(int player, int *s) {
    return payoffs[findIndex(player, s)];
  }

  inline void setPurePayoff(int player, int *s, double value) {
    payoffs[findIndex(player, s)]= value;
  }

  double getMixedPayoff(int player, cvector &s);
  void payoffMatrix(cmatrix &dest, cvector &s, double fuzz);

  void getPayoffVector(cvector &dest, int player, const cvector &s);
  

 private:
  int findIndex(int player, int *s);
  void localPayoffMatrix(double *dest, int player1, int player2, cvector &s, double *m, int n);
  void localPayoffVector(double *dest, int player, cvector &s, double *m, int n);
  double localPayoff(cvector &s, double *m, int n);
  double *scaleMatrix(cvector &s, double *m, int n);
  cvector payoffs;
  int *blockSize;
};
inline ostream& operator<< (ostream& s, nfgame& g){

  s<<g.numPlayers<<endl;
  for (int i=0;i<g.numPlayers; i++){
    if (i>0) s<< " ";
    s<< g.actions[i];
  }
  s<<endl;
  s << g.payoffs;
  s<<endl;
  return s;
}


#endif
