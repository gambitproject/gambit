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
#include "game.h"

#include "game-file.h"
#include "tree-game.h"

//=========================================================================
//                  Temporary representation classes
//=========================================================================

//
// The following classes temporarily contain and organize the data
// read in from a file:
// class gbtTreeFileInfoset
// class gbtTreeFileOutcome
// class gbtTreeFileNode
// class gbtTreeFilePlayer
// class gbtTreeFileGame
//

class gbtTreeFileInfoset {
public:
  int m_number;
  std::string m_name;
  gbtBlock<std::string> m_actions;
  gbtBlock<gbtRational> m_probs;

  gbtTreeFileInfoset(void)
    : m_number(0), m_name("") { }
  void AddAction(const std::string &p_action) { m_actions.Append(p_action); }
  void AddProb(const gbtRational &p_prob) { m_probs.Append(p_prob); }
};

class gbtTreeFileOutcome {
public:
  std::string m_name;
  gbtBlock<gbtRational> m_payoffs;
  
  gbtTreeFileOutcome(const std::string &p_name) : m_name(p_name) { }
};

class gbtTreeFileNode {
public:
  std::string m_name;
  int m_player, m_infoset, m_outcome;
  gbtTreeFileInfoset *m_infosetData;
  gbtTreeFileOutcome *m_outcomeData;
  gbtTreeFileNode *m_next;

  gbtTreeFileNode(void) 
    : m_name(""), m_player(-1), m_infoset(-1), m_outcome(-1),
      m_infosetData(0), m_outcomeData(0), m_next(0) { }
  ~gbtTreeFileNode();
  gbtTreeFileInfoset *AddInfoset(const std::string &);
};

gbtTreeFileNode::~gbtTreeFileNode()
{
  if (m_infosetData)  delete m_infosetData;
  if (m_outcomeData)  delete m_outcomeData;
}
  
gbtTreeFileInfoset *gbtTreeFileNode::AddInfoset(const std::string &m_infosetName)
{
  m_infosetData = new gbtTreeFileInfoset;
  m_infosetData->m_name = m_infosetName;
  return m_infosetData;
}

class gbtTreeFileDefinedInfoset {
public:
  int m_fileID;
  gbtGameInfoset m_infoset;
  gbtTreeFileDefinedInfoset *m_next;

  gbtTreeFileDefinedInfoset(void) : m_fileID(-1), m_infoset(0), m_next(0) { }
};

class gbtTreeFilePlayer {
public:
  std::string m_name;
  gbtTreeFileDefinedInfoset *m_firstInfoset, *m_lastInfoset;
  gbtTreeFilePlayer *m_next;

  gbtTreeFilePlayer(void);
  ~gbtTreeFilePlayer();
  void AddInfoset(int p_number, gbtGameInfoset p_infoset);
  gbtGameInfoset GetInfoset(int p_number);
};

gbtTreeFilePlayer::gbtTreeFilePlayer(void)
  : m_name(""), m_firstInfoset(0), m_lastInfoset(0), m_next(0)
{ }

gbtTreeFilePlayer::~gbtTreeFilePlayer()
{
  gbtTreeFileDefinedInfoset *infoset = m_firstInfoset;
  while (infoset) {
    gbtTreeFileDefinedInfoset *nextInfoset = infoset->m_next;
    delete infoset;
    infoset = nextInfoset;
  }
}

void gbtTreeFilePlayer::AddInfoset(int p_number, gbtGameInfoset p_infoset)
{
  gbtTreeFileDefinedInfoset *infoset = new gbtTreeFileDefinedInfoset;
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
gbtGameInfoset gbtTreeFilePlayer::GetInfoset(int p_number)
{
  for (gbtTreeFileDefinedInfoset *infoset = m_firstInfoset;
       infoset; infoset = infoset->m_next) {
    if (infoset->m_fileID == p_number) {
      return infoset->m_infoset;
    }
  }

  return 0;
}

class gbtTreeFileDefinedOutcome {
public:
  int m_fileID;
  gbtGameOutcome m_outcome;

  gbtTreeFileDefinedOutcome(int p_number, gbtGameOutcome p_outcome)
    : m_fileID(p_number), m_outcome(p_outcome) { }
};

class gbtTreeFileGame {
public:
  std::string m_title;
  std::string m_comment;
  gbtTreeFilePlayer *m_firstPlayer, *m_lastPlayer, m_chancePlayer;
  gbtTreeFileNode *m_firstNode, *m_lastNode;
  gbtBlock<gbtTreeFileDefinedOutcome *> m_outcomes;

  gbtTreeFileGame(void);
  ~gbtTreeFileGame();

  void AddPlayer(const std::string &);
  gbtTreeFileNode *AddNode(const std::string &, int, int);
  gbtGameOutcome GetOutcome(int p_number) const;
};

gbtTreeFileGame::gbtTreeFileGame(void)
  : m_title(""), m_comment(""), m_firstPlayer(0), m_lastPlayer(0),
    m_firstNode(0), m_lastNode(0)
{ }

gbtTreeFileGame::~gbtTreeFileGame()
{
  if (m_firstPlayer) {
    gbtTreeFilePlayer *player = m_firstPlayer;
    while (player) {
      gbtTreeFilePlayer *nextPlayer = player->m_next;
      delete player;
      player = nextPlayer;
    }
  }

  if (m_firstNode) {
    gbtTreeFileNode *node = m_firstNode;
    while (node) {
      gbtTreeFileNode *nextNode = node->m_next;
      delete node;
      node = nextNode;
    }
  }

  for (int outc = 1; outc <= m_outcomes.Length(); outc++) {
    delete m_outcomes[outc];
  }
}

void gbtTreeFileGame::AddPlayer(const std::string &p_player)
{
  gbtTreeFilePlayer *player = new gbtTreeFilePlayer;
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

gbtTreeFileNode *gbtTreeFileGame::AddNode(const std::string &p_name, int p_player, int p_infoset)
{
  gbtTreeFileNode *node = new gbtTreeFileNode;
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
gbtGameOutcome gbtTreeFileGame::GetOutcome(int p_number) const
{
  for (int outc = 1; outc <= m_outcomes.Length(); outc++) {
    if (m_outcomes[outc]->m_fileID == p_number) {
      return m_outcomes[outc]->m_outcome;
    }
  }
  return 0;
}

static void ReadPlayers(gbtGameParserState &p_state, gbtTreeFileGame &p_treeData)
{
  if (p_state.GetNextSymbol() != symLBRACE) {
    throw gbtEfgParserException();
  }

  while (p_state.GetNextSymbol() == symTEXT) {
    p_treeData.AddPlayer(p_state.GetLastText());
  }

  if (p_state.GetCurrentSymbol() != symRBRACE) {
    throw gbtEfgParserException();
  }
}

static void ParseOutcome(gbtGameParserState &p_state, gbtTreeFileGame &p_treeData, 
			 gbtTreeFileNode *p_node)
{
  if (p_state.GetCurrentSymbol() == symTEXT) {
    p_node->m_outcomeData = new gbtTreeFileOutcome(p_state.GetLastText());

    if (p_state.GetNextSymbol() != symLBRACE) {
      throw gbtEfgParserException();
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
	throw gbtEfgParserException();
      }

      if (p_state.GetNextSymbol() == symCOMMA) {
	p_state.GetNextSymbol();
      }
    } while (p_state.GetCurrentSymbol() != symRBRACE);
    p_state.GetNextSymbol();
  }
}

static void ParseChanceNode(gbtGameParserState &p_state, gbtTreeFileGame &p_treeData)
{
  if (p_state.GetNextSymbol() != symTEXT) {
    throw gbtEfgParserException();
  }
  std::string nodeName = p_state.GetLastText();

  if (p_state.GetNextSymbol() != symINTEGER) {
    throw gbtEfgParserException();
  }
  int nodeInfoset = p_state.GetLastInteger().as_long();

  gbtTreeFileNode *node = p_treeData.AddNode(nodeName, 0, nodeInfoset);

  p_state.GetNextSymbol();

  if (p_state.GetCurrentSymbol() == symTEXT) {
    // information set name is specified
    gbtTreeFileInfoset *infoset = node->AddInfoset(p_state.GetLastText());

    if (p_state.GetNextSymbol() != symLBRACE) {
      throw gbtEfgParserException();
    }
    p_state.GetNextSymbol();
    do {
      if (p_state.GetCurrentSymbol() != symTEXT) {
	throw gbtEfgParserException();
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
	throw gbtEfgParserException();
      }

      p_state.GetNextSymbol();
    } while (p_state.GetCurrentSymbol() != symRBRACE);
    p_state.GetNextSymbol();
  }

  if (p_state.GetCurrentSymbol() != symINTEGER) {
    throw gbtEfgParserException();
  }
  node->m_outcome = p_state.GetLastInteger().as_long();

  p_state.GetNextSymbol();
  ParseOutcome(p_state, p_treeData, node);
}

static void ParsePersonalNode(gbtGameParserState &p_state, gbtTreeFileGame &p_treeData)
{
  if (p_state.GetNextSymbol() != symTEXT) {
    throw gbtEfgParserException();
  }
  std::string nodeName = p_state.GetLastText();

  if (p_state.GetNextSymbol() != symINTEGER) {
    throw gbtEfgParserException();
  }
  int nodePlayer = p_state.GetLastInteger().as_long();

  if (p_state.GetNextSymbol() != symINTEGER) {
    throw gbtEfgParserException();
  }
  int nodeInfoset = p_state.GetLastInteger().as_long();

  gbtTreeFileNode *node = p_treeData.AddNode(nodeName, nodePlayer, nodeInfoset);

  p_state.GetNextSymbol();
  if (p_state.GetCurrentSymbol() == symTEXT) {
    // information set name is specified
    gbtTreeFileInfoset *infoset = node->AddInfoset(p_state.GetLastText());

    if (p_state.GetNextSymbol() != symLBRACE) {
      throw gbtEfgParserException();
    }
    p_state.GetNextSymbol();
    do {
      if (p_state.GetCurrentSymbol() != symTEXT) {
	throw gbtEfgParserException();
      }
      infoset->AddAction(p_state.GetLastText());

      p_state.GetNextSymbol();
    } while (p_state.GetCurrentSymbol() != symRBRACE);
    p_state.GetNextSymbol();
  }

  if (p_state.GetCurrentSymbol() != symINTEGER) {
    throw gbtEfgParserException();
  }
  node->m_outcome = p_state.GetLastInteger().as_long();

  p_state.GetNextSymbol();
  ParseOutcome(p_state, p_treeData, node);
}

static void ParseTerminalNode(gbtGameParserState &p_state, gbtTreeFileGame &p_treeData)
{
  if (p_state.GetNextSymbol() != symTEXT) {
    throw gbtEfgParserException();
  }
  
  gbtTreeFileNode *node = p_treeData.AddNode(p_state.GetLastText(), -1, -1);

  if (p_state.GetNextSymbol() != symINTEGER) {
    throw gbtEfgParserException();
  }
  node->m_outcome = p_state.GetLastInteger().as_long();

  p_state.GetNextSymbol();
  ParseOutcome(p_state, p_treeData, node);
}

static void Parse(gbtGameParserState &p_state, gbtTreeFileGame &p_treeData)
{
  gbtGameParserSymbol symbol;

  if (p_state.GetNextSymbol() != symEFG) {
    throw gbtEfgParserException();
  }
  if (p_state.GetNextSymbol() != symINTEGER) {
    throw gbtEfgParserException();
  }
  if (p_state.GetLastInteger() != 2) {
    throw gbtEfgParserException();
  }

  symbol = p_state.GetNextSymbol();
  if (symbol != symD && symbol != symR) {
    throw gbtEfgParserException();
  }
  if (p_state.GetNextSymbol() != symTEXT) {
    throw gbtEfgParserException();
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
      throw gbtEfgParserException();
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

static void BuildSubtree(gbtGame p_efg, gbtGameNode p_node,
			 gbtTreeFileGame &p_treeData, gbtTreeFileNode **p_nodeData)
{
  p_node->SetLabel((*p_nodeData)->m_name);

  if ((*p_nodeData)->m_outcome > 0) {
    if (!p_treeData.GetOutcome((*p_nodeData)->m_outcome).IsNull()) {
      p_node->SetOutcome(p_treeData.GetOutcome((*p_nodeData)->m_outcome));
    }
    else {
      gbtGameOutcome outcome = p_efg->NewOutcome();
      outcome->SetLabel((*p_nodeData)->m_outcomeData->m_name);
      p_treeData.m_outcomes.Append(new gbtTreeFileDefinedOutcome((*p_nodeData)->m_outcome,
							  outcome));
      p_node->SetOutcome(outcome);
      for (int pl = 1; pl <= p_efg->NumPlayers(); pl++) {
	outcome->SetPayoff(p_efg->GetPlayer(pl),
			   (*p_nodeData)->m_outcomeData->m_payoffs[pl]);
      }
    }
  }

  if ((*p_nodeData)->m_player > 0) {
    gbtTreeFilePlayer *player = p_treeData.m_firstPlayer;
    for (int i = 1; i < (*p_nodeData)->m_player; i++, player = player->m_next);

    if (!player->GetInfoset((*p_nodeData)->m_infoset).IsNull()) {
      gbtGameInfoset infoset = player->GetInfoset((*p_nodeData)->m_infoset);
      p_node->InsertMove(infoset);
    }
    else {
      gbtGameInfoset infoset = 
	p_efg->GetPlayer((*p_nodeData)->m_player)->NewInfoset((*p_nodeData)->m_infosetData->m_actions.Length());
      p_node->InsertMove(infoset);
      infoset->SetLabel((*p_nodeData)->m_infosetData->m_name);
      for (int act = 1; act <= infoset->NumActions(); act++) {
	infoset->GetAction(act)->SetLabel((*p_nodeData)->m_infosetData->m_actions[act]);
      }
      player->AddInfoset((*p_nodeData)->m_infoset, infoset);
    }

    // Do this because of the semantics of InsertMove()
    p_node = p_node->GetParent();

    *(p_nodeData) = (*(p_nodeData))->m_next;
    for (int i = 1; i <= p_node->NumChildren(); i++) {
      BuildSubtree(p_efg, p_node->GetChild(i), p_treeData, p_nodeData);
    }
  }
  else if ((*p_nodeData)->m_player == 0) {
    gbtTreeFilePlayer *player = &p_treeData.m_chancePlayer;

    if (!player->GetInfoset((*p_nodeData)->m_infoset).IsNull()) {
      gbtGameInfoset infoset = player->GetInfoset((*p_nodeData)->m_infoset);
      p_node->InsertMove(infoset);
    }
    else {
      gbtGameInfoset infoset =
	p_efg->GetChance()->NewInfoset((*p_nodeData)->m_infosetData->m_actions.Length());
      p_node->InsertMove(infoset);
      infoset->SetLabel((*p_nodeData)->m_infosetData->m_name);
      for (int act = 1; act <= infoset->NumActions(); act++) {
	infoset->GetAction(act)->SetLabel((*p_nodeData)->m_infosetData->m_actions[act]);
	infoset->GetAction(act)->SetChanceProb((*p_nodeData)->m_infosetData->m_probs[act]);
      }
    }

    // Do this because of the semantics of InsertMove()
    p_node = p_node->GetParent();

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

static void BuildEfg(gbtTreeGameRep *p_efg, gbtTreeFileGame &p_treeData)
{
  p_efg->SetLabel(p_treeData.m_title);
  p_efg->SetComment(p_treeData.m_comment);
  for (gbtTreeFilePlayer *player = p_treeData.m_firstPlayer; player;
       player = player->m_next) {
    p_efg->NewPlayer()->SetLabel(player->m_name);
  }
  gbtTreeFileNode *node = p_treeData.m_firstNode;
  BuildSubtree(p_efg, p_efg->GetRoot(), p_treeData, &node);
}


//=========================================================================
//  ReadEfg: Global visible function to read an extensive form savefile
//=========================================================================

gbtGame ReadEfg(std::istream &p_file)
{
  gbtGameParserState parser(p_file);
  gbtTreeFileGame treeData;

  try {
    gbtTreeGameRep *efg = new gbtTreeGameRep();
    // Putting this in the handle here ensures a reference to the game
    // is counted while the game is being built
    gbtGame efgHandle(efg);
    Parse(parser, treeData);
    BuildEfg(efg, treeData);
    return efgHandle;
  }
  catch (...) {
    // We'll just lump anything that goes wrong in here as a "parse error"
    throw gbtEfgParserException();
  }
}

