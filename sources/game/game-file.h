//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Parser for reading Gambit savefiles
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

#ifndef GAME_FILE_H
#define GAME_FILE_H

#include <iostream>
#include "base/base.h"
#include "math/rational.h"

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

  gbtGameParserSymbol m_lastSymbol;
  double m_lastDouble;
  gbtInteger m_lastInteger;
  gbtRational m_lastRational;
  std::string m_lastText;

public:
  gbtGameParserState(std::istream &p_file) : m_file(p_file) { }

  gbtGameParserSymbol GetNextSymbol(void);
  gbtGameParserSymbol GetCurrentSymbol(void) const { return m_lastSymbol; }
  const gbtInteger &GetLastInteger(void) const { return m_lastInteger; }
  const gbtRational &GetLastRational(void) const { return m_lastRational; }
  double GetLastDouble(void) const { return m_lastDouble; }
  std::string GetLastText(void) const { return m_lastText; }
};  

class gbtGameParserException : public gbtException {
public:
  virtual ~gbtGameParserException() { }
  std::string GetDescription(void) const { return "Error in getting symbol"; }
};

#endif  // GAME_FILE_H
