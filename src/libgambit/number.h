//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/number.h
// A simple class for storing numerical data in a game
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

#ifndef LIBGAMBIT_NUMBER_H
#define LIBGAMBIT_NUMBER_H

namespace Gambit {

/// This simple class stores a numerical datum.
class Number {
private:
  std::string m_text;
  Rational m_rational;
  double m_double;

public:
  Number(void)
    : m_text("0"), m_rational(0), m_double(0.0) { }
  Number(const std::string &p_text)
    : m_text(p_text), m_rational(lexical_cast<Rational>(p_text)), 
      m_double((double) m_rational)
  { }
  
  Number &operator=(const std::string &p_text)
  {
    // We call lexical_cast<Rational>() first because it throws a ValueException
    // if the conversion of the text fails
    m_rational = lexical_cast<Rational>(p_text);
    m_text = p_text;
    m_double = (double) m_rational;
    return *this; 
  }

  operator const double &(void) const { return m_double; }
  operator const Rational &(void) const { return m_rational; }
  operator const std::string &(void) const { return m_text; }
};

}

#endif // LIBGAMBIT_NUMBER_H
