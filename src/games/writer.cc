//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: library/include/gambit/writer.h
// Classes for writing out games to various formats
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

#include "gambit.h"
#include "writer.h"

using namespace Gambit;

std::string HTMLGameWriter::Write(const Game &p_game, int p_rowPlayer, int p_colPlayer) const
{
  std::string theHtml;
  theHtml += "<center><h1>" + p_game->GetTitle() + "</h1></center>\n";

  for (StrategyProfileIterator iter(p_game, p_rowPlayer, 1, p_colPlayer, 1); !iter.AtEnd();
       iter++) {
    if (p_game->NumPlayers() > 2) {
      theHtml += "<center><b>Subtable with strategies:</b></center>";
      for (int pl = 1; pl <= p_game->NumPlayers(); pl++) {
        if (pl == p_rowPlayer || pl == p_colPlayer) {
          continue;
        }

        theHtml += "<center><b>Player ";
        theHtml += lexical_cast<std::string>(pl);
        theHtml += " Strategy ";
        theHtml += lexical_cast<std::string>((*iter)->GetStrategy(pl)->GetNumber());
        theHtml += "</b></center>";
      }
    }

    theHtml += "<table>";
    theHtml += "<tr>";
    theHtml += "<td></td>";
    for (int st = 1; st <= p_game->GetPlayer(p_colPlayer)->NumStrategies(); st++) {
      theHtml += "<td align=center><b>";
      theHtml += p_game->GetPlayer(p_colPlayer)->GetStrategy(st)->GetLabel();
      theHtml += "</b></td>";
    }
    theHtml += "</tr>";
    for (int st1 = 1; st1 <= p_game->GetPlayer(p_rowPlayer)->NumStrategies(); st1++) {
      PureStrategyProfile profile = *iter;
      profile->SetStrategy(p_game->GetPlayer(p_rowPlayer)->GetStrategy(st1));
      theHtml += "<tr>";
      theHtml += "<td align=center><b>";
      theHtml += p_game->GetPlayer(p_rowPlayer)->GetStrategy(st1)->GetLabel();
      theHtml += "</b></td>";
      for (int st2 = 1; st2 <= p_game->GetPlayer(p_colPlayer)->NumStrategies(); st2++) {
        profile->SetStrategy(p_game->GetPlayer(p_colPlayer)->GetStrategy(st2));
        theHtml += "<td align=center>";
        for (int pl = 1; pl <= p_game->NumPlayers(); pl++) {
          try {
            if (profile->GetOutcome()) {
              theHtml += static_cast<std::string>(profile->GetOutcome()->GetPayoff(pl));
            }
            else {
              theHtml += "0";
            }
          }
          catch (UndefinedException &) {
            theHtml += lexical_cast<std::string>(profile->GetPayoff(pl));
          }
          if (pl < p_game->NumPlayers()) {
            theHtml += ",";
          }
        }
        theHtml += "</td>";
      }
      theHtml += "</tr>";
    }

    theHtml += "</table>";
  }
  theHtml += "\n";
  return theHtml;
}

std::string LaTeXGameWriter::Write(const Game &p_game, int p_rowPlayer, int p_colPlayer) const
{
  std::string theHtml;

  for (StrategyProfileIterator iter(p_game, p_rowPlayer, 1, p_colPlayer, 1); !iter.AtEnd();
       iter++) {
    theHtml += "\\begin{game}{";
    theHtml += lexical_cast<std::string>(p_game->GetPlayer(p_rowPlayer)->NumStrategies());
    theHtml += "}{";
    theHtml += lexical_cast<std::string>(p_game->GetPlayer(p_colPlayer)->NumStrategies());
    theHtml += "}[";
    theHtml += p_game->GetPlayer(p_rowPlayer)->GetLabel();
    theHtml += "][";
    theHtml += p_game->GetPlayer(p_colPlayer)->GetLabel();
    theHtml += "]";

    if (p_game->NumPlayers() > 2) {
      theHtml += "[";
      for (int pl = 1; pl <= p_game->NumPlayers(); pl++) {
        if (pl == p_rowPlayer || pl == p_colPlayer) {
          continue;
        }

        theHtml += "Player ";
        theHtml += lexical_cast<std::string>(pl);
        theHtml += " Strategy ";
        theHtml += lexical_cast<std::string>((*iter)->GetStrategy(pl)->GetNumber());
        theHtml += " ";
      }
      theHtml += "]";
    }

    theHtml += "\n&";

    for (int st = 1; st <= p_game->GetPlayer(p_colPlayer)->NumStrategies(); st++) {
      theHtml += p_game->GetPlayer(p_colPlayer)->GetStrategy(st)->GetLabel();
      if (st < p_game->GetPlayer(p_colPlayer)->NumStrategies()) {
        theHtml += " & ";
      }
    }
    theHtml += "\\\\\n";

    for (int st1 = 1; st1 <= p_game->GetPlayer(p_rowPlayer)->NumStrategies(); st1++) {
      PureStrategyProfile profile = *iter;
      profile->SetStrategy(p_game->GetPlayer(p_rowPlayer)->GetStrategy(st1));
      theHtml += p_game->GetPlayer(p_rowPlayer)->GetStrategy(st1)->GetLabel();
      theHtml += " & ";
      for (int st2 = 1; st2 <= p_game->GetPlayer(p_colPlayer)->NumStrategies(); st2++) {
        profile->SetStrategy(p_game->GetPlayer(p_colPlayer)->GetStrategy(st2));
        theHtml += " $";
        for (int pl = 1; pl <= p_game->NumPlayers(); pl++) {
          try {
            if (profile->GetOutcome()) {
              theHtml += static_cast<std::string>(profile->GetOutcome()->GetPayoff(pl));
            }
            else {
              theHtml += "0";
            }
          }
          catch (UndefinedException &) {
            theHtml += lexical_cast<std::string>(profile->GetPayoff(pl));
          }
          if (pl < p_game->NumPlayers()) {
            theHtml += ",";
          }
        }
        theHtml += "$ ";
        if (st2 < p_game->GetPlayer(p_colPlayer)->NumStrategies()) {
          theHtml += " & ";
        }
      }
      if (st1 < p_game->GetPlayer(p_rowPlayer)->NumStrategies()) {
        theHtml += "\\\\\n";
      }
    }

    theHtml += "\n\\end{game}";
  }
  theHtml += "\n";
  return theHtml;
}
