//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/linalg/lemketab.cc
// Lemke tableau instantiations
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

#include "lemketab.h"

namespace Gambit::linalg {

template <class T> int LemkeTableau<T>::SF_PivotIn(int inlabel)
{
  const int outindex = SF_ExitIndex(inlabel);
  if (outindex == 0) {
    return inlabel;
  }
  const int outlabel = this->GetLabel(outindex);
  this->Pivot(outindex, inlabel);
  return outlabel;
}

template <class T> int LemkeTableau<T>::SF_ExitIndex(int inlabel)
{
  Array<int> BestSet;
  int c;
  T ratio, tempmax;
  Vector<T> incol(this->MinRow(), this->MaxRow());
  Vector<T> col(this->MinRow(), this->MaxRow());

  this->SolveColumn(inlabel, incol);
  // Find all row indices for which column col has positive entries.
  for (int i = this->MinRow(); i <= this->MaxRow(); i++) {
    if (incol[i] > this->m_zeroTolerance) {
      BestSet.push_back(i);
    }
  }
  if (BestSet.size() == 0) {
    return 0;
  }
  if (BestSet.size() <= 0) {
    throw BadExitIndex();
  }

  // If there are multiple candidates, break ties by
  // looking at ratios with other columns,
  // eliminating nonmaximizers of
  // a similar ratio, until only one candidate remains.
  c = this->MinRow() - 1;
  this->GetBasisVector(col);
  while (BestSet.size() > 1) {
    if (c > this->MaxRow()) {
      throw BadExitIndex();
    }
    if (c >= this->MinRow()) {
      this->SolveColumn(-c, col);
    }
    // Initialize tempmax.
    tempmax = col[BestSet[1]] / incol[BestSet[1]];
    // Find the maximum ratio.
    for (size_t i = 2; i <= BestSet.size(); i++) {
      ratio = col[BestSet[i]] / incol[BestSet[i]];
      if (ratio < tempmax) {
        tempmax = ratio;
      }
    }

    // Remove nonmaximizers from the list of candidate columns.
    for (size_t i = BestSet.size(); i >= 1; i--) {
      ratio = col[BestSet[i]] / incol[BestSet[i]];
      if (ratio > tempmax + this->m_zeroTolerance) {
        BestSet.erase_at(i);
      }
    }
    c++;
  }
  if (BestSet.empty()) {
    throw BadExitIndex();
  }
  return BestSet.front();
}

template <class T> int LemkeTableau<T>::ExitIndex(int inlabel)
{
  Array<int> BestSet;
  int i, c;
  T ratio, tempmax;
  Vector<T> incol(this->MinRow(), this->MaxRow());
  Vector<T> col(this->MinRow(), this->MaxRow());

  this->SolveColumn(inlabel, incol);
  // Find all row indices for which column col has positive entries.
  for (i = this->MinRow(); i <= this->MaxRow(); i++) {
    if (incol[i] > this->m_zeroTolerance) {
      BestSet.push_back(i);
    }
  }
  // Is this really needed?
  if (BestSet.size() == 0) {
    if (BestSet.size() == 0 && incol[this->GetPosition(0)] <= this->m_zeroTolerance &&
        incol[this->GetPosition(0)] >= (-this->m_zeroTolerance)) {
      return this->GetPosition(0);
    }
  }
  if (BestSet.size() <= 0) {
    throw BadExitIndex();
  }

  // If there are multiple candidates, break ties by
  // looking at ratios with other columns,
  // eliminating nonmaximizers of
  // a similar ratio, until only one candidate remains.
  c = this->MinRow() - 1;
  this->GetBasisVector(col);
  while (BestSet.size() > 1) {
    if (c > this->MaxRow()) {
      throw BadExitIndex();
    }
    if (c >= this->MinRow()) {
      this->SolveColumn(-c, col);
    }
    // Initialize tempmax.
    tempmax = col[BestSet[1]] / incol[BestSet[1]];
    // Find the maximum ratio.
    for (size_t j = 2; j <= BestSet.size(); j++) {
      ratio = col[BestSet[j]] / incol[BestSet[j]];
      if (ratio > tempmax) {
        tempmax = ratio;
      }
    }

    // Remove nonmaximizers from the list of candidate columns.
    for (size_t j = BestSet.size(); j >= 1; j--) {
      ratio = col[BestSet[j]] / incol[BestSet[j]];
      if (ratio < tempmax - this->m_feasibilityTolerance) {
        BestSet.erase_at(j);
      }
    }
    c++;
  }
  if (BestSet.empty()) {
    throw BadExitIndex();
  }
  return BestSet.front();
}

template <class T> int LemkeTableau<T>::SF_LCPPath(int dup, const CancelToken &p_cancel)
{
  int enter, exit;
  enter = dup;
  // Central loop - pivot until another CBFS is found
  do {
    p_cancel.Check();
    // Talk about optimism! This is dumb, but better than nothing (I guess):
    exit = SF_PivotIn(enter);
    if (exit == enter) {
      return 0;
    }
    enter = -exit;
  } while (exit != 0);
  return 1;
}

template class LemkeTableau<double>;
template class LemkeTableau<Rational>;

} // end namespace Gambit::linalg
