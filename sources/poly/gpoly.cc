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
#include "math/rational.h"

template<> double gbtPolyMulti<double>::String_Coeff(double nega)
{
  double doub;
  std::string Coeff = "";
  while (charc >= '0' && charc <= '9' || charc == '.'){
    Coeff += charc;
    charnum++;
    GetChar();
  }
  if (Coeff == "") return (nega);
  else return (nega * FromText(Coeff,doub));  
}

template<> gbtRational gbtPolyMulti<gbtRational>::String_Coeff(gbtRational nega)
{
  gbtRational rat;
  std::string Coeff = "";
  while (charc >= '0' && charc <= '9' || charc == '/' || charc == '.'){
    Coeff += charc;
    charnum++;
    GetChar();
  }
  if (Coeff == "") return (nega);
  else return (nega * FromText(Coeff,rat));  
}

template class gbtPolyMulti<gbtRational>;
#ifndef __BCC55__
template gbtPolyMulti<gbtRational> operator*(const gbtRational val, const gbtPolyMulti<gbtRational> &poly);
#endif  // _BCC55__
template gbtPolyMulti<gbtRational> operator*(const gbtPolyMulti<gbtRational> &poly, const gbtRational val);

template gbtPolyMulti<double> NormalizationOfPoly(const gbtPolyMulti<gbtRational>&);

template std::ostream &operator<<(std::ostream &f, const gbtPolyMulti<gbtRational> &y);
#ifndef __BCC55__
template std::string &operator<<(std::string &, const gbtPolyMulti<gbtRational> &);
#endif  // __BCC55__

template class gbtPolyMulti<double>;
#ifndef __BCC55__
template gbtPolyMulti<double> operator*(const double val, const gbtPolyMulti<double> &poly);
#endif  // __BCC55__
template gbtPolyMulti<double> operator*(const gbtPolyMulti<double> &poly, const double val);
template gbtPolyMulti<double> NormalizationOfPoly(const gbtPolyMulti<double>&);
template std::ostream &operator<<(std::ostream &f, const gbtPolyMulti<double> &y);
#ifndef __BCC55__
template std::string &operator<<(std::string &, const gbtPolyMulti<double> &);
#endif  // __BCC55__

#include "base/glist.imp"
#include "base/garray.imp"
#include "base/gblock.imp"

template class gbtArray< gbtPolyMulti< gbtRational > *>;
template class gbtList< gbtPolyMulti<gbtRational> * >;
template class gbtList<gbtPolyMulti<gbtRational> >;

