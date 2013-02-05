//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
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
#include <sstream>
#include <map>

#include "libgambit.h"

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
  GameFileToken m_lastToken;
  std::string m_lastText;

public:
  GameParserState(std::istream &p_file) : 
    m_file(p_file), m_currentLine(1) { }

  GameFileToken GetNextToken(void);
  GameFileToken GetCurrentToken(void) const { return m_lastToken; }
  int GetCurrentLine(void) const { return m_currentLine; }
  const std::string &GetLastText(void) const { return m_lastText; }
};  

GameFileToken GameParserState::GetNextToken(void)
{
  char c = ' '; 
  if (m_file.eof()) {
    return (m_lastToken = TOKEN_EOF);
  }

  while (isspace(c)) {
    m_file.get(c);
    if (!m_file.good()) {
      return (m_lastToken = TOKEN_EOF);
    } 
    else if (c == '\n') {
      m_currentLine++;
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
    m_file.get(c);

    while (!m_file.eof() && m_file.good() && isdigit(c)) {
      buf += c;
      m_file.get(c);
    }
    
    if (m_file.eof() || !m_file.good()) {
      m_lastText = buf;
      return (m_lastToken = TOKEN_NUMBER);
    }      

    if (c == '.') {
      buf += c;
      m_file.get(c);
      while (isdigit(c)) {
      	buf += c;
      	m_file.get(c);
      }

      if (c == 'e' || c == 'E') {
	buf += c;
	m_file.get(c);
	if (c == '+' && c == '-' && !isdigit(c)) {
	  throw InvalidFileException();
	}
	buf += c;
	m_file.get(c);
	while (isdigit(c)) {
	  buf += c;
	  m_file.get(c);
	}
      }

      m_file.unget();
      m_lastText = buf;

      return (m_lastToken = TOKEN_NUMBER);
    }
    else if (c == '/') {
      buf += c;
      m_file.get(c);
      while (isdigit(c)) {
      	buf += c;
      	m_file.get(c);
      }
      m_file.unget();
      m_lastText = buf;
      return (m_lastToken = TOKEN_NUMBER);
    }
    else if (c == 'e' || c == 'E') {
      buf += c;
      m_file.get(c);
      if (c == '+' && c == '-' && !isdigit(c)) {
	throw InvalidFileException();
      }
      buf += c;
      m_file.get(c);
      while (isdigit(c)) {
	buf += c;
	m_file.get(c);
      }
      m_file.unget();
      m_lastText = buf;
      return (m_lastToken = TOKEN_NUMBER);
    }
    else {
      m_file.unget();
      m_lastText = buf;
      return (m_lastToken = TOKEN_NUMBER);
    }
  }
  else if (c == '.') {
    std::string buf;
    buf += c;
    m_file.get(c);
    
    while (isdigit(c)) {
      buf += c;
      m_file.get(c);
    }
    m_file.unget();
    m_lastText = buf;
    return (m_lastToken = TOKEN_NUMBER);
  }

  else if (c == '"') {
    // We need to do a little magic here, since escaped quotes inside
    // the string are treated as quotes (not end-of-string)
    m_file.unget();
    char a;
  
    m_lastText = "";
  
    do  {
      m_file.get(a);
      if (isspace(a) && a=='\n') {
        m_currentLine++;
      }
    }  while (isspace(a));

    if (a == '\"')  {
      bool lastslash = false;

      m_file.get(a);
      while  (a != '\"' || lastslash)  {
      	if (lastslash && a == '"')  
      	  m_lastText += '"';
      	else if (lastslash)  {
      	  m_lastText += '\\';
      	  m_lastText += a;
      	}
      	else if (a != '\\')
      	  m_lastText += a;
      	
      	lastslash = (a == '\\');
      	m_file.get(a);
      }
    }
    else  {
      do  {
      	m_lastText += a;
      	m_file.get(a);
        if (isspace(a) && a=='\n') {
          m_currentLine++;
        }
      }  while (!isspace(a));
    }

    return (m_lastToken = TOKEN_TEXT);
  }

  m_lastText = "";
  while (!isspace(c) && !m_file.eof()) {
    m_lastText += c;
    m_file.get(c);
  }
  return (m_lastToken = TOKEN_SYMBOL);
}


class TableFilePlayer {
public:
  std::string m_name;
  Array<std::string> m_strategies;
  TableFilePlayer *m_next;

  TableFilePlayer(void);
};

TableFilePlayer::TableFilePlayer(void)
  : m_next(0)
{ }

class TableFileGame {
public:
  std::string m_title, m_comment;
  TableFilePlayer *m_firstPlayer, *m_lastPlayer;
  int m_numPlayers;

  TableFileGame(void);
  ~TableFileGame();

  void AddPlayer(const std::string &);
  int NumPlayers(void) const { return m_numPlayers; }
  int NumStrategies(int pl) const;
  std::string GetPlayer(int pl) const;
  std::string GetStrategy(int pl, int st) const;
};

TableFileGame::TableFileGame(void)
  : m_firstPlayer(0), m_lastPlayer(0), m_numPlayers(0)
{ }

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
  TableFilePlayer *player = new TableFilePlayer;
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
    throw InvalidFileException();
  }

  while (p_state.GetNextToken() == TOKEN_TEXT) {
    p_data.AddPlayer(p_state.GetLastText());
  }

  if (p_state.GetCurrentToken() != TOKEN_RBRACE) {
    throw InvalidFileException();
  }

  p_state.GetNextToken();
}

void ReadStrategies(GameParserState &p_state, TableFileGame &p_data)
{
  if (p_state.GetCurrentToken() != TOKEN_LBRACE) {
    throw InvalidFileException();
  }
  p_state.GetNextToken();

  if (p_state.GetCurrentToken() == TOKEN_LBRACE) {
    TableFilePlayer *player = p_data.m_firstPlayer;

    while (p_state.GetCurrentToken() == TOKEN_LBRACE) {
      if (!player) {
	// Not enough players for number of strategy entries
	throw InvalidFileException();
      }

      while (p_state.GetNextToken() == TOKEN_TEXT) {
	player->m_strategies.Append(p_state.GetLastText());
      }

      if (p_state.GetCurrentToken() != TOKEN_RBRACE) {
	throw InvalidFileException();
      }

      p_state.GetNextToken();
      player = player->m_next;
    }

    if (player) {
      // Players with strategies undefined
      throw InvalidFileException();
    }

    if (p_state.GetCurrentToken() != TOKEN_RBRACE) {
      throw InvalidFileException();
    }

    p_state.GetNextToken();
  }
  else if (p_state.GetCurrentToken() == TOKEN_NUMBER) {
    TableFilePlayer *player = p_data.m_firstPlayer;

    while (p_state.GetCurrentToken() == TOKEN_NUMBER) {
      if (!player) {
	// Not enough players for number of strategy entries
	throw InvalidFileException();
      }

      for (int st = 1; st <= atoi(p_state.GetLastText().c_str()); st++) {
	player->m_strategies.Append(lexical_cast<std::string>(st));
      }

      p_state.GetNextToken();
      player = player->m_next;
    }

    if (p_state.GetCurrentToken() != TOKEN_RBRACE) {
      throw InvalidFileException();
    }

    if (player) {
      // Players with strategies undefined
      throw InvalidFileException();
    }

    p_state.GetNextToken();
  }
  else {
    throw InvalidFileException();
  }
}

void ParseNfgHeader(GameParserState &p_state, TableFileGame &p_data)
{
  if (p_state.GetNextToken() != TOKEN_NUMBER ||
      p_state.GetLastText() != "1") {
    throw InvalidFileException();
  }

  if (p_state.GetNextToken() != TOKEN_SYMBOL || 
      (p_state.GetLastText() != "D" && p_state.GetLastText() != "R")) {
    throw InvalidFileException();
  }
  if (p_state.GetNextToken() != TOKEN_TEXT) {
    throw InvalidFileException();
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


void ReadOutcomeList(GameParserState &p_parser, GameRep *p_nfg)
{
  if (p_parser.GetNextToken() == TOKEN_RBRACE) {
    // Special case: empty outcome list
    p_parser.GetNextToken();
    return;
  }

  if (p_parser.GetCurrentToken() != TOKEN_LBRACE) {
    throw InvalidFileException();
  }

  int nOutcomes = 0;

  while (p_parser.GetCurrentToken() == TOKEN_LBRACE) {
    nOutcomes++;
    int pl = 1;
    
    if (p_parser.GetNextToken() != TOKEN_TEXT) {
      throw InvalidFileException();
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
	outcome->SetPayoff(pl++, p_parser.GetLastText());
	if (p_parser.GetNextToken() == TOKEN_COMMA) {
	  p_parser.GetNextToken();
	}
      }
    }
    catch (IndexException &) {
      // This would be triggered by too many payoffs
      throw InvalidFileException();
    }

    if (pl <= p_nfg->NumPlayers() ||
	p_parser.GetCurrentToken() != TOKEN_RBRACE) {
      throw InvalidFileException();
    }

    p_parser.GetNextToken();
  }

  if (p_parser.GetCurrentToken() != TOKEN_RBRACE) {
    throw InvalidFileException();
  }
  p_parser.GetNextToken();
}

void ParseOutcomeBody(GameParserState &p_parser, GameRep *p_nfg)
{
  ReadOutcomeList(p_parser, p_nfg);

  StrategyIterator iter(StrategySupport(static_cast<GameRep *>(p_nfg)));

  while (p_parser.GetCurrentToken() != TOKEN_EOF) {
    if (p_parser.GetCurrentToken() != TOKEN_NUMBER) {
      throw InvalidFileException();
    }

    int outcomeId = atoi(p_parser.GetLastText().c_str());
    if (outcomeId > 0)  {
      (*iter)->SetOutcome(p_nfg->GetOutcome(outcomeId));
    }
    else {
      (*iter)->SetOutcome(0);
    }
    p_parser.GetNextToken();
    iter++;
  }
}

void ParsePayoffBody(GameParserState &p_parser, GameRep *p_nfg)
{
  StrategyIterator iter(StrategySupport(static_cast<GameRep *>(p_nfg)));
  int pl = 1;

  while (p_parser.GetCurrentToken() != TOKEN_EOF) {
    if (p_parser.GetCurrentToken() == TOKEN_NUMBER) {
      (*iter)->GetOutcome()->SetPayoff(pl, p_parser.GetLastText());
    }
    else {
      throw InvalidFileException();
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

  GameRep *nfg = NewTable(dim);
  // Assigning this to the container assures that, if something goes
  // wrong, the class will automatically be cleaned up
  Game game = nfg;

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
    throw InvalidFileException();
  }

  return game;
}



//=========================================================================
//                  Temporary representation classes
//=========================================================================

class TreeData {
public:
  std::map<int, GameOutcome> m_outcomeMap;
  std::map<int, GameInfoset> m_chanceInfosetMap;
  List<std::map<int, GameInfoset> > m_infosetMap;

  TreeData(void)  { }
  ~TreeData() { }
};

void ReadPlayers(GameParserState &p_state, 
		 Game p_game, TreeData &p_treeData)
{
  if (p_state.GetNextToken() != TOKEN_LBRACE) {
    throw InvalidFileException();
  }

  while (p_state.GetNextToken() == TOKEN_TEXT) {
    p_game->NewPlayer()->SetLabel(p_state.GetLastText());
    p_treeData.m_infosetMap.Append(std::map<int, GameInfoset>());
  }

  if (p_state.GetCurrentToken() != TOKEN_RBRACE) {
    throw InvalidFileException();
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
		  Game p_game, TreeData &p_treeData, 
		  GameNode p_node)
{
  if (p_state.GetCurrentToken() != TOKEN_NUMBER) {
    throw InvalidFileException();
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
      throw InvalidFileException();
    }
    p_state.GetNextToken();

    for (int pl = 1; pl <= p_game->NumPlayers(); pl++) {
      if (p_state.GetCurrentToken() == TOKEN_NUMBER) {
	outcome->SetPayoff(pl, p_state.GetLastText());
      }
      else {
	throw InvalidFileException();
      }

      // Commas are optional between payoffs
      if (p_state.GetNextToken() == TOKEN_COMMA) {
	p_state.GetNextToken();
      }
    }

    if (p_state.GetCurrentToken() != TOKEN_RBRACE) {
      throw InvalidFileException();
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
      throw InvalidFileException();
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
		     Game p_game, GameNode p_node, TreeData &p_treeData)
{
  if (p_state.GetNextToken() != TOKEN_TEXT) {
    throw InvalidFileException();
  }
  p_node->SetLabel(p_state.GetLastText());

  if (p_state.GetNextToken() != TOKEN_NUMBER) {
    throw InvalidFileException();
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
      throw InvalidFileException();
    }
    p_state.GetNextToken();
    do {
      if (p_state.GetCurrentToken() != TOKEN_TEXT) {
	throw InvalidFileException();
      }
      actions.Append(p_state.GetLastText());

      p_state.GetNextToken();
      
      if (p_state.GetCurrentToken() == TOKEN_NUMBER) {
	probs.Append(p_state.GetLastText());
      }
      else {
	throw InvalidFileException();
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
	infoset->SetActionProb(act, probs[act]);
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
    throw InvalidFileException();
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
    throw InvalidFileException();
  }
  p_node->SetLabel(p_state.GetLastText());

  if (p_state.GetNextToken() != TOKEN_NUMBER) {
    throw InvalidFileException();
  }
  int playerId = atoi(p_state.GetLastText().c_str());
  // This will throw an exception if the player ID is not valid
  GamePlayer player = p_game->GetPlayer(playerId);
  std::map<int, GameInfoset> &infosetMap = p_treeData.m_infosetMap[playerId];

  if (p_state.GetNextToken() != TOKEN_NUMBER) {
    throw InvalidFileException();
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
      throw InvalidFileException();
    }
    p_state.GetNextToken();
    do {
      if (p_state.GetCurrentToken() != TOKEN_TEXT) {
	throw InvalidFileException();
      }
      actions.Append(p_state.GetLastText());

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
    throw InvalidFileException();
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
    throw InvalidFileException();
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
    throw InvalidFileException();
  }
}

void ParseEfg(GameParserState &p_state, Game p_game, TreeData &p_treeData)
{
  if (p_state.GetNextToken() != TOKEN_NUMBER ||
      p_state.GetLastText() != "2") {
    throw InvalidFileException();
  }

  if (p_state.GetNextToken() != TOKEN_SYMBOL ||
      (p_state.GetLastText() != "D" && p_state.GetLastText() != "R")) {
    throw InvalidFileException();
  }
  if (p_state.GetNextToken() != TOKEN_TEXT) {
    throw InvalidFileException();
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


namespace Gambit {

//=========================================================================
//    ReadGame: Global visible function to read an .efg or .nfg file
//=========================================================================

Game ReadGame(std::istream &p_file) throw (InvalidFileException)
{
  GameParserState parser(p_file);

  try {
    if (parser.GetNextToken() != TOKEN_SYMBOL) {
      throw InvalidFileException();
    }

    if (parser.GetLastText() == "NFG") {
      TableFileGame data;
      ParseNfgHeader(parser, data);
      return BuildNfg(parser, data);
    }
    else if (parser.GetLastText() == "EFG") {
      TreeData treeData;
      Game game = NewTree();
      ParseEfg(parser, game, treeData);
      return game;
    }
    else {
      throw InvalidFileException();
    }
  }
  catch (...) {
    throw InvalidFileException();
  }
}

} // end namespace Gambit
