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


DECLARE_UNARY(gelfuncNot, gTriState, gTriState)
DECLARE_UNARY(gelfuncNegate, gNumber, gNumber)


gTriState gelfuncNot::EvalItem( gTriState x1 ) const
{
  return TriStateNot( x1 );
}

gNumber gelfuncNegate::EvalItem( gNumber x1 ) const
{
  return -x1;
}



DECLARE_BINARY(gelfuncDivide, gNumber, gNumber, gNumber)
DECLARE_BINARY(gelfuncMinus, gNumber, gNumber, gNumber)
DECLARE_BINARY(gelfuncPlus, gNumber, gNumber, gNumber)
DECLARE_BINARY(gelfuncTimes, gNumber, gNumber, gNumber)


gNumber gelfuncDivide::EvalItem( gNumber x1, gNumber x2 ) const
{
  return x1 / x2;
}

gNumber gelfuncMinus::EvalItem( gNumber x1, gNumber x2 ) const
{
  return x1 - x2;
}

gNumber gelfuncPlus::EvalItem( gNumber x1, gNumber x2 ) const
{
  return x1 + x2;
}

gNumber gelfuncTimes::EvalItem( gNumber x1, gNumber x2 ) const
{
  return x1 * x2;
}

DECLARE_BINARY(gelfuncLessNumber, gNumber, gNumber, gTriState)
DECLARE_BINARY(gelfuncLessEqualNumber, gNumber, gNumber, gTriState)
DECLARE_BINARY(gelfuncGreaterNumber, gNumber, gNumber, gTriState)
DECLARE_BINARY(gelfuncGreaterEqualNumber, gNumber, gNumber, gTriState)
DECLARE_BINARY(gelfuncEqualNumber, gNumber, gNumber, gTriState)
DECLARE_BINARY(gelfuncNotEqualNumber, gNumber, gNumber, gTriState)


gTriState gelfuncLessNumber::EvalItem( gNumber x1, gNumber x2 ) const
{
  return gTriState( x1 < x2 );
}

gTriState gelfuncLessEqualNumber::EvalItem( gNumber x1, gNumber x2 ) const
{
  return gTriState( x1 <= x2 );
}

gTriState gelfuncGreaterNumber::EvalItem( gNumber x1, gNumber x2 ) const
{
  return gTriState( x1 > x2 );
}

gTriState gelfuncGreaterEqualNumber::EvalItem( gNumber x1, gNumber x2 ) const
{
  return gTriState( x1 >= x2);
}

gTriState gelfuncEqualNumber::EvalItem( gNumber x1, gNumber x2 ) const
{
  return gTriState( x1 == x2 );
}

gTriState gelfuncNotEqualNumber::EvalItem( gNumber x1, gNumber x2 ) const
{
  return gTriState( x1 != x2 );
}

DECLARE_BINARY(gelfuncAnd, gTriState, gTriState, gTriState)
DECLARE_BINARY(gelfuncOr, gTriState, gTriState, gTriState)

gTriState gelfuncAnd::EvalItem( gTriState x1, gTriState x2 ) const
{
  return TriStateAnd( x1, x2 );
}

gTriState gelfuncOr::EvalItem( gTriState x1, gTriState x2 ) const
{
  return TriStateOr( x1, x2 );
}

DECLARE_BINARY(gelfuncLessText, gText, gText, gTriState)
DECLARE_BINARY(gelfuncLessEqualText, gText, gText, gTriState)
DECLARE_BINARY(gelfuncGreaterText, gText, gText, gTriState)
DECLARE_BINARY(gelfuncGreaterEqualText, gText, gText, gTriState)
DECLARE_BINARY(gelfuncEqualText, gText, gText, gTriState)
DECLARE_BINARY(gelfuncNotEqualText, gText, gText, gTriState)


gTriState gelfuncLessText::EvalItem( gText x1, gText x2 ) const
{
  return gTriState( x1 < x2 );
}

gTriState gelfuncLessEqualText::EvalItem( gText x1, gText x2 ) const
{
  return gTriState( x1 <= x2 );
}

gTriState gelfuncGreaterText::EvalItem( gText x1, gText x2 ) const
{
  return gTriState( x1 > x2 );
}

gTriState gelfuncGreaterEqualText::EvalItem( gText x1, gText x2 ) const
{
  return gTriState( x1 >= x2);
}

gTriState gelfuncEqualText::EvalItem( gText x1, gText x2 ) const
{
  return gTriState( x1 == x2 );
}

gTriState gelfuncNotEqualText::EvalItem( gText x1, gText x2 ) const
{
  return gTriState( x1 != x2 );
}

DECLARE_UNARY(gelfuncNumChars, gText, gNumber)

gNumber gelfuncNumChars::EvalItem( gText x1 ) const
{
  return x1.Length();
}

DECLARE_BINARY(gelfuncNthElement, gText, gNumber, gText)


#ifdef USE_EXCEPTIONS
class gelExceptionNonInteger : public gException  {
  public:
    virtual ~gelExceptionNonInteger()   { }
    gText Description(void) const  { return "Expected integer index"; }
};
#endif   // USE_EXCEPTIONS

gText gelfuncNthElement::EvalItem( gText x1, gNumber x2 ) const
{
#ifdef USE_EXCEPTIONS
  if (!x2.IsInteger())   
    throw gelExceptionNonInteger();
#else
  assert( x2.IsInteger() );
#endif   // USE_EXCEPTIONS
  return x1[x2 - gNumber(1)];
}


DECLARE_BINARY(gelfuncEqualBoolean, gTriState, gTriState, gTriState)
DECLARE_BINARY(gelfuncNotEqualBoolean, gTriState, gTriState, gTriState)

gTriState gelfuncEqualBoolean::EvalItem( gTriState x1, gTriState x2 ) const
{
  return gTriState( x1 == x2 );
}

gTriState gelfuncNotEqualBoolean::EvalItem( gTriState x1, gTriState x2 ) const
{
  return gTriState( x1 != x2 );
}


gelExpr *GEL_And(const gArray<gelExpr *> &params)
{
  return new gelfuncAnd((gelExpression<gTriState> *) params[1],
			(gelExpression<gTriState> *) params[2]);
}

gelExpr *GEL_Divide(const gArray<gelExpr *> &params)
{
  return new gelfuncDivide((gelExpression<gNumber> *) params[1],
			   (gelExpression<gNumber> *) params[2]);
}

gelExpr *GEL_EqualBoolean(const gArray<gelExpr *> &params)
{
  return new gelfuncEqualBoolean((gelExpression<gTriState> *) params[1],
				 (gelExpression<gTriState> *) params[2]);
}

gelExpr *GEL_EqualNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncEqualNumber((gelExpression<gNumber> *) params[1],
				(gelExpression<gNumber> *) params[2]);
}

gelExpr *GEL_EqualText(const gArray<gelExpr *> &params)
{
  return new gelfuncEqualText((gelExpression<gText> *) params[1],
			      (gelExpression<gText> *) params[2]);
}

gelExpr *GEL_GreaterNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncGreaterNumber((gelExpression<gNumber> *) params[1],
				  (gelExpression<gNumber> *) params[2]);
}

gelExpr *GEL_GreaterText(const gArray<gelExpr *> &params)
{
  return new gelfuncGreaterText((gelExpression<gText> *) params[1],
				(gelExpression<gText> *) params[2]);
}

gelExpr *GEL_GreaterEqualNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncGreaterEqualNumber((gelExpression<gNumber> *) params[1],
				       (gelExpression<gNumber> *) params[2]);
}

gelExpr *GEL_GreaterEqualText(const gArray<gelExpr *> &params)
{
  return new gelfuncGreaterEqualText((gelExpression<gText> *) params[1],
				     (gelExpression<gText> *) params[2]);
}

gelExpr *GEL_LessNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncLessNumber((gelExpression<gNumber> *) params[1],
			       (gelExpression<gNumber> *) params[2]);
}

gelExpr *GEL_LessText(const gArray<gelExpr *> &params)
{
  return new gelfuncLessText((gelExpression<gText> *) params[1],
			     (gelExpression<gText> *) params[2]);
}

gelExpr *GEL_LessEqualNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncLessEqualNumber((gelExpression<gNumber> *) params[1],
				    (gelExpression<gNumber> *) params[2]);
}

gelExpr *GEL_LessEqualText(const gArray<gelExpr *> &params)
{
  return new gelfuncLessEqualText((gelExpression<gText> *) params[1],
				  (gelExpression<gText> *) params[2]);
}


gelExpr *GEL_Minus(const gArray<gelExpr *> &params)
{
  return new gelfuncMinus((gelExpression<gNumber> *) params[1],
			  (gelExpression<gNumber> *) params[2]);
}

gelExpr *GEL_Negate(const gArray<gelExpr *> &params)
{
  return new gelfuncNegate((gelExpression<gNumber> *) params[1]);
}

gelExpr *GEL_Not(const gArray<gelExpr *> &params)
{
  return new gelfuncNot((gelExpression<gTriState> *) params[1]);
}

gelExpr *GEL_NotEqualBoolean(const gArray<gelExpr *> &params)
{
  return new gelfuncNotEqualBoolean((gelExpression<gTriState> *) params[1],
				    (gelExpression<gTriState> *) params[2]);
}

gelExpr *GEL_NotEqualNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncNotEqualNumber((gelExpression<gNumber> *) params[1],
				   (gelExpression<gNumber> *) params[2]);
}

gelExpr *GEL_NotEqualText(const gArray<gelExpr *> &params)
{
  return new gelfuncNotEqualText((gelExpression<gText> *) params[1],
				 (gelExpression<gText> *) params[2]);
}

gelExpr *GEL_NthElement(const gArray<gelExpr *> &params)
{
  return new gelfuncNthElement((gelExpression<gText> *) params[1],
			       (gelExpression<gNumber> *) params[2]);
}

gelExpr *GEL_NumChars(const gArray<gelExpr *> &params)
{
  return new gelfuncNumChars((gelExpression<gText> *) params[1]);
}

gelExpr *GEL_Or(const gArray<gelExpr *> &params)
{
  return new gelfuncOr((gelExpression<gTriState> *) params[1],
		       (gelExpression<gTriState> *) params[2]);
}

gelExpr *GEL_Plus(const gArray<gelExpr *> &params)
{
  return new gelfuncPlus((gelExpression<gNumber> *) params[1],
			 (gelExpression<gNumber> *) params[2]);
}


gelExpr *GEL_Times(const gArray<gelExpr *> &params)
{
  return new gelfuncTimes((gelExpression<gNumber> *) params[1],
			  (gelExpression<gNumber> *) params[2]);
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












