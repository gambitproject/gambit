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

namespace Gambit {

namespace {

/**
 * @brief Internal interface for formatting game tables in different formats
 */
class TableFormatter {
public:
  // Virtual destructor ensures that the correct destructor is called when
  // deleting a formatter through a pointer to this base class.
  virtual ~TableFormatter() = default;

  // These virtual functions must be provided by subclasses.
  virtual void WriteTitle(const std::string &title) = 0;

  // Setup for a specific 2D subtable (fixing strategies for other players)
  virtual void BeginSubtable(const Game &game, const GamePlayer &rowPlayer,
                             const GamePlayer &colPlayer, const PureStrategyProfile &profile) = 0;

  virtual void EndSubtable() = 0;

  // Writes the top header of the table showing column strategies
  virtual void WriteColumnHeaders(const GamePlayer &colPlayer) = 0;

  // Start a new row in the 2D table
  virtual void BeginRow(const GamePlayer &rowPlayer, const GameStrategy &rowStrategy,
                        bool isFirst) = 0;

  // Close the current row (e.g., </tr> in HTML or \\ in LaTeX)
  virtual void WriteRowEnd(bool isLast) = 0;

  // Write a single cell containing the payoffs for all players
  virtual void WriteCell(const std::string &payoffs, bool isLastCol) = 0;

  // Return the final accumulated string result
  virtual std::string GetResult() const = 0;
};

/**
 * @brief Formatter for HTML output.
 */
class HTMLTableFormatter : public TableFormatter {
public:
  void WriteTitle(const std::string &title) override
  {
    m_result += "<center><h1>" + title + "</h1></center>\n";
  }

  void BeginSubtable(const Game &game, const GamePlayer &rowPlayer, const GamePlayer &colPlayer,
                     const PureStrategyProfile &profile) override
  {
    // If the game has more than 2 players, we are looking at a "projection"
    // of the game where all players except the row and column players have
    // fixed strategies. We display those fixed strategies here.
    if (game->NumPlayers() > 2) {
      m_result += "<center><b>Subtable with strategies:</b></center>";
      for (auto player : game->GetPlayers()) {
        if (player == rowPlayer || player == colPlayer) {
          continue; // Skip the players who are free in this subtable
        }

        m_result += "<center><b>Player ";
        m_result += lexical_cast<std::string>(player->GetNumber());
        m_result += " Strategy ";
        m_result += lexical_cast<std::string>(profile->GetStrategy(player)->GetNumber());
        m_result += "</b></center>";
      }
    }
    m_result += "<table>";
  }

  void EndSubtable() override { m_result += "</table>"; }

  void WriteColumnHeaders(const GamePlayer &colPlayer) override
  {
    // Create the header row for the column player's label and strategies
    m_result += "<tr>";
    m_result += R"(<td colspan="2" rowspan="2"></td>)";
    m_result += R"(<td colspan=")" + lexical_cast<std::string>(colPlayer->GetStrategies().size()) +
                R"(" align="center"><b>)";
    m_result += colPlayer->GetLabel();
    m_result += "</b></td>";
    m_result += "</tr>";
    m_result += "<tr>";
    for (const auto &strategy : colPlayer->GetStrategies()) {
      m_result += R"(<td align="center"><b>)";
      m_result += strategy->GetLabel();
      m_result += "</b></td>";
    }
    m_result += "</tr>";
  }

  void BeginRow(const GamePlayer &rowPlayer, const GameStrategy &rowStrategy,
                bool isFirst) override
  {
    m_result += "<tr>";
    // For the very first strategy row, add a cell that spans all rows
    // to show the row player's name.
    if (isFirst) {
      m_result += R"(<td rowspan=")" +
                  lexical_cast<std::string>(rowPlayer->GetStrategies().size()) +
                  R"(" align="center" valign="middle"><b>)";
      m_result += rowPlayer->GetLabel();
      m_result += "</b></td>";
    }
    m_result += R"(<td align="center"><b>)";
    m_result += rowStrategy->GetLabel();
    m_result += "</b></td>";
  }

  void WriteRowEnd(bool isLast) override { m_result += "</tr>"; }

  void WriteCell(const std::string &payoffs, bool isLastCol) override
  {
    m_result += "<td align=center>" + payoffs + "</td>";
  }

  std::string GetResult() const override { return m_result; }

private:
  std::string m_result; // Accumulates the HTML string
};

/**
 * @brief Formatter for LaTeX output using the sgame package style.
 */
class LaTeXTableFormatter : public TableFormatter {
public:
  void WriteTitle(const std::string &title) override
  {
    if (!title.empty()) {
      m_result += "\\begin{center}\n";
      m_result += "{\\Large \\textbf{";
      m_result += title;
      m_result += "}}\n";
      m_result += "\\end{center}\n";
    }
  }

  void BeginSubtable(const Game &game, const GamePlayer &rowPlayer, const GamePlayer &colPlayer,
                     const PureStrategyProfile &profile) override
  {
    // If the game has more than 2 players, display fixed strategies
    if (game->NumPlayers() > 2) {
      m_result += "\\begin{center}\n";
      m_result += "\\textbf{Subtable with strategies:}\\\\\n";
      for (auto player : game->GetPlayers()) {
        if (player == rowPlayer || player == colPlayer) {
          continue;
        }

        m_result += "Player ";
        m_result += lexical_cast<std::string>(player->GetNumber());
        m_result += " Strategy ";
        m_result += lexical_cast<std::string>(profile->GetStrategy(player)->GetNumber());
        m_result += "\\\\\n";
      }
      m_result += "\\end{center}\n";
    }

    m_colSize = colPlayer->GetStrategies().size();
    m_result += "\\begin{center}\n";
    m_result += "\\begin{tabular}{cc";
    for (size_t i = 0; i < m_colSize; ++i) {
      m_result += "|c";
    }
    m_result += "}\n";

    // Column player label row
    m_result += "\\multicolumn{2}{c}{} & \\multicolumn{";
    m_result += lexical_cast<std::string>(m_colSize);
    m_result += "}{c}{\\textbf{";
    m_result += colPlayer->GetLabel();
    m_result += "}} \\\\\n";
  }

  void EndSubtable() override
  {
    m_result += "\\end{tabular}\n";
    m_result += "\\end{center}";
  }

  void WriteColumnHeaders(const GamePlayer &colPlayer) override
  {
    // Column strategies row
    m_result += "\\multicolumn{2}{c}{} ";
    for (const auto &strategy : colPlayer->GetStrategies()) {
      m_result += " & \\textbf{";
      m_result += strategy->GetLabel();
      m_result += "}";
    }
    m_result += " \\\\ \\cline{3-";
    m_result += lexical_cast<std::string>(2 + m_colSize);
    m_result += "}\n";
  }

  void BeginRow(const GamePlayer &rowPlayer, const GameStrategy &rowStrategy,
                bool isFirst) override
  {
    if (isFirst) {
      m_result += "\\textbf{";
      m_result += rowPlayer->GetLabel();
      m_result += "} ";
    }
    m_result += "& \\textbf{";
    m_result += rowStrategy->GetLabel();
    m_result += "} ";
  }

  void WriteRowEnd(bool isLast) override
  {
    m_result += " \\\\ \\cline{3-";
    m_result += lexical_cast<std::string>(2 + m_colSize);
    m_result += "}\n";
  }

  void WriteCell(const std::string &payoffs, bool isLastCol) override
  {
    m_result += "& $" + payoffs + "$ ";
  }

  std::string GetResult() const override { return m_result; }

private:
  std::string m_result;
  size_t m_colSize{0};
};

/**
 * @brief Shared logic for iterating over strategies and writing game tables.
 *
 * This function implements the logic of projecting a multi-player game
 * into a series of 2D tables. It iterates over all possible strategy profiles
 * of the "fixed" players and for each, creates a table showing all
 * strategy combinations of the row and column players.
 */
void WriteGameTableInternal(const Game &p_game, const GamePlayer &p_rowPlayer,
                            const GamePlayer &p_colPlayer, TableFormatter &p_formatter)
{
  p_formatter.WriteTitle(p_game->GetTitle());

  // Create a profile where row and column players are restricted to their first
  // strategies. This allows us to find all combinations of strategies for
  // the OTHER players.
  auto support = StrategySupportProfile(p_game)
                     .RestrictTo(p_rowPlayer->GetStrategies().front())
                     .RestrictTo(p_colPlayer->GetStrategies().front());

  // Iterate through all subtables (contingencies for players other than row/col)
  for (const auto &iter : StrategyContingencies(support)) {
    p_formatter.BeginSubtable(p_game, p_rowPlayer, p_colPlayer, iter);
    p_formatter.WriteColumnHeaders(p_colPlayer);

    // Loop over the row player's strategies
    for (const auto &row_strategy : p_rowPlayer->GetStrategies()) {
      // Create a profile based on the current subtable's fixed strategies
      const PureStrategyProfile profile = iter;
      profile->SetStrategy(row_strategy);

      p_formatter.BeginRow(p_rowPlayer, row_strategy,
                           row_strategy == p_rowPlayer->GetStrategies().front());

      // Loop over the column player's strategies
      for (const auto &col_strategy : p_colPlayer->GetStrategies()) {
        profile->SetStrategy(col_strategy);

        // Extract the payoffs for every player for this specific outcome
        std::string payoffs;
        for (const auto &player : p_game->GetPlayers()) {
          try {
            if (profile->GetOutcome()) {
              payoffs += profile->GetOutcome()->GetPayoff<std::string>(player);
            }
            else {
              payoffs += "0";
            }
          }
          catch (UndefinedException &) {
            // Some game representations might not have outcomes defined directly
            payoffs += lexical_cast<std::string>(profile->GetPayoff(player));
          }
          if (player != p_game->GetPlayers().back()) {
            payoffs += ",";
          }
        }

        p_formatter.WriteCell(payoffs, col_strategy == p_colPlayer->GetStrategies().back());
      }

      // Finalize the current row
      p_formatter.WriteRowEnd(row_strategy == p_rowPlayer->GetStrategies().back());
    }
    p_formatter.EndSubtable();
  }
}

} // end anonymous namespace

std::string WriteHTMLFile(const Game &p_game, const GamePlayer &p_rowPlayer,
                          const GamePlayer &p_colPlayer)
{
  HTMLTableFormatter formatter;
  WriteGameTableInternal(p_game, p_rowPlayer, p_colPlayer, formatter);
  return formatter.GetResult() + "\n";
}

std::string WriteLaTeXFile(const Game &p_game, const GamePlayer &p_rowPlayer,
                           const GamePlayer &p_colPlayer)
{
  LaTeXTableFormatter formatter;
  WriteGameTableInternal(p_game, p_rowPlayer, p_colPlayer, formatter);
  return formatter.GetResult() + "\n";
}

} // end namespace Gambit
