//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute Nash equilibria by minimizing Liapunov function
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
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

#include "numerical/gfuncmin.h"
#include "nfgliap.h"

//---------------------------------------------------------------------
//                        class NFLiapFunc
//---------------------------------------------------------------------

class NFLiapFunc : public gC1Function<double>  {
private:
  mutable long _nevals;
  const Nfg &_nfg;
  mutable MixedProfile<double> _p;

  double Value(const gVector<double> &) const;
  bool Gradient(const gVector<double> &, gVector<double> &) const;

  double LiapDerivValue(int, int, const MixedProfile<double> &) const;
    

public:
  NFLiapFunc(const Nfg &, const MixedProfile<double> &);
  virtual ~NFLiapFunc();
    
  long NumEvals(void) const  { return _nevals; }
};

NFLiapFunc::NFLiapFunc(const Nfg &N,
		       const MixedProfile<double> &start)
  : _nevals(0L), _nfg(N), _p(start)
{ }

NFLiapFunc::~NFLiapFunc()
{ }

double NFLiapFunc::LiapDerivValue(int i1, int j1,
				  const MixedProfile<double> &p) const
{
  int i, j;
  double x, x1, psum;
  
  x = 0.0;
  for (i = 1; i <= _nfg.NumPlayers(); i++)  {
    psum = 0.0;
    for (j = 1; j <= p.Support().NumStrats(i); j++)  {
      psum += p(i,j);
      x1 = p.Payoff(i, i, j) - p.Payoff(i);
      if (i1 == i) {
	if (x1 > 0.0) {
	  x -= x1 * p.Payoff(i, i1, j1);
	}
      }
      else {
	if (x1 > 0.0) {
	  x += x1 * (p.Payoff(i, i, j, i1, j1) - p.Payoff(i, i1, j1));
	}
      }
    }
  }
  if (p(i1, j1) < 0.0)   x += p(i1, j1);
  return 2.0 * x;
}

//
// This function projects a gradient into the plane of the simplex.
// (Actually, it works by computing the projection of 'x' onto the
// vector perpendicular to the plane, then subtracting to compute the
// component parallel to the plane.)  Also imposes binding nonnegativity
// constraints as appropriate.
//
static void Project(gVector<double> &grad, const gVector<double> &x,
		    const gArray<int> &lengths)
{
  int index = 1;
  for (int part = 1; part <= lengths.Length(); part++)  {
    double avg = 0.0;
    int nactive = 0;
    int j;
    for (j = 1; j <= lengths[part]; j++, index++)  {
      // If x[index] is small, assume nonnegativity is binding.
      // On the other hand, if gradient suggests that the minimizing
      // direction is towards the interior, let it go (relax constraint).
      if (x[index] > 1.0e-7 || grad[index] < 0.0) {
	avg += grad[index];
	nactive++;
      }
    }
    avg /= (double) nactive;
    index -= lengths[part];
    for (j = 1; j <= lengths[part]; j++, index++)  {
      if (x[index] > 1.0e-7 || grad[index] < 0.0) {
	grad[index] -= avg;
      }
      else {
	grad[index] = 0.0;
      }
    }
  }
}

bool NFLiapFunc::Gradient(const gVector<double> &v, gVector<double> &d) const
{
  ((gVector<double> &) _p).operator=(v);
  int i1, j1, ii;
  
  for (i1 = 1, ii = 1; i1 <= _nfg.NumPlayers(); i1++) {
    for (j1 = 1; j1 <= _p.Support().NumStrats(i1); j1++) {
      d[ii++] = LiapDerivValue(i1, j1, _p);
    }
  }

  // Project for constraints
  Project(d, v, _p.Lengths());
  return true;
}
  
double NFLiapFunc::Value(const gVector<double> &v) const
{
  static const double BIG1 = 100.0;
  static const double BIG2 = 100.0;

  _nevals++;

  ((gVector<double> &) _p).operator=(v);
  
  MixedProfile<double> tmp(_p);
  gPVector<double> payoff(_p);

  double x, result = 0.0, avg, sum;
  payoff = 0.0;
  
  for (int i = 1; i <= _nfg.NumPlayers(); i++)  {
    tmp.CopyRow(i, payoff);
    avg = sum = 0.0;

    // then for each strategy for player i, consider the value of
    // deviating to that strategy

    int j;
    for (j = 1; j <= _p.Support().NumStrats(i); j++)  {
      tmp(i, j) = 1.0;
      x = _p(i, j);
      payoff(i, j) = tmp.Payoff(i);
      avg += x * payoff(i, j);
      sum += x;
      if (x > 0.0)  x = 0.0;
      result += BIG1 * x * x;   // penalty for neg probabilities
      tmp(i, j) = 0.0;
    }

    tmp.CopyRow(i, _p);
    for (j = 1; j <= _p.Support().NumStrats(i); j++)  {
      x = payoff(i, j) - avg;
      if (x < 0.0)  x = 0.0;
      result += x * x;        // penalty for not best response
    }
    
    x = sum - 1.0;
    result += BIG2 * x * x;   // penalty for not summing to 1
  }
  return result;
}

static void PickRandomProfile(MixedProfile<double> &p)
{
  double sum, tmp;

  for (int pl = 1; pl <= p.Game().NumPlayers(); pl++)  {
    sum = 0.0;
    int st;
    
    for (st = 1; st < p.Support().NumStrats(pl); st++)  {
      do
	tmp = Uniform();
      while (tmp + sum > 1.0);
      p(pl, st) = tmp;
      sum += tmp;
    }
    p(pl, st) = 1.0 - sum;
  }
}

//---------------------------------------------------------------------
//                  class gbtNfgNashLiap: Member functions
//---------------------------------------------------------------------

gbtNfgNashLiap::gbtNfgNashLiap(void)
  : m_stopAfter(1), m_numTries(10), m_maxits1(100), m_maxitsN(20),
    m_tol1(2.0e-10), m_tolN(1.0e-10)
{ }

gList<MixedSolution> gbtNfgNashLiap::Solve(const gbtNfgSupport &p_support,
					   gStatus &p_status)
{
  static const double ALPHA = .00000001;
  MixedProfile<double> p(p_support);
  NFLiapFunc F(p.Game(), p);

  // if starting vector not interior, perturb it towards centroid
  int kk;
  for (kk = 1; kk <= p.Length() && p[kk] > ALPHA; kk++);
  if (kk <= p.Length()) {
    MixedProfile<double> centroid(p.Support());
    for (int k = 1; k <= p.Length(); k++) {
      p[k] = centroid[k] * ALPHA + p[k] * (1.0-ALPHA);
    }
  }

  gList<MixedSolution> solutions;

  try {
    for (int i = 1; ((m_numTries == 0 || i <= m_numTries) &&
		     (m_stopAfter == 0 || solutions.Length() < m_stopAfter));
	 i++) { 
      p_status.Get();
      p_status.SetProgress((double) i / (double) m_numTries,
			   gText("Attempt ") + ToText(i) + 
			   gText(", equilibria so far: ") +
			   ToText(solutions.Length())); 
      gConjugatePR minimizer(p.Length());
      gVector<double> gradient(p.Length()), dx(p.Length());
      double fval;
      minimizer.Set(F, p, fval, gradient, .01, .0001);

      try {
	for (int iter = 1; iter <= m_maxitsN; iter++) {
	  if (iter % 20 == 0) {
	    p_status.Get();
	  }
	  
	  if (!minimizer.Iterate(F, p, fval, gradient, dx)) {
	    break;
	  }

	  if (sqrt(gradient.NormSquared()) < .001 &&
	      fval < 1.0e-8) {
	    solutions.Append(MixedSolution(p, "Liap[NFG]"));
	    break;
	  }
	}
      }
      catch (gFuncMinException &) { }
      PickRandomProfile(p);
    }
  }
  catch (gSignalBreak &) {
    // Just stop and return any solutions found so far
  }
  // Any other exceptions propagate out, assuming something Real Bad happened

  return solutions;
}



