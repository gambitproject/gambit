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

#include "game-file.h"

gbtGameParserSymbol gbtGameParserState::GetNextSymbol(void)
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
	  throw gbtGameParserException();
	}
	else {
	  return (m_lastSymbol = symNFG);
	}
      }
    }
    throw gbtGameParserException(); 
  }
  else if (c == 'E') {
    m_file.get(c);
    if (c == 'F') {
      m_file.get(c);
      if (c == 'G') {
	m_file.get(c);
	if (!isspace(c)) {
	  throw gbtGameParserException();
	}
	else {
	  return (m_lastSymbol = symEFG);
	}
      }
    }
    throw gbtGameParserException(); 
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
      m_file.unget();
      m_lastDouble = strtod(buf.c_str(), 0);
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
      FromText(buf, m_lastRational);
      return (m_lastSymbol = symRATIONAL);
    }
      
    else {
      m_file.unget();

      m_lastInteger = atoi(buf.c_str());
      return (m_lastSymbol = symINTEGER);
    }
  }
  else if (c == '"') {
    // We need to do a little magic here, since escaped quotes inside
    // the string are treated as quotes (not end-of-string)
    m_file.unget();
    char a;
  
    m_lastText = "";
  
    do  {
      m_file.get(a);
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
      }  while (!isspace(a));
    }

    return (m_lastSymbol = symTEXT);
  }

  throw gbtGameParserException();
}

