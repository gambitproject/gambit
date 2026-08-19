//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/tools/simpdiv/nfgsimpdiv.cc
// Compute Nash equilibria via simplicial subdivision on the normal form
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
#include "solvers/simpdiv/simpdiv.h"

using namespace Gambit;
using namespace Gambit::Nash;

template <class Renderer>
void RenderSimpdivEvent(const Renderer &p_renderer, const SimpdivEvent &p_event)
{
  std::visit(
      [&]<typename EventType>(const EventType &event) {
        if constexpr (std::is_same_v<EventType, SimpdivStartEvent>) {
          p_renderer->Render(event.profile, "start");
        }
        else if constexpr (std::is_same_v<EventType, SimpdivRefinementEvent>) {
          p_renderer->Render(event.profile, std::to_string(event.gridSize));
        }
      },
      p_event);
}

std::vector<MixedStrategyProfile<Rational>> ReadProfiles(const Game &p_game,
                                                         std::istream &p_stream)
{
  std::vector<MixedStrategyProfile<Rational>> profiles;
  while (!p_stream.eof() && !p_stream.bad()) {
    MixedStrategyProfile<Rational> p(p_game->NewMixedStrategyProfile(Rational(0)));
    p_stream >> p[1];
    if (!p_stream) {
      break;
    }
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

class MixedStrategyCSVAsFloatRenderer final : public MixedStrategyProfileRenderer<Rational> {
public:
  explicit MixedStrategyCSVAsFloatRenderer(std::ostream &p_stream, int p_numDecimals = 6)
    : m_stream(p_stream), m_numDecimals(p_numDecimals)
  {
  }
  ~MixedStrategyCSVAsFloatRenderer() override = default;
  void Render(const MixedStrategyProfile<Rational> &p_profile,
              const std::string &p_label = "NE") const override;

private:
  std::ostream &m_stream;
  int m_numDecimals;
};

void MixedStrategyCSVAsFloatRenderer::Render(const MixedStrategyProfile<Rational> &p_profile,
                                             const std::string &p_label) const
{
  m_stream << p_label;
  for (size_t i = 1; i <= p_profile.MixedProfileLength(); i++) {
    m_stream << "," << lexical_cast<std::string>(double(p_profile[i]), m_numDecimals);
  }
  m_stream << std::endl;
}

void PrintBanner(std::ostream &p_stream)
{
  p_stream << "Compute Nash equilibria using simplicial subdivision\n";
  p_stream << "Gambit version " VERSION ", Copyright (C) 1994-2026, The Gambit Project\n";
  p_stream << "This is free software, distributed under the GNU GPL\n\n";
}

void PrintHelp(char *progname)
{
  PrintBanner(std::cerr);
  std::cerr << "Usage: " << progname << " [OPTIONS] [file]\n";
  std::cerr << "If file is not specified, attempts to read game from standard input.\n";
  std::cerr << "With no options, computes one approximate Nash equilibrium.\n\n";

  std::cerr << "Options:\n";
  std::cerr << "  -g MULT          granularity of grid refinement at each step (default is 2)\n";
  std::cerr << "  -h, --help       print this help message\n";
  std::cerr << "  -r DENOM         generate random starting points with denominator DENOM\n";
  std::cerr << "                   (mutually exclusive with -s)\n";
  std::cerr << "  -n COUNT         number of starting points to generate (requires -r)\n";
  std::cerr << "  -R SEED          seed the random number generator used to generate\n";
  std::cerr << "                   starting points (default is to seed from system entropy);\n";
  std::cerr << "                   requires -n\n";
  std::cerr << "  -s FILE          file containing starting points\n";
  std::cerr << "                   (mutually exclusive with -r)\n";
  std::cerr << "  -d DECIMALS      show profiles as floating point with DECIMALS digits\n";
  std::cerr << "                   (default is to display rational numbers)\n";
  std::cerr << "  -m MAXREGRET     maximum regret acceptable as a proportion of range of\n";
  std::cerr << "                   payoffs in the game (default is 1e-7)\n";
  std::cerr << "  -q               quiet mode (suppresses banner)\n";
  std::cerr << "  -V, --verbose    verbose mode (shows intermediate output)\n";
  std::cerr << "                   (default is to only show equilibria)\n";
  std::cerr << "  -v, --version    print version information\n";
  exit(0);
}

int main(int argc, char *argv[])
{
  opterr = 0;
  std::string startFile;
  bool useRandom = false, numTriesSet = false;
  int randDenom = 1, gridResize = 2, stopAfter = 1, decimals = 0;
  bool verbose = false, quiet = false;
  Rational maxregret(1, 10000000);
  std::optional<unsigned long> seed;

  int long_opt_index = 0;
  struct option long_options[] = {{"help", 0, nullptr, 'h'},
                                  {"version", 0, nullptr, 'v'},
                                  {"verbose", 0, nullptr, 'V'},
                                  {nullptr, 0, nullptr, 0}};
  int c;
  while ((c = getopt_long(argc, argv, "g:hVvn:r:s:m:d:R:q", long_options, &long_opt_index)) !=
         -1) {
    switch (c) {
    case 'v':
      PrintBanner(std::cerr);
      exit(0);
    case 'g':
      gridResize = atoi(optarg);
      break;
    case 'h':
      PrintHelp(argv[0]);
      break;
    case 'r':
      useRandom = true;
      randDenom = atoi(optarg);
      break;
    case 'n':
      stopAfter = atoi(optarg);
      numTriesSet = true;
      break;
    case 'R':
      seed = std::strtoul(optarg, nullptr, 10);
      break;
    case 'm':
      maxregret = lexical_cast<Rational>(std::string(optarg));
      break;
    case 'd':
      decimals = atoi(optarg);
      break;
    case 's':
      startFile = optarg;
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

  if (useRandom && !startFile.empty()) {
    std::cerr << "Error: The -r and -s options are mutually exclusive.\n";
    return 1;
  }
  if (numTriesSet && !useRandom) {
    std::cerr << "Error: The -n option requires -r.\n";
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
    std::vector<MixedStrategyProfile<Rational>> starts;
    if (!startFile.empty()) {
      std::ifstream startPoints(startFile.c_str());
      starts = ReadProfiles(game, startPoints);
    }
    else if (useRandom) {
      auto engine = MakeRandomEngine(seed);
      starts = NewRandomStrategyProfiles(game, stopAfter, randDenom, engine);
    }
    else {
      starts.push_back(SimpdivDefaultStart(game));
    }
    for (auto start : starts) {
      if (decimals > 0) {
        auto renderer = std::make_shared<MixedStrategyCSVAsFloatRenderer>(std::cout, decimals);
        SimpdivStrategySolve(
            start, maxregret, gridResize, 0,
            [&](const MixedStrategyProfile<Rational> &p) { renderer->Render(p); },
            [&](const SimpdivEvent &event) {
              if (verbose) {
                RenderSimpdivEvent(renderer, event);
              }
            });
      }
      else {
        auto renderer = std::make_shared<MixedStrategyProfileCSVRenderer<Rational>>(std::cout);
        SimpdivStrategySolve(
            start, maxregret, gridResize, 0,
            [&](const MixedStrategyProfile<Rational> &p) { renderer->Render(p); },
            [&](const SimpdivEvent &event) {
              if (verbose) {
                RenderSimpdivEvent(renderer, event);
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
