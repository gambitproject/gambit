//
// $Source$
// $Date$
// $Revision$
//

// This may look like C code, but it is really -*- C++ -*-

/* 
Copyright (C) 1988 Free Software Foundation
    written by Doug Lea (dl@rocky.oswego.edu)

This file is part of the GNU C++ Library.  This library is free
software; you can redistribute it and/or modify it under the terms of
the GNU Library General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your
option) any later version.  This library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU Library General Public License for more details.
You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef _Integer_h
#ifdef __GNUG__
#pragma interface
#endif
#define _Integer_h 1

#include "base/gtext.h"

class gInput;
class gOutput;

struct IntRep                    // internal Integer representations
{
  unsigned short  len;          // current length
  unsigned short  sz;           // allocated space (0 means static).
  short           sgn;          // 1 means >= 0; 0 means < 0 
  unsigned short  s[1];         // represented as ushort array starting here
};

// True if REP is staticly (or manually) allocated,
// and should not be deleted by an Integer destructor.
#define STATIC_IntRep(rep) ((rep)->sz==0)

extern IntRep*  Ialloc(IntRep*, const unsigned short *, int, int, int);
extern IntRep*  Icalloc(IntRep*, int);
extern IntRep*  Icopy_ulong(IntRep*, unsigned long);
extern IntRep*  Icopy_long(IntRep*, long);
extern IntRep*  Icopy(IntRep*, const IntRep*);
extern IntRep*  Iresize(IntRep*, int);
extern IntRep*  add(const IntRep*, int, const IntRep*, int, IntRep*);
extern IntRep*  add(const IntRep*, int, long, IntRep*);
extern IntRep*  multiply(const IntRep*, const IntRep*, IntRep*);
extern IntRep*  multiply(const IntRep*, long, IntRep*);
extern IntRep*  lshift(const IntRep*, long, IntRep*);
extern IntRep*  lshift(const IntRep*, const IntRep*, int, IntRep*);
extern IntRep*  bitop(const IntRep*, const IntRep*, IntRep*, char);
extern IntRep*  bitop(const IntRep*, long, IntRep*, char);
extern IntRep*  power(const IntRep*, long, IntRep*);
extern IntRep*  div(const IntRep*, const IntRep*, IntRep*);
extern IntRep*  mod(const IntRep*, const IntRep*, IntRep*);
extern IntRep*  div(const IntRep*, long, IntRep*);
extern IntRep*  mod(const IntRep*, long, IntRep*);
extern IntRep*  Compl(const IntRep*, IntRep*);
extern IntRep*  abs(const IntRep*, IntRep*);
extern IntRep*  negate(const IntRep*, IntRep*);
extern IntRep*  pow(const IntRep*, long);
extern IntRep*  gcd(const IntRep*, const IntRep* y);
extern int      compare(const IntRep*, const IntRep*);
extern int      compare(const IntRep*, long);
extern int      ucompare(const IntRep*, const IntRep*);
extern int      ucompare(const IntRep*, long);
extern char*    Itoa(const IntRep* x, int base = 10, int width = 0);
extern char*    cvtItoa(const IntRep* x, char* fmt, int& fmtlen, int base,
                        int showbase, int width, int align_right, 
                        char fillchar, char Xcase, int showpos);
extern IntRep*  atoIntRep(const char* s, int base = 10);
extern long     Itolong(const IntRep*);
extern double   Itodouble(const IntRep*);
extern int      Iislong(const IntRep*);
extern int      Iisdouble(const IntRep*);
extern long     lg(const IntRep*);

extern IntRep _ZeroRep, _OneRep, _MinusOneRep;

class gInteger
{
protected:
  IntRep*         rep;
public:
                  gInteger();
                  gInteger(int);
                  gInteger(long);
                  gInteger(unsigned long);
                  gInteger(IntRep*);
                  gInteger(const gInteger&);

                  ~gInteger();

  void            operator =  (const gInteger&);
  void            operator =  (long);

// unary operations to self

  void            operator ++ ();
  void            operator -- ();
  void            negate();          // negate in-place
  void            abs();             // absolute-value in-place
  void            complement();      // bitwise complement in-place

// assignment-based operations

  void            operator += (const gInteger&);
  void            operator -= (const gInteger&);
  void            operator *= (const gInteger&);
  void            operator /= (const gInteger&);
  void            operator %= (const gInteger&);
  void            operator <<=(const gInteger&);
  void            operator >>=(const gInteger&);
  void            operator &= (const gInteger&);
  void            operator |= (const gInteger&);
  void            operator ^= (const gInteger&);

  void            operator += (long);
  void            operator -= (long);
  void            operator *= (long);
  void            operator /= (long);
  void            operator %= (long);
  void            operator <<=(long);
  void            operator >>=(long);
  void            operator &= (long);
  void            operator |= (long);
  void            operator ^= (long);

// (constructive binary operations are inlined below)

#ifdef __GNUG__
  friend gInteger operator <? (const gInteger& x, const gInteger& y); // min
  friend gInteger operator >? (const gInteger& x, const gInteger& y); // max
#endif

// builtin Integer functions that must be friends

  friend long     lg (const gInteger&); // floor log base 2 of abs(x)
  friend double   ratio(const gInteger& x, const gInteger& y);
                  // return x/y as a double

  friend gInteger  gcd(const gInteger&, const gInteger&);
  friend int      even(const gInteger&); // true if even
  friend int      odd(const gInteger&); // true if odd
  friend int      sign(const gInteger&); // returns -1, 0, +1

  friend void     (setbit)(gInteger& x, long b);   // set b'th bit of x
  friend void     clearbit(gInteger& x, long b); // clear b'th bit
  friend int      testbit(const gInteger& x, long b);  // return b'th bit

// procedural versions of operators

  friend void     abs(const gInteger& x, gInteger& dest);
  friend void     negate(const gInteger& x, gInteger& dest);
  friend void     complement(const gInteger& x, gInteger& dest);

  friend int      compare(const gInteger&, const gInteger&);  
  friend int      ucompare(const gInteger&, const gInteger&); 
  friend void     add(const gInteger& x, const gInteger& y, gInteger& dest);
  friend void     sub(const gInteger& x, const gInteger& y, gInteger& dest);
  friend void     mul(const gInteger& x, const gInteger& y, gInteger& dest);
  friend void     div(const gInteger& x, const gInteger& y, gInteger& dest);
  friend void     mod(const gInteger& x, const gInteger& y, gInteger& dest);
  friend void     divide(const gInteger& x, const gInteger& y, 
                         gInteger& q, gInteger& r);
  friend void     And(const gInteger& x, const gInteger& y, gInteger& dest);
  friend void     Or(const gInteger& x, const gInteger& y, gInteger& dest);
  friend void     Xor(const gInteger& x, const gInteger& y, gInteger& dest);
  friend void     lshift(const gInteger& x, const gInteger& y, gInteger& dest);
  friend void     rshift(const gInteger& x, const gInteger& y, gInteger& dest);
  friend void     pow(const gInteger& x, const gInteger& y, gInteger& dest);

  friend int      compare(const gInteger&, long);  
  friend int      ucompare(const gInteger&, long); 
  friend void     add(const gInteger& x, long y, gInteger& dest);
  friend void     sub(const gInteger& x, long y, gInteger& dest);
  friend void     mul(const gInteger& x, long y, gInteger& dest);
  friend void     div(const gInteger& x, long y, gInteger& dest);
  friend void     mod(const gInteger& x, long y, gInteger& dest);
  friend void     divide(const gInteger& x, long y, gInteger& q, long& r);
  friend void     And(const gInteger& x, long y, gInteger& dest);
  friend void     Or(const gInteger& x, long y, gInteger& dest);
  friend void     Xor(const gInteger& x, long y, gInteger& dest);
  friend void     lshift(const gInteger& x, long y, gInteger& dest);
  friend void     rshift(const gInteger& x, long y, gInteger& dest);
  friend void     pow(const gInteger& x, long y, gInteger& dest);

  friend int      compare(long, const gInteger&);  
  friend int      ucompare(long, const gInteger&); 
  friend void     add(long x, const gInteger& y, gInteger& dest);
  friend void     sub(long x, const gInteger& y, gInteger& dest);
  friend void     mul(long x, const gInteger& y, gInteger& dest);
  friend void     And(long x, const gInteger& y, gInteger& dest);
  friend void     Or(long x, const gInteger& y, gInteger& dest);
  friend void     Xor(long x, const gInteger& y, gInteger& dest);

// coercion & conversion

  int             fits_in_long() const { return Iislong(rep); }
  int             fits_in_double() const { return Iisdouble(rep); }

#if 0
  // There two operators cause a number of ambiguities.
                  operator long() const { return Itolong(rep); }
                  operator double() const { return Itodouble(rep); }
#endif
  long		  as_long() const { return Itolong(rep); }
  double	  as_double() const { return Itodouble(rep); }

  friend char*    Itoa(const gInteger& x, int base = 10, int width = 0);
  friend gInteger  atoI(const char* s, int base = 10);
//  void		  printon(ostream& s, int base = 10, int width = 0) const;
  
  friend gInput& operator >> (gInput &s, gInteger& y);
  friend gOutput& operator << (gOutput &s, const gInteger& y);

// error detection

  int             initialized() const;
  void   error(const char* msg) const;
  int             OK() const;  
};


//  (These are declared inline)

  int      operator == (const gInteger&, const gInteger&);
  int      operator == (const gInteger&, long);
  int      operator != (const gInteger&, const gInteger&);
  int      operator != (const gInteger&, long);
  int      operator <  (const gInteger&, const gInteger&);
  int      operator <  (const gInteger&, long);
  int      operator <= (const gInteger&, const gInteger&);
  int      operator <= (const gInteger&, long);
  int      operator >  (const gInteger&, const gInteger&);
  int      operator >  (const gInteger&, long);
  int      operator >= (const gInteger&, const gInteger&);
  int      operator >= (const gInteger&, long);
  gInteger  operator -  (const gInteger&);
  gInteger  operator ~  (const gInteger&);
  gInteger  operator +  (const gInteger&, const gInteger&);
  gInteger  operator +  (const gInteger&, long);
  gInteger  operator +  (long, const gInteger&);
  gInteger  operator -  (const gInteger&, const gInteger&);
  gInteger  operator -  (const gInteger&, long);
  gInteger  operator -  (long, const gInteger&);
  gInteger  operator *  (const gInteger&, const gInteger&);
  gInteger  operator *  (const gInteger&, long);
  gInteger  operator *  (long, const gInteger&);
  gInteger  operator /  (const gInteger&, const gInteger&);
  gInteger  operator /  (const gInteger&, long);
  gInteger  operator %  (const gInteger&, const gInteger&);
  gInteger  operator %  (const gInteger&, long);
  gInteger  operator << (const gInteger&, const gInteger&);
  gInteger  operator << (const gInteger&, long);
  gInteger  operator >> (const gInteger&, const gInteger&);
  gInteger  operator >> (const gInteger&, long);
  gInteger  operator &  (const gInteger&, const gInteger&);
  gInteger  operator &  (const gInteger&, long);
  gInteger  operator &  (long, const gInteger&);
  gInteger  operator |  (const gInteger&, const gInteger&);
  gInteger  operator |  (const gInteger&, long);
  gInteger  operator |  (long, const gInteger&);
  gInteger  operator ^  (const gInteger&, const gInteger&);
  gInteger  operator ^  (const gInteger&, long);
  gInteger  operator ^  (long, const gInteger&);

  gInteger  abs(const gInteger&); // absolute value
  gInteger  sqr(const gInteger&); // square

  gInteger  pow(const gInteger& x, const gInteger& y);
  gInteger  pow(const gInteger& x, long y);
  gInteger  Ipow(long x, long y); // x to the y as gInteger 


extern char*    dec(const gInteger& x, int width = 0);
extern char*    oct(const gInteger& x, int width = 0);
extern char*    hex(const gInteger& x, int width = 0);
extern gInteger  sqrt(const gInteger&); // floor of square root
extern gInteger  lcm(const gInteger& x, const gInteger& y); // least common mult

gText ToText(const gInteger &);

#endif /* _Integer_h */



