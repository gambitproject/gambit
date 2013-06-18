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

#include <vector>
#include "cmatrix.h"
#include "nfgame.h"

nfgame::nfgame(int numPlayers, int *actions, const cvector &payoffs) : gnmgame(numPlayers, actions), payoffs(payoffs) {
  blockSize = new int[numPlayers + 1];
  blockSize[0] = 1;
  for(int i = 1; i <= numPlayers; i++) {
    blockSize[i] = blockSize[i-1]*actions[i-1];
  }
}

nfgame::~nfgame() {
  delete[] blockSize;
}

int nfgame::findIndex(int player, int *s) {
  int i, retIndex=player * blockSize[numPlayers];
  for(i = 0; i < numPlayers; i++)
    retIndex += blockSize[i] * s[i];
  return retIndex;
}

double nfgame::getMixedPayoff(int player, cvector &s) {
  double *m = new double[blockSize[numPlayers]];
  try {
    memcpy(m, payoffs.values() + player * blockSize[numPlayers], blockSize[numPlayers]*sizeof(double));
    double p = localPayoff(s, m, numPlayers-1);
    delete [] m;
    return p;
  }
  catch (...) {
    delete [] m;
    throw;
  }
}

void nfgame::getPayoffVector(cvector &dest, int player, const cvector &s){
  vector<double> t(payoffs.values()+player*blockSize[numPlayers], payoffs.values()+(player+1)*blockSize[numPlayers]);

  localPayoffVector(dest.values(), player,const_cast<cvector&>(s),&(t[0]),numPlayers-1);
}

void nfgame::payoffMatrix(cmatrix &dest, cvector &s, double fuzz) {
  int rown, coln, rowi, coli;
  double fuzzcount;
  double *m = new double[blockSize[numPlayers]];
  double *local = new double[maxActions*maxActions];
  try {
    for(rown = 0; rown < numPlayers; rown++) {
      for(coln = 0; coln < numPlayers; coln++) {
        if(rown == coln) {
	  fuzzcount = fuzz;
	  for(rowi=firstAction(rown); rowi < lastAction(rown); rowi++) {
	    for(coli=firstAction(coln); coli < lastAction(coln); coli++) {
	      dest[rowi][coli]=fuzzcount;
	      fuzzcount += fuzz;
	    }
	  }
        } else {
	  // set m to be the payoffs for player rown
	  memcpy(m, payoffs.values() + rown * blockSize[numPlayers], blockSize[numPlayers] * sizeof(double));
	  localPayoffMatrix(local, rown, coln, s, m, numPlayers-1);
	  for(rowi = firstAction(rown); rowi < lastAction(rown); rowi++) {
	    for(coli = firstAction(coln); coli < lastAction(coln); coli++) {
	      if(rown > coln) {
	        dest[rowi][coli] = *(local + (rowi - firstAction(rown))*actions[coln] + (coli - firstAction(coln)));
	      } else {
	        dest[rowi][coli] = *(local + (coli - firstAction(coln))*actions[rown] + (rowi - firstAction(rown)));
	      }
	    }
	  }
        }
      }
    }
    delete [] local;
    delete [] m;
  }
  catch (...) {
    delete [] local;
    delete [] m;
    throw;
  }
}


//assumes m = memcpy(m, payoffs + blockSize[numPlayers] * player1, blockSize[numPlayers]*sizeof(double)), player1 != player2
//i.e. m points to payoff cmatrix for the desired player

void nfgame::localPayoffMatrix(double *dest, int player1, int player2, cvector &s, double *m, int n) {
  int i;
  if(player1 == n) {
    for(i = 0; i < actions[player1]; i++) {
      localPayoffVector(dest+i*actions[player2], player2, s, m+i*blockSize[player1], n-1);
    }
  } else if(player2 == n) {
    for(i = 0; i < actions[player2]; i++) {
      localPayoffVector(dest+i*actions[player1], player1, s, m+i*blockSize[player2], n-1);
    }
  } else {
    m = scaleMatrix(s, m, n);
    localPayoffMatrix(dest, player1, player2, s, m, n-1);
  }
}

double *nfgame::scaleMatrix(cvector &s, double *m, int n) {
  int i,j, curbase, newbase = -1;
  double scale;
  for(i = 0; i < actions[n]; i++) {
    if(s[i+firstAction(n)] > 0.0) {
      scale = s[i+firstAction(n)];
      curbase = i*blockSize[n];
      if(newbase < 0) {
	newbase = curbase;
	for(j = curbase; j < curbase+blockSize[n]; j++) {
	  m[j] *= scale;
	}
      } else {
	for(j = 0; j < blockSize[n]; j++) {
	  m[newbase + j] += scale * m[curbase+j];
	}
      }
    }
  }
  return m+newbase;
}

void nfgame::localPayoffVector(double *dest, int player, cvector &s, double *m, int n) {
  if(player == n) {
    for(int i = 0; i < actions[player]; i++) {
      dest[i] = localPayoff(s, m+i*blockSize[player], n-1);
    }
  } else {
    m = scaleMatrix(s, m, n);
    localPayoffVector(dest, player, s, m, n-1);
  }
}

double nfgame::localPayoff(cvector &s, double *m, int n) {
  if(n < 0)
    return *m;
  else {
    m = scaleMatrix(s, m, n);
    return localPayoff(s, m, n-1);
  }
}
