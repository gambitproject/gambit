//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Instantiation of common gbtPolyMulti classes
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
double gbtPolyMulti<double>::String_Coeff(double nega)
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

template<> gbtDouble gbtPolyMulti<gbtDouble>::String_Coeff(gbtDouble nega)
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

template<> gbtRational gbtPolyMulti<gbtRational>::String_Coeff(gbtRational nega)
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

template<> gbtNumber gbtPolyMulti<gbtNumber>::String_Coeff(gbtNumber nega)
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

template class gbtPolyMulti<gbtRational>;
#ifndef __BCC55__
template gbtPolyMulti<gbtRational> operator*(const gbtRational val, const gbtPolyMulti<gbtRational> &poly);
#endif  // _BCC55__
template gbtPolyMulti<gbtRational> operator*(const gbtPolyMulti<gbtRational> &poly, const gbtRational val);

template gbtPolyMulti<gbtDouble> TogDouble(const gbtPolyMulti<gbtRational>&);
template gbtPolyMulti<gbtDouble> NormalizationOfPoly(const gbtPolyMulti<gbtRational>&);

template gbtOutput &operator<<(gbtOutput &f, const gbtPolyMulti<gbtRational> &y);
#ifndef __BCC55__
template gbtText &operator<<(gbtText &, const gbtPolyMulti<gbtRational> &);
#endif  // __BCC55__

template class gbtPolyMulti<gbtNumber>;
#ifndef __BCC55__
template gbtPolyMulti<gbtNumber> operator*(const gbtNumber val, const gbtPolyMulti<gbtNumber> &poly);
#endif  // __BCC55__
template gbtPolyMulti<gbtNumber> operator*(const gbtPolyMulti<gbtNumber> &poly, const gbtNumber val);

template gbtPolyMulti<gbtDouble> TogDouble(const gbtPolyMulti<gbtNumber>&);
template gbtPolyMulti<gbtDouble> NormalizationOfPoly(const gbtPolyMulti<gbtNumber>&);

template gbtOutput &operator<<(gbtOutput &f, const gbtPolyMulti<gbtNumber> &y);
#ifndef __BCC55__
template gbtText &operator<<(gbtText &, const gbtPolyMulti<gbtNumber> &);
#endif  // __BCC55__

#ifdef UNUSED
template class gbtPolyMulti<double>;
template gbtPolyMulti<double> operator*(const double val, const gbtPolyMulti<double> &poly);
template gbtPolyMulti<double> operator*(const gbtPolyMulti<double> &poly, const double val);
#endif // UNUSED

template class gbtPolyMulti<gbtDouble>;
#ifndef __BCC55__
template gbtPolyMulti<gbtDouble> operator*(const gbtDouble val, const gbtPolyMulti<gbtDouble> &poly);
#endif  // __BCC55__
template gbtPolyMulti<gbtDouble> operator*(const gbtPolyMulti<gbtDouble> &poly, const gbtDouble val);
template gbtPolyMulti<gbtDouble> operator+(const gbtDouble val, const gbtPolyMulti<gbtDouble> &poly);
template gbtPolyMulti<gbtDouble> operator+(const gbtPolyMulti<gbtDouble> &poly, const gbtDouble val);

template gbtPolyMulti<gbtDouble> TogDouble(const gbtPolyMulti<gbtDouble>&);
template gbtPolyMulti<gbtDouble> NormalizationOfPoly(const gbtPolyMulti<gbtDouble>&);
template gbtOutput &operator<<(gbtOutput &f, const gbtPolyMulti<gbtDouble> &y);
#ifndef __BCC55__
template gbtText &operator<<(gbtText &, const gbtPolyMulti<gbtDouble> &);
#endif  // __BCC55__

gbtText ToText(const gbtPolyMulti<gbtNumber> &p)
{
  gbtText t;
  t << p;
  return t;
}


#include "base/glist.imp"
#include "base/garray.imp"
#include "base/gblock.imp"

template class gbtArray< gbtPolyMulti< gbtRational > *>;
template class gbtList< gbtPolyMulti<gbtRational> * >;
template class gbtList<gbtPolyMulti<gbtRational> >;
template class gbtList<gbtPolyMulti<gbtNumber> >;
template class gbtList<gbtPolyMulti<gbtNumber> * >;
template gbtOutput& operator << (gbtOutput& output, 
			       const gbtList<gbtPolyMulti<gbtRational> >&);
template gbtOutput& operator << (gbtOutput& output, 
			       const gbtList<gbtPolyMulti<gbtNumber> >&);

template class gbtList< gbtPolyMulti<gbtDouble> * >;
template class gbtList< gbtPolyMulti<gbtDouble> >;
template class gbtList<gbtVector<gbtDouble> >;
template gbtOutput& operator << (gbtOutput& output, 
			       const gbtList<gbtVector<gbtDouble> >&);
