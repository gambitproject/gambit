//
// FILE: exprtree.cc -- Basic expression tree classes for GEL
//
// $Id$
//

#include "exprtree.h"
#include "tristate.h"
#include "gnumber.h"
#include "gtext.h"


gelExpr::~gelExpr()  { }
template <class T> gelExpression<T>::~gelExpression()   { }
template <class T> void gelExpression<T>::Execute(gelVariableTable *vt) const
{ Evaluate(vt); }
  
template class gelExpression<gNumber>;
gelType gelExpression<gNumber>::Type(void) const   { return gelNUMBER; }
template class gelExpression<gTriState>;
gelType gelExpression<gTriState>::Type(void) const { return gelBOOLEAN; }
template class gelExpression<gText>;
gelType gelExpression<gText>::Type(void) const   { return gelTEXT; }

template class gelConstant<gNumber>;
template class gelConstant<gTriState>;
template class gelConstant<gText>;



gelConstant<gNumber>::gelConstant(const gNumber &v) 
{ m_Value.Data().Append(v); }
gelConstant<gNumber>::gelConstant(const gNestedList<gNumber> &v) 
  : m_Value(v) { }
gelConstant<gNumber>::~gelConstant()   { }
gNestedList<gNumber> gelConstant<gNumber>::Evaluate(gelVariableTable *) const 
{ return m_Value; }


gelConstant<gTriState>::gelConstant(const gTriState &v) 
{ m_Value.Data().Append(v); }
gelConstant<gTriState>::gelConstant(const gNestedList<gTriState>& v) 
  : m_Value(v) { }
gelConstant<gTriState>::~gelConstant() { }
gNestedList<gTriState> gelConstant<gTriState>::Evaluate(gelVariableTable *) const 
{ return m_Value; }


gelConstant<gText>::gelConstant(const gText &v) 
{ m_Value.Data().Append(v); }
gelConstant<gText>::gelConstant(const gNestedList<gText>& v) 
  : m_Value(v) { }
gelConstant<gText>::~gelConstant()   { }
gNestedList<gText> gelConstant<gText>::Evaluate(gelVariableTable *) const 
{ return m_Value; }


#include "gvartbl.h"

template <class T> gelVariable<T>::gelVariable(const gText &varname) 
  : m_Name(varname) { }

template <class T> gelVariable<T>::~gelVariable()   { }

template <class T> 
gNestedList<T> gelVariable<T>::Evaluate(gelVariableTable *vt) const
{
  gNestedList<T> ret;
  vt->Value(m_Name, ret);
  return ret;
}

template class gelVariable<gNumber>;
template class gelVariable<gTriState>;
template class gelVariable<gText>;



template <class T> gelAssignment<T>::gelAssignment( const gText& varname,
						    gelExpression<T> *v )
  : name(varname), rhs(v)
{ }

template <class T> gelAssignment<T>::~gelAssignment()   
{
  delete rhs;
}

template <class T> 
gNestedList<T> gelAssignment<T>::Evaluate(gelVariableTable *vt) const
{
  gNestedList<T> ret = rhs->Evaluate(vt);
  if( !vt->IsDefined( name ) )
    vt->Define( name, Type() );
  vt->SetValue(name, ret);
  return ret;
}

template class gelAssignment<gNumber>;
template class gelAssignment<gTriState>;
template class gelAssignment<gText>;


template <class T>
gelConditional<T>::gelConditional(gelExpression<gTriState> *g,
				  gelExpression<T> *t)
  : guard(g), truebr(t), falsebr(0)
{ }

template <class T> 
gelConditional<T>::gelConditional(gelExpression<gTriState> *g,
				  gelExpression<T> *t,
				  gelExpression<T> *f)
  : guard(g), truebr(t), falsebr(f)
{ }

template <class T>
gelConditional<T>::~gelConditional()
{
  delete guard;
  delete truebr;
  delete falsebr;
}

template <class T> 
gNestedList<T> gelConditional<T>::Evaluate(gelVariableTable *vt) const
{
  if (guard->Evaluate(vt).Data()[1] == triTRUE)
    return truebr->Evaluate(vt);
  else
    return falsebr->Evaluate(vt);
}

template class gelConditional<gNumber>;
template class gelConditional<gTriState>;
template class gelConditional<gText>;

template <class T> gelWhileLoop<T>::gelWhileLoop(gelExpression<gTriState> *g,
						 gelExpression<T> *b)
  : guard(g), body(b)
{ }

template <class T> gelWhileLoop<T>::~gelWhileLoop()
{
  delete guard;
  delete body;
}

template <class T> 
gNestedList<T> gelWhileLoop<T>::Evaluate(gelVariableTable *vt) const
{
  gTriState guardval = guard->Evaluate(vt).Data()[1];
  gNestedList<T> bodyval;

  while (guardval == triTRUE)  {
    bodyval = body->Evaluate(vt);
    guardval = guard->Evaluate(vt).Data()[1];
  }
  return bodyval;
}

template class gelWhileLoop<gNumber>;
template class gelWhileLoop<gTriState>;
template class gelWhileLoop<gText>;


template <class T> gelForLoop<T>::gelForLoop(gelExpr *ini,
					     gelExpression<gTriState> *g,
					     gelExpr *inc,
					     gelExpression<T> *b)
  : init(ini), guard(g), incr(inc), body(b)
{ }

template <class T> gelForLoop<T>::~gelForLoop()
{
  delete init;
  delete guard;
  delete incr;
  delete body;
}

template <class T> 
gNestedList<T> gelForLoop<T>::Evaluate(gelVariableTable *vt) const
{
  init->Execute(vt);
  gTriState guardval = guard->Evaluate(vt).Data()[1];
  gNestedList<T> bodyval;

  while (guardval == triTRUE)  
  {
    bodyval = body->Evaluate(vt);
    incr->Execute(vt);
    guardval = guard->Evaluate(vt).Data()[1];
  }
  return bodyval;
}

template class gelForLoop<gNumber>;
template class gelForLoop<gTriState>;
template class gelForLoop<gText>;

#include "gcmdline.h"

gNestedList<gTriState> gelQuitExpr::Evaluate(gelVariableTable *) const
{
  gCmdLineInput::RestoreTermAttr();
  exit(0);
  gNestedList<gTriState> ret;
  ret.Data().Append( triTRUE );
  return ret;
}



template <class T> gelUDF<T>::gelUDF(const gelSignature& sig, 
				     const gArray<gelExpr *>& params, 
				     const gelExpression<T>* exp )
: m_Signature( sig ), m_Params( params ), m_Body( exp )
{ 
  assert( m_Body );
}

template <class T> gelUDF<T>::~gelUDF()
{
  // do not delete m_Body!
}

template <class T>
gNestedList<T> gelUDF<T>::Evaluate( gelVariableTable* vt ) const
{
  assert( m_Body );

  gelVariableTable subvt;
  m_Signature.AssignParams( &subvt, vt, m_Params );
  return m_Body->Evaluate( &subvt );
}



template class gelUDF<gNumber>;
template class gelUDF<gTriState>;
template class gelUDF<gText>;

