//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
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

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cfloat>
#include <cctype>

#include "libgambit/libgambit.h"
#include "complex.h"

//--------------------------------------------------------------------------
//                              class: gComplex
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
//                       constructors and a destructor
//--------------------------------------------------------------------------

gComplex::gComplex() 
: re(0), im(0)
{}

gComplex::gComplex(const double& x, const double& y)
: re(x), im(y)
{}

gComplex::gComplex(const gComplex& y) 
: re(y.re), im(y.im)
{}

gComplex::gComplex(const int& n)
: re(n), im(0)
{}

gComplex::gComplex(const long& n) 
: re(n), im(0)
{}

gComplex::~gComplex() 
{}

//--------------------------------------------------------------------------
//                               operators
//--------------------------------------------------------------------------

double gComplex::RealPart() const
{
  return re;
}

double gComplex::ImaginaryPart() const
{
  return im;
}

gComplex& gComplex::operator =  (const gComplex& y)
{
  if (this == &y) return *this;

  re = y.re;   
  im = y.im;   
  return *this;
}

bool gComplex::operator == (const gComplex& y) const
{
  if (re == y.re && im == y.im) return true;
  else                          return false;
}

bool gComplex::operator != (const gComplex& y) const
{
  return !(*this == y);
}

void gComplex::operator += (const gComplex& y)
{
  *this = gComplex(re + y.re,im + y.im);
}

void gComplex::operator -= (const gComplex& y) 
{
  *this = gComplex(re - y.re,im - y.im);
}

void gComplex::operator *= (const gComplex& y) 
{
  *this = gComplex(re*y.re - im*y.im,re*y.im + im*y.re);
}

void gComplex::operator /= (const gComplex& y) 
{
  if (y == (gComplex) 0)  {
    throw Gambit::ZeroDivideException();
  }
  *this = gComplex((re*y.re + im*y.im)/(y.re*y.re + y.im*y.im),
		   (- re*y.im + im*y.re)/(y.re*y.re + y.im*y.im));
}

gComplex gComplex::operator + (const gComplex& y) const
{
  return gComplex(re + y.re,im + y.im);
}

gComplex gComplex::operator - (const gComplex& y) const
{
  return gComplex(re - y.re,im - y.im);
}

gComplex gComplex::operator * (const gComplex& y) const
{
  return gComplex(re*y.re - im*y.im,re*y.im + im*y.re);
}

gComplex gComplex::operator / (const gComplex& y) const
{
  if (y == (gComplex)0)  {
    throw Gambit::ZeroDivideException();
  }
  return gComplex((re*y.re + im*y.im)/(y.re*y.re + y.im*y.im),
		  (- re*y.im + im*y.re)/(y.re*y.re + y.im*y.im));
}

gComplex gComplex::operator - () const
{
  return gComplex(-re,-im);
}

// FUNCTIONS OUTSIDE THE CLASS

double   fabs(const gComplex& x) 
{
  return sqrt(x.re*x.re + x.im*x.im);
}

gComplex sqr(const gComplex& x)
{
  return x*x;
}

gComplex pow(const gComplex& x, const long y)
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

std::string ToText(const gComplex /* d */)
{
  //  gout << "\nError: ToText(gComplex) must be defined for compilation,";
  // gout << " but is not functional.\n";
  exit(0);
  return "";
}

// conversions from strings to numbers

gComplex TOgComplex(const std::string & /* s */)
{ 
  //  gout << "\nError: TOgComplex must be defined for compilation,";
  // gout << " but is not functional.\n";
  exit(0);
  return gComplex(0, 0);
}

#include "libgambit/vector.imp"

template class Gambit::Vector<gComplex>;
