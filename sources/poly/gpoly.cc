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
#include "math/double.h"
#include "math/rational.h"
#include "math/gnumber.h"

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

TEMPLATE_SPECIALIZATION() gDouble gPoly<gDouble>::String_Coeff(gDouble nega)
{
  double doub;
  gText Coeff = "";
  while (charc >= '0' && charc <= '9' || charc == '.'){
    Coeff += charc;
    charnum++;
    GetChar();
  }
  if (Coeff == "") return (nega);
  else return (nega * (gDouble)FromText(Coeff,doub));  
}

TEMPLATE_SPECIALIZATION() gRational gPoly<gRational>::String_Coeff(gRational nega)
{
  gRational rat;
  gText Coeff = "";
  while (charc >= '0' && charc <= '9' || charc == '/' || charc == '.'){
    Coeff += charc;
    charnum++;
    GetChar();
  }
  if (Coeff == "") return (nega);
  else return (nega * FromText(Coeff,rat));  
}

TEMPLATE_SPECIALIZATION() gNumber gPoly<gNumber>::String_Coeff(gNumber nega)
{
  gNumber num;
  gText Coeff = "";
  while (charc >= '0' && charc <= '9' || charc == '/' || charc == '.'){
    Coeff += charc;
    charnum++;
    GetChar();
  }

  if (Coeff == "") return (nega);
  else return (nega * gNumber(FromText(Coeff, num)));  
}

template class gPoly<gRational>;
#ifndef __BCC55__
template gPoly<gRational> operator*(const gRational val, const gPoly<gRational> &poly);
#endif  // _BCC55__
template gPoly<gRational> operator*(const gPoly<gRational> &poly, const gRational val);

template gPoly<gDouble> TogDouble(const gPoly<gRational>&);
template gPoly<gDouble> NormalizationOfPoly(const gPoly<gRational>&);

template gOutput &operator<<(gOutput &f, const gPoly<gRational> &y);
#ifndef __BCC55__
template gText &operator<<(gText &, const gPoly<gRational> &);
#endif  // __BCC55__

template class gPoly<gNumber>;
#ifndef __BCC55__
template gPoly<gNumber> operator*(const gNumber val, const gPoly<gNumber> &poly);
#endif  // __BCC55__
template gPoly<gNumber> operator*(const gPoly<gNumber> &poly, const gNumber val);

template gPoly<gDouble> TogDouble(const gPoly<gNumber>&);
template gPoly<gDouble> NormalizationOfPoly(const gPoly<gNumber>&);

template gOutput &operator<<(gOutput &f, const gPoly<gNumber> &y);
#ifndef __BCC55__
template gText &operator<<(gText &, const gPoly<gNumber> &);
#endif  // __BCC55__

#ifdef UNUSED
template class gPoly<double>;
template gPoly<double> operator*(const double val, const gPoly<double> &poly);
template gPoly<double> operator*(const gPoly<double> &poly, const double val);
#endif // UNUSED

template class gPoly<gDouble>;
#ifndef __BCC55__
template gPoly<gDouble> operator*(const gDouble val, const gPoly<gDouble> &poly);
#endif  // __BCC55__
template gPoly<gDouble> operator*(const gPoly<gDouble> &poly, const gDouble val);
template gPoly<gDouble> operator+(const gDouble val, const gPoly<gDouble> &poly);
template gPoly<gDouble> operator+(const gPoly<gDouble> &poly, const gDouble val);

template gPoly<gDouble> TogDouble(const gPoly<gDouble>&);
template gPoly<gDouble> NormalizationOfPoly(const gPoly<gDouble>&);
template gOutput &operator<<(gOutput &f, const gPoly<gDouble> &y);
#ifndef __BCC55__
template gText &operator<<(gText &, const gPoly<gDouble> &);
#endif  // __BCC55__

gText ToText(const gPoly<gNumber> &p)
{
  gText t;
  t << p;
  return t;
}


#include "base/glist.imp"
#include "base/garray.imp"
#include "base/gblock.imp"

template class gArray< gPoly< gRational > *>;
template class gList< gPoly<gRational> * >;
template class gList<gPoly<gRational> >;
template class gList<gPoly<gNumber> >;
template class gList<gPoly<gNumber> * >;
template gOutput& operator << (gOutput& output, 
			       const gList<gPoly<gRational> >&);
template gOutput& operator << (gOutput& output, 
			       const gList<gPoly<gNumber> >&);

template class gList< gPoly<gDouble> * >;
template class gList< gPoly<gDouble> >;
template class gList<gVector<gDouble> >;
template gOutput& operator << (gOutput& output, 
			       const gList<gVector<gDouble> >&);




