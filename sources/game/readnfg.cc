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
#include "nfg.h"

class gbtNfgFilePlayer {
public:
  gText m_name;
  gBlock<gText> m_strategies;
  gbtNfgFilePlayer *m_next;

  gbtNfgFilePlayer(void);
};

gbtNfgFilePlayer::gbtNfgFilePlayer(void)
  : m_next(0)
{ }

class gbtNfgFileData {
public:
  gText m_title, m_comment;
  gbtNfgFilePlayer *m_firstPlayer, *m_lastPlayer;
  int m_numPlayers;

  gbtNfgFileData(void);
  ~gbtNfgFileData();

  void AddPlayer(const gText &);
  int NumPlayers(void) const { return m_numPlayers; }
  int NumStrategies(int pl) const;
  gText GetStrategy(int pl, int st) const;
};

gbtNfgFileData::gbtNfgFileData(void)
  : m_firstPlayer(0), m_lastPlayer(0), m_numPlayers(0)
{ }

gbtNfgFileData::~gbtNfgFileData()
{
  if (m_firstPlayer) {
    gbtNfgFilePlayer *player = m_firstPlayer;
    while (player) {
      gbtNfgFilePlayer *nextPlayer = player->m_next;
      delete player;
      player = nextPlayer;
    }
  }
}

void gbtNfgFileData::AddPlayer(const gText &p_name)
{
  gbtNfgFilePlayer *player = new gbtNfgFilePlayer;
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

int gbtNfgFileData::NumStrategies(int p_player) const
{
  gbtNfgFilePlayer *player = m_firstPlayer;
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

gText gbtNfgFileData::GetStrategy(int p_player, int p_strategy) const
{
  gbtNfgFilePlayer *player = m_firstPlayer;
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

//=========================================================================
//                 Parser classes and static functions   
//=========================================================================

//
// These classes and functions are responsible for parsing the
// savefile and building the temporary data structures
//

typedef enum {
  symINTEGER = 0, symDOUBLE = 1, symRATIONAL = 2, symTEXT = 3,
  symNFG = 4, symD = 5, symR = 6, symC = 7, symP = 8, symT = 9,
  symLBRACE = 10, symRBRACE = 11, symCOMMA = 12, symEOF = 13
} gbtNfgParserSymbol;

class gbtNfgParserState {
private:
  gInput &m_file;

  gbtNfgParserSymbol m_lastSymbol;
  double m_lastDouble;
  gInteger m_lastInteger;
  gRational m_lastRational;
  gText m_lastText;

public:
  gbtNfgParserState(gInput &p_file) : m_file(p_file) { }

  gbtNfgParserSymbol GetNextSymbol(void);
  gbtNfgParserSymbol GetCurrentSymbol(void) const { return m_lastSymbol; }
  const gInteger &GetLastInteger(void) const { return m_lastInteger; }
  const gRational &GetLastRational(void) const { return m_lastRational; }
  double GetLastDouble(void) const { return m_lastDouble; }
  gText GetLastText(void) const { return m_lastText; }
};  

class gbtNfgParserError { };

gbtNfgParserSymbol gbtNfgParserState::GetNextSymbol(void)
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
  if (c == 'N') {
    m_file.get(c);
    if (c == 'F') {
      m_file.get(c);
      if (c == 'G') {
	m_file.get(c);
	if (!isspace(c)) {
	  throw gbtNfgParserError();
	}
	else {
	  return (m_lastSymbol = symNFG);
	}
      }
    }
    throw gbtNfgParserError(); 
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

  throw gbtNfgParserError();
}

static void ReadPlayers(gbtNfgParserState &p_state, gbtNfgFileData &p_data)
{
  if (p_state.GetNextSymbol() != symLBRACE) {
    throw gbtNfgParserError();
  }

  while (p_state.GetNextSymbol() == symTEXT) {
    p_data.AddPlayer(p_state.GetLastText());
  }

  if (p_state.GetCurrentSymbol() != symRBRACE) {
    throw gbtNfgParserError();
  }

  p_state.GetNextSymbol();
}

static void ReadStrategies(gbtNfgParserState &p_state, gbtNfgFileData &p_data)
{
  if (p_state.GetCurrentSymbol() != symLBRACE) {
    throw gbtNfgParserError();
  }
  p_state.GetNextSymbol();

  if (p_state.GetCurrentSymbol() == symLBRACE) {
    gbtNfgFilePlayer *player = p_data.m_firstPlayer;

    while (p_state.GetCurrentSymbol() == symLBRACE) {
      if (!player) {
	// Not enough players for number of strategy entries
	throw gbtNfgParserError();
      }

      while (p_state.GetNextSymbol() == symTEXT) {
	player->m_strategies.Append(p_state.GetLastText());
      }

      if (p_state.GetCurrentSymbol() != symRBRACE) {
	throw gbtNfgParserError();
      }

      p_state.GetNextSymbol();
      player = player->m_next;
    }

    if (player) {
      // Players with strategies undefined
      throw gbtNfgParserError();
    }

    if (p_state.GetCurrentSymbol() != symRBRACE) {
      throw gbtNfgParserError();
    }

    p_state.GetNextSymbol();
  }
  else if (p_state.GetCurrentSymbol() == symINTEGER) {
    gbtNfgFilePlayer *player = p_data.m_firstPlayer;

    while (p_state.GetCurrentSymbol() == symINTEGER) {
      if (!player) {
	// Not enough players for number of strategy entries
	throw gbtNfgParserError();
      }

      for (int st = 1; st <= p_state.GetLastInteger(); st++) {
	player->m_strategies.Append(ToText(st));
      }

      p_state.GetNextSymbol();
      player = player->m_next;
    }

    if (p_state.GetCurrentSymbol() != symRBRACE) {
      throw gbtNfgParserError();
    }

    if (player) {
      // Players with strategies undefined
      throw gbtNfgParserError();
    }

    p_state.GetNextSymbol();
  }
  else {
    throw gbtNfgParserError();
  }
}

static void ParseHeader(gbtNfgParserState &p_state, gbtNfgFileData &p_data)
{
  gbtNfgParserSymbol symbol;

  if (p_state.GetNextSymbol() != symNFG) {
    throw gbtNfgParserError();
  }
  if (p_state.GetNextSymbol() != symINTEGER) {
    throw gbtNfgParserError();
  }
  if (p_state.GetLastInteger() != 1) {
    throw gbtNfgParserError();
  }

  symbol = p_state.GetNextSymbol();
  if (symbol != symD && symbol != symR) {
    throw gbtNfgParserError();
  }
  if (p_state.GetNextSymbol() != symTEXT) {
    throw gbtNfgParserError();
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

static void ReadOutcomeList(gbtNfgParserState &p_parser, Nfg *p_nfg)
{
  if (p_parser.GetNextSymbol() != symLBRACE) {
    throw gbtNfgParserError();
  }

  while (p_parser.GetCurrentSymbol() == symLBRACE) {
    gArray<gNumber> payoffs(p_nfg->NumPlayers());
    int pl = 1;

    if (p_parser.GetNextSymbol() != symTEXT) {
      throw gbtNfgParserError();
    }
    gText label = p_parser.GetLastText();
    p_parser.GetNextSymbol();

    while (p_parser.GetCurrentSymbol() == symINTEGER ||
	   p_parser.GetCurrentSymbol() == symRATIONAL ||
	   p_parser.GetCurrentSymbol() == symDOUBLE) {
      if (pl > p_nfg->NumPlayers()) {
	throw gbtNfgParserError();
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
      throw gbtNfgParserError();
    }

    if (p_parser.GetCurrentSymbol() != symRBRACE) {
      throw gbtNfgParserError();
    }

    gbtNfgOutcome outcome = p_nfg->NewOutcome();
    outcome.SetLabel(label);
    for (pl = 1; pl <= p_nfg->NumPlayers(); pl++) {
      outcome.SetPayoff(p_nfg->GetPlayer(pl), payoffs[pl]);
    }

    p_parser.GetNextSymbol();
  }

  if (p_parser.GetCurrentSymbol() != symRBRACE) {
    throw gbtNfgParserError();
  }
  p_parser.GetNextSymbol();
}

static void ParseOutcomeBody(gbtNfgParserState &p_parser, Nfg *p_nfg)
{
  ReadOutcomeList(p_parser, p_nfg);

  int cont = 1;

  while (p_parser.GetCurrentSymbol() != symEOF) {
    if (p_parser.GetCurrentSymbol() != symINTEGER) {
      throw gbtNfgParserError();
    }

    p_nfg->SetOutcomeIndex(cont++,
			   p_nfg->GetOutcomeId(p_parser.GetLastInteger().as_long()));
    p_parser.GetNextSymbol();
  }
}

static void SetPayoff(Nfg *p_nfg, int p_cont, int p_pl, const gNumber &p_value)
{
  if (p_pl == 1)  {
    p_nfg->SetOutcomeIndex(p_cont, p_nfg->NewOutcome());
  }
  p_nfg->GetOutcomeIndex(p_cont).SetPayoff(p_nfg->GetPlayer(p_pl), p_value);
}

static void ParsePayoffBody(gbtNfgParserState &p_parser, Nfg *p_nfg)
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
      throw gbtNfgParserError();
    }

    if (++pl > p_nfg->NumPlayers()) {
      cont++;
      pl = 1;
    }
    p_parser.GetNextSymbol();
  }
}

static Nfg *BuildNfg(gbtNfgParserState &p_parser, gbtNfgFileData &p_data)
{
  Nfg *nfg = 0;

  try {
    gArray<int> dim(p_data.NumPlayers());
    for (int pl = 1; pl <= dim.Length(); pl++) {
      dim[pl] = p_data.NumStrategies(pl);
    }
    nfg = new Nfg(dim);
    nfg->SetTitle(p_data.m_title);
    nfg->SetComment(p_data.m_comment);

    for (int pl = 1; pl <= dim.Length(); pl++) {
      for (int st = 1; st <= dim[pl]; st++) {
	nfg->GetPlayer(pl).GetStrategy(st).SetLabel(p_data.GetStrategy(pl,st));
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
      throw gbtNfgParserError();
    }

    return nfg;
  }
  catch (...) {
    if (nfg) {
      delete nfg;
    }
    throw;
  }
}

//=========================================================================
//  ReadNfgFile: Global visible function to read a normal form savefile
//=========================================================================

Nfg *ReadNfgFile(gInput &p_file)
{
  gbtNfgParserState parser(p_file);
  gbtNfgFileData data;

  try {
    ParseHeader(parser, data);
    return BuildNfg(parser, data);
  }
  catch (gbtNfgParserError &) {
    return 0;
  }
  catch (...) {
    return 0;
  }
}


