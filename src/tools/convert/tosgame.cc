//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
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

  for (Gambit::StrategyProfileIterator iter(p_nfg, p_rowPlayer, 1, p_colPlayer, 1);
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
      Gambit::PureStrategyProfile profile = *iter;
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

