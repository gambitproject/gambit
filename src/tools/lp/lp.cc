//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/tools/lp/lp.cc
// Implementation of algorithm to compute mixed strategy equilibria
// of constant sum normal form games via linear programming
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
#include <memory>
#include <getopt.h>
#include "gambit.h"
#include "tools/util.h"
#include "solvers/lp/lp.h"

using namespace Gambit;
using namespace Gambit::Nash;

void PrintBanner(std::ostream &p_stream)
{
  p_stream << "Compute Nash equilibria by solving a linear program\n";
  p_stream << "Gambit version " VERSION ", Copyright (C) 1994-2025, The Gambit Project\n";
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
  std::cerr << "  -h, --help       print this help message\n";
  std::cerr << "  -q               quiet mode (suppresses banner)\n";
  std::cerr << "  -v, --version    print version information\n";
  exit(1);
}

int main(int argc, char *argv[])
{
  int c;
  int numDecimals = 6;
  bool useFloat = false, useStrategic = false, quiet = false, printDetail = false;
  bool bySubgames = false;

  int long_opt_index = 0;
  struct option long_options[] = {
      {"help", 0, nullptr, 'h'}, {"version", 0, nullptr, 'v'}, {nullptr, 0, nullptr, 0}};
  while ((c = getopt_long(argc, argv, "d:DvqhSP", long_options, &long_opt_index)) != -1) {
    switch (c) {
    case 'v':
      PrintBanner(std::cerr);
      exit(1);
    case 'd':
      useFloat = true;
      numDecimals = atoi(optarg);
      break;
    case 'D':
      printDetail = true;
      break;
    case 'h':
      PrintHelp(argv[0]);
      break;
    case 'q':
      quiet = true;
      break;
    case 'S':
      useStrategic = true;
      break;
    case 'P':
      bySubgames = true;
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

  std::istream *input_stream = &std::cin;
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
    const Gambit::Game game = Gambit::ReadGame(*input_stream);
    if (!game->IsTree() || useStrategic) {
      if (useFloat) {
        std::shared_ptr<StrategyProfileRenderer<double>> renderer;
        if (printDetail) {
          renderer = std::make_shared<MixedStrategyDetailRenderer<double>>(std::cout, numDecimals);
        }
        else {
          renderer = std::make_shared<MixedStrategyCSVRenderer<double>>(std::cout, numDecimals);
        }
        LpStrategySolve<double>(game,
                                [&](const MixedStrategyProfile<double> &p,
                                    const std::string &label) { renderer->Render(p, label); });
      }
      else {
        std::shared_ptr<StrategyProfileRenderer<Rational>> renderer;
        if (printDetail) {
          renderer = std::make_shared<MixedStrategyDetailRenderer<Rational>>(std::cout);
        }
        else {
          renderer = std::make_shared<MixedStrategyCSVRenderer<Rational>>(std::cout);
        }
        LpStrategySolve<Rational>(game,
                                  [&](const MixedStrategyProfile<Rational> &p,
                                      const std::string &label) { renderer->Render(p, label); });
      }
    }
    else {
      if (!bySubgames) {
        if (useFloat) {
          std::shared_ptr<StrategyProfileRenderer<double>> renderer;
          if (printDetail) {
            renderer =
                std::make_shared<BehavStrategyDetailRenderer<double>>(std::cout, numDecimals);
          }
          else {
            renderer = std::make_shared<BehavStrategyCSVRenderer<double>>(std::cout, numDecimals);
          }
          LpBehaviorSolve<double>(game,
                                  [&](const MixedBehaviorProfile<double> &p,
                                      const std::string &label) { renderer->Render(p, label); });
        }
        else {
          std::shared_ptr<StrategyProfileRenderer<Rational>> renderer;
          if (printDetail) {
            renderer = std::make_shared<BehavStrategyDetailRenderer<Rational>>(std::cout);
          }
          else {
            renderer = std::make_shared<BehavStrategyCSVRenderer<Rational>>(std::cout);
          }
          LpBehaviorSolve<Rational>(game,
                                    [&](const MixedBehaviorProfile<Rational> &p,
                                        const std::string &label) { renderer->Render(p, label); });
        }
      }
      else {
        if (useFloat) {
          std::shared_ptr<StrategyProfileRenderer<double>> renderer;
          if (printDetail) {
            renderer =
                std::make_shared<BehavStrategyDetailRenderer<double>>(std::cout, numDecimals);
          }
          else {
            renderer = std::make_shared<BehavStrategyCSVRenderer<double>>(std::cout, numDecimals);
          }
          const BehaviorSolverType<double> func = [&](const Game &g) {
            return LpBehaviorSolve<double>(
                g, [&](const MixedBehaviorProfile<double> &p, const std::string &label) {
                  renderer->Render(p, label);
                });
          };
          SolveBySubgames<double>(game, func,
                                  [&](const MixedBehaviorProfile<double> &p,
                                      const std::string &label) { renderer->Render(p, label); });
        }
        else {
          std::shared_ptr<StrategyProfileRenderer<Rational>> renderer;
          if (printDetail) {
            renderer =
                std::make_shared<BehavStrategyDetailRenderer<Rational>>(std::cout, numDecimals);
          }
          else {
            renderer =
                std::make_shared<BehavStrategyCSVRenderer<Rational>>(std::cout, numDecimals);
          }
          const BehaviorSolverType<Rational> func = [&](const Game &g) {
            return LpBehaviorSolve<Rational>(
                g, [&](const MixedBehaviorProfile<Rational> &p, const std::string &label) {
                  renderer->Render(p, label);
                });
          };
          SolveBySubgames<Rational>(game, func,
                                    [&](const MixedBehaviorProfile<Rational> &p,
                                        const std::string &label) { renderer->Render(p, label); });
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
