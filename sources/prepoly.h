//#
//# File: pre_poly.h  --  Declaration of classes supporting 
//#                       multivariate polynomials.
//# @(#)pre_poly.h 1.0 12/12/95
//#

#ifndef PRE_POLY_H
#define PRE_POLY_H

#include <math.h>
#include "gambitio.h"
#include "gstring.h"
#include "garray.h"
#include "glist.h"
#include "gblock.h"
#include "plyhdrn.h"

/*
   The classes in this file are prior to the notion of a 
multivariate polynomial.  First of all, one needs a space which the
polynomials refer to.  This is given by the notion of a gVariableList.
A polynomial is a sum of monomials, where each monomial is a
coefficient multiplied by a power product, and each power product is
the vector of variables "raised" by an exponent vector.  The exponent
vectors play a special role in many algorithms, especially those
associated with Groebner bases, and they have a class.  Finally, the
notion of a term order on the set of exponent vector, which
encompasses potentially infinitely many distinct orders, seems best
implemented through a class that delivers a full menu of services
built on top of a pointer to a function for computing an order.
*/


// *************************
// gVariableList declaration
// *************************

struct Variable{
  gString Name;
  int number;
};

class gVariableList {
private:
  
  int NoOfVars;
  gBlock < Variable * > Variables;

public:
  gVariableList(int nvars = 1);
  gVariableList(const gVariableList &);
  ~gVariableList();

  gVariableList& operator=(const gVariableList & rhs);
  
  int             NumVariables(void) const;
  Variable*       VariableWithNumber(int) const;
  const gString&  GetVariableName(int i) const;
  void            SetVariableName(int i, const gString &);
  void            CreateVariables (int nvars = 1);
//  gPolyFamily<T>  NewFamilyWithoutVariable(int var);

  Variable*       operator[](int i) const;
  bool            operator==(const gVariableList & rhs) const;
  bool            operator!=(const gVariableList & rhs) const;

  void            Dump(gOutput &) const;  // Debugging output
};


// ***********************
// class exp_vect 
// ***********************

/*
   Exponent vectors are vectors of integers.  In specifying operators
   we take the view that addition, subtraction, and order are defined,
   but not inner or scalar products.
*/

class exp_vect {
  friend gOutput& operator<<(gOutput&, const exp_vect&);

private:
  const gVariableList* List;
  gVector<int> components;

public:
  exp_vect(const gVariableList*);
  exp_vect(const gVariableList*, int*);
  exp_vect(const exp_vect &);
  ~exp_vect();

// Operators
  exp_vect& operator=(const exp_vect & RHS);

  int operator[](int index) const;

  bool operator==(const exp_vect & RHS) const;
  bool operator!=(const exp_vect & RHS) const;
  bool operator<=(const exp_vect & RHS) const;
  bool operator>=(const exp_vect & RHS) const;
  bool operator< (const exp_vect & RHS) const;
  bool operator> (const exp_vect & RHS) const;

  exp_vect  operator -  () const;
  exp_vect  operator +  (const exp_vect &) const;
  exp_vect  operator -  (const exp_vect &) const;
  void      operator += (const exp_vect &);
  void      operator -= (const exp_vect &);

// Other operations
  exp_vect  LCM(const exp_vect &) const;

// Information
  int  NumberOfVariables() const;
  bool IsPositive() const;
  bool IsNonnegative() const;
  int  TotalDegree() const;

// Manipulation
  void SetExp(int varno, int pow);
};


// ***********************
// class term_order
// ***********************

/*
   A term order is a total order of the set of exponent vectors
associated with a particular variable list, that has the properties:

   a) 1 < alpha for all alpha \ne 1; 
   b) if alpha < beta, then alpha + gamma < beta + gamma for all gamma >= 0.

   In our implementation we take the view that the order itself is a
variable of an object of the class, and implement this in terms of
pointers to functions. 
*/

// THE FOLLOWING FUNCTIONS SHOULD BE VIEWED AS PRIVATE MEMBERS OF 
// class term_order  I WAS BAFFLED AS TO HOW TO HAVE A MEMBER THAT
// IS A POINTER-TO-OTHER-MEMBER-FUNCTION
typedef  const bool (*ORD_PTR)(const exp_vect &, const exp_vect &);
  bool lex(const exp_vect &, const exp_vect &);
  bool deglex(const exp_vect &, const exp_vect &);
  bool reversedeglex(const exp_vect &, const exp_vect &);
  bool degrevlex(const exp_vect &, const exp_vect &);
  bool reversedegrevlex(const exp_vect &, const exp_vect &);

class term_order {
private:
  const gVariableList* List;
  ORD_PTR actual_order;

public:
  term_order(const gVariableList*, ORD_PTR);
  term_order(const term_order &);
  ~term_order();

// Operators
  term_order& operator=(term_order & RHS);

  bool operator==(const term_order & RHS) const;
  bool operator!=(const term_order & RHS) const;

// Comparisons invoking the underlying order
  bool Less(const exp_vect &, const exp_vect &) const;
  bool LessOrEqual(const exp_vect &, const exp_vect &) const;
  bool Greater(const exp_vect &, const exp_vect &) const;
  bool GreaterOrEqual(const exp_vect &, const exp_vect &) const;
};

#endif //# PRE_POLY_H
