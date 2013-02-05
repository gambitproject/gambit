//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
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
#include "libgambit/subgame.h"

using namespace Gambit;

template <class T>
List<MixedBehavProfile<T> > SolveExtensive(const BehavSupport &p);
template <class T>
List<MixedBehavProfile<T> > SolveExtensiveSilent(const BehavSupport &p);
template <class T> void SolveStrategic(const Game &p_game);

void PrintBanner(std::ostream &p_stream)
{
  p_stream << "Compute Nash equilibria by solving a linear complementarity program\n";
  p_stream << "Gambit version " VERSION ", Copyright (C) 1994-2010, The Gambit Project\n";
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

int g_numDecimals = 6;
bool g_printDetail = false;
int g_stopAfter = 0;
int g_maxDepth = 0;

extern void PrintProfile(std::ostream &, const std::string &,
			 const MixedBehavProfile<double> &);
extern void PrintProfile(std::ostream &, const std::string &,
			 const MixedBehavProfile<Rational> &);


int main(int argc, char *argv[])
{
  int c;
  bool useFloat = false, useStrategic = false, bySubgames = false, quiet = false;

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
      g_numDecimals = atoi(optarg);
      break;
    case 'D':
      g_printDetail = true;
      break;
    case 'e':
      g_stopAfter = atoi(optarg);
      break;
    case 'h':
      PrintHelp(argv[0]);
      break;
    case 'q':
      quiet = true;
      break;
    case 'r':
      g_maxDepth = atoi(optarg);
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

    if (game->NumPlayers() != 2) {
      std::cerr << "Error: Game does not have two players.\n";
      return 1;
    }

    if (!game->IsTree() || useStrategic) {
      if (useFloat) {
	SolveStrategic<double>(game);
      }
      else {
	SolveStrategic<Rational>(game);
      }
    }
    else {
      if (!bySubgames) {
	if (useFloat) {
	  SolveExtensive<double>(game);
	}
	else {
	  SolveExtensive<Rational>(game);
	}
      }
      else {
	if (useFloat) {
	  List<MixedBehavProfile<double> > solutions;
	  solutions = SolveBySubgames<double>(BehavSupport(game),
					      &SolveExtensiveSilent<double>);
	  for (int i = 1; i <= solutions.Length(); i++) {
	    PrintProfile(std::cout, "NE", solutions[i]);
	  }
	}
	else {
	  List<MixedBehavProfile<Rational> > solutions;
	  solutions = SolveBySubgames<Rational>(BehavSupport(game),
						&SolveExtensiveSilent<Rational>);
	  for (int i = 1; i <= solutions.Length(); i++) {
	    PrintProfile(std::cout, "NE", solutions[i]);
	  }
	}
      }
    }
    return 0;
  }
  catch (InvalidFileException) {
    std::cerr << "Error: Game not in a recognized format.\n";
    return 1;
  }
  catch (...) {
    std::cerr << "Error: An internal error occurred.\n";
    return 1;
  }
}


