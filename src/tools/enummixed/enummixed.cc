//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/tools/enummixed/enummixed.cc
// Compute Nash equilibria via Mangasarian's algorithm
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
#include <iostream>
#include <fstream>

#include "gambit.h"
#include "tools/util.h"
#include "solvers/enummixed/enummixed.h"

using namespace Gambit;
using namespace Gambit::Nash;

template <class T>
void PrintCliques(const List<List<MixedStrategyProfile<T>>> &p_cliques,
                  std::shared_ptr<StrategyProfileRenderer<T>> p_renderer)
{
  for (size_t cl = 1; cl <= p_cliques.size(); cl++) {
    for (size_t i = 1; i <= p_cliques[cl].size(); i++) {
      p_renderer->Render(p_cliques[cl][i], "convex-" + lexical_cast<std::string>(cl));
    }
  }
}

void PrintBanner(std::ostream &p_stream)
{
  p_stream << "Compute Nash equilibria by enumerating extreme points\n";
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
  std::cerr << "  -c               output connectedness information\n";
  std::cerr << "  -h, --help       print this help message\n";
  std::cerr << "  -q               quiet mode (suppresses banner)\n";
  std::cerr << "  -v, --version    print version information\n";
  exit(1);
}

int main(int argc, char *argv[])
{
  int c;
  bool useFloat = false, quiet = false;
  bool showConnect = false;
  int numDecimals = 6;

  int long_opt_index = 0;
  struct option long_options[] = {
      {"help", 0, nullptr, 'h'}, {"version", 0, nullptr, 'v'}, {nullptr, 0, nullptr, 0}};
  while ((c = getopt_long(argc, argv, "d:vhqcS", long_options, &long_opt_index)) != -1) {
    switch (c) {
    case 'v':
      PrintBanner(std::cerr);
      exit(1);
    case 'd':
      useFloat = true;
      numDecimals = atoi(optarg);
      break;
    case 'h':
      PrintHelp(argv[0]);
      break;
    case 'c':
      showConnect = true;
      break;
    case 'S':
      break;
    case 'q':
      quiet = true;
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
    const Game game = ReadGame(*input_stream);
    if (useFloat) {
      std::shared_ptr<StrategyProfileRenderer<double>> renderer =
          std::make_shared<MixedStrategyCSVRenderer<double>>(std::cout, numDecimals);
      auto solution = EnumMixedStrategySolveDetailed<double>(
          game, [&](const MixedStrategyProfile<double> &p, const std::string &label) {
            renderer->Render(p, label);
          });
      if (showConnect) {
        PrintCliques(solution->GetCliques(), renderer);
      }
    }
    else {
      std::shared_ptr<StrategyProfileRenderer<Rational>> renderer(
          new MixedStrategyCSVRenderer<Rational>(std::cout));
      auto solution = EnumMixedStrategySolveDetailed<Rational>(
          game, [&](const MixedStrategyProfile<Rational> &p, const std::string &label) {
            renderer->Render(p, label);
          });
      if (showConnect) {
        PrintCliques(solution->GetCliques(), renderer);
      }
    }
    return 0;
  }
  catch (std::runtime_error &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
