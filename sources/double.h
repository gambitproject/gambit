//
// FILE: double.h -- Declaration of class gDouble: double with tolerant ==
//
// $Id$
//

/*

   The class  gDouble  is just like double except that operator == will 
return true when the two numbers are `close.'  (See the definition of 
operator == in double.cc for details.)  The guiding philosophy here is
that in floating point calculations that begin with integers or rationals,
and branch according to the signs of the computed numbers, the most common
error is to be tricked by roundoff error into thinking that some number 
is different from zero.

*/

#ifndef _Double_h
#ifdef __GNUG__
#pragma interface
#endif
#define _Double_h 1

#include <math.h>
#include "gambitio.h"
// #include "objcount.h"

class gDouble
// : private Counted<gDouble>
{
protected:
  double dbl;

public:
// Constructors, and the destructor
  gDouble();
  gDouble(const double&);
  gDouble(const gDouble&);
  gDouble(const int& n);
  gDouble(const long& n);
  
  ~gDouble();

// Conversion
  double ToDouble() const;

// Operators
  gDouble&   operator =  (const gDouble& y);

  bool       operator == (const gDouble& y) const;
  bool       operator != (const gDouble& y) const;
  bool       operator <  (const gDouble& y) const;
  bool       operator <= (const gDouble& y) const;
  bool       operator >  (const gDouble& y) const;
  bool       operator >= (const gDouble& y) const;

  gDouble    operator +  (const gDouble& y) const;
  gDouble    operator -  (const gDouble& y) const;
  gDouble    operator *  (const gDouble& y) const;
  gDouble    operator /  (const gDouble& y) const;

  gDouble    operator -  ()                 const;

  void       operator += (const gDouble& y);
  void       operator -= (const gDouble& y);
  void       operator *= (const gDouble& y);
  void       operator /= (const gDouble& y);

// error reporting and object counting
  void    error(const char* msg) const;
//  inline  int static Count() { return Counted<gDouble>::objCount(); }

// printing
  friend gInput&    operator >> (gInput& s, gDouble& y);
  friend gOutput&   operator << (gOutput& s, const gDouble& y);

// friends outside the class
  friend int      sign(const gDouble& x);             // -1, 0, or +1
  friend gDouble  fabs(const gDouble& x);              
  friend gDouble  sqr(const gDouble& x);              
  friend gDouble  sqrt(const gDouble& x);              
  friend gDouble  pow(const gDouble& x, const long y);
  friend gDouble  pow(const gDouble& x, const gDouble& y);
};

// The following are facilities that are required of all arithmetic types.
// For other types they are given in gmisc.h

class gText;
gText         ToText(gDouble);
gDouble         TOgDouble(const gText &);

#endif

