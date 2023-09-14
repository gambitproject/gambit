//
// This file is part of Gambit
// Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
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
#include "cmatrix.h"
#include "gnmgame.h"

namespace Gambit {
namespace gametracer {

  gnmgame::gnmgame(int numPlayers, std::vector<int> &actions): numPlayers(numPlayers) {
  int i;
  numStrategies = 1;
  for(i = 0; i < numPlayers; i++) {
    numStrategies *= actions[i];
  }

  this->actions = new int[numPlayers];
  strategyOffset = new int[numPlayers+1];
  numActions = 0;
  maxActions = 0;
  for(i = 0; i < numPlayers; i++) {
    strategyOffset[i] = numActions;
    this->actions[i] = actions[i];
    numActions+=actions[i];
    if(actions[i] > maxActions)
      maxActions = actions[i];
  }
  strategyOffset[i] = numActions;
}
  
gnmgame::~gnmgame() {
  delete[] strategyOffset;
  delete[] actions;
}



void gnmgame::retractJac(cmatrix &dest, std::vector<int> &support) {
  int n, i, j;  
  double totalk;
  for(n = 0; n < numPlayers; n++) {
    totalk = 0.0;
    for(i = firstAction(n); i < lastAction(n); i++) {
      totalk += support[i];
    }
    for(i = 0; i < numActions; i++) {
      for(j = firstAction(n); j < lastAction(n); j++) {
	if(i >= firstAction(n) && i < lastAction(n) && support[i] && support[j]) {
	  if(i==j)
	    dest[i][j] = 1.0-1.0/totalk;
	  else
	    dest[i][j] = -1.0/totalk;
	} else
	  dest[i][j] = 0.0;
      }
    }
  }
}

int compareDouble(const void *d1, const void *d2) {
  if(*(double *)d1 > *(double *)d2)
    return -1;
  else if(*(double *)d1 < *(double *)d2)
    return 1;
  else
    return 0;
}

void gnmgame::retract(cvector &dest, cvector &z) {
  int n, i;
  double v, sumz;
  // FIXME: The original Gametracer code had a variable-sized array for 'y', which is
  // not permitted by ISO C++.  However, the implementation of this function is
  // not well-encapsulated, in particular, it uses pointer manipulation to segregate
  // out the probabilities for each player, and uses qsort() directly on those pointers.
  // So, it is not straightforward to translate this to use, e.g., std::vector.
  // 
  // The immediate solution is to allocate dynamically, and use try..catch to ensure
  // deallocation.  Encapsulation is of course a superior solution eventually.
  auto *y = new double[numActions];
  try {
    memcpy(y,z.values(),numActions*sizeof(double));
    for(n = 0; n < numPlayers; n++) {
      qsort(y+firstAction(n),actions[n],sizeof(double),compareDouble);
      sumz = y[firstAction(n)];
      for(i=firstAction(n)+1; i < lastAction(n); i++) {
        if(sumz - (i-firstAction(n)) * y[i] > 1)
	      break;
        sumz += y[i];
      }
      v = (sumz - 1) / (double)(i-firstAction(n));
      for(i = firstAction(n); i < lastAction(n); i++) {
        dest[i] = z[i] - v;
        if(dest[i] < 0.0)
          dest[i] = 0.0;
      }
    }
    delete [] y;
  }
  catch (...) {
    delete [] y;
    throw;
  }
}
void gnmgame::retract(cvector &dest, cvector &z, bool ksym){
  if(!ksym){
    retract(dest,z);
    return;
  }
  int n, i;
  double v, sumz;
  auto* y = new double[getNumKSymActions()];
  int offs=0;
  //CPY(y,z.values(),getNumKSymActions());
 try{
  memcpy(y,z.values(),getNumKSymActions()*sizeof(double));
  for(n = 0; n < getNumPlayerClasses(); n++) {
    qsort(y+offs,getNumKSymActions(n),sizeof(double),compareDouble);
    sumz = y[offs];
    for(i=1; i < getNumKSymActions(n); i++) {
      if(sumz - (i) * y[i+offs] > 1)
        break;
      sumz += y[i+offs];
    }
    v = (sumz - 1.0) / i;
    for(i = offs; i < offs+getNumKSymActions(n); i++) {
      dest[i] = z[i] - v;
      if(dest[i] < 0.0)
        dest[i] = 0.0;
    }
    offs+=getNumKSymActions(n);
  }
  delete [] y;
 }
 catch (...) {
     delete [] y;
     throw;
 }
}

void gnmgame::KSymRetractJac(cmatrix &dest, int *support){
  int n, i, j, offs;
  double totalk;
  for(n = 0, offs=0; n < getNumPlayerClasses(); n++) {
    totalk = 0.0;
    for(i = offs; i < offs+getNumKSymActions(n); i++) {
      totalk += support[i];
    }
    for(i = 0; i < getNumKSymActions(); i++) {
      for(j = offs; j < offs+getNumKSymActions(n); j++) {
        if(i >= offs && i < offs+getNumKSymActions(n) && support[i] && support[j]) {
          if(i==j)
            dest[i][j] = (1.0) - (1.0)/totalk;
          else
            dest[i][j] = (-1.0)/totalk;
        }
        else
          dest[i][j] = 0.0;
      }
    }
    offs+=getNumKSymActions(n);
  }
}

void gnmgame::KSymNormalizeStrategy(cvector& s){
  double sum;
  int offs=0;
  for(int n = 0; n < getNumPlayerClasses(); n++) {
    sum = 0.0;
    for(int i = 0; i < getNumKSymActions(n); i++) {
      sum += s[offs+i];
    }
    for(int i = 0; i < getNumKSymActions(n); i++) {
      s[offs+i] /= sum;
    }
    offs+=getNumKSymActions(n);
  }
}

double gnmgame::LNM(cvector &z, const cvector &g, double det, cmatrix &J, cmatrix &DG, cvector &s, int MaxLNM, double fuzz, cvector &del, cvector &scratch, cvector &backup, bool ksym) {
  double b, e = BIGFLOAT, ee;
  int k, faulted = 0;
  if(MaxLNM >= 1 && det != 0.0) {
    b = 1.0/det;
    for(k = 0; k < MaxLNM; k++) {
      //      del = z - s - DG*s / (double)(numPlayers - 1) - g; 
      DG.multiply(s,del);
      del /= (double)(numPlayers - 1);
      del += g;
      del += s;
      del -= z;
      del.negate();
      ee = max(del.max(),-del.min());

      if(ee < fuzz) {
	e = ee;
	break;
      } else if(e < ee) { // we got worse
	z = backup;
	retract(s, z, ksym);
	payoffMatrix(DG, s, fuzz, ksym);
      	if(faulted) // if we've already failed once, quit.
	  return e;
	b /= MaxLNM; // if the full LNM step fails to improve things,
	e = BIGFLOAT; // take smaller steps.
	faulted++;
	continue;
      }
      e = ee;
      J.multiply(del, scratch);
      scratch *= b;
      backup = z;
      z -= scratch;
      //      z = z - (J * del) * b;
      retract(s, z, ksym);
      payoffMatrix(DG, s, fuzz, ksym);
    }
    return ee;
  } else return fuzz;
}

void gnmgame::normalizeStrategy(cvector &s) {
  double sum;  
  for(int n = 0; n < numPlayers; n++) {
    sum = 0.0;
    for(int i = firstAction(n); i < lastAction(n); i++) {
      sum += s[i];
    }
    for(int i = firstAction(n); i < lastAction(n); i++) {
      s[i] /= sum;
    }
  }
}

int indexOf(const std::vector<int> &list, int target, int length) {
  for(int i = 0; i < length; i++) {
    if(list[i] == target)
      return i;
  }
  return -1;
}

void gnmgame::LemkeHowson(cvector &dest, cmatrix &T, std::vector<int> &Im) {
  double D = 1;
  int cg = numActions + numPlayers ;
  int K = cg+1;
  int n, pc, pr, p;
  double m;
  std::vector<int> col(numActions+numPlayers+2);
  std::vector<int> row(numActions+numPlayers);
  for(n = 0; n < numActions+numPlayers+2; n++)
    col[n] = n+1;
  for(n = 0; n < numActions+numPlayers; n++)
    row[n] = -n-1;
  for(n = 0; n < numPlayers; n++) {
    pc = indexOf(col, Im[n]+1, numActions+numPlayers+2);
    pr = indexOf(row,-numActions-n-1, numActions+numPlayers);
    p = Pivot(T, pr, pc, row, col, D);
    pc = indexOf(col, numActions+n+1, numActions+numPlayers+2);
    pr = indexOf(row, -Im[n]-1, numActions+numPlayers);
    p = Pivot(T, pr, pc, row, col, D);
  }
  pc = indexOf(col, cg+1, numActions+numPlayers+2);
  m = -BIGFLOAT;
  pr = -1;
  for(n = 0; n < numPlayers + numActions; n++) {
    if(T[n][pc] < 0) {
      if(T[n][K] / T[n][pc] > m) {
        m = T[n][K] / T[n][pc];
	pr = n;
      }
    }
  }

  if(m > 0) {
    p = Pivot(T, pr, pc, row, col, D);
    do {
      pc = indexOf(col, -p, numActions+numPlayers+2);
      m = BIGFLOAT;
      pr = -1;
      for(n = 0; n < numPlayers + numActions; n++) {
	if(T[n][pc] > 0 && (row[n] <= numActions || row[n] > numActions+numPlayers)) {
	  if(T[n][K] / T[n][pc] < m) {
	    m = T[n][K] / T[n][pc];
	    pr = n;
	  }
	}
      }
      p = Pivot(T, pr, pc, row, col, D);
    } while(p != cg+1);
  }
  for(n = 0; n < numActions; n++) {
    pr = indexOf(row,n+1,numActions+numPlayers);
    if(pr == -1)
      dest[n] = 0.0;
    else
      dest[n] = T[pr][K] / D;
  }
}

int gnmgame::Pivot(cmatrix &T, int pr, int pc, std::vector<int> &row, 
		   std::vector<int> &col, double &D) {
  double pivot = T[pr][pc];
  int i0,j0,p,sgn = pivot < 0 ? -1 : 1;
  
  for(i0 = 0; i0 < numActions+numPlayers; i0++) {
    if(i0 != pr) {
      for(j0 = 0; j0 < numActions+numPlayers+2; j0++) {
	if(j0 != pc) {
	  T[i0][j0] *= pivot;
	  T[i0][j0] -= T[i0][pc] * T[pr][j0];
	  T[i0][j0] /= D * sgn;
	}
      }
    }
  }
  if(sgn == 1) {
    for(i0 = 0; i0 < numActions+numPlayers; i0++) {
      T[i0][pc] = -T[i0][pc];
    }
  } else {
    for(j0 = 0; j0 < numActions+numPlayers+2; j0++) {
      T[pr][j0] = -T[pr][j0];
    }
  }
  T[pr][pc] = sgn * D;
  D = pivot < 0 ? -pivot : pivot;
  p = row[pr];
  row[pr] = col[pc];
  col[pc] = p;
  if(D > 1e6) {
    T /= D;
    D = 1;
  }
  return p;
}

}  // end namespace Gambit::gametracer
}  // end namespace Gambit
