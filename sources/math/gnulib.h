//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Utilities from the g++ library, for gInteger and gRational
//
// This file is part of Gambit
// Modifications copyright (c) 2002, The Gambit Project
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

#ifndef GNULIB_H
#define GNULIB_H

#ifdef __GNUG__
#include <_G_config.h>
#endif   // __GNUG__
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#ifdef __GNUG__
#include <unistd.h>
#endif  // __GNUG__
#include <stdio.h> 
#include <errno.h>
#include <fcntl.h>

#ifdef __GNUG__
extern "C" {
int strcasecmp _G_ARGS((const char*, const char*));
}
#endif   // __GNUG__


#include <math.h>

#ifdef __GNUG__
#define _VOLATILE_VOID volatile void
#else
#define _VOLATILE_VOID void
#endif

typedef void (*one_arg_error_handler_t)(const char*);
typedef void (*two_arg_error_handler_t)(const char*, const char*);

long         gcd(long, long);
long         lg(unsigned long); 
double       pow(double, long);
long         pow(long, long);

double       start_timer();
double       return_elapsed_time(double last_time = 0.0);

char*        dtoa(double x, char cvt = 'g', int width = 0, int prec = 6);

unsigned int hashpjw(const char*);
unsigned int multiplicativehash(int);
unsigned int foldhash(double);

extern _VOLATILE_VOID default_one_arg_error_handler(const char*);
extern _VOLATILE_VOID default_two_arg_error_handler(const char*, const char*);

//extern two_arg_error_handler_t lib_error_handler;

extern two_arg_error_handler_t 
       set_lib_error_handler(two_arg_error_handler_t f);


#if defined(IV)

float abs(float arg);
short abs(short arg);
long abs(long arg);
int sign(long arg);
int sign(double arg);
long sqr(long arg);
double sqr(double arg);
int even(long arg);
int odd(long arg);
long lcm(long x, long y);
void (setbit)(long& x, long b);
void clearbit(long& x, long b);
int testbit(long x, long b);

#endif

#if !defined(IV)

inline float abs(float arg) 
{
  return (arg < 0.0)? -arg : arg;
}

inline short abs(short arg) 
{
  return (arg < 0)? -arg : arg;
}

inline long abs(long arg) 
{
  return (arg < 0)? -arg : arg;
}

inline int sign(long arg)
{
  return (arg == 0) ? 0 : ( (arg > 0) ? 1 : -1 );
}

inline int sign(double arg)
{
  return (arg == 0.0) ? 0 : ( (arg > 0.0) ? 1 : -1 );
}

inline long sqr(long arg)
{
  return arg * arg;
}

#ifndef hpux
inline double sqr(double arg)
{
  return arg * arg;
}
#endif

inline int even(long arg)
{
  return !(arg & 1);
}

inline int odd(long arg)
{
  return (int) (arg & 1L);
}

inline long lcm(long x, long y)
{
  return x / gcd(x, y) * y;
}

inline void (setbit)(long& x, long b)
{
  x |= (long) (1 << (int) b);
}

inline void clearbit(long& x, long b)
{
  x &= ~((long) (1 << (int) b));
}

inline int testbit(long x, long b)
{
  return ((x & (1 << (int) b)) != 0);
}

#endif


/*
  An AllocRing holds the last n malloc'ed strings, reallocating/reusing 
  one only when the queue wraps around. It thus guarantees that the
  last n allocations are intact. It is useful for things like I/O
  formatting where reasonable restrictions may be made about the
  number of allowable live allocations before auto-deletion.
*/

class AllocRing
{

  struct AllocQNode
  {
    void*  ptr;
    int    sz;
  };

  AllocQNode* nodes;
  int         n;
  int         current;

  int         find(void* p);

public:

              AllocRing(int max);
             ~AllocRing();

  void*       alloc(int size);
  int         contains(void* ptr);
  void        clear();
  void        free(void* p);
};

#endif   // GNULIB_H

