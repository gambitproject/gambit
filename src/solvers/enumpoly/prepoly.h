//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/prepoly.h
// Declaration of supporting classes for polynomials
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

#include <cmath>
#include "gambit.h"

using namespace Gambit;

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

class gSpace {
public:
  struct Variable {
    std::string name;
    int number;
  };

  explicit gSpace(size_t nvars)
  {
    for (size_t i = 1; i <= nvars; i++) {
      m_variables.push_back({"n" + std::to_string(i), static_cast<int>(i)});
    }
  }
  gSpace(const gSpace &) = delete;
  ~gSpace() = default;
  gSpace &operator=(const gSpace &) = delete;
  const Variable &operator[](const int index) const { return m_variables[index]; }
  int Dmnsn() const { return m_variables.size(); }

private:
  Array<Variable> m_variables;
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
  const gSpace *Space;
  Vector<int> components;

public:
  explicit exp_vect(const gSpace *p) : Space(p), components(p->Dmnsn()) { components = 0; }
  // Construct x_i^j
  exp_vect(const gSpace *p, const int i, const int j) : Space(p), components(p->Dmnsn())
  {
    components = 0;
    components[i] = j;
  }
  exp_vect(const gSpace *space, const Vector<int> &exps) : Space(space), components(exps) {}
  exp_vect(const exp_vect &) = default;
  ~exp_vect() = default;

  // Operators
  exp_vect &operator=(const exp_vect &) = default;

  int operator[](int index) const { return components[index]; }

  bool operator==(const exp_vect &RHS) const
  {
    return Space == RHS.Space && components == RHS.components;
  }
  bool operator!=(const exp_vect &RHS) const
  {
    return Space != RHS.Space || components != RHS.components;
  }
  bool operator<=(const exp_vect &RHS) const;
  bool operator>=(const exp_vect &RHS) const;
  bool operator<(const exp_vect &RHS) const;
  bool operator>(const exp_vect &RHS) const;

  exp_vect operator+(const exp_vect &v) const
  {
    exp_vect tmp(*this);
    tmp.components += v.components;
    return tmp;
  }

  // Other operations
  exp_vect AfterZeroingOutExpOfVariable(const int varnumber) const
  {
    exp_vect tmp(*this);
    tmp.components[varnumber] = 0;
    return tmp;
  }
  exp_vect AfterDecrementingExpOfVariable(const int varnumber) const
  {
    exp_vect tmp(*this);
    tmp.components[varnumber]--;
    return tmp;
  }

  // Information
  int Dmnsn() const { return Space->Dmnsn(); }
  bool IsConstant() const
  {
    for (int i = 1; i <= Dmnsn(); i++) {
      if ((*this)[i] > 0) {
        return false;
      }
    }
    return true;
  }
  bool IsMultiaffine() const
  {
    for (int i = 1; i <= Dmnsn(); i++) {
      if ((*this)[i] > 1) {
        return false;
      }
    }
    return true;
  }
  int TotalDegree() const
  {
    int exp_sum = 0;
    for (int i = 1; i <= Dmnsn(); i++) {
      exp_sum += (*this)[i];
    }
    return exp_sum;
  }

  // Manipulation
  void ToZero()
  {
    for (int i = 1; i <= Dmnsn(); i++) {
      components[i] = 0;
    }
  }
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
typedef bool (*ORD_PTR)(const exp_vect &, const exp_vect &);
bool lex(const exp_vect &, const exp_vect &);
bool reverselex(const exp_vect &, const exp_vect &);
bool deglex(const exp_vect &, const exp_vect &);
bool reversedeglex(const exp_vect &, const exp_vect &);
bool degrevlex(const exp_vect &, const exp_vect &);
bool reversedegrevlex(const exp_vect &, const exp_vect &);

class term_order {
private:
  const gSpace *Space;
  ORD_PTR actual_order;

public:
  term_order(const gSpace *, ORD_PTR);
  term_order(const term_order &) = default;
  ~term_order() = default;

  // Operators
  term_order &operator=(const term_order &RHS);

  bool operator==(const term_order &RHS) const;
  bool operator!=(const term_order &RHS) const;

  // Comparisons invoking the underlying order
  bool Less(const exp_vect &, const exp_vect &) const;
  bool LessOrEqual(const exp_vect &, const exp_vect &) const;
  bool Greater(const exp_vect &, const exp_vect &) const;
  bool GreaterOrEqual(const exp_vect &, const exp_vect &) const;
};

#endif // PREPOLY_H
