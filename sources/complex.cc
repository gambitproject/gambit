//
// FILE: complex.cc -- Implementation of gComplex
//
// $Id$
//

#ifdef __GNUG__
#pragma implementation
#endif

#include "complex.h"
#include "gnulib.h"
#include <math.h>
#include <values.h>
#include <float.h>
#include <assert.h>
#include <ctype.h>

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
  if (y == (gComplex)0) error("Attempt to divide by 0.");
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
  if (y == (gComplex)0) error("Attempt to divide by 0.");
  return gComplex((re*y.re + im*y.im)/(y.re*y.re + y.im*y.im),
		  (- re*y.im + im*y.re)/(y.re*y.re + y.im*y.im));
}

gComplex gComplex::operator - () const
{
  return gComplex(-re,-im);
}

//--------------------------------------------------------------------------
//                                  errors
//--------------------------------------------------------------------------

void gComplex::error(const char* msg) const
{
  gerr << "gComplex class error: " << msg << '\n';
  assert(0);
}

//--------------------------------------------------------------------------
//                            input/output
//--------------------------------------------------------------------------

gOutput& operator << (gOutput& s, const gComplex& y)
{
  s << y.re;
  if (y.im >= 0.0) gout << "+";
  gout << y.im << "i";
  return s;
}

gInput &operator>>(gInput &f, gComplex &y)
{
  char ch = ' ';
  int sign = 1;

  f >> y.re;

  while (isspace(ch)) f >> ch;

  if (ch != '+' && ch != '-') { 
    gout << "Error in input of complex.\n";
    exit(1);
  }

  if (ch == '-') sign = -1;
  f >> y.im;
  y.im *= sign;

  ch = ' '; while (isspace(ch)) f >> ch;

  if (ch != 'i') { 
    gout << "Error in input of complex.\n";
    exit(1);
  }

  return f;
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
    assert (x != (gComplex)0);
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

#include <stdio.h>
#include "gtext.h"


//#define MYGCONVERT_BUFFER_LENGTH     64
//char mygconvert_buffer[MYGCONVERT_BUFFER_LENGTH];
//int myprecision = 2;

gText ToText(const gComplex /* d */)
{
  gout << "\nError: ToText(gComplex) must be defined for compilation,";
  gout << " but is not functional.\n";
  exit(0);
}

// conversions from strings to numbers

gComplex TOgComplex(const gText & /* s */)
{ 
  gout << "\nError: TOgComplex must be defined for compilation,";
  gout << " but is not functional.\n";
  exit(0);
}

#include "glist.imp"

template class gList<gComplex>;
template gOutput& operator << (gOutput& output, const gList<gComplex>&);
