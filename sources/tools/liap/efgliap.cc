//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute Nash equilibria via Lyapunov function minimization
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

#include <unistd.h>
#include <iostream>
#include <fstream>

#include "libgambit/libgambit.h"
#include "funcmin.h"

class EFLiapFunc : public gC1Function<double>  {
private:
  mutable long _nevals;
  gbtEfgGame _efg;
  mutable gbtBehavProfile<double> _p;

  double Value(const gbtVector<double> &x) const;
  bool Gradient(const gbtVector<double> &, gbtVector<double> &) const;

public:
  EFLiapFunc(gbtEfgGame, const gbtBehavProfile<double> &);
  virtual ~EFLiapFunc();
    
  long NumEvals(void) const  { return _nevals; }
};


EFLiapFunc::EFLiapFunc(gbtEfgGame E,
		       const gbtBehavProfile<double> &start)
  : _nevals(0L), _efg(E), _p(start)
{ }

EFLiapFunc::~EFLiapFunc()
{ }


double EFLiapFunc::Value(const gbtVector<double> &v) const
{
  _nevals++;
  ((gbtVector<double> &) _p).operator=(v);
    //_p = v;
  return _p.LiapValue();
}

//
// This function projects a gradient into the plane of the simplex.
// (Actually, it works by computing the projection of 'x' onto the
// vector perpendicular to the plane, then subtracting to compute the
// component parallel to the plane.)
//
static void Project(gbtVector<double> &x, const gbtArray<int> &lengths)
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

bool EFLiapFunc::Gradient(const gbtVector<double> &x,
			  gbtVector<double> &grad) const
{
  const double DELTA = .00001;

  ((gbtVector<double> &) _p).operator=(x);
  for (int i = 1; i <= x.Length(); i++) {
    _p[i] += DELTA;
    double value = Value(_p.GetDPVector());
    _p[i] -= 2.0 * DELTA;
    value -= Value(_p.GetDPVector());
    _p[i] += DELTA;
    grad[i] = value / (2.0 * DELTA);
  }

  Project(grad, _p.GetPVector().Lengths());

  return true;
}

static void PickRandomProfile(gbtBehavProfile<double> &p)
{
  double sum, tmp;

  for (int pl = 1; pl <= p.GetGame()->NumPlayers(); pl++)  {
    for (int iset = 1; iset <= p.GetGame()->GetPlayer(pl)->NumInfosets();
	 iset++)  {
      sum = 0.0;
      int act;
    
      for (act = 1; act < p.Support().NumActions(pl, iset); act++)  {
	do
	  tmp = ((double) rand()) / ((double) RAND_MAX);
	while (tmp + sum > 1.0);
	p(pl, iset, act) = tmp;
	sum += tmp;
      }
  
// with truncation, this is unnecessary
      p(pl, iset, act) = 1.0 - sum;
    }
  }
}

int g_numDecimals = 6;

void PrintProfile(std::ostream &p_stream,
		  const std::string &p_label,
		  const gbtBehavProfile<double> &p_profile)
{
  p_stream << p_label;
  for (int i = 1; i <= p_profile.Length(); i++) {
    p_stream.setf(std::ios::fixed);
    p_stream << ", " << std::setprecision(g_numDecimals) << p_profile[i];
  }

  p_stream << std::endl;
}

bool ReadProfile(std::istream &p_stream,
		 gbtBehavProfile<double> &p_profile)
{
  for (int i = 1; i <= p_profile.Length(); i++) {
    if (p_stream.eof() || p_stream.bad()) {
      return false;
    }

    p_stream >> p_profile[i];
    if (i < p_profile.Length()) {
      char comma;
      p_stream >> comma;
    }
  }

  // Read in the rest of the line and discard
  std::string foo;
  std::getline(p_stream, foo);
  return true;
}

void PrintBanner(std::ostream &p_stream)
{
  p_stream << "Compute Nash equilibria by minimizing the Lyapunov function\n";
  p_stream << "Gambit version " VERSION ", Copyright (C) 2005, The Gambit Project\n";
  p_stream << "This is free software, distributed under the GNU GPL\n\n";
}

void PrintHelp(char *progname)
{
  PrintBanner(std::cerr);
  std::cerr << "Usage: " << progname << " [OPTIONS]\n";
  std::cerr << "Accepts strategic game on standard input.\n";
  std::cerr << "With no options, attempts to compute one equilibrium starting at centroid.\n";

  std::cerr << "Options:\n";
  std::cerr << "  -d DECIMALS      print probabilities with DECIMALS digits\n";
  std::cerr << "  -h               print this help message\n";
  std::cerr << "  -n COUNT         number of starting points to generate\n";
  std::cerr << "  -s FILE          file containing starting points\n";
  std::cerr << "  -q               quiet mode (suppresses banner)\n";
  std::cerr << "  -v               verbose mode (shows intermediate output)\n";
  std::cerr << "                   (default is to only show equilibria)\n";
  exit(1);
}

int m_stopAfter = 0;
int m_numTries = 10;
int m_maxits1 = 100;
int m_maxitsN = 20;
double m_tol1 = 2.0e-10;
double m_tolN = 1.0e-10;

int main(int argc, char *argv[])
{
  opterr = 0;
  std::string startFile;
  bool useRandom = false;
  bool quiet = false, verbose = false;

  int c;
  while ((c = getopt(argc, argv, "d:n:s:hqv")) != -1) {
    switch (c) {
    case 'd':
      g_numDecimals = atoi(optarg);
      break;
    case 'n':
      m_numTries = atoi(optarg);
      break;
    case 's':
      startFile = optarg;
      break;
    case 'h':
      PrintHelp(argv[0]);
      break;
    case 'q':
      quiet = true;
      break;
    case 'v':
      verbose = true;
      break;
    case '?':
      if (isprint(optopt)) {
	std::cerr << argv[0] << ": Unknown option `-" << ((char) optopt) << "'.\n";
      }
      else {
	std::cerr << argv[0] << ": Unknown option character `\\x" << optopt << "`.\n";
      }
      return 1;
    default:
      abort();
    }
  }

  if (!quiet) {
    PrintBanner(std::cerr);
  }

  gbtEfgGame efg;

  try {
    efg = ReadEfg(std::cin);
  }
  catch (...) {
    return 1;
  }


  gbtList<gbtBehavProfile<double> > starts;

  if (startFile != "") {
    std::ifstream startPoints(startFile.c_str());

    while (!startPoints.eof() && !startPoints.bad()) {
      gbtBehavProfile<double> start(efg);
      if (ReadProfile(startPoints, start)) {
	starts.Append(start);
      }
    }
  }
  else {
    // Generate the desired number of points randomly
    for (int i = 1; i <= m_numTries; i++) {
      gbtBehavProfile<double> start(efg);
      PickRandomProfile(start);
      starts.Append(start);
    }
  }

  static const double ALPHA = .00000001;

  try {
    for (int i = 1; i <= starts.Length(); i++) {
      gbtBehavProfile<double> p(starts[i]);

      if (verbose) {
	PrintProfile(std::cout, "start", p);
      }

      EFLiapFunc F(efg, p);

      // if starting vector not interior, perturb it towards centroid
      int kk;
      for (int kk = 1; kk <= p.Length() && p[kk] > ALPHA; kk++);
      if (kk <= p.Length()) {
	gbtBehavProfile<double> c(efg);
	for (int k = 1; k <= p.Length(); k++) {
	  p[k] = c[k]*ALPHA + p[k]*(1.0-ALPHA);
	}
      }

      gbtMatrix<double> xi(p.Length(), p.Length());
  
      gConjugatePR minimizer(p.Length());
      gbtVector<double> gradient(p.Length()), dx(p.Length());
      double fval;
      minimizer.Set(F, p.GetDPVector(), fval, gradient, .01, .0001);

      try {
	for (int iter = 1; iter <= m_maxitsN; iter++) {
	  if (!minimizer.Iterate(F, p.GetDPVector(), fval, gradient, dx)) {
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
  catch (...) {
    return 1;
  }

  return 0;
}

