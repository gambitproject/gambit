//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of multivariate gbtPolyUni type
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

#ifndef GPOLY_H
#define GPOLY_H

#include "math/gsmatrix.h"
#include "monomial.h"
#include "poly.h"

// These classes are used to store and mathematically manipulate polynomials.

//  **NOTE**
//  Every type T to be used needs a procedure to convert a std::string coefficient
//  to the type T for the std::string SOP input form and a procedure to convert 
//  the coefficient into a std::string for the SOP output form.  


// *******************
//  gbtPolyMulti declaration
// *******************
  
template <class T> class gbtPolyMulti {
  
private:
  const gbtPolySpace*        Space;    // pointer to variable Space of space
  const gbtPolyTermOrder*    Order;
  gbtList<gbtMonomial<T> >     Terms;    // alternative implementation

  // used for std::string parsing;
  unsigned int charnum;   char charc;   std::string TheString;

  //----------------------
  // some private members
  //----------------------

    // Information
  gbtPolyExponent  OrderMaxMonomialDivisibleBy(const gbtPolyTermOrder& order,
					const gbtPolyExponent& expv);
    // Arithmetic
  gbtList<gbtMonomial<T> > Adder(const gbtList<gbtMonomial<T> >&, 
			 const gbtList<gbtMonomial<T> >&)          const;
  gbtList<gbtMonomial<T> > Mult(const gbtList<gbtMonomial<T> >&, 
			const gbtList<gbtMonomial<T> >&)           const;
  gbtPolyMulti<T>         DivideByPolynomial(const gbtPolyMulti<T> &den) const; 

  // The following is used to construct the translate of *this.
  gbtPolyMulti<T> TranslateOfMono(const gbtMonomial<T>&, const gbtVector<T>&) const;
  gbtPolyMulti<T> MonoInNewCoordinates(const gbtMonomial<T>&, 
				const gbtSquareMatrix<T>&)       const;


  //-----------------------------------------------
  // Going back and forth from std::strings to gbtPolyMulti's
  //-----------------------------------------------

  // std::string input parser functions
  void      String_Term(T          nega);
  T         String_Coeff(T       nega);
  int       String_GetPow(void);
  void      String_VarAndPow(gbtArray<int> &PowArray);
  void      GetChar();
  // Is the string a valid gbtPolyUni?
  bool      Check_String(const std::string &Hold);

  //----------------------
  //   private friends
  //----------------------

  //  friend gbtPolyMulti<T> operator*<>(const gbtPolyMulti<T> &poly, const T val);
  //  friend gbtPolyMulti<T> operator*(const T val, const gbtPolyMulti<T> &poly);

public:

  //---------------------------
  // Construction, destruction:
  //---------------------------

  // Null gbtPolyMulti constructor
  gbtPolyMulti(const gbtPolySpace *, const gbtPolyTermOrder *);
  // Constructs a gbtPolyMulti equal to the SOP representation in the std::string
  gbtPolyMulti(const gbtPolySpace *, const std::string &, const gbtPolyTermOrder *);
  // Constructs a constant gbtPolyMulti
  gbtPolyMulti(const gbtPolySpace *, const T &, const gbtPolyTermOrder *);
  // Constructs a gbtPolyMulti equal to another;
  gbtPolyMulti(const gbtPolyMulti<T> &);
  // Constructs a gbtPolyMulti that is x_{var_no}^exp;
  gbtPolyMulti(const gbtPolySpace *p, int var_no, int exp, const gbtPolyTermOrder *);
  // Constructs a gbtPolyMulti that is the monomial coeff*vars^exps;
  gbtPolyMulti(const gbtPolySpace *p, gbtPolyExponent exps, T coeff, const gbtPolyTermOrder *);
  // Constructs a gbtPolyMulti with single monomial
  gbtPolyMulti(const gbtPolySpace *p, const gbtMonomial<T>&, const gbtPolyTermOrder *);

  ~gbtPolyMulti();

  //----------
  //Operators:
  //----------
  
  gbtPolyMulti<T>& operator =  (const gbtPolyMulti<T> &);
  gbtPolyMulti<T>& operator =  (const std::string &);  
                        //Set gbtPolyUni equal to the SOP form in the string
  gbtPolyMulti<T>  operator -  ()                  const;
  gbtPolyMulti<T>  operator -  (const gbtPolyMulti<T> &) const;
  void      operator -= (const gbtPolyMulti<T> &);
  gbtPolyMulti<T>  operator +  (const gbtPolyMulti<T> &) const;
  void      operator += (const gbtPolyMulti<T> &);
  void      operator += (const T&);
  gbtPolyMulti<T>  operator *  (const gbtPolyMulti<T> &) const;
  void      operator *= (const gbtPolyMulti<T> &);
  void      operator *= (const T&);
  gbtPolyMulti<T>  operator /  (const T val)       const;// division by a constant
  gbtPolyMulti<T>  operator /  (const gbtPolyMulti<T> &) const;// division by a gbtPolyUni

  bool      operator == (const gbtPolyMulti<T> &p)      const;
  bool      operator != (const gbtPolyMulti<T> &p)      const;

  //-------------
  // Information:
  //-------------

  const gbtPolySpace*       GetSpace(void)                       const; 
  const gbtPolyTermOrder*   GetOrder(void)                       const; 
  int                 Dmnsn()                              const;
  bool                IsZero()                             const;
  int                 DegreeOfVar(int var_no)              const;
  int                 Degree()                             const;
  T                   GetCoef(const gbtArray<int> &Powers)   const;
  T                   GetCoef(const gbtPolyExponent &Powers)      const;
  gbtPolyMulti<T>            LeadingCoefficient(int varnumber)    const;
  T                   NumLeadCoeff()                       const; // deg == 0
  bool                IsConstant()                         const;
  bool                IsMultiaffine()                      const;
  int                 UniqueActiveVariable()               const;
                      // returns 0 if constant, -1 if truly multivariate
  gbtPolyUni<T>       UnivariateEquivalent(int activar)    const;
                      // assumes UniqueActiveVariable() is true
  T                   Evaluate(const gbtArray<T> &values)    const;
  gbtPolyMulti<T>           EvaluateOneVar(int varnumber, T val)  const;
  gbtPolyMulti<T>           PartialDerivative(int varnumber)      const;
  int                No_Monomials()                        const;
  gbtList<gbtPolyExponent>    ExponentVectors()                     const;
  gbtList<gbtMonomial<T> >   MonomialList()                        const;

  gbtPolyMulti<T>           TranslateOfPoly(const gbtVector<T>&)    const;
  gbtPolyMulti<T>   PolyInNewCoordinates(const gbtSquareMatrix<T>&) const;
  T                  MaximalValueOfNonlinearPart(const T&) const;

  //--------------------
  // Term Order Concepts
  //--------------------

  gbtPolyExponent       LeadingPowerProduct(const gbtPolyTermOrder &)              const;
  T              LeadingCoefficient(const gbtPolyTermOrder &)               const;
  gbtPolyMulti<T>       LeadingTerm(const gbtPolyTermOrder &)                      const;
  void           ToMonic(const gbtPolyTermOrder &)                               ;
  void           ReduceByDivisionAtExpV(const gbtPolyTermOrder &, 
					const gbtPolyMulti<T> &, 
					const gbtPolyExponent &);
  void           ReduceByRepeatedDivision(const gbtPolyTermOrder &, 
					  const gbtPolyMulti<T> &);
  gbtPolyMulti<T>       S_Polynomial(const gbtPolyTermOrder &, const gbtPolyMulti<T> &) const;

  //---------------
  // Printing Stuff
  //---------------

 // Print gbtPolyUni in SOP form
  void Output(std::string &) const;
};

template <class T> std::string &operator<< (std::string &, const gbtPolyMulti<T> &);
template <class T> std::ostream &operator<< (std::ostream &f, const gbtPolyMulti<T> &y);

  //-------------
  // Conversion:
  //-------------

template <class T> gbtPolyMulti<double> NormalizationOfPoly(const gbtPolyMulti<T>&);

// global multiply by scalar operators
template <class T> gbtPolyMulti<T> operator*(const T val, const gbtPolyMulti<T> &poly);
template <class T> gbtPolyMulti<T> operator*(const gbtPolyMulti<T> &poly, const T val);

// global add to scalar operators
template <class T> gbtPolyMulti<T> operator+(const T val, const gbtPolyMulti<T> &poly);
template <class T> gbtPolyMulti<T> operator+(const gbtPolyMulti<T> &poly, const T val);

template <class T> std::string ToText(const gbtPolyMulti<T> &p);

template <class T> std::ostream &operator<< (std::ostream &f, const gbtPolyMulti<T> &y);

#endif //# GPOLY_H
