//
// File: odometer.h -- Declaration of gIndexOdometer class //#
// 
// $Id$
//

/*
   When the cartesian product of ordered sets in ordered 
lexicographically, there is a relation between such a relation,
and the numerical indexing derived from the lexicographical
ordering, that is similar to an odometer.  Here the least
significant idex is the first ("leftmost").
*/

#ifndef ODOMETER_H
#define ODOMETER_H

#include "gblock.h"


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

#endif ODOMETER_H
