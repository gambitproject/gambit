//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Miscellaneous generally-useful functions
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <ctype.h>

#include "gmisc.h"
#include "gstream.h"
#include "gtext.h"
#include "garray.h"

//--------------------------------------------------------------------------
//                      Generation of random numbers
//--------------------------------------------------------------------------

#ifdef RAND_MAX
#define GRAND_MAX   RAND_MAX
#elif defined INT_MAX
#define GRAND_MAX   INT_MAX
#endif

// Generates a random number between 0 and IM exclusive of endpoints
// Adapted from _Numerical_Recipes_for_C_

#define IA 16807
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define NTAB 32
#define NDIV (1+(IM-1)/NTAB)

long ran1(long* idum)
{
  int j;
  long k;
  static long iy = 0;
  static long iv[NTAB];

  if(*idum <= 0 || !iy) {
    if(-(*idum) < 1) *idum = 1;
    else *idum = -(*idum);
    for(j = NTAB+7; j >= 0; j--) {
      k = (*idum)/IQ;
      *idum = IA*(*idum-k*IQ)-IR*k;
      if(*idum < 0) *idum += IM;
      if(j < NTAB) iv[j] = *idum;
    }
    iy = iv[0];
  }
  k = (*idum)/IQ;
  *idum = IA*(*idum-k*IQ)-IR*k;
  if(*idum < 0) *idum += IM;
  j = iy/NDIV;
  iy = iv[j];
  iv[j] = *idum;
  return iy;
}


void SetSeed(unsigned int seed)
{
  srand(seed);
}

double Uniform(void)
{
  return ((double) rand()) / ((double) GRAND_MAX);
}

//--------------------------------------------------------------------------
//                         Text string conversions
//--------------------------------------------------------------------------

#define GCONVERT_BUFFER_LENGTH     64
static char gconvert_buffer[GCONVERT_BUFFER_LENGTH];
static int precision = 6;
static int width = 0;

void ToTextWidth(int i)
{
  width = i;
}

int ToTextWidth(void)
{
  return width;
}


void ToTextPrecision(int i)
{
  precision = i;
}

int ToTextPrecision(void)
{
  return precision;
}

gText ToText(int i)
{
  sprintf(gconvert_buffer, "%d", i);
  return gText(gconvert_buffer);
}

gText ToText(long l)
{
  // sprintf(gconvert_buffer, "%.*ld", precision, l);
  sprintf(gconvert_buffer, "%*ld", width, l);
  return gText(gconvert_buffer);
}

gText ToText(double d)
{
  // sprintf(gconvert_buffer, "%.*f", precision, d);
  sprintf(gconvert_buffer, "%*.*f", width, precision, d);
  return gText(gconvert_buffer);
}

gText ToText(double p_number, int p_precision)
{
  sprintf(gconvert_buffer, "%*.*f", width, p_precision, p_number);
  return gText(gconvert_buffer);
}

gText ToText(long double d)
{
  // sprintf(gconvert_buffer, "%.*f", precision, d);
  sprintf(gconvert_buffer, "%*.*Lf", width, precision, d);
  return gText(gconvert_buffer);
}

double ToDouble(const gText &s)
{ return strtod(s, NULL); }

gText EscapeQuotes(const gText &s)
{
  gText ret;
  
  for (unsigned int i = 0; i < s.Length(); i++)  {
    if (s[i] == '"')   ret += '\\';
    ret += s[i];
  }

  return ret;
}


//------------------------ TriState functions -----------------//

gText ToText(gTriState b)
{
  switch (b) {
  case triTRUE:
    return "Y";
  case triFALSE:
    return "N"; 
  default:
    return "DK";
  }
}

gOutput &operator<<(gOutput &f, gTriState b)
{
  return (f << ToText(b));
}

//------------------------ Type dependent epsilon -----------------//

void gEpsilon(double &v, int i)
{ v=pow(10.0,(double)-i); }


double pow(int x, long n)
{
  return pow((double)x,(double)n);
}

double pow(double x, long n)
{
  return pow((double)x,(double)n);
}

//--------------------------------------------------------------------------
//               A Simple Class for Compact Reference to Pairs
//--------------------------------------------------------------------------



//---------------------------
// Constructors / Destructors
//---------------------------

index_pair::index_pair(const int& f, const int& s)
: first(f), second(s)
{
}

index_pair::~index_pair()
{
}

bool index_pair::operator == (const index_pair& other) const
{
  if ((first == other.first) && (second == other.second))
    return true;
  else
    return false;
}

bool index_pair::operator != (const index_pair& other) const
{
  if ((first != other.first) || (second != other.second))
    return true;
  else
    return false;
}

int index_pair::operator [] (const int& index) const 
{
  if (index == 1) return first;
  else            return second;
}
 
gOutput &operator<<(gOutput &p_output, const index_pair &p_indexPair)
{
  p_output << "(" << p_indexPair.first << "," << p_indexPair.second << ")";
  return p_output;
}

//--------------------Exceptions Related Stuff---------------------

gText gNewFailed::Description(void) const
{
  return "Memory exhausted in call to new";
}

gException::~gException()
{ }
