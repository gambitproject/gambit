//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
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
#include <fstream>
#include <cstdlib>
#include <getopt.h>
#include "gambit.h"
#include "solvers/enumpoly/enumpoly.h"

using namespace Gambit;
using namespace Gambit::Nash;

int g_numDecimals = 6;
bool g_verbose = false;

void PrintBanner(std::ostream &p_stream)
{
  p_stream << "Compute Nash equilibria by solving polynomial systems\n";
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
  std::cerr << "  -d DECIMALS      show equilibrium probabilities with DECIMALS digits\n";
  std::cerr << "  -h, --help       print this help message\n";
  std::cerr << "  -S               use strategic game\n";
  std::cerr << "  -m MAXREGRET     maximum regret acceptable as a proportion of range of\n";
  std::cerr << "                   payoffs in the game\n";
  std::cerr << "  -e EQA           terminate after finding EQA equilibria\n";
  std::cerr << "                   (default is to search in all supports)\n";
  std::cerr << "  -q               quiet mode (suppresses banner)\n";
  std::cerr << "  -V, --verbose    verbose mode (shows supports investigated)\n";
  std::cerr << "  -v, --version    print version information\n";
  std::cerr << "                   (default is only to show equilibria)\n";
  exit(1);
}

void PrintProfile(std::ostream &p_stream, const std::string &p_label,
                  const MixedStrategyProfile<double> &p_profile)
{
  p_stream << p_label;
  for (size_t i = 1; i <= p_profile.MixedProfileLength(); i++) {
    p_stream.setf(std::ios::fixed);
    p_stream << ',' << std::setprecision(g_numDecimals) << p_profile[i];
  }
  p_stream << std::endl;
}

void PrintSupport(std::ostream &p_stream, const std::string &p_label,
                  const StrategySupportProfile &p_support)
{
  if (!g_verbose) {
    return;
  }
  p_stream << p_label;
  for (auto player : p_support.GetGame()->GetPlayers()) {
    p_stream << ",";
    for (auto strategy : player->GetStrategies()) {
      p_stream << ((p_support.Contains(strategy)) ? 1 : 0);
    }
  }
  p_stream << std::endl;
}

void PrintProfile(std::ostream &p_stream, const std::string &p_label,
                  const MixedBehaviorProfile<double> &p_profile)
{
  p_stream << p_label;
  for (size_t i = 1; i <= p_profile.BehaviorProfileLength(); i++) {
    p_stream.setf(std::ios::fixed);
    p_stream << "," << std::setprecision(g_numDecimals) << p_profile[i];
  }
  p_stream << std::endl;
}

void PrintSupport(std::ostream &p_stream, const std::string &p_label,
                  const BehaviorSupportProfile &p_support)
{
  if (!g_verbose) {
    return;
  }
  p_stream << p_label;
  for (auto player : p_support.GetGame()->GetPlayers()) {
    for (auto infoset : player->GetInfosets()) {
      p_stream << ",";
      for (auto action : infoset->GetActions()) {
        p_stream << ((p_support.Contains(action)) ? 1 : 0);
      }
    }
  }
  p_stream << std::endl;
}

int main(int argc, char *argv[])
{
  opterr = 0;

  bool quiet = false;
  bool useStrategic = false;
  double maxregret = 1.0e-4;
  int stopAfter = 0;

  int long_opt_index = 0;
  struct option long_options[] = {{"help", 0, nullptr, 'h'},
                                  {"version", 0, nullptr, 'v'},
                                  {"verbose", 0, nullptr, 'V'},
                                  {nullptr, 0, nullptr, 0}};
  int c;
  while ((c = getopt_long(argc, argv, "d:hSm:e:qvV", long_options, &long_opt_index)) != -1) {
    switch (c) {
    case 'v':
      PrintBanner(std::cerr);
      exit(1);
    case 'd':
      g_numDecimals = atoi(optarg);
      break;
    case 'h':
      PrintHelp(argv[0]);
      break;
    case 'S':
      useStrategic = true;
      break;
    case 'm':
      maxregret = atof(optarg);
      break;
    case 'e':
      stopAfter = atoi(optarg);
      break;
    case 'q':
      quiet = true;
      break;
    case 'V':
      g_verbose = true;
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
    if (!game->IsPerfectRecall()) {
      throw Gambit::UndefinedException(
          "Computing equilibria of games with imperfect recall is not supported.");
    }

    if (!game->IsTree() || useStrategic) {
      EnumPolyStrategySolve(
          game, stopAfter, maxregret,
          [](const MixedStrategyProfile<double> &eqm) { PrintProfile(std::cout, "NE", eqm); },
          [](const std::string &label, const StrategySupportProfile &support) {
            PrintSupport(std::cout, label, support);
          });
    }
    else {
      EnumPolyBehaviorSolve(
          game, stopAfter, maxregret,
          [](const MixedBehaviorProfile<double> &eqm) { PrintProfile(std::cout, "NE", eqm); },
          [](const std::string &label, const BehaviorSupportProfile &support) {
            PrintSupport(std::cout, label, support);
          });
    }
    return 0;
  }
  catch (std::runtime_error &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
