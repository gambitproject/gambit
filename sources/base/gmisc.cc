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
#include "garray.h"

//--------------------------------------------------------------------------
//                         Text string conversions
//--------------------------------------------------------------------------

std::string ToText(int i)
{
  char buffer[256];
  sprintf(buffer, "%d", i);
  return std::string(buffer);
}

std::string ToText(long l)
{
  char buffer[256];
  sprintf(buffer, "%ld", l);
  return std::string(buffer);
}

std::string ToText(double d)
{
  char buffer[256];
  sprintf(buffer, "%f", d);
  return std::string(buffer);
}

std::string ToText(double p_number, int p_precision)
{
  char buffer[256];
  sprintf(buffer, "%.*f", p_precision, p_number);
  return std::string(buffer);
}

double ToDouble(const std::string &s)
{ return strtod(s.c_str(), NULL); }

std::string EscapeQuotes(const std::string &s)
{
  std::string ret;
  
  for (unsigned int i = 0; i < s.length(); i++)  {
    if (s[i] == '"')   ret += '\\';
    ret += s[i];
  }

  return ret;
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

gbtIndexPair::gbtIndexPair(const int& f, const int& s)
: first(f), second(s)
{
}

gbtIndexPair::~gbtIndexPair()
{
}

bool gbtIndexPair::operator == (const gbtIndexPair& other) const
{
  if ((first == other.first) && (second == other.second))
    return true;
  else
    return false;
}

bool gbtIndexPair::operator != (const gbtIndexPair& other) const
{
  if ((first != other.first) || (second != other.second))
    return true;
  else
    return false;
}

int gbtIndexPair::operator [] (const int& index) const 
{
  if (index == 1) return first;
  else            return second;
}
 

std::string gbtIndexException::GetDescription(void) const
{ return "Index out of range"; }

std::string gbtDimensionException::GetDescription(void) const
{ return "Dimension out of range"; }

std::string gbtRangeException::GetDescription(void) const 
{ return "Invalid range"; }


