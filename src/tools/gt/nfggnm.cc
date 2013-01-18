//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/gt/nfggnm.cc
// Gambit frontend to Gametracer global Newton method
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
#include <getopt.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include <cerrno>
#include "libgambit/libgambit.h"

#include "nfgame.h"
#include "gnmgame.h"
#include "gnm.h"

// GNM CONSTANTS
const int STEPS = 100;
const double FUZZ = 1e-12;
const int LNMFREQ = 3;
const int LNMMAX = 10;
const double LAMBDAMIN = -10.0;
const bool WOBBLE = false;
const double THRESHOLD = 1e-2;

int g_numDecimals = 6;
bool g_verbose = false;
int g_numVectors = 1;
std::string g_startFile;

bool ReadProfile(std::istream &p_stream, cvector &p_profile)
{
  for (int i = 0; i < p_profile.getm(); i++) {
    if (p_stream.eof() || p_stream.bad()) {
      return false;
    }

    p_stream >> p_profile[i];
    if (i < p_profile.getm() - 1) {
      char comma;
      p_stream >> comma;
    }
  }

  // Read in the rest of the line and discard
  std::string foo;
  std::getline(p_stream, foo);
  return true;
}

extern void PrintProfile(std::ostream &, const std::string &,
			 const cvector &);

void PrintBanner(std::ostream &p_stream)
{
  p_stream << "Compute Nash equilibria using a global Newton method\n";
  p_stream << "Gametracer version 0.2, Copyright (C) 2002, Ben Blum and Christian Shelton\n";
  p_stream << "Gambit version " VERSION ", Copyright (C) 1994-2010, The Gambit Project\n";
  p_stream << "This is free software, distributed under the GNU GPL\n\n";
}

void PrintHelp(char *progname)
{
  PrintBanner(std::cerr);
  std::cerr << "Usage: " << progname << " [OPTIONS] [file]\n";
  std::cerr << "If file is not specified, attempts to read game from standard input.\n";

  std::cerr << "Options:\n";
  std::cerr << "  -d DECIMALS      show equilibria as floating point with DECIMALS digits\n";
  std::cerr << "  -h, --help       print this help message\n";
  std::cerr << "  -n COUNT         number of perturbation vectors to generate\n";
  std::cerr << "  -s FILE          file containing perturbation vectors\n";
  std::cerr << "  -q               quiet mode (suppresses banner)\n";
  std::cerr << "  -V, --verbose    verbose mode (shows intermediate output)\n";
  std::cerr << "  -v, --version    print version information\n";
  std::cerr << "                   (default is to only show equilibria)\n";
  exit(1);
}

void Solve(const Gambit::Game &p_game)
{
  int i;

  Gambit::Rational maxPay = p_game->GetMaxPayoff();
  Gambit::Rational minPay = p_game->GetMinPayoff();
  double scale = 1.0 / (maxPay - minPay);

  int *actions = new int[p_game->NumPlayers()];
  int veclength = p_game->NumPlayers();
  for (int pl = 1; pl <= p_game->NumPlayers(); pl++) {
    actions[pl-1] = p_game->GetPlayer(pl)->NumStrategies();
    veclength *= p_game->GetPlayer(pl)->NumStrategies();
  }
  cvector payoffs(veclength);
  
  gnmgame *A = new nfgame(p_game->NumPlayers(), actions, payoffs);
  
  int *profile = new int[p_game->NumPlayers()];
  for (Gambit::StrategyIterator iter(p_game); !iter.AtEnd(); iter++) {
    for (int pl = 1; pl <= p_game->NumPlayers(); pl++) {
      profile[pl-1] = (*iter)->GetStrategy(pl)->GetNumber() - 1;
    }

    for (int pl = 1; pl <= p_game->NumPlayers(); pl++) {
      A->setPurePayoff(pl-1, profile, 
		       (double) ((*iter)->GetPayoff(pl) - minPay) *
		       scale);
    }
  }

  cvector g(A->getNumActions()); // choose a random perturbation ray
  int numEq;

  if (g_startFile != "") {
    std::ifstream startVectors(g_startFile.c_str());

    while (!startVectors.eof() && !startVectors.bad()) {
      cvector **answers;
      if (ReadProfile(startVectors, g)) {
	g /= g.norm(); // normalized
	if (g_verbose) {
	  PrintProfile(std::cout, "pert", g);
	}

	numEq = GNM(*A, g, answers, STEPS, FUZZ, LNMFREQ, LNMMAX, LAMBDAMIN, WOBBLE, THRESHOLD);
	for (i = 0; i < numEq; i++) {
	  free(answers[i]);
	}
	free(answers);
      }
    }
  }
  else {
    for (int iter = 0; iter < g_numVectors; iter++) {
      cvector **answers;
      for(i = 0; i < A->getNumActions(); i++) {
#if !defined(HAVE_DRAND48)
	g[i] = rand();
#else
	g[i] = drand48();
#endif  // HAVE_DRAND48
      }
      g /= g.norm(); // normalized
      if (g_verbose) {
	PrintProfile(std::cout, "pert", g);
      }
      numEq = GNM(*A, g, answers, STEPS, FUZZ, LNMFREQ, LNMMAX, LAMBDAMIN, WOBBLE, THRESHOLD);
      for (i = 0; i < numEq; i++) {
	free(answers[i]);
      }
      free(answers);
    }
  }

  delete A;
}

int main(int argc, char *argv[])
{
  opterr = 0;
  bool quiet = false;

  int long_opt_index = 0;
  struct option long_options[] = {
    { "help", 0, NULL, 'h'   },
    { "version", 0, NULL, 'v'  },
    { "verbose", 0, NULL, 'V'  },
    { 0,    0,    0,    0   }
  };
  int c;
  while ((c = getopt_long(argc, argv, "d:n:s:qvVhS", long_options, &long_opt_index)) != -1) {
    switch (c) {
    case 'v':
      PrintBanner(std::cerr); exit(1);
    case 'q':
      quiet = true;
      break;
    case 'V':
      g_verbose = true;
      break;
    case 'd':
      g_numDecimals = atoi(optarg);
      break;
    case 'n':
      g_numVectors = atoi(optarg);
      break;
    case 's':
      g_startFile = optarg;
      break;
    case 'S':
      break;
    case 'h':
      PrintHelp(argv[0]);
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

  try {
    Gambit::Game game = Gambit::ReadGame(*input_stream);
    Solve(game);
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
