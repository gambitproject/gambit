//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/python/gambit/lib/util.h
// Convenience functions for Cython wrapper
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
  return ReadGame(f);
}

Game ParseGame(char *s) throw (InvalidFileException)
{
  std::istringstream f(s);
  return ReadGame(f);
}

std::string WriteGame(const Game &p_game, const std::string &p_format)
{
  std::ostringstream f;
  p_game->Write(f, p_format);
  return f.str();
}        

std::string WriteGame(const StrategySupportProfile &p_support)
{
  std::ostringstream f;
  p_support.WriteNfgFile(f);
  return f.str();
}

// Create a copy on the heap (via new) of the element at index p_index of 
// container p_container.
template <template<class> class C, class T, class X> 
T *copyitem(const C<T> &p_container, const X &p_index)
{ return new T(p_container[p_index]); }

// Set item p_index to value p_value in container p_container
template <class C, class X, class T> 
void setitem(C *p_container, const X &p_index, const T &p_value)
{ (*p_container)[p_index] = p_value; }

// Convert the (C-style) string p_value to a Rational
inline Rational to_rational(const char *p_value)
{ return lexical_cast<Rational>(std::string(p_value)); }
