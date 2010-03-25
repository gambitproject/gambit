//
// This file is used by the Cython wrappers to import the necessary namespaces
// and to provide some convenience functions to make interfacing with C++
// classes easier.
//

#include <string>
#include <fstream>
#include <sstream>
#include "libgambit/libgambit.h"

using namespace std;
using namespace Gambit;

Game ReadGame(char *fn) throw (InvalidFileException)
{ 
  std::ifstream f(fn);
  return Gambit::ReadGame(f);
}

std::string WriteGame(const Game &p_game, bool strategic)
{
  std::ostringstream f;
  if (strategic) {
    p_game->WriteNfgFile(f);
  }
  else {
    p_game->WriteEfgFile(f);
  }
  return f.str();
}        

