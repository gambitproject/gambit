//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/tools/gt/nfgipa.cc
// Gambit frontend to Gametracer IPA
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
#include <iostream>
#include <fstream>
#include <type_traits>
#include "gambit.h"
#include "tools/util.h"
#include "solvers/ipa/ipa.h"

using namespace Gambit;
using namespace Gambit::Nash;

namespace {

void RenderIPAEvent(const IPAEvent &p_event,
                    const std::shared_ptr<MixedProfileRenderer<double>> &p_renderer)
{
  std::visit(
      [p_renderer]<typename Event>(const Event &event) {
        if constexpr (std::is_same_v<Event, IPAStepEvent>) {
          p_renderer->Render(event.profile, "iter-" + lexical_cast<std::string>(event.iteration));
        }
      },
      p_event);
}

} // namespace

extern std::vector<MixedStrategyProfile<double>> ReadStrategyPerturbations(const Game &p_game,
                                                                           std::istream &p_stream);

void PrintBanner(std::ostream &p_stream)
{
  p_stream << "Compute Nash equilibria using iterated polymatrix approximation\n";
  p_stream << "Gametracer version 0.2, Copyright (C) 2002, Ben Blum and Christian Shelton\n";
  p_stream << "Gambit version " VERSION ", Copyright (C) 1994-2026, The Gambit Project\n";
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
  std::cerr << "  -n COUNT         number of perturbation vectors to generate randomly\n";
  std::cerr << "                   (mutually exclusive with -s)\n";
  std::cerr << "  -R SEED          seed the random number generator used to generate\n";
  std::cerr << "                   perturbation vectors (default is to seed from system\n";
  std::cerr << "                   entropy); requires -n\n";
  std::cerr << "  -s FILE          file containing perturbation vectors\n";
  std::cerr << "                   (mutually exclusive with -n)\n";
  std::cerr << "  -q               quiet mode (suppresses banner)\n";
  std::cerr << "  -V, --verbose    verbose mode (shows intermediate output)\n";
  std::cerr << "  -v, --version    print version information\n";
  exit(0);
}

int main(int argc, char *argv[])
{
  opterr = 0;
  bool quiet = false, verbose = false, numVectorsSet = false;
  int numDecimals = 6, numVectors = 1;
  std::string startFile;
  std::optional<unsigned long> seed;

  int long_opt_index = 0;
  option long_options[] = {{"help", 0, nullptr, 'h'},
                           {"version", 0, nullptr, 'v'},
                           {"verbose", 0, nullptr, 'V'},
                           {nullptr, 0, nullptr, 0}};
  int c;
  while ((c = getopt_long(argc, argv, "d:n:s:R:vVqh", long_options, &long_opt_index)) != -1) {
    switch (c) {
    case 'v':
      PrintBanner(std::cerr);
      exit(0);
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
      numVectorsSet = true;
      break;
    case 'R':
      seed = std::strtoul(optarg, nullptr, 10);
      break;
    case 's':
      startFile = optarg;
      break;
    case 'h':
      PrintHelp(argv[0]);
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

  if (numVectorsSet && !startFile.empty()) {
    std::cerr << "Error: The -n and -s options are mutually exclusive.\n";
    return 1;
  }
  if (seed && !numVectorsSet) {
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
    auto renderer = MakeMixedStrategyProfileRenderer<double>(std::cout, numDecimals, false);

    std::vector<MixedStrategyProfile<double>> perts;
    if (!startFile.empty()) {
      std::ifstream startPerts(startFile.c_str());
      perts = ReadStrategyPerturbations(game, startPerts);
    }
    else {
      // Generate the desired number of points randomly
      auto engine = MakeRandomEngine(seed);
      perts = NewRandomStrategyProfiles(game, numVectors, engine);
    }

    for (auto pert : perts) {
      IPAStrategySolve(
          pert,
          [renderer](const MixedStrategyProfile<double> &p_profile) {
            renderer->Render(p_profile);
          },
          [renderer, verbose](const IPAEvent &p_event) {
            if (verbose) {
              RenderIPAEvent(p_event, renderer);
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
