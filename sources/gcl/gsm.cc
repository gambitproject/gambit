//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of GSM environment
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//


class Portion;
class CallFuncObj;
class RefHashTable;

#include "base/base.h"

#include "gsm.h"
#include "portion.h"
#include "gsmhash.h"
#include "gsmfunc.h"
#include "gsminstr.h"

#include "game/nfg.h"
#include "nash/mixedsol.h"
#include "nash/behavsol.h"

#include "game/efbasis.h"


//----------------------------------------------------------------
//        declaration of the game reference counter
//----------------------------------------------------------------

RefCountHashTable< void* > GSM::_GameRefCount;
int& GSM::GameRefCount(void* game)
{
  if(_GameRefCount.IsDefined(game))
    return _GameRefCount(game);
  else {
    _GameRefCount.Define(game, 0);
    return _GameRefCount(game);
  }
}


//----------------------------------------------------------------
//       function list sorting functions
//----------------------------------------------------------------


class gFuncListSorter : public gListSorter<gclFunction*>
{
protected:
  bool LessThan(gclFunction* const& a, gclFunction* const& b) const
  {
    return (a->FuncName() > b->FuncName());
  }
public:
  gFuncListSorter(void) { }
};

class gTextListSorter : public gListSorter<gText>
{
protected:
  bool LessThan(gText const& a, gText const& b) const
  {
    return (a > b);
  }
public:
  gTextListSorter(void) { }
};




//--------------------------------------------------------------------
//              implementation of GSM (Stack machine)
//--------------------------------------------------------------------

int GSM::_NumObj = 0;

GSM::GSM(gInput& s_in, gOutput& s_out, gOutput& s_err)
  : _StdIn(s_in), _StdOut(s_out), _StdErr(s_err)
{
  // global function default variables initialization
  // these should be done before InitFunctions() is called

  _RefTableStack = new gStack< RefHashTable* >(1);
  _RefTableStack->Push(new RefHashTable);
  _FuncNameStack = new gStack< gText >;

  _FuncTable     = new FunctionHashTable;
  InitFunctions();  // This function is located in gsmfunc.cc

  _NumObj++;
}


GSM::~GSM()
{
  _NumObj--;

  delete _FuncTable;

  while (_RefTableStack->Depth()) 
    delete _RefTableStack->Pop();
  delete _RefTableStack;

  delete _FuncNameStack;
}


bool GSM::VarIsDefined(const gText &p_name) const
{
  if (p_name == "")
    throw gclRuntimeError("Tried to see if empty variable name defined");

  else if (p_name[0u] == '$') {
    if (p_name[1u] == '$')
      return _GlobalRefTable.IsDefined(p_name.Right(p_name.Length() - 2));
    else
      return _GlobalRefTable.IsDefined(UserFuncName() + gText((char) 1) +
				       p_name.Right(p_name.Length() - 1));
  }

  return _RefTableStack->Peek()->IsDefined(p_name);
}


void GSM::VarDefine(const gText& var_name, Portion* p)
{
  bool type_match = true;
  Portion *old_value = 0;

  if (var_name == "")
    throw gclRuntimeError("Tried to define empty variable name");

  _ResolveRef(p);

  if (var_name[0u] == '$') {
    gText global_name;
    if (var_name[1u] != '$')
      global_name = (UserFuncName() + gText((char) 1) +
		     var_name.Right(var_name.Length() - 1));
    else
      global_name = var_name.Right(var_name.Length() - 2);

    if (_GlobalRefTable.IsDefined(global_name)) {
      old_value = _GlobalRefTable(global_name);
      if (p->Spec().ListDepth > 0) {
	if (((ListPortion*) old_value)->Spec().Type != 
	    ((ListPortion*) p)->Spec().Type)  {
	  if (((ListPortion*) p)->Spec().Type == porUNDEFINED)
	    ((ListPortion*) p)->SetDataType(old_value->Spec().Type);
	  else if (old_value->Spec().Type != porUNDEFINED)
	    type_match = false;
	}
      }
      else {
	PortionSpec ospec = old_value->Spec();
	PortionSpec pspec = p->Spec();
	if (ospec.Type == porNULL)
	  ospec = ((NullPortion*) old_value)->DataType();
	if (pspec.Type == porNULL)
	  pspec = ((NullPortion*) p)->DataType();
	if (ospec.Type != pspec.Type)
	  if (!PortionSpecMatch(ospec, pspec))
	    type_match = false;
      }
    }

    if (!type_match) {
      delete p;
      throw gclRuntimeError("Cannot change the type of variable \"" +
			    var_name + "\"");
    }
    else {
      if (old_value)
	delete _VarRemove(var_name);
      _GlobalRefTable.Define(global_name, p);
    }
  }    
  else  {
    if (_RefTableStack->Peek()->IsDefined(var_name)) {
      old_value = (*_RefTableStack->Peek())(var_name);
      if (p->Spec().ListDepth > 0) {
	if (((ListPortion*) old_value)->Spec().Type != 
	    ((ListPortion*) p)->Spec().Type)  {
	  if (((ListPortion*) p)->Spec().Type == porUNDEFINED)
	    ((ListPortion*) p)->SetDataType(old_value->Spec().Type);
	  else if (old_value->Spec().Type != porUNDEFINED)
	    type_match = false;
	}
      }
      else {
	PortionSpec ospec = old_value->Spec();
	PortionSpec pspec = p->Spec();
	if (ospec.Type == porNULL)
	  ospec = ((NullPortion*) old_value)->DataType();
	if (pspec.Type == porNULL)
	  pspec = ((NullPortion*) p)->DataType();
	if (ospec.Type != pspec.Type)
	  if (!PortionSpecMatch(ospec, pspec))
	    type_match = false;
      }
    }

    if (!type_match) {
      delete p;
      throw gclRuntimeError("Cannot change the type of variable \"" +
			    var_name + "\"");
    }
    else {
      if (old_value)
	delete _VarRemove(var_name);
      _RefTableStack->Peek()->Define(var_name, p);
    }
  }
}


Portion* GSM::VarValue(const gText &p_name) const
{
  if (p_name == "")
    throw gclRuntimeError("Tried to get value of empty variable name");

  else if (p_name[0u] == '$') {
    if (p_name[1u] == '$')
      return _GlobalRefTable(p_name.Right(p_name.Length() - 2));
    else
      return _GlobalRefTable(UserFuncName() + gText((char) 1) +
			     p_name.Right(p_name.Length() - 1));
  }

  return (*_RefTableStack->Peek())(p_name);
}


Portion* GSM::_VarRemove(const gText &p_name)
{
  if (p_name == "")
    throw gclRuntimeError("Tried to remove empty variable name");

  else if (p_name[0u] == '$') {
    if (p_name[1u] == '$')
      return _GlobalRefTable.Remove(p_name.Right(p_name.Length() - 2));
    else
      return _GlobalRefTable.Remove(UserFuncName() + gText((char) 1) +
				    p_name.Right(p_name.Length() - 1));
  }

  return _RefTableStack->Peek()->Remove(p_name);
}

//---------------------------------------------------------------------
//     Reference related functions
//---------------------------------------------------------------------


Portion* GSM::Assign( Portion* p1, Portion* p2 )
{
  Portion* result = 0;
  gText varname;

  if(p1->Spec().Type == porREFERENCE)
    varname = ((ReferencePortion*) p1)->Value();

  _ResolveRef( p2 );
  _ResolveRef( p1 );


  PortionSpec p1Spec(p1->Spec());
  PortionSpec p2Spec(p2->Spec());

  if(p1->Original() == p2->Original())  // assigning a variable to itself
  {
    delete p2;
    result = p1;
  }  
  else if(p2Spec.Type == porREFERENCE) // assigning from undefined variable
  {
    throw gclRuntimeError("Undefined variable " +
			      ((ReferencePortion *) p2)->Value());
  }
  else if(p1Spec.Type == porREFERENCE) // assigning to a new variable
  {    
    delete p1;
    if( p2->IsReference() )
    {
      VarDefine( varname, p2->ValCopy() );
      delete p2;
    }
    else
      VarDefine( varname, p2 );
    result = VarValue( varname )->RefCopy();
  }
  else if( p1Spec == p2Spec && 
	   p1Spec.Type != porNULL && 
	   !(p1Spec.Type & porMIXED) &&
	   !(p1Spec.Type & porBEHAV) )
  {
    if(p1Spec.ListDepth == 0) // not a list
    {
      result = p1;

      switch (p1Spec.Type) {
      case porNUMBER:
	((NumberPortion*) p1)->SetValue(((NumberPortion*) p2)->Value());
	break;
      case porTEXT:
	((TextPortion*) p1)->SetValue(((TextPortion*) p2)->Value());
	break;
      case porBOOLEAN:
	((BoolPortion*) p1)->SetValue(((BoolPortion*) p2)->Value());
	break;
      case porEFOUTCOME:
	((EfOutcomePortion *) p1)->SetValue(((EfOutcomePortion *) p2)->Value());
	break;
      case porNFOUTCOME:
	((NfOutcomePortion *) p1)->SetValue(((NfOutcomePortion *) p2)->Value());
	break;
      case porNFSUPPORT:
	((NfSupportPortion *) p1)->SetValue(new NFSupport(*((NfSupportPortion *) p2)->Value()));
	break;
      case porEFSUPPORT:
	((EfSupportPortion *) p1)->SetValue(new EFSupport(*((EfSupportPortion *) p2)->Value()));
	break;
      case porEFBASIS:
	((EfBasisPortion *) p1)->SetValue(new EFBasis(*((EfBasisPortion *) p2)->Value()));
	break;
      case porINFOSET:
	((InfosetPortion *) p1)->SetValue(((InfosetPortion *) p2)->Value());
	break;
      case porNFPLAYER:
	((NfPlayerPortion *) p1)->SetValue(((NfPlayerPortion *) p2)->Value());
	break;
      case porEFPLAYER:
	((EfPlayerPortion *) p1)->SetValue(((EfPlayerPortion *) p2)->Value());
	break;
      case porSTRATEGY:
	((StrategyPortion *) p1)->SetValue(((StrategyPortion *) p2)->Value());
	break;
      case porNODE:
	((NodePortion *) p1)->SetValue(((NodePortion *) p2)->Value());
	break;
      case porACTION:
	((ActionPortion *) p1)->SetValue((Action *)((ActionPortion *) p2)->Value());
	break;
      case porNFG:
	((NfgPortion *) p1)->SetValue(((NfgPortion *) p2)->Value());
	break;
      case porEFG:
	((EfgPortion *) p1)->SetValue(((EfgPortion *) p2)->Value());
	break;
      case porINPUT:
      case porOUTPUT:
	delete p1;
	throw gclRuntimeError("Cannot assign from INPUT/OUTPUT variable" );
      default:
	throw gclRuntimeError("Assigning to unknown type " +
			      PortionSpecToText(p1Spec));
      }

      delete p2;
      // result already assigned on top of this block
    }
    // both p1 and p2 are lists
    else if((p1Spec.Type == p2Spec.Type) ||
	    (p1Spec.Type == porUNDEFINED))
    {
      if(!(p1Spec.Type & (porINPUT|porOUTPUT)))
      {
	((ListPortion*) p1)->AssignFrom(p2);
	delete p2;
	result = p1;
      }
      else // error: assigning to (list of) INPUT or OUTPUT
      {
	delete p2;
	delete p1;
	throw gclRuntimeError("Cannot assign from INPUT/OUTPUT variable" );
      }
    }
    else // error: changing the type of a list
    {
      delete p2;
      delete p1;
      throw gclRuntimeError("Cannot change list type");
    }
  }
  else if(varname != "") // make sure variable is associated with a var name
  {
    if(p1Spec.Type == porNULL)
      p1Spec = ((NullPortion*) p1)->DataType();
    if(p2Spec.Type == porNULL)
      p2Spec = ((NullPortion*) p2)->DataType();
    
    if(PortionSpecMatch(p1Spec, p2Spec) ||
       (p1Spec.Type == porUNDEFINED && p1Spec.ListDepth > 0 &&
	p2Spec.ListDepth > 0))
    {
      delete p1;
      if( p2->IsReference() )
      {
	VarDefine( varname, p2->ValCopy() );
	delete p2;
      }
      else
	VarDefine( varname, p2 );
      result = VarValue( varname )->RefCopy();
    }
    else { // error: changing the type of variable
      delete p2;
      delete p1;
      throw gclRuntimeError("Cannot change the type of variable \"" +
			    varname + "\" from " +
			    PortionSpecToText(p1Spec) + " to " +
			    PortionSpecToText(p2Spec));
    }
  }
  else {
    delete p2;
    delete p1;
    throw gclRuntimeError("Must assign to a variable");
  }

  return result;
}





bool GSM::UnAssign(Portion *p)
{
  if (p->Spec().Type == porREFERENCE) {
    if(VarIsDefined(((ReferencePortion*) p)->Value())) {
      delete _VarRemove(((ReferencePortion *) p)->Value());
      delete p;
      return true;
    }
    else {
      delete p;
      return true;
    }
  }
  else {
    throw gclRuntimeError("UnAssign[] called on a non-reference value");
  }
}


Portion* GSM::UnAssignExt(Portion *p)
{
  gText txt;

  if(p->Spec().Type == porREFERENCE)
  {
    gText varname = ((ReferencePortion*) p)->Value(); 
    if(VarIsDefined( varname ) )
    {
      delete p;
      delete _VarRemove( varname );
      return new BoolPortion(triTRUE);
    }
    else
    {
      delete p;
      return new BoolPortion(triFALSE);
    }
  }
  else
  {
    throw gclRuntimeError("UnAssign[] called on a non-reference value");
  }
}


bool GSM::VarRemove(const gText& var_name)
{
  if(VarIsDefined(var_name))
    _VarRemove(var_name);
  return true;
}


//-----------------------------------------------------------------------
//                        _ResolveRef functions
//-----------------------------------------------------------------------

void GSM::_ResolveRef(Portion *&p)
{
  if (p->Spec().Type == porREFERENCE) {
    gText ref = ((ReferencePortion*) p)->Value();

    if (VarIsDefined(ref)) {
      Portion *result = VarValue(ref)->RefCopy();
      delete p;
      p = result;
    }
  }
}


//-------------------------------------------------------------------
//               CallFunction() related functions
//-------------------------------------------------------------------

bool GSM::AddFunction(gclFunction* func)
{
  gclFunction *old_func;
  bool result;
  if (func == 0)  return false;
  if(!_FuncTable->IsDefined(func->FuncName()))
  {
    _FuncTable->Define(func->FuncName(), func);
    return true;
  }
  else {
    old_func = (*_FuncTable)(func->FuncName());
    result = old_func->Combine(func);
    if (!result)
      throw gclRuntimeError("New " + old_func->FuncName() +
			    "[] ambiguous with existing function");
    return result;
  }
}


bool GSM::DeleteFunction(gclFunction* func)
{
  gclFunction *old_func = 0;
  if (func == 0)  return 0;
  if (!_FuncTable->IsDefined(func->FuncName())) {
    throw gclRuntimeError("Function " + old_func->FuncName() + " not found");
  }
  else {
    old_func = (*_FuncTable)(func->FuncName());
    if (!old_func->Delete(func))
      throw gclRuntimeError("No matching prototype found");
  }
  return true;
}




//----------------------------------------------------------------------------
//                       Execute function
//----------------------------------------------------------------------------

Portion *GSM::Execute(gclExpression *expr, bool /*user_func*/)
{
  return expr->Evaluate(*this);
}


Portion* GSM::ExecuteUserFunc(gclExpression& program, 
			      const gclSignature& func_info,
			      Portion** param, 
			      const gText& funcname )
{
  Portion *result, *result_copy;

  _RefTableStack->Push(new RefHashTable);
  _FuncNameStack->Push( funcname );

  for (int i = 0; i < func_info.NumParams; i++) {
    if (param[i] != 0 && param[i]->Spec().Type != porREFERENCE) {
      VarDefine(func_info.ParamInfo[i].Name, param[i]);
      param[i] = param[i]->RefCopy();
    }
  }


  try {
    result = Execute(&program, true);
  
    if (result)   {
      _ResolveRef(result);

      if (result->Spec().Type == porREFERENCE) {
	throw gclRuntimeError("Reference to unknown variable " +
			      ((ReferencePortion *) result)->Value() +
			      " returned from function " +
			      UserFuncName());
      }

      if (result->IsReference())  {
	result_copy = result->ValCopy();
	delete result;
	result = result_copy;
      }
      
      for (int i = 0; i < func_info.NumParams; i++) {
	if (func_info.ParamInfo[i].PassByReference) {
	  if (VarIsDefined(func_info.ParamInfo[i].Name)) {
	    delete param[i];
	    param[i] = _VarRemove(func_info.ParamInfo[i].Name);
	  }
	}
      }
    
      delete _RefTableStack->Pop();
      _FuncNameStack->Pop();
    }
  }
  // The reason for this dual handler is that Borland apparently has
  // some compiler bug with rethrowing exceptions.  When the throw
  // above was introduced to check for returning undefined variables
  // crashes ensued.  This seems to fix matters.  -- magyar, 3/18/99
  catch (gclRuntimeError &E) {
    delete _RefTableStack->Pop();
    _FuncNameStack->Pop();
    throw E;
  }
  catch (...) {
    delete _RefTableStack->Pop();
    _FuncNameStack->Pop();
    throw;
  }

  return result;
}



gText GSM::UserFuncName( void ) const
{
  if( _FuncNameStack->Depth() > 0 )
    return _FuncNameStack->Peek();
  else
    return "";
}



//----------------------------------------------------------------------------
//                   miscellaneous functions
//----------------------------------------------------------------------------


void GSM::Clear(void)
{
  while (_RefTableStack->Depth() > 0)
    delete _RefTableStack->Pop();

  _RefTableStack->Push(new RefHashTable);
}



Portion* GSM::Help(gText funcname, bool udf, bool bif, bool getdesc)
{
  int i;
  int j;
  unsigned int fk, ck;
  int cfk;
  bool match;
  int found = 0;
  gText curname;
  const gList<gclFunction*>* funcs = _FuncTable->Value();
  gclFunction *func;
  gList<gclFunction*> funclist;
  gSortList<gclFunction*> funcslist;
  Portion* result;

  if(_FuncTable->IsDefined(funcname))
  {
    func = (*_FuncTable)(funcname);
    gList<gText> list = func->FuncList( udf, bif, getdesc );
    result = new ListPortion();
    for(i=1; i<=list.Length(); i++)
      ((ListPortion*) result)->Append(new TextPortion(list[i]));
  }
  else
  {
    funcname = funcname.Dncase();
    for(i=0; i<_FuncTable->NumBuckets(); i++)
      for(j=1; j<=funcs[i].Length(); j++)
	funclist.Append(funcs[i][j]);

    for(i=1; i<=funclist.Length(); i++)
    {
      match = true;
      curname = funclist[i]->FuncName().Dncase();
      fk = 0; 
      ck = 0;
      cfk = -1;
      while(match && (fk <funcname.Length()) && (ck<curname.Length()))
      {
	if(funcname[fk]=='*')
	{
	  if(fk+1==funcname.Length())
	    break;
	  cfk = fk;
	  fk++;
	  while(ck<curname.Length() && funcname[fk]!=curname[ck])
	    ck++;
	  if(ck==curname.Length())
	  { match = false; break; }	  
	}
	
	if(funcname[fk]==curname[ck])
	{ fk++; ck++; }
	else if(funcname[fk]=='?')
	{ fk++; ck++; }
	else
	{
	  if(cfk<0)
	    match = false;
	  else
	  { fk = cfk; }
	}
      }

      if((fk>=funcname.Length()) != (ck>=curname.Length()))	
	match = false;
      if(fk+1==funcname.Length() && funcname[fk]=='*')
	match = true;
      if(match)
      {	
	if( (udf && funclist[i]->UDF()) ||
	   (bif && funclist[i]->BIF()) )
	{
	  func = funclist[i];
	  funcslist.Append(func);
	  found++;
	}
      }      
    }

    gFuncListSorter sorter;
    if(found==1)
    {
      gList<gText> list = func->FuncList( udf, bif, getdesc );
      result = new ListPortion();
      for(i=1; i<=list.Length(); i++)
	((ListPortion*) result)->Append(new TextPortion(list[i]));
    }
    else
    {
      sorter.Sort(funcslist);
      result = new ListPortion();
      for(i=1; i<=funcslist.Length(); i++)
	((ListPortion*) result)->
	  Append(new TextPortion(funcslist[i]->FuncName()));
    }
  }

  if(!result)
    throw gclRuntimeError("No match found");
  return result;
}


Portion* GSM::HelpVars(gText varname)
{
  int i;
  int j;
  unsigned int fk, ck;
  int cfk;
  bool match;
  gText curname;
  const gList<gText>* vars = _RefTableStack->Peek()->Key();
  gText var;
  gList<gText> varlist;
  gSortList<gText> varslist;
  Portion* result;

  if(_RefTableStack->Peek()->IsDefined(varname))
  {
    result = new ListPortion();
    ((ListPortion*) result)->Append(new TextPortion(varname + ":" + 
      PortionSpecToText((*(_RefTableStack->Peek()))(varname)->Spec())));
  }
  else
  {
    varname = varname.Dncase();
    for(i=0; i<_RefTableStack->Peek()->NumBuckets(); i++)
      for(j=1; j<=vars[i].Length(); j++)
	varlist.Append(vars[i][j]);

    for(i=1; i<=varlist.Length(); i++)
    {
      match = true;
      curname = varlist[i].Dncase();
      fk = 0; 
      ck = 0;
      cfk = -1;
      while(match && (fk<varname.Length()) && (ck<curname.Length()))
      {
	if(varname[fk]=='*')
	{
	  if(fk+1==varname.Length())
	    break;
	  cfk = fk;
	  fk++;
	  while(ck<curname.Length() && varname[fk]!=curname[ck])
	    ck++;
	  if(ck==curname.Length())
	  { match = false; break; }	  
	}
	
	if(varname[fk]==curname[ck])
	{ fk++; ck++; }
	else if(varname[fk]=='?')
	{ fk++; ck++; }
	else
	{
	  if(cfk<0)
	    match = false;
	  else
	  { fk = cfk; }
	}
      }

      if((fk>=varname.Length()) != (ck>=curname.Length()))	
	match = false;
      if(fk+1==varname.Length() && varname[fk]=='*')
	match = true;
      if (match) {
	var = varlist[i];
	varslist.Append(var);
      }      
    }

    gTextListSorter sorter;
    sorter.Sort(varslist);
    result = new ListPortion();
    for(i=1; i<=varslist.Length(); i++)
      ((ListPortion*) result)->Append(new TextPortion(varslist[i] + ":" + 
	PortionSpecToText((*(_RefTableStack->Peek()))(varslist[i])->Spec())));
  }

  if(!result)
    throw gclRuntimeError("No match found");      
  return result;
}





//-------------------------
// InvalidateGameProfile
//-------------------------

void GSM::InvalidateGameProfile( void* game, bool IsEfg )
{
  gStack< RefHashTable* > tempRefTableStack;

  while( _RefTableStack->Depth() > 0 )
  {
    const gList<Portion*>* vars = _RefTableStack->Peek()->Value();


    gList<Portion*> varslist;
    
    for (int i = 0; i<_RefTableStack->Peek()->NumBuckets(); i++)
      for (int j = 1; j<=vars[i].Length(); j++)
	varslist.Append(vars[i][j]);

    for (int i = 1; i <= varslist.Length(); i++ )
    {
      if( varslist[i]->Game() == game && varslist[i]->GameIsEfg() == IsEfg )
      {
	if (!IsEfg && varslist[i]->Spec() == porMIXED)  
	  ((MixedPortion *) varslist[i])->Value()->Invalidate();
	else if (IsEfg && varslist[i]->Spec() == porBEHAV)
	  ((BehavPortion *) varslist[i])->Value()->Invalidate();
      }
    }

    // go through all scopes on the stack; restore later
    tempRefTableStack.Push( _RefTableStack->Pop() );
  }
  
  while( tempRefTableStack.Depth() > 0 )
  {
    // restore the original variable stack
    _RefTableStack->Push( tempRefTableStack.Pop() );    
  }

}



//------------------------
// UnAssignGameElement
//------------------------

void GSM::UnAssignGameElement( void* game, bool /*IsEfg*/, PortionSpec spec )
{
  if (spec.ListDepth > 0)
    return;

  gStack< RefHashTable* > tempRefTableStack;

  while( _RefTableStack->Depth() > 0 )
  {

    const gList<Portion*>* vars = _RefTableStack->Peek()->Value();
    gList<Portion*> varslist;
    
    for (int i=0; i<_RefTableStack->Peek()->NumBuckets(); i++)
      for (int j=1; j<=vars[i].Length(); j++)
	varslist.Append(vars[i][j]);

    for (int i = 1; i <= varslist.Length(); i++ )
    {
      if( varslist[i]->Spec().ListDepth == 0 )
      {
	if( varslist[i]->Game() == game )
	{
	  if( varslist[i]->Spec().Type & spec.Type )
	  {
	    _RefTableStack->Peek()->Remove( varslist[i] );
	  }
	}
      }
      else
      {
	if( ((ListPortion*) varslist[i])->BelongsToGame( game ) )
	{
	  if( varslist[i]->Spec().Type & spec.Type )
	  {
	    _RefTableStack->Peek()->Remove( varslist[i] );
	  }
	}
      }
    }

    // go through all scopes on the stack; restore later
    tempRefTableStack.Push( _RefTableStack->Pop() );
  }
  
  while( tempRefTableStack.Depth() > 0 )
  {
    // restore the original variable stack
    _RefTableStack->Push( tempRefTableStack.Pop() );    
  }
}


//---------------------
// UnAssignEfgElement
//---------------------

void GSM::UnAssignEfgElement( efgGame *game, PortionSpec spec, void* data )
{
  gStack< RefHashTable* > tempRefTableStack;

  while( _RefTableStack->Depth() > 0 )
  {

    const gList<Portion*>* vars = _RefTableStack->Peek()->Value();
    gList<Portion*> varslist;
    
    for (int i=0; i<_RefTableStack->Peek()->NumBuckets(); i++)
      for (int j=1; j<=vars[i].Length(); j++)
	varslist.Append(vars[i][j]);

    for (int i = 1; i <= varslist.Length(); i++ )
    {
      if( varslist[i]->Spec().ListDepth == 0 )
      {
	if( varslist[i]->Game() == game )
	{
	  if( spec.Type & porEFSUPPORT )
	  {
	    if( ((EfSupportPortion*) varslist[i])->Value() == data )
	      _RefTableStack->Peek()->Remove( varslist[i] );
	  }
	  if( spec.Type & porEFBASIS )
	  {
	    if( ((EfBasisPortion*) varslist[i])->Value() == data )
	      _RefTableStack->Peek()->Remove( varslist[i] );
	  }
	  if( spec.Type & porEFPLAYER )
	  {
	    if( ((EfPlayerPortion*) varslist[i])->Value() == data )
	      _RefTableStack->Peek()->Remove( varslist[i] );
	  }
	  if( spec.Type & porINFOSET )
	  {
	    if( ((InfosetPortion*) varslist[i])->Value() == data )
	      _RefTableStack->Peek()->Remove( varslist[i] );
	  }
	  if( spec.Type & porNODE )
	  {
	    if( ((NodePortion*) varslist[i])->Value() == data )
	      _RefTableStack->Peek()->Remove( varslist[i] );
	  }
	  if( spec.Type & porACTION )
	  {
	    if( ((ActionPortion*) varslist[i])->Value() == data )
	      _RefTableStack->Peek()->Remove( varslist[i] );
	  }
	}
      }
      else // varslist[i] is a list
      {
	if( spec.Type & varslist[i]->Spec().Type )
	{
	  if( ((ListPortion*) varslist[i])->MatchGameData( game, data ) )
	  {
	    _RefTableStack->Peek()->Remove( varslist[i] );	
	  }
	}
      }
    }

    // go through all scopes on the stack; restore later
    tempRefTableStack.Push( _RefTableStack->Pop() );
  }
  
  while( tempRefTableStack.Depth() > 0 )
  {
    // restore the original variable stack
    _RefTableStack->Push( tempRefTableStack.Pop() );    
  }
}

void GSM::UnAssignEfgOutcome(efgGame *game, const efgOutcome *data)
{
  gStack< RefHashTable* > tempRefTableStack;

  while (_RefTableStack->Depth() > 0) {
    const gList<Portion*>* vars = _RefTableStack->Peek()->Value();
    gList<Portion*> varslist;
    
    for (int i=0; i<_RefTableStack->Peek()->NumBuckets(); i++)
      for (int j=1; j<=vars[i].Length(); j++)
	varslist.Append(vars[i][j]);

    for (int i = 1; i <= varslist.Length(); i++) {
      if (varslist[i]->Spec().ListDepth == 0) {
	if (varslist[i]->Game() == game &&
	    varslist[i]->Spec() == porEFOUTCOME) {
	  if (((EfOutcomePortion*) varslist[i])->Value() == data) {
	    _RefTableStack->Peek()->Remove(varslist[i]);
	  }
	}
      }
#ifdef FIXME
      // FIXME: need to check lists
      else  { // varslist[i] is a list
	if (spec.Type & varslist[i]->Spec().Type) {
	  if (((ListPortion*) varslist[i])->MatchGameData( game, data ))  {
	    _RefTableStack->Peek()->Remove( varslist[i] );	
	  }
	}
      }
#endif // FIXME
    }

    // go through all scopes on the stack; restore later
    tempRefTableStack.Push( _RefTableStack->Pop() );
  }
  
  while (tempRefTableStack.Depth() > 0) {
    // restore the original variable stack
    _RefTableStack->Push(tempRefTableStack.Pop());    
  }
}



void GSM::UnAssignEfgInfoset(efgGame * game, Infoset* infoset )
{
  for (int i = 1; i <= infoset->NumActions(); i++ )
    UnAssignEfgElement( game, porACTION, (Action *)infoset->Actions()[i] );
  UnAssignEfgElement( game, porINFOSET, infoset );  
}


void GSM::UnAssignEfgSubTree( efgGame * game, Node* node )
{
  for (int i = 1; i <= game->NumChildren(node); i++)  {
    Infoset* infoset = node->GetInfoset();
    if (infoset) {
      const gArray<Action *>& actions = infoset->Actions();
      for (int j = actions.First(); j <= actions.Last(); j++ )
	UnAssignEfgElement( game, porACTION, (Action *)actions[j] );
      UnAssignEfgElement( game, porINFOSET, infoset );
    }
    UnAssignEfgSubTree( game, node->GetChild( i ) );
  }
  UnAssignEfgElement( game, porNODE, node );
}


void GSM::GlobalVarDefine     ( const gText& var_name, Portion* p )
{
  if (var_name == "")
    throw gclRuntimeError("Attempted to define empty variable name");
  if( GlobalVarIsDefined( var_name ) )
    GlobalVarRemove( var_name );
  _GlobalRefTable.Define(var_name, p);
}

bool GSM::GlobalVarIsDefined  ( const gText& var_name ) const
{
  if (var_name == "")
    throw gclRuntimeError("Attempted to define empty variable name");
  return _GlobalRefTable.IsDefined(var_name);
}

Portion* GSM::GlobalVarValue  ( const gText& var_name ) const
{
  if (var_name == "")
    throw gclRuntimeError("Attempted to get value of empty variable name");
  return _GlobalRefTable(var_name);
}

void GSM::GlobalVarRemove     ( const gText& var_name )
{
  if (var_name == "")
    throw gclRuntimeError("Attempted to remove empty variable name");
  delete _GlobalRefTable.Remove(var_name);
}

void GSM::StartAlgorithmMonitor(const gText &)
{ }

void GSM::EndAlgorithmMonitor(void)
{
  GetStatusMonitor().Reset();
}

gclRuntimeError::gclRuntimeError(const gText &s)
  : message(s)
{ }

gclRuntimeError::~gclRuntimeError()
{ }

gText gclRuntimeError::Description(void) const
{ return message; }

