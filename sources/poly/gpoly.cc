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
  gbtText Coeff = "";
  while (charc >= '0' && charc <= '9' || charc == '.'){
    Coeff += charc;
    charnum++;
    GetChar();
  }
  if (Coeff == "") return (nega);
  else return (nega * FromText(Coeff,doub));  
}
#endif  // UNUSED

template<> gbtDouble gPoly<gbtDouble>::String_Coeff(gbtDouble nega)
{
  double doub;
  gbtText Coeff = "";
  while (charc >= '0' && charc <= '9' || charc == '.'){
    Coeff += charc;
    charnum++;
    GetChar();
  }
  if (Coeff == "") return (nega);
  else return (nega * (gbtDouble)FromText(Coeff,doub));  
}

template<> gbtRational gPoly<gbtRational>::String_Coeff(gbtRational nega)
{
  gbtRational rat;
  gbtText Coeff = "";
  while (charc >= '0' && charc <= '9' || charc == '/' || charc == '.'){
    Coeff += charc;
    charnum++;
    GetChar();
  }
  if (Coeff == "") return (nega);
  else return (nega * FromText(Coeff,rat));  
}

template<> gbtNumber gPoly<gbtNumber>::String_Coeff(gbtNumber nega)
{
  gbtNumber num;
  gbtText Coeff = "";
  while (charc >= '0' && charc <= '9' || charc == '/' || charc == '.'){
    Coeff += charc;
    charnum++;
    GetChar();
  }

  if (Coeff == "") return (nega);
  else return (nega * gbtNumber(FromText(Coeff, num)));  
}

template class gPoly<gbtRational>;
#ifndef __BCC55__
template gPoly<gbtRational> operator*(const gbtRational val, const gPoly<gbtRational> &poly);
#endif  // _BCC55__
template gPoly<gbtRational> operator*(const gPoly<gbtRational> &poly, const gbtRational val);

template gPoly<gbtDouble> TogDouble(const gPoly<gbtRational>&);
template gPoly<gbtDouble> NormalizationOfPoly(const gPoly<gbtRational>&);

template gbtOutput &operator<<(gbtOutput &f, const gPoly<gbtRational> &y);
#ifndef __BCC55__
template gbtText &operator<<(gbtText &, const gPoly<gbtRational> &);
#endif  // __BCC55__

template class gPoly<gbtNumber>;
#ifndef __BCC55__
template gPoly<gbtNumber> operator*(const gbtNumber val, const gPoly<gbtNumber> &poly);
#endif  // __BCC55__
template gPoly<gbtNumber> operator*(const gPoly<gbtNumber> &poly, const gbtNumber val);

template gPoly<gbtDouble> TogDouble(const gPoly<gbtNumber>&);
template gPoly<gbtDouble> NormalizationOfPoly(const gPoly<gbtNumber>&);

template gbtOutput &operator<<(gbtOutput &f, const gPoly<gbtNumber> &y);
#ifndef __BCC55__
template gbtText &operator<<(gbtText &, const gPoly<gbtNumber> &);
#endif  // __BCC55__

#ifdef UNUSED
template class gPoly<double>;
template gPoly<double> operator*(const double val, const gPoly<double> &poly);
template gPoly<double> operator*(const gPoly<double> &poly, const double val);
#endif // UNUSED

template class gPoly<gbtDouble>;
#ifndef __BCC55__
template gPoly<gbtDouble> operator*(const gbtDouble val, const gPoly<gbtDouble> &poly);
#endif  // __BCC55__
template gPoly<gbtDouble> operator*(const gPoly<gbtDouble> &poly, const gbtDouble val);
template gPoly<gbtDouble> operator+(const gbtDouble val, const gPoly<gbtDouble> &poly);
template gPoly<gbtDouble> operator+(const gPoly<gbtDouble> &poly, const gbtDouble val);

template gPoly<gbtDouble> TogDouble(const gPoly<gbtDouble>&);
template gPoly<gbtDouble> NormalizationOfPoly(const gPoly<gbtDouble>&);
template gbtOutput &operator<<(gbtOutput &f, const gPoly<gbtDouble> &y);
#ifndef __BCC55__
template gbtText &operator<<(gbtText &, const gPoly<gbtDouble> &);
#endif  // __BCC55__

gbtText ToText(const gPoly<gbtNumber> &p)
{
  gbtText t;
  t << p;
  return t;
}


#include "base/glist.imp"
#include "base/garray.imp"
#include "base/gblock.imp"

template class gbtArray< gPoly< gbtRational > *>;
template class gbtList< gPoly<gbtRational> * >;
template class gbtList<gPoly<gbtRational> >;
template class gbtList<gPoly<gbtNumber> >;
template class gbtList<gPoly<gbtNumber> * >;
template gbtOutput& operator << (gbtOutput& output, 
			       const gbtList<gPoly<gbtRational> >&);
template gbtOutput& operator << (gbtOutput& output, 
			       const gbtList<gPoly<gbtNumber> >&);

template class gbtList< gPoly<gbtDouble> * >;
template class gbtList< gPoly<gbtDouble> >;
template class gbtList<gbtVector<gbtDouble> >;
template gbtOutput& operator << (gbtOutput& output, 
			       const gbtList<gbtVector<gbtDouble> >&);
