//
// FILE: gsminstr.cc -- implementation of Instruction classes for GSM's
//                      instruction queue subsystem
//                      companion to GSM
//
//  $Id$
//


#include "gsminstr.h"
#include "gstream.h"
#include "gcmdline.h"

PortionSpec gclQuitExpression::Type(void) const
{
  return porBOOL;
}

Portion *gclQuitExpression::Evaluate(void)
{
  gCmdLineInput::RestoreTermAttr();
  exit(0);
  return 0;
}

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


gclFunctionCall::gclFunctionCall(const gText &s)
  : name(s), params(new gclParameterList), funcptr(0), type(porANYTYPE)
{ }

gclFunctionCall::gclFunctionCall(const gText &s, gclExpression *op)
  : name(s), params(new gclParameterList), funcptr(0), type(porANYTYPE)
{
  params->req->Append(op);
  AttemptMatch();
}

gclFunctionCall::gclFunctionCall(const gText &s,
				 gclExpression *op1, gclExpression *op2)
  : name(s), params(new gclParameterList), funcptr(0), type(porANYTYPE)
{
  params->req->Append(op1);
  params->req->Append(op2);
  AttemptMatch();
}

gclFunctionCall::gclFunctionCall(const gText &s, gclParameterList *p)
  : name(s), params(p), funcptr(0), type(porANYTYPE)
{
  AttemptMatch();
}

gclFunctionCall::~gclFunctionCall()
{
  delete params;
}


PortionSpec gclFunctionCall::Type(void) const
{
  return type;
}

#include "gsm.h"
#include "gsmfunc.h"
#include "gsmhash.h"

extern GSM &_gsm;

void gclFunctionCall::AttemptMatch(void)
{
  if (!_gsm._FuncTable->IsDefined(name))
    return;

  FuncDescObj *func = (*_gsm._FuncTable)(name);
  for (int i = 0; i < func->_NumFuncs; i++)  {
    if (func->_FuncInfo[i].NumParams != params->req->NumParams())
      continue;
    
    int j;
    
    for (j = 0; j < func->_FuncInfo[i].NumParams; j++)  {
      if (!(func->_FuncInfo[i].ParamInfo[j].Spec == 
	  (*params->req)[j+1]->Type()))
	break;
    }

    if (j < func->_FuncInfo[i].NumParams)  continue;

//    gout << "Matched function signature " << i << " of " << name << '\n';
    type = func->_FuncInfo[i].ReturnSpec;
    break;
  }
}

Portion *gclFunctionCall::Evaluate(void)
{
  if (!_gsm._FuncTable->IsDefined(name))  
    throw gclRuntimeError("Undefined function " + name);

  CallFuncObj *call = new CallFuncObj((*_gsm._FuncTable)(name),
				      _gsm._StdOut, _gsm._StdErr);
  
  int undefined = 0;
  int i;
  for (i = 1; i <= params->req->NumParams(); i++)   {
    Portion *val = (*params->req)[i]->Evaluate();
    if (val->Spec().Type == porREFERENCE)   {
      if (_gsm.VarIsDefined(((ReferencePortion *) val)->Value()))
	call->SetCurrParam(_gsm.VarValue(((ReferencePortion *) val)->Value())->RefCopy(), AUTO_VAL_OR_REF);
      else  {
	undefined = i;
	break;
      }
    }
    else  
      call->SetCurrParam(val, AUTO_VAL_OR_REF);
  }
  
  for (i = 1; i <= params->opt->NumParams(); i++)  {
    call->SetCurrParamIndex(params->opt->FormalName(i));
    Portion *val = (*params->opt)[i]->Evaluate();
    if (val->Spec().Type == porREFERENCE)   {
      if (_gsm.VarIsDefined(((ReferencePortion *) val)->Value()))
	call->SetCurrParam(_gsm.VarValue(((ReferencePortion *) val)->Value())->RefCopy(), true);
      else  {
	undefined = i;
	break;
      }
    }
    else  
      call->SetCurrParam(val);
  }
    
  Portion *ret;

  if (undefined == 0)  {
    Portion **param = new Portion *[call->NumParams()];
    ret = call->CallFunction(&_gsm, param);
    
    for (int index = 0; index < call->NumParams(); index++)   {
      ReferencePortion *refp = call->GetParamRef(index);

      assert((refp == 0) == (param[index] == 0));

      if (refp != 0)  {
	_gsm.VarDefine(refp->Value(), param[index]);
	delete refp;
      }
    }
  }
  else 
    throw gclRuntimeError("Parameter " + ToText(undefined) +
                           " undefined in call to " + name);
    
  delete call;

  return ret;
}


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
      // not clear what causes this condition...?
      if (ret->Append(v) == 0)
        throw gclRuntimeError("");
    }

    return ret;
  }
  catch (...)   {
    delete ret;
    throw;
  }
}


gclVarName::gclVarName(const gText &name)
  : value(new ReferencePortion(name))
{ }

gclVarName::~gclVarName()
{
  delete value;
}

Portion *gclVarName::Evaluate(void)
{
  return value->ValCopy();
}


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








