//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/integer.h
// Interface to an arbitrary-length integer class
//
// The original copyright and license are included below.
//

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

#ifndef LIBGAMBIT_INTEGER_H
#define LIBGAMBIT_INTEGER_H

#include <string>

namespace Gambit {

struct IntegerRep                    // internal Integer representations
{
  unsigned short  len;          // current length
  unsigned short  sz;           // allocated space (0 means static).
  short           sgn;          // 1 means >= 0; 0 means < 0 
  unsigned short  s[1];         // represented as ushort array starting here
};

// True if REP is staticly (or manually) allocated,
// and should not be deleted by an Integer destructor.
#define STATIC_IntegerRep(rep) ((rep)->sz==0)

extern IntegerRep*  Ialloc(IntegerRep*, const unsigned short *, int, int, int);
extern IntegerRep*  Icalloc(IntegerRep*, int);
extern IntegerRep*  Icopy_ulong(IntegerRep*, unsigned long);
extern IntegerRep*  Icopy_long(IntegerRep*, long);
extern IntegerRep*  Icopy(IntegerRep*, const IntegerRep*);
extern IntegerRep*  Iresize(IntegerRep*, int);
extern IntegerRep*  add(const IntegerRep*, int, const IntegerRep*, int, IntegerRep*);
extern IntegerRep*  add(const IntegerRep*, int, long, IntegerRep*);
extern IntegerRep*  multiply(const IntegerRep*, const IntegerRep*, IntegerRep*);
extern IntegerRep*  multiply(const IntegerRep*, long, IntegerRep*);
extern IntegerRep*  lshift(const IntegerRep*, long, IntegerRep*);
extern IntegerRep*  lshift(const IntegerRep*, const IntegerRep*, int, IntegerRep*);
extern IntegerRep*  bitop(const IntegerRep*, const IntegerRep*, IntegerRep*, char);
extern IntegerRep*  bitop(const IntegerRep*, long, IntegerRep*, char);
extern IntegerRep*  power(const IntegerRep*, long, IntegerRep*);
extern IntegerRep*  div(const IntegerRep*, const IntegerRep*, IntegerRep*);
extern IntegerRep*  mod(const IntegerRep*, const IntegerRep*, IntegerRep*);
extern IntegerRep*  div(const IntegerRep*, long, IntegerRep*);
extern IntegerRep*  mod(const IntegerRep*, long, IntegerRep*);
extern IntegerRep*  Compl(const IntegerRep*, IntegerRep*);
extern IntegerRep*  abs(const IntegerRep*, IntegerRep*);
extern IntegerRep*  negate(const IntegerRep*, IntegerRep*);
extern IntegerRep*  gcd(const IntegerRep*, const IntegerRep* y);
extern int      compare(const IntegerRep*, const IntegerRep*);
extern int      compare(const IntegerRep*, long);
extern int      ucompare(const IntegerRep*, const IntegerRep*);
extern int      ucompare(const IntegerRep*, long);
extern std::string Itoa(const IntegerRep* x, int base = 10, int width = 0);
extern std::string cvtItoa(const IntegerRep* x, std::string fmt, int& fmtlen, int base,
                           int showbase, int width, int align_right, 
                           char fillchar, char Xcase, int showpos);
extern IntegerRep*  atoIntegerRep(const char* s, int base = 10);
extern long     Itolong(const IntegerRep*);
extern double   Itodouble(const IntegerRep*);
extern int      Iislong(const IntegerRep*);
extern int      Iisdouble(const IntegerRep*);
extern long     lg(const IntegerRep*);

class Integer {
protected:
  IntegerRep *rep;

public:
  /// @name Lifecycle
  //@{
  Integer();
  explicit Integer(int);
  explicit Integer(long);
  explicit Integer(unsigned long);
  explicit Integer(IntegerRep *);
  Integer(const Integer &);
  ~Integer();

  Integer &operator=(const Integer &);
  Integer &operator=(long);
  //@}


  /// @name Unary operations on self
  //@{
  void operator ++ ();
  void operator -- ();
  void negate();          // negate in-place
  void abs();             // absolute-value in-place
  //@{

  /// @name Comparison operators
  //@{
  bool operator==(const Integer &) const;
  bool operator==(long) const;
  bool operator!=(const Integer &) const;
  bool operator!=(long) const;
  bool operator< (const Integer &) const;
  bool operator< (long) const;
  bool operator<=(const Integer &) const;
  bool operator<=(long) const;
  bool operator> (const Integer &) const;
  bool operator> (long) const;
  bool operator>=(const Integer &) const;
  bool operator>=(long) const;
  //@}

  /// @name Assignment-based operations
  //@{
  Integer &operator+=(const Integer &);
  Integer &operator-=(const Integer &);
  Integer &operator*=(const Integer &);
  Integer &operator/=(const Integer &);
  Integer &operator%=(const Integer &);
  Integer &operator<<=(const Integer &);
  Integer &operator>>=(const Integer &);

  Integer &operator+=(long);
  Integer &operator-=(long);
  Integer &operator*=(long);
  Integer &operator/=(long);
  Integer &operator%=(long);
  Integer &operator<<=(long);
  Integer &operator>>=(long);
  //@}

  /// @name Operator overloading
  //@{
  Integer operator-() const;
  Integer operator+(const Integer &) const;
  Integer operator+(long) const;
  Integer operator-(const Integer &) const;
  Integer operator-(long) const;
  Integer operator*(const Integer &) const;
  Integer operator*(long) const;
  Integer operator/(const Integer &) const;
  Integer operator/(long) const;
  Integer operator%(const Integer &) const;
  Integer operator%(long) const;
  Integer operator<<(const Integer &) const;
  Integer operator<<(long) const;
  Integer operator>>(const Integer &) const;
  Integer operator>>(long) const;
  //@}

  // builtin Integer functions that must be friends

  friend long     lg (const Integer&); // floor log base 2 of abs(x)
  friend double   ratio(const Integer& x, const Integer& y);
  // return x/y as a double

  friend Integer  gcd(const Integer&, const Integer&);
  friend int      even(const Integer&); // true if even
  friend int      odd(const Integer&); // true if odd
  friend int      sign(const Integer&); // returns -1, 0, +1

  friend void     setbit(Integer& x, long b);   // set b'th bit of x
  friend void     clearbit(Integer& x, long b); // clear b'th bit
  friend int      testbit(const Integer& x, long b);  // return b'th bit

  // procedural versions of operators

  friend void     abs(const Integer& x, Integer& dest);
  friend void     negate(const Integer& x, Integer& dest);
  friend void     complement(const Integer& x, Integer& dest);

  friend int      compare(const Integer&, const Integer&);  
  friend int      ucompare(const Integer&, const Integer&); 
  friend void     add(const Integer& x, const Integer& y, Integer& dest);
  friend void     sub(const Integer& x, const Integer& y, Integer& dest);
  friend void     mul(const Integer& x, const Integer& y, Integer& dest);
  friend void     div(const Integer& x, const Integer& y, Integer& dest);
  friend void     mod(const Integer& x, const Integer& y, Integer& dest);
  friend void     divide(const Integer& x, const Integer& y, 
                         Integer& q, Integer& r);
  friend void     lshift(const Integer& x, const Integer& y, Integer& dest);
  friend void     rshift(const Integer& x, const Integer& y, Integer& dest);
  friend void     pow(const Integer& x, const Integer& y, Integer& dest);

  friend int      compare(const Integer&, long);  
  friend int      ucompare(const Integer&, long); 
  friend void     add(const Integer& x, long y, Integer& dest);
  friend void     sub(const Integer& x, long y, Integer& dest);
  friend void     mul(const Integer& x, long y, Integer& dest);
  friend void     div(const Integer& x, long y, Integer& dest);
  friend void     mod(const Integer& x, long y, Integer& dest);
  friend void     divide(const Integer& x, long y, Integer& q, long& r);
  friend void     lshift(const Integer& x, long y, Integer& dest);
  friend void     rshift(const Integer& x, long y, Integer& dest);
  friend void     pow(const Integer& x, long y, Integer& dest);

  friend int      compare(long, const Integer&);  
  friend int      ucompare(long, const Integer&); 
  friend void     add(long x, const Integer& y, Integer& dest);
  friend void     sub(long x, const Integer& y, Integer& dest);
  friend void     mul(long x, const Integer& y, Integer& dest);

  // coercion & conversion

  int             fits_in_long() const { return Iislong(rep); }
  int             fits_in_double() const { return Iisdouble(rep); }

  long		  as_long() const { return Itolong(rep); }
  double	  as_double() const { return Itodouble(rep); }

  friend std::string Itoa(const Integer &x, int base /*= 10*/, int width /*= 0*/);
  friend Integer atoI(const char *s, int base/*= 10*/);
  
  friend std::istream &operator>>(std::istream &s, Integer& y);
  friend std::ostream &operator<<(std::ostream &s, const Integer& y);

  // error detection

  int             initialized() const;
  void   error(const char* msg) const;
  int             OK() const;  
};


//  (These are declared inline)

Integer  abs(const Integer&); // absolute value
Integer  sqr(const Integer&); // square

Integer  pow(const Integer& x, const Integer& y);
Integer  pow(const Integer& x, long y);
Integer  Ipow(long x, long y); // x to the y as Integer 


extern Integer  sqrt(const Integer&); // floor of square root
extern Integer  lcm(const Integer& x, const Integer& y); // least common mult

} // end namespace Gambit

#endif // LIBGAMBIT_INTEGER_H



