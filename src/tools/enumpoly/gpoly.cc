//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/gpoly.cc
// Instantiation of common gPoly classes
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

#include <string.h>
#include "gpoly.imp"
#include "libgambit/libgambit.h"

template<> double gPoly<double>::String_Coeff(double nega)
{
  std::string Coeff = "";
  while ((charc >= '0' && charc <= '9') || charc == '.'){
    Coeff += charc;
    charnum++;
    GetChar();
  }
  if (Coeff == "") return (nega);
  else return (nega * strtod(Coeff.c_str(), NULL));
}

template class gPoly<double>;
template gPoly<double> operator+(const gPoly<double> &poly, const double val);
template gPoly<double> operator*(const double val, const gPoly<double> &poly);
template gPoly<double> operator*(const gPoly<double> &poly, const double val);

template gPoly<double> TogDouble(const gPoly<double>&);
template gPoly<double> NormalizationOfPoly(const gPoly<double>&);

template std::string &operator<<(std::string &, const gPoly<double> &);

/*
template<> Gambit::Rational gPoly<Gambit::Rational>::String_Coeff(Gambit::Rational nega)
{
  Gambit::Rational rat;
  std::string Coeff = "";
  while (charc >= '0' && charc <= '9' || charc == '/' || charc == '.'){
    Coeff += charc;
    charnum++;
    GetChar();
  }
  if (Coeff == "") return (nega);
  else return (nega * Gambit::ToRational(Coeff));
}

template class gPoly<Gambit::Rational>;
template gPoly<Gambit::Rational> operator*(const Gambit::Rational val, const gPoly<Gambit::Rational> &poly);
template gPoly<Gambit::Rational> operator*(const gPoly<Gambit::Rational> &poly, const Gambit::Rational val);

template gPoly<double> TogDouble(const gPoly<Gambit::Rational>&);
template gPoly<double> NormalizationOfPoly(const gPoly<Gambit::Rational>&);

template std::string &operator<<(std::string &, const gPoly<Gambit::Rational> &);
*/






