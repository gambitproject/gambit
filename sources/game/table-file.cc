//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Parser for reading normal form savefiles
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
#include "table-game.h"

class gbtTableFilePlayer {
public:
  std::string m_name;
  gbtBlock<std::string> m_strategies;
  gbtTableFilePlayer *m_next;

  gbtTableFilePlayer(void);
};

gbtTableFilePlayer::gbtTableFilePlayer(void)
  : m_next(0)
{ }

class gbtTableFileGame {
public:
  std::string m_title, m_comment;
  gbtTableFilePlayer *m_firstPlayer, *m_lastPlayer;
  int m_numPlayers;

  gbtTableFileGame(void);
  ~gbtTableFileGame();

  void AddPlayer(const std::string &);
  int NumPlayers(void) const { return m_numPlayers; }
  int NumStrategies(int pl) const;
  std::string GetPlayer(int pl) const;
  std::string GetStrategy(int pl, int st) const;
};

gbtTableFileGame::gbtTableFileGame(void)
  : m_firstPlayer(0), m_lastPlayer(0), m_numPlayers(0)
{ }

gbtTableFileGame::~gbtTableFileGame()
{
  if (m_firstPlayer) {
    gbtTableFilePlayer *player = m_firstPlayer;
    while (player) {
      gbtTableFilePlayer *nextPlayer = player->m_next;
      delete player;
      player = nextPlayer;
    }
  }
}

void gbtTableFileGame::AddPlayer(const std::string &p_name)
{
  gbtTableFilePlayer *player = new gbtTableFilePlayer;
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

int gbtTableFileGame::NumStrategies(int p_player) const
{
  gbtTableFilePlayer *player = m_firstPlayer;
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

std::string gbtTableFileGame::GetPlayer(int p_player) const
{
  gbtTableFilePlayer *player = m_firstPlayer;
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

std::string gbtTableFileGame::GetStrategy(int p_player, int p_strategy) const
{
  gbtTableFilePlayer *player = m_firstPlayer;
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

static void ReadPlayers(gbtGameParserState &p_state, gbtTableFileGame &p_data)
{
  if (p_state.GetNextSymbol() != symLBRACE) {
    throw gbtNfgParserException();
  }

  while (p_state.GetNextSymbol() == symTEXT) {
    p_data.AddPlayer(p_state.GetLastText());
  }

  if (p_state.GetCurrentSymbol() != symRBRACE) {
    throw gbtNfgParserException();
  }

  p_state.GetNextSymbol();
}

static void ReadStrategies(gbtGameParserState &p_state, gbtTableFileGame &p_data)
{
  if (p_state.GetCurrentSymbol() != symLBRACE) {
    throw gbtNfgParserException();
  }
  p_state.GetNextSymbol();

  if (p_state.GetCurrentSymbol() == symLBRACE) {
    gbtTableFilePlayer *player = p_data.m_firstPlayer;

    while (p_state.GetCurrentSymbol() == symLBRACE) {
      if (!player) {
	// Not enough players for number of strategy entries
	throw gbtNfgParserException();
      }

      while (p_state.GetNextSymbol() == symTEXT) {
	player->m_strategies.Append(p_state.GetLastText());
      }

      if (p_state.GetCurrentSymbol() != symRBRACE) {
	throw gbtNfgParserException();
      }

      p_state.GetNextSymbol();
      player = player->m_next;
    }

    if (player) {
      // Players with strategies undefined
      throw gbtNfgParserException();
    }

    if (p_state.GetCurrentSymbol() != symRBRACE) {
      throw gbtNfgParserException();
    }

    p_state.GetNextSymbol();
  }
  else if (p_state.GetCurrentSymbol() == symINTEGER) {
    gbtTableFilePlayer *player = p_data.m_firstPlayer;

    while (p_state.GetCurrentSymbol() == symINTEGER) {
      if (!player) {
	// Not enough players for number of strategy entries
	throw gbtNfgParserException();
      }

      for (int st = 1; st <= p_state.GetLastInteger(); st++) {
	player->m_strategies.Append(ToText(st));
      }

      p_state.GetNextSymbol();
      player = player->m_next;
    }

    if (p_state.GetCurrentSymbol() != symRBRACE) {
      throw gbtNfgParserException();
    }

    if (player) {
      // Players with strategies undefined
      throw gbtNfgParserException();
    }

    p_state.GetNextSymbol();
  }
  else {
    throw gbtNfgParserException();
  }
}

static void ParseHeader(gbtGameParserState &p_state, gbtTableFileGame &p_data)
{
  gbtGameParserSymbol symbol;

  if (p_state.GetNextSymbol() != symNFG) {
    throw gbtNfgParserException();
  }
  if (p_state.GetNextSymbol() != symINTEGER) {
    throw gbtNfgParserException();
  }
  if (p_state.GetLastInteger() != 1) {
    throw gbtNfgParserException();
  }

  symbol = p_state.GetNextSymbol();
  if (symbol != symD && symbol != symR) {
    throw gbtNfgParserException();
  }
  if (p_state.GetNextSymbol() != symTEXT) {
    throw gbtNfgParserException();
  }
  p_data.m_title = p_state.GetLastText();

  ReadPlayers(p_state, p_data);
  ReadStrategies(p_state, p_data);

  if (p_state.GetCurrentSymbol() == symTEXT) {
    // Read optional comment
    p_data.m_comment = p_state.GetLastText();
    p_state.GetNextSymbol();
  }
}

static void ReadOutcomeList(gbtGameParserState &p_parser, gbtGame p_nfg)
{
  if (p_parser.GetNextSymbol() != symLBRACE) {
    throw gbtNfgParserException();
  }

  while (p_parser.GetCurrentSymbol() == symLBRACE) {
    gbtArray<gbtRational> payoffs(p_nfg->NumPlayers());
    int pl = 1;

    if (p_parser.GetNextSymbol() != symTEXT) {
      throw gbtNfgParserException();
    }
    std::string label = p_parser.GetLastText();
    p_parser.GetNextSymbol();

    while (p_parser.GetCurrentSymbol() == symINTEGER ||
	   p_parser.GetCurrentSymbol() == symRATIONAL ||
	   p_parser.GetCurrentSymbol() == symDOUBLE) {
      if (pl > p_nfg->NumPlayers()) {
	throw gbtNfgParserException();
      }

      if (p_parser.GetCurrentSymbol() == symINTEGER) {
	payoffs[pl++] = p_parser.GetLastInteger();
      }
      else if (p_parser.GetCurrentSymbol() == symRATIONAL) {
	payoffs[pl++] = p_parser.GetLastRational();
      }
      else {
	payoffs[pl++] = p_parser.GetLastDouble();
      }
      if (p_parser.GetNextSymbol() == symCOMMA) {
	p_parser.GetNextSymbol();
      }
    }

    if (pl <= p_nfg->NumPlayers()) {
      throw gbtNfgParserException();
    }

    if (p_parser.GetCurrentSymbol() != symRBRACE) {
      throw gbtNfgParserException();
    }

    gbtGameOutcome outcome = p_nfg->NewOutcome();
    outcome->SetLabel(label);
    for (pl = 1; pl <= p_nfg->NumPlayers(); pl++) {
      outcome->SetPayoff(p_nfg->GetPlayer(pl), payoffs[pl]);
    }

    p_parser.GetNextSymbol();
  }

  if (p_parser.GetCurrentSymbol() != symRBRACE) {
    throw gbtNfgParserException();
  }
  p_parser.GetNextSymbol();
}

static void ParseOutcomeBody(gbtGameParserState &p_parser,
			     gbtTableGameRep *p_nfg)
{
  ReadOutcomeList(p_parser, p_nfg);

  int cont = 1;

  while (p_parser.GetCurrentSymbol() != symEOF) {
    if (p_parser.GetCurrentSymbol() != symINTEGER) {
      throw gbtNfgParserException();
    }

    p_nfg->m_results[cont++] = p_nfg->m_outcomes[p_parser.GetLastInteger().as_long()];
    p_parser.GetNextSymbol();
  }
}

static void SetPayoff(gbtTableGameRep *p_nfg,
		      int p_cont, int p_pl, const gbtRational &p_value)
{
  if (p_pl == 1)  {
    p_nfg->NewOutcome();
    p_nfg->m_results[p_cont] = p_nfg->m_outcomes[p_nfg->m_outcomes.Length()];
  }
  p_nfg->m_results[p_cont]->m_payoffs[p_pl] = p_value;
}

static void ParsePayoffBody(gbtGameParserState &p_parser, 
			    gbtTableGameRep *p_nfg)
{
  int cont = 1, pl = 1;

  while (p_parser.GetCurrentSymbol() != symEOF) {
    if (p_parser.GetCurrentSymbol() == symINTEGER) {
      SetPayoff(p_nfg, cont, pl, p_parser.GetLastInteger());
    }
    else if (p_parser.GetCurrentSymbol() == symDOUBLE) {
      SetPayoff(p_nfg, cont, pl, p_parser.GetLastDouble());
    }
    else if (p_parser.GetCurrentSymbol() == symRATIONAL) {
      SetPayoff(p_nfg, cont, pl, p_parser.GetLastRational());
    }
    else {
      throw gbtNfgParserException();
    }

    if (++pl > p_nfg->NumPlayers()) {
      cont++;
      pl = 1;
    }
    p_parser.GetNextSymbol();
  }
}

static gbtGame BuildNfg(gbtGameParserState &p_parser, gbtTableFileGame &p_data)
{
  gbtArray<int> dim(p_data.NumPlayers());
  for (int pl = 1; pl <= dim.Length(); pl++) {
    dim[pl] = p_data.NumStrategies(pl);
  }
  gbtTableGameRep *nfg = new gbtTableGameRep(dim);
  // Putting this in the handle here ensures a reference to the game
  // is counted while the game is being built.
  gbtGame nfgHandle(nfg);

  nfg->SetLabel(p_data.m_title);
  nfg->SetComment(p_data.m_comment);
  
  for (int pl = 1; pl <= dim.Length(); pl++) {
    nfg->GetPlayer(pl)->SetLabel(p_data.GetPlayer(pl));
    for (int st = 1; st <= dim[pl]; st++) {
      nfg->GetPlayer(pl)->GetStrategy(st)->SetLabel(p_data.GetStrategy(pl,st));
    }
  }
  
  if (p_parser.GetCurrentSymbol() == symLBRACE) {
    ParseOutcomeBody(p_parser, nfg);
  }
  else if (p_parser.GetCurrentSymbol() == symDOUBLE ||
	   p_parser.GetCurrentSymbol() == symINTEGER ||
	   p_parser.GetCurrentSymbol() == symRATIONAL) {
    ParsePayoffBody(p_parser, nfg);
  }
  else {
    throw gbtNfgParserException();
  }

  return nfgHandle;
}

//=========================================================================
//   ReadNfg: Global visible function to read a normal form savefile
//=========================================================================

gbtGame ReadNfg(std::istream &p_file)
{
  gbtGameParserState parser(p_file);
  gbtTableFileGame data;

  try {
    ParseHeader(parser, data);
    return BuildNfg(parser, data);
  }
  catch (...) {
    // We'll just lump anything that goes wrong in here as a "parse error"
    throw gbtNfgParserException();
  }
}
