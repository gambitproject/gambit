//#
//# File: pre_poly.imp  -- Implementation of supporting classes for polynomials
//# @(#) pre_poly.imp	1.3 8/2/95
//#

#include "pre_poly.h"

//-----------------------------------------------------------
//                      gVariableList
//-----------------------------------------------------------


//-------------------------
// Constructors/Destructors
//-------------------------

gVariableList::gVariableList(int nvars )
{
  Variable *nvar;
  assert (nvars >= 1);

  for (int i = 1; i <= nvars; i++){
    nvar = new Variable;
    nvar->Name = 'n';
    nvar->Name += ToString(i);
    nvar->number = i;
    Variables.Append(nvar);
  }
  NoOfVars = nvars;  
}

gVariableList::gVariableList(const gVariableList &p)
{gout<<"pre_poly.cc3\n";//**

  NoOfVars = p.NoOfVars; 

  Variable *nvar;
  for (int i = 1; i <= NoOfVars; i++){gout<<"pre_poly.cc4\n";//**

    nvar = new Variable;
    nvar->Name = p.Variables[i]->Name;
    nvar->number = i;
    Variables.Append(nvar);
  }
} 

gVariableList::~gVariableList()
{
  for (int i = 1; i <= NoOfVars; i++) {
    delete Variables[i]; 
  }
}

//-----------------
// Member Functions
//-----------------

gVariableList& gVariableList::operator=(const gVariableList & rhs)
{gout<<"pre_poly.cc7\n";//**

  if (*this == rhs) return *this;

  NoOfVars = rhs.NoOfVars;
  Variables = rhs.Variables;
  return *this;
}

int gVariableList::NumVariables(void) const
{
  return NoOfVars;
}

Variable * gVariableList::VariableWithNumber(int i) const
{
  return Variables[i];
}

const gString & gVariableList::GetVariableName(int i) const
{gout<<"pre_poly.cc10\n";//**

  return ((Variables[i])->Name);
}

void gVariableList::SetVariableName(int i, const gString &s)
{
  (Variables[i])->Name = s;
}

void gVariableList::CreateVariables (int nvars )
{gout<<"pre_poly.cc12\n";//**

  Variable *var;
  for (int i = 1; i <= nvars; i++){gout<<"pre_poly.cc13\n";//**

    var = new Variable;
    var->Name = 'x';
    var->Name += ToString (NoOfVars + i);
    Variables.Append(var);
  }
  NoOfVars = NoOfVars + nvars;
}

Variable* gVariableList::operator[](int i) const
{
  return VariableWithNumber(i);
}

bool gVariableList::operator==(const gVariableList & rhs) const
{gout<<"pre_poly.cc15\n";//**

  if (NoOfVars == rhs.NoOfVars && Variables == rhs.Variables)
    return true;
  else
    return false;
}

bool gVariableList::operator!=(const gVariableList & rhs) const
{gout<<"pre_poly.cc16\n";//**

  return !(*this == rhs);
}

// - RESTORE WHEN NEEDED
// gVariableList gVariableList::NewFamilyWithoutVariable(int var)
// {gout<<"pre_poly.cc17\n";//**

//   gVariableList result(NoOfVars - 1);
//   for (int i = 1; i <= NoOfVars; i++)
//     {gout<<"pre_poly.cc18\n";//**

//       if (i < var)
// 	result.SetVariableName(i,GetVariableName(i));
//       else if (i > var)
// 	 result.SetVariableName(i,GetVariableName(i+1));
//     }
//   return result;
// }

void gVariableList::Dump(gOutput &f) const
{gout<<"pre_poly.cc19\n";//**

  f << "No of Variable: " << NoOfVars << "\n";
  for (int i=1; i<=NoOfVars; i++) {gout<<"pre_poly.cc20\n";//**

    f << "#" << (Variables[i])->number << " is " 
      << (Variables[i])->Name << "\n";
  }
/*
  f << "The Polynomials in this family are:\n";
  for (int n = 1; n <= polys.Length(); n++){gout<<"pre_poly.cc21\n";//**
  
    polys[n]->Print(f); f << "\n";
  }
*/		
}


//------------------------------------------------------
//                      exp_vect
//------------------------------------------------------


//-------------------------
// Constructors/Destructors
//-------------------------

exp_vect::exp_vect(const gVariableList* p) 
: components(p->NumVariables())     
{
  List = p;   
  for (int i = 1; i <= p->NumVariables(); i++)
    components[i] = 0;
}

exp_vect::exp_vect(const gVariableList* p, int* exponents)
: components(p->NumVariables())
{
  List = p;
  for (int i = 1; i <= List->NumVariables(); i++)
    components[i] = exponents[i-1];
}


exp_vect::exp_vect(const exp_vect & p)
: components(p.components)
{
  List = p.List;
}

exp_vect::~exp_vect()
{
  //  gout << "Gotcha!\n";
}



//-------------------------
//        Operators
//-------------------------

exp_vect& exp_vect::operator=(const exp_vect & RHS)
{
  if (*this == RHS) return *this;

  List = RHS.List;
  components = RHS.components;
  return *this;
}


int exp_vect::operator[](int index) const
{
  return components[index];
}


bool exp_vect::operator==(const exp_vect & RHS) const
{
  assert (List == RHS.List);

  if (components == RHS.components)
    return true;
  else
    return false;
}

bool exp_vect::operator!=(const exp_vect & RHS) const
{
  return !(*this == RHS);
}

bool exp_vect::operator<=(const exp_vect & RHS) const
{
  assert (List == RHS.List);

  for (int i = 1; i <= NumberOfVariables(); i++)
    if (components[i] > RHS.components[i])
      return false;

  return true;
}
 
bool exp_vect::operator>=(const exp_vect & RHS) const
{
  assert (List == RHS.List);

  for (int i = 1; i <= NumberOfVariables(); i++)
    if (components[i] < RHS.components[i])
      return false;

  return true;
}

bool exp_vect::operator< (const exp_vect & RHS) const
{
  return !(*this >= RHS);
}

bool exp_vect::operator> (const exp_vect & RHS) const
{
  return !(*this <= RHS);
}

exp_vect exp_vect::operator- () const
{
  exp_vect tmp(List);
  for (int i = 1; i <= NumberOfVariables(); i++)
    tmp.components[i] = -components[i];

  return tmp;
}

exp_vect  exp_vect::operator+ (const exp_vect & credit) const
{
  assert (List == credit.List);

  exp_vect tmp(List);
  for (int i = 1; i <= NumberOfVariables(); i++)
    tmp.components[i] = components[i] + credit.components[i];

  return tmp;
}

exp_vect  exp_vect::operator- (const exp_vect & debit) const
{
  assert (List == debit.List);

  exp_vect tmp(List);
  for (int i = 1; i <= NumberOfVariables(); i++)
    tmp.components[i] = components[i] - debit.components[i];

  return tmp;
}

void exp_vect::operator+=(const exp_vect & credit) 
{

  assert (List == credit.List);

  for (int i = 1; i <= NumberOfVariables(); i++)
    components[i] += credit.components[i];
}

void exp_vect::operator-=(const exp_vect & debit) 
{
  assert (List == debit.List);

  for (int i = 1; i <= NumberOfVariables(); i++)
    components[i] -= debit.components[i];
}


//----------------------------
//        Other Operations
//----------------------------

exp_vect  exp_vect::LCM(const exp_vect & arg2) const
{
  assert (List == arg2.List);

  exp_vect tmp(List);
  for (int i = 1; i <= NumberOfVariables(); i++)
    if (components[i] < arg2.components[i])
      tmp.components[i] = arg2.components[i];
    else
      tmp.components[i] = components[i];

  return tmp;
}


//--------------------------
//        Information
//--------------------------


int exp_vect::NumberOfVariables() const
{
  return List->NumVariables();
}

bool exp_vect::IsPositive() const
{gout<<"pre_poly.cc40\n";//**

  for (int i = 1; i <= NumberOfVariables(); i++)
    if (components[i] <= 0)
      return false;

  return true;
}

bool exp_vect::IsNonnegative() const
{gout<<"pre_poly.cc41\n";//**

  for (int i = 1; i <= NumberOfVariables(); i++)
    if (components[i] < 0)
      return false;

  return true;
}

int  exp_vect::TotalDegree() const
{
  int exp_sum = 0;
  for (int i = 1; i <= List->NumVariables(); i++) 
    exp_sum += (*this)[i];
  return exp_sum;
}


//--------------------------
//        Manipulation
//--------------------------

void  exp_vect::SetExp(int varno, int pow)
{
  assert (1 <= varno && varno <= List->NumVariables() && 0 <= pow);

  components[varno] = pow;
}


gOutput& operator<<(gOutput&, const exp_vect& vect)
{
  gout << "(";
  for (int i = 1; i < vect.NumberOfVariables(); i++)
    gout << vect[i] << ",";
  gout << vect[vect.NumberOfVariables()] << ")";
}


//------------------------------------------------------
//                      term_order
//------------------------------------------------------


//-------------------
// Possible Orderings
//-------------------

bool lex(const exp_vect & LHS, const exp_vect & RHS)
{
  for (int i = 1; i <= LHS.NumberOfVariables(); i++)
    if (LHS[i] < RHS[i]) return true;
    else if (LHS[i] > RHS[i]) return false;
  return false;
}

bool deglex(const exp_vect & LHS, const exp_vect & RHS)
{
  if      (LHS.TotalDegree() < RHS.TotalDegree()) return true;
  else if (LHS.TotalDegree() > RHS.TotalDegree()) return false;

  for (int i = 1; i <= LHS.NumberOfVariables(); i++)
    if      (LHS[i] < RHS[i]) return true;
    else if (LHS[i] > RHS[i]) return false;
  return false;
}

bool reversedeglex(const exp_vect & LHS, const exp_vect & RHS)
{
  if      (LHS.TotalDegree() < RHS.TotalDegree()) return true;
  else if (LHS.TotalDegree() > RHS.TotalDegree()) return false;

  for (int i = LHS.NumberOfVariables(); i >= 1; i--)
    if      (LHS[i] < RHS[i]) return true;
    else if (LHS[i] > RHS[i]) return false;
  return false;
}

bool degrevlex(const exp_vect & LHS, const exp_vect & RHS)
{
  if      (LHS.TotalDegree() < RHS.TotalDegree()) return true;
  else if (LHS.TotalDegree() > RHS.TotalDegree()) return false;

  for (int i = LHS.NumberOfVariables(); i >= 1; i--)
    if      (LHS[i] < RHS[i]) return false;
    else if (LHS[i] > RHS[i]) return true;
  return false;
}

bool reversedegrevlex(const exp_vect & LHS, const exp_vect & RHS)
{
  if      (LHS.TotalDegree() < RHS.TotalDegree()) return true;
  else if (LHS.TotalDegree() > RHS.TotalDegree()) return false;

  for (int i = 1; i <= LHS.NumberOfVariables(); i++)
    if      (LHS[i] < RHS[i]) return false;
    else if (LHS[i] > RHS[i]) return true;
  return false;
}


//-------------------------
// Constructors/Destructors
//-------------------------

term_order::term_order(const gVariableList* p,
             const bool (*act_ord)(const exp_vect &, const exp_vect &)) 
{
  List = p;   
  actual_order = act_ord;
}


term_order::term_order(const term_order & p)
{gout<<"pre_poly.cc48\n";//**

  List = p.List;
  actual_order = p.actual_order;
}

term_order::~term_order()
{
  //  gout << "Sihonara, sucker ... \n";
}



//-------------------------
//        Operators
//-------------------------

term_order& term_order::operator=(term_order & RHS) 
{gout<<"pre_poly.cc50\n";//**

  if (*this == RHS) return *this;

  List = RHS.List;
  actual_order = RHS.actual_order;
  return *this;
}


bool term_order::operator==(const term_order & RHS) const
{gout<<"pre_poly.cc51\n";//**

  return (List == RHS.List && actual_order == RHS.actual_order);
}

bool term_order::operator!=(const term_order & RHS) const
{gout<<"pre_poly.cc52\n";//**

  return !(*this == RHS);
}


//-------------------------
//        Comparisons
//-------------------------

bool term_order::Less(const exp_vect & LHS, const exp_vect & RHS) const
{gout<<"pre_poly.cc53\n";//**

  return (*actual_order)(LHS, RHS);
}

bool term_order::LessOrEqual(const exp_vect & LHS, const exp_vect & RHS) const
{
  return ((*actual_order)(LHS, RHS) || LHS == RHS);
}

bool term_order::Greater(const exp_vect & LHS, const exp_vect & RHS) const
{
  return !(LessOrEqual(LHS, RHS));
}

bool term_order::GreaterOrEqual(const exp_vect & LHS, 
				const exp_vect & RHS) const
{gout<<"pre_poly.cc56\n";//**

  return !(Less(LHS, RHS));
}

