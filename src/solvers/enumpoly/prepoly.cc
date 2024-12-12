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

//-----------------------------------------------------------
//                      gSpace
//-----------------------------------------------------------

//-------------------------
// Constructors/Destructors
//-------------------------

gSpace::gSpace(int nvars) : Variables()
{
  Variable *newvar;
  for (int i = 1; i <= nvars; i++) {
    newvar = new Variable;
    newvar->Name = 'n';
    newvar->Name += Gambit::lexical_cast<std::string>(i);
    newvar->number = i;
    Variables.push_back(newvar);
  }
}

gSpace::gSpace(const gSpace &p) : Variables()
{
  Variable *newvar;
  for (int i = 1; i <= p.Variables.Length(); i++) {
    newvar = new Variable;
    newvar->Name = p.Variables[i]->Name;
    newvar->number = i;
    Variables.push_back(newvar);
  }
}

gSpace::~gSpace()
{
  for (int i = 1; i <= Variables.Length(); i++) {
    delete Variables[i];
  }
}

//-----------------
// Member Functions
//-----------------

gSpace &gSpace::operator=(const gSpace &rhs)
{
  if (*this == rhs) {
    return *this;
  }

  Variables = rhs.Variables;
  return *this;
}

int gSpace::Dmnsn() const { return Variables.Length(); }

Variable *gSpace::VariableWithNumber(int i) const { return Variables[i]; }

Variable *gSpace::operator[](int i) const { return VariableWithNumber(i); }

bool gSpace::operator==(const gSpace &rhs) const
{
  if (Variables.Length() == rhs.Variables.Length() && Variables == rhs.Variables) {
    return true;
  }
  else {
    return false;
  }
}

bool gSpace::operator!=(const gSpace &rhs) const { return !(*this == rhs); }

//------------------------------------------------------
//                      exp_vect
//------------------------------------------------------

//-------------------------
// Constructors/Destructors
//-------------------------

exp_vect::exp_vect(const gSpace *p) : Space(p), components(p->Dmnsn())
{
  for (int i = 1; i <= p->Dmnsn(); i++) {
    components[i] = 0;
  }
}

exp_vect::exp_vect(const gSpace *p, const int &var, const int &exp)
  : Space(p), components(p->Dmnsn())
{
  for (int i = 1; i <= Dmnsn(); i++) {
    components[i] = 0;
  }
  components[var] = exp;
}

exp_vect::exp_vect(const gSpace *p, int *exponents) : Space(p), components(p->Dmnsn())
{
  for (int i = 1; i <= Dmnsn(); i++) {
    components[i] = exponents[i - 1];
  }
}

exp_vect::exp_vect(const gSpace *p, Gambit::Vector<int> exponents)
  : Space(p), components(p->Dmnsn())
{
  for (int i = 1; i <= Dmnsn(); i++) {
    components[i] = exponents[i];
  }
}

exp_vect::exp_vect(const gSpace *p, Gambit::Array<int> exponents)
  : Space(p), components(p->Dmnsn())
{
  for (int i = 1; i <= Dmnsn(); i++) {
    components[i] = exponents[i];
  }
}

//-------------------------
//        Operators
//-------------------------

exp_vect &exp_vect::operator=(const exp_vect &RHS)
{
  if (this == &RHS) {
    return *this;
  }

  Space = RHS.Space;
  components = RHS.components;
  return *this;
}

int exp_vect::operator[](int index) const { return components[index]; }

bool exp_vect::operator==(const exp_vect &RHS) const
{
  if (components == RHS.components) {
    return true;
  }
  else {
    return false;
  }
}

bool exp_vect::operator!=(const exp_vect &RHS) const { return !(*this == RHS); }

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

exp_vect exp_vect::operator-() const
{
  exp_vect tmp(Space);
  for (int i = 1; i <= Dmnsn(); i++) {
    tmp.components[i] = -components[i];
  }

  return tmp;
}

exp_vect exp_vect::operator+(const exp_vect &credit) const
{
  // assert (Space == credit.Space);

  exp_vect tmp(Space);
  for (int i = 1; i <= Dmnsn(); i++) {
    tmp.components[i] = components[i] + credit.components[i];
  }

  return tmp;
}

exp_vect exp_vect::operator-(const exp_vect &debit) const
{
  // assert (Space == debit.Space);

  exp_vect tmp(Space);
  for (int i = 1; i <= Dmnsn(); i++) {
    tmp.components[i] = components[i] - debit.components[i];
  }

  return tmp;
}

void exp_vect::operator+=(const exp_vect &credit)
{

  // assert (Space == credit.Space);

  for (int i = 1; i <= Dmnsn(); i++) {
    components[i] += credit.components[i];
  }
}

void exp_vect::operator-=(const exp_vect &debit)
{
  // assert (Space == debit.Space);

  for (int i = 1; i <= Dmnsn(); i++) {
    components[i] -= debit.components[i];
  }
}

//----------------------------
//        Other Operations
//----------------------------

exp_vect exp_vect::LCM(const exp_vect &arg2) const
{
  // assert (Space == arg2.Space);

  exp_vect tmp(Space);
  for (int i = 1; i <= Dmnsn(); i++) {
    if (components[i] < arg2.components[i]) {
      tmp.components[i] = arg2.components[i];
    }
    else {
      tmp.components[i] = components[i];
    }
  }

  return tmp;
}

exp_vect exp_vect::AfterZeroingOutExpOfVariable(int &varnumber) const
{
  exp_vect tmp(*this);
  tmp.components[varnumber] = 0;
  return tmp;
}

exp_vect exp_vect::AfterDecrementingExpOfVariable(int &varnumber) const
{
  exp_vect tmp(*this);
  tmp.components[varnumber]--;
  return tmp;
}

//--------------------------
//        Information
//--------------------------

int exp_vect::Dmnsn() const { return Space->Dmnsn(); }

bool exp_vect::IsConstant() const
{
  for (int i = 1; i <= Dmnsn(); i++) {
    if ((*this)[i] > 0) {
      return false;
    }
  }
  return true;
}

bool exp_vect::IsMultiaffine() const
{
  for (int i = 1; i <= Dmnsn(); i++) {
    if ((*this)[i] > 1) {
      return false;
    }
  }
  return true;
}

int exp_vect::TotalDegree() const
{
  int exp_sum = 0;
  for (int i = 1; i <= Dmnsn(); i++) {
    exp_sum += (*this)[i];
  }
  return exp_sum;
}

//--------------------------
//        Manipulation
//--------------------------

void exp_vect::ToZero()
{
  for (int i = 1; i <= Dmnsn(); i++) {
    components[i] = 0;
  }
}

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
