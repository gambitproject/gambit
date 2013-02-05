//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/liap/nfgliap.cc
// Compute Nash equilibria by minimizing Liapunov function
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

#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <fstream>

#include "libgambit/libgambit.h"
#include "funcmin.h"

extern int m_stopAfter;
extern int m_numTries;
extern int m_maxits1;
extern int m_maxitsN;
extern double m_tol1;
extern double m_tolN;
extern std::string startFile;
extern bool useRandom;
extern int g_numDecimals;
extern bool verbose;

//---------------------------------------------------------------------
//                        class NFLiapFunc
//---------------------------------------------------------------------

class NFLiapFunc : public gC1Function<double>  {
private:
  mutable long _nevals;
  Gambit::Game _nfg;
  mutable Gambit::MixedStrategyProfile<double> _p;

  double Value(const Gambit::Vector<double> &) const;
  bool Gradient(const Gambit::Vector<double> &, Gambit::Vector<double> &) const;

  double LiapDerivValue(int, int, const Gambit::MixedStrategyProfile<double> &) const;
    

public:
  NFLiapFunc(const Gambit::Game &, const Gambit::MixedStrategyProfile<double> &);
  virtual ~NFLiapFunc();
    
  long NumEvals(void) const  { return _nevals; }
};

NFLiapFunc::NFLiapFunc(const Gambit::Game &N,
		       const Gambit::MixedStrategyProfile<double> &start)
  : _nevals(0L), _nfg(N), _p(start)
{ }

NFLiapFunc::~NFLiapFunc()
{ }

double NFLiapFunc::LiapDerivValue(int i1, int j1,
				  const Gambit::MixedStrategyProfile<double> &p) const
{
  int i, j;
  double x, x1, psum;
  
  x = 0.0;
  for (i = 1; i <= _nfg->NumPlayers(); i++)  {
    psum = 0.0;
    for (j = 1; j <= p.GetSupport().NumStrategies(i); j++)  {
      psum += p[p.GetSupport().GetStrategy(i,j)];
      x1 = p.GetPayoff(p.GetSupport().GetStrategy(i, j)) - p.GetPayoff(i);
      if (i1 == i) {
	if (x1 > 0.0)
	  x -= x1 * p.GetPayoffDeriv(i, p.GetSupport().GetStrategy(i1, j1));
      }
      else {
	if (x1> 0.0)
	  x += x1 * (p.GetPayoffDeriv(i, 
				      p.GetSupport().GetStrategy(i, j),
				      p.GetSupport().GetStrategy(i1, j1)) - 
		     p.GetPayoffDeriv(i,
				      p.GetSupport().GetStrategy(i1, j1)));
      }
    }
    if (i == i1)  x += 100.0 * (psum - 1.0);
  }
  if (p[p.GetSupport().GetStrategy(i1, j1)] < 0.0) {
    x += p[p.GetSupport().GetStrategy(i1, j1)];
  }
  return 2.0 * x;
}

//
// This function projects a gradient into the plane of the simplex.
// (Actually, it works by computing the projection of 'x' onto the
// vector perpendicular to the plane, then subtracting to compute the
// component parallel to the plane.)
//
static void Project(Gambit::Vector<double> &x, const Gambit::Array<int> &lengths)
{
  int index = 1;
  for (int part = 1; part <= lengths.Length(); part++)  {
    double avg = 0.0;
    int j;
    for (j = 1; j <= lengths[part]; j++, index++)  {
      avg += x[index];
    }
    avg /= (double) lengths[part];
    index -= lengths[part];
    for (j = 1; j <= lengths[part]; j++, index++)  {
      x[index] -= avg;
    }
  }
}

bool NFLiapFunc::Gradient(const Gambit::Vector<double> &v, Gambit::Vector<double> &d) const
{
  ((Gambit::Vector<double> &) _p).operator=(v);
  int i1, j1, ii;
  
  for (i1 = 1, ii = 1; i1 <= _nfg->NumPlayers(); i1++) {
    for (j1 = 1; j1 <= _p.GetSupport().NumStrategies(i1); j1++) {
      d[ii++] = LiapDerivValue(i1, j1, _p);
    }
  }

  Project(d, _p.GetSupport().NumStrategies());
  return true;
}
  
double NFLiapFunc::Value(const Gambit::Vector<double> &v) const
{
  _nevals++;

  ((Gambit::Vector<double> &) _p).operator=(v);
  return _p.GetLiapValue();
}

static void PickRandomProfile(Gambit::MixedStrategyProfile<double> &p)
{
  double sum, tmp;

  for (int pl = 1; pl <= p.GetGame()->NumPlayers(); pl++)  {
    sum = 0.0;
    int st;
    
    for (st = 1; st < p.GetSupport().NumStrategies(pl); st++)  {
      do
	tmp = ((double) rand()) / ((double) RAND_MAX);
      while (tmp + sum > 1.0);
      p[p.GetSupport().GetStrategy(pl, st)] = tmp;
      sum += tmp;
    }
    p[p.GetSupport().GetStrategy(pl, st)] = 1.0 - sum;
  }
}

void PrintProfile(std::ostream &p_stream,
		  const std::string &p_label,
		  const Gambit::MixedStrategyProfile<double> &p_profile)
{
  p_stream << p_label;
  for (int i = 1; i <= p_profile.MixedProfileLength(); i++) {
    p_stream.setf(std::ios::fixed);
    p_stream << ", " << std::setprecision(g_numDecimals) << p_profile[i];
  }

  p_stream << std::endl;
}

bool ReadProfile(std::istream &p_stream,
		 Gambit::MixedStrategyProfile<double> &p_profile)
{
  for (int i = 1; i <= p_profile.MixedProfileLength(); i++) {
    if (p_stream.eof() || p_stream.bad()) {
      return false;
    }

    p_stream >> p_profile[i];
    if (i < p_profile.MixedProfileLength()) {
      char comma;
      p_stream >> comma;
    }
  }

  // Read in the rest of the line and discard
  std::string foo;
  std::getline(p_stream, foo);
  return true;
}

extern std::string startFile;

void SolveStrategic(const Gambit::Game &p_game)
{
  Gambit::List<Gambit::MixedStrategyProfile<double> > starts;

  if (startFile != "") {
    std::ifstream startPoints(startFile.c_str());

    while (!startPoints.eof() && !startPoints.bad()) {
      Gambit::MixedStrategyProfile<double> start(p_game->NewMixedStrategyProfile(0.0));
      if (ReadProfile(startPoints, start)) {
	starts.Append(start);
      }
    }
  }
  else {
    // Generate the desired number of points randomly
    for (int i = 1; i <= m_numTries; i++) {
      Gambit::MixedStrategyProfile<double> start(p_game->NewMixedStrategyProfile(0.0));
      PickRandomProfile(start);
      starts.Append(start);
    }
  }

  static const double ALPHA = .00000001;

  for (int i = 1; i <= starts.Length(); i++) {
    Gambit::MixedStrategyProfile<double> p(starts[i]);

    if (verbose) {
      PrintProfile(std::cout, "start", p);
    }

    NFLiapFunc F(p.GetGame(), p);

    // if starting vector not interior, perturb it towards centroid
    int kk;
    for (kk = 1; kk <= p.MixedProfileLength() && p[kk] > ALPHA; kk++);
    if (kk <= p.MixedProfileLength()) {
      Gambit::MixedStrategyProfile<double> centroid(p.GetSupport().NewMixedStrategyProfile<double>());
      for (int k = 1; k <= p.MixedProfileLength(); k++) {
	p[k] = centroid[k] * ALPHA + p[k] * (1.0-ALPHA);
      }
    }

    gConjugatePR minimizer(p.MixedProfileLength());
    Gambit::Vector<double> gradient(p.MixedProfileLength()), dx(p.MixedProfileLength());
    double fval;
    minimizer.Set(F, (const Gambit::Vector<double> &) p,
		  fval, gradient, .01, .0001);

    try {
      for (int iter = 1; iter <= m_maxitsN; iter++) {
	if (!minimizer.Iterate(F, (Gambit::Vector<double> &) p, 
			       fval, gradient, dx)) {
	  break;
	}

	if (sqrt(gradient.NormSquared()) < .001) {
	  PrintProfile(std::cout, "NE", p);
	  break;
	}
      }

      if (verbose && sqrt(gradient.NormSquared()) >= .001) {
	PrintProfile(std::cout, "end", p);
      }
    }
    catch (gFuncMinException &) { }
  }
}




