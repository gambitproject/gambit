//
// FILE: gnumber.h -- Number class for Gambit project. 
//
// $Id$
//

#ifndef _GNumber_h
#define _GNumber_h 1

#include "integer.h"
#include "rational.h"
#include <math.h>

class gOutput;

typedef enum {RATIONAL, DOUBLE} gPrecision;

class gNumber
{
protected:
  gPrecision        rep;
  gRational         *rval;
  double            dval;

public:
  gNumber();
  gNumber(double);
  gNumber(int n);
  gNumber(long n);
  gNumber(const gInteger& y);
  gNumber(const gRational& y);
  gNumber(const gNumber& y);

 ~gNumber();

  gNumber &operator =  (const gNumber& y);

  friend bool      operator == (const gNumber& x, const gNumber& y);
  friend bool      operator != (const gNumber& x, const gNumber& y);
  friend bool      operator <  (const gNumber& x, const gNumber& y);
  friend bool      operator <= (const gNumber& x, const gNumber& y);
  friend bool      operator >  (const gNumber& x, const gNumber& y);
  friend bool      operator >= (const gNumber& x, const gNumber& y);

  friend gNumber   operator +  (const gNumber& x, const gNumber& y);
  friend gNumber   operator -  (const gNumber& x, const gNumber& y);
  friend gNumber   operator *  (const gNumber& x, const gNumber& y);
  friend gNumber   operator /  (const gNumber& x, const gNumber& y);

  void             operator += (const gNumber& y);
  void             operator -= (const gNumber& y);
  void             operator *= (const gNumber& y);
  void             operator /= (const gNumber& y);

  friend gNumber  operator - (const gNumber& x);

  friend gOutput&    operator << (gOutput& s, const gNumber& y);
  friend gInput&     operator >> (gInput& s,  const gNumber& y);

  void             error(const char* msg) const;
  double           GetDub() const;
  operator double() const   { return GetDub(); }
  gRational       GetRat() const;
  operator gRational() const  { return GetRat(); }
  gPrecision&      GetRep() const;

};

#endif

gOutput &operator<<(gOutput &, const gNumber &);

