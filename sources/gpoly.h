//#
//# File: gpoly.h  --  Declaration of gPoly data type
//# @(#)gpoly.h	1.6 12/12/95
//#

#ifndef GPOLY_H
#define GPOLY_H

#include "pre_poly.h"

// These classes are used to store and mathematically manipulate polynomials.
// 
//  To define a polynomial, first declare a gPolyFamily describing the 
//  the variables that will be used.  Mathematical operations of polynomials
//  are only possible if the two polynomials are of the same gPolyFamily.
//  It is then possible to create gPoly's for a family and use them in
//  calculations.
//  example:
//  gPolyFamily Fam(5);  creates a gPolyFamily named Fam of 5 variables
//         the default variable names are n1 - n5;
//  gPoly first(&Fam);  creates a polynomial belonging to the gPolyFamily
//         Fam, so it have terms any of the 5 variables of that family.


//  **NOTE**
//  Every type T to be used needs a procedure to convert a gString coefficient
//  to the type T for the gString SOP input form and a procedure to convert 
//  the coefficient into a gString for the SOP output form.  


template <class T> class gPoly; 


// *******************
//  gPoly declaration
// *******************
  
// structures declared outside of the class to allow recursing procedures  in 
// the class to return these structures.

template <class T> struct term {
  T Coef;
  Power<T> *down;
  Variable *Var;
};

template <class T> struct Power {
  term<T> *down;
  int pow;
  Power<T> *next;
};

               // actual gPoly class declaration.

template <class T> class gPoly {
  
private:

  Power<T>* Root;         // root node of tree
  const gVariableList* List;    // pointer to variable list of space

  // used for gString parsing;
  int charnum;   char charc;   gString TheString;

  //----------------------
  // some private members
  //----------------------

    // Construction, destruction, cleanliness
  void     CreateTree(const gArray<int> &Powers, int i, Power<T> **p, T val);
  Power<T> *CreatDupl( Power<T> *source ); // Copies a tree beginning at source
  void     Remover(Power<T> *source);   // Removes a tree beginning at source
  Power<T> *LowZero( Power<T> *start); // lowest Power<T> with a power of zero.
  void     Collapsor(Power<T> *base, Power<T> *caller); 
                    // erase needless 0 powers
  void     ZeroCoefRemover(Power<T> *base, Power<T> *caller);
                    // erase 0 coefficients
  const Power<T>* Locator(const exp_vect &Powers, const Power<T> *p) const;
  Power<T>* Locator(const gArray<int> &Powers, int i, Power<T> *p) const;
                    // locates a term based on the Powers 
  int      ZeroCheck(const gArray<int> &Powers) const;
                     // last non-zero integer value of the array
  Power<T> *ReverseList( Power<T> *start);

    // Information
  int      Equality(Power<T> *one, Power<T> *two) const;  
                    // Recursive Equality checker
  int      DegreeOfVar(int var_no, Power<T> *start) const;
  int      TotalDegree(Power<T> *start) const;
  void     count_monomials(Power<T> *base,int &count) const;
                    // the number of monomials in the polynomial

  void     find_more_exp_vecs(gList<exp_vect*>& cur_list,
			      exp_vect& cur_vect,
			      Power<T>* start)     const;

  bool     ExpVect(const Power<T> *here, 
		   const Power<T> *target, 
		   exp_vect& current) const;
  Power<T>* OrderMaxMonomialDivisibleBy(const term_order& order,
					const Power<T>* start,
					const exp_vect& expv);
  Power<T>* OrderMaxMonomialDivisibleBy(const term_order& order,
					const exp_vect& expv);

    // Arithmetic
  Power<T> *Adder( Power<T> *One, Power<T> *Two);   // recursive tree adder.
  void     Negate (Power<T> *start);
  void     MultbyConst (T val, Power<T> *start);
  Power<T> *Mult( Power<T> *one, Power<T> *two);
                    // recursive tree multiplier.
  void     DividebyConst(T val, Power<T> *start);
  gPoly<T> DivideByPolynomial(const gPoly<T> den) const;
  Power<T> *PrtlDrvtv(int num, Power<T> *start, int pow_of_var);
  Power<T> *LdngCffcnt(int num, int degree, Power<T> *start, bool keep);
  void     evaluator( const gArray<T> &values, Power<T> *, T woAns, T &) const;
  Power<T> *EvalVar(int num, T val, Power<T> *start);
                    // recursive part of the EvaluateOneVar
  exp_vect LeadingPowProd(const term_order &, const Power<T>*) const;
                    // recursive part of LeadingPowerProduct
  T        LeadingCoeff(const term_order &, const Power<T>*) const;
                    // recursive part of LeadingCoefficient

    // Going back and forth from gStrings to gPoly's
  // prints the tree to a gString
  void printer(Power<T> *base, gString woCoef, gString &s, 
	       const gVariableList* List) const;
  // gString input parser functions
  void String_Term(T nega);
  int String_Coeff(int nega);
  long String_Coeff(long nega);
  double String_Coeff(double nega);
  gRational String_Coeff(gRational nega);
  int String_GetPow(void);
  void String_VarAndPow(gArray<int> &PowArray);
  void GetChar();
  // different instantiations of the GetCoefToString
  gString GetCoefToString (int val) const;
  gString GetCoefToString (long val) const;
  gString GetCoefToString (double val) const;
  gString GetCoefToString (gRational val) const;  


//Private friends
  friend gPoly<T> operator*(const gPoly<T> poly, const T val);
  friend gPoly<T> operator*(const T val, const gPoly<T> poly);

public:

  //---------------------------
  // Construction, destruction:
  //---------------------------

  // Null gPoly constructor
  gPoly(const gVariableList *);
  // Constructs a gPoly equal to the SOP representation in the gString
  gPoly(const gVariableList *, const gString &);
  // Constructs a constant gPoly
  gPoly(const gVariableList *, const T &);
  // Constructs a gPoly equal to another;
  gPoly(const gPoly<T> &);
  // Constructs a gPoly that is x_{var_no}^exp;
  gPoly(const gVariableList *p, int var_no, int exp);
  // Constructs a gPoly that is the monomial coeff*vars^exps;
  gPoly(const gVariableList *p, exp_vect exps, T coeff);

  ~gPoly();
  
  //Sets the coefficient of a term in a gPoly identified by the powers in
  // the gArray. The term is created if it does not exist already
  void SetCoef(const gArray<int> &Powers, const T &value);

  //----------
  //Operators:
  //----------
  
  gPoly<T> &operator =  (const gPoly<T> &);
  gPoly<T>  operator -  ()                 const;
  gPoly<T>  operator -  (const gPoly<T> &) const;
  void      operator -= (const gPoly<T> &);
  gPoly<T>  operator +  (const gPoly<T> &) const;
  void      operator += (const gPoly<T> &);
  gPoly<T>  operator *  (const gPoly<T> &) const;
  void      operator *= (const gPoly<T> &);
  gPoly<T>  operator /  (const T val)      const;  // division by a constant
  gPoly<T>  operator /  (const gPoly<T> &) const;  // division by a polynomial
  int       operator == (gPoly<T> &p)      const;
  int       operator != (gPoly<T> &p)      const;
  gPoly<T> &operator =  (const gString &);  
                        //Set polynomial equal to the SOP form in the string

  //-------------
  // Information:
  //-------------

  gVariableList* GetList(void) const; //Returns pointer to the List
  int            DegreeOfVar(int var_no) const;
  int            TotalDegree() const;
  T              GetCoef(const gArray<int> &Powers) const;
  T              GetCoef(const exp_vect &Powers) const;
  gPoly<T>       LeadingCoefficient(int varnumber);
  T              NumLeadCoeff() const;       // assumes Degree() == 0
  T              Evaluate(const gArray<T> &values) const;
  gPoly<T>       EvaluateOneVar(int varnumber, T val);
  gPoly<T>       PartialDerivative(int varnumber);
  int            No_Monomials() const;
  gList<exp_vect*> ExponentVectors() const;
  exp_vect       ExpVect(const Power<T> *target) const;

  //--------------------
  // Term Order Concepts
  //--------------------

  exp_vect       LeadingPowerProduct(const term_order &) const;
  T              LeadingCoefficient(const term_order &) const;
  gPoly<T>       LeadingTerm(const term_order &) const;
  void           ReduceByDivisionAtExpV(const term_order &, 
					const gPoly<T> &, 
					const exp_vect &);
  void           ReduceByRepeatedDivision(const term_order &, 
					  const gPoly<T> &);
  gPoly<T>       S_Polynomial(const term_order &, 
			      const gPoly<T> &) const;

  //---------------
  // Printing Stuff
  //---------------

 // Print polynomial in SOP form
  void Print(gOutput &) const;
  void Print(gString &) const;

  friend gOutput &operator<<(gOutput &f, const gPoly<T> &y);
};


// global multiply by scalar operators
template <class T> gPoly<T> operator*(const T val, const gPoly<T> poly);
template <class T> gPoly<T> operator*(const gPoly<T> poly, const T val);


#endif //# GPOLY_H













