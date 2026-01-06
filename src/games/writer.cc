//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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

namespace Gambit {

std::string WriteHTMLFile(const Game &p_game, const GamePlayer &p_rowPlayer,
                          const GamePlayer &p_colPlayer)
{
  std::string theHtml;
  theHtml += "<center><h1>" + p_game->GetTitle() + "</h1></center>\n";

  for (auto iter : StrategyContingencies(
           p_game, {p_rowPlayer->GetStrategies().front(), p_colPlayer->GetStrategies().front()})) {
    if (p_game->NumPlayers() > 2) {
      theHtml += "<center><b>Subtable with strategies:</b></center>";
      for (auto player : p_game->GetPlayers()) {
        if (player == p_rowPlayer || player == p_colPlayer) {
          continue;
        }

        theHtml += "<center><b>Player ";
        theHtml += std::to_string(player->GetNumber());
        theHtml += " Strategy ";
        theHtml += std::to_string(iter->GetStrategy(player)->GetNumber());
        theHtml += "</b></center>";
      }
    }

    theHtml += "<table>";
    theHtml += "<tr>";
    theHtml += "<td></td>";
    for (const auto &strategy : p_colPlayer->GetStrategies()) {
      theHtml += "<td align=center><b>";
      theHtml += strategy->GetLabel();
      theHtml += "</b></td>";
    }
    theHtml += "</tr>";
    for (const auto &row_strategy : p_rowPlayer->GetStrategies()) {
      const PureStrategyProfile profile = iter;
      profile->SetStrategy(row_strategy);
      theHtml += "<tr>";
      theHtml += "<td align=center><b>";
      theHtml += row_strategy->GetLabel();
      theHtml += "</b></td>";
      for (const auto &col_strategy : p_colPlayer->GetStrategies()) {
        profile->SetStrategy(col_strategy);
        theHtml += "<td align=center>";
        for (const auto &player : p_game->GetPlayers()) {
          try {
            if (profile->GetOutcome()) {
              theHtml += profile->GetOutcome()->GetPayoff<std::string>(player);
            }
            else {
              theHtml += "0";
            }
          }
          catch (UndefinedException &) {
            theHtml += lexical_cast<std::string>(profile->GetPayoff(player));
          }
          if (player != p_game->GetPlayers().back()) {
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

std::string WriteLaTeXFile(const Game &p_game, const GamePlayer &p_rowPlayer,
                           const GamePlayer &p_colPlayer)
{
  std::string theHtml;

  for (auto iter : StrategyContingencies(
           p_game, {p_rowPlayer->GetStrategies().front(), p_colPlayer->GetStrategies().front()})) {
    theHtml += "\\begin{game}{";
    theHtml += std::to_string(p_rowPlayer->GetStrategies().size());
    theHtml += "}{";
    theHtml += std::to_string(p_colPlayer->GetStrategies().size());
    theHtml += "}[";
    theHtml += p_rowPlayer->GetLabel();
    theHtml += "][";
    theHtml += p_colPlayer->GetLabel();
    theHtml += "]";

    if (p_game->NumPlayers() > 2) {
      theHtml += "[";
      for (auto player : p_game->GetPlayers()) {
        if (player == p_rowPlayer || player == p_colPlayer) {
          continue;
        }

        theHtml += "Player ";
        theHtml += lexical_cast<std::string>(player->GetNumber());
        theHtml += " Strategy ";
        theHtml += lexical_cast<std::string>(iter->GetStrategy(player)->GetNumber());
        theHtml += " ";
      }
      theHtml += "]";
    }

    theHtml += "\n&";

    for (const auto &strategy : p_colPlayer->GetStrategies()) {
      theHtml += strategy->GetLabel();
      if (strategy != p_colPlayer->GetStrategies().back()) {
        theHtml += " & ";
      }
    }
    theHtml += "\\\\\n";

    for (const auto &row_strategy : p_rowPlayer->GetStrategies()) {
      const PureStrategyProfile profile = iter;
      profile->SetStrategy(row_strategy);
      theHtml += row_strategy->GetLabel();
      theHtml += " & ";
      for (const auto &col_strategy : p_colPlayer->GetStrategies()) {
        profile->SetStrategy(col_strategy);
        theHtml += " $";
        for (const auto &player : p_game->GetPlayers()) {
          try {
            if (profile->GetOutcome()) {
              theHtml += profile->GetOutcome()->GetPayoff<std::string>(player);
            }
            else {
              theHtml += "0";
            }
          }
          catch (UndefinedException &) {
            theHtml += lexical_cast<std::string>(profile->GetPayoff(player));
          }
          if (player != p_game->GetPlayers().back()) {
            theHtml += ",";
          }
        }
        theHtml += "$ ";
        if (col_strategy != p_colPlayer->GetStrategies().back()) {
          theHtml += " & ";
        }
      }
      if (row_strategy != p_rowPlayer->GetStrategies().back()) {
        theHtml += "\\\\\n";
      }
    }

    theHtml += "\n\\end{game}";
  }
  theHtml += "\n";
  return theHtml;
}

} // end namespace Gambit
