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

#include <libgambit/libgambit.h>

#include "cmatrix.h"
#include "gnm.h"
#include "gnmgame.h"

extern bool g_verbose;
extern int g_numDecimals;

void PrintProfile(std::ostream &p_stream, 
		  const std::string &p_label,
		  const cvector &p_profile) 
{
  p_stream << p_label;
  for (int i = 0; i < p_profile.getm(); i++) {
    p_stream.setf(std::ios::fixed);
    p_stream << ',' << std::setprecision(g_numDecimals) << p_profile[i];
  }
  p_stream << std::endl;
}

// gnm(A,g,Eq,steps,fuzz,LNMFreq,LNMMax,LambdaMin,wobble,threshold)
// ----------------------------------------------------------------
// This executes the GNM algorithm on game A.
// Interpretation of parameters:
// g: perturbation ray.
// Eq: an array of equilibria will be stored here
// steps: number of steps to take within a support cell; higher 
//        values of this parameter slow GNM down, but may help it
//        avoid getting off the path.
// fuzz: a small floating point cutoff for a variety of things.
//       can probably be left at 1e-12.
// LNMFreq: a Local Newton Method subroutine will be run every
//          LNMFreq steps to decrease accumulated errors.  This 
//          executes fairly quickly, so LNMFreq can be around 3.
// LNMMax: the maximum allowed iterations within the LNM algorithm.
// LambdaMin: should always be negative.  Once the trajectory
//            gets this far out, the algorithm terminates, assuming
//            that there are no more equilibria on the path.
// wobble: this is a boolean value indicating whether or not to use
//         "wobbles" of the perturbation vector to remove
//         accumulated errors.  This removes the theoretical guarantee
//         of convergence, but in practice may help keep GNM on the path.
// threshold: the equilibrium error threshold for doing a wobble.  If
//            wobbles are disabled, GNM will terminate if the error
//            reaches this threshold.

int GNM(gnmgame &A, cvector &g, cvector **&Eq, int steps, double fuzz, int LNMFreq, int LNMMax, double LambdaMin, bool wobble, double threshold) {
  int i, // utility variables
    bestAction,  
    k, 
    j, 
    n, 
    n_hat, // player whose pure strategy next enters or leaves the support
    s_hat_old=-1, // the last pure strategy to enter or leave the support
    s_hat, // the next pure strategy to enter or leave the support
    Index = 1, // index of the equilibrium we're moving towards
    numEq = 0, // number of equilibria found so far
    stepsLeft; // number of linear steps remaining until we hit the boundary

  int N = A.getNumPlayers(), 
    M = A.getNumActions(); // the two most important cvector sizes, stored locally for brevity
  double bestPayoff, 
    det, // determinant of the jacobian
    newV, // utility variable
    lambda, // current position along the ray
    dlambda, // derivative of lambda w.r.t time
    minBound, // distance to closest change of support
    bound, // utility variable
    del, // amount of time required to reach the next support boundary, 
    // assuming linear cvector field
    delta, // the actual amount of time we will step forward (smaller than del)
    ee,
    V = 0.0; // scale factor for perturbation

  std::vector<int> s(M); // current best responses
  std::vector<int> B(M); // current support

  for (int i = 0; i < M; B[i++] = 0);

  cmatrix DG(M,M), // jacobian of the payoff function
    R(M,M), // jacobian of the retraction operator
    I(M,M,1,1), // identity
    Dpsi, // jacobian of the cvector field
    J(M,M); // adjoint of Dpsi

  cvector sigma(M), // current strategy profile
    g0(M), // original perturbation ray
    z(M), // current position in space of games
    v(M), // current cvector of payoffs for each pure strategy
    dz(M), // derivative of z w.r.t. time
    dv(M), // derivative of v w.r.t. time
    nothing(M,0),// cvector of all zeros
    err(M),
    backup(M); 


  // utility variables for use as intermediate values in computations
  cmatrix Y1(M,M), Y2(M,M), Y3(M,M);
  cvector G(N), yn1(N), ym1(M), ym2(M), ym3(M);

  // INITIALIZATION
  Eq = (cvector **)malloc(sizeof(cvector *));

  // Find the lone equilibrium of the perturbed game
  for(n = 0; n < N; n++) {
    bestPayoff = g[A.firstAction(n)];
    bestAction = A.firstAction(n);
    for(j = bestAction+1; j < A.lastAction(n); j++) {
      if(g[j] > bestPayoff) {
	bestPayoff = g[j];
	bestAction = j;
      }
    }
    s[n] = bestAction;
    B[bestAction] = 1;
    G[n] = bestPayoff;
  }

  // initialize sigma to be the pure strategy profile
  // that is the lone equilibrium of the perturbed game
  for(i = 0; i < M; i++) {
    sigma[i] = (double)B[i];
  }

  if (g_verbose) {
    PrintProfile(std::cout, "start", sigma);
  }

  A.payoffMatrix(DG, sigma, fuzz);
  DG.multiply(sigma, v);
  v /= (double)(N-1);

  // Scale g until the equilibrium sigma calculated above
  // is in fact the one unique equilibrium, and set lambda
  // equal to 1

  V = 0;

  for(n = 0; n < N; n++) {
    yn1[n] = v[s[n]];
    for(i = A.firstAction(n); i < A.lastAction(n); i++) {
      if(!B[i]) {
        if (G[n]-g[i]<threshold) g[i]-=threshold;  //make sure we don't divide by (almost) zero
        newV = (v[i]-yn1[n]) / (G[n]-g[i]);
        if(newV > V) 
          V = newV;
      }
    }
  }
       
  lambda = 1.0;  // we scale g instead
  V = V+1; // a little extra padding
  g *= V;
/*
  for(n = 0; n < N; n++) {
    yn1[n] = v[s[n]]; // yn1[n] is the payoff n receives for the action we wish to make dominant
    for(i = A.firstAction(n); i < A.lastAction(n); i++) {
      if(B[i]) // if i is the action we wish to make dominant
	newV = yn1[n]-G[n];
      else
	newV = yn1[n]-G[n]*(v[i]-yn1[n])/(g[i]-G[n]);
      if(newV>V)
	V = newV;
    }
  }

  lambda = 1.0;  // we scale g instead
  V = V+1; // a little extra padding
  for(n = 0; n < N; n++)
    for(i = A.firstAction(n); i < A.lastAction(n); i++) {
      g[i] *= (V-yn1[n])/G[n];
    }
*/
  if(N <= 2) { // ensure we don't do small steps and LNM
    LNMFreq = 0;
    steps = 1;
  }

  z = g;
  z += v;
  z += sigma;
  //  z=sigma+v+g*lambda;

  A.retractJac(R,B);

  // this outer while loop executes once for each support boundary
  // that the path crosses.
  while(1) {
    k = 0; // iteration counter; when k reaches LNMFreq, run LNM
     // within a single boundary, support unchanged

    // take the specified number of steps within these support boundaries.  
    for(stepsLeft = steps; stepsLeft > 0; stepsLeft--) { 
      //find J = Adj psi
      J = I;
      J += DG;
      J *= R;
      J -= I;
      J.negate();
      // J = I-((I+DG)*R);
      det = J.adjoint(); // sets J = adjoint(J)

      // find derivatives of z and lambda
      J.multiply(g,dz);
      dz.negate();      
       //dz = -(J*g);
      dlambda = -det;
      R.multiply(dz, ym1);
      DG.multiply(ym1,dv);
      //dv = (DG*(R*dz));
      ym1 = g;
      ym1 *= dlambda;
      dv += ym1;
      //dv += g*dlambda;
      
      //Calculate payoff cvector
      DG.multiply(sigma, v);      
      v /=  (double)(N-1);
      ym1 = g;
      ym1 *= lambda;
      v += ym1;
      // v = DG*sigma / (double)(N-1) + g * lambda;
      
      //Find next action that will enter or leave the support
      //This bit pretends that z and v change linearly and calculates
      //at what point z will equal v at a certain action; this
      //indicates that the action's probability is either
      //becoming 0 or becoming positive.
      minBound = BIGFLOAT;
      for(n = 0; n < N; n++) {
	for(i = A.firstAction(n); i < A.lastAction(n); i++) {
	  // do not cross the same boundary we just crossed
	  if(dz[i] != dv[s[n]] && s_hat_old != i) {
	    bound= (z[i]-v[s[n]])/(dv[s[n]]-dz[i]);
	    if(bound > 0.0) { // forward in time
	      if(bound< minBound) {
		minBound = bound;
		s_hat = i;
		n_hat = n;
	      }
	    }
	  }
	}
      }

      delta = del = minBound;

      // if the path doesn't seem to cross any more support boundaries,
      // and there is no equilibrium in sight, then quit; we don't know
      // how big a step size to take, and anyway there's a good chance
      // there are no more equilibria on the path.  This could be
      // handled differently.
      if(minBound == BIGFLOAT && Index*(lambda+dlambda*delta) > 0) {
	if(g_verbose) std::cerr<<"gnm(): return since the path crosses no more support boundaries and no next eqlm"<<endl;
	return numEq;
      }
      
      // each step covers 1.0/steps of the distance to the boundary
      delta = del / stepsLeft;

      // test whether lambda will become 0 in the course of this
      // step, which means there's an equilibrium there
      if(Index*(lambda+dlambda*delta) <= 0.0) {
	// if there's no next support boundary, treat the equilibrium
	// as the next support boundary and step up to it incrementally
	if(minBound == BIGFLOAT && N > 2 && stepsLeft > 1) { 
	  del = -lambda / dlambda;
	  delta = del / stepsLeft;
	} else {
	  delta -= -lambda / dlambda; // delta is now just big enough
	  ym1 = dz;                   // to get us to the equilibrium
	  ym1 *= (-lambda / dlambda); 
	  z += ym1;
	  //  z += dz*delta;
	  lambda = 0;
	  A.retract(sigma, z);
	  A.payoffMatrix(DG, sigma, fuzz);
	  ee = 0.0;
	  if(N > 2) { // if N=2, the graph is linear, so we are at a
	    //precise equilibrium.  otherwise, refine it.
	    J = DG;
	    J += I;
	    J *= R;
	    J -= I; 
	    J.negate();
	    //J=I-((I+DG)*R);
	    det = J.adjoint();
	    ee = A.LNM(z, nothing, det, J, DG, sigma, LNMMax, fuzz,ym1,ym2,ym3);
	  }
	  for (int idx=0;idx<M;idx++)
	    if (! isfinite(sigma[idx])){
	              if(g_verbose) std::cerr<<"gnm(): return since sigma is not finite"<<endl;
	              return numEq;
	    }
	  if(ee < fuzz) { // only save high quality equilibria;
	    // this restriction could be removed.
	    Eq = (cvector **)realloc(Eq, (numEq+2)*sizeof(cvector *));	
	    Eq[numEq] = new cvector(M);
	    *(Eq[numEq++]) = sigma;

	    PrintProfile(std::cout, "NE", sigma);
      }
	  Index = -Index;
	  s_hat_old = -1;
	  stepsLeft++;
	  continue;
	}
      }
      if(del == BIGFLOAT) {
	if (g_verbose) std::cerr<<"gnm(): return since no next support boundary after this eqlm"<<endl;
	return numEq;
      }

      backup = z;

      // do the step
      ym1 = dz;
      ym1 *= delta;
      z += ym1;
      // z = z+dz*delta;
      lambda += dlambda*delta;

      // if we're sufficiently far out on the ray in the reverse
      // direction, we're probably not going back
      if(lambda < LambdaMin && Index == -1) {
	if (g_verbose) std::cerr<<"gnm(): return due to too far out in the reverse direction"<<endl;
	return numEq;
      }
      A.retract(sigma,z);
      A.payoffMatrix(DG, sigma,fuzz);
      
      if(N <= 2) 
	break; // already at the support boundary
      
      DG.multiply(sigma,err);
      err /= (double)(N-1);
      g0 = g;
      g0 *= lambda;
      err += g0;
      err += sigma;
      err -= z;
      err.negate();
      ee = max(err.max(),-err.min());
      if(ee < fuzz && stepsLeft > 2) { // path is probably near-linear;
       	stepsLeft = 2;                 // step all the way to boundary
	k = LNMFreq - 1;               // then run LNM
      }
      if(ee > threshold) { // if we've accumulated too much error, either
	if(wobble) {       // wobble or quit.
	  DG.multiply(sigma, ym1);
	  ym1 /= (double)(N-1);
	  g = z;
	  g -= sigma;
	  g -= ym1;
	  g /= lambda;
	  // g = ((z-sigma)-((DG*sigma) / (double)(N-1)))/lambda;
	} else {
	  if(g_verbose) std::cerr<<"gnm(): return due to too much error. error is "<<ee<<endl;
	  return numEq;
	}
      }

      // if we've done LNMMax repetitions, time to get back on the path
      if(stepsLeft > 1 && (++k == LNMFreq)) {
	A.LNM(z, g0, det, J, DG, sigma, LNMMax, fuzz,ym1,ym2,ym3);
	k = 0;
      }
    } // end of for loop

    // now we've reached a support boundary

    // if a player's current best response is leaving the
    // support, we must find a new one for that player
    if(s[n_hat] == s_hat)
      for(i = A.firstAction(n_hat); i < A.lastAction(n_hat); i++)
	if(B[i] && i != s_hat) {
	  s[n_hat] = i;
	  break;
	}
    B[s_hat] = !B[s_hat];
    A.retractJac(R,B);
    s_hat_old = s_hat;
    A.retract(ym1, z);
    sigma = ym1;
    sigma.support(B);
    sigma.unfuzz(fuzz);
    A.normalizeStrategy(sigma);

    if (g_verbose) {
      PrintProfile(std::cout, Gambit::lexical_cast<std::string>(lambda), sigma);
    }

    z -= ym1;
    z += sigma;
    // z = (z-x)+sigma;
     
    // wobble the perturbation cvector to put us back on an equilibrium
    if(N > 2 && wobble) {
      A.payoffMatrix(DG, sigma, fuzz);
      DG.multiply(sigma, ym1);
      ym1 /= (double)(N-1);
      g = z;
      g -= sigma;
      g -= ym1;
      g /= lambda;
      // g = ((z-sigma)-((DG*sigma) / (double)(N-1)))/lambda;

    }
  }
}
