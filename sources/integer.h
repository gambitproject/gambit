// This may look like C code, but it is really -*- C++ -*-
// @(#)integer.h	1.1 1/12/94

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

#include "gambitio.h"

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
extern IntRep*  compl(const IntRep*, IntRep*);
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
  friend void     and(const gInteger& x, const gInteger& y, gInteger& dest);
  friend void     or(const gInteger& x, const gInteger& y, gInteger& dest);
  friend void     xor(const gInteger& x, const gInteger& y, gInteger& dest);
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
  friend void     and(const gInteger& x, long y, gInteger& dest);
  friend void     or(const gInteger& x, long y, gInteger& dest);
  friend void     xor(const gInteger& x, long y, gInteger& dest);
  friend void     lshift(const gInteger& x, long y, gInteger& dest);
  friend void     rshift(const gInteger& x, long y, gInteger& dest);
  friend void     pow(const gInteger& x, long y, gInteger& dest);

  friend int      compare(long, const gInteger&);  
  friend int      ucompare(long, const gInteger&); 
  friend void     add(long x, const gInteger& y, gInteger& dest);
  friend void     sub(long x, const gInteger& y, gInteger& dest);
  friend void     mul(long x, const gInteger& y, gInteger& dest);
  friend void     and(long x, const gInteger& y, gInteger& dest);
  friend void     or(long x, const gInteger& y, gInteger& dest);
  friend void     xor(long x, const gInteger& y, gInteger& dest);

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


typedef gInteger IntTmp; // for backward compatibility

inline gInteger::gInteger() :rep(&_ZeroRep) {}

inline gInteger::gInteger(IntRep* r) :rep(r) {}

inline gInteger::gInteger(int y) :rep(Icopy_long(0, (long)y)) {}

inline gInteger::gInteger(long y) :rep(Icopy_long(0, y)) {}

inline gInteger::gInteger(unsigned long y) :rep(Icopy_ulong(0, y)) {}

inline gInteger::gInteger(const gInteger&  y) :rep(Icopy(0, y.rep)) {}

inline gInteger::~gInteger() { if (rep && !STATIC_IntRep(rep)) delete rep; }

inline void  gInteger::operator = (const gInteger&  y)
{
  rep = Icopy(rep, y.rep);
}

inline void gInteger::operator = (long y)
{
  rep = Icopy_long(rep, y); 
}

inline int gInteger::initialized() const
{
  return rep != 0;
}

// procedural versions

inline int compare(const gInteger& x, const gInteger& y)
{
  return compare(x.rep, y.rep);
}

inline int ucompare(const gInteger& x, const gInteger& y)
{
  return ucompare(x.rep, y.rep);
}

inline int compare(const gInteger& x, long y)
{
  return compare(x.rep, y);
}

inline int ucompare(const gInteger& x, long y)
{
  return ucompare(x.rep, y);
}

inline int compare(long x, const gInteger& y)
{
  return -compare(y.rep, x);
}

inline int ucompare(long x, const gInteger& y)
{
  return -ucompare(y.rep, x);
}

inline void  add(const gInteger& x, const gInteger& y, gInteger& dest)
{
  dest.rep = add(x.rep, 0, y.rep, 0, dest.rep);
}

inline void  sub(const gInteger& x, const gInteger& y, gInteger& dest)
{
  dest.rep = add(x.rep, 0, y.rep, 1, dest.rep);
}

inline void  mul(const gInteger& x, const gInteger& y, gInteger& dest)
{
  dest.rep = multiply(x.rep, y.rep, dest.rep);
}

inline void  div(const gInteger& x, const gInteger& y, gInteger& dest)
{
  dest.rep = div(x.rep, y.rep, dest.rep);
}

inline void  mod(const gInteger& x, const gInteger& y, gInteger& dest)
{
  dest.rep = mod(x.rep, y.rep, dest.rep);
}

inline void  and(const gInteger& x, const gInteger& y, gInteger& dest)
{
  dest.rep = bitop(x.rep, y.rep, dest.rep, '&');
}

inline void  or(const gInteger& x, const gInteger& y, gInteger& dest)
{
  dest.rep = bitop(x.rep, y.rep, dest.rep, '|');
}

inline void  xor(const gInteger& x, const gInteger& y, gInteger& dest)
{
  dest.rep = bitop(x.rep, y.rep, dest.rep, '^');
}

inline void  lshift(const gInteger& x, const gInteger& y, gInteger& dest)
{
  dest.rep = lshift(x.rep, y.rep, 0, dest.rep);
}

inline void  rshift(const gInteger& x, const gInteger& y, gInteger& dest)
{
  dest.rep = lshift(x.rep, y.rep, 1, dest.rep);
}

inline void  pow(const gInteger& x, const gInteger& y, gInteger& dest)
{
  dest.rep = power(x.rep, Itolong(y.rep), dest.rep); // not incorrect
}

inline void  add(const gInteger& x, long y, gInteger& dest)
{
  dest.rep = add(x.rep, 0, y, dest.rep);
}

inline void  sub(const gInteger& x, long y, gInteger& dest)
{
  dest.rep = add(x.rep, 0, -y, dest.rep);
}

inline void  mul(const gInteger& x, long y, gInteger& dest)
{
  dest.rep = multiply(x.rep, y, dest.rep);
}

inline void  div(const gInteger& x, long y, gInteger& dest)
{
  dest.rep = div(x.rep, y, dest.rep);
}

inline void  mod(const gInteger& x, long y, gInteger& dest)
{
  dest.rep = mod(x.rep, y, dest.rep);
}

inline void  and(const gInteger& x, long y, gInteger& dest)
{
  dest.rep = bitop(x.rep, y, dest.rep, '&');
}

inline void  or(const gInteger& x, long y, gInteger& dest)
{
  dest.rep = bitop(x.rep, y, dest.rep, '|');
}

inline void  xor(const gInteger& x, long y, gInteger& dest)
{
  dest.rep = bitop(x.rep, y, dest.rep, '^');
}

inline void  lshift(const gInteger& x, long y, gInteger& dest)
{
  dest.rep = lshift(x.rep, y, dest.rep);
}

inline void  rshift(const gInteger& x, long y, gInteger& dest)
{
  dest.rep = lshift(x.rep, -y, dest.rep);
}

inline void  pow(const gInteger& x, long y, gInteger& dest)
{
  dest.rep = power(x.rep, y, dest.rep);
}

inline void abs(const gInteger& x, gInteger& dest)
{
  dest.rep = abs(x.rep, dest.rep);
}

inline void negate(const gInteger& x, gInteger& dest)
{
  dest.rep = negate(x.rep, dest.rep);
}

inline void complement(const gInteger& x, gInteger& dest)
{
  dest.rep = compl(x.rep, dest.rep);
}

inline void  add(long x, const gInteger& y, gInteger& dest)
{
  dest.rep = add(y.rep, 0, x, dest.rep);
}

inline void  sub(long x, const gInteger& y, gInteger& dest)
{
  dest.rep = add(y.rep, 1, x, dest.rep);
}

inline void  mul(long x, const gInteger& y, gInteger& dest)
{
  dest.rep = multiply(y.rep, x, dest.rep);
}

inline void  and(long x, const gInteger& y, gInteger& dest)
{
  dest.rep = bitop(y.rep, x, dest.rep, '&');
}

inline void  or(long x, const gInteger& y, gInteger& dest)
{
  dest.rep = bitop(y.rep, x, dest.rep, '|');
}

inline void  xor(long x, const gInteger& y, gInteger& dest)
{
  dest.rep = bitop(y.rep, x, dest.rep, '^');
}


// operator versions

inline int operator == (const gInteger&  x, const gInteger&  y)
{
  return compare(x, y) == 0; 
}

inline int operator == (const gInteger&  x, long y)
{
  return compare(x, y) == 0; 
}

inline int operator != (const gInteger&  x, const gInteger&  y)
{
  return compare(x, y) != 0; 
}

inline int operator != (const gInteger&  x, long y)
{
  return compare(x, y) != 0; 
}

inline int operator <  (const gInteger&  x, const gInteger&  y)
{
  return compare(x, y) <  0; 
}

inline int operator <  (const gInteger&  x, long y)
{
  return compare(x, y) <  0; 
}

inline int operator <= (const gInteger&  x, const gInteger&  y)
{
  return compare(x, y) <= 0; 
}

inline int operator <= (const gInteger&  x, long y)
{
  return compare(x, y) <= 0; 
}

inline int operator >  (const gInteger&  x, const gInteger&  y)
{
  return compare(x, y) >  0; 
}

inline int operator >  (const gInteger&  x, long y)
{
  return compare(x, y) >  0; 
}

inline int operator >= (const gInteger&  x, const gInteger&  y)
{
  return compare(x, y) >= 0; 
}

inline int operator >= (const gInteger&  x, long y)
{
  return compare(x, y) >= 0; 
}


inline void  gInteger::operator += (const gInteger& y)
{
  add(*this, y, *this);
}

inline void  gInteger::operator += (long y)
{
  add(*this, y, *this);
}

inline void gInteger::operator ++ ()
{
  add(*this, 1, *this);
}


inline void  gInteger::operator -= (const gInteger& y)
{
  sub(*this, y, *this);
}

inline void  gInteger::operator -= (long y)
{
  sub(*this, y, *this);
}

inline void gInteger::operator -- ()
{
  add(*this, -1, *this);
}



inline void gInteger::operator *= (const gInteger& y)
{
  mul(*this, y, *this);
}

inline void gInteger::operator *= (long y)
{
  mul(*this, y, *this);
}


inline void  gInteger::operator &= (const gInteger& y)
{
  and(*this, y, *this);
}

inline void  gInteger::operator &= (long y)
{
  and(*this, y, *this);
}

inline void  gInteger::operator |= (const gInteger& y)
{
  or(*this, y, *this);
}

inline void  gInteger::operator |= (long y)
{
  or(*this, y, *this);
}


inline void  gInteger::operator ^= (const gInteger& y)
{
  xor(*this, y, *this);
}

inline void  gInteger::operator ^= (long y)
{
  xor(*this, y, *this);
}



inline void gInteger::operator /= (const gInteger& y)
{
  div(*this, y, *this);
}

inline void gInteger::operator /= (long y)
{
  div(*this, y, *this);
}


inline void gInteger::operator <<= (const gInteger&  y)
{
  lshift(*this, y, *this);
}

inline void gInteger::operator <<= (long  y)
{
  lshift(*this, y, *this);
}


inline void gInteger::operator >>= (const gInteger&  y)
{
  rshift(*this, y, *this);
}

inline void  gInteger::operator >>= (long y)
{
  rshift(*this, y, *this);
}

#ifdef __GNUG__
inline gInteger operator <? (const gInteger& x, const gInteger& y)
{
  return (compare(x.rep, y.rep) <= 0) ? x : y;
}

inline gInteger operator >? (const gInteger& x, const gInteger& y)
{
  return (compare(x.rep, y.rep) >= 0)?  x : y;
}
#endif


inline void gInteger::abs()
{
  ::abs(*this, *this);
}

inline void gInteger::negate()
{
  ::negate(*this, *this);
}


inline void gInteger::complement()
{
  ::complement(*this, *this);
}


inline int sign(const gInteger& x)
{
  return (x.rep->len == 0) ? 0 : ( (x.rep->sgn == 1) ? 1 : -1 );
}

inline int even(const gInteger& y)
{
  return y.rep->len == 0 || !(y.rep->s[0] & 1);
}

inline int odd(const gInteger& y)
{
  return y.rep->len > 0 && (y.rep->s[0] & 1);
}

inline char* Itoa(const gInteger& y, int base, int width)
{
  return Itoa(y.rep, base, width);
}



inline long lg(const gInteger& x) 
{
  return lg(x.rep);
}

// constructive operations 

#if defined(__GNUG__) && !defined(NO_NRV)

inline gInteger  operator +  (const gInteger& x, const gInteger& y) return r
{
  add(x, y, r);
}

inline gInteger  operator +  (const gInteger& x, long y) return r
{
  add(x, y, r);
}

inline gInteger  operator +  (long  x, const gInteger& y) return r
{
  add(x, y, r);
}

inline gInteger  operator -  (const gInteger& x, const gInteger& y) return r
{
  sub(x, y, r);
}

inline gInteger  operator -  (const gInteger& x, long y) return r
{
  sub(x, y, r);
}

inline gInteger  operator -  (long  x, const gInteger& y) return r
{
  sub(x, y, r);
}

inline gInteger  operator *  (const gInteger& x, const gInteger& y) return r
{
  mul(x, y, r);
}

inline gInteger  operator *  (const gInteger& x, long y) return r
{
  mul(x, y, r);
}

inline gInteger  operator *  (long  x, const gInteger& y) return r
{
  mul(x, y, r);
}

inline gInteger sqr(const gInteger& x) return r
{
  mul(x, x, r);
}

inline gInteger  operator &  (const gInteger& x, const gInteger& y) return r
{
  and(x, y, r);
}

inline gInteger  operator &  (const gInteger& x, long y) return r
{
  and(x, y, r);
}

inline gInteger  operator &  (long  x, const gInteger& y) return r
{
  and(x, y, r);
}

inline gInteger  operator |  (const gInteger& x, const gInteger& y) return r
{
  or(x, y, r);
}

inline gInteger  operator |  (const gInteger& x, long y) return r
{
  or(x, y, r);
}

inline gInteger  operator |  (long  x, const gInteger& y) return r
{
  or(x, y, r);
}

inline gInteger  operator ^  (const gInteger& x, const gInteger& y) return r
{
  xor(x, y, r);
}

inline gInteger  operator ^  (const gInteger& x, long y) return r
{
  xor(x, y, r);
}

inline gInteger  operator ^  (long  x, const gInteger& y) return r
{
  xor(x, y, r);
}

inline gInteger  operator /  (const gInteger& x, const gInteger& y) return r
{
  div(x, y, r);
}

inline gInteger operator /  (const gInteger& x, long y) return r
{
  div(x, y, r);
}

inline gInteger operator %  (const gInteger& x, const gInteger& y) return r
{
  mod(x, y, r);
}

inline gInteger operator %  (const gInteger& x, long y) return r
{
  mod(x, y, r);
}

inline gInteger operator <<  (const gInteger& x, const gInteger& y) return r
{
  lshift(x, y, r);
}

inline gInteger operator <<  (const gInteger& x, long y) return r
{
  lshift(x, y, r);
}

inline gInteger operator >>  (const gInteger& x, const gInteger& y) return r;
{
  rshift(x, y, r);
}

inline gInteger operator >>  (const gInteger& x, long y) return r
{
  rshift(x, y, r);
}

inline gInteger pow(const gInteger& x, long y) return r
{
  pow(x, y, r);
}

inline gInteger Ipow(long x, long y) return r(x)
{
  pow(r, y, r);
}

inline gInteger pow(const gInteger& x, const gInteger& y) return r
{
  pow(x, y, r);
}



inline gInteger abs(const gInteger& x) return r
{
  abs(x, r);
}

inline gInteger operator - (const gInteger& x) return r
{
  negate(x, r);
}

inline gInteger operator ~ (const gInteger& x) return r
{
  complement(x, r);
}

inline gInteger  atoI(const char* s, int base) return r
{
  r.rep = atoIntRep(s, base);
}

inline gInteger  gcd(const gInteger& x, const gInteger& y) return r
{
  r.rep = gcd(x.rep, y.rep);
}

#else /* NO_NRV */

inline gInteger  operator +  (const gInteger& x, const gInteger& y) 
{
  gInteger r; add(x, y, r); return r;
}

inline gInteger  operator +  (const gInteger& x, long y) 
{
  gInteger r; add(x, y, r); return r;
}

inline gInteger  operator +  (long  x, const gInteger& y) 
{
  gInteger r; add(x, y, r); return r;
}

inline gInteger  operator -  (const gInteger& x, const gInteger& y) 
{
  gInteger r; sub(x, y, r); return r;
}

inline gInteger  operator -  (const gInteger& x, long y) 
{
  gInteger r; sub(x, y, r); return r;
}

inline gInteger  operator -  (long  x, const gInteger& y) 
{
  gInteger r; sub(x, y, r); return r;
}

inline gInteger  operator *  (const gInteger& x, const gInteger& y) 
{
  gInteger r; mul(x, y, r); return r;
}

inline gInteger  operator *  (const gInteger& x, long y) 
{
  gInteger r; mul(x, y, r); return r;
}

inline gInteger  operator *  (long  x, const gInteger& y) 
{
  gInteger r; mul(x, y, r); return r;
}

inline gInteger sqr(const gInteger& x) 
{
  gInteger r; mul(x, x, r); return r;
}

inline gInteger  operator &  (const gInteger& x, const gInteger& y) 
{
  gInteger r; and(x, y, r); return r;
}

inline gInteger  operator &  (const gInteger& x, long y) 
{
  gInteger r; and(x, y, r); return r;
}

inline gInteger  operator &  (long  x, const gInteger& y) 
{
  gInteger r; and(x, y, r); return r;
}

inline gInteger  operator |  (const gInteger& x, const gInteger& y) 
{
  gInteger r; or(x, y, r); return r;
}

inline gInteger  operator |  (const gInteger& x, long y) 
{
  gInteger r; or(x, y, r); return r;
}

inline gInteger  operator |  (long  x, const gInteger& y) 
{
  gInteger r; or(x, y, r); return r;
}

inline gInteger  operator ^  (const gInteger& x, const gInteger& y) 
{
  gInteger r; xor(x, y, r); return r;
}

inline gInteger  operator ^  (const gInteger& x, long y) 
{
  gInteger r; xor(x, y, r); return r;
}

inline gInteger  operator ^  (long  x, const gInteger& y) 
{
  gInteger r; xor(x, y, r); return r;
}

inline gInteger  operator /  (const gInteger& x, const gInteger& y) 
{
  gInteger r; div(x, y, r); return r;
}

inline gInteger operator /  (const gInteger& x, long y) 
{
  gInteger r; div(x, y, r); return r;
}

inline gInteger operator %  (const gInteger& x, const gInteger& y) 
{
  gInteger r; mod(x, y, r); return r;
}

inline gInteger operator %  (const gInteger& x, long y) 
{
  gInteger r; mod(x, y, r); return r;
}

inline gInteger operator <<  (const gInteger& x, const gInteger& y) 
{
  gInteger r; lshift(x, y, r); return r;
}

inline gInteger operator <<  (const gInteger& x, long y) 
{
  gInteger r; lshift(x, y, r); return r;
}

inline gInteger operator >>  (const gInteger& x, const gInteger& y) 
{
  gInteger r; rshift(x, y, r); return r;
}

inline gInteger operator >>  (const gInteger& x, long y) 
{
  gInteger r; rshift(x, y, r); return r;
}

inline gInteger pow(const gInteger& x, long y) 
{
  gInteger r; pow(x, y, r); return r;
}

inline gInteger Ipow(long x, long y) 
{
  gInteger r(x); pow(r, y, r); return r;
}

inline gInteger pow(const gInteger& x, const gInteger& y) 
{
  gInteger r; pow(x, y, r); return r;
}



inline gInteger abs(const gInteger& x) 
{
  gInteger r; abs(x, r); return r;
}

inline gInteger operator - (const gInteger& x) 
{
  gInteger r; negate(x, r); return r;
}

inline gInteger operator ~ (const gInteger& x) 
{
  gInteger r; complement(x, r); return r;
}

inline gInteger  atoI(const char* s, int base) 
{
  gInteger r; r.rep = atoIntRep(s, base); return r;
}

inline gInteger  gcd(const gInteger& x, const gInteger& y) 
{
  gInteger r; r.rep = gcd(x.rep, y.rep); return r;
}

#endif  /* NO_NRV */

inline void gInteger::operator %= (const gInteger& y)
{
  *this = *this % y; // mod(*this, y, *this) doesn't work.
}

inline void gInteger::operator %= (long y)
{
  *this = *this % y; // mod(*this, y, *this) doesn't work.
}

//
// This is for backward compatibility, and may be removed at any time
//
typedef gInteger Integer;

#endif /* _Integer_h */



