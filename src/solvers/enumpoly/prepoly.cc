//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/prepoly.cc
// Implementation of supporting classes for polynomials
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

#include "gambit.h"

#include "prepoly.h"

//------------------------------------------------------
//                      exp_vect
//------------------------------------------------------

//-------------------------
//        Operators
//-------------------------

bool exp_vect::operator<=(const exp_vect &RHS) const
{
  for (int i = 1; i <= Dmnsn(); i++) {
    if (components[i] > RHS.components[i]) {
      return false;
    }
  }

  return true;
}

bool exp_vect::operator>=(const exp_vect &RHS) const
{
  for (int i = 1; i <= Dmnsn(); i++) {
    if (components[i] < RHS.components[i]) {
      return false;
    }
  }

  return true;
}

bool exp_vect::operator<(const exp_vect &RHS) const { return !(*this >= RHS); }

bool exp_vect::operator>(const exp_vect &RHS) const { return !(*this <= RHS); }

//------------------------------------------------------
//                      term_order
//------------------------------------------------------

//-------------------
// Possible Orderings
//-------------------

bool lex(const exp_vect &LHS, const exp_vect &RHS)
{
  for (int i = 1; i <= LHS.Dmnsn(); i++) {
    if (LHS[i] < RHS[i]) {
      return true;
    }
    else if (LHS[i] > RHS[i]) {
      return false;
    }
  }
  return false;
}

bool reverselex(const exp_vect &LHS, const exp_vect &RHS)
{
  for (int i = LHS.Dmnsn(); i >= 1; i--) {
    if (LHS[i] < RHS[i]) {
      return true;
    }
    else if (LHS[i] > RHS[i]) {
      return false;
    }
  }
  return false;
}

bool deglex(const exp_vect &LHS, const exp_vect &RHS)
{
  if (LHS.TotalDegree() < RHS.TotalDegree()) {
    return true;
  }
  else if (LHS.TotalDegree() > RHS.TotalDegree()) {
    return false;
  }

  for (int i = 1; i <= LHS.Dmnsn(); i++) {
    if (LHS[i] < RHS[i]) {
      return true;
    }
    else if (LHS[i] > RHS[i]) {
      return false;
    }
  }
  return false;
}

bool reversedeglex(const exp_vect &LHS, const exp_vect &RHS)
{
  if (LHS.TotalDegree() < RHS.TotalDegree()) {
    return true;
  }
  else if (LHS.TotalDegree() > RHS.TotalDegree()) {
    return false;
  }

  for (int i = LHS.Dmnsn(); i >= 1; i--) {
    if (LHS[i] < RHS[i]) {
      return true;
    }
    else if (LHS[i] > RHS[i]) {
      return false;
    }
  }
  return false;
}

bool degrevlex(const exp_vect &LHS, const exp_vect &RHS)
{
  if (LHS.TotalDegree() < RHS.TotalDegree()) {
    return true;
  }
  else if (LHS.TotalDegree() > RHS.TotalDegree()) {
    return false;
  }

  for (int i = LHS.Dmnsn(); i >= 1; i--) {
    if (LHS[i] < RHS[i]) {
      return false;
    }
    else if (LHS[i] > RHS[i]) {
      return true;
    }
  }
  return false;
}

bool reversedegrevlex(const exp_vect &LHS, const exp_vect &RHS)
{
  if (LHS.TotalDegree() < RHS.TotalDegree()) {
    return true;
  }
  else if (LHS.TotalDegree() > RHS.TotalDegree()) {
    return false;
  }

  for (int i = 1; i <= LHS.Dmnsn(); i++) {
    if (LHS[i] < RHS[i]) {
      return false;
    }
    else if (LHS[i] > RHS[i]) {
      return true;
    }
  }
  return false;
}

//-------------------------
// Constructors/Destructors
//-------------------------

term_order::term_order(const gSpace *p, ORD_PTR act_ord) : Space(p), actual_order(act_ord) {}

//-------------------------
//        Operators
//-------------------------

term_order &term_order::operator=(const term_order &RHS)
{
  if (this == &RHS) {
    return *this;
  }

  Space = RHS.Space;
  actual_order = RHS.actual_order;
  return *this;
}

bool term_order::operator==(const term_order &RHS) const
{
  return (Space == RHS.Space && actual_order == RHS.actual_order);
}

bool term_order::operator!=(const term_order &RHS) const { return !(*this == RHS); }

//-------------------------
//        Comparisons
//-------------------------

bool term_order::Less(const exp_vect &LHS, const exp_vect &RHS) const
{
  return (*actual_order)(LHS, RHS);
}

bool term_order::LessOrEqual(const exp_vect &LHS, const exp_vect &RHS) const
{
  return ((*actual_order)(LHS, RHS) || LHS == RHS);
}

bool term_order::Greater(const exp_vect &LHS, const exp_vect &RHS) const
{
  return !(LessOrEqual(LHS, RHS));
}

bool term_order::GreaterOrEqual(const exp_vect &LHS, const exp_vect &RHS) const
{
  return !(Less(LHS, RHS));
}
