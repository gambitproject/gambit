//
// FILE: prepoly.h  --  Declaration of classes supporting 
//                      multivariate polynomials.
//
// $Id$
//

#ifndef PREPOLY_H
#define PREPOLY_H

#include <math.h>
#include "gstream.h"
#include "gtext.h"
#include "garray.h"
#include "glist.h"
#include "gblock.h"
#include "rational.h"
#include "gvector.h"

/*
   The classes in this file are prior to the notion of a 
multivariate polynomial.  First of all, one needs a space which the
polynomials refer to.  This is given by the notion of a gSpace.
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
// gSpace declaration
// *************************

struct Variable  { 
  gText Name;
  int number;
};

class gSpace {
private:  
  gBlock < Variable * > Variables;

public:
  gSpace(int nvars = 0);
  gSpace(const gSpace &);
  ~gSpace();

  // operators
  gSpace&         operator=(const gSpace & rhs);

  Variable*       operator[](int)              const;
  bool            operator==(const gSpace & rhs) const;
  bool            operator!=(const gSpace & rhs) const;
  
  // information
  int             Dmnsn(void)              const;
  Variable*       VariableWithNumber(int)  const;
  const gText&  GetVariableName(int)     const;
  gSpace          WithVariableAppended()   const;

  // manipulation
  void            SetVariableName(int, const gText &);
  void            CreateVariables (int nvars = 1);

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

private:
  const gSpace* Space;
  gVector<int> components;

public:
  exp_vect(const gSpace*);
  exp_vect(const gSpace*, const int&, const int&);   // x_i^j
  exp_vect(const gSpace*, int*);
  exp_vect(const gSpace*, gVector<int>);
  exp_vect(const gSpace*, gArray<int>);
  exp_vect(const exp_vect*);
  exp_vect(const exp_vect&);
  ~exp_vect();

// Operators
  exp_vect& operator=(const exp_vect & RHS);

  int  operator[](int index)           const;

  bool operator==(const exp_vect& RHS) const;
  bool operator!=(const exp_vect& RHS) const;
  bool operator<=(const exp_vect& RHS) const;
  bool operator>=(const exp_vect& RHS) const;
  bool operator< (const exp_vect& RHS) const;
  bool operator> (const exp_vect& RHS) const;

  exp_vect  operator -  ()                 const;
  exp_vect  operator +  (const exp_vect &) const;
  exp_vect  operator -  (const exp_vect &) const;
  void      operator += (const exp_vect &);
  void      operator -= (const exp_vect &);

// Other operations
  exp_vect  LCM(const exp_vect &)                    const;
  exp_vect  WithVariableAppended(const gSpace*)      const;
  exp_vect  AfterZeroingOutExpOfVariable(int&)       const;
  exp_vect  AfterDecrementingExpOfVariable(int&)     const;

// Information
  int  Dmnsn()                                     const;
  bool IsPositive()                                const;
  bool IsNonnegative()                             const;
  bool IsConstant()                                const;
  bool IsUnivariate()                              const;
  int  SoleActiveVariable()                        const;
  int  TotalDegree()                               const;
  bool Divides(const exp_vect&)                    const;
  bool UsesDifferentVariablesThan(const exp_vect&) const;

// Manipulation
  void SetExp(int varno, int pow);
  void ToZero();

  friend gOutput& operator<<(gOutput&, const exp_vect&);
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
typedef  bool (*ORD_PTR)(const exp_vect &, const exp_vect &);
  bool lex(const exp_vect &, const exp_vect &);
  bool reverselex(const exp_vect &, const exp_vect &);
  bool deglex(const exp_vect &, const exp_vect &);
  bool reversedeglex(const exp_vect &, const exp_vect &);
  bool degrevlex(const exp_vect &, const exp_vect &);
  bool reversedegrevlex(const exp_vect &, const exp_vect &);

class term_order {
private:
  const gSpace* Space;
  ORD_PTR actual_order;

public:
  term_order(const gSpace*, ORD_PTR);
  term_order(const term_order &);
  ~term_order();

// Operators
  term_order& operator=(term_order & RHS);

  bool operator==(const term_order & RHS) const;
  bool operator!=(const term_order & RHS) const;

// Comparisons invoking the underlying order
  bool Less          (const exp_vect &, const exp_vect &) const;
  bool LessOrEqual   (const exp_vect &, const exp_vect &) const;
  bool Greater       (const exp_vect &, const exp_vect &) const;
  bool GreaterOrEqual(const exp_vect &, const exp_vect &) const;

// Manipulation and Information
  term_order WithVariableAppended(const gSpace*) const;
};

#endif  // PREPOLY_H
