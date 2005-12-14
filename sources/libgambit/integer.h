//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to an arbitrary-length integer class
//
// This file is part of Gambit
// Modifications copyright (c) 2002, The Gambit Project
//
// The original copyright and license are included below.
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
#if defined(__GNUG__) && !defined(__APPLE_CC__)
#pragma interface
#endif
#define _Integer_h 1

#include <string>

struct gbtIntegerRep                    // internal Integer representations
{
  unsigned short  len;          // current length
  unsigned short  sz;           // allocated space (0 means static).
  short           sgn;          // 1 means >= 0; 0 means < 0 
  unsigned short  s[1];         // represented as ushort array starting here
};

// True if REP is staticly (or manually) allocated,
// and should not be deleted by an Integer destructor.
#define STATIC_gbtIntegerRep(rep) ((rep)->sz==0)

extern gbtIntegerRep*  Ialloc(gbtIntegerRep*, const unsigned short *, int, int, int);
extern gbtIntegerRep*  Icalloc(gbtIntegerRep*, int);
extern gbtIntegerRep*  Icopy_ulong(gbtIntegerRep*, unsigned long);
extern gbtIntegerRep*  Icopy_long(gbtIntegerRep*, long);
extern gbtIntegerRep*  Icopy(gbtIntegerRep*, const gbtIntegerRep*);
extern gbtIntegerRep*  Iresize(gbtIntegerRep*, int);
extern gbtIntegerRep*  add(const gbtIntegerRep*, int, const gbtIntegerRep*, int, gbtIntegerRep*);
extern gbtIntegerRep*  add(const gbtIntegerRep*, int, long, gbtIntegerRep*);
extern gbtIntegerRep*  multiply(const gbtIntegerRep*, const gbtIntegerRep*, gbtIntegerRep*);
extern gbtIntegerRep*  multiply(const gbtIntegerRep*, long, gbtIntegerRep*);
extern gbtIntegerRep*  lshift(const gbtIntegerRep*, long, gbtIntegerRep*);
extern gbtIntegerRep*  lshift(const gbtIntegerRep*, const gbtIntegerRep*, int, gbtIntegerRep*);
extern gbtIntegerRep*  bitop(const gbtIntegerRep*, const gbtIntegerRep*, gbtIntegerRep*, char);
extern gbtIntegerRep*  bitop(const gbtIntegerRep*, long, gbtIntegerRep*, char);
extern gbtIntegerRep*  power(const gbtIntegerRep*, long, gbtIntegerRep*);
extern gbtIntegerRep*  div(const gbtIntegerRep*, const gbtIntegerRep*, gbtIntegerRep*);
extern gbtIntegerRep*  mod(const gbtIntegerRep*, const gbtIntegerRep*, gbtIntegerRep*);
extern gbtIntegerRep*  div(const gbtIntegerRep*, long, gbtIntegerRep*);
extern gbtIntegerRep*  mod(const gbtIntegerRep*, long, gbtIntegerRep*);
extern gbtIntegerRep*  Compl(const gbtIntegerRep*, gbtIntegerRep*);
extern gbtIntegerRep*  abs(const gbtIntegerRep*, gbtIntegerRep*);
extern gbtIntegerRep*  negate(const gbtIntegerRep*, gbtIntegerRep*);
extern gbtIntegerRep*  pow(const gbtIntegerRep*, long);
extern gbtIntegerRep*  gcd(const gbtIntegerRep*, const gbtIntegerRep* y);
extern int      compare(const gbtIntegerRep*, const gbtIntegerRep*);
extern int      compare(const gbtIntegerRep*, long);
extern int      ucompare(const gbtIntegerRep*, const gbtIntegerRep*);
extern int      ucompare(const gbtIntegerRep*, long);
extern std::string Itoa(const gbtIntegerRep* x, int base = 10, int width = 0);
extern std::string cvtItoa(const gbtIntegerRep* x, std::string fmt, int& fmtlen, int base,
                           int showbase, int width, int align_right, 
                           char fillchar, char Xcase, int showpos);
extern gbtIntegerRep*  atogbtIntegerRep(const char* s, int base = 10);
extern long     Itolong(const gbtIntegerRep*);
extern double   Itodouble(const gbtIntegerRep*);
extern int      Iislong(const gbtIntegerRep*);
extern int      Iisdouble(const gbtIntegerRep*);
extern long     lg(const gbtIntegerRep*);

extern gbtIntegerRep _ZeroRep, _OneRep, _MinusOneRep;

class gbtInteger
{
protected:
  gbtIntegerRep*         rep;
public:
                  gbtInteger();
                  gbtInteger(int);
                  gbtInteger(long);
                  gbtInteger(unsigned long);
                  gbtInteger(gbtIntegerRep*);
                  gbtInteger(const gbtInteger&);

                  ~gbtInteger();

  void            operator =  (const gbtInteger&);
  void            operator =  (long);

// unary operations to self

  void            operator ++ ();
  void            operator -- ();
  void            negate();          // negate in-place
  void            abs();             // absolute-value in-place
  void            complement();      // bitwise complement in-place

// assignment-based operations

  void            operator += (const gbtInteger&);
  void            operator -= (const gbtInteger&);
  void            operator *= (const gbtInteger&);
  void            operator /= (const gbtInteger&);
  void            operator %= (const gbtInteger&);
  void            operator <<=(const gbtInteger&);
  void            operator >>=(const gbtInteger&);
  void            operator &= (const gbtInteger&);
  void            operator |= (const gbtInteger&);
  void            operator ^= (const gbtInteger&);

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

// builtin Integer functions that must be friends

  friend long     lg (const gbtInteger&); // floor log base 2 of abs(x)
  friend double   ratio(const gbtInteger& x, const gbtInteger& y);
                  // return x/y as a double

  friend gbtInteger  gcd(const gbtInteger&, const gbtInteger&);
  friend int      even(const gbtInteger&); // true if even
  friend int      odd(const gbtInteger&); // true if odd
  friend int      sign(const gbtInteger&); // returns -1, 0, +1

  friend void     setbit(gbtInteger& x, long b);   // set b'th bit of x
  friend void     clearbit(gbtInteger& x, long b); // clear b'th bit
  friend int      testbit(const gbtInteger& x, long b);  // return b'th bit

// procedural versions of operators

  friend void     abs(const gbtInteger& x, gbtInteger& dest);
  friend void     negate(const gbtInteger& x, gbtInteger& dest);
  friend void     complement(const gbtInteger& x, gbtInteger& dest);

  friend int      compare(const gbtInteger&, const gbtInteger&);  
  friend int      ucompare(const gbtInteger&, const gbtInteger&); 
  friend void     add(const gbtInteger& x, const gbtInteger& y, gbtInteger& dest);
  friend void     sub(const gbtInteger& x, const gbtInteger& y, gbtInteger& dest);
  friend void     mul(const gbtInteger& x, const gbtInteger& y, gbtInteger& dest);
  friend void     div(const gbtInteger& x, const gbtInteger& y, gbtInteger& dest);
  friend void     mod(const gbtInteger& x, const gbtInteger& y, gbtInteger& dest);
  friend void     divide(const gbtInteger& x, const gbtInteger& y, 
                         gbtInteger& q, gbtInteger& r);
  friend void     And(const gbtInteger& x, const gbtInteger& y, gbtInteger& dest);
  friend void     Or(const gbtInteger& x, const gbtInteger& y, gbtInteger& dest);
  friend void     Xor(const gbtInteger& x, const gbtInteger& y, gbtInteger& dest);
  friend void     lshift(const gbtInteger& x, const gbtInteger& y, gbtInteger& dest);
  friend void     rshift(const gbtInteger& x, const gbtInteger& y, gbtInteger& dest);
  friend void     pow(const gbtInteger& x, const gbtInteger& y, gbtInteger& dest);

  friend int      compare(const gbtInteger&, long);  
  friend int      ucompare(const gbtInteger&, long); 
  friend void     add(const gbtInteger& x, long y, gbtInteger& dest);
  friend void     sub(const gbtInteger& x, long y, gbtInteger& dest);
  friend void     mul(const gbtInteger& x, long y, gbtInteger& dest);
  friend void     div(const gbtInteger& x, long y, gbtInteger& dest);
  friend void     mod(const gbtInteger& x, long y, gbtInteger& dest);
  friend void     divide(const gbtInteger& x, long y, gbtInteger& q, long& r);
  friend void     And(const gbtInteger& x, long y, gbtInteger& dest);
  friend void     Or(const gbtInteger& x, long y, gbtInteger& dest);
  friend void     Xor(const gbtInteger& x, long y, gbtInteger& dest);
  friend void     lshift(const gbtInteger& x, long y, gbtInteger& dest);
  friend void     rshift(const gbtInteger& x, long y, gbtInteger& dest);
  friend void     pow(const gbtInteger& x, long y, gbtInteger& dest);

  friend int      compare(long, const gbtInteger&);  
  friend int      ucompare(long, const gbtInteger&); 
  friend void     add(long x, const gbtInteger& y, gbtInteger& dest);
  friend void     sub(long x, const gbtInteger& y, gbtInteger& dest);
  friend void     mul(long x, const gbtInteger& y, gbtInteger& dest);
  friend void     And(long x, const gbtInteger& y, gbtInteger& dest);
  friend void     Or(long x, const gbtInteger& y, gbtInteger& dest);
  friend void     Xor(long x, const gbtInteger& y, gbtInteger& dest);

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

  friend std::string    Itoa(const gbtInteger& x, int base = 10, int width = 0);
  friend gbtInteger  atoI(const char* s, int base = 10);
//  void		  printon(ostream& s, int base = 10, int width = 0) const;
  
  friend std::istream &operator>>(std::istream &s, gbtInteger& y);
  friend std::ostream &operator<<(std::ostream &s, const gbtInteger& y);

// error detection

  int             initialized() const;
  void   error(const char* msg) const;
  int             OK() const;  
};


//  (These are declared inline)

  int      operator == (const gbtInteger&, const gbtInteger&);
  int      operator == (const gbtInteger&, long);
  int      operator != (const gbtInteger&, const gbtInteger&);
  int      operator != (const gbtInteger&, long);
  int      operator <  (const gbtInteger&, const gbtInteger&);
  int      operator <  (const gbtInteger&, long);
  int      operator <= (const gbtInteger&, const gbtInteger&);
  int      operator <= (const gbtInteger&, long);
  int      operator >  (const gbtInteger&, const gbtInteger&);
  int      operator >  (const gbtInteger&, long);
  int      operator >= (const gbtInteger&, const gbtInteger&);
  int      operator >= (const gbtInteger&, long);
  gbtInteger  operator -  (const gbtInteger&);
  gbtInteger  operator ~  (const gbtInteger&);
  gbtInteger  operator +  (const gbtInteger&, const gbtInteger&);
  gbtInteger  operator +  (const gbtInteger&, long);
  gbtInteger  operator +  (long, const gbtInteger&);
  gbtInteger  operator -  (const gbtInteger&, const gbtInteger&);
  gbtInteger  operator -  (const gbtInteger&, long);
  gbtInteger  operator -  (long, const gbtInteger&);
  gbtInteger  operator *  (const gbtInteger&, const gbtInteger&);
  gbtInteger  operator *  (const gbtInteger&, long);
  gbtInteger  operator *  (long, const gbtInteger&);
  gbtInteger  operator /  (const gbtInteger&, const gbtInteger&);
  gbtInteger  operator /  (const gbtInteger&, long);
  gbtInteger  operator %  (const gbtInteger&, const gbtInteger&);
  gbtInteger  operator %  (const gbtInteger&, long);
  gbtInteger  operator << (const gbtInteger&, const gbtInteger&);
  gbtInteger  operator << (const gbtInteger&, long);
  gbtInteger  operator >> (const gbtInteger&, const gbtInteger&);
  gbtInteger  operator >> (const gbtInteger&, long);
  gbtInteger  operator &  (const gbtInteger&, const gbtInteger&);
  gbtInteger  operator &  (const gbtInteger&, long);
  gbtInteger  operator &  (long, const gbtInteger&);
  gbtInteger  operator |  (const gbtInteger&, const gbtInteger&);
  gbtInteger  operator |  (const gbtInteger&, long);
  gbtInteger  operator |  (long, const gbtInteger&);
  gbtInteger  operator ^  (const gbtInteger&, const gbtInteger&);
  gbtInteger  operator ^  (const gbtInteger&, long);
  gbtInteger  operator ^  (long, const gbtInteger&);

  gbtInteger  abs(const gbtInteger&); // absolute value
  gbtInteger  sqr(const gbtInteger&); // square

  gbtInteger  pow(const gbtInteger& x, const gbtInteger& y);
  gbtInteger  pow(const gbtInteger& x, long y);
  gbtInteger  Ipow(long x, long y); // x to the y as gbtInteger 


extern std::string    dec(const gbtInteger& x, int width = 0);
extern std::string    oct(const gbtInteger& x, int width = 0);
extern std::string    hex(const gbtInteger& x, int width = 0);
extern gbtInteger  sqrt(const gbtInteger&); // floor of square root
extern gbtInteger  lcm(const gbtInteger& x, const gbtInteger& y); // least common mult

std::string ToText(const gbtInteger &);

#endif /* _Integer_h */



