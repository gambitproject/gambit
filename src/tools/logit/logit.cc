//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Computation of quantal response equilibrium correspondence
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

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include "libgambit/libgambit.h"
#include "logbehav.h"

double g_maxDecel = 1.1;
double g_hStart = .03;
bool g_fullGraph = true;
int g_numDecimals = 6;
bool g_maxLike = false;
Gambit::Array<double> g_obsProbs;
double g_targetLambda = -1.0;


void PrintBanner(std::ostream &p_stream)
{
  p_stream << "Compute a branch of the logit equilibrium correspondence\n";
  p_stream << "Gambit version " VERSION ", Copyright (C) 2005, The Gambit Project\n";
  p_stream << "This is free software, distributed under the GNU GPL\n\n";
}

void PrintHelp(char *progname)
{
  PrintBanner(std::cerr);
  std::cerr << "Usage: " << progname << " [OPTIONS]\n";
  std::cerr << "Accepts game on standard input.\n";

  std::cerr << "Options:\n";
  std::cerr << "  -d DECIMALS      show equilibria as floating point with DECIMALS digits\n";
  std::cerr << "  -s STEP          initial stepsize (default is .03)\n";
  std::cerr << "  -a ACCEL         maximum acceleration (default is 1.1)\n";
  std::cerr << "  -m MAXLAMBDA     stop when reaching MAXLAMBDA (default is 1000000)\n";
  std::cerr << "  -l LAMBDA        compute QRE at `lambda` accurately\n";
  std::cerr << "  -L FILE          compute maximum likelihood estimates;\n";
  std::cerr << "                   read strategy frequencies from FILE\n";
  std::cerr << "  -h               print this help message\n";
  std::cerr << "  -q               quiet mode (suppresses banner)\n";
  std::cerr << "  -e               print only the terminal equilibrium\n";
  std::cerr << "                   (default is to print the entire branch)\n";
  exit(1);
}

//
// Read in a comma-separated values list of observed data values
//
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

extern void 
TraceStrategicPath(const Gambit::MixedStrategyProfile<double> &p_start,
		   double p_startLambda, double p_maxLambda, double p_omega);
extern void 
TraceAgentPath(const LogBehavProfile<double> &p_start,
	       double p_startLambda, double p_maxLambda, double p_omega);


int main(int argc, char *argv[])
{
  opterr = 0;

  bool quiet = false, useStrategic = false;
  double maxLambda = 1000000.0;
  std::string mleFile = "", startFile = "";

  int c;
  while ((c = getopt(argc, argv, "d:s:a:m:qehSL:p:l:")) != -1) {
    switch (c) {
    case 'q':
      quiet = true;
      break;
    case 'd':
      g_numDecimals = atoi(optarg);
      break;
    case 's':
      g_hStart = atof(optarg);
      break;
    case 'a':
      g_maxDecel = atof(optarg);
      break;
    case 'm':
      maxLambda = atof(optarg);
      break;
    case 'e':
      g_fullGraph = false;
      break;
    case 'h':
      PrintHelp(argv[0]);
      break;
    case 'S':
      useStrategic = true;
      break;
    case 'L':
      mleFile = optarg;
      break;
    case 'p':
      startFile = optarg;
      break;
    case 'l':
      g_targetLambda = atof(optarg);
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

  try {
    Gambit::Game game = Gambit::ReadGame(std::cin);

    if (mleFile != "" && (!game->IsTree() || useStrategic)) {
      g_obsProbs = Gambit::Array<double>(game->MixedProfileLength());
      std::ifstream mleData(mleFile.c_str());
      ReadProfile(mleData, g_obsProbs);
      g_maxLike = true;
    }
  

    if (!game->IsTree() || useStrategic) {
      game->BuildComputedValues();

      if (startFile == "") {
	Gambit::MixedStrategyProfile<double> start(game);
	TraceStrategicPath(start, 0.0, maxLambda, 1.0);
      }
      else {
	Gambit::Array<double> profile(game->MixedProfileLength() + 1);
	std::ifstream startData(startFile.c_str());
	ReadProfile(startData, profile);
	Gambit::MixedStrategyProfile<double> start(game);
	for (int i = 1; i <= start.Length(); i++) {
	  start[i] = profile[i+1];
	}
	TraceStrategicPath(start, profile[1], maxLambda, 1.0);
	std::cout << std::endl;
	TraceStrategicPath(start, profile[1], maxLambda, -1.0);
      }

    }
    else {
      LogBehavProfile<double> start(game);
      TraceAgentPath(start, 0.0, maxLambda, 1.0);
    }
    return 0;
  }
  catch (Gambit::InvalidFileException) {
    std::cerr << "Error: Game not in a recognized format.\n";
    return 1;
  }
  catch (...) {
    std::cerr << "Error: An internal error occurred.\n";
    return 1;
  }
}
