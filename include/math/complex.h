//
// FILE: complex.h -- Declaration of class gComplex
//
// $Id$
//

#ifndef _Complex_h
#ifdef __GNUG__
#pragma interface
#endif
#define _Complex_h 1

#include "base/gstream.h"
#include <math.h>

class gComplex
{
protected:
  double re;
  double im;

public:
// Constructors, and the destructor
  gComplex();
  gComplex(const double&, const double&);
  gComplex(const gComplex&);
  gComplex(const int& n);
  gComplex(const long& n);
  
  ~gComplex();

// Operators
  double      RealPart()                      const;
  double      ImaginaryPart()                 const;

  gComplex&   operator =  (const gComplex& y);

  bool        operator == (const gComplex& y) const;
  bool        operator != (const gComplex& y) const;

  gComplex    operator +  (const gComplex& y) const;
  gComplex    operator -  (const gComplex& y) const;
  gComplex    operator *  (const gComplex& y) const;
  gComplex    operator /  (const gComplex& y) const;

  gComplex    operator -  ()                 const;

  void       operator += (const gComplex& y);
  void       operator -= (const gComplex& y);
  void       operator *= (const gComplex& y);
  void       operator /= (const gComplex& y);

// error reporting
  void    error(const char* msg) const;

// printing
  friend gInput&    operator >> (gInput& s, gComplex& y);
  friend gOutput&   operator << (gOutput& s, const gComplex& y);

// friends outside the class
  friend double    fabs(const gComplex& x);              
  friend gComplex  sqr(const gComplex& x);              
  friend gComplex  pow(const gComplex& x, const long y);
};

// The following are facilities that are required of all arithmetic types.
// For other types they are given in gmisc.h

class gString;
gString         ToString(gComplex);
gComplex        TOgComplex(const gString &);

#endif

