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


typedef enum { GBT_TRISTATE_FALSE = 0, GBT_TRISTATE_TRUE = 1, 
	           GBT_TRISTATE_UNKNOWN = 2 } gbtTriState;

class gbtText;
class gbtOutput;
gbtOutput &operator<<(gbtOutput &, gbtTriState);

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

class gbtInteger;
class gbtRational;

void ToTextWidth(int); // Set # of decimal places for floating point
int  ToTextWidth(void); // Get the current value of the above
void ToTextPrecision(int); // Set # of decimal places for floating point
int  ToTextPrecision(void); // Get the current value of the above

gbtText ToText(int);
gbtText ToText(long);
gbtText ToText(double);
gbtText ToText(double p_number, int p_precision);
gbtText ToText(gbtTriState);

double ToDouble(const gbtText &);

//
/// Return a copy of the string with all quotes preceded by a backslash
//
gbtText EscapeQuotes(const gbtText &);

//
// Type dependent epsilon
//

void gEpsilon(double &v, int i = 8);

double pow(int,long);
double pow(double,long);

//
// Simple class for compact reference to pairs of indices
//

class gbtIndexPair {
private:
  const int first;
  const int second;
  
public:
  gbtIndexPair(const int&, const int&);
  ~gbtIndexPair();
  
  bool operator == (const gbtIndexPair&) const;
  bool operator != (const gbtIndexPair&) const;
  int operator [] (const int&) const; 

  friend gbtOutput& operator << (gbtOutput& output, const gbtIndexPair& x);  
};

class gbtException   {
public:
  virtual ~gbtException();
  
  virtual gbtText Description(void) const = 0;
};


#endif    // GMISC_H
