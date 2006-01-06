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

template <class T>
void PrintProfile(std::ostream &p_stream,
		  const Gambit::MixedBehavProfile<T> &p_profile)
{
  for (int i = 1; i <= p_profile.Length(); i++) {
    p_stream << p_profile[i];
    if (i < p_profile.Length()) {
      p_stream << ',';
    }
  }

  p_stream << std::endl;
}

void SolveBehav(Gambit::Game p_efg)
{
  Gambit::BehavIterator citer(p_efg);
  gbtPVector<Gambit::Rational> probs(p_efg->NumInfosets());

  int contNumber = 1;
  try {
    do {
      bool flag = true;
      // Commenting this out means we don't take advantage of reachability
      // information; fix this in future.
      //citer.GetProfile().InfosetProbs(probs);

      for (int pl = 1; flag && pl <= p_efg->NumPlayers(); pl++)  {
	Gambit::Rational current = citer.GetPayoff(pl);
	Gambit::PureBehavProfile p = citer.GetProfile();
	
	Gambit::GamePlayer player = p_efg->GetPlayer(pl);

	for (int iset = 1; flag && iset <= player->NumInfosets(); iset++)  {
	  Gambit::GameInfoset infoset = player->GetInfoset(iset);

	  for (int act = 1; act <= infoset->NumActions(); act++)  {
	    p.SetAction(infoset->GetAction(act));
	    if (p.GetPayoff(pl) > current)  {
	      flag = false;
	      break;
	    }
	  }
	}
      }
      
      if (flag)  {
	Gambit::MixedBehavProfile<Gambit::Rational> temp(p_efg);
	// zero out all the entries, since any equilibria are pure
	((gbtVector<Gambit::Rational> &) temp).operator=(Gambit::Rational(0));
	const Gambit::PureBehavProfile &profile = citer.GetProfile();
	for (int pl = 1; pl <= p_efg->NumPlayers(); pl++)  {
	  for (int iset = 1;
	       iset <= p_efg->GetPlayer(pl)->NumInfosets();
	       iset++) {
	    int act = profile.GetAction(p_efg->GetPlayer(pl)->GetInfoset(iset))->GetNumber();
	    temp(pl, iset, act) = 1;
	  }
	}

	PrintProfile(std::cout, temp);
      }
      contNumber++;
    }  while (citer.NextContingency());
  }
  catch (...) {
    // catch exception; return solutions computed (if any)
  }
}

template <class T>
void PrintProfile(std::ostream &p_stream,
		  const Gambit::MixedStrategyProfile<T> &p_profile)
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

void SolveMixed(Gambit::Game p_nfg)
{
  Gambit::StrategyIterator citer(p_nfg);

  int ncont = 1;
  for (int pl = 1; pl <= p_nfg->NumPlayers(); pl++) {
    ncont *= p_nfg->GetPlayer(pl)->NumStrategies();
  }

  int contNumber = 1;
  do  {
    bool flag = true;

    for (int pl = 1; flag && pl <= p_nfg->NumPlayers(); pl++)  {
      Gambit::Rational current = citer.GetPayoff(pl);
      Gambit::PureStrategyProfile p = citer.GetProfile();
      for (int i = 1; i <= p_nfg->GetPlayer(pl)->NumStrategies(); i++)  {
	p.SetStrategy(p_nfg->GetPlayer(pl)->GetStrategy(i));
	if (p.GetPayoff(pl) > current)  {
	  flag = false;
	  break;
	}
      }
    }
    
    if (flag)  {
      Gambit::MixedStrategyProfile<Gambit::Rational> temp(p_nfg);
      ((gbtVector<Gambit::Rational> &) temp).operator=(Gambit::Rational(0));
      Gambit::PureStrategyProfile profile = citer.GetProfile();
      for (int pl = 1; pl <= p_nfg->NumPlayers(); pl++) {
	temp(pl, profile.GetStrategy(pl)->GetNumber()) = 1;
      }
      
      PrintProfile(std::cout, temp);
    }
    contNumber++;
  }  while (citer.NextContingency());
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
    Gambit::Game game = Gambit::ReadGame(std::cin);
    if (!game->IsTree() || useStrategic) {
      SolveMixed(game);
    }
    else {
      SolveBehav(game);
    }
    return 0;
  }
  catch (Gambit::InvalidFileException) {
    std::cerr << "Error: Game not in a recognized format.\n";
    return 1;
  }
  catch (...) {
    std::cerr << "Error: An internal error occurred.\n";
    return 1;
  }
}





