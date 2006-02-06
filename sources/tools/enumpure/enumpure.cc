//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute pure-strategy equilibria in extensive form games
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
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

#include <unistd.h>
#include <iostream>
#include "libgambit/libgambit.h"

using namespace Gambit;

template <class T>
void PrintProfile(std::ostream &p_stream,
		  const MixedBehavProfile<T> &p_profile)
{
  for (int i = 1; i <= p_profile.Length(); i++) {
    p_stream << p_profile[i];
    if (i < p_profile.Length()) {
      p_stream << ',';
    }
  }

  p_stream << std::endl;
}

void SolveBehav(Game p_efg)
{
  for (BehavIterator citer(p_efg); !citer.AtEnd(); citer++) {
    bool isNash = true;

    for (GamePlayerIterator player = p_efg->Players(); 
	 isNash && !player.AtEnd(); player++)  {
      Rational current = citer->GetPayoff<Rational>((GamePlayerRep *) player);
	
      for (GameInfosetIterator infoset = player->Infosets();
	   isNash && !infoset.AtEnd(); infoset++)  {
	for (GameActionIterator action = infoset->Actions();
	     !action.AtEnd(); action++) {
	  if (citer->GetActionValue<Rational>((GameActionRep *) action) > current)  {
	    isNash = false;
	    break;
	  }
	}
      }
    }
      
    if (isNash)  {
      MixedBehavProfile<Rational> temp(p_efg);
      // zero out all the entries, since any equilibria are pure
      ((Vector<Rational> &) temp).operator=(Rational(0));

      for (GamePlayerIterator player = p_efg->Players();
	   !player.AtEnd(); player++) {
	for (GameInfosetIterator infoset = player->Infosets();
	     !infoset.AtEnd(); infoset++) {
	  temp(citer->GetAction((GameInfosetRep *) infoset)) = 1;
	}
      }

      PrintProfile(std::cout, temp);
    }
  }
}

template <class T>
void PrintProfile(std::ostream &p_stream,
		  const MixedStrategyProfile<T> &p_profile)
{
  p_stream << "NE,";
  for (int i = 1; i <= p_profile.Length(); i++) {
    p_stream << p_profile[i];
    if (i < p_profile.Length()) {
      p_stream << ',';
    }
  }

  p_stream << std::endl;
}

void SolveMixed(Game p_nfg)
{
  for (StrategyIterator citer(p_nfg); !citer.AtEnd(); citer++) {
    bool flag = true;

    for (GamePlayerIterator player = p_nfg->Players(); 
	 flag && !player.AtEnd(); player++)  {
      Rational current = citer->GetPayoff<Rational>((GamePlayerRep *) player);
      PureStrategyProfile p(*citer); 
      for (GameStrategyIterator strategy = player->Strategies();
	   !strategy.AtEnd(); strategy++) {
	if (p.GetStrategyValue<Rational>((GameStrategyRep *) strategy) > current)  {
	  flag = false;
	  break;
	}
      }
    }
    
    if (flag)  {
      MixedStrategyProfile<Rational> temp(p_nfg);
      ((Vector<Rational> &) temp).operator=(Rational(0));
      PureStrategyProfile profile(*citer);
      for (GamePlayerIterator player = p_nfg->Players();
	   !player.AtEnd(); player++) {
	temp(profile.GetStrategy((GamePlayerRep *) player)) = 1;
      }
      
      PrintProfile(std::cout, temp);
    }
  }
}


void PrintBanner(std::ostream &p_stream)
{
  p_stream << "Search for Nash equilibria in pure strategies\n";
  p_stream << "Gambit version " VERSION ", Copyright (C) 2005, The Gambit Project\n";
  p_stream << "This is free software, distributed under the GNU GPL\n\n";
}

void PrintHelp(char *progname)
{
  PrintBanner(std::cerr);
  std::cerr << "Usage: " << progname << " [OPTIONS]\n";
  std::cerr << "Accepts game on standard input.\n";
  std::cerr << "With no options, locates all Nash equilibria in pure strategies.\n\n";

  std::cerr << "Options:\n";
  std::cerr << "  -S               use strategic game\n";
  std::cerr << "  -h               print this help message\n";
  std::cerr << "  -q               quiet mode (suppresses banner)\n";
  exit(1);
}


int main(int argc, char *argv[])
{
  opterr = 0;
  bool quiet = false, useStrategic = false;

  int c;
  while ((c = getopt(argc, argv, "hqS")) != -1) {
    switch (c) {
    case 'S':
      useStrategic = true;
      break;
    case 'h':
      PrintHelp(argv[0]);
      break;
    case 'q':
      quiet = true;
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

  try {
    Game game = ReadGame(std::cin);

    game->BuildComputedValues();
    if (!game->IsTree() || useStrategic) {
      SolveMixed(game);
    }
    else {
      SolveBehav(game);
    }
    return 0;
  }
  catch (InvalidFileException) {
    std::cerr << "Error: Game not in a recognized format.\n";
    return 1;
  }
  catch (...) {
    std::cerr << "Error: An internal error occurred.\n";
    return 1;
  }
}





