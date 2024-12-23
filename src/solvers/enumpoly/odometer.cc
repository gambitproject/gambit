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
  : MinIndices(IndexUpperBounds.size()), MaxIndices(IndexUpperBounds),
    CurIndices(IndexUpperBounds.size())
{
  int i;
  for (i = 1; i <= NoIndices(); i++) {
    MinIndices[i] = 1;
  }
  CurIndices[1] = 0;
  for (i = 2; i <= NoIndices(); i++) {
    CurIndices[i] = 1;
  }
}

gIndexOdometer::gIndexOdometer(const Gambit::Array<int> &IndexLowerBounds,
                               const Gambit::Array<int> &IndexUpperBounds)
  : MinIndices(IndexLowerBounds), MaxIndices(IndexUpperBounds), CurIndices(IndexUpperBounds.size())
{
  CurIndices[1] = MinIndices[1] - 1;
  for (int i = 2; i <= NoIndices(); i++) {
    CurIndices[i] = MinIndices[i];
  }
}

//----------------------------------
//            Manipulate
//----------------------------------

bool gIndexOdometer::Turn()
{
  if (CurIndices[1] == MinIndices[1] - 1) {
    CurIndices[1] = MinIndices[1];
    return true;
  }

  int turn_index = 1;
  while (turn_index <= NoIndices() && CurIndices[turn_index] == MaxIndices[turn_index]) {
    turn_index++;
  }
  if (turn_index > NoIndices()) {
    return false;
  }

  for (int j = 1; j < turn_index; j++) {
    CurIndices[j] = MinIndices[j];
  }
  CurIndices[turn_index]++;
  return true;
}
