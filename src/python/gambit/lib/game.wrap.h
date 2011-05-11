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

inline bool lt_Number(const Number *x, const Number *y)
{ return (static_cast<const Rational &>(*x) < 
	  static_cast<const Rational &>(*y)); }
inline bool le_Number(const Number *x, const Number *y)
{ return (static_cast<const Rational &>(*x) <= 
	  static_cast<const Rational &>(*y)); }
inline bool eq_Number(const Number *x, const Number *y)
{ return (static_cast<const Rational &>(*x) == 
	  static_cast<const Rational &>(*y)); }
inline bool ne_Number(const Number *x, const Number *y)
{ return (static_cast<const Rational &>(*x) != 
	  static_cast<const Rational &>(*y)); }
inline bool gt_Number(const Number *x, const Number *y)
{ return (static_cast<const Rational &>(*x) > 
	  static_cast<const Rational &>(*y)); }
inline bool ge_Number(const Number *x, const Number *y)
{ return (static_cast<const Rational &>(*x) >= 
	  static_cast<const Rational &>(*y)); }
inline Number add_Number(const Number *x, const Number *y)
{ return Number(lexical_cast<std::string>(static_cast<const Rational &>(*x) + 
					  static_cast<const Rational &>(*y))); }
inline Number sub_Number(const Number *x, const Number *y)
{ return Number(lexical_cast<std::string>(static_cast<const Rational &>(*x) - 
					  static_cast<const Rational &>(*y))); }
inline Number mul_Number(const Number *x, const Number *y)
{ return Number(lexical_cast<std::string>(static_cast<const Rational &>(*x) * 
					  static_cast<const Rational &>(*y))); }
inline Number div_Number(const Number *x, const Number *y)
{ return Number(lexical_cast<std::string>(static_cast<const Rational &>(*x) / 
					  static_cast<const Rational &>(*y))); }


inline void setitem_ArrayInt(Array<int> *array, int index, int value)
{ (*array)[index] = value; }

inline void setitem_MixedStrategyProfileDouble_int(MixedStrategyProfile<double> *profile,
						   int index, double value)
{ (*profile)[index] = value; }

inline void setitem_MixedStrategyProfileDouble_Strategy(MixedStrategyProfile<double> *profile,
							GameStrategy index, double value)
{ (*profile)[index] = value; }
