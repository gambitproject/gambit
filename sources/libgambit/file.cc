//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Parser for reading game savefiles
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

#include <stdlib.h>
#include <ctype.h>
#include <sstream>

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
  else if (isdigit(c) || c == '-') {
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
  Gambit::Array<std::string> m_strategies;
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

static void ReadPlayers(GameParserState &p_state, TableFileGame &p_data)
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

static void ReadStrategies(GameParserState &p_state, TableFileGame &p_data)
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
	player->m_strategies.Append(ToText(Gambit::Integer(st)));
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

static void ParseNfgHeader(GameParserState &p_state, TableFileGame &p_data)
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

static void ReadOutcomeList(GameParserState &p_parser, Game p_nfg)
{
  if (p_parser.GetNextToken() == TOKEN_RBRACE) {
    // Special case: empty outcome list
    p_parser.GetNextToken();
    return;
  }

  if (p_parser.GetCurrentToken() != TOKEN_LBRACE) {
    throw InvalidFileException();
  }

  while (p_parser.GetCurrentToken() == TOKEN_LBRACE) {
    Gambit::Array<std::string> payoffs(p_nfg->NumPlayers());
    int pl = 1;

    if (p_parser.GetNextToken() != TOKEN_TEXT) {
      throw InvalidFileException();
    }
    std::string label = p_parser.GetLastText();
    p_parser.GetNextToken();

    while (p_parser.GetCurrentToken() == TOKEN_NUMBER) {
      if (pl > p_nfg->NumPlayers()) {
	throw InvalidFileException();
      }

      payoffs[pl++] = p_parser.GetLastText();
      if (p_parser.GetNextToken() == TOKEN_COMMA) {
	p_parser.GetNextToken();
      }
    }

    if (pl <= p_nfg->NumPlayers()) {
      throw InvalidFileException();
    }

    if (p_parser.GetCurrentToken() != TOKEN_RBRACE) {
      throw InvalidFileException();
    }

    GameOutcome outcome = p_nfg->NewOutcome();
    outcome->SetLabel(label);
    for (pl = 1; pl <= p_nfg->NumPlayers(); pl++) {
      outcome->SetPayoff(pl, payoffs[pl]);
    }

    p_parser.GetNextToken();
  }

  if (p_parser.GetCurrentToken() != TOKEN_RBRACE) {
    throw InvalidFileException();
  }
  p_parser.GetNextToken();
}

void ParseOutcomeBody(GameParserState &p_parser, Game p_nfg)
{
  ReadOutcomeList(p_parser, p_nfg);

  StrategyIterator iter(p_nfg);

  while (p_parser.GetCurrentToken() != TOKEN_EOF) {
    if (p_parser.GetCurrentToken() != TOKEN_NUMBER) {
      throw InvalidFileException();
    }

    int outcomeId = atoi(p_parser.GetLastText().c_str());
    if (outcomeId > 0)  {
      iter->SetOutcome(p_nfg->GetOutcome(outcomeId));
    }
    else {
      iter->SetOutcome(0);
    }
    p_parser.GetNextToken();
    iter++;
  }
}

static void ParsePayoffBody(GameParserState &p_parser, 
			    Game p_nfg)
{
  StrategyIterator iter(p_nfg);
  int pl = 1;

  while (p_parser.GetCurrentToken() != TOKEN_EOF) {
    if (pl == 1) {
      iter->SetOutcome(p_nfg->NewOutcome());
    }

    if (p_parser.GetCurrentToken() == TOKEN_NUMBER) {
      iter->GetOutcome()->SetPayoff(pl, p_parser.GetLastText());
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

static Game BuildNfg(GameParserState &p_parser, 
			   TableFileGame &p_data)
{
  Gambit::Array<int> dim(p_data.NumPlayers());
  for (int pl = 1; pl <= dim.Length(); pl++) {
    dim[pl] = p_data.NumStrategies(pl);
  }
  Game nfg = new GameRep(dim);

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

  return nfg;
}



//=========================================================================
//                  Temporary representation classes
//=========================================================================

//
// The following classes temporarily contain and organize the data
// read in from a file:
// class InfosetData
// class OutcomeData
// class NodeData
// class PlayerData
// class TreeData
//

class InfosetData {
public:
  int m_number;
  std::string m_name;
  Gambit::Array<std::string> m_actions;
  Gambit::Array<std::string> m_probs;

  InfosetData(void)
    : m_number(0), m_name("") { }
  void AddAction(const std::string &p_action) { m_actions.Append(p_action); }
  void AddProb(const std::string &p_prob) { m_probs.Append(p_prob); }
};

class OutcomeData {
public:
  std::string m_name;
  Gambit::Array<std::string> m_payoffs;
  
  OutcomeData(const std::string &p_name) : m_name(p_name) { }
};

class NodeData {
public:
  std::string m_name;
  int m_player, m_infoset, m_outcome;
  InfosetData *m_infosetData;
  OutcomeData *m_outcomeData;
  NodeData *m_next;

  NodeData(void) 
    : m_name(""), m_player(-1), m_infoset(-1), m_outcome(-1),
      m_infosetData(0), m_outcomeData(0), m_next(0) { }
  ~NodeData();
  InfosetData *AddInfosetData(const std::string &);
};

NodeData::~NodeData()
{
  if (m_infosetData)  delete m_infosetData;
  if (m_outcomeData)  delete m_outcomeData;
}
  
InfosetData *NodeData::AddInfosetData(const std::string &m_infosetName)
{
  m_infosetData = new InfosetData;
  m_infosetData->m_name = m_infosetName;
  return m_infosetData;
}

class DefinedInfosetData {
public:
  int m_fileID;
  GameInfoset m_infoset;
  DefinedInfosetData *m_next;

  DefinedInfosetData(void) : m_fileID(-1), m_infoset(0), m_next(0) { }
};

class PlayerData {
public:
  std::string m_name;
  DefinedInfosetData *m_firstInfoset, *m_lastInfoset;
  PlayerData *m_next;

  PlayerData(void);
  ~PlayerData();
  void AddInfoset(int p_number, GameInfoset p_infoset);
  GameInfoset GetInfoset(int p_number);
};

PlayerData::PlayerData(void)
  : m_name(""), m_firstInfoset(0), m_lastInfoset(0), m_next(0)
{ }

PlayerData::~PlayerData()
{
  DefinedInfosetData *infoset = m_firstInfoset;
  while (infoset) {
    DefinedInfosetData *nextInfoset = infoset->m_next;
    delete infoset;
    infoset = nextInfoset;
  }
}

void PlayerData::AddInfoset(int p_number, GameInfoset p_infoset)
{
  DefinedInfosetData *infoset = new DefinedInfosetData;
  infoset->m_fileID = p_number;
  infoset->m_infoset = p_infoset;

  if (m_firstInfoset) {
    m_lastInfoset->m_next = infoset;
    m_lastInfoset = infoset;
  }
  else {
    m_firstInfoset = infoset;
    m_lastInfoset = infoset;
  }
}

//
// If information set p_number (as numbered in the savefile) has
// been created, returns the pointer to the Infoset structure; otherwise,
// returns null.
//
GameInfoset PlayerData::GetInfoset(int p_number)
{
  for (DefinedInfosetData *infoset = m_firstInfoset;
       infoset; infoset = infoset->m_next) {
    if (infoset->m_fileID == p_number) {
      return infoset->m_infoset;
    }
  }

  return 0;
}

class DefinedOutcomeData {
public:
  int m_fileID;
  GameOutcome m_outcome;

  DefinedOutcomeData(int p_number, GameOutcome p_outcome)
    : m_fileID(p_number), m_outcome(p_outcome) { }
};

class TreeData {
public:
  std::string m_title;
  std::string m_comment;
  PlayerData *m_firstPlayer, *m_lastPlayer, m_chancePlayer;
  NodeData *m_firstNode, *m_lastNode;
  Gambit::Array<DefinedOutcomeData *> m_outcomes;

  TreeData(void);
  ~TreeData();

  void AddPlayer(const std::string &);
  NodeData *AddNode(const std::string &, int, int);
  GameOutcome GetOutcome(int p_number) const;
};

TreeData::TreeData(void)
  : m_title(""), m_comment(""), m_firstPlayer(0), m_lastPlayer(0),
    m_firstNode(0), m_lastNode(0)
{ }

TreeData::~TreeData()
{
  if (m_firstPlayer) {
    PlayerData *player = m_firstPlayer;
    while (player) {
      PlayerData *nextPlayer = player->m_next;
      delete player;
      player = nextPlayer;
    }
  }

  if (m_firstNode) {
    NodeData *node = m_firstNode;
    while (node) {
      NodeData *nextNode = node->m_next;
      delete node;
      node = nextNode;
    }
  }

  for (int outc = 1; outc <= m_outcomes.Length(); outc++) {
    delete m_outcomes[outc];
  }
}

void TreeData::AddPlayer(const std::string &p_player)
{
  PlayerData *player = new PlayerData;
  player->m_name = p_player;

  if (m_firstPlayer) {
    m_lastPlayer->m_next = player;
    m_lastPlayer = player;
  }
  else {
    m_firstPlayer = player;
    m_lastPlayer = player;
  }
}

NodeData *TreeData::AddNode(const std::string &p_name, int p_player, int p_infoset)
{
  NodeData *node = new NodeData;
  node->m_name = p_name;
  node->m_player = p_player;
  node->m_infoset = p_infoset;

  if (m_firstNode) {
    m_lastNode->m_next = node;
    m_lastNode = node;
  }
  else {
    m_firstNode = node;
    m_lastNode = node;
  }

  return node;
}

//
// If outcome number p_number (as numbered in the savefile) has
// been created, returns a pointer to the outcome;
// otherwise, returns a null outcome
//
GameOutcome TreeData::GetOutcome(int p_number) const
{
  for (int outc = 1; outc <= m_outcomes.Length(); outc++) {
    if (m_outcomes[outc]->m_fileID == p_number) {
      return m_outcomes[outc]->m_outcome;
    }
  }
  return 0;
}

static void ReadPlayers(GameParserState &p_state, TreeData &p_treeData)
{
  if (p_state.GetNextToken() != TOKEN_LBRACE) {
    throw InvalidFileException();
  }

  while (p_state.GetNextToken() == TOKEN_TEXT) {
    p_treeData.AddPlayer(p_state.GetLastText());
  }

  if (p_state.GetCurrentToken() != TOKEN_RBRACE) {
    throw InvalidFileException();
  }
}

static void ParseOutcome(GameParserState &p_state, TreeData &p_treeData, 
			 NodeData *p_node)
{
  if (p_state.GetCurrentToken() == TOKEN_TEXT) {
    p_node->m_outcomeData = new OutcomeData(p_state.GetLastText());

    if (p_state.GetNextToken() != TOKEN_LBRACE) {
      throw InvalidFileException();
    }
    p_state.GetNextToken();
    do {
      if (p_state.GetCurrentToken() == TOKEN_NUMBER) {
	p_node->m_outcomeData->m_payoffs.Append(p_state.GetLastText());
      }
      else {
	throw InvalidFileException();
      }

      if (p_state.GetNextToken() == TOKEN_COMMA) {
	p_state.GetNextToken();
      }
    } while (p_state.GetCurrentToken() != TOKEN_RBRACE);
    p_state.GetNextToken();
  }
}

static void ParseChanceNode(GameParserState &p_state, TreeData &p_treeData)
{
  if (p_state.GetNextToken() != TOKEN_TEXT) {
    throw InvalidFileException();
  }
  std::string nodeName = p_state.GetLastText();

  if (p_state.GetNextToken() != TOKEN_NUMBER) {
    throw InvalidFileException();
  }
  int nodeInfoset = atoi(p_state.GetLastText().c_str());

  NodeData *node = p_treeData.AddNode(nodeName, 0, nodeInfoset);

  p_state.GetNextToken();

  if (p_state.GetCurrentToken() == TOKEN_TEXT) {
    // information set name is specified
    InfosetData *infoset = node->AddInfosetData(p_state.GetLastText());

    if (p_state.GetNextToken() != TOKEN_LBRACE) {
      throw InvalidFileException();
    }
    p_state.GetNextToken();
    do {
      if (p_state.GetCurrentToken() != TOKEN_TEXT) {
	throw InvalidFileException();
      }
      infoset->AddAction(p_state.GetLastText());

      p_state.GetNextToken();
      
      if (p_state.GetCurrentToken() == TOKEN_NUMBER) {
	infoset->AddProb(p_state.GetLastText());
      }
      else {
	throw InvalidFileException();
      }

      p_state.GetNextToken();
    } while (p_state.GetCurrentToken() != TOKEN_RBRACE);
    p_state.GetNextToken();
  }

  if (p_state.GetCurrentToken() != TOKEN_NUMBER) {
    throw InvalidFileException();
  }
  node->m_outcome = atoi(p_state.GetLastText().c_str());

  p_state.GetNextToken();
  ParseOutcome(p_state, p_treeData, node);
}

static void ParsePersonalNode(GameParserState &p_state, TreeData &p_treeData)
{
  if (p_state.GetNextToken() != TOKEN_TEXT) {
    throw InvalidFileException();
  }
  std::string nodeName = p_state.GetLastText();

  if (p_state.GetNextToken() != TOKEN_NUMBER) {
    throw InvalidFileException();
  }
  int nodePlayer = atoi(p_state.GetLastText().c_str());

  if (p_state.GetNextToken() != TOKEN_NUMBER) {
    throw InvalidFileException();
  }
  int nodeInfoset = atoi(p_state.GetLastText().c_str());

  NodeData *node = p_treeData.AddNode(nodeName, nodePlayer, nodeInfoset);

  p_state.GetNextToken();
  if (p_state.GetCurrentToken() == TOKEN_TEXT) {
    // information set name is specified
    InfosetData *infoset = node->AddInfosetData(p_state.GetLastText());

    if (p_state.GetNextToken() != TOKEN_LBRACE) {
      throw InvalidFileException();
    }
    p_state.GetNextToken();
    do {
      if (p_state.GetCurrentToken() != TOKEN_TEXT) {
	throw InvalidFileException();
      }
      infoset->AddAction(p_state.GetLastText());

      p_state.GetNextToken();
    } while (p_state.GetCurrentToken() != TOKEN_RBRACE);
    p_state.GetNextToken();
  }

  if (p_state.GetCurrentToken() != TOKEN_NUMBER) {
    throw InvalidFileException();
  }
  node->m_outcome = atoi(p_state.GetLastText().c_str());

  p_state.GetNextToken();
  ParseOutcome(p_state, p_treeData, node);
}

static void ParseTerminalNode(GameParserState &p_state, TreeData &p_treeData)
{
  if (p_state.GetNextToken() != TOKEN_TEXT) {
    throw InvalidFileException();
  }
  
  NodeData *node = p_treeData.AddNode(p_state.GetLastText(), -1, -1);

  if (p_state.GetNextToken() != TOKEN_NUMBER) {
    throw InvalidFileException();
  }
  node->m_outcome = atoi(p_state.GetLastText().c_str());

  p_state.GetNextToken();
  ParseOutcome(p_state, p_treeData, node);
}

static void ParseEfg(GameParserState &p_state, TreeData &p_treeData)
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
  p_treeData.m_title = p_state.GetLastText();
  
  ReadPlayers(p_state, p_treeData);

  if (p_state.GetNextToken() == TOKEN_TEXT) {
    // Read optional comment
    p_treeData.m_comment = p_state.GetLastText();
    p_state.GetNextToken();
  }

  while (p_state.GetCurrentToken() != TOKEN_EOF) {
    if (p_state.GetCurrentToken() != TOKEN_SYMBOL) {
      throw InvalidFileException();
    }

    if (p_state.GetLastText() == "c") {
      ParseChanceNode(p_state, p_treeData);
    }
    else if (p_state.GetLastText() == "p") {
      ParsePersonalNode(p_state, p_treeData);
    }
    else if (p_state.GetLastText() == "t") {
      ParseTerminalNode(p_state, p_treeData);
    }
    else {
      throw InvalidFileException();
    }
  }

}

//=========================================================================
//                       Tree-building routines
//=========================================================================

//
// These routines use the temporary data structures to construct 
// the actual tree to be returned
//

static void BuildSubtree(Game p_efg, GameNode p_node,
			 TreeData &p_treeData, NodeData **p_nodeData)
{
  p_node->SetLabel((*p_nodeData)->m_name);

  if ((*p_nodeData)->m_outcome > 0) {
    if (p_treeData.GetOutcome((*p_nodeData)->m_outcome)) {
      p_node->SetOutcome(p_treeData.GetOutcome((*p_nodeData)->m_outcome));
    }
    else {
      GameOutcome outcome = p_efg->NewOutcome();
      outcome->SetLabel((*p_nodeData)->m_outcomeData->m_name);
      p_treeData.m_outcomes.Append(new DefinedOutcomeData((*p_nodeData)->m_outcome,
							  outcome));
      p_node->SetOutcome(outcome);
      for (int pl = 1; pl <= p_efg->NumPlayers(); pl++) {
	outcome->SetPayoff(pl, (*p_nodeData)->m_outcomeData->m_payoffs[pl]);
      }
    }
  }

  if ((*p_nodeData)->m_player > 0) {
    PlayerData *player = p_treeData.m_firstPlayer;
    for (int i = 1; i < (*p_nodeData)->m_player; i++, player = player->m_next);

    if (player->GetInfoset((*p_nodeData)->m_infoset)) {
      GameInfoset infoset = player->GetInfoset((*p_nodeData)->m_infoset);
      p_node->AppendMove(infoset);
    }
    else {
      GameInfoset infoset =
	p_node->AppendMove(p_efg->GetPlayer((*p_nodeData)->m_player),
			   (*p_nodeData)->m_infosetData->m_actions.Length());

      infoset->SetLabel((*p_nodeData)->m_infosetData->m_name);
      for (int act = 1; act <= infoset->NumActions(); act++) {
	infoset->GetAction(act)->SetLabel((*p_nodeData)->m_infosetData->m_actions[act]);
      }
      player->AddInfoset((*p_nodeData)->m_infoset, infoset);
    }

    *(p_nodeData) = (*(p_nodeData))->m_next;
    for (int i = 1; i <= p_node->NumChildren(); i++) {
      BuildSubtree(p_efg, p_node->GetChild(i), p_treeData, p_nodeData);
    }
  }
  else if ((*p_nodeData)->m_player == 0) {
    PlayerData *player = &p_treeData.m_chancePlayer;

    if (player->GetInfoset((*p_nodeData)->m_infoset)) {
      GameInfoset infoset = player->GetInfoset((*p_nodeData)->m_infoset);
      p_node->AppendMove(infoset);
    }
    else {
      GameInfoset infoset = p_node->AppendMove(p_efg->GetChance(),
					       (*p_nodeData)->m_infosetData->m_actions.Length());

      infoset->SetLabel((*p_nodeData)->m_infosetData->m_name);
      for (int act = 1; act <= infoset->NumActions(); act++) {
	infoset->GetAction(act)->SetLabel((*p_nodeData)->m_infosetData->m_actions[act]);
	infoset->SetActionProb(act, 
			       (*p_nodeData)->m_infosetData->m_probs[act]);
      }
      player->AddInfoset((*p_nodeData)->m_infoset, infoset);
    }

    *(p_nodeData) = (*(p_nodeData))->m_next;
    for (int i = 1; i <= p_node->NumChildren(); i++) {
      BuildSubtree(p_efg, p_node->GetChild(i), p_treeData, p_nodeData);
    }
  }
  else {
    // Terminal node
    *(p_nodeData) = (*(p_nodeData))->m_next;
  }
}

static void BuildEfg(Game p_efg, TreeData &p_treeData)
{
  p_efg->SetTitle(p_treeData.m_title);
  p_efg->SetComment(p_treeData.m_comment);
  for (PlayerData *player = p_treeData.m_firstPlayer; player;
       player = player->m_next) {
    p_efg->NewPlayer()->SetLabel(player->m_name);
  }
  NodeData *node = p_treeData.m_firstNode;
  BuildSubtree(p_efg, p_efg->GetRoot(), p_treeData, &node);
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
      Game efg = new GameRep;
      TreeData treeData;
      ParseEfg(parser, treeData);
      BuildEfg(efg, treeData);
      efg->Canonicalize();
      return efg;
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
