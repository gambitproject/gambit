//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/enumpoly.cc
// Enumerates all Nash equilibria via support enumeration
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
#include <unistd.h>
#include <stdlib.h>
#include "libgambit/libgambit.h"
#include "nfghs.h"

int g_numDecimals = 6;
bool g_verbose = false;

void PrintBanner(std::ostream &p_stream)
{
  p_stream << "Compute Nash equilibria by solving polynomial systems\n";
  p_stream << "Gambit version " VERSION ", Copyright (C) 1994-2010, The Gambit Project\n";
  p_stream << "Heuristic search implementation Copyright (C) 2006, Litao Wei\n";
  p_stream << "This is free software, distributed under the GNU GPL\n\n";
}

void PrintHelp(char *progname)
{
  PrintBanner(std::cerr);
  std::cerr << "Usage: " << progname << " [OPTIONS]\n";
  std::cerr << "Accepts game on standard input.\n";
  std::cerr << "With no options, reports all Nash equilibria found.\n\n";

  std::cerr << "Options:\n";
  std::cerr << "  -d DECIMALS      show equilibrium probabilities with DECIMALS digits\n";
  std::cerr << "  -h               print this help message\n";
  std::cerr << "  -S               use strategic game\n";
  std::cerr << "  -H               use heuristic search method to optimize time\n";
  std::cerr << "                   to find first equilibrium (strategic games only)\n";
  std::cerr << "  -q               quiet mode (suppresses banner)\n";
  std::cerr << "  -v               verbose mode (shows supports investigated)\n";
  std::cerr << "                   (default is only to show equilibria)\n";
  exit(1);
}

extern void SolveStrategic(const Gambit::Game &);
extern void SolveExtensive(const Gambit::Game &);

int main(int argc, char *argv[])
{
  opterr = 0;

  bool quiet = false;
  bool useHeuristic = false, useStrategic = false;

  int c;
  while ((c = getopt(argc, argv, "d:hHSqv")) != -1) {
    switch (c) {
    case 'd':
      g_numDecimals = atoi(optarg);
      break;
    case 'h':
      PrintHelp(argv[0]);
      break;
    case 'H':
      useHeuristic = true;
      break;
    case 'S':
      useStrategic = true;
      break;
    case 'q':
      quiet = true;
      break;
    case 'v':
      g_verbose = true;
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

    if (!game->IsTree() || useStrategic) {
      game->BuildComputedValues();
    
      if (useHeuristic) {
	gbtNfgHs algorithm(0);
	algorithm.Solve(game);
      }
      else {
	SolveStrategic(game);
      }
    }
    else {
      SolveExtensive(game);
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

