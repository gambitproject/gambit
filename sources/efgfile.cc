//
// FILE: efgfile.cc -- Read in .efg file format and create Efg object
//
// $Id$
//

#include "efgfile.h"

EfgFile::EfgFile(gInput &p_file)
  : gLexer(p_file)
{ }

EfgFile::~EfgFile()
{ }

gList<gText> EfgFile::ReadPlayers(void)
{
  gList<gText> players;

  GetToken("{");
}

FullEfg *EfgFile::Read(void)
{
  FullEfg *efg = new FullEfg;

  try {
    GetToken("EFG");
    GetNumber();    // revision number
    GetToken("D", "R");    // (obsolete) type label

    efg->SetTitle(GetString());
  }
  catch (...) {
    delete efg;
    throw;
  }

  return efg;
}
