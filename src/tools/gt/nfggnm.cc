//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
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

#include <getopt.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include "gambit.h"
#include "tools/util.h"
#include "solvers/gnm/gnm.h"

using namespace Gambit;
using namespace Gambit::Nash;

extern List<MixedStrategyProfile<double>> ReadStrategyPerturbations(const Game &p_game,
                                                                    std::istream &p_stream);
extern List<MixedStrategyProfile<double>> RandomStrategyPerturbations(const Game &p_game,
                                                                      int p_count);

void PrintBanner(std::ostream &p_stream)
{
  p_stream << "Compute Nash equilibria using a global Newton method\n";
  p_stream << "Gametracer version 0.2, Copyright (C) 2002, Ben Blum and Christian Shelton\n";
  p_stream << "Gambit version " VERSION ", Copyright (C) 1994-2025, The Gambit Project\n";
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
  std::cerr << "  -m LAMBDA        lambda to end tracing (must be negative, default "
            << std::to_string(GNM_LAMBDA_END_DEFAULT) << ")\n";
  std::cerr << "  -f FREQ          frequency to run local Newton method (default "
            << std::to_string(GNM_LOCAL_NEWTON_INTERVAL_DEFAULT) << ")\n";
  std::cerr << "  -i MAXITS        maximum iterations in local Newton (default "
            << std::to_string(GNM_LOCAL_NEWTON_MAXITS_DEFAULT) << ")\n";
  std::cerr << "  -c STEPS         number of steps in each support cell (default "
            << std::to_string(GNM_STEPS_DEFAULT) << ")\n";
  std::cerr << "  -q               quiet mode (suppresses banner)\n";
  std::cerr << "  -V, --verbose    verbose mode (shows intermediate output)\n";
  std::cerr << "  -v, --version    print version information\n";
  exit(1);
}

int main(int argc, char *argv[])
{
  opterr = 0;
  bool quiet = false, verbose = false;
  int numDecimals = 6, numVectors = 1;
  double lambdaEnd = GNM_LAMBDA_END_DEFAULT;
  int localNewtonInterval = GNM_LOCAL_NEWTON_INTERVAL_DEFAULT;
  int localNewtonMaxits = GNM_LOCAL_NEWTON_MAXITS_DEFAULT;
  int steps = GNM_STEPS_DEFAULT;
  std::string startFile;

  int long_opt_index = 0;
  struct option long_options[] = {{"help", 0, nullptr, 'h'},
                                  {"version", 0, nullptr, 'v'},
                                  {"verbose", 0, nullptr, 'V'},
                                  {nullptr, 0, nullptr, 0}};
  int c;
  while ((c = getopt_long(argc, argv, "d:n:s:m:f:i:c:qvVhS", long_options, &long_opt_index)) !=
         -1) {
    switch (c) {
    case 'v':
      PrintBanner(std::cerr);
      exit(1);
    case 'q':
      quiet = true;
      break;
    case 'V':
      verbose = true;
      break;
    case 'd':
      numDecimals = atoi(optarg);
      break;
    case 'n':
      numVectors = atoi(optarg);
      break;
    case 's':
      startFile = optarg;
      break;
    case 'm':
      lambdaEnd = atof(optarg);
      break;
    case 'f':
      localNewtonInterval = atoi(optarg);
      break;
    case 'i':
      localNewtonMaxits = atoi(optarg);
      break;
    case 'c':
      steps = atoi(optarg);
      break;
    case 'S':
      break;
    case 'h':
      PrintHelp(argv[0]);
      break;
    case '?':
      if (isprint(optopt)) {
        std::cerr << argv[0] << ": Unknown option `-" << ((char)optopt) << "'.\n";
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

  if (lambdaEnd >= 0.0) {
    std::cerr << "Error: Value for -m (end lambda) must be negative\n";
    return 1;
  }
  if (localNewtonInterval <= 0) {
    std::cerr << "Error: Value for -f (local Newton frequency) must be at least 1\n";
    return 1;
  }
  if (localNewtonMaxits <= 0) {
    std::cerr << "Error: Value for -i (local Newton iterations) must be at least 1\n";
    return 1;
  }
  if (steps <= 0) {
    std::cerr << "Error: Value for -c (steps in support cell) must be at least 1\n";
    return 1;
  }

  std::istream *input_stream = &std::cin;
  std::ifstream file_stream;
  if (optind < argc) {
    file_stream.open(argv[optind]);
    if (!file_stream.is_open()) {
      std::ostringstream error_message;
      error_message << argv[0] << ": " << argv[optind];
      perror(error_message.str().c_str());
      return 1;
    }
    input_stream = &file_stream;
  }

  try {
    const Game game = ReadGame(*input_stream);
    const std::shared_ptr<StrategyProfileRenderer<double>> renderer(
        new MixedStrategyCSVRenderer<double>(std::cout, numDecimals));

    List<MixedStrategyProfile<double>> perts;
    if (!startFile.empty()) {
      std::ifstream startPerts(startFile.c_str());
      perts = ReadStrategyPerturbations(game, startPerts);
    }
    else {
      // Generate the desired number of points randomly
      perts = RandomStrategyPerturbations(game, numVectors);
    }
    for (auto pert : perts) {
      GNMStrategySolve(pert, lambdaEnd, steps, localNewtonInterval, localNewtonMaxits,
                       [renderer, verbose](const MixedStrategyProfile<double> &p_profile,
                                           const std::string &p_label) {
                         if (p_label == "NE" || verbose) {
                           renderer->Render(p_profile, p_label);
                         }
                       });
    }
    return 0;
  }
  catch (std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
