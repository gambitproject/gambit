//
// File: odometer.cc
//        -- Implementation of class gIndexOdometer
// $Id$ 
//

#include "odometer.h"


//---------------------------------------------------------------
//                      gIndexOdometer
//---------------------------------------------------------------

//---------------------------
// Constructors / Destructors
//---------------------------

gIndexOdometer::gIndexOdometer(const gArray<int> IndexUpperBounds) 
: MinIndices(IndexUpperBounds.Length()), 
  MaxIndices(IndexUpperBounds), 
  CurIndices(IndexUpperBounds.Length())
{
  for (int i = 1; i <= NoIndices(); i++) MinIndices[i] = 1;
  CurIndices[1] = 0;
  for (int i = 2; i <= NoIndices(); i++) CurIndices[i] = 1;
}

gIndexOdometer::gIndexOdometer(const gBlock<int> IndexUpperBounds) 
:  MinIndices(IndexUpperBounds.Length()),
   MaxIndices(IndexUpperBounds), 
   CurIndices(IndexUpperBounds.Length())
{
  for (int i = 1; i <= NoIndices(); i++) MinIndices[i] = 1;
  CurIndices[1] = 0;
  for (int i = 2; i <= NoIndices(); i++) CurIndices[i] = 1;
}

gIndexOdometer::gIndexOdometer(const gArray<int> IndexLowerBounds,
                               const gArray<int> IndexUpperBounds) 
: MinIndices(IndexLowerBounds), 
  MaxIndices(IndexUpperBounds), 
  CurIndices(IndexUpperBounds.Length())
{
  CurIndices[1] = MinIndices[1] - 1;;
  for (int i = 2; i <= NoIndices(); i++) CurIndices[i] = MinIndices[i];
}

gIndexOdometer::gIndexOdometer(const int* IndexUpperBounds, const int NoInd) 
: MinIndices(NoInd), 
  MaxIndices(NoInd), 
  CurIndices(NoInd)
{
  for (int i = 1; i <= NoIndices(); i++) MinIndices[i] = 1;
  for (int i = 1; i <= NoIndices(); i++) {
    MaxIndices[i] = IndexUpperBounds[i-1];
    CurIndices[i] = 1;
  }
  CurIndices[1] = 0;
}

gIndexOdometer::gIndexOdometer(const gIndexOdometer & odo)
: MaxIndices(odo.MaxIndices), CurIndices(odo.CurIndices)
{
}

gIndexOdometer::~gIndexOdometer()
{
}

//----------------------------------
//        Operators
//----------------------------------

 
gIndexOdometer& gIndexOdometer::operator=(const gIndexOdometer & rhs)
{
  if (*this != rhs) {
    MinIndices = rhs.MinIndices;
    MaxIndices = rhs.MaxIndices;
    CurIndices = rhs.CurIndices;
  }
}

  
bool gIndexOdometer::operator==(const gIndexOdometer & rhs) const
{
  if (MinIndices != rhs.MinIndices) return false;
  if (MaxIndices != rhs.MaxIndices) return false;
  if (CurIndices != rhs.CurIndices) return false;
                                    return true;
}

  
bool gIndexOdometer::operator!=(const gIndexOdometer & rhs) const
{
  return !(*this == rhs);
}

int gIndexOdometer::operator[](const int place) const
{
  assert(1 <= place && place <= NoIndices());
  return CurIndices[place];
}

//----------------------------------
//            Manipulate
//----------------------------------

void gIndexOdometer::SetIndex(const int& place, const int& newind)
{
  CurIndices[place] = newind;
}

bool gIndexOdometer::Turn()
{
  if (CurIndices[1] == MinIndices[1]-1) {
    CurIndices[1] = MinIndices[1];
    return true;
  }

  int turn_index = 1;
  while (            turn_index <=  NoIndices() &&
         CurIndices[turn_index] == MaxIndices[turn_index]) turn_index++;
  if (turn_index > NoIndices()) return false;

  for (int j = 1; j < turn_index; j++) CurIndices[j] = MinIndices[j];
  CurIndices[turn_index]++;
  return true;
}

//----------------------------------
//           Information
//----------------------------------

int gIndexOdometer::NoIndices() const 
{ 
  return MaxIndices.Length(); 
}

int gIndexOdometer::LinearIndex() const
{
  int index = (*this)[1];
  int factor = 1;

  for (int i = 2; i <= NoIndices(); i++) {
    factor *= MaxIndices[i-1] - MinIndices[i-1] + 1;
    index += factor * (CurIndices[i] - 1);
  }

  return index;
}

gArray<int> gIndexOdometer::CurrentIndices() const
{
  return CurIndices;
}

gIndexOdometer gIndexOdometer::AfterExcisionOf(int& to_be_zapped) const
{
  gBlock<int> NewMins, NewMaxs;
  for (int i = 1;              i < to_be_zapped; i++) 
    { NewMins += MinIndices[i]; NewMaxs += MaxIndices[i]; }
  for (int i = to_be_zapped+1; i <= NoIndices(); i++) 
    { NewMins += MinIndices[i]; NewMaxs += MaxIndices[i]; }

  gIndexOdometer NewOdo(NewMins,NewMaxs);

  for (int i = 1;              i < to_be_zapped; i++) 
    NewOdo.SetIndex(i  ,CurIndices[i]);
  for (int i = to_be_zapped+1; i <= NoIndices(); i++) 
    NewOdo.SetIndex(i-1,CurIndices[i]);

  return NewOdo;
}

//----------------------------------
//           Printing
//----------------------------------

gOutput& operator << (gOutput& output, const gIndexOdometer& odo)
{
  output << "[" << odo.CurIndices[1];
  for(int t = 2; t <= odo.NoIndices(); t++)
    output << "," << odo.CurIndices[t]; 
  output << "]";
  return  output;
}



//---------------------------------------------------------------
//                      gPermutationOdometer
//---------------------------------------------------------------

//---------------------------
// Constructors / Destructors
//---------------------------

gPermutationOdometer::gPermutationOdometer(const int& given_n) 
: n(given_n), CurIndices(n), CurSign(0)
{
  CurIndices[1] = 0;                   // Codes for virginity - see Turn() below
  for (int i = 2; i <= n; i++) CurIndices[i] = i;
}

gPermutationOdometer::gPermutationOdometer(const gPermutationOdometer & odo)
: n(odo.n), CurIndices(odo.CurIndices), CurSign(odo.CurSign)
{
}

gPermutationOdometer::~gPermutationOdometer()
{
}

//----------------------------------
//        Operators
//----------------------------------

 
gPermutationOdometer& gPermutationOdometer::operator=(const gPermutationOdometer & rhs)
{  
  gout << "For const'ness, operator = not allowed for gPermutationOdometer's\n";
  exit (0);
  gout << rhs; // suppresses warnings compiling with -Wall
/*
  if (*this != rhs) {
    n = rhs.n;
    CurIndices = rhs.CurIndices;
    CurSign = rhs.CurSign;
  }
*/
}

  
bool gPermutationOdometer::operator==(const gPermutationOdometer & rhs) const
{
  if (n != rhs.n)                           return false;
  for (int i = 1; i <= n; i++) 
    if (CurIndices[i] != rhs.CurIndices[i]) return false;

  if (CurSign != rhs.CurSign) { gout << "Error in gPermutationOdometer\n"; exit(1); }

  return true;
}

  
bool gPermutationOdometer::operator!=(const gPermutationOdometer & rhs) const
{
  return !(*this == rhs);
}

int gPermutationOdometer::operator[](const int place) const
{
  assert(1 <= place && place <= n);
  return CurIndices[place];
}

//----------------------------------
//            Manipulate
//----------------------------------

bool gPermutationOdometer::Turn()
{
  if (CurIndices[1] == 0) { // First turn gives identity permutation
    CurIndices[1] = 1;
    CurSign = 1;
    return true;
  }

  if (n ==1) return false;

  int cursor1 = n-1;
  while (cursor1 >= 1 && CurIndices[cursor1] > CurIndices[cursor1 + 1]) cursor1--;

  if (cursor1 == 0) return false;

  int cursor2 = cursor1 + 1;
  while (cursor2 < n && CurIndices[cursor2 + 1] > CurIndices[cursor1]) cursor2++; 

  int tmp = CurIndices[cursor2]; 
  CurIndices[cursor2] = CurIndices[cursor1];
  CurIndices[cursor1] = tmp;
  CurSign *= -1;

  cursor1++; cursor2 = n;
  while (cursor1 < cursor2) {
    int tmp = CurIndices[cursor2]; 
    CurIndices[cursor2] = CurIndices[cursor1];
    CurIndices[cursor1] = tmp;
    CurSign *= -1;
    cursor1++; cursor2--;
  }

  return true;
}

//----------------------------------
//           Information
//----------------------------------

int gPermutationOdometer::NoIndices() const 
{ 
  return n;
}

gArray<int> gPermutationOdometer::CurrentIndices() const
{
  return CurIndices;
}

int  gPermutationOdometer::CurrentSign() const
{
  return CurSign;
}

//----------------------------------
//           Printing
//----------------------------------

gOutput& operator << (gOutput& output, const gPermutationOdometer& odo)
{
  output << "[" << odo.CurIndices[1];
  for(int t = 2; t <= odo.NoIndices(); t++)
    output << "," << odo.CurIndices[t]; 
  output << "]";
  return  output;
}
