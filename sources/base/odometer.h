//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of gIndexOdometer class 
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

#include "base.h"

// *****************************
//      class gIndexOdometer
// *****************************

class gIndexOdometer {
 private:
  gArray<int> MinIndices;
  gArray<int> MaxIndices;
  gArray<int> CurIndices;
   
 public:
   gIndexOdometer(const gArray<int>);  
   gIndexOdometer(const gBlock<int>);  
   gIndexOdometer(const gArray<int>, const gArray<int>);  
   gIndexOdometer(const int*, const int);
   gIndexOdometer(const gIndexOdometer &);

   ~gIndexOdometer();                 // Deletes all pointees

   // Operators
   gIndexOdometer& operator= (const gIndexOdometer &);

   bool       operator==(const gIndexOdometer &) const;
   bool       operator!=(const gIndexOdometer &) const;

   int        operator[](const int)         const;

  // Manipulate
  void        SetIndex(const int&, const int&);
  bool        Turn();

   // Information
  int             NoIndices()           const;
  int             LinearIndex()         const;
  gArray<int>     CurrentIndices()      const;
  gIndexOdometer  AfterExcisionOf(int&) const;

friend gOutput& operator << (gOutput& output, const gIndexOdometer& x);
};  


// *****************************
//      class gPermutationOdometer
// *****************************

class gPermutationOdometer {
 private:
  const int   n;
  gArray<int> CurIndices;
  int         CurSign;

  // Declared but not defined to prohibit assignment
  gPermutationOdometer &operator=(const gPermutationOdometer &);

 public:
   gPermutationOdometer(const int&);  
   gPermutationOdometer(const gPermutationOdometer &);
   ~gPermutationOdometer();                 // Deletes all pointees

   // Operators

   bool       operator==(const gPermutationOdometer &) const;
   bool       operator!=(const gPermutationOdometer &) const;

   int        operator[](const int)         const;

  // Manipulate
  bool        Turn();

   // Information
  int             NoIndices()           const;
  gArray<int>     CurrentIndices()      const;
  int             CurrentSign()         const;

friend gOutput& operator << (gOutput& output, const gPermutationOdometer& x);
};  

#endif // ODOMETER_H
