//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of supporting classes for polynomials
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

#include <assert.h>
#include "base/base.h"
#include "prepoly.h"

//-----------------------------------------------------------
//                      gbtPolySpace
//-----------------------------------------------------------


//-------------------------
// Constructors/Destructors
//-------------------------

gbtPolySpace::gbtPolySpace(int nvars)
: Variables()
{
  Variable *newvar;

  for (int i = 1; i <= nvars; i++){
    newvar = new Variable;
    newvar->Name = 'n';
    newvar->Name += ToText(i);
    newvar->number = i;
    Variables.Append(newvar);
  }
}

gbtPolySpace::gbtPolySpace(const gbtPolySpace &p)
: Variables()
{
  Variable *newvar;
  for (int i = 1; i <= p.Variables.Length(); i++){
    newvar = new Variable;
    newvar->Name = p.Variables[i]->Name;
    newvar->number = i;
    Variables.Append(newvar);
  }
} 

gbtPolySpace::~gbtPolySpace()
{
  for (int i = 1; i <= Variables.Length(); i++) delete Variables[i]; 
}

//-----------------
// Member Functions
//-----------------

gbtPolySpace& gbtPolySpace::operator=(const gbtPolySpace & rhs)
{
  // gout<<"IF OK, ZAP ME:prepoly.cc7\n";//**

  if (*this == rhs) return *this;

  Variables = rhs.Variables;
  return *this;
}

int gbtPolySpace::Dmnsn(void) const
{
  return Variables.Length();
}

Variable * gbtPolySpace::VariableWithNumber(int i) const
{
  return Variables[i];
}

const std::string & gbtPolySpace::GetVariableName(int i) const
{
  // gout<<"IF OK, ZAP ME:prepoly.cc10\n";//**

  return ((Variables[i])->Name);
}

void gbtPolySpace::SetVariableName(int i, const std::string &s)
{
  (Variables[i])->Name = s;
}

void gbtPolySpace::CreateVariables (int nvars )
{
  // gout<<"IF OK, ZAP ME:prepoly.cc12\n";//**

  Variable *var;
  int n = Variables.Length();
  for (int i = 1; i <= nvars; i++) {
    // gout<<"IF OK, ZAP ME:prepoly.cc13\n";//**

    var = new Variable;
    var->Name = 'n';
    var->Name += ToText(n + i);
    Variables.Append(var);
  }
}

gbtPolySpace gbtPolySpace::WithVariableAppended() const
{
  // gout<<"IF OK, ZAP ME:prepoly.cc14\n";//**
 gbtPolySpace enlarged(*this);
 enlarged.CreateVariables(1);
 return enlarged;
}

Variable* gbtPolySpace::operator[](int i) const
{
  return VariableWithNumber(i);
}

bool gbtPolySpace::operator==(const gbtPolySpace & rhs) const
{
  // gout<<"IF OK, ZAP ME:prepoly.cc15\n";//**

  if (Variables.Length() == rhs.Variables.Length() && 
      Variables          == rhs.Variables)
    return true;
  else
    return false;
}

bool gbtPolySpace::operator!=(const gbtPolySpace & rhs) const
{
  // gout<<"IF OK, ZAP ME:prepoly.cc16\n";//**

  return !(*this == rhs);
}

// - RESTORE WHEN NEEDED
// gbtPolySpace gbtPolySpace::NewFamilyWithoutVariable(int var)
// {gout<<"IF OK, ZAP ME:prepoly.cc17\n";//**

//   gbtPolySpace result(NoOfVars - 1);
//   for (int i = 1; i <= NoOfVars; i++)
//     {gout<<"IF OK, ZAP ME:prepoly.cc18\n";//**

//       if (i < var)
// 	result.SetVariableName(i,GetVariableName(i));
//       else if (i > var)
// 	 result.SetVariableName(i,GetVariableName(i+1));
//     }
//   return result;
// }

void gbtPolySpace::Dump(std::ostream &f) const
{
  f << "No of Variable: " << Variables.Length() << "\n";
  for (int i=1; i<=Variables.Length(); i++) {

    f << "#" << (Variables[i])->number << " is " 
      << (Variables[i])->Name << "\n";
  }
}


//------------------------------------------------------
//                      gbtPolyExponent
//------------------------------------------------------


//-------------------------
// Constructors/Destructors
//-------------------------

gbtPolyExponent::gbtPolyExponent(const gbtPolySpace* p) 
: Space(p), components(p->Dmnsn())     
{
  for (int i = 1; i <= p->Dmnsn(); i++) components[i] = 0;
}

gbtPolyExponent::gbtPolyExponent(const gbtPolySpace* p, const int& var, const int& exp)
: Space(p), components(p->Dmnsn())
{
  for (int i = 1; i <= Dmnsn(); i++) components[i] = 0;
  components[var] = exp;
}

gbtPolyExponent::gbtPolyExponent(const gbtPolySpace* p, int* exponents)
: Space(p), components(p->Dmnsn())
{
  for (int i = 1; i <= Dmnsn(); i++) components[i] = exponents[i-1];
}

gbtPolyExponent::gbtPolyExponent(const gbtPolySpace* p, gbtVector<int> exponents)
: Space(p), components(p->Dmnsn())
{
  for (int i = 1; i <= Dmnsn(); i++) components[i] = exponents[i];
}

gbtPolyExponent::gbtPolyExponent(const gbtPolySpace* p, gbtArray<int> exponents)
: Space(p), components(p->Dmnsn())
{
  for (int i = 1; i <= Dmnsn(); i++) components[i] = exponents[i];
}

gbtPolyExponent::gbtPolyExponent(const gbtPolyExponent* p)
: Space(p->Space), components(p->components)
{
}

gbtPolyExponent::gbtPolyExponent(const gbtPolyExponent & p)
: Space(p.Space), components(p.components)
{
}

gbtPolyExponent::~gbtPolyExponent()
{
  //  gout << "Gotcha!\n";
}



//-------------------------
//        Operators
//-------------------------

gbtPolyExponent& gbtPolyExponent::operator=(const gbtPolyExponent & RHS)
{
  if (*this == RHS) return *this;

  Space = RHS.Space;
  components = RHS.components;
  return *this;
}


int gbtPolyExponent::operator[](int index) const
{
  return components[index];
}


bool gbtPolyExponent::operator==(const gbtPolyExponent & RHS) const
{
  assert (Space == RHS.Space);

  if (components == RHS.components)
    return true;
  else
    return false;
}

bool gbtPolyExponent::operator!=(const gbtPolyExponent & RHS) const
{
  return !(*this == RHS);
}

bool gbtPolyExponent::operator<=(const gbtPolyExponent & RHS) const
{
  assert (Space == RHS.Space);

  for (int i = 1; i <= Dmnsn(); i++)
    if (components[i] > RHS.components[i])
      return false;

  return true;
}
 
bool gbtPolyExponent::operator>=(const gbtPolyExponent & RHS) const
{
  assert (Space == RHS.Space);

  for (int i = 1; i <= Dmnsn(); i++)
    if (components[i] < RHS.components[i])
      return false;

  return true;
}

bool gbtPolyExponent::operator< (const gbtPolyExponent & RHS) const
{
  return !(*this >= RHS);
}

bool gbtPolyExponent::operator> (const gbtPolyExponent & RHS) const
{
  return !(*this <= RHS);
}

gbtPolyExponent gbtPolyExponent::operator- () const
{
  gbtPolyExponent tmp(Space);
  for (int i = 1; i <= Dmnsn(); i++)
    tmp.components[i] = -components[i];

  return tmp;
}

gbtPolyExponent  gbtPolyExponent::operator+ (const gbtPolyExponent & credit) const
{
  assert (Space == credit.Space);

  gbtPolyExponent tmp(Space);
  for (int i = 1; i <= Dmnsn(); i++)
    tmp.components[i] = components[i] + credit.components[i];

  return tmp;
}

gbtPolyExponent  gbtPolyExponent::operator- (const gbtPolyExponent & debit) const
{
  assert (Space == debit.Space);

  gbtPolyExponent tmp(Space);
  for (int i = 1; i <= Dmnsn(); i++)
    tmp.components[i] = components[i] - debit.components[i];

  return tmp;
}

void gbtPolyExponent::operator+=(const gbtPolyExponent & credit) 
{

  assert (Space == credit.Space);

  for (int i = 1; i <= Dmnsn(); i++)
    components[i] += credit.components[i];
}

void gbtPolyExponent::operator-=(const gbtPolyExponent & debit) 
{
  assert (Space == debit.Space);

  for (int i = 1; i <= Dmnsn(); i++)
    components[i] -= debit.components[i];
}


//----------------------------
//        Other Operations
//----------------------------

gbtPolyExponent  gbtPolyExponent::LCM(const gbtPolyExponent & arg2) const
{
  assert (Space == arg2.Space);

  gbtPolyExponent tmp(Space);
  for (int i = 1; i <= Dmnsn(); i++)
    if (components[i] < arg2.components[i])
      tmp.components[i] = arg2.components[i];
    else
      tmp.components[i] = components[i];

  return tmp;
}

gbtPolyExponent  gbtPolyExponent::WithVariableAppended(const gbtPolySpace* EnlargedSpace) const
{
  gbtPolyExponent tmp(EnlargedSpace);

  for (int i = 1; i <= Dmnsn(); i++)
    tmp.components[i] = components[i];
  tmp.components[Dmnsn() + 1] = 0;

  return tmp;
}

gbtPolyExponent  gbtPolyExponent::AfterZeroingOutExpOfVariable(int& varnumber) const
{
  gbtPolyExponent tmp(*this);
  tmp.components[varnumber] = 0;
  return tmp;
}

gbtPolyExponent  gbtPolyExponent::AfterDecrementingExpOfVariable(int& varnumber) const
{
  gbtPolyExponent tmp(*this);
  tmp.components[varnumber]--;
  return tmp;
}


//--------------------------
//        Information
//--------------------------

int gbtPolyExponent::Dmnsn() const
{
  return Space->Dmnsn();
}

bool gbtPolyExponent::IsPositive() const
{
  // gout<<"IF OK, ZAP ME:prepoly.cc40\n";//**

  for (int i = 1; i <= Dmnsn(); i++)
    if (components[i] <= 0)
      return false;

  return true;
}

bool gbtPolyExponent::IsNonnegative() const
{
  // gout<<"IF OK, ZAP ME:prepoly.cc41\n";//**

  for (int i = 1; i <= Dmnsn(); i++)
    if (components[i] < 0)
      return false;

  return true;
}

bool  gbtPolyExponent::IsConstant() const
{
  for (int i = 1; i <= Dmnsn(); i++)
    if ((*this)[i] > 0) return false;
  return true;
}

bool  gbtPolyExponent::IsMultiaffine() const
{
  for (int i = 1; i <= Dmnsn(); i++)
    if ((*this)[i] > 1) return false;
  return true;
}

bool  gbtPolyExponent::IsUnivariate() const
{
  int no_active_variables = 0;

  for (int i = 1; i <= Dmnsn(); i++)
    if ((*this)[i] > 0) no_active_variables++;

  if (no_active_variables == 1) return true;
  else                          return false;
}

int  gbtPolyExponent::SoleActiveVariable() const
{
  int sole_active_variable = 0;

  for (int i = 1; i <= Dmnsn(); i++)
    if ((*this)[i] > 0) {
      assert(sole_active_variable == 0);
      sole_active_variable = i;
    }

  assert (sole_active_variable > 0);
  return sole_active_variable;
}

int  gbtPolyExponent::TotalDegree() const
{
  int exp_sum = 0;
  for (int i = 1; i <= Dmnsn(); i++) 
    exp_sum += (*this)[i];
  return exp_sum;
}

bool  gbtPolyExponent::Divides(const gbtPolyExponent& n) const
{
  for (int i = 1; i <= Dmnsn(); i++)
    if ((*this)[i] > n[i])
      return false;
  return true;
}

bool  gbtPolyExponent::UsesDifferentVariablesThan(const gbtPolyExponent& n) const
{
  for (int i = 1; i <= Dmnsn(); i++)
    if ( ((*this)[i] > 0) && (n[i] > 0) )
      return false;
  return true;
}

//--------------------------
//        Manipulation
//--------------------------

void  gbtPolyExponent::SetExp(int varno, int pow)
{
  assert (1 <= varno && varno <= Dmnsn() && 0 <= pow);

  components[varno] = pow;
}

void  gbtPolyExponent::ToZero()
{
  for (int i = 1; i <= Dmnsn(); i++)
    components[i] = 0;
}

//--------------------------
//        Printing
//--------------------------


std::ostream& operator<<(std::ostream &f, const gbtPolyExponent& vect)
{
  f << "(";
  for (int i = 1; i < vect.Dmnsn(); i++)
    f << vect[i] << ",";
  f << vect[vect.Dmnsn()] << ")";
  return f; 
}


//------------------------------------------------------
//                      gbtPolyTermOrder
//------------------------------------------------------


//-------------------
// Possible Orderings
//-------------------

bool lex(const gbtPolyExponent & LHS, const gbtPolyExponent & RHS)
{
  for (int i = 1; i <= LHS.Dmnsn(); i++)
    if (LHS[i] < RHS[i]) return true;
    else if (LHS[i] > RHS[i]) return false;
  return false;
}

bool reverselex(const gbtPolyExponent & LHS, const gbtPolyExponent & RHS)
{
  for (int i = LHS.Dmnsn(); i >= 1; i--)
    if (LHS[i] < RHS[i]) return true;
    else if (LHS[i] > RHS[i]) return false;
  return false;
}

bool deglex(const gbtPolyExponent & LHS, const gbtPolyExponent & RHS)
{
  if      (LHS.TotalDegree() < RHS.TotalDegree()) return true;
  else if (LHS.TotalDegree() > RHS.TotalDegree()) return false;

  for (int i = 1; i <= LHS.Dmnsn(); i++)
    if      (LHS[i] < RHS[i]) return true;
    else if (LHS[i] > RHS[i]) return false;
  return false;
}

bool reversedeglex(const gbtPolyExponent & LHS, const gbtPolyExponent & RHS)
{
  if      (LHS.TotalDegree() < RHS.TotalDegree()) return true;
  else if (LHS.TotalDegree() > RHS.TotalDegree()) return false;

  for (int i = LHS.Dmnsn(); i >= 1; i--)
    if      (LHS[i] < RHS[i]) return true;
    else if (LHS[i] > RHS[i]) return false;
  return false;
}

bool degrevlex(const gbtPolyExponent & LHS, const gbtPolyExponent & RHS)
{
  if      (LHS.TotalDegree() < RHS.TotalDegree()) return true;
  else if (LHS.TotalDegree() > RHS.TotalDegree()) return false;

  for (int i = LHS.Dmnsn(); i >= 1; i--)
    if      (LHS[i] < RHS[i]) return false;
    else if (LHS[i] > RHS[i]) return true;
  return false;
}

bool reversedegrevlex(const gbtPolyExponent & LHS, const gbtPolyExponent & RHS)
{
  if      (LHS.TotalDegree() < RHS.TotalDegree()) return true;
  else if (LHS.TotalDegree() > RHS.TotalDegree()) return false;

  for (int i = 1; i <= LHS.Dmnsn(); i++)
    if      (LHS[i] < RHS[i]) return false;
    else if (LHS[i] > RHS[i]) return true;
  return false;
}


//-------------------------
// Constructors/Destructors
//-------------------------

gbtPolyTermOrder::gbtPolyTermOrder(const gbtPolySpace* p, ORD_PTR act_ord) 
: Space(p), actual_order(act_ord)
{
}

gbtPolyTermOrder::gbtPolyTermOrder(const gbtPolyTermOrder & p)
: Space(p.Space), actual_order(p.actual_order)
{
}

gbtPolyTermOrder::~gbtPolyTermOrder()
{
  //  gout << "Sihonara, sucker ... \n";
}



//-------------------------
//        Operators
//-------------------------

gbtPolyTermOrder& gbtPolyTermOrder::operator=(gbtPolyTermOrder & RHS) 
{
  // gout<<"IF OK, ZAP ME:prepoly.cc50\n";//**

  if (*this == RHS) return *this;

  Space = RHS.Space;
  actual_order = RHS.actual_order;
  return *this;
}


bool gbtPolyTermOrder::operator==(const gbtPolyTermOrder & RHS) const
{
  return (Space == RHS.Space && actual_order == RHS.actual_order);
}

bool gbtPolyTermOrder::operator!=(const gbtPolyTermOrder & RHS) const
{
  // gout<<"IF OK, ZAP ME:prepoly.cc52\n";//**

  return !(*this == RHS);
}


//-------------------------
//        Comparisons
//-------------------------

bool gbtPolyTermOrder::Less(const gbtPolyExponent & LHS, const gbtPolyExponent & RHS) const
{
  return (*actual_order)(LHS, RHS);
}

bool gbtPolyTermOrder::LessOrEqual(const gbtPolyExponent & LHS, const gbtPolyExponent & RHS) const
{
  return ((*actual_order)(LHS, RHS) || LHS == RHS);
}

bool gbtPolyTermOrder::Greater(const gbtPolyExponent & LHS, const gbtPolyExponent & RHS) const
{
  return !(LessOrEqual(LHS, RHS));
}

bool gbtPolyTermOrder::GreaterOrEqual(const gbtPolyExponent & LHS, 
				const gbtPolyExponent & RHS) const
{
  // gout<<"IF OK, ZAP ME:prepoly.cc56\n";//**

  return !(Less(LHS, RHS));
}


//-------------------------------------------
//        Manipulation and Information
//-------------------------------------------

gbtPolyTermOrder gbtPolyTermOrder::WithVariableAppended(const gbtPolySpace* ExtendedSpace) const
{
  return gbtPolyTermOrder(ExtendedSpace,actual_order);
}

