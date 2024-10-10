//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/file.cc
// Parser for reading game savefiles
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
#include <cctype>
#include <iostream>
#include <fstream>
#include <map>

#include "gambit.h"
// for explicit access to turning off canonicalization
#include "gametree.h"

namespace {
// This anonymous namespace encapsulates the file-parsing code

using namespace Gambit;

using GameFileToken = enum {
  TOKEN_NUMBER = 0,
  TOKEN_TEXT = 1,
  TOKEN_SYMBOL = 2,
  TOKEN_LBRACE = 3,
  TOKEN_RBRACE = 4,
  TOKEN_COMMA = 5,
  TOKEN_EOF = 6,
  TOKEN_NONE = 7
};

//!
//! This parser class implements the semantics of Gambit savefiles,
//! including the nonsignificance of whitespace and the possibility of
//! escaped-quotes within text labels.
//!
class GameParserState {
private:
  std::istream &m_file;

  int m_currentLine{1};
  int m_currentColumn{1};
  GameFileToken m_lastToken{TOKEN_NONE};
  std::string m_lastText;

  void ReadChar(char &c);
  void UnreadChar();
  void IncreaseLine();

public:
  explicit GameParserState(std::istream &p_file) : m_file(p_file) {}

  GameFileToken GetNextToken();
  GameFileToken GetCurrentToken() const { return m_lastToken; }
  /// Throw an InvalidFileException with the specified message
  void OnParseError(const std::string &p_message) const;
  const std::string &GetLastText() const { return m_lastText; }

  void ExpectCurrentToken(GameFileToken p_tokenType, const std::string &p_message)
  {
    if (GetCurrentToken() != p_tokenType) {
      OnParseError("Expected " + p_message);
    }
  }
  void ExpectNextToken(GameFileToken p_tokenType, const std::string &p_message)
  {
    if (GetNextToken() != p_tokenType) {
      OnParseError("Expected " + p_message);
    }
  }
  void AcceptNextToken(GameFileToken p_tokenType)
  {
    if (GetNextToken() == p_tokenType) {
      GetNextToken();
    }
  }
};

void GameParserState::OnParseError(const std::string &p_message) const
{
  throw InvalidFileException("line " + std::to_string(m_currentLine) + ":" +
                             std::to_string(m_currentColumn) + ": " + p_message);
}

void GameParserState::ReadChar(char &c)
{
  m_file.get(c);
  m_currentColumn++;
}

void GameParserState::UnreadChar()
{
  if (!m_file.eof()) {
    m_file.unget();
    m_currentColumn--;
  }
}

void GameParserState::IncreaseLine()
{
  m_currentLine++;
  m_currentColumn = 1;
}

GameFileToken GameParserState::GetNextToken()
{
  char c = ' ';
  if (m_file.eof()) {
    return (m_lastToken = TOKEN_EOF);
  }

  while (isspace(c)) {
    ReadChar(c);
    if (m_file.eof()) {
      return (m_lastToken = TOKEN_EOF);
    }
    else if (c == '\n') {
      IncreaseLine();
    }
  }

  if (c == '{') {
    return (m_lastToken = TOKEN_LBRACE);
  }
  else if (c == '}') {
    return (m_lastToken = TOKEN_RBRACE);
  }
  else if (c == ',') {
    return (m_lastToken = TOKEN_COMMA);
  }
  else if (isdigit(c) || c == '-' || c == '+') {
    std::string buf;
    buf += c;
    ReadChar(c);

    while (!m_file.eof() && isdigit(c)) {
      buf += c;
      ReadChar(c);
    }

    if (m_file.eof()) {
      m_lastText = buf;
      return (m_lastToken = TOKEN_NUMBER);
    }

    if (c == '.') {
      buf += c;
      ReadChar(c);
      while (!m_file.eof() && isdigit(c)) {
        buf += c;
        ReadChar(c);
      }

      if (c == 'e' || c == 'E') {
        buf += c;
        ReadChar(c);
        if (c != '+' && c != '-' && !isdigit(c)) {
          OnParseError("Invalid Token +/-");
        }
        buf += c;
        ReadChar(c);
        while (!m_file.eof() && isdigit(c)) {
          buf += c;
          ReadChar(c);
        }
      }

      UnreadChar();
      m_lastText = buf;
      return (m_lastToken = TOKEN_NUMBER);
    }
    else if (c == '/') {
      buf += c;
      ReadChar(c);
      while (!m_file.eof() && isdigit(c)) {
        buf += c;
        ReadChar(c);
      }
      UnreadChar();
      m_lastText = buf;
      return (m_lastToken = TOKEN_NUMBER);
    }
    else if (c == 'e' || c == 'E') {
      buf += c;
      ReadChar(c);
      if (c != '+' && c != '-' && !isdigit(c)) {
        OnParseError("Invalid Token +/-");
      }
      buf += c;
      ReadChar(c);
      while (!m_file.eof() && isdigit(c)) {
        buf += c;
        ReadChar(c);
      }
      UnreadChar();
      m_lastText = buf;
      return (m_lastToken = TOKEN_NUMBER);
    }
    else {
      UnreadChar();
      m_lastText = buf;
      return (m_lastToken = TOKEN_NUMBER);
    }
  }
  else if (c == '.') {
    std::string buf;
    buf += c;
    ReadChar(c);

    while (!m_file.eof() && isdigit(c)) {
      buf += c;
      ReadChar(c);
    }
    UnreadChar();
    m_lastText = buf;
    return (m_lastToken = TOKEN_NUMBER);
  }

  else if (c == '"') {
    // We need to do a little magic here, since escaped quotes inside
    // the string are treated as quotes (not end-of-string)
    UnreadChar();
    char a;

    m_lastText = "";

    do {
      ReadChar(a);
      if (a == '\n') {
        IncreaseLine();
      }
    } while (!m_file.eof() && isspace(a));

    if (a == '\"') {
      bool lastslash = false;

      ReadChar(a);
      while (a != '\"' || lastslash) {
        if (m_file.eof() || !m_file.good()) {
          OnParseError("End of file encountered when reading string label");
        }
        if (lastslash && a == '"') {
          m_lastText += '"';
        }
        else if (lastslash) {
          m_lastText += '\\';
          m_lastText += a;
        }
        else if (a != '\\') {
          m_lastText += a;
        }

        lastslash = (a == '\\');
        ReadChar(a);
      }
    }
    else {
      do {
        m_lastText += a;
        ReadChar(a);
        if (m_file.eof()) {
          OnParseError("End of file encountered when reading string label");
        }
        if (a == '\n') {
          IncreaseLine();
        }
      } while (!isspace(a));
    }

    return (m_lastToken = TOKEN_TEXT);
  }

  m_lastText = "";
  while (!m_file.eof() && !isspace(c)) {
    m_lastText += c;
    ReadChar(c);
  }
  return (m_lastToken = TOKEN_SYMBOL);
}

class TableFilePlayer {
public:
  std::string m_name;
  Array<std::string> m_strategies;

  TableFilePlayer(const std::string &p_name) : m_name(p_name) {}
};

class TableFileGame {
public:
  std::string m_title, m_comment;
  std::vector<TableFilePlayer> m_players;

  size_t NumPlayers() const { return m_players.size(); }
  size_t NumStrategies(int p_player) const { return m_players[p_player - 1].m_strategies.size(); }
  std::string GetPlayer(int p_player) const { return m_players[p_player - 1].m_name; }
  std::string GetStrategy(int p_player, int p_strategy) const
  {
    return m_players[p_player - 1].m_strategies[p_strategy];
  }
};

void ReadPlayers(GameParserState &p_state, TableFileGame &p_data)
{
  p_state.ExpectNextToken(TOKEN_LBRACE, "'{'");
  while (p_state.GetNextToken() == TOKEN_TEXT) {
    p_data.m_players.push_back(p_state.GetLastText());
  }
  p_state.ExpectCurrentToken(TOKEN_RBRACE, "'}'");
}

void ReadStrategies(GameParserState &p_state, TableFileGame &p_data)
{
  p_state.ExpectNextToken(TOKEN_LBRACE, "'{'");
  auto token = p_state.GetNextToken();

  if (token == TOKEN_LBRACE) {
    // Nested list of strategy labels
    for (auto &player : p_data.m_players) {
      p_state.ExpectCurrentToken(TOKEN_LBRACE, "'{'");
      while (p_state.GetNextToken() == TOKEN_TEXT) {
        player.m_strategies.push_back(p_state.GetLastText());
      }
      p_state.ExpectCurrentToken(TOKEN_RBRACE, "'}'");
      p_state.GetNextToken();
    }
  }
  else {
    // List of number of strategies for each player, no labels
    for (auto &player : p_data.m_players) {
      p_state.ExpectCurrentToken(TOKEN_NUMBER, "number");
      for (int st = 1; st <= std::stoi(p_state.GetLastText()); st++) {
        player.m_strategies.push_back(std::to_string(st));
      }
      p_state.GetNextToken();
    }
  }
  p_state.ExpectCurrentToken(TOKEN_RBRACE, "'}'");
  p_state.GetNextToken();
}

void ParseNfgHeader(GameParserState &p_state, TableFileGame &p_data)
{
  if (p_state.GetNextToken() != TOKEN_SYMBOL || p_state.GetLastText() != "NFG") {
    p_state.OnParseError("Expecting NFG file type indicator");
  }
  if (p_state.GetNextToken() != TOKEN_NUMBER || p_state.GetLastText() != "1") {
    p_state.OnParseError("Accepting only NFG version 1");
  }
  if (p_state.GetNextToken() != TOKEN_SYMBOL ||
      (p_state.GetLastText() != "D" && p_state.GetLastText() != "R")) {
    p_state.OnParseError("Accepting only NFG D or R data type");
  }
  if (p_state.GetNextToken() != TOKEN_TEXT) {
    p_state.OnParseError("Game title missing");
  }
  p_data.m_title = p_state.GetLastText();

  ReadPlayers(p_state, p_data);
  ReadStrategies(p_state, p_data);

  if (p_state.GetCurrentToken() == TOKEN_TEXT) {
    // Read optional comment
    p_data.m_comment = p_state.GetLastText();
    p_state.GetNextToken();
  }
}

void ReadOutcomeList(GameParserState &p_parser, Game &p_nfg)
{
  auto players = p_nfg->GetPlayers();
  p_parser.GetNextToken();

  while (p_parser.GetCurrentToken() == TOKEN_LBRACE) {
    p_parser.ExpectNextToken(TOKEN_TEXT, "outcome name");
    auto outcome = p_nfg->NewOutcome();
    outcome->SetLabel(p_parser.GetLastText());
    p_parser.GetNextToken();

    for (auto player : players) {
      p_parser.ExpectCurrentToken(TOKEN_NUMBER, "numerical payoff");
      outcome->SetPayoff(player, Number(p_parser.GetLastText()));
      p_parser.AcceptNextToken(TOKEN_COMMA);
    }
    p_parser.ExpectCurrentToken(TOKEN_RBRACE, "'}'");
    p_parser.GetNextToken();
  }

  p_parser.ExpectCurrentToken(TOKEN_RBRACE, "'}'");
  p_parser.GetNextToken();
}

void ParseOutcomeBody(GameParserState &p_parser, Game &p_nfg)
{
  ReadOutcomeList(p_parser, p_nfg);

  for (StrategyProfileIterator iter({p_nfg}); !iter.AtEnd(); iter++) {
    p_parser.ExpectCurrentToken(TOKEN_NUMBER, "outcome index");
    if (int outcomeId = std::stoi(p_parser.GetLastText())) {
      (*iter)->SetOutcome(p_nfg->GetOutcome(outcomeId));
    }
    p_parser.GetNextToken();
  }
}

void ParsePayoffBody(GameParserState &p_parser, Game &p_nfg)
{
  for (StrategyProfileIterator iter({p_nfg}); !iter.AtEnd(); iter++) {
    for (auto player : p_nfg->GetPlayers()) {
      p_parser.ExpectCurrentToken(TOKEN_NUMBER, "numerical payoff");
      (*iter)->GetOutcome()->SetPayoff(player, Number(p_parser.GetLastText()));
      p_parser.GetNextToken();
    }
  }
}

Game BuildNfg(GameParserState &p_parser, TableFileGame &p_data)
{
  if (p_parser.GetCurrentToken() != TOKEN_LBRACE && p_parser.GetCurrentToken() != TOKEN_NUMBER) {
    p_parser.OnParseError("Expecting outcome or payoff");
  }

  Array<int> dim(p_data.NumPlayers());
  for (int pl = 1; pl <= dim.Length(); pl++) {
    dim[pl] = p_data.NumStrategies(pl);
  }

  // If this looks lke an outcome-based format, then don't create outcomes in advance
  Game nfg = NewTable(dim, p_parser.GetCurrentToken() == TOKEN_LBRACE);
  nfg->SetTitle(p_data.m_title);
  nfg->SetComment(p_data.m_comment);

  for (int pl = 1; pl <= dim.Length(); pl++) {
    nfg->GetPlayer(pl)->SetLabel(p_data.GetPlayer(pl));
    for (int st = 1; st <= dim[pl]; st++) {
      nfg->GetPlayer(pl)->GetStrategy(st)->SetLabel(p_data.GetStrategy(pl, st));
    }
  }

  if (p_parser.GetCurrentToken() == TOKEN_LBRACE) {
    ParseOutcomeBody(p_parser, nfg);
  }
  else {
    ParsePayoffBody(p_parser, nfg);
  }
  p_parser.ExpectCurrentToken(TOKEN_EOF, "end-of-file");
  return nfg;
}

//=========================================================================
//                  Temporary representation classes
//=========================================================================

class TreeData {
public:
  std::map<int, GameOutcome> m_outcomeMap;
  std::map<int, GameInfoset> m_chanceInfosetMap;
  List<std::map<int, GameInfoset>> m_infosetMap;

  TreeData() = default;
  ~TreeData() = default;
};

void ReadPlayers(GameParserState &p_state, Game &p_game, TreeData &p_treeData)
{
  p_state.ExpectNextToken(TOKEN_LBRACE, "'{'");
  while (p_state.GetNextToken() == TOKEN_TEXT) {
    p_game->NewPlayer()->SetLabel(p_state.GetLastText());
    p_treeData.m_infosetMap.push_back(std::map<int, GameInfoset>());
  }
  p_state.ExpectCurrentToken(TOKEN_RBRACE, "'}'");
}

//
// Precondition: Parser state should be expecting the integer index
//               of the outcome in a node entry
//
// Postcondition: Parser state is past the outcome entry and should be
//                pointing to the 'c', 'p', or 't' token starting the
//                next node declaration.
//
void ParseOutcome(GameParserState &p_state, Game &p_game, TreeData &p_treeData, GameNode &p_node)
{
  p_state.ExpectCurrentToken(TOKEN_NUMBER, "index of outcome");

  int outcomeId = atoi(p_state.GetLastText().c_str());
  p_state.GetNextToken();

  if (p_state.GetCurrentToken() == TOKEN_TEXT) {
    // This node entry contains information about the outcome
    GameOutcome outcome;
    if (p_treeData.m_outcomeMap.count(outcomeId)) {
      outcome = p_treeData.m_outcomeMap[outcomeId];
    }
    else {
      outcome = p_game->NewOutcome();
      p_treeData.m_outcomeMap[outcomeId] = outcome;
    }

    outcome->SetLabel(p_state.GetLastText());
    p_node->SetOutcome(outcome);

    p_state.ExpectNextToken(TOKEN_LBRACE, "'{'");

    p_state.GetNextToken();
    for (int pl = 1; pl <= p_game->NumPlayers(); pl++) {
      if (p_state.GetCurrentToken() == TOKEN_NUMBER) {
        outcome->SetPayoff(pl, Number(p_state.GetLastText()));
      }
      else {
        p_state.OnParseError("Payoffs should be numbers");
      }
      p_state.AcceptNextToken(TOKEN_COMMA);
    }

    p_state.ExpectCurrentToken(TOKEN_RBRACE, "'}'");
    p_state.GetNextToken();
  }
  else if (outcomeId != 0) {
    // The node entry does not contain information about the outcome.
    // This means the outcome better have been defined already;
    // if not, raise an error.
    if (p_treeData.m_outcomeMap.count(outcomeId)) {
      p_node->SetOutcome(p_treeData.m_outcomeMap[outcomeId]);
    }
    else {
      p_state.OnParseError("Outcome not defined");
    }
  }
}

void ParseNode(GameParserState &p_state, Game p_game, GameNode p_node, TreeData &p_treeData);

//
// Precondition: parser state is expecting the node label
//
// Postcondition: parser state is pointing at the 'c', 'p', or 't'
//                beginning the next node entry
//
void ParseChanceNode(GameParserState &p_state, Game &p_game, GameNode &p_node,
                     TreeData &p_treeData)
{
  p_state.ExpectNextToken(TOKEN_TEXT, "node label");
  p_node->SetLabel(p_state.GetLastText());

  p_state.ExpectNextToken(TOKEN_NUMBER, "infoset id");
  int infosetId = atoi(p_state.GetLastText().c_str());
  GameInfoset infoset;
  if (p_treeData.m_chanceInfosetMap.count(infosetId)) {
    infoset = p_treeData.m_chanceInfosetMap[infosetId];
  }

  p_state.GetNextToken();

  if (p_state.GetCurrentToken() == TOKEN_TEXT) {
    // Information set data is specified
    List<std::string> actions, probs;
    std::string label = p_state.GetLastText();

    if (p_state.GetNextToken() != TOKEN_LBRACE) {
      p_state.OnParseError("Expecting '{' before information set data");
    }
    p_state.GetNextToken();
    do {
      p_state.ExpectCurrentToken(TOKEN_TEXT, "action label");
      actions.push_back(p_state.GetLastText());
      p_state.ExpectNextToken(TOKEN_NUMBER, "action probability");
      probs.push_back(p_state.GetLastText());
      p_state.GetNextToken();
    } while (p_state.GetCurrentToken() != TOKEN_RBRACE);
    p_state.GetNextToken();

    if (!infoset) {
      infoset = p_node->AppendMove(p_game->GetChance(), actions.Length());
      p_treeData.m_chanceInfosetMap[infosetId] = infoset;
      infoset->SetLabel(label);
      for (int act = 1; act <= actions.Length(); act++) {
        infoset->GetAction(act)->SetLabel(actions[act]);
      }
      Array<Number> prob_numbers(probs.size());
      for (int act = 1; act <= actions.Length(); act++) {
        prob_numbers[act] = Number(probs[act]);
      }
      p_game->SetChanceProbs(infoset, prob_numbers);
    }
    else {
      // TODO: Verify actions match up to previous specifications
      p_node->AppendMove(infoset);
    }
  }
  else if (infoset) {
    p_node->AppendMove(infoset);
  }
  else {
    // Referencing an undefined infoset is an error
    p_state.OnParseError("Referencing an undefined infoset");
  }

  ParseOutcome(p_state, p_game, p_treeData, p_node);

  for (int i = 1; i <= p_node->NumChildren(); i++) {
    ParseNode(p_state, p_game, p_node->GetChild(i), p_treeData);
  }
}

void ParsePersonalNode(GameParserState &p_state, Game p_game, GameNode p_node,
                       TreeData &p_treeData)
{
  p_state.ExpectNextToken(TOKEN_TEXT, "node label");
  p_node->SetLabel(p_state.GetLastText());

  p_state.ExpectNextToken(TOKEN_NUMBER, "player id");
  int playerId = atoi(p_state.GetLastText().c_str());
  // This will throw an exception if the player ID is not valid
  GamePlayer player = p_game->GetPlayer(playerId);
  std::map<int, GameInfoset> &infosetMap = p_treeData.m_infosetMap[playerId];

  p_state.ExpectNextToken(TOKEN_NUMBER, "infoset id");
  int infosetId = atoi(p_state.GetLastText().c_str());
  GameInfoset infoset;
  if (infosetMap.count(infosetId)) {
    infoset = infosetMap[infosetId];
  }

  p_state.GetNextToken();
  if (p_state.GetCurrentToken() == TOKEN_TEXT) {
    // Information set data is specified
    List<std::string> actions;
    std::string label = p_state.GetLastText();

    p_state.ExpectNextToken(TOKEN_LBRACE, "'{'");
    p_state.GetNextToken();
    do {
      p_state.ExpectCurrentToken(TOKEN_TEXT, "action label");
      actions.push_back(p_state.GetLastText());
      p_state.GetNextToken();
    } while (p_state.GetCurrentToken() != TOKEN_RBRACE);
    p_state.GetNextToken();

    if (!infoset) {
      infoset = p_node->AppendMove(player, actions.Length());
      infosetMap[infosetId] = infoset;
      infoset->SetLabel(label);
      for (int act = 1; act <= actions.Length(); act++) {
        infoset->GetAction(act)->SetLabel(actions[act]);
      }
    }
    else {
      // TODO: Verify actions match up to previous specifications
      p_node->AppendMove(infoset);
    }
  }
  else if (infoset) {
    p_node->AppendMove(infoset);
  }
  else {
    // Referencing an undefined infoset is an error
    p_state.OnParseError("Referencing an undefined infoset");
  }

  ParseOutcome(p_state, p_game, p_treeData, p_node);

  for (int i = 1; i <= p_node->NumChildren(); i++) {
    ParseNode(p_state, p_game, p_node->GetChild(i), p_treeData);
  }
}

void ParseTerminalNode(GameParserState &p_state, Game p_game, GameNode p_node,
                       TreeData &p_treeData)
{
  p_state.ExpectNextToken(TOKEN_TEXT, "node label");
  p_node->SetLabel(p_state.GetLastText());
  p_state.GetNextToken();
  ParseOutcome(p_state, p_game, p_treeData, p_node);
}

void ParseNode(GameParserState &p_state, Game p_game, GameNode p_node, TreeData &p_treeData)
{
  if (p_state.GetLastText() == "c") {
    ParseChanceNode(p_state, p_game, p_node, p_treeData);
  }
  else if (p_state.GetLastText() == "p") {
    ParsePersonalNode(p_state, p_game, p_node, p_treeData);
  }
  else if (p_state.GetLastText() == "t") {
    ParseTerminalNode(p_state, p_game, p_node, p_treeData);
  }
  else {
    p_state.OnParseError("Invalid type of node");
  }
}

} // end of anonymous namespace

#include "core/tinyxml.h"

namespace Gambit {

class GameXMLSavefile {
private:
  TiXmlDocument doc;

public:
  explicit GameXMLSavefile(const std::string &p_xml);
  ~GameXMLSavefile() = default;

  Game GetGame() const;
};

GameXMLSavefile::GameXMLSavefile(const std::string &p_xml)
{
  doc.Parse(p_xml.c_str());
  if (doc.Error()) {
    throw InvalidFileException("Not a valid XML document");
  }
}

Game GameXMLSavefile::GetGame() const
{
  const TiXmlNode *docroot = doc.FirstChild("gambit:document");
  if (!docroot) {
    throw InvalidFileException("Not a Gambit game savefile document");
  }

  const TiXmlNode *game = docroot->FirstChild("game");
  if (!game) {
    throw InvalidFileException("No game representation found in document");
  }

  const TiXmlNode *efgfile = game->FirstChild("efgfile");
  if (efgfile) {
    std::istringstream s(efgfile->FirstChild()->Value());
    return ReadGame(s);
  }

  const TiXmlNode *nfgfile = game->FirstChild("nfgfile");
  if (nfgfile) {
    std::istringstream s(nfgfile->FirstChild()->Value());
    return ReadGame(s);
  }

  throw InvalidFileException("No game representation found in document");
}

Game ReadEfgFile(std::istream &p_stream)
{
  GameParserState parser(p_stream);

  if (parser.GetNextToken() != TOKEN_SYMBOL || parser.GetLastText() != "EFG") {
    parser.OnParseError("Expecting EFG file type indicator");
  }
  if (parser.GetNextToken() != TOKEN_NUMBER || parser.GetLastText() != "2") {
    parser.OnParseError("Accepting only EFG version 2");
  }
  if (parser.GetNextToken() != TOKEN_SYMBOL ||
      (parser.GetLastText() != "D" && parser.GetLastText() != "R")) {
    parser.OnParseError("Accepting only EFG R or D data type");
  }
  if (parser.GetNextToken() != TOKEN_TEXT) {
    parser.OnParseError("Game title missing");
  }

  TreeData treeData;
  Game game = NewTree();
  dynamic_cast<GameTreeRep &>(*game).SetCanonicalization(false);
  game->SetTitle(parser.GetLastText());
  ReadPlayers(parser, game, treeData);
  if (parser.GetNextToken() == TOKEN_TEXT) {
    // Read optional comment
    game->SetComment(parser.GetLastText());
    parser.GetNextToken();
  }
  ParseNode(parser, game, game->GetRoot(), treeData);
  dynamic_cast<GameTreeRep &>(*game).SetCanonicalization(true);
  return game;
}

Game ReadNfgFile(std::istream &p_stream)
{
  GameParserState parser(p_stream);
  TableFileGame data;
  ParseNfgHeader(parser, data);
  return BuildNfg(parser, data);
}

Game ReadGbtFile(std::istream &p_stream)
{
  std::stringstream buffer;
  buffer << p_stream.rdbuf();
  return GameXMLSavefile(buffer.str()).GetGame();
}

Game ReadGame(std::istream &p_file)
{
  std::stringstream buffer;
  buffer << p_file.rdbuf();
  if (buffer.str().empty()) {
    throw InvalidFileException("Empty file or string provided");
  }
  try {
    return ReadGbtFile(buffer);
  }
  catch (InvalidFileException &) {
    buffer.seekg(0, std::ios::beg);
  }

  GameParserState parser(buffer);
  try {
    if (parser.GetNextToken() != TOKEN_SYMBOL) {
      parser.OnParseError("Expecting file type");
    }
    buffer.seekg(0, std::ios::beg);
    if (parser.GetLastText() == "NFG") {
      return ReadNfgFile(buffer);
    }
    else if (parser.GetLastText() == "EFG") {
      return ReadEfgFile(buffer);
    }
    else if (parser.GetLastText() == "#AGG") {
      return ReadAggFile(buffer);
    }
    else if (parser.GetLastText() == "#BAGG") {
      return ReadBaggFile(buffer);
    }
    else {
      throw InvalidFileException("Unrecognized file format");
    }
  }
  catch (std::exception &ex) {
    throw InvalidFileException(ex.what());
  }
}

} // end namespace Gambit
