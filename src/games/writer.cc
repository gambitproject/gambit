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
  virtual ~TableFormatter() = default;

  virtual void WriteTitle(const std::string &title) = 0;

  virtual void BeginSubtable(const Game &game, const GamePlayer &rowPlayer,
                             const GamePlayer &colPlayer, const PureStrategyProfile &profile) = 0;

  virtual void EndSubtable() = 0;

  virtual void WriteColumnHeaders(const GamePlayer &colPlayer) = 0;

  virtual void BeginRow(const GamePlayer &rowPlayer, const GameStrategy &rowStrategy,
                        bool isFirst) = 0;

  virtual void WriteRowEnd(bool isLast) = 0;

  virtual void WriteCell(const std::string &payoffs, bool isLastCol) = 0;

  virtual std::string GetResult() const = 0;
};

/**
 * @brief Formatter for HTML output
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
    if (game->NumPlayers() > 2) {
      m_result += "<center><b>Subtable with strategies:</b></center>";
      for (auto player : game->GetPlayers()) {
        if (player == rowPlayer || player == colPlayer) {
          continue;
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
  std::string m_result;
};

/**
 * @brief Formatter for LaTeX output
 */
class LaTeXTableFormatter : public TableFormatter {
public:
  void WriteTitle(const std::string &title) override
  {
    // LaTeX output currently doesn't include the title in this function
  }

  void BeginSubtable(const Game &game, const GamePlayer &rowPlayer, const GamePlayer &colPlayer,
                     const PureStrategyProfile &profile) override
  {
    m_result += "\\begin{game}{";
    m_result += lexical_cast<std::string>(rowPlayer->GetStrategies().size());
    m_result += "}{";
    m_result += lexical_cast<std::string>(colPlayer->GetStrategies().size());
    m_result += "}[";
    m_result += rowPlayer->GetLabel();
    m_result += "][";
    m_result += colPlayer->GetLabel();
    m_result += "]";

    if (game->NumPlayers() > 2) {
      m_result += "[";
      for (auto player : game->GetPlayers()) {
        if (player == rowPlayer || player == colPlayer) {
          continue;
        }

        m_result += "Player ";
        m_result += lexical_cast<std::string>(player->GetNumber());
        m_result += " Strategy ";
        m_result += lexical_cast<std::string>(profile->GetStrategy(player)->GetNumber());
        m_result += " ";
      }
      m_result += "]";
    }
    m_result += "\n&";
  }

  void EndSubtable() override { m_result += "\n\\end{game}"; }

  void WriteColumnHeaders(const GamePlayer &colPlayer) override
  {
    for (const auto &strategy : colPlayer->GetStrategies()) {
      m_result += strategy->GetLabel();
      if (strategy != colPlayer->GetStrategies().back()) {
        m_result += " & ";
      }
    }
    m_result += "\\\\\n";
  }

  void BeginRow(const GamePlayer &rowPlayer, const GameStrategy &rowStrategy,
                bool isFirst) override
  {
    m_result += rowStrategy->GetLabel();
    m_result += " & ";
  }

  void WriteRowEnd(bool isLast) override
  {
    if (!isLast) {
      m_result += "\\\\\n";
    }
  }

  void WriteCell(const std::string &payoffs, bool isLastCol) override
  {
    m_result += " $" + payoffs + "$ ";
    if (!isLastCol) {
      m_result += " & ";
    }
  }

  std::string GetResult() const override { return m_result; }

private:
  std::string m_result;
};

/**
 * @brief Shared logic for iterating over strategies and writing game tables
 */
void WriteGameTableInternal(const Game &p_game, const GamePlayer &p_rowPlayer,
                            const GamePlayer &p_colPlayer, TableFormatter &p_formatter)
{
  p_formatter.WriteTitle(p_game->GetTitle());

  auto support = StrategySupportProfile(p_game)
                     .RestrictTo(p_rowPlayer->GetStrategies().front())
                     .RestrictTo(p_colPlayer->GetStrategies().front());

  for (const auto &iter : StrategyContingencies(support)) {
    p_formatter.BeginSubtable(p_game, p_rowPlayer, p_colPlayer, iter);
    p_formatter.WriteColumnHeaders(p_colPlayer);

    for (const auto &row_strategy : p_rowPlayer->GetStrategies()) {
      const PureStrategyProfile profile = iter;
      profile->SetStrategy(row_strategy);

      p_formatter.BeginRow(p_rowPlayer, row_strategy,
                           row_strategy == p_rowPlayer->GetStrategies().front());

      for (const auto &col_strategy : p_colPlayer->GetStrategies()) {
        profile->SetStrategy(col_strategy);

        // Generate payoffs string
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
            payoffs += lexical_cast<std::string>(profile->GetPayoff(player));
          }
          if (player != p_game->GetPlayers().back()) {
            payoffs += ",";
          }
        }

        p_formatter.WriteCell(payoffs, col_strategy == p_colPlayer->GetStrategies().back());
      }

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
