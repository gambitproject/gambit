//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/odometer.cc
// Implementation of class gIndexOdometer
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

#include <cstdlib>
#include "odometer.h"


//---------------------------------------------------------------
//                      gIndexOdometer
//---------------------------------------------------------------

//---------------------------
// Constructors / Destructors
//---------------------------

gIndexOdometer::gIndexOdometer(const Gambit::Array<int> &IndexUpperBounds)
: MinIndices(IndexUpperBounds.Length()), 
  MaxIndices(IndexUpperBounds), 
  CurIndices(IndexUpperBounds.Length())
{
  int i;
  for (i = 1; i <= NoIndices(); i++) MinIndices[i] = 1;
  CurIndices[1] = 0;
  for (i = 2; i <= NoIndices(); i++) CurIndices[i] = 1;
}

gIndexOdometer::gIndexOdometer(const Gambit::Array<int> &IndexLowerBounds,
                               const Gambit::Array<int> &IndexUpperBounds)
: MinIndices(IndexLowerBounds), 
  MaxIndices(IndexUpperBounds), 
  CurIndices(IndexUpperBounds.Length())
{
  CurIndices[1] = MinIndices[1] - 1;
  for (int i = 2; i <= NoIndices(); i++) CurIndices[i] = MinIndices[i];
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

Gambit::Array<int> gIndexOdometer::CurrentIndices() const
{
  return CurIndices;
}

gIndexOdometer gIndexOdometer::AfterExcisionOf(int& to_be_zapped) const
{
  Gambit::Array<int> NewMins, NewMaxs;
  int i;
  for (i = 1;              i < to_be_zapped; i++)
    { NewMins.push_back(MinIndices[i]); NewMaxs.push_back(MaxIndices[i]); }
  for (i = to_be_zapped+1; i <= NoIndices(); i++)
    { NewMins.push_back(MinIndices[i]); NewMaxs.push_back(MaxIndices[i]); }

  gIndexOdometer NewOdo(NewMins,NewMaxs);

  for (i = 1;              i < to_be_zapped; i++)
    NewOdo.SetIndex(i  ,CurIndices[i]);
  for (i = to_be_zapped+1; i <= NoIndices(); i++)
    NewOdo.SetIndex(i-1,CurIndices[i]);

  return NewOdo;
}

//---------------------------------------------------------------
//                      gPermutationOdometer
//---------------------------------------------------------------

//---------------------------
// Constructors / Destructors
//---------------------------

gPermutationOdometer::gPermutationOdometer(int given_n)
: n(given_n), CurIndices(n), CurSign(0)
{
  CurIndices[1] = 0;                   // Codes for virginity - see Turn() below
  for (int i = 2; i <= n; i++) CurIndices[i] = i;
}

//----------------------------------
//        Operators
//----------------------------------
  
bool gPermutationOdometer::operator==(const gPermutationOdometer & rhs) const
{
  if (n != rhs.n)                           return false;
  for (int i = 1; i <= n; i++) 
    if (CurIndices[i] != rhs.CurIndices[i]) return false;

  if (CurSign != rhs.CurSign) {
    //gout << "Error in gPermutationOdometer\n"; 
    exit(1);
  }

  return true;
}

  
bool gPermutationOdometer::operator!=(const gPermutationOdometer & rhs) const
{
  return !(*this == rhs);
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
    tmp = CurIndices[cursor2];
    CurIndices[cursor2] = CurIndices[cursor1];
    CurIndices[cursor1] = tmp;
    CurSign *= -1;
    cursor1++; cursor2--;
  }

  return true;
}
