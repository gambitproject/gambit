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
  
template class gelExpression<gNumber *>;
gelType gelExpression<gNumber *>::Type(void) const   { return gelNUMBER; }
template class gelExpression<gTriState *>;
gelType gelExpression<gTriState *>::Type(void) const { return gelBOOLEAN; }
template class gelExpression<gText *>;
gelType gelExpression<gText *>::Type(void) const   { return gelTEXT; }
template class gelExpression<Efg *>;
gelType gelExpression<Efg *>::Type(void) const   { return gelEFG; }
template class gelExpression<Node *>;
gelType gelExpression<Node *>::Type(void) const  { return gelNODE; }
template class gelExpression<Action *>;
gelType gelExpression<Action *>::Type(void) const  { return gelACTION; }
template class gelExpression<Infoset *>;
gelType gelExpression<Infoset *>::Type(void) const  { return gelINFOSET; }
template class gelExpression<EFOutcome *>;
gelType gelExpression<EFOutcome *>::Type(void) const  { return gelEFOUTCOME; }
template class gelExpression<EFPlayer *>;
gelType gelExpression<EFPlayer *>::Type(void) const   { return gelEFPLAYER; }
template class gelExpression<EFSupport *>;
gelType gelExpression<EFSupport *>::Type(void) const  { return gelEFSUPPORT; }
template class gelExpression<BehavSolution *>;
gelType gelExpression<BehavSolution *>::Type(void) const  { return gelBEHAV; } 

template class gelExpression<Nfg *>;
gelType gelExpression<Nfg *>::Type(void) const   { return gelNFG; }
template class gelExpression<Strategy *>;
gelType gelExpression<Strategy *>::Type(void) const   { return gelSTRATEGY; }
template class gelExpression<NFOutcome *>;
gelType gelExpression<NFOutcome *>::Type(void) const   { return gelNFOUTCOME; }
template class gelExpression<NFPlayer *>;
gelType gelExpression<NFPlayer *>::Type(void) const   { return gelNFPLAYER; }
template class gelExpression<NFSupport *>;
gelType gelExpression<NFSupport *>::Type(void) const  { return gelNFSUPPORT; }
template class gelExpression<MixedSolution *>;
gelType gelExpression<MixedSolution *>::Type(void) const  { return gelMIXED; }


template class gelConstant<gNumber *>;
template class gelConstant<gTriState *>;
template class gelConstant<gText *>;

gelConstant<gNumber *>::gelConstant(gNumber *v)
{ m_Value.Data().Append(v); }
gelConstant<gNumber *>::gelConstant(const gNestedList<gNumber *> &v)
  : m_Value(v) { }
gelConstant<gNumber *>::~gelConstant()   { }
gNestedList<gNumber *> gelConstant<gNumber *>::Evaluate(gelVariableTable *) const
{ return m_Value; }

gelConstant<gTriState *>::gelConstant(gTriState *v)
{ m_Value.Data().Append(v); }
gelConstant<gTriState *>::gelConstant(const gNestedList<gTriState *>& v)
  : m_Value(v) { }
gelConstant<gTriState *>::~gelConstant() { }
gNestedList<gTriState *> gelConstant<gTriState *>::Evaluate(gelVariableTable *) const
{ return m_Value; }

gelConstant<gText *>::gelConstant(gText *v) 
{ m_Value.Data().Append(v); }
gelConstant<gText *>::gelConstant(const gNestedList<gText *>& v) 
  : m_Value(v) { }
gelConstant<gText *>::~gelConstant()   { }
gNestedList<gText *> gelConstant<gText *>::Evaluate(gelVariableTable *) const 
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

template class gelVariable<gNumber *>;
template class gelVariable<gTriState *>;
template class gelVariable<gText *>;
template class gelVariable<Efg *>;
template class gelVariable<Node *>;
template class gelVariable<Infoset *>;
template class gelVariable<Action *>;
template class gelVariable<EFOutcome *>;
template class gelVariable<EFPlayer *>;
template class gelVariable<EFSupport *>;
template class gelVariable<BehavSolution *>;
template class gelVariable<Nfg *>;
template class gelVariable<Strategy *>;
template class gelVariable<NFOutcome *>;
template class gelVariable<NFPlayer *>;
template class gelVariable<NFSupport *>;
template class gelVariable<MixedSolution *>;



template <class T> gelAssignment<T>::gelAssignment(const gText &varname,
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

template class gelAssignment<gNumber *>;
template class gelAssignment<gTriState *>;
template class gelAssignment<gText *>;
template class gelAssignment<Efg *>;
template class gelAssignment<Node *>;
template class gelAssignment<Action *>;
template class gelAssignment<Infoset *>;
template class gelAssignment<EFPlayer *>;
template class gelAssignment<EFOutcome *>;
template class gelAssignment<EFSupport *>;
template class gelAssignment<BehavSolution *>;
template class gelAssignment<Nfg *>;
template class gelAssignment<Strategy *>;
template class gelAssignment<NFPlayer *>;
template class gelAssignment<NFOutcome *>;
template class gelAssignment<NFSupport *>;
template class gelAssignment<MixedSolution *>;

template <class T>
gelConditional<T>::gelConditional(gelExpression<gTriState *> *g,
				  gelExpression<T> *t)
  : guard(g), truebr(t), falsebr(0)
{ }

template <class T> 
gelConditional<T>::gelConditional(gelExpression<gTriState *> *g,
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
  if (*guard->Evaluate(vt).Data()[1] == triTRUE)
    return truebr->Evaluate(vt);
  else
    return falsebr->Evaluate(vt);
}

template class gelConditional<gNumber *>;
template class gelConditional<gTriState *>;
template class gelConditional<gText *>;
template class gelConditional<Efg *>;
template class gelConditional<Node *>;
template class gelConditional<Action *>;
template class gelConditional<Infoset *>;
template class gelConditional<EFPlayer *>;
template class gelConditional<EFOutcome *>;
template class gelConditional<EFSupport *>;
template class gelConditional<BehavSolution *>;
template class gelConditional<Nfg *>;
template class gelConditional<Strategy *>;
template class gelConditional<NFPlayer *>;
template class gelConditional<NFOutcome *>;
template class gelConditional<NFSupport *>;
template class gelConditional<MixedSolution *>;


template <class T> gelWhileLoop<T>::gelWhileLoop(gelExpression<gTriState *> *g,
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
  gTriState *guardval = guard->Evaluate(vt).Data()[1];
  gNestedList<T> bodyval;

  while (*guardval == triTRUE)  {
    bodyval = body->Evaluate(vt);
    guardval = guard->Evaluate(vt).Data()[1];
  }
  return bodyval;
}

template class gelWhileLoop<gNumber *>;
template class gelWhileLoop<gTriState *>;
template class gelWhileLoop<gText *>;
template class gelWhileLoop<Efg *>;
template class gelWhileLoop<Node *>;
template class gelWhileLoop<Action *>;
template class gelWhileLoop<Infoset *>;
template class gelWhileLoop<EFPlayer *>;
template class gelWhileLoop<EFOutcome *>;
template class gelWhileLoop<EFSupport *>;
template class gelWhileLoop<BehavSolution *>;
template class gelWhileLoop<Nfg *>;
template class gelWhileLoop<Strategy *>;
template class gelWhileLoop<NFPlayer *>;
template class gelWhileLoop<NFOutcome *>;
template class gelWhileLoop<NFSupport *>;
template class gelWhileLoop<MixedSolution *>;


template <class T> gelForLoop<T>::gelForLoop(gelExpr *ini,
					     gelExpression<gTriState *> *g,
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
  gTriState *guardval = guard->Evaluate(vt).Data()[1];
  gNestedList<T> bodyval;

  while (*guardval == triTRUE)  
  {
    bodyval = body->Evaluate(vt);
    incr->Execute(vt);
    guardval = guard->Evaluate(vt).Data()[1];
  }
  return bodyval;
}

template class gelForLoop<gNumber *>;
template class gelForLoop<gTriState *>;
template class gelForLoop<gText *>;
template class gelForLoop<Efg *>;
template class gelForLoop<Node *>;
template class gelForLoop<Action *>;
template class gelForLoop<Infoset *>;
template class gelForLoop<EFPlayer *>;
template class gelForLoop<EFOutcome *>;
template class gelForLoop<EFSupport *>;
template class gelForLoop<BehavSolution *>;
template class gelForLoop<Nfg *>;
template class gelForLoop<Strategy *>;
template class gelForLoop<NFPlayer *>;
template class gelForLoop<NFOutcome *>;
template class gelForLoop<NFSupport *>;
template class gelForLoop<MixedSolution *>;


#include "gcmdline.h"

gNestedList<gTriState *> gelQuitExpr::Evaluate(gelVariableTable *) const
{
  gCmdLineInput::RestoreTermAttr();
  exit(0);
  gNestedList<gTriState *> ret;
  ret.Data().Append(new gTriState(triTRUE));
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



template class gelUDF<gNumber *>;
template class gelUDF<gTriState *>;
template class gelUDF<gText *>;
template class gelUDF<Efg *>;
template class gelUDF<Node *>;
template class gelUDF<Action *>;
template class gelUDF<Infoset *>;
template class gelUDF<EFPlayer *>;
template class gelUDF<EFOutcome *>;
template class gelUDF<EFSupport *>;
template class gelUDF<BehavSolution *>;
template class gelUDF<Nfg *>;
template class gelUDF<Strategy *>;
template class gelUDF<NFPlayer *>;
template class gelUDF<NFOutcome *>;
template class gelUDF<NFSupport *>;
template class gelUDF<MixedSolution *>;



#include "glist.imp"

template class gList<Efg *>;
template class gList<Action *>;
template class gList<Infoset *>;
template class gList<EFPlayer *>;
// gList<EFOutcome *> provided by subsolve.cc currently
//template class gList<EFOutcome *>;
template class gList<EFSupport *>;
template class gList<BehavSolution *>;
template class gList<Nfg *>;
template class gList<Strategy *>;
template class gList<NFPlayer *>;
template class gList<NFOutcome *>;
template class gList<NFSupport *>;
template class gList<MixedSolution *>;

