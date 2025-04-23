//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
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
#include <cstdlib>
#include <iostream>
#include <fstream>
#include "gambit.h"
#include "tools/util.h"
#include "solvers/simpdiv/simpdiv.h"

using namespace Gambit;
using namespace Gambit::Nash;

List<MixedStrategyProfile<Rational>> ReadProfiles(const Game &p_game, std::istream &p_stream)
{
  List<MixedStrategyProfile<Rational>> profiles;
  while (!p_stream.eof() && !p_stream.bad()) {
    MixedStrategyProfile<Rational> p(p_game->NewMixedStrategyProfile(Rational(0)));
    for (size_t i = 1; i <= p.MixedProfileLength(); i++) {
      if (p_stream.eof() || p_stream.bad()) {
        break;
      }
      p_stream >> p[i];
      if (i < p.MixedProfileLength()) {
        char comma;
        p_stream >> comma;
      }
    }
    // Read in the rest of the line and discard
    std::string foo;
    std::getline(p_stream, foo);
    profiles.push_back(p);
  }
  return profiles;
}

List<MixedStrategyProfile<Rational>> RandomProfiles(const Game &p_game, int p_count,
                                                    const Rational &denom)
{
  std::default_random_engine engine;
  List<MixedStrategyProfile<Rational>> profiles;
  for (int i = 1; i <= p_count; i++) {
    std::cout << "profile " << i << std::endl;
    profiles.push_back(p_game->NewRandomStrategyProfile(denom, engine));
  }
  return profiles;
}

class MixedStrategyCSVAsFloatRenderer : public MixedStrategyRenderer<Rational> {
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
  p_stream << "Gambit version " VERSION ", Copyright (C) 1994-2025, The Gambit Project\n";
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
  std::cerr << "  -n COUNT         number of starting points to generate (requires -r)\n";
  std::cerr << "  -s FILE          file containing starting points\n";
  std::cerr << "  -d DECIMALS      show profiles as floating point with DECIMALS digits\n";
  std::cerr << "                   (default is to display rational numbers)\n";
  std::cerr << "  -m MAXREGRET     maximum regret acceptable as a proportion of range of\n";
  std::cerr << "                   payoffs in the game\n";
  std::cerr << "  -q               quiet mode (suppresses banner)\n";
  std::cerr << "  -V, --verbose    verbose mode (shows intermediate output)\n";
  std::cerr << "  -v, --version    print version information\n";
  std::cerr << "                   (default is to only show equilibria)\n";
  exit(1);
}

int main(int argc, char *argv[])
{
  opterr = 0;
  std::string startFile;
  bool useRandom = false;
  int randDenom = 1, gridResize = 2, stopAfter = 1, decimals = 0;
  bool verbose = false, quiet = false;
  Rational maxregret(1, 10000000);

  int long_opt_index = 0;
  struct option long_options[] = {{"help", 0, nullptr, 'h'},
                                  {"version", 0, nullptr, 'v'},
                                  {"verbose", 0, nullptr, 'V'},
                                  {nullptr, 0, nullptr, 0}};
  int c;
  while ((c = getopt_long(argc, argv, "g:hVvn:r:s:m:d:qS", long_options, &long_opt_index)) != -1) {
    switch (c) {
    case 'v':
      PrintBanner(std::cerr);
      exit(1);
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
    case 'S':
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
    List<MixedStrategyProfile<Rational>> starts;
    if (!startFile.empty()) {
      std::ifstream startPoints(startFile.c_str());
      starts = ReadProfiles(game, startPoints);
    }
    else if (useRandom) {
      starts = RandomProfiles(game, stopAfter, Rational(randDenom));
    }
    else {
      starts.push_back(game->NewMixedStrategyProfile(Rational(0)));
      starts.back() = Rational(0);
      for (const auto &player : game->GetPlayers()) {
        starts.back()[player->GetStrategies().back()] = Rational(1);
      }
    }
    for (auto start : starts) {
      if (decimals > 0) {
        auto renderer = std::make_shared<MixedStrategyCSVAsFloatRenderer>(std::cout, decimals);
        SimpdivStrategySolve(
            start, maxregret, gridResize, 0,
            [&](const MixedStrategyProfile<Rational> &p, const std::string &label) {
              if (label == "NE" || verbose) {
                renderer->Render(p, label);
              }
            });
      }
      else {
        auto renderer = std::make_shared<MixedStrategyCSVRenderer<Rational>>(std::cout);
        SimpdivStrategySolve(
            start, maxregret, gridResize, 0,
            [&](const MixedStrategyProfile<Rational> &p, const std::string &label) {
              if (label == "NE" || verbose) {
                renderer->Render(p, label);
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
