//
// FILE: prepoly.cc  -- Implementation of supporting classes for polynomials
//
// @(#)prepoly.cc	2.1 3/27/97
//

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
bool operator!=(const gArray<int> &, const gArray<int> &);
void gList<gBlock<int> >::Dump(gOutput &) const { };
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

#include "prepoly.h"
#include "glist.imp"

//-----------------------------------------------------------
//                      gSpace
//-----------------------------------------------------------


//-------------------------
// Constructors/Destructors
//-------------------------

gSpace::gSpace(int nvars)
: Variables()
{
  Variable *newvar;
  assert (nvars >= 0);

  for (int i = 1; i <= nvars; i++){
    newvar = new Variable;
    newvar->Name = 'n';
    newvar->Name += ToString(i);
    newvar->number = i;
    Variables.Append(newvar);
  }
}

gSpace::gSpace(const gSpace &p)
: Variables()
{gout<<"IF OK, ZAP ME:prepoly.cc3\n";//**

  Variable *newvar;
  for (int i = 1; i <= Variables.Length(); i++){gout<<"IF OK, ZAP ME:prepoly.cc4\n";//**

    newvar = new Variable;
    newvar->Name = p.Variables[i]->Name;
    newvar->number = i;
    Variables.Append(newvar);
  }
} 

gSpace::~gSpace()
{
  for (int i = 1; i <= Variables.Length(); i++) delete Variables[i]; 
}

//-----------------
// Member Functions
//-----------------

gSpace& gSpace::operator=(const gSpace & rhs)
{gout<<"IF OK, ZAP ME:prepoly.cc7\n";//**

  if (*this == rhs) return *this;

  Variables = rhs.Variables;
  return *this;
}

int gSpace::Dmnsn(void) const
{
  return Variables.Length();
}

Variable * gSpace::VariableWithNumber(int i) const
{
  return Variables[i];
}

const gString & gSpace::GetVariableName(int i) const
{gout<<"IF OK, ZAP ME:prepoly.cc10\n";//**

  return ((Variables[i])->Name);
}

void gSpace::SetVariableName(int i, const gString &s)
{
  (Variables[i])->Name = s;
}

void gSpace::CreateVariables (int nvars )
{ gout<<"IF OK, ZAP ME:prepoly.cc12\n";//**

  Variable *var;
  int n = Variables.Length();
  for (int i = 1; i <= nvars; i++){gout<<"IF OK, ZAP ME:prepoly.cc13\n";//**

    var = new Variable;
    var->Name = 'n';
    var->Name += ToString(n + i);
    Variables.Append(var);
  }
}

gSpace gSpace::WithVariableAppended() const
{gout<<"IF OK, ZAP ME:prepoly.cc14\n";//**
 gSpace enlarged(*this);
 enlarged.CreateVariables(1);
 return enlarged;
}

Variable* gSpace::operator[](int i) const
{
  return VariableWithNumber(i);
}

bool gSpace::operator==(const gSpace & rhs) const
{gout<<"IF OK, ZAP ME:prepoly.cc15\n";//**

  if (Variables.Length() == rhs.Variables.Length() && 
      Variables          == rhs.Variables)
    return true;
  else
    return false;
}

bool gSpace::operator!=(const gSpace & rhs) const
{gout<<"IF OK, ZAP ME:prepoly.cc16\n";//**

  return !(*this == rhs);
}

// - RESTORE WHEN NEEDED
// gSpace gSpace::NewFamilyWithoutVariable(int var)
// {gout<<"IF OK, ZAP ME:prepoly.cc17\n";//**

//   gSpace result(NoOfVars - 1);
//   for (int i = 1; i <= NoOfVars; i++)
//     {gout<<"IF OK, ZAP ME:prepoly.cc18\n";//**

//       if (i < var)
// 	result.SetVariableName(i,GetVariableName(i));
//       else if (i > var)
// 	 result.SetVariableName(i,GetVariableName(i+1));
//     }
//   return result;
// }

void gSpace::Dump(gOutput &f) const
{
  f << "No of Variable: " << Variables.Length() << "\n";
  for (int i=1; i<=Variables.Length(); i++) {

    f << "#" << (Variables[i])->number << " is " 
      << (Variables[i])->Name << "\n";
  }
}


//------------------------------------------------------
//                      exp_vect
//------------------------------------------------------


//-------------------------
// Constructors/Destructors
//-------------------------

exp_vect::exp_vect(const gSpace* p) 
: Space(p), components(p->Dmnsn())     
{
  for (int i = 1; i <= p->Dmnsn(); i++) components[i] = 0;
}

exp_vect::exp_vect(const gSpace* p, int& var, int& exp)
: Space(p), components(p->Dmnsn())
{
  for (int i = 1; i <= Dmnsn(); i++) components[i] = 0;
  components[var] = exp;
}

exp_vect::exp_vect(const gSpace* p, int* exponents)
: Space(p), components(p->Dmnsn())
{
  for (int i = 1; i <= Dmnsn(); i++) components[i] = exponents[i-1];
}

exp_vect::exp_vect(const gSpace* p, gVector<int> exponents)
: Space(p), components(p->Dmnsn())
{
  for (int i = 1; i <= Dmnsn(); i++) components[i] = exponents[i];
}

exp_vect::exp_vect(const gSpace* p, gArray<int> exponents)
: Space(p), components(p->Dmnsn())
{
  for (int i = 1; i <= Dmnsn(); i++) components[i] = exponents[i];
}

exp_vect::exp_vect(const exp_vect* p)
: Space(p->Space), components(p->components)
{
}

exp_vect::exp_vect(const exp_vect & p)
: Space(p.Space), components(p.components)
{
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

  Space = RHS.Space;
  components = RHS.components;
  return *this;
}


int exp_vect::operator[](int index) const
{
  return components[index];
}


bool exp_vect::operator==(const exp_vect & RHS) const
{
  assert (Space == RHS.Space);

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
  assert (Space == RHS.Space);

  for (int i = 1; i <= Dmnsn(); i++)
    if (components[i] > RHS.components[i])
      return false;

  return true;
}
 
bool exp_vect::operator>=(const exp_vect & RHS) const
{
  assert (Space == RHS.Space);

  for (int i = 1; i <= Dmnsn(); i++)
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
  exp_vect tmp(Space);
  for (int i = 1; i <= Dmnsn(); i++)
    tmp.components[i] = -components[i];

  return tmp;
}

exp_vect  exp_vect::operator+ (const exp_vect & credit) const
{
  assert (Space == credit.Space);

  exp_vect tmp(Space);
  for (int i = 1; i <= Dmnsn(); i++)
    tmp.components[i] = components[i] + credit.components[i];

  return tmp;
}

exp_vect  exp_vect::operator- (const exp_vect & debit) const
{
  assert (Space == debit.Space);

  exp_vect tmp(Space);
  for (int i = 1; i <= Dmnsn(); i++)
    tmp.components[i] = components[i] - debit.components[i];

  return tmp;
}

void exp_vect::operator+=(const exp_vect & credit) 
{

  assert (Space == credit.Space);

  for (int i = 1; i <= Dmnsn(); i++)
    components[i] += credit.components[i];
}

void exp_vect::operator-=(const exp_vect & debit) 
{
  assert (Space == debit.Space);

  for (int i = 1; i <= Dmnsn(); i++)
    components[i] -= debit.components[i];
}


//----------------------------
//        Other Operations
//----------------------------

exp_vect  exp_vect::LCM(const exp_vect & arg2) const
{
  assert (Space == arg2.Space);

  exp_vect tmp(Space);
  for (int i = 1; i <= Dmnsn(); i++)
    if (components[i] < arg2.components[i])
      tmp.components[i] = arg2.components[i];
    else
      tmp.components[i] = components[i];

  return tmp;
}

exp_vect  exp_vect::WithVariableAppended(const gSpace* EnlargedSpace) const
{
  exp_vect tmp(EnlargedSpace);

  for (int i = 1; i <= Dmnsn(); i++)
    tmp.components[i] = components[i];
  tmp.components[Dmnsn() + 1] = 0;

  return tmp;
}

exp_vect  exp_vect::AfterZeroingOutExpOfVariable(int& varnumber) const
{
  exp_vect tmp(*this);
  tmp.components[varnumber] = 0;
  return tmp;
}

exp_vect  exp_vect::AfterDecrementingExpOfVariable(int& varnumber) const
{
  exp_vect tmp(*this);
  tmp.components[varnumber]--;
  return tmp;
}


//--------------------------
//        Information
//--------------------------

int exp_vect::Dmnsn() const
{
  return Space->Dmnsn();
}

bool exp_vect::IsPositive() const
{gout<<"IF OK, ZAP ME:prepoly.cc40\n";//**

  for (int i = 1; i <= Dmnsn(); i++)
    if (components[i] <= 0)
      return false;

  return true;
}

bool exp_vect::IsNonnegative() const
{gout<<"IF OK, ZAP ME:prepoly.cc41\n";//**

  for (int i = 1; i <= Dmnsn(); i++)
    if (components[i] < 0)
      return false;

  return true;
}

bool  exp_vect::IsConstant() const
{
  for (int i = 1; i <= Dmnsn(); i++)
    if ((*this)[i] > 0) return false;
  return true;
}

bool  exp_vect::IsUnivariate() const
{
  int no_active_variables = 0;

  for (int i = 1; i <= Dmnsn(); i++)
    if ((*this)[i] > 0) no_active_variables++;

  if (no_active_variables == 1) return true;
  else                          return false;
}

int  exp_vect::SoleActiveVariable() const
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

int  exp_vect::TotalDegree() const
{
  int exp_sum = 0;
  for (int i = 1; i <= Dmnsn(); i++) 
    exp_sum += (*this)[i];
  return exp_sum;
}

bool  exp_vect::Divides(const exp_vect& n) const
{
  for (int i = 1; i <= Dmnsn(); i++)
    if ((*this)[i] > n[i])
      return false;
  return true;
}

bool  exp_vect::UsesDifferentVariablesThan(const exp_vect& n) const
{
  for (int i = 1; i <= Dmnsn(); i++)
    if ( ((*this)[i] > 0) && (n[i] > 0) )
      return false;
  return true;
}

//--------------------------
//        Manipulation
//--------------------------

void  exp_vect::SetExp(int varno, int pow)
{
  assert (1 <= varno && varno <= Dmnsn() && 0 <= pow);

  components[varno] = pow;
}

void  exp_vect::ToZero()
{
  for (int i = 1; i <= Dmnsn(); i++)
    components[i] = 0;
}

//--------------------------
//        Printing
//--------------------------


gOutput& operator<<(gOutput &f, const exp_vect& vect)
{
  f << "(";
  for (int i = 1; i < vect.Dmnsn(); i++)
    f << vect[i] << ",";
  f << vect[vect.Dmnsn()] << ")";
  return f; 
}


//------------------------------------------------------
//                      term_order
//------------------------------------------------------


//-------------------
// Possible Orderings
//-------------------

bool lex(const exp_vect & LHS, const exp_vect & RHS)
{
  for (int i = 1; i <= LHS.Dmnsn(); i++)
    if (LHS[i] < RHS[i]) return true;
    else if (LHS[i] > RHS[i]) return false;
  return false;
}

bool reverselex(const exp_vect & LHS, const exp_vect & RHS)
{
  for (int i = LHS.Dmnsn(); i >= 1; i--)
    if (LHS[i] < RHS[i]) return true;
    else if (LHS[i] > RHS[i]) return false;
  return false;
}

bool deglex(const exp_vect & LHS, const exp_vect & RHS)
{
  if      (LHS.TotalDegree() < RHS.TotalDegree()) return true;
  else if (LHS.TotalDegree() > RHS.TotalDegree()) return false;

  for (int i = 1; i <= LHS.Dmnsn(); i++)
    if      (LHS[i] < RHS[i]) return true;
    else if (LHS[i] > RHS[i]) return false;
  return false;
}

bool reversedeglex(const exp_vect & LHS, const exp_vect & RHS)
{
  if      (LHS.TotalDegree() < RHS.TotalDegree()) return true;
  else if (LHS.TotalDegree() > RHS.TotalDegree()) return false;

  for (int i = LHS.Dmnsn(); i >= 1; i--)
    if      (LHS[i] < RHS[i]) return true;
    else if (LHS[i] > RHS[i]) return false;
  return false;
}

bool degrevlex(const exp_vect & LHS, const exp_vect & RHS)
{
  if      (LHS.TotalDegree() < RHS.TotalDegree()) return true;
  else if (LHS.TotalDegree() > RHS.TotalDegree()) return false;

  for (int i = LHS.Dmnsn(); i >= 1; i--)
    if      (LHS[i] < RHS[i]) return false;
    else if (LHS[i] > RHS[i]) return true;
  return false;
}

bool reversedegrevlex(const exp_vect & LHS, const exp_vect & RHS)
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

term_order::term_order(const gSpace* p, ORD_PTR act_ord) 
: Space(p), actual_order(act_ord)
{
}

term_order::term_order(const term_order & p)
: Space(p.Space), actual_order(p.actual_order)
{
}

term_order::~term_order()
{
  //  gout << "Sihonara, sucker ... \n";
}



//-------------------------
//        Operators
//-------------------------

term_order& term_order::operator=(term_order & RHS) 
{gout<<"IF OK, ZAP ME:prepoly.cc50\n";//**

  if (*this == RHS) return *this;

  Space = RHS.Space;
  actual_order = RHS.actual_order;
  return *this;
}


bool term_order::operator==(const term_order & RHS) const
{
  return (Space == RHS.Space && actual_order == RHS.actual_order);
}

bool term_order::operator!=(const term_order & RHS) const
{gout<<"IF OK, ZAP ME:prepoly.cc52\n";//**

  return !(*this == RHS);
}


//-------------------------
//        Comparisons
//-------------------------

bool term_order::Less(const exp_vect & LHS, const exp_vect & RHS) const
{
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
{gout<<"IF OK, ZAP ME:prepoly.cc56\n";//**

  return !(Less(LHS, RHS));
}


//-------------------------------------------
//        Manipulation and Information
//-------------------------------------------

term_order term_order::WithVariableAppended(const gSpace* ExtendedSpace) const
{
  return term_order(ExtendedSpace,actual_order);
}


// INSTATIATIONS OF NEEDED LISTS

TEMPLATE class gList<exp_vect>;
TEMPLATE class gNode<exp_vect>;

TEMPLATE class gList<exp_vect*>;
TEMPLATE class gNode<exp_vect*>;
