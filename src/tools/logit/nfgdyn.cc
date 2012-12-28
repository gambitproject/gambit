//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/logit/nfgdyn.cc
// Computation of stable quantal response equilibria via perturbed
// best-reply dynamics
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
#include <cmath>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <cerrno>
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

void LogitBR(const Gambit::MixedStrategyProfile<double> &p_profile, double p_lambda,
	     Gambit::MixedStrategyProfile<double> &p_br)
{
  Gambit::Game nfg = p_profile.GetGame();

  for (int pl = 1; pl <= nfg->NumPlayers(); pl++) {
    Gambit::Array<double> lval(nfg->GetPlayer(pl)->NumStrategies());
    double sum = 0.0;

    for (int st = 1; st <= nfg->GetPlayer(pl)->NumStrategies(); st++) {
      lval[st] = exp(p_lambda * p_profile.GetStrategyValue(nfg->GetPlayer(pl)->GetStrategy(st)));
      sum += lval[st];
    }

    for (int st = 1; st <= nfg->GetPlayer(pl)->NumStrategies(); st++) {
      p_br[nfg->GetPlayer(pl)->GetStrategy(st)] = lval[st] / sum;
    }
  }
}

void Randomize(Gambit::MixedStrategyProfile<double> &p_profile)
{
  Gambit::Game nfg = p_profile.GetGame();

  ((Gambit::Vector<double> &) p_profile) = 0.0;

  for (int pl = 1; pl <= nfg->NumPlayers(); pl++) {
    double sum = 0.0;
    for (int st = 1; st < nfg->GetPlayer(pl)->NumStrategies(); st++) {
      p_profile[nfg->GetPlayer(pl)->GetStrategy(st)] = (1.0 - sum) * (double) rand() / (double) RAND_MAX;
      sum += p_profile[nfg->GetPlayer(pl)->GetStrategy(st)];
    }

    p_profile[nfg->GetPlayer(pl)->GetStrategy(nfg->GetPlayer(pl)->NumStrategies())] = 1.0 - sum;
  }
}

void PrintProfile(std::ostream &p_stream,
		  const std::string &p_label,
		  const Gambit::MixedStrategyProfile<double> &p_profile)
{
  p_stream << p_label;
  for (int i = 1; i <= p_profile.MixedProfileLength(); i++) {
    p_stream.setf(std::ios::fixed);
    p_stream << "," << std::setprecision(g_numDecimals) << p_profile[i];
  }

  p_stream << std::endl;
}

double Distance(const Gambit::MixedStrategyProfile<double> &a, const Gambit::MixedStrategyProfile<double> &b)
{
  double dist = 0.0;
  for (int i = 1; i <= a.MixedProfileLength(); i++) {
    if (fabs(a[i] - b[i]) > dist) {
      dist = fabs(a[i] - b[i]);
    }
  }

  return dist;
} 

bool ReadProfile(std::istream &p_stream, Gambit::Array<double> &p_profile)
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

int main(int argc, char *argv[])
{
  int stopAfter = 100;
  double lambda;
  bool lambdaSpec = false;
  double tol = 1.0e-6;
  std::string startFile = "";

  int c;
  while ((c = getopt(argc, argv, "f:n:l:t:p:")) != -1) {
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
    case 'p':
      startFile = optarg;
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

  std::istream* input_stream = &std::cin;
  std::ifstream file_stream;
  if (optind < argc) {
    file_stream.open(argv[optind]);
    if (!file_stream.is_open()) {
      std::ostringstream error_message;
      error_message << argv[0] << ": " << argv[optind];
      perror(error_message.str().c_str());
      exit(1);
    }
    input_stream = &file_stream;
  }

  Gambit::Game nfg;

  try {
    nfg = Gambit::ReadGame(*input_stream);
  }
  catch (...) {
    return 1;
  }

  if (startFile == "") {
    for (int i = 1; i <= stopAfter; i++) {
      Gambit::MixedStrategyProfile<double> profile(nfg->NewMixedStrategyProfile(0.0));
      Randomize(profile);
      
      PrintProfile(std::cout, "start", profile);

      Gambit::MixedStrategyProfile<double> br(nfg->NewMixedStrategyProfile(0.0));
    
      double c_delta = .001;
      
      do {
	LogitBR(profile, lambda, br);
	(Gambit::Vector<double> &) profile =
	  (((Gambit::Vector<double> &) profile) * (1.0 - c_delta) + 
	   ((Gambit::Vector<double> &) br) * c_delta);
      } while (Distance(profile, br) > tol);
    
      PrintProfile(std::cout, "QRE", profile);
    }
  }
  else {
    Gambit::Array<double> x(nfg->MixedProfileLength() + 1);
    std::ifstream startData(startFile.c_str());
    ReadProfile(startData, x);
    Gambit::MixedStrategyProfile<double> profile(nfg->NewMixedStrategyProfile(0.0));
    for (int i = 1; i <= profile.MixedProfileLength(); i++) {
      profile[i] = x[i+1];
      if (profile[i] == 0.0) {
	profile[i] = 0.0001;
      }
    }
    lambda = x[1];
    double c_delta = .001;

    Gambit::MixedStrategyProfile<double> br(nfg->NewMixedStrategyProfile(0.0));
    
    do {
      PrintProfile(std::cout, "step", profile);
      LogitBR(profile, lambda, br);
      (Gambit::Vector<double> &) profile =
	(((Gambit::Vector<double> &) profile) * (1.0 - c_delta) + 
	 ((Gambit::Vector<double> &) br) * c_delta);
    } while (Distance(profile, br) > tol);

    PrintProfile(std::cout, "QRE", profile);
  }
}


