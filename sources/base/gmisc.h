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

#ifndef GMISC_H
#define GMISC_H

//
// Defining shorthand names for some types
//
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned short ushort;


typedef enum { triFALSE = 0, triTRUE = 1, triUNKNOWN = 2 } gTriState;

class gText;
class gOutput;
gOutput &operator<<(gOutput &, gTriState);

//
// Generation of random numbers
//

// Set the seed of the random number generator
void SetSeed(unsigned int);

// Generate a random variable from the distribution U[0..1]
double Uniform(void);

// Generates a random number between 0 and IM exclusive of endpoints
// Adapted from _Numerical_Recipes_for_C_
#define IM 2147483647
long ran1(long* idum);

//
// Converting between strings and other datatypes
//

class gInteger;
class gRational;

void ToTextWidth(int); // Set # of decimal places for floating point
int  ToTextWidth(void); // Get the current value of the above
void ToTextPrecision(int); // Set # of decimal places for floating point
int  ToTextPrecision(void); // Get the current value of the above

gText ToText(int);
gText ToText(long);
gText ToText(double);
gText ToText(double p_number, int p_precision);
gText ToText(long double);
gText ToText(gTriState);

double ToDouble(const gText &);

//
/// Return a copy of the string with all quotes preceded by a backslash
//
gText EscapeQuotes(const gText &);

//
// Type dependent epsilon
//

void gEpsilon(double &v, int i = 8);

double pow(int,long);
double pow(double,long);

//
// Simple class for compact reference to pairs of indices
//

class index_pair {
private:
  const int first;
  const int second;
  
public:
  index_pair(const int&, const int&);
  ~index_pair();
  
  bool operator == (const index_pair&) const;
  bool operator != (const index_pair&) const;
  int operator [] (const int&) const; 

  friend gOutput& operator << (gOutput& output, const index_pair& x);  
};

class gException   {
public:
  virtual ~gException();
  
  virtual gText Description(void) const = 0;
};

class gNewFailed : public gException   {
public:
  virtual ~gNewFailed()   { }
  gText Description(void) const;
}; 

#endif    // GMISC_H

