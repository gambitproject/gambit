//
// FILE: gpoly.h  --  Declaration of gPoly data type
// 
// $Id$
//

#ifndef GPOLY_H
#define GPOLY_H

#include "monomial.h"
#include "poly.h"
#include "gsmatrix.h"

// These classes are used to store and mathematically manipulate polynomials.

//  **NOTE**
//  Every type T to be used needs a procedure to convert a gText coefficient
//  to the type T for the gText SOP input form and a procedure to convert 
//  the coefficient into a gText for the SOP output form.  


// *******************
//  gPoly declaration
// *******************
  
template <class T> class gPoly {
  
private:
  const gSpace*        Space;    // pointer to variable Space of space
  const term_order*    Order;
  gList<gMono<T> >     Terms;    // alternative implementation

  // used for gText parsing;
  int charnum;   char charc;   gText TheString;

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
  gPoly<T>         DivideByPolynomial(const gPoly<T> &den) const; 

  // The following is used to construct the translate of *this.
  gPoly<T> TranslateOfMono(const gMono<T>&, const gVector<T>&) const;
  gPoly<T> MonoInNewCoordinates(const gMono<T>&, 
				const gSquareMatrix<T>&)       const;


  //-----------------------------------------------
  // Going back and forth from gTexts to gPoly's
  //-----------------------------------------------

  // gText input parser functions
  void      String_Term(T          nega);
  T         String_Coeff(T       nega);
  int       String_GetPow(void);
  void      String_VarAndPow(gArray<int> &PowArray);
  void      GetChar();
  // Is the string a valid polynomial?
  bool      Check_String(const gText &Hold);

  //----------------------
  //   private friends
  //----------------------

  friend gPoly<T> operator*(const gPoly<T> &poly, const T val);
  friend gPoly<T> operator*(const T val, const gPoly<T> &poly);

public:

  //---------------------------
  // Construction, destruction:
  //---------------------------

  // Null gPoly constructor
  gPoly(const gSpace *, const term_order *);
  // Constructs a gPoly equal to the SOP representation in the gText
  gPoly(const gSpace *, const gText &, const term_order *);
  // Constructs a constant gPoly
  gPoly(const gSpace *, const T &, const term_order *);
  // Constructs a gPoly equal to another;
  gPoly(const gPoly<T> &);
  // Constructs a gPoly that is x_{var_no}^exp;
  gPoly(const gSpace *p, int var_no, int exp, const term_order *);
  // Constructs a gPoly that is the monomial coeff*vars^exps;
  gPoly(const gSpace *p, exp_vect exps, T coeff, const term_order *);
  // Constructs a gPoly with single monomial
  gPoly(const gSpace *p, const gMono<T>&, const term_order *);

  ~gPoly();

  //----------
  //Operators:
  //----------
  
  gPoly<T>& operator =  (const gPoly<T> &);
  gPoly<T>& operator =  (const gText &);  
                        //Set polynomial equal to the SOP form in the string
  gPoly<T>  operator -  ()                  const;
  gPoly<T>  operator -  (const gPoly<T> &) const;
  void      operator -= (const gPoly<T> &);
  gPoly<T>  operator +  (const gPoly<T> &) const;
  void      operator += (const gPoly<T> &);
  void      operator += (const T&);
  gPoly<T>  operator *  (const gPoly<T> &) const;
  void      operator *= (const gPoly<T> &);
  void      operator *= (const T&);
  gPoly<T>  operator /  (const T val)       const;// division by a constant
  gPoly<T>  operator /  (const gPoly<T> &) const;// division by a polynomial

  bool      operator == (const gPoly<T> &p)      const;
  bool      operator != (const gPoly<T> &p)      const;

  //-------------
  // Information:
  //-------------

  const gSpace*       GetSpace(void)                       const; 
  const term_order*   GetOrder(void)                       const; 
  int                 Dmnsn()                              const;
  bool                IsZero()                             const;
  int                 DegreeOfVar(int var_no)              const;
  int                 Degree()                             const;
  T                   GetCoef(const gArray<int> &Powers)   const;
  T                   GetCoef(const exp_vect &Powers)      const;
  gPoly<T>            LeadingCoefficient(int varnumber)    const;
  T                   NumLeadCoeff()                       const; // deg == 0
  bool                IsConstant()                         const;
  bool                IsMultiaffine()                      const;
  int                 UniqueActiveVariable()               const;
                      // returns 0 if constant, -1 if truly multivariate
  polynomial<T>       UnivariateEquivalent(int activar)    const;
                      // assumes UniqueActiveVariable() is true
  T                   Evaluate(const gArray<T> &values)    const;
  gPoly<T>           EvaluateOneVar(int varnumber, T val)  const;
  gPoly<T>           PartialDerivative(int varnumber)      const;
  int                No_Monomials()                        const;
  gList<exp_vect>    ExponentVectors()                     const;
  gList<gMono<T> >   MonomialList()                        const;

  gPoly<T>           TranslateOfPoly(const gVector<T>&)    const;
  gPoly<T>   PolyInNewCoordinates(const gSquareMatrix<T>&) const;
  T                  MaximalValueOfNonlinearPart(const T&) const;

  //--------------------
  // Term Order Concepts
  //--------------------

  exp_vect       LeadingPowerProduct(const term_order &)              const;
  T              LeadingCoefficient(const term_order &)               const;
  gPoly<T>       LeadingTerm(const term_order &)                      const;
  void           ToMonic(const term_order &)                               ;
  void           ReduceByDivisionAtExpV(const term_order &, 
					const gPoly<T> &, 
					const exp_vect &);
  void           ReduceByRepeatedDivision(const term_order &, 
					  const gPoly<T> &);
  gPoly<T>       S_Polynomial(const term_order &, const gPoly<T> &) const;

  //---------------
  // Printing Stuff
  //---------------

 // Print polynomial in SOP form
  friend gOutput &operator<< (gOutput &f, const gPoly<T> &y);
  friend gText &operator<< (gText &, const gPoly<T> &);
};

  //-------------
  // Conversion:
  //-------------

template <class T> gPoly<gDouble> TogDouble(const gPoly<T>&);
template <class T> gPoly<gDouble> NormalizationOfPoly(const gPoly<T>&);

// global multiply by scalar operators
template <class T> gPoly<T> operator*(const T val, const gPoly<T> &poly);
template <class T> gPoly<T> operator*(const gPoly<T> &poly, const T val);

// global add to scalar operators
template <class T> gPoly<T> operator+(const T val, const gPoly<T> &poly);
template <class T> gPoly<T> operator+(const gPoly<T> &poly, const T val);

template <class T> gText ToText(const gPoly<T> &p);

template <class T> gOutput &operator<< (gOutput &f, const gPoly<T> &y);

#endif //# GPOLY_H













