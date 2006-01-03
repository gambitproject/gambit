//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Computation of stable quantal response equilibria via perturbed
// best-reply dynamics
//
// This file is part of Gambit
// Copyright (c) 2005, The Gambit Project
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
#include <math.h>
#include <iostream>
#include <iomanip>
#include "libgambit/libgambit.h"

//
// This program attempts to identify the quantal response equilibria
// which are stable under the perturbed best-reply dynamics for a given
// value of the logit precision parameter 'lambda'.
//
// The program takes a normal form game on standard input.  The following
// command-line parameters modify its operation:
//
// -f #:  Number of decimal places to display in outputting profiles
//        (Default is 6)
// -n #:  Number of random starting points to generate (default is 100)
// -l #:  Value of lambda (required)
// -t #:  Tolerance for stopping criterion (parameter is exponent to
//        10^(-k), default is 6).
// 

int g_numDecimals = 6;

void LogitBR(const gbtMixedProfile<double> &p_profile, double p_lambda,
	     gbtMixedProfile<double> &p_br)
{
  Gambit::Game nfg = p_profile.GetGame();

  for (int pl = 1; pl <= nfg->NumPlayers(); pl++) {
    gbtArray<double> lval(nfg->GetPlayer(pl)->NumStrategies());
    double sum = 0.0;

    for (int st = 1; st <= nfg->GetPlayer(pl)->NumStrategies(); st++) {
      lval[st] = exp(p_lambda * p_profile.GetPayoff(pl, pl, st));
      sum += lval[st];
    }

    for (int st = 1; st <= nfg->GetPlayer(pl)->NumStrategies(); st++) {
      p_br(pl, st) = lval[st] / sum;
    }
  }
}

void Randomize(gbtMixedProfile<double> &p_profile)
{
  Gambit::Game nfg = p_profile.GetGame();

  ((gbtVector<double> &) p_profile) = 0.0;

  for (int pl = 1; pl <= nfg->NumPlayers(); pl++) {
    double sum = 0.0;
    for (int st = 1; st < nfg->GetPlayer(pl)->NumStrategies(); st++) {
      p_profile(pl, st) = (1.0 - sum) * (double) rand() / (double) RAND_MAX;
      sum += p_profile(pl, st);
    }

    p_profile(pl, nfg->GetPlayer(pl)->NumStrategies()) = 1.0 - sum;
  }
}

void PrintProfile(std::ostream &p_stream,
		  const std::string &p_label,
		  const gbtMixedProfile<double> &p_profile)
{
  p_stream << p_label;
  for (int i = 1; i <= p_profile.Length(); i++) {
    p_stream.setf(std::ios::fixed);
    p_stream << "," << std::setprecision(g_numDecimals) << p_profile[i];
  }

  p_stream << std::endl;
}

double Distance(const gbtMixedProfile<double> &a, const gbtMixedProfile<double> &b)
{
  double dist = 0.0;
  for (int i = 1; i <= a.Length(); i++) {
    if (fabs(a[i] - b[i]) > dist) {
      dist = fabs(a[i] - b[i]);
    }
  }

  return dist;
} 

int main(int argc, char *argv[])
{
  int stopAfter = 100;
  double lambda;
  bool lambdaSpec = false;
  double tol = 1.0e-6;

  int c;
  while ((c = getopt(argc, argv, "f:n:l:t:")) != -1) {
    switch (c) {
    case 'f':
      g_numDecimals = atoi(optarg);
      break;
    case 'n':
      stopAfter = atoi(optarg);
      break;
    case 'l':
      lambda = atof(optarg);
      lambdaSpec = true;
      break;
    case 't':
      tol = pow(10.0, (double) -atoi(optarg));
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

  if (!lambdaSpec) {
    std::cerr << argv[0] << ": Required parameter -l missing.\n";
    exit(1);
  }

  Gambit::Game nfg;

  try {
    nfg = Gambit::ReadNfg(std::cin);
  }
  catch (...) {
    return 1;
  }

  for (int i = 1; i <= stopAfter; i++) {
    gbtMixedProfile<double> profile(nfg);
    Randomize(profile);

    PrintProfile(std::cout, "start", profile);

    gbtMixedProfile<double> br(nfg);
    
    double c_delta = .001;

    do {
      LogitBR(profile, lambda, br);
      (gbtVector<double> &) profile =
	(((gbtVector<double> &) profile) * (1.0 - c_delta) + 
	 ((gbtVector<double> &) br) * c_delta);
    } while (Distance(profile, br) > tol);
    
    PrintProfile(std::cout, "QRE", profile);
  }
}


