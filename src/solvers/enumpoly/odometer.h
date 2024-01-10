//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/odometer.h
// Declaration of gIndexOdometer class 
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

/*
   When the cartesian product of ordered sets is ordered 
lexicographically, there is a relation between such a relation,
and the numerical indexing derived from the lexicographical
ordering, that is similar to an odometer.  Here the least
significant index is the first ("leftmost").

   The second class provides a utility for cycling through the integers 1..n.
*/

#ifndef ODOMETER_H
#define ODOMETER_H

#include "gambit.h"

// *****************************
//      class gIndexOdometer
// *****************************

class gIndexOdometer {
 private:
  Gambit::Array<int> MinIndices;
  Gambit::Array<int> MaxIndices;
  Gambit::Array<int> CurIndices;
   
 public:
   explicit gIndexOdometer(const Gambit::Array<int> &);
   gIndexOdometer(const Gambit::Array<int> &, const Gambit::Array<int> &);
   gIndexOdometer(const gIndexOdometer &) = default;
   ~gIndexOdometer() = default;

  // Operators
   gIndexOdometer& operator=(const gIndexOdometer &) = default;

   bool operator==(const gIndexOdometer &p_rhs) const
   { return (MinIndices == p_rhs.MinIndices) && (MaxIndices == p_rhs.MaxIndices) && (CurIndices == p_rhs.CurIndices); }
   bool operator!=(const gIndexOdometer &p_rhs) const
   { return (MinIndices != p_rhs.MinIndices) || (MaxIndices != p_rhs.MaxIndices) || (CurIndices != p_rhs.CurIndices); }

   int operator[](int place) const { return CurIndices[place]; }

  // Manipulate
  void        SetIndex(const int&, const int&);
  bool        Turn();

   // Information
  int             NoIndices()           const;
  int             LinearIndex()         const;
  Gambit::Array<int>     CurrentIndices()      const;
  gIndexOdometer  AfterExcisionOf(int&) const;

};  


// *****************************
//      class gPermutationOdometer
// *****************************

class gPermutationOdometer {
 private:
  int n;
  Gambit::Array<int> CurIndices;
  int CurSign;

 public:
   explicit gPermutationOdometer(int);
   gPermutationOdometer(const gPermutationOdometer &) = default;
   ~gPermutationOdometer() = default;
   gPermutationOdometer &operator=(const gPermutationOdometer &) = delete;

   // Operators
   bool operator==(const gPermutationOdometer &) const;
   bool operator!=(const gPermutationOdometer &) const;

   int operator[](int place) const { return CurIndices[place]; }

  // Manipulate
  bool        Turn();

   // Information
  int NoIndices() const { return n; }
  const Gambit::Array<int> &CurrentIndices() const { return CurIndices; }
  int CurrentSign() const { return CurSign; }

};  

#endif // ODOMETER_H
