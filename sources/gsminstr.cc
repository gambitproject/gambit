//
// FILE: gsminstr.cc -- implementation of GCL expression classes
//
//  $Id$
//


#include "gsminstr.h"
#include "gstream.h"
#include "gcmdline.h"

#include "gsm.h"
#include "gsmfunc.h"
#include "gsmhash.h"

extern GSM &_gsm;

//-------------
// Base class
//-------------

gclExpression::gclExpression(void)
  : m_line(0), m_file("Unknown")
{ }

gclExpression::gclExpression(int p_line, const gText &p_file)
  : m_line(p_line), m_file(p_file)
{ }


//---------
// Quit
//---------

PortionSpec gclQuitExpression::Type(void) const
{
  return porBOOL;
}

Portion *gclQuitExpression::Evaluate(void)
{
  throw gclQuitOccurred(0);
}

//--------
// Semi
//--------

gclSemiExpr::gclSemiExpr(gclExpression *e1, gclExpression *e2)
  : lhs(e1), rhs(e2)
{ }

gclSemiExpr::~gclSemiExpr()
{
  delete lhs;
  delete rhs;
}

PortionSpec gclSemiExpr::Type(void) const
{
  return rhs->Type();
}

Portion *gclSemiExpr::Evaluate(void) 
{
  delete lhs->Evaluate();
  return rhs->Evaluate();
}


//--------------------
// ReqParameterList
//--------------------

gclReqParameterList::gclReqParameterList(void)
{ }

gclReqParameterList::gclReqParameterList(gclExpression *p)
{
  exprs.Append(p);
}

gclReqParameterList::~gclReqParameterList()
{
  for (int i = 1; i <= exprs.Length(); delete exprs[i++]);
}

void gclReqParameterList::Append(gclExpression *p)
{
  exprs.Append(p);
}

int gclReqParameterList::NumParams(void) const
{
  return exprs.Length();
}

gclExpression *gclReqParameterList::operator[](int index) const
{
  return exprs[index];
}

//-------------------
// OptParameterList
//-------------------

gclOptParameterList::gclOptParameterList(void)
{ }

gclOptParameterList::gclOptParameterList(const gText &s, gclExpression *p)
{
  names.Append(s);
  exprs.Append(p);
}

gclOptParameterList::~gclOptParameterList()
{
  for (int i = 1; i <= exprs.Length(); delete exprs[i++]);
}

void gclOptParameterList::Append(const gText &s, gclExpression *p)
{
  names.Append(s);
  exprs.Append(p);
}

int gclOptParameterList::NumParams(void) const
{
  return exprs.Length();
}

gclExpression *gclOptParameterList::operator[](int index) const
{
  return exprs[index];
}

gText gclOptParameterList::FormalName(int index) const
{
  return names[index];
}

//-----------------
// ParameterList
//-----------------

gclParameterList::gclParameterList(void)
  : req(new gclReqParameterList), opt(new gclOptParameterList)
{ }

gclParameterList::gclParameterList(gclReqParameterList *r)
  : req(r), opt(new gclOptParameterList)
{ }

gclParameterList::gclParameterList(gclOptParameterList *o)
  : req(new gclReqParameterList), opt(o)
{ }

gclParameterList::gclParameterList(gclReqParameterList *r,
				   gclOptParameterList *o)
  : req(r), opt(o)
{ }

gclParameterList::~gclParameterList()
{
  delete req;
  delete opt;
}



//----------------
// FunctionCall
//----------------

gclFunctionCall::gclFunctionCall(const gText &p_name,
				 int p_line, const gText &p_file)
  : gclExpression(p_line, p_file),
    name(p_name), params(new gclParameterList), funcptr(0), type(porANYTYPE)
{ }

gclFunctionCall::gclFunctionCall(const gText &p_name, gclExpression *p_op,
				 int p_line, const gText &p_file)
  : gclExpression(p_line, p_file),
    name(p_name), params(new gclParameterList), funcptr(0), type(porANYTYPE)
{
  params->req->Append(p_op);
}

gclFunctionCall::gclFunctionCall(const gText &p_name,
				 gclExpression *p_op1, gclExpression *p_op2,
				 int p_line, const gText &p_file)
  : gclExpression(p_line, p_file),
    name(p_name), params(new gclParameterList), funcptr(0), type(porANYTYPE)
{
  params->req->Append(p_op1);
  params->req->Append(p_op2);
}

gclFunctionCall::gclFunctionCall(const gText &p_name,
				 gclParameterList *p_params,
				 int p_line, const gText &p_file)
  : gclExpression(p_line, p_file),
    name(p_name), params(p_params), funcptr(0), type(porANYTYPE)
{ }

gclFunctionCall::~gclFunctionCall()
{
  delete params;
}


PortionSpec gclFunctionCall::Type(void) const
{
  return type;
}


Portion *gclFunctionCall::Evaluate(void)
{
  if (!_gsm._FuncTable->IsDefined(name))  
    throw gclRuntimeError("Undefined function " + name);

  CallFuncObj *call = new CallFuncObj((*_gsm._FuncTable)(name), 
				      m_line, m_file);
  
  try {
    for (int i = 1; i <= params->req->NumParams(); i++)   {
      Portion *val = (*params->req)[i]->Evaluate();
      if (val->Spec().Type == porREFERENCE)   {
	if (_gsm.VarIsDefined(((ReferencePortion *) val)->Value()))
	  call->SetCurrParam(_gsm.VarValue(((ReferencePortion *) val)->Value())->RefCopy(), AUTO_VAL_OR_REF);
	else  {
	  call->SetCurrParam(val, AUTO_VAL_OR_REF);
	}
      }
      else  
	call->SetCurrParam(val, AUTO_VAL_OR_REF);
    }
  
    for (int i = 1; i <= params->opt->NumParams(); i++)  {
      call->SetCurrParamIndex(params->opt->FormalName(i));
      Portion *val = (*params->opt)[i]->Evaluate();
      if (val->Spec().Type == porREFERENCE)   {
	if (_gsm.VarIsDefined(((ReferencePortion *) val)->Value()))
	  call->SetCurrParam(_gsm.VarValue(((ReferencePortion *) val)->Value())->RefCopy(), true);
	else  {
	  call->SetCurrParam(val);
	}
      }
      else  
	call->SetCurrParam(val);
    }
    
    Portion **param = new Portion *[call->NumParams()];
    Portion *ret = call->CallFunction(&_gsm, param);
      
    for (int index = 0; index < call->NumParams(); index++)   {
      ReferencePortion *refp = call->GetParamRef(index);
      if (refp != 0)  {
	_gsm.VarDefine(refp->Value(), param[index]);
	delete refp;
      }
    }

    return ret;
  }
  catch (...) {
    delete call;
    throw;
  }
}



//--------------
// Assignment
//--------------

gclAssignment::gclAssignment(gclExpression *lhs, gclExpression *rhs)
  : variable(lhs), value(rhs)
{ }

gclAssignment::~gclAssignment()
{
  delete variable;
  delete value;
}

Portion *gclAssignment::Evaluate(void)
{
  Portion *lhs = variable->Evaluate();
  Portion *rhs;

  try  {
    rhs = value->Evaluate();
  }
  catch (gclRuntimeError &)  {
    delete lhs;
    throw;
  }

  // Assign() will delete lhs and rhs
  return _gsm.Assign(lhs, rhs);
}


//----------------
// UnAssignment
//----------------

gclUnAssignment::gclUnAssignment(gclExpression *lhs)
  : variable(lhs)
{ }

gclUnAssignment::~gclUnAssignment()
{
  delete variable;
}

Portion *gclUnAssignment::Evaluate(void)
{
  Portion *lhs = variable->Evaluate();
  _gsm.UnAssign(lhs);
  return new BoolPortion(true);
}
  

//-------------
// ConstExpr
//-------------

gclConstExpr::gclConstExpr(Portion *v)
  : value(v)
{ }

gclConstExpr::~gclConstExpr()
{
  delete value;
}

PortionSpec gclConstExpr::Type(void) const
{
  return value->Spec();
}

Portion *gclConstExpr::Evaluate(void)
{
  return value->ValCopy();
}


//---------------
// ListConstant
//---------------

gclListConstant::gclListConstant(void)
{ }

gclListConstant::gclListConstant(gclExpression *expr)
{
  values.Append(expr);
}

gclListConstant::~gclListConstant()
{
  for (int i = 1; i <= values.Length(); delete values[i++]);
}

void gclListConstant::Append(gclExpression *expr)
{
  values.Append(expr);
}

Portion *gclListConstant::Evaluate(void)
{
  ListPortion *ret = new ListPortion;

  try  {
    for (int i = 1; i <= values.Length(); i++)  {
      Portion *v = values[i]->Evaluate();
      _gsm._ResolveRef(v);
      // v is deleted by ListPortion::Append if this call fails
      ret->Append(v);
    }

    return ret;
  }
  catch (...)   {
    delete ret;
    throw;
  }
}


//-----------
// VarName
//-----------

gclVarName::gclVarName(const gText &name)
  : value(new ReferencePortion(name))
{ }

gclVarName::~gclVarName()
{
  delete value;
}

Portion *gclVarName::Evaluate(void)
{
  if (((ReferencePortion *) value)->Value() == "Quit")
    throw gclQuitOccurred(0);

  return value->ValCopy();
}


//---------------
// Conditional
//---------------

gclConditional::gclConditional(gclExpression *g, gclExpression *t,
                               gclExpression *f)
  : guard(g), truebr(t), falsebr(f)
{ }

gclConditional::~gclConditional()
{
  delete guard;
  delete truebr;
  delete falsebr;
}

Portion *gclConditional::Evaluate(void)
{
  Portion *guardval = guard->Evaluate();
  _gsm._ResolveRef(guardval);
  if (guardval->Spec().Type != porBOOL ||
      guardval->Spec().ListDepth > 0)
    throw gclRuntimeError("Guard must evaluate to BOOLEAN");

  try  {
    Portion *ret;
    if (((BoolPortion *) guardval)->Value() == triTRUE)
      ret = truebr->Evaluate();
    else
      ret = falsebr->Evaluate();
    delete guardval;
    return ret;
  }
  catch (...)   {
    delete guardval;
    throw;
  }
}


//------------
// WhileExpr
//------------

gclWhileExpr::gclWhileExpr(gclExpression *g, gclExpression *b)
  : guard(g), body(b)
{ }

gclWhileExpr::~gclWhileExpr()
{
  delete guard;
  delete body;
}

Portion *gclWhileExpr::Evaluate(void)
{
  Portion *ret = new NullPortion(porNUMBER);

  while (1)   {
    Portion *guardval;
    try  {
      guardval = guard->Evaluate();
    }
    catch (...)  {
      delete ret;
      throw;
    }

    try  {
      _gsm._ResolveRef(guardval);
    }
    catch (...)   {
      delete guardval;
      delete ret;
      throw;
    }
    
    if (guardval->Spec().Type != porBOOL ||
        guardval->Spec().ListDepth > 0)
      throw gclRuntimeError("Guard must evaluate to BOOLEAN");

    if (((BoolPortion *) guardval)->Value() != triTRUE)  {
      delete guardval;
      return ret;
    }

    delete guardval;
    delete ret;
    ret = body->Evaluate();
  }
}


//-----------
// ForExpr
//-----------

gclForExpr::gclForExpr(gclExpression *i, gclExpression *g,
		       gclExpression *s, gclExpression *b)
  : init(i), guard(g), step(s), body(b)
{ }

gclForExpr::~gclForExpr()
{
  delete init;
  delete guard;
  delete step;
  delete body;
}

Portion *gclForExpr::Evaluate(void)
{
  Portion *ret = new NullPortion(porNUMBER);

  try  {
    delete init->Evaluate();
  }
  catch (...)   {
    delete ret;
    throw;
  }

  while (1)   {
    Portion *guardval;

    try  {
      guardval = guard->Evaluate();
    }
    catch (...)  {
      delete ret;
      throw;
    }

    try  {
      _gsm._ResolveRef(guardval);
    }
    catch (...)  {
      delete ret;
      delete guardval;
      throw;
    }

    if (guardval->Spec().Type != porBOOL ||
	      guardval->Spec().ListDepth > 0)
      throw gclRuntimeError("Guard must evaluate to BOOLEAN"); 

    if (((BoolPortion *) guardval)->Value() != triTRUE)  {
      delete guardval;
      return ret;
    }

    delete guardval;
    delete ret;
    ret = body->Evaluate();

    try   {
      delete step->Evaluate();
    }
    catch (...)  {
      delete ret;
      throw;
    }
  }
}
    

//---------------
// FunctionDef
//---------------

gclFunctionDef::gclFunctionDef(FuncDescObj *f, gclExpression *b)
  : func(f), body(b)
{ }

gclFunctionDef::~gclFunctionDef()
{ }

Portion *gclFunctionDef::Evaluate(void)
{
  _gsm.AddFunction(func);
  return new BoolPortion(true);
}








