//
// FILE: glexer.cc -- Implementation of generic lexer class
//
// $Id$
//

#include <ctype.h>
#include "glexer.h"

gText gLexer::GetString(void)
{
  char c;

  do {
    m_file >> c;
  } while (isspace(c));

  if (c == '"') {
    gText string;
    m_file.unget(c);
    m_file >> string;
    return string;
  }
  else {
    throw Unexpected();
  }
}

gNumber gLexer::GetNumber(void)
{
  char c;

  do {
    m_file >> c;
  } while (isspace(c));

  if (isdigit(c) || c == '-') {
    gNumber number;
    m_file.unget(c);
    m_file >> number;
    return number;
  }
  else {
    throw Unexpected();
  }
}

void gLexer::GetToken(const gText &p_token)
{
  char c;
  
  do {
    m_file >> c;
  } while (isspace(c));

  gText token;
  m_file.unget(c);
  m_file >> token;

  if (token != p_token) {
    throw Unexpected();
  }
}
