//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/convert/nfg2tex.cc
// Convert a normal form game to LaTeX (using Osborne's sgame style)
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
#include <unistd.h>
#include <getopt.h>
#include <iostream>
#include <fstream>
#include <cerrno>
#include <iomanip>

#include "libgambit/libgambit.h"

void WriteOsborneFile(std::ostream &p_file, const Gambit::Game &p_nfg,
		      int p_rowPlayer, int p_colPlayer)
{
  std::string theHtml;

  for (Gambit::StrategyIterator iter(p_nfg, p_rowPlayer, 1, p_colPlayer, 1);
       !iter.AtEnd(); iter++) {
    theHtml += "\\begin{game}{";
    theHtml += Gambit::lexical_cast<std::string>(p_nfg->GetPlayer(p_rowPlayer)->NumStrategies());
    theHtml += "}{";
    theHtml += Gambit::lexical_cast<std::string>(p_nfg->GetPlayer(p_colPlayer)->NumStrategies());
    theHtml += "}[";
    theHtml += p_nfg->GetPlayer(p_rowPlayer)->GetLabel();
    theHtml += "][";
    theHtml += p_nfg->GetPlayer(p_colPlayer)->GetLabel();
    theHtml += "]";

    if (p_nfg->NumPlayers() > 2) {
      theHtml += "[";
      for (int pl = 1; pl <= p_nfg->NumPlayers(); pl++) {
	if (pl == p_rowPlayer || pl == p_colPlayer) {
	  continue;
	}

	theHtml += "Player ";
	theHtml += Gambit::lexical_cast<std::string>(pl);
	theHtml += " Strategy ";
	theHtml += Gambit::lexical_cast<std::string>((*iter)->GetStrategy(pl)->GetNumber());
	theHtml += " ";
      }
      theHtml += "]";
    }

    theHtml += "\n&";

    for (int st = 1; st <= p_nfg->GetPlayer(p_colPlayer)->NumStrategies(); st++) {
      theHtml += p_nfg->GetPlayer(p_colPlayer)->GetStrategy(st)->GetLabel();
      if (st < p_nfg->GetPlayer(p_colPlayer)->NumStrategies()) {
	theHtml += " & ";
      }
    } 
    theHtml += "\\\\\n";

    for (int st1 = 1; st1 <= p_nfg->GetPlayer(p_rowPlayer)->NumStrategies(); st1++) {
      Gambit::PureStrategyProfile profile = (*iter)->Copy();
      profile->SetStrategy(p_nfg->GetPlayer(p_rowPlayer)->GetStrategy(st1));
      theHtml += p_nfg->GetPlayer(p_rowPlayer)->GetStrategy(st1)->GetLabel();
      theHtml += " & ";
      for (int st2 = 1; st2 <= p_nfg->GetPlayer(p_colPlayer)->NumStrategies(); st2++) {
	profile->SetStrategy(p_nfg->GetPlayer(p_colPlayer)->GetStrategy(st2));
	theHtml += " $";
	for (int pl = 1; pl <= p_nfg->NumPlayers(); pl++) {
	  if (profile->GetOutcome()) {
	    theHtml += profile->GetOutcome()->GetPayoff<std::string>(pl);
	  }
	  else {
	    theHtml += "0";
	  }
	  if (pl < p_nfg->NumPlayers()) {
	    theHtml += ",";
	  }
	}
	theHtml += "$ ";
	if (st2 < p_nfg->GetPlayer(p_colPlayer)->NumStrategies()) {
	  theHtml += " & ";
	}
      }
      if (st1 < p_nfg->GetPlayer(p_rowPlayer)->NumStrategies()) {
	theHtml += "\\\\\n";
      }
    } 

    theHtml += "\n\\end{game}";
  }

  theHtml += "\n";

  p_file << theHtml;
}

void PrintBanner(std::ostream &p_stream)
{
  p_stream << "Convert a Gambit .nfg file to LaTeX tables\n";
  p_stream << "Gambit version " VERSION ", Copyright (C) 1994-2010, The Gambit Project\n";
  p_stream << "This is free software, distributed under the GNU GPL\n\n";
}

void PrintHelp(char *progname)
{
  PrintBanner(std::cerr);
  std::cerr << "Usage: " << progname << " [OPTIONS] [file]\n";
  std::cerr << "If file is not specified, attempts to read game from standard input.\n";
  std::cerr << "Converts a game to LaTeX tables\n";

  std::cerr << "Options:\n";
  std::cerr << "  -c PLAYER        the player to show on columns (default is 2)\n";
  std::cerr << "  -r PLAYER        the player to show on rows (default is 1)\n";
  std::cerr << "  -h, --help       print this help message\n";
  std::cerr << "  -q               quiet mode (suppresses banner)\n";
  std::cerr << "  -v, --version    print version information\n";
  exit(1);
}


int main(int argc, char *argv[])
{
  int c;
  int rowPlayer = 1, colPlayer = 2;
  bool quiet = false;

  int long_opt_index = 0;
  struct option long_options[] = {
    { "help", 0, NULL, 'h'   },
    { "version", 0, NULL, 'v'  },
    { 0,    0,    0,    0   }
  };
  while ((c = getopt_long(argc, argv, "r:c:hvq", long_options, &long_opt_index)) != -1) {
    switch (c) {
    case 'v':
      PrintBanner(std::cerr); exit(1);
    case 'r':
      rowPlayer = atoi(optarg);
      break;
    case 'c':
      colPlayer = atoi(optarg);
      break;
    case 'q':
      quiet = true;
      break;
    case 'h':
      PrintHelp(argv[0]);
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

  if (rowPlayer == colPlayer) {
    std::cerr << argv[0] << ": Row and column players must be different.\n";
    return 1;
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

  Gambit::Game nfg;

  try {
    nfg = Gambit::ReadGame(*input_stream);
  }
  catch (...) {
    return 1;
  }

  if (rowPlayer < 1 || rowPlayer > nfg->NumPlayers()) {
    std::cerr << argv[0] << ": Player " << rowPlayer << " does not exist.\n";
  }
  if (colPlayer < 1 || colPlayer > nfg->NumPlayers()) {
    std::cerr << argv[0] << ": Player " << colPlayer << " does not exist.\n";
  }

  WriteOsborneFile(std::cout, nfg, rowPlayer, colPlayer);
  return 0;
}










