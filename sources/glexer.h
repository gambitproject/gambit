//
// FILE: glexer.h -- Declaration of generic lexer class
//
// $Id$
//

#ifndef GLEXER_H
#define GLEXER_H

#include "gstream.h"
#include "gtext.h"
#include "gnumber.h"

class gLexer {
private:
  gInput &m_file;

public:
  class Unexpected : public gException {
  public:
    virtual ~Unexpected() { }
    gText Description(void) const { return "Unexpected input"; }
  };

  gLexer(gInput &p_file) : m_file(p_file) { }
  virtual ~gLexer() { }

  gText GetString(void);
  gNumber GetNumber(void);
  void GetToken(const gText &);
};

#endif  // GLEXER_H
