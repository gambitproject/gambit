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

inline Game NewTable(Array<int> *dim)
{ return NewTable(*dim); }

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

inline void setitem_ArrayInt(Array<int> *array, int index, int value)
{ (*array)[index] = value; }

inline void 
setitem_MixedStrategyProfileDouble(MixedStrategyProfile<double> *profile,
				   int index, double value)
{ (*profile)[index] = value; }


inline void
setitem_MixedStrategyProfileRational(MixedStrategyProfile<Rational> *profile,
				     int index, const char *value)
{ (*profile)[index] = lexical_cast<Rational>(std::string(value)); }

inline void 
setitem_MixedStrategyProfileDoubleStrategy(MixedStrategyProfile<double> *profile,
				           GameStrategy index, double value)
{ (*profile)[index] = value; }


inline void
setitem_MixedStrategyProfileRationalStrategy(MixedStrategyProfile<Rational> *profile,
				             GameStrategy index, const char *value)
{ (*profile)[index] = lexical_cast<Rational>(std::string(value)); }


inline void 
setitem_MixedBehavProfileDouble(MixedBehavProfile<double> *profile,
           int index, double value)
{ (*profile)[index] = value; }

inline void 
setitem_MixedBehavProfileRational(MixedBehavProfile<Rational> *profile,
           int index, const char *value)
{ (*profile)[index] = lexical_cast<Rational>(std::string(value)); }

inline void 
setaction_MixedBehavProfileDouble(MixedBehavProfile<double> *profile,
           GameAction &action, double value)
{ (*profile)(action) = value; }

inline void 
setaction_MixedBehavProfileRational(MixedBehavProfile<Rational> *profile,
           GameAction &action, const char *value)
{ (*profile)(action) = lexical_cast<Rational>(std::string(value)); }
