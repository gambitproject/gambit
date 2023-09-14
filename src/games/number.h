//
// This file is part of Gambit
// Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
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

/// This class represents a numerical data value in a game.
///
/// Numerical data in a game (outcome payoffs and chance move probabilities) are
/// stored using instances of this class.  This class accomplishes two objectives:
/// 1.  The canonical representation of a numerical data value is a rational number.
///     This means that algorithms which compute using rational numbers will produce
///     the expected results reliably.
/// 2.  The text representation of a number is recorded.  Therefore, calling code
///     can use either rational numbers or decimals, and this class will recall that
///     representation correctly.  Therefore, "10.1" is represented internally
///     as Rational(101, 100), but the representation "10.1" is returned by the
///     string, and not "101/100".
class Number {
private:
  std::string m_text;
  Rational m_rational;
  double m_double;

public:
  /// Construct a new number with representation "0"
  Number()
    : m_text("0"), m_rational(0), m_double(0.0) { }

  /// Construct a new number with representation p_text
  /// @param p_text The text representation of the number
  explicit Number(const std::string &p_text)
    : m_text(p_text), m_rational(lexical_cast<Rational>(p_text)), 
      m_double(static_cast<double>(m_rational))
  { }
  /// Construct a new number with rational representation p_rational
  /// @param p_rational The rational representation of the number
  explicit Number(const Rational &p_rational)
    : m_text(lexical_cast<std::string>(p_rational)), m_rational(p_rational),
      m_double(static_cast<double>(p_rational))
  { }
  ~Number() = default;

  Number &operator=(const Number &p_number) = default;
  Number &operator=(const std::string &p_text)
  {
    // We call lexical_cast<Rational>() first because it throws a ValueException
    // if the conversion of the text fails
    m_rational = lexical_cast<Rational>(p_text);
    m_text = p_text;
    m_double = static_cast<double>(m_rational);
    return *this; 
  }
  Number &operator=(const Rational &p_rational)
  {
    m_text = lexical_cast<std::string>(p_rational);
    m_rational = p_rational;
    m_double = static_cast<double>(p_rational);
    return *this;
  }

  explicit operator const double &() const { return m_double; }
  explicit operator const Rational &() const { return m_rational; }
  explicit operator const std::string &() const { return m_text; }
};

}

#endif // LIBGAMBIT_NUMBER_H
