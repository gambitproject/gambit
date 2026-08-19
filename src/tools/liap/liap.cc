//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/tools/liap/liap.cc
// Compute Nash equilibria by minimizing Liapunov function
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
#include <type_traits>
#include <getopt.h>
#include "gambit.h"
#include "tools/util.h"
#include "solvers/liap/liap.h"

using namespace Gambit;
using namespace Gambit::Nash;

template <class Renderer, class Profile>
void RenderLiapEvent(const Renderer &p_renderer, const LiapEvent<Profile> &p_event)
{
  std::visit(
      [&]<typename EventType>(const EventType &event) {
        if constexpr (std::is_same_v<EventType, LiapStartEvent<Profile>>) {
          p_renderer->Render(event.profile, "start");
        }
        else if constexpr (std::is_same_v<EventType, LiapEndEvent<Profile>>) {
          p_renderer->Render(event.profile, "end");
        }
      },
      p_event);
}

void PrintBanner(std::ostream &p_stream)
{
  p_stream << "Compute Nash equilibria by minimizing the Lyapunov function\n";
  p_stream << "Gambit version " VERSION ", Copyright (C) 1994-2026, The Gambit Project\n";
  p_stream << "This is free software, distributed under the GNU GPL\n\n";
}

void PrintHelp(char *progname)
{
  PrintBanner(std::cerr);
  std::cerr << "Usage: " << progname << " [OPTIONS] [file]\n";
  std::cerr << "If file is not specified, attempts to read game from standard input.\n";
  std::cerr << "With no options, attempts to compute one equilibrium starting at centroid.\n";

  std::cerr << "Options:\n";
  std::cerr << "  -A               compute agent form equilibria for extensive games\n";
  std::cerr << "  -d DECIMALS      print probabilities with DECIMALS digits\n";
  std::cerr << "  -h, --help       print this help message\n";
  std::cerr << "  -n COUNT         number of starting points to generate randomly\n";
  std::cerr << "                   (mutually exclusive with -s)\n";
  std::cerr << "  -R SEED          seed the random number generator used to generate\n";
  std::cerr << "                   starting points (default is to seed from system entropy);\n";
  std::cerr << "                   requires -n\n";
  std::cerr << "  -i MAXITER       maximum number of iterations per point (default is 1000)\n";
  std::cerr << "  -m MAXREGRET     maximum regret acceptable as a proportion of range of\n";
  std::cerr << "                   payoffs in the game\n";
  std::cerr << "  -s FILE          file containing starting points\n";
  std::cerr << "                   (mutually exclusive with -n)\n";
  std::cerr << "  -q               quiet mode (suppresses banner)\n";
  std::cerr << "  -V, --verbose    verbose mode (shows intermediate output)\n";
  std::cerr << "                   (default is to only show equilibria)\n";
  std::cerr << "  -v, --version    print version information\n";
  exit(0);
}

std::vector<MixedStrategyProfile<double>> ReadStrategyProfiles(const Game &p_game,
                                                               std::istream &p_stream)
{
  std::vector<MixedStrategyProfile<double>> profiles;
  while (!p_stream.eof() && !p_stream.bad()) {
    MixedStrategyProfile<double> p(p_game->NewMixedStrategyProfile(0.0));
    if (p_stream.peek() == EOF) {
      break;
    }
    p_stream >> p[1];
    for (size_t i = 2; i <= p.MixedProfileLength(); i++) {
      if (p_stream.eof() || p_stream.bad()) {
        break;
      }
      char comma;
      p_stream >> comma;
      p_stream >> p[i];
    }
    // Read in the rest of the line and discard
    std::string foo;
    std::getline(p_stream, foo);
    profiles.push_back(p);
  }
  return profiles;
}

std::vector<MixedBehaviorProfile<double>> ReadBehaviorProfiles(const Game &p_game,
                                                               std::istream &p_stream)
{
  std::vector<MixedBehaviorProfile<double>> profiles;
  while (!p_stream.eof() && !p_stream.bad()) {
    MixedBehaviorProfile<double> p(p_game);
    if (p_stream.peek() == EOF) {
      break;
    }
    p_stream >> p[1];
    for (size_t i = 2; i <= p.BehaviorProfileLength(); i++) {
      if (p_stream.eof() || p_stream.bad()) {
        break;
      }
      char comma;
      p_stream >> comma;
      p_stream >> p[i];
    }
    // Read in the rest of the line and discard
    std::string foo;
    std::getline(p_stream, foo);
    profiles.push_back(p);
  }
  return profiles;
}

int main(int argc, char *argv[])
{
  opterr = 0;
  bool quiet = false, solveAgent = false, verbose = false, numTriesSet = false;
  int numTries = 10;
  int maxitsN = 1000;
  int numDecimals = 6;
  double maxregret = 1.0e-4;
  std::string startFile;
  std::optional<unsigned long> seed;

  int long_opt_index = 0;
  option long_options[] = {{"help", 0, nullptr, 'h'},
                           {"version", 0, nullptr, 'v'},
                           {"verbose", 0, nullptr, 'V'},
                           {nullptr, 0, nullptr, 0}};
  int c;
  while ((c = getopt_long(argc, argv, "d:n:i:s:m:R:hqVvA", long_options, &long_opt_index)) != -1) {
    switch (c) {
    case 'v':
      PrintBanner(std::cerr);
      exit(0);
    case 'd':
      numDecimals = atoi(optarg);
      break;
    case 'n':
      numTries = atoi(optarg);
      numTriesSet = true;
      break;
    case 'R':
      seed = std::strtoul(optarg, nullptr, 10);
      break;
    case 'm':
      maxregret = atof(optarg);
      break;
    case 'i':
      maxitsN = atoi(optarg);
      break;
    case 's':
      startFile = optarg;
      break;
    case 'h':
      PrintHelp(argv[0]);
      break;
    case 'A':
      solveAgent = true;
      break;
    case 'q':
      quiet = true;
      break;
    case 'V':
      verbose = true;
      break;
    case '?':
      if (isprint(optopt)) {
        std::cerr << argv[0] << ": Unknown option `-" << static_cast<char>(optopt) << "'.\n";
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

  if (numTriesSet && !startFile.empty()) {
    std::cerr << "Error: The -n and -s options are mutually exclusive.\n";
    return 1;
  }
  if (seed && !numTriesSet) {
    std::cerr << "Error: The -R option requires -n.\n";
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
      exit(1);
    }
    input_stream = &file_stream;
  }

  try {
    const Game game = ReadGame(*input_stream);
    if (!game->IsTree() || !solveAgent) {
      std::vector<MixedStrategyProfile<double>> starts;
      if (!startFile.empty()) {
        std::ifstream startPoints(startFile.c_str());
        starts = ReadStrategyProfiles(game, startPoints);
      }
      else {
        // Generate the desired number of points randomly
        auto engine = MakeRandomEngine(seed);
        starts = NewRandomStrategyProfiles(game, numTries, engine);
      }

      for (const auto &start : starts) {
        auto renderer = MakeMixedStrategyProfileRenderer<double>(std::cout, numDecimals, false);
        LiapStrategySolve(
            start, maxregret, maxitsN,
            [renderer](const MixedStrategyProfile<double> &p_profile) {
              renderer->Render(p_profile);
            },
            [renderer, verbose](const LiapEvent<MixedStrategyProfile<double>> &event) {
              if (verbose) {
                RenderLiapEvent(renderer, event);
              }
            });
      }
    }
    else {
      std::vector<MixedBehaviorProfile<double>> starts;
      if (!startFile.empty()) {
        std::ifstream startPoints(startFile.c_str());
        starts = ReadBehaviorProfiles(game, startPoints);
      }
      else {
        // Generate the desired number of points randomly
        auto engine = MakeRandomEngine(seed);
        starts = NewRandomBehaviorProfiles(game, numTries, engine);
      }

      for (const auto &start : starts) {
        auto renderer = MakeMixedBehaviorProfileRenderer<double>(std::cout, numDecimals, false);
        LiapAgentSolve(
            start, maxregret, maxitsN,
            [renderer](const MixedBehaviorProfile<double> &p_profile) {
              renderer->Render(p_profile);
            },
            [renderer, verbose](const LiapEvent<MixedBehaviorProfile<double>> &event) {
              if (verbose) {
                RenderLiapEvent(renderer, event);
              }
            });
      }
    }
    return 0;
  }
  catch (std::runtime_error &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
