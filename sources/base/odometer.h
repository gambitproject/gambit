//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of gbtIndexOdometer class 
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
//      class gbtIndexOdometer
// *****************************

class gbtIndexOdometer {
 private:
  gbtArray<int> MinIndices;
  gbtArray<int> MaxIndices;
  gbtArray<int> CurIndices;
   
 public:
   gbtIndexOdometer(const gbtArray<int>);  
   gbtIndexOdometer(const gbtBlock<int>);  
   gbtIndexOdometer(const gbtArray<int>, const gbtArray<int>);  
   gbtIndexOdometer(const int*, const int);
   gbtIndexOdometer(const gbtIndexOdometer &);

   ~gbtIndexOdometer();                 // Deletes all pointees

   // Operators
   gbtIndexOdometer& operator= (const gbtIndexOdometer &);

   bool       operator==(const gbtIndexOdometer &) const;
   bool       operator!=(const gbtIndexOdometer &) const;

   int        operator[](const int)         const;

  // Manipulate
  void        SetIndex(const int&, const int&);
  bool        Turn();

   // Information
  int             NoIndices()           const;
  int             LinearIndex()         const;
  gbtArray<int>     CurrentIndices()      const;
  gbtIndexOdometer  AfterExcisionOf(int&) const;

friend std::ostream& operator << (std::ostream& output, const gbtIndexOdometer& x);
};  


// *****************************
//      class gbtPermutationOdometer
// *****************************

class gbtPermutationOdometer {
 private:
  const int   n;
  gbtArray<int> CurIndices;
  int         CurSign;

  // Declared but not defined to prohibit assignment
  gbtPermutationOdometer &operator=(const gbtPermutationOdometer &);

 public:
   gbtPermutationOdometer(const int&);  
   gbtPermutationOdometer(const gbtPermutationOdometer &);
   ~gbtPermutationOdometer();                 // Deletes all pointees

   // Operators

   bool       operator==(const gbtPermutationOdometer &) const;
   bool       operator!=(const gbtPermutationOdometer &) const;

   int        operator[](const int)         const;

  // Manipulate
  bool        Turn();

   // Information
  int             NoIndices()           const;
  gbtArray<int>     CurrentIndices()      const;
  int             CurrentSign()         const;

friend std::ostream& operator << (std::ostream& output, 
				  const gbtPermutationOdometer& x);
};  

#endif // ODOMETER_H
