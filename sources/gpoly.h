//#
//# File: gpoly.h  --  Declaration of gPoly data type
//# $Id$
//#

#ifndef GPOLY_H
#define GPOLY_H

#include <math.h>
#include "gambitio.h"
#include "gstring.h"
#include "garray.h"
#include "glist.h"
#include "gblock.h"
#include "ufortify.hpp"
#include "ZFortify.hpp"

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

// ***********************
// gPolyFamily declaration
// ***********************

struct Variable{
  gString Name;
  int number;
};

template <class T> class gPolyFamily {
  friend class gPoly<T>;
private:
  
  int NoOfVars;

  // list of variables defined (numbered 1..N)  
  gBlock < Variable * > Variables;

  // list of polynomials in the family
  gList< gPoly <T> * > polys;



public:
  gPolyFamily(int nvars = 1);
  ~gPolyFamily();
  
  // Return number of variables defined
  int NumVariables(void) const;

  // Return name of ith variable, fail assertionif out of range
  const gString &GetVariableName(int i) const;

  // Set name of ith variables, fail assertion if out of range
  void SetVariableName(int i, const gString &);

  // Create new variables(s)
  void CreateVariables (int nvars = 1);

  // Debugging output
  void Dump(gOutput &) const;
};

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

// global multiply by constant operators

template <class T> gPoly<T> operator*(const T val, const gPoly<T> poly);
template <class T> gPoly<T> operator*(const gPoly<T> poly, const T val);


// actual gPoly class declaration.

template <class T> class gPoly {
  
private:  

  friend gPoly<T> operator*(const gPoly<T> poly, const T val);
  friend gPoly<T> operator*(const T val, const gPoly<T> poly);

  // used for gString parsing;
  int charnum;
  char charc;
  gString TheString;

  // root node of tree
  Power<T> *Root;

  // pointer to family of which this is member
  gPolyFamily<T> *Fam;

  //----------------------
  // some private members
  //----------------------

  // Removes a tree beginning at source
  void Remover(Power<T> *source);

  // Copies a tree begging at source
  Power<T> *CreatDupl( Power<T> *source );

  // locates a term based on the Powers in the garray
  Power<T> *Locator(const gArray<int> &Powers, int i, Power<T> *p) const;

  // returns the last non-zero integer value of the array
  int ZeroCheck(const gArray<int> &Powers) const;

  // prints the tree to a gString
  void printer(Power<T> *base, gString woCoef, gString &s, 
	       gPolyFamily<T> *Famil) const;

  // evaluates the tree based on the values of the array recursively
  void evaluator( const gArray<T> &values, Power<T> *, T woAns, T &) const;

  // recursive part of the EvaluateOneVar .
  Power<T> *EvalVar(int num, T val, Power<T> *start);

  // creates the rest of the tree if found results in null (for 
  // SetCoef and the string parser.)
  void CreateTree(const gArray<int> &Powers, int i, Power<T> **p, T val);

  // gets rid of unneccesary 0 power terms (for sparsity)
  void Collapsor(Power<T> *base, Power<T> *caller);

  // gets rid of terms with zeros in the Coefficient
  void ZeroCoefRemover(Power<T> *base, Power<T> *caller);

  // finds the lowest (farthest down) Power<T> with a power of zero.
  Power<T> *LowZero( Power<T> *start);

  // recursive tree adder.
  Power<T> *Adder( Power<T> *One, Power<T> *Two);

  // negates the coefficients of a tree.
  void Negate (Power<T> *start);

  // Multiplies all the coefficients of a tree by constants.
  void MultbyConst (T val, Power<T> *start);

  // Divide all the coefficients of a tree by constants.
  void DividebyConst(T val, Power<T> *start);

  // Recursive Equality checker
  int Equality( Power<T> *one, Power<T> *two);

  // recursive tree multiplier.
  Power<T> *Mult( Power<T> *one, Power<T> *two);

  // gString input parser functions
  void String_Term(T nega);
  int String_Coeff(int nega);
  double String_Coeff(double nega);
  gRational String_Coeff(gRational nega);
  int String_GetPow(void);
  void String_VarAndPow(gArray<int> &PowArray);
  void GetChar();

  // different instantiations of the GetCoefToString
  gString GetCoefToString (int val) const;
  gString GetCoefToString (double val) const;
  gString GetCoefToString (gRational val) const;

public:
  // Null gPoly constructor
  gPoly(gPolyFamily<T> *);

  // Constructs a gPoly equal to the SOP representation in the gString
  gPoly(gPolyFamily<T> *, const gString &);

  // Constructs a gPoly equal to another;
  gPoly(const gPoly<T> &);

  // destructor
  ~gPoly();

  //----------
  //Operators:
  //----------
  
  // sets two gPolys equal
  gPoly<T> &operator=(const gPoly<T> &);

  // negation of a gPoly
  gPoly<T> operator-();

  // subtration of gPolys
  gPoly<T> operator-(const gPoly<T> &);

  void operator-=(const gPoly<T> &);

  // addition of gPolys
  gPoly<T> operator+(const gPoly<T> &);

  void operator+=(const gPoly<T> &);

  // multiplication of gPolys
  gPoly<T> operator*(const gPoly<T> &);

  void operator*=(const gPoly<T> &);

  // division of a gPoly by a constant
  gPoly<T> operator/(const T val);
  
  // Returns true of two polys are equal
  int operator==(gPoly<T> &p);
  
  //Set polynomial equal to the SOP form in the string
  gPoly<T> &operator=(const gString &);

  //Returns a pointer to the gPolyFamily of a gPoly;
  gPolyFamily<T> *GetFamily(void) const;
  
  //Sets the coefficient of a term in a gPoly identified by the powers in
  // the gArray. The term is created if it does not exist already
  void SetCoef(const gArray<int> &Powers, const T &value);
  
  //Returns the Coefficient of the term identified by the gArray
  T GetCoef(const gArray<int> &Powers) const;
  
  //Evaluates a gPoly with the values in the gArray
  T Evaluate(const gArray<T> &values) const;
  
  //Evaluates the variable identified by varnumber with respect to the value
  // val.
  gPoly<T> EvaluateOneVar(int varnumber, T val);

  //List Reverser
  Power<T> *ReverseList( Power<T> *start);

  // Print polynomial in SOP form
  void Print(gOutput &) const;
  void Print(gString &) const;

  friend gOutput &operator<<(gOutput &f, const gPoly<T> &y);

};



#endif //# GPOLY_H













