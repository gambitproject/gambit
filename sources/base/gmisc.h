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

#include <iostream>
#include <string>

//
// Converting between strings and other datatypes
//
std::string ToText(int);
std::string ToText(long);
std::string ToText(double);
std::string ToText(double p_number, int p_precision);

double ToDouble(const std::string &);

//
/// Return a copy of the string with all quotes preceded by a backslash
//
std::string EscapeQuotes(const std::string &);

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
};

inline std::ostream &operator<<(std::ostream &f, const gbtIndexPair &)
{ return f; }


//!
//! Abstract base class for all exceptions
//!
class gbtException   {
public:
  virtual ~gbtException() { }
  virtual std::string GetDescription(void) const = 0;
};

//!
//! An exception thrown when indexing outside the range of, for example,
//! an array.
//!
class gbtIndexException : public gbtException  {
public:
  virtual ~gbtIndexException() { }
  std::string GetDescription(void) const;
};

//!
//! An exception thrown on a dimension error (for example, operating on
//! two different length vectors).
//!
class gbtDimensionException : public gbtException {
public:
  virtual ~gbtDimensionException() { }
  std::string GetDescription(void) const;
};

//!
//! An exception thrown when a range is invalid (for example, last index
//! is smaller than first index)
//!
class gbtRangeException : public gbtException {
public:
  virtual ~gbtRangeException() { }
  std::string GetDescription(void) const;
};

#endif    // GMISC_H
