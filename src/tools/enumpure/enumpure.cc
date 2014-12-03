//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpure/enumpure.cc
// Compute pure-strategy equilibria in extensive form games
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
#include <getopt.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <cerrno>
#include "enumpure.h"


void PrintBanner(std::ostream &p_stream)
{
  p_stream << "Search for Nash equilibria in pure strategies\n";
  p_stream << "Gambit version " VERSION ", Copyright (C) 1994-2014, The Gambit Project\n";
  p_stream << "This is free software, distributed under the GNU GPL\n\n";
}

void PrintHelp(char *progname)
{
  PrintBanner(std::cerr);
  std::cerr << "Usage: " << progname << " [OPTIONS] [file]\n";
  std::cerr << "If file is not specified, attempts to read game from standard input.\n";
  std::cerr << "With no options, locates all Nash equilibria in pure strategies.\n\n";

  std::cerr << "Options:\n";
  std::cerr << "  -S               report equilibria in strategies even for extensive games\n";
  std::cerr << "  -A               compute agent form equilibria\n";
  std::cerr << "  -P               find only subgame-perfect equilibria\n";
  std::cerr << "  -h, --help       print this help message\n";
  std::cerr << "  -q               quiet mode (suppresses banner)\n";
  std::cerr << "  -v, --version    print version information\n";
  exit(1);
}


int main(int argc, char *argv[])
{
  opterr = 0;
  bool quiet = false, reportStrategic = false, solveAgent = false, bySubgames = false;
  bool printDetail = false;
  
  int long_opt_index = 0;
  struct option long_options[] = {
    { "help", 0, NULL, 'h'   },
    { "version", 0, NULL, 'v'  },
    { 0,    0,    0,    0   }
  };
  int c;
  while ((c = getopt_long(argc, argv, "DvhqASP", long_options, &long_opt_index)) != -1) {
    switch (c) {
    case 'v':
      PrintBanner(std::cerr); exit(1);
    case 'D':
      printDetail = true;
      break;
    case 'S':
      reportStrategic = true;
      break;
    case 'A':
      solveAgent = true;
      break;
    case 'P':
      bySubgames = true;
      break;
    case 'h':
      PrintHelp(argv[0]);
      break;
    case 'q':
      quiet = true;
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
    Game game = ReadGame(*input_stream);
    shared_ptr<StrategyProfileRenderer<Rational> > renderer;
    if (reportStrategic || !game->IsTree()) {
      if (printDetail) {
	renderer = new MixedStrategyDetailRenderer<Rational>(std::cout);
      }
      else {
	renderer = new MixedStrategyCSVRenderer<Rational>(std::cout);
      }
    }
    else {
      if (printDetail) {
	renderer = new BehavStrategyDetailRenderer<Rational>(std::cout);
      }
      else {
	renderer = new BehavStrategyCSVRenderer<Rational>(std::cout);
      }
    }

    if (game->IsTree())  {
      if (bySubgames) {
	shared_ptr<NashBehavSolver<Rational> > stage;
        if (solveAgent) {
	  stage = new NashEnumPureAgentSolver();
	}
	else {
	  shared_ptr<NashStrategySolver<Rational> > substage = 
	    new NashEnumPureStrategySolver();
	  stage = new NashBehavViaStrategySolver<Rational>(substage);
	}
	SubgameNashBehavSolver<Rational> algorithm(stage, renderer);
	algorithm.Solve(game);
      }
      else {
	if (solveAgent) {
	  NashEnumPureAgentSolver algorithm(renderer);
	  algorithm.Solve(game);
	}
	else {
	  NashEnumPureStrategySolver algorithm(renderer);
	  algorithm.Solve(game);
	}
      }
    }
    else {
      NashEnumPureStrategySolver algorithm(renderer);
      algorithm.Solve(game);
    }
    return 0;
  }
  catch (std::runtime_error &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}





