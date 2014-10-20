//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
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
#include <cerrno>
#include <cstdlib>
#include <unistd.h>
#include <getopt.h>
#include "libgambit/libgambit.h"
#include "efgliap.h"
#include "nfgliap.h"

using namespace Gambit;

void PrintBanner(std::ostream &p_stream)
{
  p_stream << "Compute Nash equilibria by minimizing the Lyapunov function\n";
  p_stream << "Gambit version " VERSION ", Copyright (C) 1994-2014, The Gambit Project\n";
  p_stream << "This is free software, distributed under the GNU GPL\n\n";
}

void PrintHelp(char *progname)
{
  PrintBanner(std::cerr);
  std::cerr << "Usage: " << progname << " [OPTIONS] [file]\n";
  std::cerr << "If file is not specified, attempts to read game from standard input.\n";
  std::cerr << "With no options, attempts to compute one equilibrium starting at centroid.\n";

  std::cerr << "Options:\n";
  std::cerr << "  -d DECIMALS      print probabilities with DECIMALS digits\n";
  std::cerr << "  -h, --help       print this help message\n";
  std::cerr << "  -n COUNT         number of starting points to generate\n";
  std::cerr << "  -s FILE          file containing starting points\n";
  std::cerr << "  -q               quiet mode (suppresses banner)\n";
  std::cerr << "  -V, --verbose    verbose mode (shows intermediate output)\n";
  std::cerr << "                   (default is to only show equilibria)\n";
  std::cerr << "  -v, --version    print version information\n";
  exit(1);
}

List<MixedStrategyProfile<double> > 
ReadStrategyProfiles(const Game &p_game, std::istream &p_stream)
{
  List<MixedStrategyProfile<double> > profiles;
  while (!p_stream.eof() && !p_stream.bad()) {
    MixedStrategyProfile<double> p(p_game->NewMixedStrategyProfile(0.0));
    for (int i = 1; i <= p.MixedProfileLength(); i++) {
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

List<MixedStrategyProfile<double> > 
RandomStrategyProfiles(const Game &p_game, int p_count)
{
  List<MixedStrategyProfile<double> > profiles;
  for (int i = 1; i <= p_count; i++) {
    MixedStrategyProfile<double> p(p_game->NewMixedStrategyProfile(0.0));
    p.Randomize();
    profiles.push_back(p);
  }
  return profiles;
}

List<MixedBehaviorProfile<double> > 
ReadBehaviorProfiles(const Game &p_game, std::istream &p_stream)
{
  List<MixedBehaviorProfile<double> > profiles;
  while (!p_stream.eof() && !p_stream.bad()) {
    MixedBehaviorProfile<double> p(p_game);
    for (int i = 1; i <= p.Length(); i++) {
      if (p_stream.eof() || p_stream.bad()) {
	break;
      }
      p_stream >> p[i];
      if (i < p.Length()) {
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

List<MixedBehaviorProfile<double> > 
RandomBehaviorProfiles(const Game &p_game, int p_count)
{
  List<MixedBehaviorProfile<double> > profiles;
  for (int i = 1; i <= p_count; i++) {
    MixedBehaviorProfile<double> p(p_game);
    p.Randomize();
    profiles.push_back(p);
  }
  return profiles;
}

int main(int argc, char *argv[])
{
  opterr = 0;
  bool quiet = false, useStrategic = false, useRandom = false, verbose = false;
  int numTries = 10;
  int maxitsN = 100;
  int numDecimals = 6;
  double tolN = 1.0e-10;
  std::string startFile = "";
 
  int long_opt_index = 0;
  struct option long_options[] = {
    { "help", 0, NULL, 'h'   },
    { "version", 0, NULL, 'v'  },
    { "verbose", 0, NULL, 'V'  },
    { 0,    0,    0,    0   }
  };
  int c;
  while ((c = getopt_long(argc, argv, "d:n:s:hqVvS", long_options, &long_opt_index)) != -1) {
    switch (c) {
    case 'v':
      PrintBanner(std::cerr); exit(1);
    case 'd':
      numDecimals = atoi(optarg);
      break;
    case 'n':
      numTries = atoi(optarg);
      break;
    case 's':
      startFile = optarg;
      break;
    case 'h':
      PrintHelp(argv[0]);
      break;
    case 'S':
      useStrategic = true;
      break;
    case 'q':
      quiet = true;
      break;
    case 'V':
      verbose = true;
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
      List<MixedStrategyProfile<double> > starts;
      if (startFile != "") {
	std::ifstream startPoints(startFile.c_str());
	starts = ReadStrategyProfiles(game, startPoints);
      }
      else {
	// Generate the desired number of points randomly
	starts = RandomStrategyProfiles(game, numTries);
      }

      for (int i = 1; i <= starts.size(); i++) {
	shared_ptr<StrategyProfileRenderer<double> > renderer;
	renderer = new MixedStrategyCSVRenderer<double>(std::cout,
							numDecimals);
	NashLiapStrategySolver algorithm(maxitsN, verbose, renderer);
	algorithm.Solve(starts[i]);
      }
    }
    else {
      List<MixedBehaviorProfile<double> > starts;
      if (startFile != "") {
	std::ifstream startPoints(startFile.c_str());
	starts = ReadBehaviorProfiles(game, startPoints);
      }
      else {
	// Generate the desired number of points randomly
	starts = RandomBehaviorProfiles(game, numTries);
      }

      for (int i = 1; i <= starts.size(); i++) {
	shared_ptr<StrategyProfileRenderer<double> > renderer;
	renderer = new BehavStrategyCSVRenderer<double>(std::cout,
							numDecimals);
	NashLiapBehavSolver algorithm(maxitsN, verbose, renderer);
	algorithm.Solve(starts[i]);
      }
    }
    return 0;
  }
  catch (std::runtime_error &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
