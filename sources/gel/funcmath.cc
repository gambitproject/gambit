//
// FILE: funcmath.cc -- Arithmetic functions for GEL
//
// $Id$
//

#include "gmisc.h"
#include "exprtree.h"
#include "funcmisc.h"
#include "glist.h"
#include "tristate.h"


//-------
// And
//-------

DECLARE_BINARY(gelfuncAnd, gTriState *, gTriState *, gTriState *)

gTriState *gelfuncAnd::EvalItem(gTriState *x1, gTriState *x2) const
{
  if (!x1 || !x2)   {
    if (x1)  delete x1;
    if (x2)  delete x2;
    return 0;
  }
  *x1 = TriStateAnd(*x1, *x2);
  delete x2;
  return x1;
}

//----------
// Divide
//----------

DECLARE_BINARY(gelfuncDivide, gNumber *, gNumber *, gNumber *)

gNumber *gelfuncDivide::EvalItem(gNumber *x1, gNumber *x2) const
{
  if (!x1 || !x2)   {
    if (x1)  delete x1;
    if (x2)  delete x2;
    return 0;
  }
  *x1 = *x1 / *x2;
  delete x2;
  return x1;
}


//----------
// Equal
//----------

DECLARE_BINARY(gelfuncEqualBoolean, gTriState *, gTriState *, gTriState *)

gTriState *gelfuncEqualBoolean::EvalItem(gTriState *x1, gTriState *x2) const
{
  if (!x1 || !x2)   {
    if (x1)  delete x1;
    if (x2)  delete x2;
    return 0;
  }
  gTriState *ret = new gTriState((*x1 == *x2) ? triTRUE : triFALSE);
  delete x1;
  delete x2;
  return ret;
}

DECLARE_BINARY(gelfuncEqualNumber, gNumber *, gNumber *, gTriState *)

gTriState *gelfuncEqualNumber::EvalItem(gNumber *x1, gNumber *x2) const
{
  if (!x1 || !x2)   {
    if (x1)  delete x1;
    if (x2)  delete x2;
    return 0;
  }
  gTriState *ret = new gTriState((*x1 == *x2) ? triTRUE : triFALSE);
  delete x1;
  delete x2;
  return ret;
}

DECLARE_BINARY(gelfuncEqualText, gText *, gText *, gTriState *)

gTriState *gelfuncEqualText::EvalItem(gText *x1, gText *x2) const
{
  if (!x1 || !x2)   {
    if (x1)  delete x1;
    if (x2)  delete x2;
    return 0;
  }
  gTriState *ret = new gTriState((*x1 == *x2) ? triTRUE : triFALSE);
  delete x1;
  delete x2;
  return ret;
}

//-----------
// Greater
//-----------

DECLARE_BINARY(gelfuncGreaterNumber, gNumber *, gNumber *, gTriState *)

gTriState *gelfuncGreaterNumber::EvalItem(gNumber *x1, gNumber *x2) const
{
  if (!x1 || !x2)   {
    if (x1)  delete x1;
    if (x2)  delete x2;
    return 0;
  }
  gTriState *ret = new gTriState((*x1 > *x2) ? triTRUE : triFALSE);
  delete x1;
  delete x2;
  return ret;
}

DECLARE_BINARY(gelfuncGreaterText, gText *, gText *, gTriState *)

gTriState *gelfuncGreaterText::EvalItem(gText *x1, gText *x2) const
{
  if (!x1 || !x2)   {
    if (x1)  delete x1;
    if (x2)  delete x2;
    return 0;
  }
  gTriState *ret = new gTriState((*x1 > *x2) ? triTRUE : triFALSE);
  delete x1;
  delete x2;
  return ret;
}

//---------------
// GreaterEqual
//---------------

DECLARE_BINARY(gelfuncGreaterEqualNumber, gNumber *, gNumber *, gTriState *)

gTriState *gelfuncGreaterEqualNumber::EvalItem(gNumber *x1, gNumber *x2) const
{
  if (!x1 || !x2)   {
    if (x1)  delete x1;
    if (x2)  delete x2;
    return 0;
  }
  gTriState *ret = new gTriState((*x1 >= *x2) ? triTRUE : triFALSE);
  delete x1;
  delete x2;
  return ret;
}

DECLARE_BINARY(gelfuncGreaterEqualText, gText *, gText *, gTriState *)

gTriState *gelfuncGreaterEqualText::EvalItem(gText *x1, gText *x2) const
{
  if (!x1 || !x2)   {
    if (x1)  delete x1;
    if (x2)  delete x2;
    return 0;
  }
  gTriState *ret = new gTriState((*x1 >= *x2) ? triTRUE : triFALSE);
  delete x1;
  delete x2;
  return ret;
}

//--------
// Less
//--------

DECLARE_BINARY(gelfuncLessNumber, gNumber *, gNumber *, gTriState *)

gTriState *gelfuncLessNumber::EvalItem(gNumber *x1, gNumber *x2) const
{
  if (!x1 || !x2)   {
    if (x1)  delete x1;
    if (x2)  delete x2;
    return 0;
  }
  gTriState *ret = new gTriState((*x1 < *x2) ? triTRUE : triFALSE);
  delete x1;
  delete x2;
  return ret;
}

DECLARE_BINARY(gelfuncLessText, gText *, gText *, gTriState *)

gTriState *gelfuncLessText::EvalItem(gText *x1, gText *x2) const
{
  if (!x1 || !x2)   {
    if (x1)  delete x1;
    if (x2)  delete x2;
    return 0;
  }
  gTriState *ret = new gTriState((*x1 < *x2) ? triTRUE : triFALSE);
  delete x1;
  delete x2;
  return ret;
}

//------------
// LessEqual
//------------

DECLARE_BINARY(gelfuncLessEqualNumber, gNumber *, gNumber *, gTriState *)

gTriState *gelfuncLessEqualNumber::EvalItem(gNumber *x1, gNumber *x2) const
{
  if (!x1 || !x2)   {
    if (x1)  delete x1;
    if (x2)  delete x2;
    return 0;
  }
  gTriState *ret = new gTriState((*x1 <= *x2) ? triTRUE : triFALSE);
  delete x1;
  delete x2;
  return ret;
}

DECLARE_BINARY(gelfuncLessEqualText, gText *, gText *, gTriState *)

gTriState *gelfuncLessEqualText::EvalItem(gText *x1, gText *x2) const
{
  if (!x1 || !x2)   {
    if (x1)  delete x1;
    if (x2)  delete x2;
    return 0;
  }
  gTriState *ret = new gTriState((*x1 <= *x2) ? triTRUE : triFALSE);
  delete x1;
  delete x2;
  return ret;
}

//---------
// Minus
//---------

DECLARE_BINARY(gelfuncMinus, gNumber *, gNumber *, gNumber *)

gNumber *gelfuncMinus::EvalItem(gNumber *x1, gNumber *x2) const
{
  if (!x1 || !x2)   {
    if (x1)  delete x1;
    if (x2)  delete x2;
    return 0;
  }
  *x1 = *x1 - *x2;
  delete x2;
  return x1;
}

//----------
// Negate
//----------

DECLARE_UNARY(gelfuncNegate, gNumber *, gNumber *)

gNumber *gelfuncNegate::EvalItem(gNumber *x1) const
{
  return (x1) ? &(*x1 = -*x1) : 0;
}

//--------
// Not
//--------

DECLARE_UNARY(gelfuncNot, gTriState *, gTriState *)

gTriState *gelfuncNot::EvalItem(gTriState *x1) const
{
  return (x1) ? &(*x1 = TriStateNot(*x1)) : 0;
}

//------------
// NotEqual
//------------

DECLARE_BINARY(gelfuncNotEqualBoolean, gTriState *, gTriState *, gTriState *)

gTriState *gelfuncNotEqualBoolean::EvalItem(gTriState *x1, gTriState *x2) const
{
  if (!x1 || !x2)   {
    if (x1)  delete x1;
    if (x2)  delete x2;
    return 0;
  }
  gTriState *ret = new gTriState((*x1 != *x2) ? triTRUE : triFALSE);
  delete x1;
  delete x2;
  return ret;
}

DECLARE_BINARY(gelfuncNotEqualNumber, gNumber *, gNumber *, gTriState *)

gTriState *gelfuncNotEqualNumber::EvalItem(gNumber *x1, gNumber *x2) const
{
  if (!x1 || !x2)   {
    if (x1)  delete x1;
    if (x2)  delete x2;
    return 0;
  }
  gTriState *ret = new gTriState((*x1 != *x2) ? triTRUE : triFALSE);
  delete x1;
  delete x2;
  return ret;
}

DECLARE_BINARY(gelfuncNotEqualText, gText *, gText *, gTriState *)

gTriState *gelfuncNotEqualText::EvalItem(gText *x1, gText *x2) const
{
  if (!x1 || !x2)   {
    if (x1)  delete x1;
    if (x2)  delete x2;
    return 0;
  }
  gTriState *ret = new gTriState((*x1 != *x2) ? triTRUE : triFALSE);
  delete x1;
  delete x2;
  return ret;
}


//--------------
// NthElement
//--------------

DECLARE_BINARY(gelfuncNthElement, gText *, gNumber *, gText *)

class gelExceptionNonInteger : public gException  {
  public:
    virtual ~gelExceptionNonInteger()   { }
    gText Description(void) const  { return "Expected integer index"; }
};

gText *gelfuncNthElement::EvalItem(gText *x1, gNumber *x2) const
{
  if (!x1 || !x2)    {
    if (x1)  delete x1;
    if (x2)  delete x2;
    return 0;
  }

  if (!x2->IsInteger())
    throw gelExceptionNonInteger();
  gText *ret = new gText((*x1)[*x2 - gNumber(1)]);
  delete x1;
  delete x2;
  return ret;
}

//------------
// NumChars
//------------

DECLARE_UNARY(gelfuncNumChars, gText *, gNumber *)

gNumber *gelfuncNumChars::EvalItem(gText *s) const
{
  if (!s)  return 0;
  gNumber *ret = new gNumber(s->Length());
  delete s;
  return ret;
}

//------
// Or
//------

DECLARE_BINARY(gelfuncOr, gTriState *, gTriState *, gTriState *)

gTriState *gelfuncOr::EvalItem(gTriState *x1, gTriState *x2) const
{
  if (!x1 || !x2)   {
    if (x1)  delete x1;
    if (x2)  delete x2;
    return 0;
  }
  *x1 = TriStateOr(*x1, *x2);
  delete x2;
  return x1;
}

//-------
// Plus
//-------

DECLARE_BINARY(gelfuncPlus, gNumber *, gNumber *, gNumber *)

gNumber *gelfuncPlus::EvalItem(gNumber *x1, gNumber *x2) const
{
  if (!x1 || !x2)   {
    if (x1)  delete x1;
    if (x2)  delete x2;
    return 0;
  }
  *x1 = *x1 + *x2;
  delete x2;
  return x1;
}


//--------
// Times
//--------

DECLARE_BINARY(gelfuncTimes, gNumber *, gNumber *, gNumber *)

gNumber *gelfuncTimes::EvalItem(gNumber *x1, gNumber *x2) const
{
  if (!x1 || !x2)   {
    if (x1)  delete x1;
    if (x2)  delete x2;
    return 0;
  }
  *x1 = *x1 * *x2;
  delete x2;
  return x1;
}



gelExpr *GEL_And(const gArray<gelExpr *> &params)
{
  return new gelfuncAnd((gelExpression<gTriState *> *) params[1],
			(gelExpression<gTriState *> *) params[2]);
}

gelExpr *GEL_Divide(const gArray<gelExpr *> &params)
{
  return new gelfuncDivide((gelExpression<gNumber *> *) params[1],
			   (gelExpression<gNumber *> *) params[2]);
}

gelExpr *GEL_EqualBoolean(const gArray<gelExpr *> &params)
{
  return new gelfuncEqualBoolean((gelExpression<gTriState *> *) params[1],
				 (gelExpression<gTriState *> *) params[2]);
}

gelExpr *GEL_EqualNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncEqualNumber((gelExpression<gNumber *> *) params[1],
				(gelExpression<gNumber *> *) params[2]);
}

gelExpr *GEL_EqualText(const gArray<gelExpr *> &params)
{
  return new gelfuncEqualText((gelExpression<gText *> *) params[1],
			      (gelExpression<gText *> *) params[2]);
}

gelExpr *GEL_GreaterNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncGreaterNumber((gelExpression<gNumber *> *) params[1],
				  (gelExpression<gNumber *> *) params[2]);
}

gelExpr *GEL_GreaterText(const gArray<gelExpr *> &params)
{
  return new gelfuncGreaterText((gelExpression<gText *> *) params[1],
				(gelExpression<gText *> *) params[2]);
}

gelExpr *GEL_GreaterEqualNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncGreaterEqualNumber((gelExpression<gNumber *> *) params[1],
				       (gelExpression<gNumber *> *) params[2]);
}

gelExpr *GEL_GreaterEqualText(const gArray<gelExpr *> &params)
{
  return new gelfuncGreaterEqualText((gelExpression<gText *> *) params[1],
				     (gelExpression<gText *> *) params[2]);
}

gelExpr *GEL_LessNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncLessNumber((gelExpression<gNumber *> *) params[1],
			       (gelExpression<gNumber *> *) params[2]);
}

gelExpr *GEL_LessText(const gArray<gelExpr *> &params)
{
  return new gelfuncLessText((gelExpression<gText *> *) params[1],
			     (gelExpression<gText *> *) params[2]);
}

gelExpr *GEL_LessEqualNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncLessEqualNumber((gelExpression<gNumber *> *) params[1],
				    (gelExpression<gNumber *> *) params[2]);
}

gelExpr *GEL_LessEqualText(const gArray<gelExpr *> &params)
{
  return new gelfuncLessEqualText((gelExpression<gText *> *) params[1],
				  (gelExpression<gText *> *) params[2]);
}


gelExpr *GEL_Minus(const gArray<gelExpr *> &params)
{
  return new gelfuncMinus((gelExpression<gNumber *> *) params[1],
			  (gelExpression<gNumber *> *) params[2]);
}

gelExpr *GEL_Negate(const gArray<gelExpr *> &params)
{
  return new gelfuncNegate((gelExpression<gNumber *> *) params[1]);
}

gelExpr *GEL_Not(const gArray<gelExpr *> &params)
{
  return new gelfuncNot((gelExpression<gTriState *> *) params[1]);
}

gelExpr *GEL_NotEqualBoolean(const gArray<gelExpr *> &params)
{
  return new gelfuncNotEqualBoolean((gelExpression<gTriState *> *) params[1],
				    (gelExpression<gTriState *> *) params[2]);
}

gelExpr *GEL_NotEqualNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncNotEqualNumber((gelExpression<gNumber *> *) params[1],
				   (gelExpression<gNumber *> *) params[2]);
}

gelExpr *GEL_NotEqualText(const gArray<gelExpr *> &params)
{
  return new gelfuncNotEqualText((gelExpression<gText *> *) params[1],
				 (gelExpression<gText *> *) params[2]);
}

gelExpr *GEL_NthElement(const gArray<gelExpr *> &params)
{
  return new gelfuncNthElement((gelExpression<gText *> *) params[1],
			       (gelExpression<gNumber *> *) params[2]);
}

gelExpr *GEL_NumChars(const gArray<gelExpr *> &params)
{
  return new gelfuncNumChars((gelExpression<gText *> *) params[1]);
}

gelExpr *GEL_Or(const gArray<gelExpr *> &params)
{
  return new gelfuncOr((gelExpression<gTriState *> *) params[1],
		       (gelExpression<gTriState *> *) params[2]);
}

gelExpr *GEL_Plus(const gArray<gelExpr *> &params)
{
  return new gelfuncPlus((gelExpression<gNumber *> *) params[1],
			 (gelExpression<gNumber *> *) params[2]);
}


gelExpr *GEL_Times(const gArray<gelExpr *> &params)
{
  return new gelfuncTimes((gelExpression<gNumber *> *) params[1],
			  (gelExpression<gNumber *> *) params[2]);
}

#include "match.h"

void gelMathInit(gelEnvironment *env)
{
  struct  { gelAdapter *func; char *sig; }  sigarray[] = {
    { GEL_And, "And[x->BOOLEAN, y->BOOLEAN] =: BOOLEAN" },
    { GEL_Divide, "Divide[x->NUMBER, y->NUMBER] =: NUMBER" },
    { GEL_EqualBoolean, "Equal[x->BOOLEAN, y->BOOLEAN] =: BOOLEAN" },
    { GEL_EqualNumber, "Equal[x->NUMBER, y->NUMBER] =: BOOLEAN" },
    { GEL_EqualText, "Equal[x->TEXT, y->TEXT] =: BOOLEAN" },
    { GEL_GreaterNumber, "Greater[x->NUMBER, y->NUMBER] =: BOOLEAN" },
    { GEL_GreaterText, "Greater[x->TEXT, y->TEXT] =: BOOLEAN" },
    { GEL_GreaterEqualNumber, "GreaterEqual[x->NUMBER, y->NUMBER] =: BOOLEAN" },
    { GEL_GreaterEqualText, "GreaterEqual[x->TEXT, y->TEXT] =: BOOLEAN" },
    { GEL_LessNumber, "Less[x->NUMBER, y->NUMBER] =: BOOLEAN" },
    { GEL_LessText, "Less[x->TEXT, y->TEXT] =: BOOLEAN" },
    { GEL_LessEqualNumber, "LessEqual[x->NUMBER, y->NUMBER] =: BOOLEAN" },
    { GEL_LessEqualText, "LessEqual[x->TEXT, y->TEXT] =: BOOLEAN" },
    { GEL_Minus, "Minus[x->NUMBER, y->NUMBER] =: NUMBER" },
    { GEL_Negate, "Negate[x->NUMBER] =: BOOLEAN" },
    { GEL_Not, "Not[x->BOOLEAN] =: BOOLEAN" },
    { GEL_NotEqualBoolean, "NotEqual[x->BOOLEAN, y->BOOLEAN] =: BOOLEAN" },
    { GEL_NotEqualNumber, "NotEqual[x->NUMBER, y->NUMBER] =: BOOLEAN" },
    { GEL_NotEqualText, "NotEqual[x->TEXT, y->TEXT] =: BOOLEAN" },
    { GEL_NthElement, "NthElement[x->TEXT, y->NUMBER] =: TEXT" },
    { GEL_NumChars, "NumChars[x->TEXT] =: NUMBER" },
    { GEL_Or, "Or[x->BOOLEAN, y->BOOLEAN] =: BOOLEAN" },
    { GEL_Plus, "Plus[x->NUMBER, y->NUMBER] =: NUMBER" },
    { GEL_Times, "Times[x->NUMBER, y->NUMBER] =: NUMBER" },
    { 0, 0 } };

  for (int i = 0; sigarray[i].func; i++)  
    env->Register(sigarray[i].func, sigarray[i].sig);
}












