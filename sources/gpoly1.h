//#
//# File: gpoly1.h  --  Declaration of gPoly data type
//# @(#)gpoly1.h	1.6 12/12/95
//#

#ifndef GPOLY1_H
#define GPOLY1_H

#include "monomial.h"

// These classes are used to store and mathematically manipulate polynomials.

//  **NOTE**
//  Every type T to be used needs a procedure to convert a gString coefficient
//  to the type T for the gString SOP input form and a procedure to convert 
//  the coefficient into a gString for the SOP output form.  


// template <class T> class gPoly1; 


// *******************
//  gPoly1 declaration
// *******************
  
template <class T> class gPoly1 {
  
private:
  const gSpace*        Space;    // pointer to variable Space of space
  const term_order*    Order;
  gList<gMono<T> >     Terms;    // alternative implementation

  // used for gString parsing;
  int charnum;   char charc;   gString TheString;

  //----------------------
  // some private members
  //----------------------

    // Information
  exp_vect  OrderMaxMonomialDivisibleBy(const term_order& order,
					const exp_vect& expv);

    // Arithmetic
  gList<gMono<T> > Adder(const gList<gMono<T> >&, 
			 const gList<gMono<T> >&)          const;
  gList<gMono<T> > Mult(const gList<gMono<T> >&, 
			const gList<gMono<T> >&)           const;
  gPoly1<T>        DivideByPolynomial(const gPoly1<T> den) const; 


  //-----------------------------------------------
  // Going back and forth from gStrings to gPoly1's
  //-----------------------------------------------

  // gString input parser functions
  void      String_Term(T          nega);
  int       String_Coeff(int       nega);
  long      String_Coeff(long      nega);
  double    String_Coeff(double    nega);
  gRational String_Coeff(gRational nega);
  int       String_GetPow(void);
  void      String_VarAndPow(gArray<int> &PowArray);
  void      GetChar();

  // different instantiations of the GetCoefToString
  gString GetCoefToString(int       val) const;
  gString GetCoefToString(long      val) const;
  gString GetCoefToString(double    val) const;
  gString GetCoefToString(gRational val) const;  

  //----------------------
  //   private friends
  //----------------------

  friend gPoly1<T> operator*(const gPoly1<T> poly, const T val);
  friend gPoly1<T> operator*(const T val, const gPoly1<T> poly);

public:

  //---------------------------
  // Construction, destruction:
  //---------------------------

  // Null gPoly1 constructor
  gPoly1(const gSpace *, const term_order *);
  // Constructs a gPoly1 equal to the SOP representation in the gString
  gPoly1(const gSpace *, const gString &, const term_order *);
  // Constructs a constant gPoly1
  gPoly1(const gSpace *, const T &, const term_order *);
  // Constructs a gPoly1 equal to another;
  gPoly1(const gPoly1<T> &);
  // Constructs a gPoly1 that is x_{var_no}^exp;
  gPoly1(const gSpace *p, int var_no, int exp, const term_order *);
  // Constructs a gPoly1 that is the monomial coeff*vars^exps;
  gPoly1(const gSpace *p, exp_vect exps, T coeff, const term_order *);
  // Constructs a gPoly1 with single monomial
  gPoly1(const gSpace *p, const gMono<T>&, const term_order *);

  ~gPoly1();

  //----------
  //Operators:
  //----------
  
  gPoly1<T>& operator =  (const gPoly1<T> &);
  gPoly1<T>& operator =  (const gString &);  
                        //Set polynomial equal to the SOP form in the string

  gPoly1<T>  operator -  ()                  const;
  gPoly1<T>  operator -  (const gPoly1<T> &) const;
  void       operator -= (const gPoly1<T> &);
  gPoly1<T>  operator +  (const gPoly1<T> &) const;
  void       operator += (const gPoly1<T> &);
  gPoly1<T>  operator *  (const gPoly1<T> &) const;
  void       operator *= (const gPoly1<T> &);
  void       operator *= (const T&);
  gPoly1<T>  operator /  (const T val)       const;// division by a constant
  gPoly1<T>  operator /  (const gPoly1<T> &) const;// division by a polynomial

  bool       operator == (gPoly1<T> &p)      const;
  bool       operator != (gPoly1<T> &p)      const;

  //-------------
  // Information:
  //-------------

  const gSpace*   GetSpace(void)                       const; 
  int             Dmnsn()                              const;
  int             DegreeOfVar(int var_no)              const;
  int             Degree()                             const;
  T               GetCoef(const gArray<int> &Powers)   const;
  T               GetCoef(const exp_vect &Powers)      const;
  gPoly1<T>       LeadingCoefficient(int varnumber)    const;
  T               NumLeadCoeff()                       const; //assume deg == 0
  T               Evaluate(const gArray<T> &values)    const;
  gPoly1<T>       EvaluateOneVar(int varnumber, T val) const;
  gPoly1<T>       PartialDerivative(int varnumber)     const;
  int             No_Monomials()                       const;
  gList<exp_vect> ExponentVectors()                    const;

  //--------------------
  // Term Order Concepts
  //--------------------

  exp_vect       LeadingPowerProduct(const term_order &)              const;
  T              LeadingCoefficient(const term_order &)               const;
  gPoly1<T>      LeadingTerm(const term_order &)                      const;
  void           ReduceByDivisionAtExpV(const term_order &, 
					const gPoly1<T> &, 
					const exp_vect &);
  void           ReduceByRepeatedDivision(const term_order &, 
					  const gPoly1<T> &);
  gPoly1<T>       S_Polynomial(const term_order &, const gPoly1<T> &) const;

  //---------------
  // Printing Stuff
  //---------------

 // Print polynomial in SOP form
  friend gOutput &operator<<(gOutput &f, const gPoly1<T> &y);
};


// global multiply by scalar operators
template <class T> gPoly1<T> operator*(const T val, const gPoly1<T> poly);
template <class T> gPoly1<T> operator*(const gPoly1<T> poly, const T val);


#endif //# GPOLY1_H













