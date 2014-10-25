//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/lcp/lcp.cc
// Compute Nash equilibria via linear complementarity program
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
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <getopt.h>
#include "libgambit/libgambit.h"
#include "efglcp.h"
#include "nfglcp.h"

using namespace Gambit;

void PrintBanner(std::ostream &p_stream)
{
  p_stream << "Compute Nash equilibria by solving a linear complementarity program\n";
  p_stream << "Gambit version " VERSION ", Copyright (C) 1994-2014, The Gambit Project\n";
  p_stream << "This is free software, distributed under the GNU GPL\n\n";
}

void PrintHelp(char *progname)
{
  PrintBanner(std::cerr);
  std::cerr << "Usage: " << progname << " [OPTIONS] [file]\n";
  std::cerr << "If file is not specified, attempts to read game from standard input.\n";
  std::cerr << "With no options, reports all Nash equilibria found.\n\n";

  std::cerr << "Options:\n";
  std::cerr << "  -d DECIMALS      compute using floating-point arithmetic;\n";
  std::cerr << "                   display results with DECIMALS digits\n";
  std::cerr << "  -S               use strategic game\n";
  std::cerr << "  -P               find only subgame-perfect equilibria\n";
  std::cerr << "  -e EQA           terminate after finding EQA equilibria\n";
  std::cerr << "                   (default is to find all accessible equilbria\n";
  std::cerr << "  -r DEPTH         terminate recursion at DEPTH\n";
  std::cerr << "                   (only if number of equilibria sought is not 1)\n";
  std::cerr << "  -D               print detailed information about equilibria\n";
  std::cerr << "  -h, --help       print this help message\n";
  std::cerr << "  -q               quiet mode (suppresses banner)\n";
  std::cerr << "  -v, --version    print version information\n";
  exit(1);
}


int main(int argc, char *argv[])
{
  int c;
  bool useFloat = false, useStrategic = false, bySubgames = false, quiet = false;
  bool printDetail = false;
  int numDecimals = 6, stopAfter = 0, maxDepth = 0;

  int long_opt_index = 0;
  struct option long_options[] = {
    { "help", 0, NULL, 'h'   },
    { "version", 0, NULL, 'v'  },
    { 0,    0,    0,    0   }
  };
  while ((c = getopt_long(argc, argv, "d:DvhqSPe:r:", long_options, &long_opt_index)) != -1) {
    switch (c) {
    case 'v':
      PrintBanner(std::cerr); exit(1);
    case 'd':
      useFloat = true;
      numDecimals = atoi(optarg);
      break;
    case 'D':
      printDetail = true;
      break;
    case 'e':
      stopAfter = atoi(optarg);
      break;
    case 'h':
      PrintHelp(argv[0]);
      break;
    case 'q':
      quiet = true;
      break;
    case 'r':
      maxDepth = atoi(optarg);
      break;
    case 'S':
      useStrategic = true;
      break;
    case 'P':
      bySubgames = true;
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
    if (!game->IsTree() || useStrategic) {
      if (useFloat) {
	shared_ptr<StrategyProfileRenderer<double> > renderer;
	if (printDetail)  {
	  renderer = new MixedStrategyDetailRenderer<double>(std::cout,
							     numDecimals);
	}
	else {
	  renderer = new MixedStrategyCSVRenderer<double>(std::cout, numDecimals);
	}
	NashLcpStrategySolver<double> algorithm(stopAfter, maxDepth,
						renderer);
	algorithm.Solve(game);
      }
      else {
	shared_ptr<StrategyProfileRenderer<Rational> > renderer;
	if (printDetail) {
	  renderer = new MixedStrategyDetailRenderer<Rational>(std::cout);
	}
	else {
	  renderer = new MixedStrategyCSVRenderer<Rational>(std::cout);
	}
	NashLcpStrategySolver<Rational> algorithm(stopAfter, maxDepth,
						  renderer);
	algorithm.Solve(game);
      }
    }
    else {
      if (!bySubgames) {
	if (useFloat) {
	  shared_ptr<StrategyProfileRenderer<double> > renderer;
	  if (printDetail)  {
	    renderer = new BehavStrategyDetailRenderer<double>(std::cout,
							       numDecimals);
	  }
	  else {
	    renderer = new BehavStrategyCSVRenderer<double>(std::cout, 
							    numDecimals);
	  }
	  NashLcpBehaviorSolver<double> algorithm(stopAfter, maxDepth, renderer);
	  algorithm.Solve(game);
	}
	else {
	  shared_ptr<StrategyProfileRenderer<Rational> > renderer;
	  if (printDetail) {
	    renderer = new BehavStrategyDetailRenderer<Rational>(std::cout);
	  }
	  else {
	    renderer = new BehavStrategyCSVRenderer<Rational>(std::cout);
	  }
	  NashLcpBehaviorSolver<Rational> algorithm(stopAfter, maxDepth, renderer);
	  algorithm.Solve(game);
	}
      }
      else {
	if (useFloat) {
	  shared_ptr<NashBehavSolver<double> > stage = 
	    new NashLcpBehaviorSolver<double>(stopAfter, maxDepth);
	  shared_ptr<StrategyProfileRenderer<double> > renderer;
	  if (printDetail)  {
	    renderer = new BehavStrategyDetailRenderer<double>(std::cout,
							       numDecimals);
	  }
	  else {
	    renderer = new BehavStrategyCSVRenderer<double>(std::cout, 
							    numDecimals);
	  }
	  SubgameNashBehavSolver<double> algorithm(stage, renderer);
	  algorithm.Solve(game);
	}
	else {
	  shared_ptr<NashBehavSolver<Rational> > stage = 
	    new NashLcpBehaviorSolver<Rational>(stopAfter, maxDepth);
	  shared_ptr<StrategyProfileRenderer<Rational> > renderer;
	  if (printDetail)  {
	    renderer = new BehavStrategyDetailRenderer<Rational>(std::cout,
								 numDecimals);
	  }
	  else {
	    renderer = new BehavStrategyCSVRenderer<Rational>(std::cout, 
							      numDecimals);
	  }
	  SubgameNashBehavSolver<Rational> algorithm(stage, renderer);
	  algorithm.Solve(game);
	}
      }
    }
    return 0;
  }
  catch (std::runtime_error &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}


