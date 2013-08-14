//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/integer.cc
// Implementation of an arbitrary-length integer class
//
// The original copyright and license are included below.

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

/*
  Some of the following algorithms are very loosely based on those from 
  MIT C-Scheme bignum.c, which is
      Copyright (c) 1987 Massachusetts Institute of Technology

  with other guidance from Knuth, vol. 2

  Thanks to the creators of the algorithms.
*/

#include <iostream>

#include "integer.h"
#include <cctype>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cstring>
#include "libgambit/libgambit.h"

namespace Gambit {

long lg(unsigned long x)
{
  long l = 0;
  while (x > 1)
  {
    x = x >> 1;
    ++l;
  }
  return l;
}


#ifndef HUGE_VAL
#ifdef HUGE
#define HUGE_VAL HUGE
#else
#define HUGE_VAL DBL_MAX
#endif
#endif

/*
 Sizes of shifts for multiple-precision arithmetic.
 These should not be changed unless Integer representation
 as unsigned shorts is changed in the implementation files.
*/

#define I_SHIFT         (sizeof(short) * CHAR_BIT)
#define I_RADIX         ((unsigned long)(1L << I_SHIFT))
#define I_MAXNUM        ((unsigned long)((I_RADIX - 1)))
#define I_MINNUM        ((unsigned long)(I_RADIX >> 1))
#define I_POSITIVE      1
#define I_NEGATIVE      0

/* All routines assume SHORT_PER_LONG > 1 */
#define SHORT_PER_LONG  ((unsigned)(((sizeof(long) + sizeof(short) - 1) / sizeof(short))))
#define CHAR_PER_LONG   ((unsigned)sizeof(long))

/*
  minimum and maximum sizes for an IntegerRep
*/

#define MIN_INTREP_SIZE   16
#define MAX_INTREP_SIZE   I_MAXNUM

#ifndef MALLOC_MIN_OVERHEAD
#define MALLOC_MIN_OVERHEAD 4
#endif

static IntegerRep _ZeroRep = {1, 0, 1, {0}};
static IntegerRep _OneRep = {1, 0, 1, {1}};
static IntegerRep _MinusOneRep = {1, 0, 0, {1}};


// utilities to extract and transfer bits

// get low bits

inline static unsigned short extract(unsigned long x)
{
  return (unsigned short) (x & I_MAXNUM);
}

// transfer high bits to low

inline static unsigned long down(unsigned long x)
{
  return (x >> I_SHIFT) & I_MAXNUM;
}

// transfer low bits to high

inline static unsigned long up(unsigned long x)
{
  return x << I_SHIFT;
}

// compare two equal-length reps

static int docmp(const unsigned short* x, const unsigned short* y, int l)
{
  int diff = 0;
  const unsigned short* xs = &(x[l]);
  const unsigned short* ys = &(y[l]);
  while (l-- > 0 && (diff = (*--xs) - (*--ys)) == 0);
  return diff;
}

// figure out max length of result of +, -, etc.

inline static int calc_len(int len1, int len2, int pad)
{
  return (len1 >= len2)? len1 + pad : len2 + pad;
}

// ensure len & sgn are correct

static void Icheck(IntegerRep* rep)
{
  int l = rep->len;
  const unsigned short* p = &(rep->s[l]);
  while (l > 0 && *--p == 0) --l;
  if ((rep->len = l) == 0) rep->sgn = I_POSITIVE;
}


// zero out the end of a rep

static void Iclear_from(IntegerRep* rep, int p)
{
  unsigned short* cp = &(rep->s[p]);
  const unsigned short* cf = &(rep->s[rep->len]);
  while(cp < cf) *cp++ = 0;
}

// copy parts of a rep

void scpy(const unsigned short* src, unsigned short* dest,int nb)
{
  while (--nb >= 0) *dest++ = *src++;
}

// make sure an argument is valid

static inline void nonnil(const IntegerRep* rep)
{
  if (rep == 0) {
    throw Gambit::NullException();
  }
}

// allocate a new Irep. Pad to something close to a power of two.

static IntegerRep* Inew(int newlen)
{
  unsigned int siz = sizeof(IntegerRep) + newlen * sizeof(short) + 
    MALLOC_MIN_OVERHEAD;
  unsigned int allocsiz = MIN_INTREP_SIZE;
  while (allocsiz < siz) allocsiz <<= 1;  // find a power of 2
  allocsiz -= MALLOC_MIN_OVERHEAD;
  //assert((unsigned long) allocsiz < MAX_INTREP_SIZE * sizeof(short));
    
  IntegerRep* rep = (IntegerRep *) new char[allocsiz];
  rep->sz = (allocsiz - sizeof(IntegerRep) + sizeof(short)) / sizeof(short);
  return rep;
}

// allocate: use the bits in src if non-null, clear the rest

IntegerRep* Ialloc(IntegerRep* old, const unsigned short* src, int srclen, int newsgn,
              int newlen)
{
  IntegerRep* rep;
  if (old == 0 || newlen > old->sz)
    rep = Inew(newlen);
  else
    rep = old;

  rep->len = newlen;
  rep->sgn = newsgn;

  scpy(src, rep->s, srclen);
  Iclear_from(rep, srclen);

  if (old != rep && old != 0 && !STATIC_IntegerRep(old)) delete old;
  return rep;
}

// allocate and clear

IntegerRep* Icalloc(IntegerRep* old, int newlen)
{
  IntegerRep* rep;
  if (old == 0 || newlen > old->sz)
  {
    if (old != 0 && !STATIC_IntegerRep(old)) delete old;
    rep = Inew(newlen);
  }
  else
    rep = old;

  rep->len = newlen;
  rep->sgn = I_POSITIVE;
  Iclear_from(rep, 0);

  return rep;
}

// reallocate

IntegerRep* Iresize(IntegerRep* old, int newlen)
{
  IntegerRep* rep;
  unsigned short oldlen;
  if (old == 0)
  {
    oldlen = 0;
    rep = Inew(newlen);
    rep->sgn = I_POSITIVE;
  }
  else 
  {
    oldlen = old->len;
    if (newlen > old->sz)
    {
      rep = Inew(newlen);
      scpy(old->s, rep->s, oldlen);
      rep->sgn = old->sgn;
      if (!STATIC_IntegerRep(old)) delete old;
    }
    else
      rep = old;
  }

  rep->len = newlen;
  Iclear_from(rep, oldlen);

  return rep;
}


// same, for straight copy

IntegerRep* Icopy(IntegerRep* old, const IntegerRep* src)
{
  if (old == src) return old; 
  IntegerRep* rep;
  if (src == 0)
  {
    if (old == 0)
      rep = Inew(0);
    else
    {
      rep = old;
      Iclear_from(rep, 0);
    }
    rep->len = 0;
    rep->sgn = I_POSITIVE;
  }
  else 
  {
    int newlen = src->len;
    if (old == 0 || newlen > old->sz)
    {
      if (old != 0 && !STATIC_IntegerRep(old)) delete old;
      rep = Inew(newlen);
    }
    else
      rep = old;

    rep->len = newlen;
    rep->sgn = src->sgn;

    scpy(src->s, rep->s, newlen);
  }

  return rep;
}

// allocate & copy space for a long

IntegerRep* Icopy_long(IntegerRep* old, long x)
{
  int newsgn = (x >= 0);
  IntegerRep* rep = Icopy_ulong(old, newsgn ? x : -x);
  rep->sgn = newsgn;
  return rep;
}

IntegerRep* Icopy_ulong(IntegerRep* old, unsigned long x)
{
  unsigned short src[SHORT_PER_LONG];
  
  unsigned short srclen = 0;
  while (x != 0)
  {
    src[srclen++] = extract(x);
    x = down(x);
  }

  IntegerRep* rep;
  if (old == 0 || srclen > old->sz)
  {
    if (old != 0 && !STATIC_IntegerRep(old)) delete old;
    rep = Inew(srclen);
  }
  else
    rep = old;

  rep->len = srclen;
  rep->sgn = I_POSITIVE;

  scpy(src, rep->s, srclen);

  return rep;
}

// special case for zero -- it's worth it!

IntegerRep* Icopy_zero(IntegerRep* old)
{
  if (old == 0 || STATIC_IntegerRep(old))
    return &_ZeroRep;

  old->len = 0;
  old->sgn = I_POSITIVE;

  return old;
}

// special case for 1 or -1

IntegerRep* Icopy_one(IntegerRep* old, int newsgn)
{
  if (old == 0 || 1 > old->sz)
  {
    if (old != 0 && !STATIC_IntegerRep(old)) delete old;
    return newsgn==I_NEGATIVE ? &_MinusOneRep : &_OneRep;
  }

  old->sgn = newsgn;
  old->len = 1;
  old->s[0] = 1;

  return old;
}

// convert to a legal two's complement long if possible
// if too big, return most negative/positive value

long Itolong(const IntegerRep* rep)
{ 
  if ((unsigned)(rep->len) > (unsigned)(SHORT_PER_LONG))
    return (rep->sgn == I_POSITIVE) ? LONG_MAX : LONG_MIN;
  else if (rep->len == 0)
    return 0;
  else if ((unsigned)(rep->len) < (unsigned)(SHORT_PER_LONG))
  {
    unsigned long a = rep->s[rep->len-1];
#ifndef __BCC55__
    // This condition is always false under BCC55
    if (SHORT_PER_LONG > 2) // normally optimized out
    {
		for (int i = rep->len - 2; i >= 0; --i)
		  a = up(a) | rep->s[i];
    }
#endif  // __BCC55__
    return (rep->sgn == I_POSITIVE)? a : -((long)a);
  }
  else 
  {
    unsigned long a = rep->s[SHORT_PER_LONG - 1];
    if (a >= I_MINNUM)
      return (rep->sgn == I_POSITIVE) ? LONG_MAX : LONG_MIN;
    else
    {
      a = up(a) | rep->s[SHORT_PER_LONG - 2];
#ifndef __BCC55__
      // This condition is always false under BCC55
      if (SHORT_PER_LONG > 2)
      {
		  for (int i = SHORT_PER_LONG - 3; i >= 0; --i)
			 a = up(a) | rep->s[i];
      }
#endif  // __BCC55__
      return (rep->sgn == I_POSITIVE)? a : -((long)a);
    }
  }
}

// test whether op long() will work.
// careful about asymmetry between LONG_MIN & LONG_MAX

int Iislong(const IntegerRep* rep)
{
  unsigned int l = rep->len;
  if (l < SHORT_PER_LONG)
    return 1;
  else if (l > SHORT_PER_LONG)
    return 0;
  else if ((unsigned)(rep->s[SHORT_PER_LONG - 1]) < (unsigned)(I_MINNUM))
    return 1;
  else if (rep->sgn == I_NEGATIVE && rep->s[SHORT_PER_LONG - 1] == I_MINNUM)
  {
    for (unsigned int i = 0; i < SHORT_PER_LONG - 1; ++i)
      if (rep->s[i] != 0)
        return 0;
    return 1;
  }
  else
    return 0;
}

// convert to a double 

double Itodouble(const IntegerRep* rep)
{ 
  double d = 0.0;
  double bound = DBL_MAX / 2.0;
  for (int i = rep->len - 1; i >= 0; --i)
  {
	 unsigned short a = (unsigned short) (I_RADIX >> 1);
	 while (a != 0)
    {
      if (d >= bound)
        return (rep->sgn == I_NEGATIVE) ? -HUGE_VAL : HUGE_VAL;
      d *= 2.0;
      if (rep->s[i] & a)
        d += 1.0;
      a >>= 1;
    }
  }
  if (rep->sgn == I_NEGATIVE)
    return -d;
  else
    return d;
}

// see whether op double() will work-
// have to actually try it in order to find out
// since otherwise might trigger fp exception

int Iisdouble(const IntegerRep* rep)
{
  double d = 0.0;
  double bound = DBL_MAX / 2.0;
  for (int i = rep->len - 1; i >= 0; --i)
  {
	 unsigned short a = (unsigned short) (I_RADIX >> 1);
    while (a != 0)
    {
      if (d > bound || (d == bound && (i > 0 || (rep->s[i] & a))))
        return 0;
      d *= 2.0;
      if (rep->s[i] & a)
        d += 1.0;
      a >>= 1;
    }
  }
  return 1;
}

// real division of num / den

double ratio(const Integer& num, const Integer& den)
{
  Integer q, r;
  divide(num, den, q, r);
  double d1 = q.as_double();
 
  if (d1 >= DBL_MAX || d1 <= -DBL_MAX || sign(r) == 0)
    return d1;
  else      // use as much precision as available for fractional part
  {
    double  d2 = 0.0;
    double  d3 = 0.0; 
    int cont = 1;
    for (int i = den.rep->len - 1; i >= 0 && cont; --i)
    {
		unsigned short a = (unsigned short) (I_RADIX >> 1);
      while (a != 0)
      {
        if (d2 + 1.0 == d2) // out of precision when we get here
        {
          cont = 0;
          break;
        }

        d2 *= 2.0;
        if (den.rep->s[i] & a)
          d2 += 1.0;

        if (i < r.rep->len)
        {
          d3 *= 2.0;
          if (r.rep->s[i] & a)
            d3 += 1.0;
        }

        a >>= 1;
      }
    }

    if (sign(r) < 0)
      d3 = -d3;
    return d1 + d3 / d2;
  }
}

// comparison functions
  
int compare(const IntegerRep* x, const IntegerRep* y)
{
  int diff  = x->sgn - y->sgn;
  if (diff == 0)
  {
    diff = x->len - y->len;
    if (diff == 0)
      diff = docmp(x->s, y->s, x->len);
    if (x->sgn == I_NEGATIVE)
      diff = -diff;
  }
  return diff;
}

int ucompare(const IntegerRep* x, const IntegerRep* y)
{
  int diff = x->len - y->len;
  if (diff == 0)
  {
    int l = x->len;
    const unsigned short* xs = &(x->s[l]);
    const unsigned short* ys = &(y->s[l]);
    while (l-- > 0 && (diff = (*--xs) - (*--ys)) == 0);
  }
  return diff;
}

int compare(const IntegerRep* x, long  y)
{
  int xl = x->len;
  int xsgn = x->sgn;
  if (y == 0)
  {
    if (xl == 0)
      return 0;
    else if (xsgn == I_NEGATIVE)
      return -1;
    else
      return 1;
  }
  else
  {
    int ysgn = y >= 0;
    unsigned long uy = (ysgn)? y : -y;
    int diff = xsgn - ysgn;
    if (diff == 0)
    {
      diff = xl - SHORT_PER_LONG;
      if (diff <= 0)
      {
        unsigned short tmp[SHORT_PER_LONG];
        int yl = 0;
        while (uy != 0)
        {
          tmp[yl++] = extract(uy);
          uy = down(uy);
        }
        diff = xl - yl;
        if (diff == 0)
          diff = docmp(x->s, tmp, xl);
      }
      if (xsgn == I_NEGATIVE)
	diff = -diff;
    }
    return diff;
  }
}

int ucompare(const IntegerRep* x, long  y)
{
  int xl = x->len;
  if (y == 0)
    return xl;
  else
  {
    unsigned long uy = (y >= 0)? y : -y;
    int diff = xl - SHORT_PER_LONG;
    if (diff <= 0)
    {
      unsigned short tmp[SHORT_PER_LONG];
      int yl = 0;
      while (uy != 0)
      {
        tmp[yl++] = extract(uy);
        uy = down(uy);
      }
      diff = xl - yl;
      if (diff == 0)
        diff = docmp(x->s, tmp, xl);
    }
    return diff;
  }
}



// arithmetic functions

IntegerRep* add(const IntegerRep* x, int negatex, 
            const IntegerRep* y, int negatey, IntegerRep* r)
{
  nonnil(x);
  nonnil(y);

  int xl = x->len;
  int yl = y->len;

  int xsgn = (negatex && xl != 0) ? !x->sgn : x->sgn;
  int ysgn = (negatey && yl != 0) ? !y->sgn : y->sgn;

  int xrsame = x == r;
  int yrsame = y == r;

  if (yl == 0)
    r = Ialloc(r, x->s, xl, xsgn, xl);
  else if (xl == 0)
    r = Ialloc(r, y->s, yl, ysgn, yl);
  else if (xsgn == ysgn)
  {
    if (xrsame || yrsame)
      r = Iresize(r, calc_len(xl, yl, 1));
    else
      r = Icalloc(r, calc_len(xl, yl, 1));
    r->sgn = xsgn;
    unsigned short* rs = r->s;
    const unsigned short* as;
    const unsigned short* bs;
    const unsigned short* topa;
    const unsigned short* topb;
    if (xl >= yl)
    {
      as =  (xrsame)? r->s : x->s;
      topa = &(as[xl]);
      bs =  (yrsame)? r->s : y->s;
      topb = &(bs[yl]);
    }
    else
    {
      bs =  (xrsame)? r->s : x->s;
      topb = &(bs[xl]);
      as =  (yrsame)? r->s : y->s;
      topa = &(as[yl]);
    }
    unsigned long sum = 0;
    while (bs < topb)
    {
      sum += (unsigned long)(*as++) + (unsigned long)(*bs++);
      *rs++ = extract(sum);
      sum = down(sum);
    }
    while (sum != 0 && as < topa)
    {
      sum += (unsigned long)(*as++);
      *rs++ = extract(sum);
      sum = down(sum);
    }
    if (sum != 0)
      *rs = extract(sum);
    else if (rs != as)
      while (as < topa)
        *rs++ = *as++;
  }
  else
  {
    int comp = ucompare(x, y);
    if (comp == 0)
      r = Icopy_zero(r);
    else
    {
      if (xrsame || yrsame)
        r = Iresize(r, calc_len(xl, yl, 0));
      else
        r = Icalloc(r, calc_len(xl, yl, 0));
      unsigned short* rs = r->s;
      const unsigned short* as;
      const unsigned short* bs;
      const unsigned short* topa;
      const unsigned short* topb;
      if (comp > 0)
      {
        as =  (xrsame)? r->s : x->s;
        topa = &(as[xl]);
        bs =  (yrsame)? r->s : y->s;
        topb = &(bs[yl]);
        r->sgn = xsgn;
      }
      else
      {
        bs =  (xrsame)? r->s : x->s;
        topb = &(bs[xl]);
        as =  (yrsame)? r->s : y->s;
        topa = &(as[yl]);
        r->sgn = ysgn;
      }
      unsigned long hi = 1;
      while (bs < topb)
      {
        hi += (unsigned long)(*as++) + I_MAXNUM - (unsigned long)(*bs++);
        *rs++ = extract(hi);
        hi = down(hi);
      }
      while (hi == 0 && as < topa)
      {
        hi = (unsigned long)(*as++) + I_MAXNUM;
        *rs++ = extract(hi);
        hi = down(hi);
      }
      if (rs != as)
        while (as < topa)
          *rs++ = *as++;
    }
  }
  Icheck(r);
  return r;
}


IntegerRep* add(const IntegerRep* x, int negatex, long y, IntegerRep* r)
{
  nonnil(x);
  int xl = x->len;
  int xsgn = (negatex && xl != 0) ? !x->sgn : x->sgn;
  int xrsame = x == r;

  int ysgn = (y >= 0);
  unsigned long uy = (ysgn)? y : -y;

  if (y == 0)
    r = Ialloc(r, x->s, xl, xsgn, xl);
  else if (xl == 0)
    r = Icopy_long(r, y);
  else if (xsgn == ysgn)
  {
    if (xrsame)
      r = Iresize(r, calc_len(xl, SHORT_PER_LONG, 1));
    else
      r = Icalloc(r, calc_len(xl, SHORT_PER_LONG, 1));
    r->sgn = xsgn;
    unsigned short* rs = r->s;
    const unsigned short* as =  (xrsame)? r->s : x->s;
    const unsigned short* topa = &(as[xl]);
    unsigned long sum = 0;
    while (as < topa && uy != 0)
    {
      unsigned long u = extract(uy);
      uy = down(uy);
      sum += (unsigned long)(*as++) + u;
      *rs++ = extract(sum);
      sum = down(sum);
    }
    while (sum != 0 && as < topa)
    {
      sum += (unsigned long)(*as++);
      *rs++ = extract(sum);
      sum = down(sum);
    }
    if (sum != 0)
      *rs = extract(sum);
    else if (rs != as)
      while (as < topa)
        *rs++ = *as++;
  }
  else
  {
    unsigned short tmp[SHORT_PER_LONG];
    int yl = 0;
    while (uy != 0)
    {
      tmp[yl++] = extract(uy);
      uy = down(uy);
    }
    int comp = xl - yl;
    if (comp == 0)
      comp = docmp(x->s, tmp, yl);
    if (comp == 0)
      r = Icopy_zero(r);
    else
    {
      if (xrsame)
        r = Iresize(r, calc_len(xl, yl, 0));
      else
        r = Icalloc(r, calc_len(xl, yl, 0));
      unsigned short* rs = r->s;
      const unsigned short* as;
      const unsigned short* bs;
      const unsigned short* topa;
      const unsigned short* topb;
      if (comp > 0)
      {
        as =  (xrsame)? r->s : x->s;
        topa = &(as[xl]);
        bs =  tmp;
        topb = &(bs[yl]);
        r->sgn = xsgn;
      }
      else
      {
        bs =  (xrsame)? r->s : x->s;
        topb = &(bs[xl]);
        as =  tmp;
        topa = &(as[yl]);
        r->sgn = ysgn;
      }
      unsigned long hi = 1;
      while (bs < topb)
      {
        hi += (unsigned long)(*as++) + I_MAXNUM - (unsigned long)(*bs++);
        *rs++ = extract(hi);
        hi = down(hi);
      }
      while (hi == 0 && as < topa)
      {
        hi = (unsigned long)(*as++) + I_MAXNUM;
        *rs++ = extract(hi);
        hi = down(hi);
      }
      if (rs != as)
        while (as < topa)
          *rs++ = *as++;
    }
  }
  Icheck(r);
  return r;
}


IntegerRep* multiply(const IntegerRep* x, const IntegerRep* y, IntegerRep* r)
{
  nonnil(x);
  nonnil(y);
  int xl = x->len;
  int yl = y->len;
  int rl = xl + yl;
  int rsgn = x->sgn == y->sgn;
  int xrsame = x == r;
  int yrsame = y == r;
  int xysame = x == y;
  
  if (xl == 0 || yl == 0)
    r = Icopy_zero(r);
  else if (xl == 1 && x->s[0] == 1)
    r = Icopy(r, y);
  else if (yl == 1 && y->s[0] == 1)
    r = Icopy(r, x);
  else if (!(xysame && xrsame))
  {
    if (xrsame || yrsame)
      r = Iresize(r, rl);
    else
      r = Icalloc(r, rl);
    unsigned short* rs = r->s;
    unsigned short* topr = &(rs[rl]);

    // use best inner/outer loop params given constraints
    unsigned short* currentr;
    const unsigned short* bota;
    const unsigned short* as;
    const unsigned short* botb;
    const unsigned short* topb;
    if (xrsame)                 
    { 
      currentr = &(rs[xl-1]);
      bota = rs;
      as = currentr;
      botb = y->s;
      topb = &(botb[yl]);
    }
    else if (yrsame)
    {
      currentr = &(rs[yl-1]);
      bota = rs;
      as = currentr;
      botb = x->s;
      topb = &(botb[xl]);
    }
    else if (xl <= yl)
    {
      currentr = &(rs[xl-1]);
      bota = x->s;
      as = &(bota[xl-1]);
      botb = y->s;
      topb = &(botb[yl]);
    }
    else
    {
      currentr = &(rs[yl-1]);
      bota = y->s;
      as = &(bota[yl-1]);
      botb = x->s;
      topb = &(botb[xl]);
    }

    while (as >= bota)
    {
      unsigned long ai = (unsigned long)(*as--);
      unsigned short* rs = currentr--;
      *rs = 0;
      if (ai != 0)
      {
        unsigned long sum = 0;
        const unsigned short* bs = botb;
        while (bs < topb)
        {
          sum += ai * (unsigned long)(*bs++) + (unsigned long)(*rs);
          *rs++ = extract(sum);
          sum = down(sum);
        }
        while (sum != 0 && rs < topr)
        {
          sum += (unsigned long)(*rs);
          *rs++ = extract(sum);
          sum = down(sum);
        }
      }
    }
  }
  else                          // x, y, and r same; compute over diagonals
  {
    r = Iresize(r, rl);
    unsigned short* botr = r->s;
    unsigned short* topr = &(botr[rl]);
    unsigned short* rs =   &(botr[rl - 2]);

    const unsigned short* bota = (xrsame)? botr : x->s;
    const unsigned short* loa =  &(bota[xl - 1]);
    const unsigned short* hia =  loa;

    for (; rs >= botr; --rs)
    {
      const unsigned short* h = hia;
      const unsigned short* l = loa;
      unsigned long prod = (unsigned long)(*h) * (unsigned long)(*l);
      *rs = 0;

      for(;;)
      {
        unsigned short* rt = rs;
        unsigned long sum = prod + (unsigned long)(*rt);
        *rt++ = extract(sum);
        sum = down(sum);
        while (sum != 0 && rt < topr)
        {
          sum += (unsigned long)(*rt);
          *rt++ = extract(sum);
          sum = down(sum);
        }
        if (h > l)
        {
          rt = rs;
          sum = prod + (unsigned long)(*rt);
          *rt++ = extract(sum);
          sum = down(sum);
          while (sum != 0 && rt < topr)
          {
            sum += (unsigned long)(*rt);
            *rt++ = extract(sum);
            sum = down(sum);
          }
          if (--h >= ++l)
            prod = (unsigned long)(*h) * (unsigned long)(*l);
          else
            break;
        }
        else
          break;
      }
      if (loa > bota)
        --loa;
      else
        --hia;
    }
  }
  r->sgn = rsgn;
  Icheck(r);
  return r;
}


IntegerRep* multiply(const IntegerRep* x, long y, IntegerRep* r)
{
  nonnil(x);
  int xl = x->len;
    
  if (xl == 0 || y == 0)
    r = Icopy_zero(r);
  else if (y == 1)
    r = Icopy(r, x);
  else
  {
    int ysgn = y >= 0;
    int rsgn = x->sgn == ysgn;
    unsigned long uy = (ysgn)? y : -y;
    unsigned short tmp[SHORT_PER_LONG];
    int yl = 0;
    while (uy != 0)
    {
      tmp[yl++] = extract(uy);
      uy = down(uy);
    }

    int rl = xl + yl;
    int xrsame = x == r;
    if (xrsame)
      r = Iresize(r, rl);
    else
      r = Icalloc(r, rl);

    unsigned short* rs = r->s;
    unsigned short* topr = &(rs[rl]);
    unsigned short* currentr;
    const unsigned short* bota;
    const unsigned short* as;
    const unsigned short* botb;
    const unsigned short* topb;

    if (xrsame)
    { 
      currentr = &(rs[xl-1]);
      bota = rs;
      as = currentr;
      botb = tmp;
      topb = &(botb[yl]);
    }
    else if (xl <= yl)
    {
      currentr = &(rs[xl-1]);
      bota = x->s;
      as = &(bota[xl-1]);
      botb = tmp;
      topb = &(botb[yl]);
    }
    else
    {
      currentr = &(rs[yl-1]);
      bota = tmp;
      as = &(bota[yl-1]);
      botb = x->s;
      topb = &(botb[xl]);
    }

    while (as >= bota)
    {
      unsigned long ai = (unsigned long)(*as--);
      unsigned short* rs = currentr--;
      *rs = 0;
      if (ai != 0)
      {
        unsigned long sum = 0;
        const unsigned short* bs = botb;
        while (bs < topb)
        {
          sum += ai * (unsigned long)(*bs++) + (unsigned long)(*rs);
          *rs++ = extract(sum);
          sum = down(sum);
        }
        while (sum != 0 && rs < topr)
        {
          sum += (unsigned long)(*rs);
          *rs++ = extract(sum);
          sum = down(sum);
        }
      }
    }
    r->sgn = rsgn;
  }
  Icheck(r);
  return r;
}


// main division routine

static void do_divide(unsigned short* rs,
                      const unsigned short* ys, int yl,
                      unsigned short* qs, int ql)
{
  const unsigned short* topy = &(ys[yl]);
  unsigned short d1 = ys[yl - 1];
  unsigned short d2 = ys[yl - 2];
 
  int l = ql - 1;
  int i = l + yl;
  
  for (; l >= 0; --l, --i)
  {
    unsigned short qhat;       // guess q
    if (d1 == rs[i])
		qhat = (unsigned short) I_MAXNUM;
    else
    {
      unsigned long lr = up((unsigned long)rs[i]) | rs[i-1];
		qhat = (unsigned short) (lr / d1);
    }

    for(;;)     // adjust q, use docmp to avoid overflow problems
    {
      unsigned short ts[3];
      unsigned long prod = (unsigned long)d2 * (unsigned long)qhat;
      ts[0] = extract(prod);
      prod = down(prod) + (unsigned long)d1 * (unsigned long)qhat;
      ts[1] = extract(prod);
      ts[2] = extract(down(prod));
      if (docmp(ts, &(rs[i-2]), 3) > 0)
        --qhat;
      else
        break;
    };
    
    // multiply & subtract
    
    const unsigned short* yt = ys;
    unsigned short* rt = &(rs[l]);
    unsigned long prod = 0;
    unsigned long hi = 1;
    while (yt < topy)
    {
      prod = (unsigned long)qhat * (unsigned long)(*yt++) + down(prod);
      hi += (unsigned long)(*rt) + I_MAXNUM - (unsigned long)(extract(prod));
      *rt++ = extract(hi);
      hi = down(hi);
    }
    hi += (unsigned long)(*rt) + I_MAXNUM - (unsigned long)(down(prod));
    *rt = extract(hi);
    hi = down(hi);
    
    // off-by-one, add back
    
    if (hi == 0)
    {
      --qhat;
      yt = ys;
      rt = &(rs[l]);
      hi = 0;
      while (yt < topy)
      {
        hi = (unsigned long)(*rt) + (unsigned long)(*yt++) + down(hi);
        *rt++ = extract(hi);
      }
      *rt = 0;
    }
    if (qs != 0)
      qs[l] = qhat;
  }
}

// divide by single digit, return remainder
// if q != 0, then keep the result in q, else just compute rem

static int unscale(const unsigned short* x, int xl, unsigned short y,
                   unsigned short* q)
{
  if (xl == 0 || y == 1)
    return 0;
  else if (q != 0)
  {
    unsigned short* botq = q;
    unsigned short* qs = &(botq[xl - 1]);
    const unsigned short* xs = &(x[xl - 1]);
    unsigned long rem = 0;
    while (qs >= botq)
    {
      rem = up(rem) | *xs--;
      unsigned long u = rem / y;
      *qs-- = extract(u);
      rem -= u * y;
    }
    int r = extract(rem);
    return r;
  }
  else                          // same loop, a bit faster if just need rem
  {
    const unsigned short* botx = x;
    const unsigned short* xs = &(botx[xl - 1]);
    unsigned long rem = 0;
    while (xs >= botx)
    {
      rem = up(rem) | *xs--;
      unsigned long u = rem / y;
      rem -= u * y;
    }
    int r = extract(rem);
    return r;
  }
}


IntegerRep* div(const IntegerRep* x, const IntegerRep* y, IntegerRep* q)
{
  nonnil(x);
  nonnil(y);
  int xl = x->len;
  int yl = y->len;
  if (yl == 0) {
    throw Gambit::ZeroDivideException();
  }

  int comp = ucompare(x, y);
  int xsgn = x->sgn;
  int ysgn = y->sgn;

  int samesign = xsgn == ysgn;

  if (comp < 0)
    q = Icopy_zero(q);
  else if (comp == 0)
    q = Icopy_one(q, samesign);
  else if (yl == 1)
  {
    q = Icopy(q, x);
    unscale(q->s, q->len, y->s[0], q->s);
  }
  else
  {
    IntegerRep* yy = 0;
    IntegerRep* r  = 0;
	 unsigned short prescale = (unsigned short) (I_RADIX / (1 + y->s[yl - 1]));
    if (prescale != 1 || y == q)
    {
      yy = multiply(y, ((long)prescale & I_MAXNUM), yy);
      r = multiply(x, ((long)prescale & I_MAXNUM), r);
    }
    else
    {
      yy = (IntegerRep*)y;
      r = Icalloc(r, xl + 1);
      scpy(x->s, r->s, xl);
    }

    int ql = xl - yl + 1;
      
    q = Icalloc(q, ql);
    do_divide(r->s, yy->s, yl, q->s, ql);

    if (yy != y && !STATIC_IntegerRep(yy)) delete yy;
    if (!STATIC_IntegerRep(r)) delete r;
  }
  q->sgn = samesign;
  Icheck(q);
  return q;
}

IntegerRep* div(const IntegerRep* x, long y, IntegerRep* q)
{
  nonnil(x);
  int xl = x->len;
  if (y == 0) {
    throw Gambit::ZeroDivideException();
  }

  unsigned short ys[SHORT_PER_LONG];
  unsigned long u;
  int ysgn = y >= 0;
  if (ysgn)
    u = y;
  else
    u = -y;
  int yl = 0;
  while (u != 0)
  {
    ys[yl++] = extract(u);
    u = down(u);
  }

  int comp = xl - yl;
  if (comp == 0) comp = docmp(x->s, ys, xl);

  int xsgn = x->sgn;
  int samesign = xsgn == ysgn;

  if (comp < 0)
    q = Icopy_zero(q);
  else if (comp == 0)
  {
    q = Icopy_one(q, samesign);
  }
  else if (yl == 1)
  {
    q = Icopy(q, x);
    unscale(q->s, q->len, ys[0], q->s);
  }
  else
  {
    IntegerRep* r  = 0;
	 unsigned short prescale = (unsigned short) (I_RADIX / (1 + ys[yl - 1]));
    if (prescale != 1)
    {
      unsigned long prod = (unsigned long)prescale * (unsigned long)ys[0];
      ys[0] = extract(prod);
      prod = down(prod) + (unsigned long)prescale * (unsigned long)ys[1];
      ys[1] = extract(prod);
      r = multiply(x, ((long)prescale & I_MAXNUM), r);
    }
    else
    {
      r = Icalloc(r, xl + 1);
      scpy(x->s, r->s, xl);
    }

    int ql = xl - yl + 1;
      
    q = Icalloc(q, ql);
    do_divide(r->s, ys, yl, q->s, ql);

    if (!STATIC_IntegerRep(r)) delete r;
  }
  q->sgn = samesign;
  Icheck(q);
  return q;
}


void divide(const Integer& Ix, long y, Integer& Iq, long& rem)
{
  const IntegerRep* x = Ix.rep;
  nonnil(x);
  IntegerRep* q = Iq.rep;
  int xl = x->len;
  if (y == 0) {
    throw Gambit::ZeroDivideException();
  }
  unsigned short ys[SHORT_PER_LONG];
  unsigned long u;
  int ysgn = y >= 0;
  if (ysgn)
    u = y;
  else
    u = -y;
  int yl = 0;
  while (u != 0)
  {
    ys[yl++] = extract(u);
    u = down(u);
  }

  int comp = xl - yl;
  if (comp == 0) comp = docmp(x->s, ys, xl);

  int xsgn = x->sgn;
  int samesign = xsgn == ysgn;

  if (comp < 0)
  {
    rem = Itolong(x);
    q = Icopy_zero(q);
  }
  else if (comp == 0)
  {
    q = Icopy_one(q, samesign);
    rem = 0;
  }
  else if (yl == 1)
  {
    q = Icopy(q, x);
    rem = unscale(q->s, q->len, ys[0], q->s);
  }
  else
  {
    IntegerRep* r  = 0;
	 unsigned short prescale = (unsigned short) (I_RADIX / (1 + ys[yl - 1]));
    if (prescale != 1)
    {
      unsigned long prod = (unsigned long)prescale * (unsigned long)ys[0];
      ys[0] = extract(prod);
      prod = down(prod) + (unsigned long)prescale * (unsigned long)ys[1];
      ys[1] = extract(prod);
      r = multiply(x, ((long)prescale & I_MAXNUM), r);
    }
    else
    {
      r = Icalloc(r, xl + 1);
      scpy(x->s, r->s, xl);
    }

    int ql = xl - yl + 1;
      
    q = Icalloc(q, ql);
    
    do_divide(r->s, ys, yl, q->s, ql);

    if (prescale != 1)
    {
      Icheck(r);
      unscale(r->s, r->len, prescale, r->s);
    }
    Icheck(r);
    rem = Itolong(r);
    if (!STATIC_IntegerRep(r)) delete r;
  }
  rem = abs(rem).as_long();
  if (xsgn == I_NEGATIVE) rem = -rem;
  q->sgn = samesign;
  Icheck(q);
  Iq.rep = q;
}


void divide(const Integer& Ix, const Integer& Iy, Integer& Iq, Integer& Ir)
{
  const IntegerRep* x = Ix.rep;
  nonnil(x);
  const IntegerRep* y = Iy.rep;
  nonnil(y);
  IntegerRep* q = Iq.rep;
  IntegerRep* r = Ir.rep;

  int xl = x->len;
  int yl = y->len;
  if (yl == 0) {
    throw Gambit::ZeroDivideException();
  }
  int comp = ucompare(x, y);
  int xsgn = x->sgn;
  int ysgn = y->sgn;

  int samesign = xsgn == ysgn;

  if (comp < 0)
  {
    q = Icopy_zero(q);
    r = Icopy(r, x);
  }
  else if (comp == 0)
  {
    q = Icopy_one(q, samesign);
    r = Icopy_zero(r);
  }
  else if (yl == 1)
  {
    q = Icopy(q, x);
    int rem =  unscale(q->s, q->len, y->s[0], q->s);
    r = Icopy_long(r, rem);
    if (rem != 0)
      r->sgn = xsgn;
  }
  else
  {
    IntegerRep* yy = 0;
	 unsigned short prescale = (unsigned short) (I_RADIX / (1 + y->s[yl - 1]));
    if (prescale != 1 || y == q || y == r)
    {
      yy = multiply(y, ((long)prescale & I_MAXNUM), yy);
      r = multiply(x, ((long)prescale & I_MAXNUM), r);
    }
    else
    {
      yy = (IntegerRep*)y;
      r = Icalloc(r, xl + 1);
      scpy(x->s, r->s, xl);
    }

    int ql = xl - yl + 1;
      
    q = Icalloc(q, ql);
    do_divide(r->s, yy->s, yl, q->s, ql);

    if (yy != y && !STATIC_IntegerRep(yy)) delete yy;
    if (prescale != 1)
    {
      Icheck(r);
      unscale(r->s, r->len, prescale, r->s);
    }
  }
  q->sgn = samesign;
  Icheck(q);
  Iq.rep = q;
  Icheck(r);
  Ir.rep = r;
}

IntegerRep* mod(const IntegerRep* x, const IntegerRep* y, IntegerRep* r)
{
  nonnil(x);
  nonnil(y);
  int xl = x->len;
  int yl = y->len;
  // if (yl == 0) (*lib_error_handler)("Integer", "attempted division by zero");
  if (yl == 0) {
    throw Gambit::ZeroDivideException();
  }

  int comp = ucompare(x, y);
  int xsgn = x->sgn;

  if (comp < 0)
    r = Icopy(r, x);
  else if (comp == 0)
    r = Icopy_zero(r);
  else if (yl == 1)
  {
    int rem = unscale(x->s, xl, y->s[0], 0);
    r = Icopy_long(r, rem);
    if (rem != 0)
      r->sgn = xsgn;
  }
  else
  {
    IntegerRep* yy = 0;
	 unsigned short prescale = (unsigned short) (I_RADIX / (1 + y->s[yl - 1]));
    if (prescale != 1 || y == r)
    {
      yy = multiply(y, ((long)prescale & I_MAXNUM), yy);
      r = multiply(x, ((long)prescale & I_MAXNUM), r);
    }
    else
    {
      yy = (IntegerRep*)y;
      r = Icalloc(r, xl + 1);
      scpy(x->s, r->s, xl);
    }
      
    do_divide(r->s, yy->s, yl, 0, xl - yl + 1);

    if (yy != y && !STATIC_IntegerRep(yy)) delete yy;

    if (prescale != 1)
    {
      Icheck(r);
      unscale(r->s, r->len, prescale, r->s);
    }
  }
  Icheck(r);
  return r;
}

IntegerRep* mod(const IntegerRep* x, long y, IntegerRep* r)
{
  nonnil(x);
  int xl = x->len;
  if (y == 0) {
    throw Gambit::ZeroDivideException();
  }
  unsigned short ys[SHORT_PER_LONG];
  unsigned long u;
  int ysgn = y >= 0;
  if (ysgn)
    u = y;
  else
    u = -y;
  int yl = 0;
  while (u != 0)
  {
    ys[yl++] = extract(u);
    u = down(u);
  }

  int comp = xl - yl;
  if (comp == 0) comp = docmp(x->s, ys, xl);

  int xsgn = x->sgn;

  if (comp < 0)
    r = Icopy(r, x);
  else if (comp == 0)
    r = Icopy_zero(r);
  else if (yl == 1)
  {
    int rem = unscale(x->s, xl, ys[0], 0);
    r = Icopy_long(r, rem);
    if (rem != 0)
      r->sgn = xsgn;
  }
  else
  {
	 unsigned short prescale = (unsigned short) (I_RADIX / (1 + ys[yl - 1]));
    if (prescale != 1)
    {
      unsigned long prod = (unsigned long)prescale * (unsigned long)ys[0];
      ys[0] = extract(prod);
      prod = down(prod) + (unsigned long)prescale * (unsigned long)ys[1];
      ys[1] = extract(prod);
      r = multiply(x, ((long)prescale & I_MAXNUM), r);
    }
    else
    {
      r = Icalloc(r, xl + 1);
      scpy(x->s, r->s, xl);
    }
      
    do_divide(r->s, ys, yl, 0, xl - yl + 1);

    if (prescale != 1)
    {
      Icheck(r);
      unscale(r->s, r->len, prescale, r->s);
    }
  }
  Icheck(r);
  return r;
}

IntegerRep* lshift(const IntegerRep* x, long y, IntegerRep* r)
{
  nonnil(x);
  int xl = x->len;
  if (xl == 0 || y == 0)
  {
    r = Icopy(r, x);
    return r;
  }

  int xrsame = x == r;
  int rsgn = x->sgn;

  long ay = (y < 0)? -y : y;
  int bw = (int) (ay / I_SHIFT);
  int sw = (int) (ay % I_SHIFT);

  if (y > 0)
  {
    int rl = bw + xl + 1;
    if (xrsame)
      r = Iresize(r, rl);
    else
      r = Icalloc(r, rl);

    unsigned short* botr = r->s;
    unsigned short* rs = &(botr[rl - 1]);
    const unsigned short* botx = (xrsame)? botr : x->s;
    const unsigned short* xs = &(botx[xl - 1]);
    unsigned long a = 0;
    while (xs >= botx)
    {
      a = up(a) | ((unsigned long)(*xs--) << sw);
      *rs-- = extract(down(a));
    }
    *rs-- = extract(a);
    while (rs >= botr)
      *rs-- = 0;
  }
  else
  {
    int rl = xl - bw;
    if (rl < 0)
      r = Icopy_zero(r);
    else
    {
      if (xrsame)
        r = Iresize(r, rl);
      else
        r = Icalloc(r, rl);
      int rw = I_SHIFT - sw;
      unsigned short* rs = r->s;
      unsigned short* topr = &(rs[rl]);
      const unsigned short* botx = (xrsame)? rs : x->s;
      const unsigned short* xs =  &(botx[bw]);
      const unsigned short* topx = &(botx[xl]);
      unsigned long a = (unsigned long)(*xs++) >> sw;
      while (xs < topx)
      {
        a |= (unsigned long)(*xs++) << rw;
        *rs++ = extract(a);
        a = down(a);
      }
      *rs++ = extract(a);
      if (xrsame) topr = (unsigned short*)topx;
      while (rs < topr)
        *rs++ = 0;
    }
  }
  r->sgn = rsgn;
  Icheck(r);
  return r;
}

IntegerRep* lshift(const IntegerRep* x, const IntegerRep* yy, int negatey, IntegerRep* r)
{
  long y = Itolong(yy);
  if (negatey)
    y = -y;

  return lshift(x, y, r);
}

IntegerRep* bitop(const IntegerRep* x, const IntegerRep* y, IntegerRep* r, char op)
{
  nonnil(x);
  nonnil(y);
  int xl = x->len;
  int yl = y->len;
  int xsgn = x->sgn;
  int xrsame = x == r;
  int yrsame = y == r;
  if (xrsame || yrsame)
    r = Iresize(r, calc_len(xl, yl, 0));
  else
    r = Icalloc(r, calc_len(xl, yl, 0));
  r->sgn = xsgn;
  unsigned short* rs = r->s;
  unsigned short* topr = &(rs[r->len]);
  const unsigned short* as;
  const unsigned short* bs;
  const unsigned short* topb;
  if (xl >= yl)
  {
    as = (xrsame)? rs : x->s;
    bs = (yrsame)? rs : y->s;
    topb = &(bs[yl]);
  }
  else
  {
    bs = (xrsame)? rs : x->s;
    topb = &(bs[xl]);
    as = (yrsame)? rs : y->s;
  }

  switch (op)
  {
  case '&':
    while (bs < topb) *rs++ = *as++ & *bs++;
    while (rs < topr) *rs++ = 0;
    break;
  case '|':
    while (bs < topb) *rs++ = *as++ | *bs++;
    while (rs < topr) *rs++ = *as++;
    break;
  case '^':
    while (bs < topb) *rs++ = *as++ ^ *bs++;
    while (rs < topr) *rs++ = *as++;
    break;
  }
  Icheck(r);
  return r;
}

IntegerRep* bitop(const IntegerRep* x, long y, IntegerRep* r, char op)
{
  nonnil(x);
  unsigned short tmp[SHORT_PER_LONG];
  unsigned long u;
  int newsgn = (y >= 0);
  if (newsgn)
	 u = y;
  else
	 u = -y;

  int l = 0;
  while (u != 0)
  {
	 tmp[l++] = extract(u);
	 u = down(u);
  }

  int xl = x->len;
  int yl = l;
  int xsgn = x->sgn;
  int xrsame = x == r;
  if (xrsame)
	 r = Iresize(r, calc_len(xl, yl, 0));
  else
	 r = Icalloc(r, calc_len(xl, yl, 0));
  r->sgn = xsgn;
  unsigned short* rs = r->s;
  unsigned short* topr = &(rs[r->len]);
  const unsigned short* as;
  const unsigned short* bs;
  const unsigned short* topb;
  if (xl >= yl)
  {
	 as = (xrsame)? rs : x->s;
	 bs = tmp;
	 topb = &(bs[yl]);
  }
  else
  {
	 bs = (xrsame)? rs : x->s;
	 topb = &(bs[xl]);
	 as = tmp;
  }

  switch (op)
  {
  case '&':
	 while (bs < topb) *rs++ = *as++ & *bs++;
	 while (rs < topr) *rs++ = 0;
	 break;
  case '|':
	 while (bs < topb) *rs++ = *as++ | *bs++;
	 while (rs < topr) *rs++ = *as++;
	 break;
  case '^':
	 while (bs < topb) *rs++ = *as++ ^ *bs++;
	 while (rs < topr) *rs++ = *as++;
	 break;
  }
  Icheck(r);
  return r;
}



IntegerRep*  Compl(const IntegerRep* src, IntegerRep* r)
{
  nonnil(src);
  r = Icopy(r, src);
  unsigned short* s = r->s;
  unsigned short* top = &(s[r->len - 1]);
  while (s < top)
  {
    unsigned short cmp = ~(*s);
    *s++ = cmp;
  }
  unsigned short a = *s;
  unsigned short b = 0;
  while (a != 0)
  {
    b <<= 1;
    if (!(a & 1)) b |= 1;
    a >>= 1;
  }
  *s = b;
  Icheck(r);
  return r;
}

void (setbit)(Integer& x, long b)
{
  if (b >= 0)
  {
	 int bw = (int) ((unsigned long)b / I_SHIFT);
	 int sw = (int) ((unsigned long)b % I_SHIFT);
    int xl = x.rep ? x.rep->len : 0;
    if (xl <= bw)
      x.rep = Iresize(x.rep, calc_len(xl, bw+1, 0));
    x.rep->s[bw] |= (1 << sw);
    Icheck(x.rep);
  }
}

void clearbit(Integer& x, long b)
{
  if (b >= 0)
    {
      if (x.rep == 0)
	x.rep = &_ZeroRep;
      else
	{
	  int bw = (int) ((unsigned long)b / I_SHIFT);
	  int sw = (int) ((unsigned long)b % I_SHIFT);
	  if (x.rep->len > bw)
	    x.rep->s[bw] &= ~(1 << sw);
	}
    Icheck(x.rep);
  }
}

int testbit(const Integer& x, long b)
{
  if (x.rep != 0 && b >= 0)
  {
	 int bw = (int) ((unsigned long)b / I_SHIFT);
	 int sw = (int) ((unsigned long)b % I_SHIFT);
    return (bw < x.rep->len && (x.rep->s[bw] & (1 << sw)) != 0);
  }
  else
    return 0;
}

// A  version of knuth's algorithm B / ex. 4.5.3.34
// A better version that doesn't bother shifting all of `t' forthcoming

IntegerRep* gcd(const IntegerRep* x, const IntegerRep* y)
{
  nonnil(x);
  nonnil(y);
  int ul = x->len;
  int vl = y->len;
  
  if (vl == 0)
    return Ialloc(0, x->s, ul, I_POSITIVE, ul);
  else if (ul == 0)
    return Ialloc(0, y->s, vl, I_POSITIVE, vl);

  IntegerRep* u = Ialloc(0, x->s, ul, I_POSITIVE, ul);
  IntegerRep* v = Ialloc(0, y->s, vl, I_POSITIVE, vl);

// find shift so that both not even

  long k = 0;
  int l = (ul <= vl)? ul : vl;
  int cont = 1, i;
  for (i = 0;  i < l && cont; ++i)
  {
    unsigned long a =  (i < ul)? u->s[i] : 0;
    unsigned long b =  (i < vl)? v->s[i] : 0;
    for (unsigned int j = 0; j < I_SHIFT; ++j)
    {
      if ((a | b) & 1)
      {
        cont = 0;
        break;
      }
      else
      {
        ++k;
        a >>= 1;
        b >>= 1;
      }
    }
  }

  if (k != 0)
  {
    u = lshift(u, -k, u);
    v = lshift(v, -k, v);
  }

  IntegerRep* t;
  if (u->s[0] & 01)
    t = Ialloc(0, v->s, v->len, !v->sgn, v->len);
  else
    t = Ialloc(0, u->s, u->len, u->sgn, u->len);

  while (t->len != 0)
  {
    long s = 0;                 // shift t until odd
    cont = 1;
    int tl = t->len;
    for (i = 0; i < tl && cont; ++i)
    {
      unsigned long a = t->s[i];
      for (unsigned int j = 0; j < I_SHIFT; ++j)
      {
        if (a & 1)
        {
          cont = 0;
          break;
        }
        else
        {
          ++s;
          a >>= 1;
        }
      }
    }

    if (s != 0) t = lshift(t, -s, t);

    if (t->sgn == I_POSITIVE)
    {
      u = Icopy(u, t);
      t = add(t, 0, v, 1, t);
    }
    else
    {
      v = Ialloc(v, t->s, t->len, !t->sgn, t->len);
      t = add(t, 0, u, 0, t);
    }
  }
  if (!STATIC_IntegerRep(t)) delete t;
  if (!STATIC_IntegerRep(v)) delete v;
  if (k != 0) u = lshift(u, k, u);
  return u;
}



long lg(const IntegerRep* x)
{
  nonnil(x);
  int xl = x->len;
  if (xl == 0)
    return 0;

  long l = (xl - 1) * I_SHIFT - 1;
  unsigned short a = x->s[xl-1];

  while (a != 0)
  {
    a = a >> 1;
    ++l;
  }
  return l;
}
  
IntegerRep* power(const IntegerRep* x, long y, IntegerRep* r)
{
  nonnil(x);
  int sgn;
  if (x->sgn == I_POSITIVE || (!(y & 1)))
    sgn = I_POSITIVE;
  else
    sgn = I_NEGATIVE;

  int xl = x->len;

  if (y == 0 || (xl == 1 && x->s[0] == 1))
    r = Icopy_one(r, sgn);
  else if (xl == 0 || y < 0)
    r = Icopy_zero(r);
  else if (y == 1 || y == -1)
    r = Icopy(r, x);
  else
  {
	 int maxsize = (int) (((lg(x) + 1) * y) / I_SHIFT + 2);     // pre-allocate space
    IntegerRep* b = Ialloc(0, x->s, xl, I_POSITIVE, maxsize);
    b->len = xl;
    r = Icalloc(r, maxsize);
    r = Icopy_one(r, I_POSITIVE);
    for(;;)
    {
      if (y & 1)
        r = multiply(r, b, r);
      if ((y >>= 1) == 0)
        break;
      else
        b = multiply(b, b, b);
    }
    if (!STATIC_IntegerRep(b)) delete b;
  }
  r->sgn = sgn;
  Icheck(r);
  return r;
}

IntegerRep* abs(const IntegerRep* src, IntegerRep* dest)
{
  nonnil(src);
  if (src != dest)
    dest = Icopy(dest, src);
  dest->sgn = I_POSITIVE;
  return dest;
}

IntegerRep* negate(const IntegerRep* src, IntegerRep* dest)
{
  nonnil(src);
  if (src != dest)
    dest = Icopy(dest, src);
  if (dest->len != 0) 
    dest->sgn = !dest->sgn;
  return dest;
}

#if defined(__GNUG__) && !defined(NO_NRV)

Integer sqrt(const Integer& x)
{
  Integer r;
  int s = sign(x);
  if (s < 0) x.error("Attempted square root of negative Integer");
  if (s != 0)
  {
    r >>= (lg(x) / 2); // get close
    Integer q;
    div(x, r, q);
    while (q < r)
    {
      r += q;
      r >>= 1;
      div(x, r, q);
    }
  }
  return r;
}

Integer lcm(const Integer& x, const Integer& y)
{
  Integer r;
  if (!x.initialized() || !y.initialized())
    x.error("operation on uninitialized Integer");
  Integer g;
  if (sign(x) == 0 || sign(y) == 0)
    g = 1;
  else 
    g = gcd(x, y);
  div(x, g, r);
  mul(r, y, r);
  return r;
}

#else 
Integer sqrt(const Integer& x) 
{
  Integer r(x);
  int s = sign(x);
  if (s < 0) x.error("Attempted square root of negative Integer");
  if (s != 0)
  {
    r >>= (lg(x) / 2); // get close
    Integer q;
    div(x, r, q);
    while (q < r)
    {
      r += q;
      r >>= 1;
      div(x, r, q);
    }
  }
  return r;
}

Integer lcm(const Integer& x, const Integer& y) 
{
  Integer r;
  if (!x.initialized() || !y.initialized())
    x.error("operation on uninitialized Integer");
  Integer g;
  if (sign(x) == 0 || sign(y) == 0)
    g = 1;
  else 
    g = gcd(x, y);
  div(x, g, r);
  mul(r, y, r);
  return r;
}

#endif



IntegerRep* atoIntegerRep(const char* s, int base)
{
  int sl = strlen(s);
  IntegerRep* r = Icalloc(0, (int) (sl * (lg(base) + 1) / I_SHIFT + 1));
  if (s != 0)
  {
    char sgn;
    while (isspace(*s)) ++s;
    if (*s == '-')
    {
      sgn = I_NEGATIVE;
      s++;
    }
    else if (*s == '+')
    {
      sgn = I_POSITIVE;
      s++;
    }
    else
      sgn = I_POSITIVE;
    for (;;)
    {
      long digit;
      if (*s >= '0' && *s <= '9') digit = *s - '0';
      else if (*s >= 'a' && *s <= 'z') digit = *s - 'a' + 10;
      else if (*s >= 'A' && *s <= 'Z') digit = *s - 'A' + 10;
      else break;
      if (digit >= base) break;
      r = multiply(r, base, r);
      r = add(r, 0, digit, r);
      ++s;
    }
    r->sgn = sgn;
  }
  return r;
}


std::string Itoa(const IntegerRep *x, int base, int width)
{
  int fmtlen = (int) ((x->len + 1) * I_SHIFT / lg(base) + 4 + width);
  std::string fmtbase;
  for (int i = 0; i < fmtlen; i++) {
    fmtbase += " ";
  }
  return cvtItoa(x, fmtbase, fmtlen, base, 0, width, 0, ' ', 'X', 0);
}

std::ostream &operator<<(std::ostream &s, const Integer &y)
{
  return s << Itoa(y.rep);
}

std::string cvtItoa(const IntegerRep *x, std::string fmt, int& fmtlen, int base, int showbase,
              int width, int align_right, char fillchar, char Xcase, 
              int showpos)
{
  char* e = const_cast<char *>(fmt.c_str()) + fmtlen - 1;
  char* s = e;
  *--s = 0;

  if (x->len == 0)
    *--s = '0';
  else
  {
    IntegerRep* z = Icopy(0, x);

    // split division by base into two parts: 
    // first divide by biggest power of base that fits in an unsigned short,
    // then use straight signed div/mods from there. 

    // find power
    int bpower = 1;
    unsigned short b = base;
	 unsigned short maxb = (unsigned short) (I_MAXNUM / base);
    while (b < maxb)
    {
      b *= base;
      ++bpower;
    }
    for(;;)
    {
      int rem = unscale(z->s, z->len, b, z->s);
      Icheck(z);
      if (z->len == 0)
      {
        while (rem != 0)
        {
          char ch = rem % base;
          rem /= base;
          if (ch >= 10)
            ch += 'a' - 10;
          else
            ch += '0';
          *--s = ch;
        }
	if (!STATIC_IntegerRep(z)) delete z;
        break;
      }
      else
      {
        for (int i = 0; i < bpower; ++i)
        {
          char ch = rem % base;
          rem /= base;
          if (ch >= 10)
            ch += 'a' - 10;
          else
            ch += '0';
          *--s = ch;
        }
      }
    }
  }

  if (base == 8 && showbase) 
    *--s = '0';
  else if (base == 16 && showbase) 
  { 
    *--s = Xcase; 
    *--s = '0'; 
  }
  if (x->sgn == I_NEGATIVE) *--s = '-';
  else if (showpos) *--s = '+';
  int w = (int) (e - s - 1);
  if (!align_right || w >= width)
  {
    while (w++ < width)
      *--s = fillchar;
	 fmtlen = (int) (e - s - 1);
    return s;
  }
  else {
    char *p = const_cast<char *>(fmt.c_str());
#ifdef UNUSED
    int gap = (int) (s - p);
#endif   // UNUSED
    for (char* t = s; *t != 0; ++t, ++p) *p = *t;
    while (w++ < width) *p++ = fillchar;
    *p = 0;
    fmtlen = (int) (p - const_cast<char *>(fmt.c_str()));
    return fmt;
  }
}

std::istream &operator>>(std::istream &s, Integer& y)
{
  char sgn = 0;
  char ch;
  y.rep = Icopy_zero(y.rep);

  do  {
	 s.get(ch);
  }  while (isspace(ch));

  s.unget();

  while (s.get(ch))
  {
	 if (ch == '-')
	 {
		if (sgn == 0)
		  sgn = '-';
		else
		  break;
	 }
	 else
	 {
		if (ch >= '0' && ch <= '9')
		{
		  long digit = ch - '0';
		  y *= 10;
		  y += digit;
		}
		else
		  break;
	 }
  }
  s.unget();

  if (sgn == '-')
	 y.negate();

  return s;
}

int Integer::OK() const
{
  if (rep != 0)
	 {
      int l = rep->len;
      int s = rep->sgn;
      int v = l <= rep->sz || STATIC_IntegerRep(rep);    // length within bounds
      v &= s == 0 || s == 1;        // legal sign
      Icheck(rep);                  // and correctly adjusted
      v &= rep->len == l;
      v &= rep->sgn == s;
      if (v)
	  return v;
    }
  error("invariant failure");
  return 0;
}

void Integer::error(const char* msg) const
{
  // (*lib_error_handler)("Integer", msg);
  //  gerr << msg << '\n';
}



// The following were moved from the header file to stop BC from squealing
// endless quantities of warnings

Integer::Integer() :rep(&_ZeroRep) {}

Integer::Integer(IntegerRep* r) :rep(r) {}

Integer::Integer(int y) :rep(Icopy_long(0, (long)y)) {}

Integer::Integer(long y) :rep(Icopy_long(0, y)) {}

Integer::Integer(unsigned long y) :rep(Icopy_ulong(0, y)) {}

Integer::Integer(const Integer&  y) :rep(Icopy(0, y.rep)) {}

Integer::~Integer() { if (rep && !STATIC_IntegerRep(rep)) delete rep; }

Integer &Integer::operator=(const Integer &y)
{
  rep = Icopy(rep, y.rep);
  return *this;
}

Integer &Integer::operator=(long y)
{
  rep = Icopy_long(rep, y); 
  return *this;
}

int Integer::initialized() const
{
  return rep != 0;
}

// procedural versions

int compare(const Integer& x, const Integer& y)
{
  return compare(x.rep, y.rep);
}

int ucompare(const Integer& x, const Integer& y)
{
  return ucompare(x.rep, y.rep);
}

int compare(const Integer& x, long y)
{
  return compare(x.rep, y);
}

int ucompare(const Integer& x, long y)
{
  return ucompare(x.rep, y);
}

int compare(long x, const Integer& y)
{
  return -compare(y.rep, x);
}

int ucompare(long x, const Integer& y)
{
  return -ucompare(y.rep, x);
}

void  add(const Integer& x, const Integer& y, Integer& dest)
{
  dest.rep = add(x.rep, 0, y.rep, 0, dest.rep);
}

void  sub(const Integer& x, const Integer& y, Integer& dest)
{
  dest.rep = add(x.rep, 0, y.rep, 1, dest.rep);
}

void  mul(const Integer& x, const Integer& y, Integer& dest)
{
  dest.rep = multiply(x.rep, y.rep, dest.rep);
}

void  div(const Integer& x, const Integer& y, Integer& dest)
{
  dest.rep = div(x.rep, y.rep, dest.rep);
}

void  mod(const Integer& x, const Integer& y, Integer& dest)
{
  dest.rep = mod(x.rep, y.rep, dest.rep);
}

void  lshift(const Integer& x, const Integer& y, Integer& dest)
{
  dest.rep = lshift(x.rep, y.rep, 0, dest.rep);
}

void  rshift(const Integer& x, const Integer& y, Integer& dest)
{
  dest.rep = lshift(x.rep, y.rep, 1, dest.rep);
}

void  pow(const Integer& x, const Integer& y, Integer& dest)
{
  dest.rep = power(x.rep, Itolong(y.rep), dest.rep); // not incorrect
}

void  add(const Integer& x, long y, Integer& dest)
{
  dest.rep = add(x.rep, 0, y, dest.rep);
}

void  sub(const Integer& x, long y, Integer& dest)
{
  dest.rep = add(x.rep, 0, -y, dest.rep);
}

void  mul(const Integer& x, long y, Integer& dest)
{
  dest.rep = multiply(x.rep, y, dest.rep);
}

void  div(const Integer& x, long y, Integer& dest)
{
  dest.rep = div(x.rep, y, dest.rep);
}

void  mod(const Integer& x, long y, Integer& dest)
{
  dest.rep = mod(x.rep, y, dest.rep);
}


void  lshift(const Integer& x, long y, Integer& dest)
{
  dest.rep = lshift(x.rep, y, dest.rep);
}

void  rshift(const Integer& x, long y, Integer& dest)
{
  dest.rep = lshift(x.rep, -y, dest.rep);
}

void  pow(const Integer& x, long y, Integer& dest)
{
  dest.rep = power(x.rep, y, dest.rep);
}

void abs(const Integer& x, Integer& dest)
{
  dest.rep = abs(x.rep, dest.rep);
}

void negate(const Integer& x, Integer& dest)
{
  dest.rep = negate(x.rep, dest.rep);
}

void complement(const Integer& x, Integer& dest)
{
  dest.rep = Compl(x.rep, dest.rep);
}

void  add(long x, const Integer& y, Integer& dest)
{
  dest.rep = add(y.rep, 0, x, dest.rep);
}

void  sub(long x, const Integer& y, Integer& dest)
{
  dest.rep = add(y.rep, 1, x, dest.rep);
}

void  mul(long x, const Integer& y, Integer& dest)
{
  dest.rep = multiply(y.rep, x, dest.rep);
}

// operator versions

bool Integer::operator==(const Integer &y) const
{
  return compare(*this, y) == 0; 
}

bool Integer::operator==(long y) const
{
  return compare(*this, y) == 0; 
}

bool Integer::operator!=(const Integer &y) const
{
  return compare(*this, y) != 0; 
}

bool Integer::operator!=(long y) const
{
  return compare(*this, y) != 0; 
}

bool Integer::operator<(const Integer &y) const
{
  return compare(*this, y) <  0; 
}

bool Integer::operator<(long y) const
{
  return compare(*this, y) <  0; 
}

bool Integer::operator<=(const Integer &y) const
{
  return compare(*this, y) <= 0; 
}

bool Integer::operator<=(long y) const
{
  return compare(*this, y) <= 0; 
}

bool Integer::operator>(const Integer &y) const
{
  return compare(*this, y) >  0; 
}

bool Integer::operator>(long y) const
{
  return compare(*this, y) >  0; 
}

bool Integer::operator>=(const Integer &y) const
{
  return compare(*this, y) >= 0; 
}

bool Integer::operator>=(long y) const
{
  return compare(*this, y) >= 0; 
}


Integer &Integer::operator+=(const Integer &y)
{
  add(*this, y, *this);
  return *this;
}

Integer &Integer::operator+=(long y)
{
  add(*this, y, *this);
  return *this;
}

void Integer::operator ++ ()
{
  add(*this, 1, *this);
}


Integer &Integer::operator-=(const Integer &y)
{
  sub(*this, y, *this);
  return *this;
}

Integer &Integer::operator-=(long y)
{
  sub(*this, y, *this);
  return *this;
}

void Integer::operator -- ()
{
  add(*this, -1, *this);
}



Integer &Integer::operator*=(const Integer &y)
{
  mul(*this, y, *this);
  return *this;
}

Integer &Integer::operator*=(long y)
{
  mul(*this, y, *this);
  return *this;
}


Integer &Integer::operator/=(const Integer &y)
{
  div(*this, y, *this);
  return *this;
}

Integer &Integer::operator/=(long y)
{
  div(*this, y, *this);
  return *this;
}

Integer &Integer::operator<<=(const Integer &y)
{
  lshift(*this, y, *this);
  return *this;
}

Integer &Integer::operator<<=(long y)
{
  lshift(*this, y, *this);
  return *this;
}

Integer &Integer::operator>>=(const Integer &y)
{
  rshift(*this, y, *this);
  return *this;
}

Integer &Integer::operator>>=(long y)
{
  rshift(*this, y, *this);
  return *this;
}


void Integer::abs()
{
  Gambit::abs(*this, *this);
}

void Integer::negate()
{
  Gambit::negate(*this, *this);
}


int sign(const Integer& x)
{
  return (x.rep->len == 0) ? 0 : ( (x.rep->sgn == 1) ? 1 : -1 );
}

int even(const Integer& y)
{
  return y.rep->len == 0 || !(y.rep->s[0] & 1);
}

int odd(const Integer& y)
{
  return y.rep->len > 0 && (y.rep->s[0] & 1);
}

std::string Itoa(const Integer& y, int base, int width)
{
  return Itoa(y.rep, base, width);
}



long lg(const Integer& x) 
{
  return lg(x.rep);
}

// constructive operations 

Integer Integer::operator+(const Integer &y) const
{
  Integer r;
  add(*this, y, r);
  return r;
}

Integer Integer::operator+(long y) const
{
  Integer r;
  add(*this, y, r);
  return r;
}

Integer Integer::operator-(const Integer &y) const
{
  Integer r;
  sub(*this, y, r);
  return r;
}

Integer Integer::operator-(long y) const
{
  Integer r;
  sub(*this, y, r);
  return r;
}

Integer Integer::operator*(const Integer &y) const
{
  Integer r;
  mul(*this, y, r);
  return r;
}

Integer Integer::operator*(long y) const
{
  Integer r;
  mul(*this, y, r);
  return r;
}

Integer sqr(const Integer& x) 
{
  Integer r;
  mul(x, x, r);
  return r;
}

Integer Integer::operator/(const Integer &y) const
{
  Integer r;
  div(*this, y, r);
  return r;
}

Integer Integer::operator/(long y) const
{
  Integer r;
  div(*this, y, r);
  return r;
}

Integer Integer::operator%(const Integer &y) const 
{
  Integer r;
  mod(*this, y, r);
  return r;
}

Integer Integer::operator%(long y) const
{
  Integer r;
  mod(*this, y, r);
  return r;
}

Integer Integer::operator<<(const Integer &y) const
{
  Integer r;
  lshift(*this, y, r);
  return r;
}

Integer Integer::operator<<(long y) const
{
  Integer r;
  lshift(*this, y, r);
  return r;
}

Integer Integer::operator>>(const Integer &y) const 
{
  Integer r;
  rshift(*this, y, r);
  return r;
}

Integer Integer::operator>>(long y) const
{
  Integer r;
  rshift(*this, y, r);
  return r;
}

Integer pow(const Integer& x, long y)
{
  Integer r;
  pow(x, y, r);
  return r;
}

Integer Ipow(long x, long y)
{
  Integer r(x);
  pow(r, y, r);
  return r;
}

Integer pow(const Integer& x, const Integer& y) 
{
  Integer r;
  pow(x, y, r);
  return r;
}



Integer abs(const Integer& x) 
{
  Integer r;
  abs(x, r);
  return r;
}

Integer Integer::operator-(void) const
{
  Integer r;
  Gambit::negate(*this, r);
  return r;
}


Integer  atoI(const char* s, int base) 
{
  Integer r;
  r.rep = atoIntegerRep(s, base);
  return r;
}

Integer  gcd(const Integer& x, const Integer& y)
{
  Integer r;
  r.rep = gcd(x.rep, y.rep);
  return r;
}







Integer &Integer::operator%=(const Integer &y)
{
  *this = *this % y; // mod(*this, y, *this) doesn't work.
  return *this;
}

Integer &Integer::operator%=(long y)
{
  *this = *this % y; // mod(*this, y, *this) doesn't work.
  return *this;
}

}
