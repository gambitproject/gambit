//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of supporting classes for polynomials
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

#ifndef PREPOLY_H
#define PREPOLY_H

#include <math.h>
#include "base/base.h"
#include "math/rational.h"
#include "math/gvector.h"

/*
   The classes in this file are prior to the notion of a 
multivariate gbtPolyUni.  First of all, one needs a space which the
polynomials refer to.  This is given by the notion of a gbtPolySpace.
A gbtPolyUni is a sum of monomials, where each monomial is a
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
// gbtPolySpace declaration
// *************************

struct Variable  { 
  gbtText Name;
  int number;
};

class gbtPolySpace {
private:  
  gbtBlock < Variable * > Variables;

public:
  gbtPolySpace(int nvars = 0);
  gbtPolySpace(const gbtPolySpace &);
  ~gbtPolySpace();

  // operators
  gbtPolySpace&         operator=(const gbtPolySpace & rhs);

  Variable*       operator[](int)              const;
  bool            operator==(const gbtPolySpace & rhs) const;
  bool            operator!=(const gbtPolySpace & rhs) const;
  
  // information
  int             Dmnsn(void)              const;
  Variable*       VariableWithNumber(int)  const;
  const gbtText&  GetVariableName(int)     const;
  gbtPolySpace          WithVariableAppended()   const;

  // manipulation
  void            SetVariableName(int, const gbtText &);
  void            CreateVariables (int nvars = 1);

  void            Dump(gbtOutput &) const;  // Debugging output
};


// ***********************
// class gbtPolyExponent 
// ***********************

/*
   Exponent vectors are vectors of integers.  In specifying operators
   we take the view that addition, subtraction, and order are defined,
   but not inner or scalar products.
*/

class gbtPolyExponent {

private:
  const gbtPolySpace* Space;
  gbtVector<int> components;

public:
  gbtPolyExponent(const gbtPolySpace*);
  gbtPolyExponent(const gbtPolySpace*, const int&, const int&);   // x_i^j
  gbtPolyExponent(const gbtPolySpace*, int*);
  gbtPolyExponent(const gbtPolySpace*, gbtVector<int>);
  gbtPolyExponent(const gbtPolySpace*, gbtArray<int>);
  gbtPolyExponent(const gbtPolyExponent*);
  gbtPolyExponent(const gbtPolyExponent&);
  ~gbtPolyExponent();

// Operators
  gbtPolyExponent& operator=(const gbtPolyExponent & RHS);

  int  operator[](int index)           const;

  bool operator==(const gbtPolyExponent& RHS) const;
  bool operator!=(const gbtPolyExponent& RHS) const;
  bool operator<=(const gbtPolyExponent& RHS) const;
  bool operator>=(const gbtPolyExponent& RHS) const;
  bool operator< (const gbtPolyExponent& RHS) const;
  bool operator> (const gbtPolyExponent& RHS) const;

  gbtPolyExponent  operator -  ()                 const;
  gbtPolyExponent  operator +  (const gbtPolyExponent &) const;
  gbtPolyExponent  operator -  (const gbtPolyExponent &) const;
  void      operator += (const gbtPolyExponent &);
  void      operator -= (const gbtPolyExponent &);

// Other operations
  gbtPolyExponent  LCM(const gbtPolyExponent &)                    const;
  gbtPolyExponent  WithVariableAppended(const gbtPolySpace*)      const;
  gbtPolyExponent  AfterZeroingOutExpOfVariable(int&)       const;
  gbtPolyExponent  AfterDecrementingExpOfVariable(int&)     const;

// Information
  int  Dmnsn()                                     const;
  bool IsPositive()                                const;
  bool IsNonnegative()                             const;
  bool IsConstant()                                const;
  bool IsMultiaffine()                             const;
  bool IsUnivariate()                              const;
  int  SoleActiveVariable()                        const;
  int  TotalDegree()                               const;
  bool Divides(const gbtPolyExponent&)                    const;
  bool UsesDifferentVariablesThan(const gbtPolyExponent&) const;

// Manipulation
  void SetExp(int varno, int pow);
  void ToZero();

  friend gbtOutput& operator<<(gbtOutput&, const gbtPolyExponent&);
};


// ***********************
// class gbtPolyTermOrder
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
// class gbtPolyTermOrder  I WAS BAFFLED AS TO HOW TO HAVE A MEMBER THAT
// IS A POINTER-TO-OTHER-MEMBER-FUNCTION
typedef  bool (*ORD_PTR)(const gbtPolyExponent &, const gbtPolyExponent &);
  bool lex(const gbtPolyExponent &, const gbtPolyExponent &);
  bool reverselex(const gbtPolyExponent &, const gbtPolyExponent &);
  bool deglex(const gbtPolyExponent &, const gbtPolyExponent &);
  bool reversedeglex(const gbtPolyExponent &, const gbtPolyExponent &);
  bool degrevlex(const gbtPolyExponent &, const gbtPolyExponent &);
  bool reversedegrevlex(const gbtPolyExponent &, const gbtPolyExponent &);

class gbtPolyTermOrder {
private:
  const gbtPolySpace* Space;
  ORD_PTR actual_order;

public:
  gbtPolyTermOrder(const gbtPolySpace*, ORD_PTR);
  gbtPolyTermOrder(const gbtPolyTermOrder &);
  ~gbtPolyTermOrder();

// Operators
  gbtPolyTermOrder& operator=(gbtPolyTermOrder & RHS);

  bool operator==(const gbtPolyTermOrder & RHS) const;
  bool operator!=(const gbtPolyTermOrder & RHS) const;

// Comparisons invoking the underlying order
  bool Less          (const gbtPolyExponent &, const gbtPolyExponent &) const;
  bool LessOrEqual   (const gbtPolyExponent &, const gbtPolyExponent &) const;
  bool Greater       (const gbtPolyExponent &, const gbtPolyExponent &) const;
  bool GreaterOrEqual(const gbtPolyExponent &, const gbtPolyExponent &) const;

// Manipulation and Information
  gbtPolyTermOrder WithVariableAppended(const gbtPolySpace*) const;
};

#endif  // PREPOLY_H
