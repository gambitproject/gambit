//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/convert/convert.cc
// Convert among various file formats
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
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <unistd.h>

#include "libgambit/libgambit.h"

void WriteHtmlFile(std::ostream &, const Gambit::Game &, int, int);
void WriteOsborneFile(std::ostream &, const Gambit::Game &, int, int);

void PrintBanner(std::ostream &p_stream)
{
  p_stream << "Convert games among various file formats\n";
  p_stream << "Gambit version " VERSION ", Copyright (C) 1994-2014, The Gambit Project\n";
  p_stream << "This is free software, distributed under the GNU GPL\n\n";
}

void PrintHelp(char *progname)
{
  PrintBanner(std::cerr);
  std::cerr << "Usage: " << progname << " [OPTIONS]\n";
  std::cerr << "Accepts input file on standard input.\n";
  std::cerr << "Converts games among various file formats\n";

  std::cerr << "Options:\n";
  std::cerr << "  -O FORMAT        output file format (required):\n";
  std::cerr << "     FORMAT=html   convert to HTML\n";
  std::cerr << "     FORMAT=sgame  convert to LaTeX sgame style\n";
  std::cerr << "  -c PLAYER        the player to show on columns (default is 2)\n";
  std::cerr << "  -r PLAYER        the player to show on rows (default is 1)\n";
  std::cerr << "  -h               print this help message\n";
  std::cerr << "  -q               quiet mode (suppresses banner)\n";
  exit(1);
}


int main(int argc, char *argv[])
{
  int c;
  int rowPlayer = 1, colPlayer = 2;
  bool quiet = false;
  std::string format;

  while ((c = getopt(argc, argv, "r:c:hqO:")) != -1) {
    switch (c) {
    case 'r':
      rowPlayer = atoi(optarg);
      break;
    case 'c':
      colPlayer = atoi(optarg);
      break;
    case 'O':
      format = optarg;
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

  if (!quiet) {
    PrintBanner(std::cerr);
  }

  if (format == "") {
    std::cerr << argv[0] << ": Output format argument -O required.\n";
    return 1;
  }
  else if (format != "sgame" && format != "html") {
    std::cerr << argv[0] << ": Unknown output format '" << format << "'.\n";
    return 1;
  }

  if (rowPlayer == colPlayer) {
    std::cerr << argv[0] << ": Row and column players must be different.\n";
    return 1;
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
    Gambit::Game game = Gambit::ReadGame(*input_stream);

    if (rowPlayer < 1 || rowPlayer > game->NumPlayers()) {
      std::cerr << argv[0] << ": Player " << rowPlayer << " does not exist.\n";
      return 1;
    }
    if (colPlayer < 1 || colPlayer > game->NumPlayers()) {
      std::cerr << argv[0] << ": Player " << colPlayer << " does not exist.\n";
      return 1;
    }

    if (format == "html") {
      WriteHtmlFile(std::cout, game, rowPlayer, colPlayer);
    }
    else {
      WriteOsborneFile(std::cout, game, rowPlayer, colPlayer);
    }
    return 0;
  }
  catch (std::runtime_error &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
