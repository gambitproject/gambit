//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Parser for reading extensive form savefiles
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
#include "base/base.h"
#include "efg.h"

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
  gText m_name;
  gBlock<gText> m_actions;
  gBlock<gNumber> m_probs;

  InfosetData(void)
    : m_number(0), m_name("") { }
  void AddAction(const gText &p_action) { m_actions.Append(p_action); }
  void AddProb(const gNumber &p_prob) { m_probs.Append(p_prob); }
};

class OutcomeData {
public:
  gText m_name;
  gBlock<gNumber> m_payoffs;
  
  OutcomeData(const gText &p_name) : m_name(p_name) { }
};

class NodeData {
public:
  gText m_name;
  int m_player, m_infoset, m_outcome;
  InfosetData *m_infosetData;
  OutcomeData *m_outcomeData;
  NodeData *m_next;

  NodeData(void) 
    : m_name(""), m_player(-1), m_infoset(-1), m_outcome(-1),
      m_infosetData(0), m_outcomeData(0), m_next(0) { }
  ~NodeData();
  InfosetData *AddInfosetData(const gText &);
};

NodeData::~NodeData()
{
  if (m_infosetData)  delete m_infosetData;
  if (m_outcomeData)  delete m_outcomeData;
}
  
InfosetData *NodeData::AddInfosetData(const gText &m_infosetName)
{
  m_infosetData = new InfosetData;
  m_infosetData->m_name = m_infosetName;
  return m_infosetData;
}

class DefinedInfosetData {
public:
  int m_fileID;
  gbtEfgInfoset m_infoset;
  DefinedInfosetData *m_next;

  DefinedInfosetData(void) : m_fileID(-1), m_infoset(0), m_next(0) { }
};

class PlayerData {
public:
  gText m_name;
  DefinedInfosetData *m_firstInfoset, *m_lastInfoset;
  PlayerData *m_next;

  PlayerData(void);
  ~PlayerData();
  void AddInfoset(int p_number, gbtEfgInfoset p_infoset);
  gbtEfgInfoset GetInfoset(int p_number);
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

void PlayerData::AddInfoset(int p_number, gbtEfgInfoset p_infoset)
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
gbtEfgInfoset PlayerData::GetInfoset(int p_number)
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
  gbtEfgOutcome m_outcome;

  DefinedOutcomeData(int p_number, gbtEfgOutcome p_outcome)
    : m_fileID(p_number), m_outcome(p_outcome) { }
};

class TreeData {
public:
  gText m_title;
  gText m_comment;
  PlayerData *m_firstPlayer, *m_lastPlayer, m_chancePlayer;
  NodeData *m_firstNode, *m_lastNode;
  gBlock<DefinedOutcomeData *> m_outcomes;

  TreeData(void);
  ~TreeData();

  void AddPlayer(const gText &);
  NodeData *AddNode(const gText &, int, int);
  gbtEfgOutcome GetOutcome(int p_number) const;
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

void TreeData::AddPlayer(const gText &p_player)
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

NodeData *TreeData::AddNode(const gText &p_name, int p_player, int p_infoset)
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
gbtEfgOutcome TreeData::GetOutcome(int p_number) const
{
  for (int outc = 1; outc <= m_outcomes.Length(); outc++) {
    if (m_outcomes[outc]->m_fileID == p_number) {
      return m_outcomes[outc]->m_outcome;
    }
  }
  return 0;
}


//=========================================================================
//                 Parser classes and static functions   
//=========================================================================

//
// These classes and functions are responsible for parsing the
// savefile and building the temporary data structures
//

typedef enum {
  symINTEGER = 0, symDOUBLE = 1, symRATIONAL = 2, symTEXT = 3,
  symEFG = 4, symD = 5, symR = 6, symC = 7, symP = 8, symT = 9,
  symLBRACE = 10, symRBRACE = 11, symCOMMA = 12, symEOF = 13
} SymbolSet;

class ParserState {
private:
  gInput &m_file;

  SymbolSet m_lastSymbol;
  double m_lastDouble;
  gInteger m_lastInteger;
  gRational m_lastRational;
  gText m_lastText;

public:
  ParserState(gInput &p_file) : m_file(p_file) { }

  SymbolSet GetNextSymbol(void);
  SymbolSet GetCurrentSymbol(void) const { return m_lastSymbol; }
  const gInteger &GetLastInteger(void) const { return m_lastInteger; }
  const gRational &GetLastRational(void) const { return m_lastRational; }
  double GetLastDouble(void) const { return m_lastDouble; }
  gText GetLastText(void) const { return m_lastText; }
};  

class ParserError { };

SymbolSet ParserState::GetNextSymbol(void)
{
  char c = ' '; 
  if (m_file.eof()) {
    return (m_lastSymbol = symEOF);
  }

  while (isspace(c)) {
    m_file.get(c);
    if (m_file.eof()) {
      return (m_lastSymbol = symEOF);
    }
  }

  if (c == 'c') {
    return (m_lastSymbol = symC);
  }
  if (c == 'D') {
    return (m_lastSymbol = symD);
  }
  if (c == 'E') {
    m_file.get(c);
    if (c == 'F') {
      m_file.get(c);
      if (c == 'G') {
	m_file.get(c);
	if (!isspace(c)) {
	  throw ParserError();
	}
	else {
	  return (m_lastSymbol = symEFG);
	}
      }
    }
    throw ParserError(); 
  }
  else if (c == 'p') {
    return (m_lastSymbol = symP);
  }
  else if (c == 'R') {
    return (m_lastSymbol = symR);
  }
  else if (c == 't') {
    return (m_lastSymbol = symT);
  }
  else if (c == '{') {
    return (m_lastSymbol = symLBRACE);
  }
  else if (c == '}') {
    return (m_lastSymbol = symRBRACE);
  }
  else if (c == ',') {
    return (m_lastSymbol = symCOMMA);
  }
  else if (isdigit(c) || c == '-') {
    gText buf;
    buf += c;
    m_file.get(c);
    
    while (isdigit(c)) {
      buf += c;
      m_file.get(c);
    }
    
    if (c == '.') {
      buf += c;
      m_file.get(c);
      while (isdigit(c)) {
	buf += c;
	m_file.get(c);
      }
      m_file.unget(c);
      m_lastDouble = strtod((char *) buf, 0);
      return (m_lastSymbol = symDOUBLE);
    }
    else if (c == '/') {
      buf += c;
      m_file.get(c);
      while (isdigit(c)) {
	buf += c;
	m_file.get(c);
      }
      m_file.unget(c);
      FromText(buf, m_lastRational);
      return (m_lastSymbol = symRATIONAL);
    }
      
    else {
      m_file.unget(c);

      m_lastInteger = atoi((char *) buf);
      return (m_lastSymbol = symINTEGER);
    }
  }
  else if (c == '"') {
    // The operator>> for gText handles quote-escaping
    m_file.unget(c);
    m_file >> m_lastText;
    return (m_lastSymbol = symTEXT);
  }

  throw ParserError();
}

static void ReadPlayers(ParserState &p_state, TreeData &p_treeData)
{
  if (p_state.GetNextSymbol() != symLBRACE) {
    throw ParserError();
  }

  while (p_state.GetNextSymbol() == symTEXT) {
    p_treeData.AddPlayer(p_state.GetLastText());
  }

  if (p_state.GetCurrentSymbol() != symRBRACE) {
    throw ParserError();
  }
}

static void ParseOutcome(ParserState &p_state, TreeData &p_treeData, 
			 NodeData *p_node)
{
  if (p_state.GetCurrentSymbol() == symTEXT) {
    p_node->m_outcomeData = new OutcomeData(p_state.GetLastText());

    if (p_state.GetNextSymbol() != symLBRACE) {
      throw ParserError();
    }
    p_state.GetNextSymbol();
    do {
      if (p_state.GetCurrentSymbol() == symINTEGER) {
	p_node->m_outcomeData->m_payoffs.Append(p_state.GetLastInteger());
      }
      else if (p_state.GetCurrentSymbol() == symDOUBLE) {
	p_node->m_outcomeData->m_payoffs.Append(p_state.GetLastDouble());
      }
      else if (p_state.GetCurrentSymbol() == symRATIONAL) {
	p_node->m_outcomeData->m_payoffs.Append(p_state.GetLastRational());
      }
      else {
	throw ParserError();
      }

      if (p_state.GetNextSymbol() == symCOMMA) {
	p_state.GetNextSymbol();
      }
    } while (p_state.GetCurrentSymbol() != symRBRACE);
    p_state.GetNextSymbol();
  }
}

static void ParseChanceNode(ParserState &p_state, TreeData &p_treeData)
{
  if (p_state.GetNextSymbol() != symTEXT) {
    throw ParserError();
  }
  gText nodeName = p_state.GetLastText();

  if (p_state.GetNextSymbol() != symINTEGER) {
    throw ParserError();
  }
  int nodeInfoset = p_state.GetLastInteger().as_long();

  NodeData *node = p_treeData.AddNode(nodeName, 0, nodeInfoset);

  p_state.GetNextSymbol();

  if (p_state.GetCurrentSymbol() == symTEXT) {
    // information set name is specified
    InfosetData *infoset = node->AddInfosetData(p_state.GetLastText());

    if (p_state.GetNextSymbol() != symLBRACE) {
      throw ParserError();
    }
    p_state.GetNextSymbol();
    do {
      if (p_state.GetCurrentSymbol() != symTEXT) {
	throw ParserError();
      }
      infoset->AddAction(p_state.GetLastText());

      p_state.GetNextSymbol();
      
      if (p_state.GetCurrentSymbol() == symINTEGER) {
	infoset->AddProb(p_state.GetLastInteger());
      }
      else if (p_state.GetCurrentSymbol() == symDOUBLE) {
	infoset->AddProb(p_state.GetLastDouble());
      }
      else if (p_state.GetCurrentSymbol() == symRATIONAL) {
	infoset->AddProb(p_state.GetLastRational());
      }
      else {
	throw ParserError();
      }

      p_state.GetNextSymbol();
    } while (p_state.GetCurrentSymbol() != symRBRACE);
    p_state.GetNextSymbol();
  }

  if (p_state.GetCurrentSymbol() != symINTEGER) {
    throw ParserError();
  }
  node->m_outcome = p_state.GetLastInteger().as_long();

  p_state.GetNextSymbol();
  ParseOutcome(p_state, p_treeData, node);
}

static void ParsePersonalNode(ParserState &p_state, TreeData &p_treeData)
{
  if (p_state.GetNextSymbol() != symTEXT) {
    throw ParserError();
  }
  gText nodeName = p_state.GetLastText();

  if (p_state.GetNextSymbol() != symINTEGER) {
    throw ParserError();
  }
  int nodePlayer = p_state.GetLastInteger().as_long();

  if (p_state.GetNextSymbol() != symINTEGER) {
    throw ParserError();
  }
  int nodeInfoset = p_state.GetLastInteger().as_long();

  NodeData *node = p_treeData.AddNode(nodeName, nodePlayer, nodeInfoset);

  p_state.GetNextSymbol();
  if (p_state.GetCurrentSymbol() == symTEXT) {
    // information set name is specified
    InfosetData *infoset = node->AddInfosetData(p_state.GetLastText());

    if (p_state.GetNextSymbol() != symLBRACE) {
      throw ParserError();
    }
    p_state.GetNextSymbol();
    do {
      if (p_state.GetCurrentSymbol() != symTEXT) {
	throw ParserError();
      }
      infoset->AddAction(p_state.GetLastText());

      p_state.GetNextSymbol();
    } while (p_state.GetCurrentSymbol() != symRBRACE);
    p_state.GetNextSymbol();
  }

  if (p_state.GetCurrentSymbol() != symINTEGER) {
    throw ParserError();
  }
  node->m_outcome = p_state.GetLastInteger().as_long();

  p_state.GetNextSymbol();
  ParseOutcome(p_state, p_treeData, node);
}

static void ParseTerminalNode(ParserState &p_state, TreeData &p_treeData)
{
  if (p_state.GetNextSymbol() != symTEXT) {
    throw ParserError();
  }
  
  NodeData *node = p_treeData.AddNode(p_state.GetLastText(), -1, -1);

  if (p_state.GetNextSymbol() != symINTEGER) {
    throw ParserError();
  }
  node->m_outcome = p_state.GetLastInteger().as_long();

  p_state.GetNextSymbol();
  ParseOutcome(p_state, p_treeData, node);
}

static void Parse(ParserState &p_state, TreeData &p_treeData)
{
  SymbolSet symbol;

  if (p_state.GetNextSymbol() != symEFG) {
    throw ParserError();
  }
  if (p_state.GetNextSymbol() != symINTEGER) {
    throw ParserError();
  }
  if (p_state.GetLastInteger() != 2) {
    throw ParserError();
  }

  symbol = p_state.GetNextSymbol();
  if (symbol != symD && symbol != symR) {
    throw ParserError();
  }
  if (p_state.GetNextSymbol() != symTEXT) {
    throw ParserError();
  }
  p_treeData.m_title = p_state.GetLastText();
  
  ReadPlayers(p_state, p_treeData);

  if (p_state.GetNextSymbol() == symTEXT) {
    // Read optional comment
    p_treeData.m_comment = p_state.GetLastText();
    p_state.GetNextSymbol();
  }

  while (p_state.GetCurrentSymbol() != symEOF) {
    switch (p_state.GetCurrentSymbol()) {
    case symC:
      ParseChanceNode(p_state, p_treeData);
      break;
    case symP:
      ParsePersonalNode(p_state, p_treeData);
      break;
    case symT:
      ParseTerminalNode(p_state, p_treeData);
      break;
    default:
      throw ParserError();
      break;
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

static void BuildSubtree(efgGame *p_efg, gbtEfgNode p_node,
			 TreeData &p_treeData, NodeData **p_nodeData)
{
  p_node.SetLabel((*p_nodeData)->m_name);

  if ((*p_nodeData)->m_outcome > 0) {
    if (!p_treeData.GetOutcome((*p_nodeData)->m_outcome).IsNull()) {
      p_node.SetOutcome(p_treeData.GetOutcome((*p_nodeData)->m_outcome));
    }
    else {
      gbtEfgOutcome outcome = p_efg->NewOutcome();
      outcome.SetLabel((*p_nodeData)->m_outcomeData->m_name);
      p_treeData.m_outcomes.Append(new DefinedOutcomeData((*p_nodeData)->m_outcome,
							  outcome));
      p_node.SetOutcome(outcome);
      for (int pl = 1; pl <= p_efg->NumPlayers(); pl++) {
	outcome.SetPayoff(p_efg->GetPlayer(pl),
			  (*p_nodeData)->m_outcomeData->m_payoffs[pl]);
      }
    }
  }

  if ((*p_nodeData)->m_player > 0) {
    PlayerData *player = p_treeData.m_firstPlayer;
    for (int i = 1; i < (*p_nodeData)->m_player; i++, player = player->m_next);

    if (!player->GetInfoset((*p_nodeData)->m_infoset).IsNull()) {
      gbtEfgInfoset infoset = player->GetInfoset((*p_nodeData)->m_infoset);
      p_efg->AppendNode(p_node, infoset);
    }
    else {
      gbtEfgInfoset infoset =
	p_efg->AppendNode(p_node, p_efg->GetPlayer((*p_nodeData)->m_player),
			  (*p_nodeData)->m_infosetData->m_actions.Length());

      infoset.SetLabel((*p_nodeData)->m_infosetData->m_name);
      for (int act = 1; act <= infoset.NumActions(); act++) {
	infoset.GetAction(act).SetLabel((*p_nodeData)->m_infosetData->m_actions[act]);
      }
      player->AddInfoset((*p_nodeData)->m_infoset, infoset);
    }

    *(p_nodeData) = (*(p_nodeData))->m_next;
    for (int i = 1; i <= p_node.NumChildren(); i++) {
      BuildSubtree(p_efg, p_node.GetChild(i), p_treeData, p_nodeData);
    }
  }
  else if ((*p_nodeData)->m_player == 0) {
    PlayerData *player = &p_treeData.m_chancePlayer;

    if (!player->GetInfoset((*p_nodeData)->m_infoset).IsNull()) {
      gbtEfgInfoset infoset = player->GetInfoset((*p_nodeData)->m_infoset);
      p_efg->AppendNode(p_node, infoset);
    }
    else {
      gbtEfgInfoset infoset = p_efg->AppendNode(p_node, p_efg->GetChance(),
						(*p_nodeData)->m_infosetData->m_actions.Length());

      infoset.SetLabel((*p_nodeData)->m_infosetData->m_name);
      for (int act = 1; act <= infoset.NumActions(); act++) {
	infoset.GetAction(act).SetLabel((*p_nodeData)->m_infosetData->m_actions[act]);
	p_efg->SetChanceProb(infoset, act, 
			     (*p_nodeData)->m_infosetData->m_probs[act]);
      }
    }

    *(p_nodeData) = (*(p_nodeData))->m_next;
    for (int i = 1; i <= p_node.NumChildren(); i++) {
      BuildSubtree(p_efg, p_node.GetChild(i), p_treeData, p_nodeData);
    }
  }
  else {
    // Terminal node
    *(p_nodeData) = (*(p_nodeData))->m_next;
  }
}

static void BuildEfg(efgGame *p_efg, TreeData &p_treeData)
{
  p_efg->SetTitle(p_treeData.m_title);
  p_efg->SetComment(p_treeData.m_comment);
  for (PlayerData *player = p_treeData.m_firstPlayer; player;
       player = player->m_next) {
    p_efg->NewPlayer().SetLabel(player->m_name);
  }
  NodeData *node = p_treeData.m_firstNode;
  BuildSubtree(p_efg, p_efg->RootNode(), p_treeData, &node);
}


//=========================================================================
// ReadEfgFile: Global visible function to read an extensive form savefile
//=========================================================================

efgGame *ReadEfgFile(gInput &p_file)
{
  ParserState parser(p_file);
  TreeData treeData;

  efgGame *efg = new efgGame;

  try {
    Parse(parser, treeData);
    BuildEfg(efg, treeData);
    return efg;
  }
  catch (ParserError &) {
    delete efg;
    return 0;
  }
  catch (...) {
    delete efg;
    return 0;
  }
}

#include "base/garray.imp"
#include "base/gblock.imp"

template class gArray<DefinedOutcomeData *>;
template class gBlock<DefinedOutcomeData *>;
