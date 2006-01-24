//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Instantiation of common gPoly classes
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
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

#include "gpoly.imp"
#include "double.h"
#include "libgambit/libgambit.h"

#ifdef UNUSED
double gPoly<double>::String_Coeff(double nega)
{
  double doub;
  gText Coeff = "";
  while (charc >= '0' && charc <= '9' || charc == '.'){
    Coeff += charc;
    charnum++;
    GetChar();
  }
  if (Coeff == "") return (nega);
  else return (nega * FromText(Coeff,doub));  
}
#endif  // UNUSED

template<> gDouble gPoly<gDouble>::String_Coeff(gDouble nega)
{
  std::string Coeff = "";
  while (charc >= '0' && charc <= '9' || charc == '.'){
    Coeff += charc;
    charnum++;
    GetChar();
  }
  if (Coeff == "") return (nega);
  else return (nega * (gDouble) Gambit::ToDouble(Coeff));
}

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

template gPoly<gDouble> TogDouble(const gPoly<Gambit::Rational>&);
template gPoly<gDouble> NormalizationOfPoly(const gPoly<Gambit::Rational>&);

template std::string &operator<<(std::string &, const gPoly<Gambit::Rational> &);

template class gPoly<gDouble>;
template gPoly<gDouble> operator*(const gDouble val, const gPoly<gDouble> &poly);
template gPoly<gDouble> operator*(const gPoly<gDouble> &poly, const gDouble val);
template gPoly<gDouble> operator+(const gDouble val, const gPoly<gDouble> &poly);
template gPoly<gDouble> operator+(const gPoly<gDouble> &poly, const gDouble val);

template gPoly<gDouble> TogDouble(const gPoly<gDouble>&);
template gPoly<gDouble> NormalizationOfPoly(const gPoly<gDouble>&);
template std::string &operator<<(std::string &, const gPoly<gDouble> &);





