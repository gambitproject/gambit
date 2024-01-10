//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/complex.cc
// Implementation of a complex number class
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

#include <cstdlib>

#include "gambit.h"
#include "gcomplex.h"

//--------------------------------------------------------------------------
//                              class: gComplex
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
//                               operators
//--------------------------------------------------------------------------

void gComplex::operator /= (const gComplex& y)
{
  if (y == (gComplex) 0)  {
    throw Gambit::ZeroDivideException();
  }
  *this = gComplex((re*y.re + im*y.im)/(y.re*y.re + y.im*y.im),
		   (- re*y.im + im*y.re)/(y.re*y.re + y.im*y.im));
}


gComplex gComplex::operator / (const gComplex& y) const
{
  if (y == (gComplex)0)  {
    throw Gambit::ZeroDivideException();
  }
  return gComplex((re*y.re + im*y.im)/(y.re*y.re + y.im*y.im),
		  (- re*y.im + im*y.re)/(y.re*y.re + y.im*y.im));
}

// FUNCTIONS OUTSIDE THE CLASS

gComplex pow(const gComplex& x, long y)
{
  if (y < 0) { 
    if (x == (gComplex) 0) {
      throw Gambit::AssertionException("Raising 0^0.");
    }
    gComplex x1((gComplex)1/x); 
    return pow(x1,-y);
  } 
  else if (y == 0) 
    return gComplex(1);
  else if (y == 1)
    return x;
  else {
    gComplex sqrt_of_answer = pow(x,y/2);
    gComplex answer = sqrt_of_answer * sqrt_of_answer;
    if (y % 2 == 1) answer *= x;
    return answer;
  }
}


#include "core/vector.imp"

template class Gambit::Vector<gComplex>;
