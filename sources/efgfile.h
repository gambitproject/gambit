//
// FILE: efgfile.h -- Read in .efg file and create Efg object
//
// $Id$
//

#ifndef EFGFILE_H
#define EFGFILE_H

#include "glexer.h"
#include "efg.h"

class EfgFile : public gLexer {
private:
  gList<gText> ReadPlayers(void);

public:
  EfgFile(gInput &);
  virtual ~EfgFile();

  FullEfg *Read(void);
};

#endif  // EFGFILE_H
