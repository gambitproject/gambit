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
#include <sstream>

#include "libgambit.h"

typedef enum {
  symINTEGER = 0, symDOUBLE = 1, symRATIONAL = 2, symTEXT = 3,
  symEFG = 4, symNFG = 5, symD = 6, symR = 7, symC = 8, symP = 9, symT = 10,
  symLBRACE = 11, symRBRACE = 12, symCOMMA = 13, symEOF = 14
} gbtGameParserSymbol;

//!
//! This parser class implements the semantics of Gambit savefiles,
//! including the nonsignificance of whitespace and the possibility of
//! escaped-quotes within text labels.
//!
class gbtGameParserState {
private:
  std::istream &m_file;

  int m_currentLine;
  gbtGameParserSymbol m_lastSymbol;
  double m_lastDouble;
  gbtInteger m_lastInteger;
  gbtRational m_lastRational;
  std::string m_lastText;

public:
  gbtGameParserState(std::istream &p_file) : m_file(p_file), m_currentLine(1) { }

  gbtGameParserSymbol GetNextSymbol(void);
  gbtGameParserSymbol GetCurrentSymbol(void) const { return m_lastSymbol; }
  int GetCurrentLine(void) const { return m_currentLine; }
  const gbtInteger &GetLastInteger(void) const { return m_lastInteger; }
  const gbtRational &GetLastRational(void) const { return m_lastRational; }
  double GetLastDouble(void) const { return m_lastDouble; }
  std::string GetLastText(void) const { return m_lastText; }
};  

class gbtGameParserException : public gbtException {
private:
  std::string m_description;
  
public:
  gbtGameParserException(void) 
    : m_description("Error in getting symbol") { }
  gbtGameParserException(int p_line, const std::string &p_description);
  virtual ~gbtGameParserException() { }

  std::string GetDescription(void) const { return m_description; }
};

gbtGameParserSymbol gbtGameParserState::GetNextSymbol(void)
{
  char c = ' '; 
  if (m_file.eof()) {
    return (m_lastSymbol = symEOF);
  }

  while (isspace(c)) {
    m_file.get(c);
    if (!m_file.good()) {
      return (m_lastSymbol = symEOF);
    } 
    else if (c == '\n') {
      m_currentLine++;
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
      	  throw gbtGameParserException(m_currentLine, "Keyword 'NFG' must be followed by whitespace.");
      	}
      	else {
      	  return (m_lastSymbol = symNFG);
      	}
      }
    }
    throw gbtGameParserException(m_currentLine, "Keyword 'NFG' expected.");
  }
  else if (c == 'E') {
    m_file.get(c);
    if (c == 'F') {
      m_file.get(c);
      if (c == 'G') {
      	m_file.get(c);
      	if (!isspace(c)) {
      	  throw gbtGameParserException(m_currentLine, "'EFG' must be followed by whitespace.");
      	}
      	else {
      	  return (m_lastSymbol = symEFG);
      	}
      }
    }
    throw gbtGameParserException(m_currentLine, "Keyword 'EFG' expected.");
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
    std::string buf;
    buf += c;
    m_file.get(c);

    while (!m_file.eof() && m_file.good() && isdigit(c)) {
      buf += c;
      m_file.get(c);
    }
    
    if (m_file.eof() || !m_file.good()) {
      m_lastInteger = atoi(buf.c_str());
      m_lastText = buf;
      return (m_lastSymbol = symINTEGER);
    }      

    if (c == '.') {
      buf += c;
      m_file.get(c);
      while (isdigit(c)) {
      	buf += c;
      	m_file.get(c);
      }

      // This next segment was contributed by Ryan Porter to
      // permit the use of scientific notation for floating point
      // values in the files.
      int digit, exp;
      double multiplier = 1.0;
      bool multexp = true;
      if (c == 'E') {
      	m_file.get(c);
      	digit = ((int) c) - 48;
      	if (c == '-') {
      	  multexp = false;
      	  m_file.get(c);
      	  digit = ((int) c) - 48;
      	}
      	exp = 0;
      	while ((digit >= 0) && (digit <= 9)) {
      	  exp = exp * 10 + digit;
      	  m_file.get(c);
      	  digit = ((int) c) - 48;
      	}
      	for (int e = 1; e <= exp; e++) {
      	  if (multexp) {
      	    multiplier *= 10.0;
      	  }
      	  else {
      	    multiplier /= 10.0;
      	  }
      	}
      }

      // Next two lines are from original code
      m_file.unget();
      m_lastDouble = strtod(buf.c_str(), 0);

      m_lastDouble *= multiplier;
      // End of Ryan's changes
      m_lastText = buf;

      return (m_lastSymbol = symDOUBLE);
    }
    else if (c == '/') {
      buf += c;
      m_file.get(c);
      while (isdigit(c)) {
      	buf += c;
      	m_file.get(c);
      }
      m_file.unget();
      m_lastRational = ToRational(buf);
      m_lastText = buf;
      return (m_lastSymbol = symRATIONAL);
    }
      
    else {
      m_file.unget();

      m_lastInteger = atoi(buf.c_str());
      m_lastText = buf;
      return (m_lastSymbol = symINTEGER);
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

    m_lastDouble = strtod(buf.c_str(), 0);
    m_lastText = buf;
    return (m_lastSymbol = symDOUBLE);
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

    return (m_lastSymbol = symTEXT);
  }

  std::ostringstream out;
  out << "Don't know what to do with keyword '" << c << "'.";
  throw gbtGameParserException(m_currentLine, out.str());
}


gbtGameParserException::gbtGameParserException(int p_line, 
					       const std::string &p_description) 
{
  std::ostringstream out;
  out << "Error in line " << p_line << ": " << p_description;
  m_description = out.str();
}


class gbtTableFilePlayer {
public:
  std::string m_name;
  gbtArray<std::string> m_strategies;
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
	player->m_strategies.Append(ToText(gbtInteger(st)));
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

static void ReadOutcomeList(gbtGameParserState &p_parser, gbtNfgGame *p_nfg)
{
  if (p_parser.GetNextSymbol() == symRBRACE) {
    // Special case: empty outcome list
    p_parser.GetNextSymbol();
    return;
  }

  if (p_parser.GetCurrentSymbol() != symLBRACE) {
    throw gbtNfgParserException();
  }

  while (p_parser.GetCurrentSymbol() == symLBRACE) {
    gbtArray<std::string> payoffs(p_nfg->NumPlayers());
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
	payoffs[pl++] = p_parser.GetLastText();
      }
      else if (p_parser.GetCurrentSymbol() == symRATIONAL) {
	payoffs[pl++] = p_parser.GetLastText();
      }
      else {
	payoffs[pl++] = p_parser.GetLastText();
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

    gbtNfgOutcome outcome = p_nfg->NewOutcome();
    outcome->SetName(label);
    for (pl = 1; pl <= p_nfg->NumPlayers(); pl++) {
      outcome->SetPayoff(pl, payoffs[pl]);
    }

    p_parser.GetNextSymbol();
  }

  if (p_parser.GetCurrentSymbol() != symRBRACE) {
    throw gbtNfgParserException();
  }
  p_parser.GetNextSymbol();
}

void ParseOutcomeBody(gbtGameParserState &p_parser, gbtNfgGame *p_nfg)
{
  ReadOutcomeList(p_parser, p_nfg);

  int cont = 1;

  while (p_parser.GetCurrentSymbol() != symEOF) {
    if (p_parser.GetCurrentSymbol() != symINTEGER) {
      throw gbtNfgParserException();
    }

    int outcomeId = p_parser.GetLastInteger().as_long();
    if (outcomeId > 0)  {
      p_nfg->results[cont++] = p_nfg->GetOutcome(outcomeId);
    }
    else {
      p_nfg->results[cont++] = 0;
    }
    p_parser.GetNextSymbol();
  }
}

void SetPayoff(gbtNfgGame *p_nfg, int p_cont, int p_pl, 
	       const std::string &p_text)
{
  if (p_pl == 1)  {
    p_nfg->NewOutcome();
    p_nfg->results[p_cont] = p_nfg->GetOutcome(p_nfg->NumOutcomes());
  }
  p_nfg->results[p_cont]->SetPayoff(p_pl, p_text);
}

static void ParsePayoffBody(gbtGameParserState &p_parser, 
			    gbtNfgGame *p_nfg)
{
  int cont = 1, pl = 1;

  while (p_parser.GetCurrentSymbol() != symEOF) {
    if (p_parser.GetCurrentSymbol() == symINTEGER) {
      SetPayoff(p_nfg, cont, pl, p_parser.GetLastText());
    }
    else if (p_parser.GetCurrentSymbol() == symDOUBLE) {
      SetPayoff(p_nfg, cont, pl, p_parser.GetLastText());
    }
    else if (p_parser.GetCurrentSymbol() == symRATIONAL) {
      SetPayoff(p_nfg, cont, pl, p_parser.GetLastText());
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

static gbtNfgGame *BuildNfg(gbtGameParserState &p_parser, 
			    gbtTableFileGame &p_data)
{
  gbtArray<int> dim(p_data.NumPlayers());
  for (int pl = 1; pl <= dim.Length(); pl++) {
    dim[pl] = p_data.NumStrategies(pl);
  }
  gbtNfgGame *nfg = new gbtNfgGame(dim);

  nfg->SetTitle(p_data.m_title);
  nfg->SetComment(p_data.m_comment);
  
  for (int pl = 1; pl <= dim.Length(); pl++) {
    nfg->GetPlayer(pl)->SetName(p_data.GetPlayer(pl));
    for (int st = 1; st <= dim[pl]; st++) {
      nfg->GetPlayer(pl)->GetStrategy(st)->SetName(p_data.GetStrategy(pl,st));
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

  return nfg;
}

//=========================================================================
//   ReadNfg: Global visible function to read a normal form savefile
//=========================================================================

gbtNfgGame *ReadNfg(std::istream &p_file)
{
  gbtGameParserState parser(p_file);
  gbtTableFileGame data;

  try {
    ParseHeader(parser, data);
    return BuildNfg(parser, data);
  }
  catch (gbtGameParserException& e) {
    throw gbtNfgParserException(e.GetDescription());
  }
  catch (gbtNfgParserException& e) {
    throw e;
  }
  catch (...) {
    // We'll just lump anything that goes wrong in here as a "parse error"
    throw gbtNfgParserException();
  }

  return 0;
}


gbtNfgParserException::gbtNfgParserException(int p_line, 
					     const std::string &p_description)
{
  std::ostringstream out;
  out << "Error in line " << p_line << ": " << p_description;
  m_description = out.str();
}
