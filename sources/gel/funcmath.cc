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



DECLARE_UNARY_LIST(gelfuncPrintNumber, gNumber, gNumber)
DECLARE_UNARY_LIST(gelfuncPrintBoolean, gTriState, gTriState)
DECLARE_UNARY_LIST(gelfuncPrintText, gText, gText)

DECLARE_UNARY(gelfuncNot, gTriState, gTriState)
DECLARE_UNARY(gelfuncNegate, gNumber, gNumber)



gNestedList<gNumber> 
gelfuncPrintNumber::Evaluate( gelVariableTable *vt ) const
{
  gNestedList<gNumber> ret = op1->Evaluate( vt );
  gout << ret;
  return ret;
}

gNestedList<gText> 
gelfuncPrintText::Evaluate( gelVariableTable *vt ) const
{
  gNestedList<gText> ret = op1->Evaluate( vt );
  gout << ret;
  return ret;
}



gNestedList<gTriState> 
gelfuncPrintBoolean::Evaluate( gelVariableTable *vt ) const
{
  gNestedList<gTriState> ret = op1->Evaluate( vt );
  ret.Output( gout, gTriState_Output );
  return ret;
}


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


template <class T> class gelfuncSemi : public gelExpression<T>  
{
private:
  gelExpr *op1;
  gelExpression<T> *op2;
  
public:
  gelfuncSemi(gelExpr *, gelExpression<T> *);
  ~gelfuncSemi();
  
  gNestedList<T> Evaluate(gelVariableTable *vt) const;
};

template <class T>
gelfuncSemi<T>::gelfuncSemi(gelExpr *x, gelExpression<T> *y)
  : op1(x), op2(y)
{ }

template <class T> gelfuncSemi<T>::~gelfuncSemi()
{ delete op1;  delete op2; }

template <class T> 
gNestedList<T> gelfuncSemi<T>::Evaluate(gelVariableTable *vt) const
{
  op1->Execute(vt);
  return op2->Evaluate(vt);
}

template class gelfuncSemi<gTriState>;
template class gelfuncSemi<gNumber>;
template class gelfuncSemi<gText>;


#include "gwatch.h"

gWatch _gelStopwatch;

DECLARE_NOPARAM(gelfuncStartWatch, gNumber);

gNestedList<gNumber> gelfuncStartWatch::Evaluate(gelVariableTable *) const
{
  _gelStopwatch.Start();
  gNestedList<gNumber> ret;
  ret.Data().Append( 0 );
  return ret;
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

gelExpr *GEL_PrintBoolean(const gArray<gelExpr *> &params)
{
  return new gelfuncPrintBoolean((gelExpression<gTriState> *) params[1]);
}

gelExpr *GEL_PrintNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncPrintNumber((gelExpression<gNumber> *) params[1]);
}

gelExpr *GEL_PrintText(const gArray<gelExpr *> &params)
{
  return new gelfuncPrintText((gelExpression<gText> *) params[1]);
}

gelExpr *GEL_SemiBoolean(const gArray<gelExpr *> &params)
{
  return new gelfuncSemi<gTriState>(params[1],
				    ((gelExpression<gTriState> *) params[2]));
}

gelExpr *GEL_SemiNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncSemi<gNumber>(params[1],
				  ((gelExpression<gNumber> *) params[2]));
}

gelExpr *GEL_SemiText(const gArray<gelExpr *> &params)
{
  return new gelfuncSemi<gText>(params[1],
				((gelExpression<gText> *) params[2]));
}

gelExpr *GEL_StartWatch(const gArray<gelExpr *> &)
{
  return new gelfuncStartWatch();
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
    { GEL_PrintBoolean, "Print[x->BOOLEAN] =: BOOLEAN" },
    { GEL_PrintNumber, "Print[x->NUMBER] =: NUMBER" },
    { GEL_PrintText, "Print[x->TEXT] =: TEXT" },
    { GEL_SemiBoolean, "Semi[x->BOOLEAN, y->BOOLEAN] =: BOOLEAN" },
    { GEL_SemiBoolean, "Semi[x->NUMBER, y->BOOLEAN] =: BOOLEAN" },
    { GEL_SemiBoolean, "Semi[x->TEXT, y->BOOLEAN] =: BOOLEAN" },
    { GEL_SemiNumber, "Semi[x->BOOLEAN, y->NUMBER] =: NUMBER" },
    { GEL_SemiNumber, "Semi[x->NUMBER, y->NUMBER] =: NUMBER" },
    { GEL_SemiNumber, "Semi[x->TEXT, y->NUMBER] =: NUMBER" },
    { GEL_SemiText, "Semi[x->BOOLEAN, y->TEXT] =: TEXT" },
    { GEL_SemiText, "Semi[x->NUMBER, y->TEXT] =: TEXT" },
    { GEL_SemiText, "Semi[x->TEXT, y->TEXT] =: TEXT" },
    { GEL_StartWatch, "StartWatch[] =: NUMBER" },
    { GEL_Times, "Times[x->NUMBER, y->NUMBER] =: NUMBER" },
    { 0, 0 } };

  for (int i = 0; sigarray[i].func; i++)  
    env->Register(sigarray[i].func, sigarray[i].sig);
}












