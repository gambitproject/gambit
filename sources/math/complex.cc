//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of a complex number class
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

#ifdef __GNUG__
#pragma implementation
#endif

#include <stdio.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include <ctype.h>

#include "base/base.h"
#include "math/complex.h"

//--------------------------------------------------------------------------
//                              class: gbtComplex
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
//                       constructors and a destructor
//--------------------------------------------------------------------------

gbtComplex::gbtComplex() 
: re(0), im(0)
{}

gbtComplex::gbtComplex(const double& x, const double& y)
: re(x), im(y)
{}

gbtComplex::gbtComplex(const gbtComplex& y) 
: re(y.re), im(y.im)
{}

gbtComplex::gbtComplex(const int& n)
: re(n), im(0)
{}

gbtComplex::gbtComplex(const long& n) 
: re(n), im(0)
{}

gbtComplex::~gbtComplex() 
{}

//--------------------------------------------------------------------------
//                               operators
//--------------------------------------------------------------------------

double gbtComplex::RealPart() const
{
  return re;
}

double gbtComplex::ImaginaryPart() const
{
  return im;
}

gbtComplex& gbtComplex::operator =  (const gbtComplex& y)
{
  if (this == &y) return *this;

  re = y.re;   
  im = y.im;   
  return *this;
}

bool gbtComplex::operator == (const gbtComplex& y) const
{
  if (re == y.re && im == y.im) return true;
  else                          return false;
}

bool gbtComplex::operator != (const gbtComplex& y) const
{
  return !(*this == y);
}

void gbtComplex::operator += (const gbtComplex& y)
{
  *this = gbtComplex(re + y.re,im + y.im);
}

void gbtComplex::operator -= (const gbtComplex& y) 
{
  *this = gbtComplex(re - y.re,im - y.im);
}

void gbtComplex::operator *= (const gbtComplex& y) 
{
  *this = gbtComplex(re*y.re - im*y.im,re*y.im + im*y.re);
}

void gbtComplex::operator /= (const gbtComplex& y) 
{
  if (y == (gbtComplex)0) error("Attempt to divide by 0.");
  *this = gbtComplex((re*y.re + im*y.im)/(y.re*y.re + y.im*y.im),
		   (- re*y.im + im*y.re)/(y.re*y.re + y.im*y.im));
}

gbtComplex gbtComplex::operator + (const gbtComplex& y) const
{
  return gbtComplex(re + y.re,im + y.im);
}

gbtComplex gbtComplex::operator - (const gbtComplex& y) const
{
  return gbtComplex(re - y.re,im - y.im);
}

gbtComplex gbtComplex::operator * (const gbtComplex& y) const
{
  return gbtComplex(re*y.re - im*y.im,re*y.im + im*y.re);
}

gbtComplex gbtComplex::operator / (const gbtComplex& y) const
{
  if (y == (gbtComplex)0) error("Attempt to divide by 0.");
  return gbtComplex((re*y.re + im*y.im)/(y.re*y.re + y.im*y.im),
		  (- re*y.im + im*y.re)/(y.re*y.re + y.im*y.im));
}

gbtComplex gbtComplex::operator - () const
{
  return gbtComplex(-re,-im);
}

//--------------------------------------------------------------------------
//                                  errors
//--------------------------------------------------------------------------

void gbtComplex::error(const char* msg) const
{
  //  gerr << "gbtComplex class error: " << msg << '\n';
  assert(0);
}

//--------------------------------------------------------------------------
//                            input/output
//--------------------------------------------------------------------------

std::ostream &operator<<(std::ostream &s, const gbtComplex &y)
{
  s << y.re;
  if (y.im >= 0.0) {
    s << "+";
  }
  s << y.im << "i";
  return s;
}

std::istream &operator>>(std::istream &f, gbtComplex &y)
{
  char ch = ' ';
  int sign = 1;

  f >> y.re;

  while (isspace(ch)) f >> ch;

  if (ch != '+' && ch != '-') { 
    //    gout << "Error in input of complex.\n";
    exit(1);
  }

  if (ch == '-') sign = -1;
  f >> y.im;
  y.im *= sign;

  ch = ' '; while (isspace(ch)) f >> ch;

  if (ch != 'i') { 
    //    gout << "Error in input of complex.\n";
    exit(1);
  }

  return f;
}


// FUNCTIONS OUTSIDE THE CLASS

double   fabs(const gbtComplex& x) 
{
  return sqrt(x.re*x.re + x.im*x.im);
}

gbtComplex sqr(const gbtComplex& x)
{
  return x*x;
}

gbtComplex pow(const gbtComplex& x, const long y)
{
  if (y < 0) { 
    assert (x != (gbtComplex)0);
    gbtComplex x1((gbtComplex)1/x); 
    return pow(x1,-y);
  } 
  else if (y == 0) 
    return gbtComplex(1);
  else if (y == 1)
    return x;
  else {
    gbtComplex sqrt_of_answer = pow(x,y/2);
    gbtComplex answer = sqrt_of_answer * sqrt_of_answer;
    if (y % 2 == 1) answer *= x;
    return answer;
  }
}

//#define MYGCONVERT_BUFFER_LENGTH     64
//char mygconvert_buffer[MYGCONVERT_BUFFER_LENGTH];
//int myprecision = 2;

std::string ToText(const gbtComplex /* d */)
{
  //  gout << "\nError: ToText(gbtComplex) must be defined for compilation,";
  // gout << " but is not functional.\n";
  exit(0);
  return "";
}

// conversions from strings to numbers

gbtComplex TOgComplex(const std::string & /* s */)
{ 
  //  gout << "\nError: TOgComplex must be defined for compilation,";
  // gout << " but is not functional.\n";
  exit(0);
  return gbtComplex(0, 0);
}

