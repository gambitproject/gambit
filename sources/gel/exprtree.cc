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

gelConstant<gNumber>::gelConstant(const gNumber &v) : value(v)   { }

gelConstant<gNumber>::~gelConstant()   { }

gNumber gelConstant<gNumber>::Evaluate(gelVariableTable *) const
{ return value; }


gelConstant<gTriState>::gelConstant(const gTriState &v) : value(v)   { }

gelConstant<gTriState>::~gelConstant()   { }

gTriState gelConstant<gTriState>::Evaluate(gelVariableTable *) const
{ return value; }


gelConstant<gText>::gelConstant(const gText &v) : value(v)   { }

gelConstant<gText>::~gelConstant()   { }

gText gelConstant<gText>::Evaluate(gelVariableTable *) const
{ return value; }


#include "gvartbl.h"

template <class T> gelVariable<T>::gelVariable(const gText &varname) 
  : m_Name(varname) { }

template <class T> gelVariable<T>::~gelVariable()   { }

template <class T> T gelVariable<T>::Evaluate(gelVariableTable *vt) const
{
  T ret;
  vt->Value(m_Name, ret);
  return ret;
}

template class gelVariable<gNumber>;
template class gelVariable<gTriState>;
template class gelVariable<gText>;

gelVariable<gTriState>::gelVariable(const gText &varname) : m_Name(varname) { }

gelVariable<gTriState>::~gelVariable()   { }

gTriState gelVariable<gTriState>::Evaluate(gelVariableTable *vt) const
{
  gTriState ret;
  vt->Value(m_Name, ret);
  return ret;
}

template class gelVariable<gTriState>;



template <class T> gelAssignment<T>::gelAssignment(const gText &varname,
						   gelExpression<T> *value)
  : name(varname), rhs(value)
{ }

template <class T> gelAssignment<T>::~gelAssignment()   
{
  delete rhs;
}

template <class T> T gelAssignment<T>::Evaluate(gelVariableTable *vt) const
{
  T value = rhs->Evaluate(vt);
  if( !vt->IsDefined( name ) )
    vt->Define( name, Type() );
  vt->SetValue(name, value);
  return value;
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

template <class T> T gelConditional<T>::Evaluate(gelVariableTable *vt) const
{
  if (guard->Evaluate(vt) == triTRUE)
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

template <class T> T gelWhileLoop<T>::Evaluate(gelVariableTable *vt) const
{
  gTriState guardval = guard->Evaluate(vt);
  T bodyval;

  while (guardval == triTRUE)  {
    bodyval = body->Evaluate(vt);
    guardval = guard->Evaluate(vt);
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

template <class T> T gelForLoop<T>::Evaluate(gelVariableTable *vt) const
{
  init->Execute(vt);
  gTriState guardval = guard->Evaluate(vt);
  T bodyval;

  while (guardval == triTRUE)  {
    bodyval = body->Evaluate(vt);
    incr->Execute(vt);
    guardval = guard->Evaluate(vt);
  }

  return bodyval;
}

template class gelForLoop<gNumber>;
template class gelForLoop<gTriState>;
template class gelForLoop<gText>;

#include "gcmdline.h"

gTriState gelQuitExpr::Evaluate(gelVariableTable *) const
{
  gCmdLineInput::RestoreTermAttr();
  exit(0);
  return triTRUE;
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

template <class T> T gelUDF<T>::Evaluate( gelVariableTable* vt ) const
{
  assert( m_Body );

  gelVariableTable subvt;
  m_Signature.AssignParams( &subvt, vt, m_Params );
  return m_Body->Evaluate( &subvt );
}



template class gelUDF<gNumber>;
template class gelUDF<gTriState>;
template class gelUDF<gText>;

