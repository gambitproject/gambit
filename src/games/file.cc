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
#include <map>

#include "gambit.h"
// for explicit access to turning off canonicalization
#include "gametree.h"
  

namespace {
// This anonymous namespace encapsulates the file-parsing code

using namespace Gambit;

typedef enum {
  TOKEN_NUMBER = 0, TOKEN_TEXT = 1, TOKEN_SYMBOL = 2,
  TOKEN_LBRACE = 3, TOKEN_RBRACE = 4, TOKEN_COMMA = 5, TOKEN_EOF = 6
} GameFileToken;

//!
//! This parser class implements the semantics of Gambit savefiles,
//! including the nonsignificance of whitespace and the possibility of
//! escaped-quotes within text labels.
//!
class GameParserState {
private:
  std::istream &m_file;

  int m_currentLine;
  int m_currentColumn;
  GameFileToken m_lastToken;
  std::string m_lastText;

  void ReadChar(char& c);
  void UnreadChar();
  void IncreaseLine();

public:
  explicit GameParserState(std::istream &p_file) :
    m_file(p_file), m_currentLine(1), m_currentColumn(1) { }

  GameFileToken GetNextToken();
  GameFileToken GetCurrentToken() const { return m_lastToken; }
  int GetCurrentLine() const { return m_currentLine; }
  int GetCurrentColumn() const { return m_currentColumn; }
  std::string CreateLineMsg(const std::string &msg) const;
  const std::string &GetLastText() const { return m_lastText; }
};

void GameParserState::ReadChar(char& c)
{
  m_file.get(c);
  m_currentColumn++;
}

void GameParserState::UnreadChar()
{
  m_file.unget();
  m_currentColumn--;
}

void GameParserState::IncreaseLine(){
  m_currentLine++;
  // Reset column
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
    if (!m_file.good()) {
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

    while (!m_file.eof() && m_file.good() && isdigit(c)) {
      buf += c;
      ReadChar(c);
    }

    if (m_file.eof() || !m_file.good()) {
      m_lastText = buf;
      return (m_lastToken = TOKEN_NUMBER);
    }

    if (c == '.') {
      buf += c;
      ReadChar(c);
      while (isdigit(c)) {
        buf += c;
        ReadChar(c);
      }

      if (c == 'e' || c == 'E') {
        buf += c;
        ReadChar(c);
        if (c != '+' && c != '-' && !isdigit(c)) {
          throw InvalidFileException(CreateLineMsg("Invalid Token +/-"));
        }
        buf += c;
        ReadChar(c);
        while (isdigit(c)) {
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
      while (isdigit(c)) {
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
        throw InvalidFileException(CreateLineMsg("Invalid Token +/-"));
      }
      buf += c;
      ReadChar(c);
      while (isdigit(c)) {
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

    while (isdigit(c)) {
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

    do  {
      ReadChar(a);
      if (a == '\n') {
        IncreaseLine();
      }
    } while (isspace(a));

    if (a == '\"')  {
      bool lastslash = false;

      ReadChar(a);
      while  (a != '\"' || lastslash)  {
	if (m_file.eof() || !m_file.good())  {
	  throw InvalidFileException(CreateLineMsg("End of file encountered when reading string label"));
	}
        if (lastslash && a == '"') {
          m_lastText += '"';
	}
        else if (lastslash)  {
          m_lastText += '\\';
          m_lastText += a;
        }
        else if (a != '\\')
          m_lastText += a;

        lastslash = (a == '\\');
        ReadChar(a);
      }
    }
    else  {
      do  {
      	m_lastText += a;
        ReadChar(a);
	if (m_file.eof() || !m_file.good())  {
	  throw InvalidFileException(CreateLineMsg("End of file encountered when reading string label"));
	}
        if (a == '\n') {
          IncreaseLine();
        }
      }  while (!isspace(a));
    }

    return (m_lastToken = TOKEN_TEXT);
  }

  m_lastText = "";
  while (!isspace(c) && !m_file.eof()) {
    m_lastText += c;
    ReadChar(c);
  }
  return (m_lastToken = TOKEN_SYMBOL);
}

std::string GameParserState::CreateLineMsg(const std::string &msg) const
{
  std::stringstream stream;
  stream << "line " << m_currentLine << ":" << m_currentColumn << ": " << msg;
  return stream.str();
}

class TableFilePlayer {
public:
  std::string m_name;
  Array<std::string> m_strategies;
  TableFilePlayer *m_next;

  TableFilePlayer();
};

TableFilePlayer::TableFilePlayer()
  : m_next(nullptr)
{ }

class TableFileGame {
public:
  std::string m_title, m_comment;
  TableFilePlayer *m_firstPlayer, *m_lastPlayer;
  int m_numPlayers;

  TableFileGame() : m_firstPlayer(nullptr), m_lastPlayer(nullptr), m_numPlayers(0) { }
  ~TableFileGame();

  void AddPlayer(const std::string &);
  int NumPlayers() const { return m_numPlayers; }
  int NumStrategies(int p_player) const;
  std::string GetPlayer(int p_player) const;
  std::string GetStrategy(int p_player, int p_strategy) const;
};

TableFileGame::~TableFileGame()
{
  if (m_firstPlayer) {
    TableFilePlayer *player = m_firstPlayer;
    while (player) {
      TableFilePlayer *nextPlayer = player->m_next;
      delete player;
      player = nextPlayer;
    }
  }
}

void TableFileGame::AddPlayer(const std::string &p_name)
{
  auto *player = new TableFilePlayer;
  player->m_name = p_name;

  if (m_firstPlayer) {
    m_lastPlayer->m_next = player;
    m_lastPlayer = player;
  }
  else {
    m_firstPlayer = player;
    m_lastPlayer = player;
  }
  m_numPlayers++;
}

int TableFileGame::NumStrategies(int p_player) const
{
  TableFilePlayer *player = m_firstPlayer;
  int pl = 1;

  while (player && pl < p_player) {
    player = player->m_next;
    pl++;
  }

  if (!player) {
    return 0;
  }
  else {
    return player->m_strategies.Length();
  }
}

std::string TableFileGame::GetPlayer(int p_player) const
{
  TableFilePlayer *player = m_firstPlayer;
  int pl = 1;

  while (player && pl < p_player) {
    player = player->m_next;
    pl++;
  }

  if (!player) {
    return "";
  }
  else {
    return player->m_name;
  }
}

std::string TableFileGame::GetStrategy(int p_player, int p_strategy) const
{
  TableFilePlayer *player = m_firstPlayer;
  int pl = 1;

  while (player && pl < p_player) {
    player = player->m_next;
    pl++;
  }

  if (!player) {
    return "";
  }
  else {
    return player->m_strategies[p_strategy];
  }
}

void ReadPlayers(GameParserState &p_state, TableFileGame &p_data)
{
  if (p_state.GetNextToken() != TOKEN_LBRACE) {
    throw InvalidFileException(
      p_state.CreateLineMsg("Expecting '{' before players"));
  }

  while (p_state.GetNextToken() == TOKEN_TEXT) {
    p_data.AddPlayer(p_state.GetLastText());
  }

  if (p_state.GetCurrentToken() != TOKEN_RBRACE) {
    throw InvalidFileException(
      p_state.CreateLineMsg("Expecting '}' after players"));
  }

  p_state.GetNextToken();
}

void ReadStrategies(GameParserState &p_state, TableFileGame &p_data)
{
  if (p_state.GetCurrentToken() != TOKEN_LBRACE) {
    throw InvalidFileException(
      p_state.CreateLineMsg("Expecting '{' before strategies"));
  }
  p_state.GetNextToken();

  if (p_state.GetCurrentToken() == TOKEN_LBRACE) {
    TableFilePlayer *player = p_data.m_firstPlayer;

    while (p_state.GetCurrentToken() == TOKEN_LBRACE) {
      if (!player) {
        throw InvalidFileException(p_state.CreateLineMsg(
          "Not enough players for number of strategy entries"));
      }

      while (p_state.GetNextToken() == TOKEN_TEXT) {
        player->m_strategies.push_back(p_state.GetLastText());
      }

      if (p_state.GetCurrentToken() != TOKEN_RBRACE) {
        throw InvalidFileException(
          p_state.CreateLineMsg("Expecting '}' after player strategy"));
      }

      p_state.GetNextToken();
      player = player->m_next;
    }

    if (player) {
      throw InvalidFileException(
        p_state.CreateLineMsg("Players with undefined strategies"));
    }

    if (p_state.GetCurrentToken() != TOKEN_RBRACE) {
      throw InvalidFileException(
        p_state.CreateLineMsg("Expecting '}' after strategies"));
    }

    p_state.GetNextToken();
  }
  else if (p_state.GetCurrentToken() == TOKEN_NUMBER) {
    TableFilePlayer *player = p_data.m_firstPlayer;

    while (p_state.GetCurrentToken() == TOKEN_NUMBER) {
      if (!player) {
        throw InvalidFileException(p_state.CreateLineMsg(
          "Not enough players for number of strategy entries"));
      }

      for (int st = 1; st <= atoi(p_state.GetLastText().c_str()); st++) {
        player->m_strategies.push_back(lexical_cast<std::string>(st));
      }

      p_state.GetNextToken();
      player = player->m_next;
    }

    if (p_state.GetCurrentToken() != TOKEN_RBRACE) {
      throw InvalidFileException(
        p_state.CreateLineMsg("Expecting '}' after strategies"));
    }

    if (player) {
      throw InvalidFileException(
        p_state.CreateLineMsg("Players with strategies undefined"));
    }

    p_state.GetNextToken();
  }
  else {
    throw InvalidFileException(
      p_state.CreateLineMsg("Unrecognizable strategies format"));
  }
}

void ParseNfgHeader(GameParserState &p_state, TableFileGame &p_data)
{
  if (p_state.GetNextToken() != TOKEN_NUMBER ||
      p_state.GetLastText() != "1") {
    throw InvalidFileException(
      p_state.CreateLineMsg("Accepting only NFG version 1"));
  }

  if (p_state.GetNextToken() != TOKEN_SYMBOL ||
      (p_state.GetLastText() != "D" && p_state.GetLastText() != "R")) {
    throw InvalidFileException(
      p_state.CreateLineMsg("Accepting only NFG D or R data type"));
  }
  if (p_state.GetNextToken() != TOKEN_TEXT) {
    throw InvalidFileException(
      p_state.CreateLineMsg("Game title missing"));
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
  if (p_parser.GetNextToken() == TOKEN_RBRACE) {
    // Special case: empty outcome list
    p_parser.GetNextToken();
    return;
  }

  if (p_parser.GetCurrentToken() != TOKEN_LBRACE) {
    throw InvalidFileException(
      p_parser.CreateLineMsg("Expecting '{' before outcome"));
  }

  int nOutcomes = 0;

  while (p_parser.GetCurrentToken() == TOKEN_LBRACE) {
    nOutcomes++;
    int pl = 1;

    if (p_parser.GetNextToken() != TOKEN_TEXT) {
      throw InvalidFileException(
        p_parser.CreateLineMsg("Expecting string for outcome"));
    }

    GameOutcome outcome;
    try {
      outcome = p_nfg->GetOutcome(nOutcomes);
    }
    catch (IndexException &) {
      // It might happen that the file contains more outcomes than
      // contingencies.  If so, just create them on the fly.
      outcome = p_nfg->NewOutcome();
    }
    outcome->SetLabel(p_parser.GetLastText());
    p_parser.GetNextToken();

    try {
      while (p_parser.GetCurrentToken() == TOKEN_NUMBER) {
        outcome->SetPayoff(pl++, Number(p_parser.GetLastText()));
        if (p_parser.GetNextToken() == TOKEN_COMMA) {
            p_parser.GetNextToken();
        }
      }
    }
    catch (IndexException &) {
      // This would be triggered by too many payoffs
      throw InvalidFileException(
        p_parser.CreateLineMsg("Exceeded number of players in outcome"));
    }

    if (pl <= p_nfg->NumPlayers() ||
     p_parser.GetCurrentToken() != TOKEN_RBRACE) {
      throw InvalidFileException(
        p_parser.CreateLineMsg("Insufficient number of players in outcome"));
    }

    p_parser.GetNextToken();
  }

  if (p_parser.GetCurrentToken() != TOKEN_RBRACE) {
    throw InvalidFileException(
        p_parser.CreateLineMsg("Expecting '}' after outcome"));
  }
  p_parser.GetNextToken();
}

void ParseOutcomeBody(GameParserState &p_parser, Game &p_nfg)
{
  ReadOutcomeList(p_parser, p_nfg);

  StrategySupportProfile profile(p_nfg);
  StrategyProfileIterator iter(profile);

  while (p_parser.GetCurrentToken() != TOKEN_EOF) {
    if (p_parser.GetCurrentToken() != TOKEN_NUMBER) {
      throw InvalidFileException(
        p_parser.CreateLineMsg("Expecting outcome index"));
    }

    int outcomeId = atoi(p_parser.GetLastText().c_str());
    if (outcomeId > 0)  {
      (*iter)->SetOutcome(p_nfg->GetOutcome(outcomeId));
    }
    else {
      (*iter)->SetOutcome(nullptr);
    }
    p_parser.GetNextToken();
    iter++;
  }
}

void ParsePayoffBody(GameParserState &p_parser, Game &p_nfg)
{
  StrategySupportProfile profile(p_nfg);
  StrategyProfileIterator iter(profile);
  int pl = 1;

  while (p_parser.GetCurrentToken() != TOKEN_EOF) {
    if (p_parser.GetCurrentToken() == TOKEN_NUMBER) {
      (*iter)->GetOutcome()->SetPayoff(pl, Number(p_parser.GetLastText()));
    }
    else {
      throw InvalidFileException(p_parser.CreateLineMsg("Expecting payoff"));
    }

    if (++pl > p_nfg->NumPlayers()) {
      iter++;
      pl = 1;
    }
    p_parser.GetNextToken();
  }
}

Game BuildNfg(GameParserState &p_parser, TableFileGame &p_data)
{
  Array<int> dim(p_data.NumPlayers());
  for (int pl = 1; pl <= dim.Length(); pl++) {
    dim[pl] = p_data.NumStrategies(pl);
  }

  Game nfg = NewTable(dim);
  nfg->SetTitle(p_data.m_title);
  nfg->SetComment(p_data.m_comment);

  for (int pl = 1; pl <= dim.Length(); pl++) {
    nfg->GetPlayer(pl)->SetLabel(p_data.GetPlayer(pl));
    for (int st = 1; st <= dim[pl]; st++) {
      nfg->GetPlayer(pl)->GetStrategy(st)->SetLabel(p_data.GetStrategy(pl,st));
    }
  }

  if (p_parser.GetCurrentToken() == TOKEN_LBRACE) {
    ParseOutcomeBody(p_parser, nfg);
  }
  else if (p_parser.GetCurrentToken() == TOKEN_NUMBER) {
    ParsePayoffBody(p_parser, nfg);
  }
  else {
    throw InvalidFileException(
      p_parser.CreateLineMsg("Expecting outcome or payoff"));
  }

  return nfg;
}



//=========================================================================
//                  Temporary representation classes
//=========================================================================

class TreeData {
public:
  std::map<int, GameOutcome> m_outcomeMap;
  std::map<int, GameInfoset> m_chanceInfosetMap;
  List<std::map<int, GameInfoset> > m_infosetMap;

  TreeData()  = default;
  ~TreeData() = default;
};

void ReadPlayers(GameParserState &p_state,
                 Game &p_game, TreeData &p_treeData)
{
  if (p_state.GetNextToken() != TOKEN_LBRACE) {
    throw InvalidFileException(
        p_state.CreateLineMsg("Expecting '{' before players"));
  }

  while (p_state.GetNextToken() == TOKEN_TEXT) {
    p_game->NewPlayer()->SetLabel(p_state.GetLastText());
    p_treeData.m_infosetMap.push_back(std::map<int, GameInfoset>());
  }

  if (p_state.GetCurrentToken() != TOKEN_RBRACE) {
    throw InvalidFileException(
      p_state.CreateLineMsg("Expecting '}' after players"));
  }
}

//
// Precondition: Parser state should be expecting the integer index
//               of the outcome in a node entry
//
// Postcondition: Parser state is past the outcome entry and should be
//                pointing to the 'c', 'p', or 't' token starting the
//                next node declaration.
//
void ParseOutcome(GameParserState &p_state,
                  Game &p_game, TreeData &p_treeData,
                  GameNode &p_node)
{
  if (p_state.GetCurrentToken() != TOKEN_NUMBER) {
    throw InvalidFileException(
      p_state.CreateLineMsg("Expecting index of outcome"));
  }

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

    if (p_state.GetNextToken() != TOKEN_LBRACE) {
      throw InvalidFileException(
        p_state.CreateLineMsg("Expecting '{' before outcome"));
    }
    p_state.GetNextToken();

    for (int pl = 1; pl <= p_game->NumPlayers(); pl++) {
      if (p_state.GetCurrentToken() == TOKEN_NUMBER) {
        outcome->SetPayoff(pl, Number(p_state.GetLastText()));
      }
      else {
        throw InvalidFileException(
          p_state.CreateLineMsg("Payoffs should be numbers"));
      }

      // Commas are optional between payoffs
      if (p_state.GetNextToken() == TOKEN_COMMA) {
        p_state.GetNextToken();
      }
    }

    if (p_state.GetCurrentToken() != TOKEN_RBRACE) {
      throw InvalidFileException(
        p_state.CreateLineMsg("Expecting '}' after outcome"));
    }
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
      throw InvalidFileException(
        p_state.CreateLineMsg("Outcome not defined"));
    }
  }
}

void ParseNode(GameParserState &p_state, Game p_game, GameNode p_node,
	       TreeData &p_treeData);

//
// Precondition: parser state is expecting the node label
//
// Postcondition: parser state is pointing at the 'c', 'p', or 't'
//                beginning the next node entry
//
void ParseChanceNode(GameParserState &p_state,
                     Game &p_game, GameNode &p_node, TreeData &p_treeData)
{
  if (p_state.GetNextToken() != TOKEN_TEXT) {
    throw InvalidFileException(p_state.CreateLineMsg("Expecting label"));
  }
  p_node->SetLabel(p_state.GetLastText());

  if (p_state.GetNextToken() != TOKEN_NUMBER) {
    throw InvalidFileException(p_state.CreateLineMsg("Expecting infoset id"));
  }

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
      throw InvalidFileException(
        p_state.CreateLineMsg("Expecting '{' before information set data"));
    }
    p_state.GetNextToken();
    do {
      if (p_state.GetCurrentToken() != TOKEN_TEXT) {
        throw InvalidFileException(p_state.CreateLineMsg("Expecting action"));
      }
      actions.push_back(p_state.GetLastText());

      p_state.GetNextToken();

      if (p_state.GetCurrentToken() == TOKEN_NUMBER) {
        probs.push_back(p_state.GetLastText());
      }
      else {
        throw InvalidFileException(p_state.CreateLineMsg("Expecting probability"));
      }

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
    throw InvalidFileException(
      p_state.CreateLineMsg("Referencing an undefined infoset"));
  }

  ParseOutcome(p_state, p_game, p_treeData, p_node);

  for (int i = 1; i <= p_node->NumChildren(); i++) {
    ParseNode(p_state, p_game, p_node->GetChild(i), p_treeData);
  }
}

void ParsePersonalNode(GameParserState &p_state,
		       Game p_game, GameNode p_node, TreeData &p_treeData)
{
  if (p_state.GetNextToken() != TOKEN_TEXT) {
    throw InvalidFileException(p_state.CreateLineMsg("Expecting label"));
  }
  p_node->SetLabel(p_state.GetLastText());

  if (p_state.GetNextToken() != TOKEN_NUMBER) {
    throw InvalidFileException(p_state.CreateLineMsg("Expecting player id"));
  }
  int playerId = atoi(p_state.GetLastText().c_str());
  // This will throw an exception if the player ID is not valid
  GamePlayer player = p_game->GetPlayer(playerId);
  std::map<int, GameInfoset> &infosetMap = p_treeData.m_infosetMap[playerId];

  if (p_state.GetNextToken() != TOKEN_NUMBER) {
    throw InvalidFileException(p_state.CreateLineMsg("Expecting infoset id"));
  }
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

    if (p_state.GetNextToken() != TOKEN_LBRACE) {
      throw InvalidFileException(
        p_state.CreateLineMsg("Expecting '{' before information set data"));
    }
    p_state.GetNextToken();
    do {
      if (p_state.GetCurrentToken() != TOKEN_TEXT) {
        throw InvalidFileException(
          p_state.CreateLineMsg("Expecting action"));
      }
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
    throw InvalidFileException(
      p_state.CreateLineMsg("Referencing an undefined infoset"));
  }

  ParseOutcome(p_state, p_game, p_treeData, p_node);

  for (int i = 1; i <= p_node->NumChildren(); i++) {
    ParseNode(p_state, p_game, p_node->GetChild(i), p_treeData);
  }
}

void ParseTerminalNode(GameParserState &p_state,
		       Game p_game, GameNode p_node, TreeData &p_treeData)
{
  if (p_state.GetNextToken() != TOKEN_TEXT) {
    throw InvalidFileException(p_state.CreateLineMsg("Expecting label"));
  }

  p_node->SetLabel(p_state.GetLastText());

  p_state.GetNextToken();
  ParseOutcome(p_state, p_game, p_treeData, p_node);
}

void ParseNode(GameParserState &p_state, Game p_game, GameNode p_node,
	       TreeData &p_treeData)
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
    throw InvalidFileException(p_state.CreateLineMsg("Invalid type of node"));
  }
}

void ParseEfg(GameParserState &p_state, Game p_game, TreeData &p_treeData)
{
  if (p_state.GetNextToken() != TOKEN_NUMBER ||
      p_state.GetLastText() != "2") {
    throw InvalidFileException(
      p_state.CreateLineMsg("Accepting only EFG version 2"));
  }

  if (p_state.GetNextToken() != TOKEN_SYMBOL ||
      (p_state.GetLastText() != "D" && p_state.GetLastText() != "R")) {
    throw InvalidFileException(
      p_state.CreateLineMsg("Accepting only EFG R or D data type"));
  }
  if (p_state.GetNextToken() != TOKEN_TEXT) {
    throw InvalidFileException(p_state.CreateLineMsg("Game title missing"));
  }
  p_game->SetTitle(p_state.GetLastText());

  ReadPlayers(p_state, p_game, p_treeData);

  if (p_state.GetNextToken() == TOKEN_TEXT) {
    // Read optional comment
    p_game->SetComment(p_state.GetLastText());
    p_state.GetNextToken();
  }

  ParseNode(p_state, p_game, p_game->GetRoot(), p_treeData);
}

} // end of anonymous namespace


#include "core/tinyxml.h"

namespace Gambit {

class GameXMLSavefile {
private:
  TiXmlDocument doc;

public:
  explicit GameXMLSavefile(const std::string &p_xml);
  ~GameXMLSavefile()  = default;

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

//=========================================================================
//    ReadGame: Global visible function to read an .efg or .nfg file
//=========================================================================

Game ReadGame(std::istream &p_file)
{
  std::stringstream buffer;
  buffer << p_file.rdbuf();
  try {
    GameXMLSavefile doc(buffer.str());
    return doc.GetGame();
  }
  catch (InvalidFileException &) {
    buffer.seekg(0, std::ios::beg);
  }

  GameParserState parser(buffer);
  try {
    if (parser.GetNextToken() != TOKEN_SYMBOL) {
      throw InvalidFileException(parser.CreateLineMsg("Expecting file type"));
    }

    if (parser.GetLastText() == "NFG") {
      TableFileGame data;
      ParseNfgHeader(parser, data);
      return BuildNfg(parser, data);
    }
    else if (parser.GetLastText() == "EFG") {
      TreeData treeData;
      Game game = NewTree();
      dynamic_cast<GameTreeRep &>(*game).SetCanonicalization(false);
      ParseEfg(parser, game, treeData);
      dynamic_cast<GameTreeRep &>(*game).SetCanonicalization(true);
      return game;
    }
    else if (parser.GetLastText() == "#AGG") {
      return GameAGGRep::ReadAggFile(buffer);
    }
    else if (parser.GetLastText() == "#BAGG") {
      return GameBAGGRep::ReadBaggFile(buffer);
    }
    else {
      throw InvalidFileException("Tokens 'EFG' or 'NFG' or '#AGG' or '#BAGG' expected at start of file");
    }
  }
  catch (std::exception &ex) {
    throw InvalidFileException(ex.what());
  }
}

} // end namespace Gambit
