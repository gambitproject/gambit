//
// FILE: gsm.cc  implementation of GSM (Stack machine)
//
// $Id$
//


class Portion;
class CallFuncObj;
class RefHashTable;
template <class T> class gStack;

//-----------------------------------------------------------------------
//                       Template instantiations
//-----------------------------------------------------------------------

#include "gstack.imp"

template class gStack< Portion* >;
template class gStack< gStack< Portion* >* >;
template class gStack< CallFuncObj* >;
template class gStack< RefHashTable* >;

#include "garray.imp"
#include "gslist.imp"
class FuncDescObj;
template class gSortList<FuncDescObj*>;
template class gListSorter<FuncDescObj*>;
#include "gstring.h"
template class gSortList<gString>;
template class gListSorter<gString>;



#include "gsm.h"

#include <assert.h>

#include "glist.h"
#include "gstack.h"

#include "portion.h"
#include "gsmhash.h"
#include "gsmfunc.h"
#include "gsminstr.h"

#include "gblock.h"

#include "nfg.h"
#include "rational.h"
#include "mixedsol.h"
#include "behavsol.h"

#include "gslist.h"
#include "garray.h"




//----------------------------------------------------------------
//        declaration of the game reference counter
//----------------------------------------------------------------

RefCountHashTable< void* > GSM::_GameRefCount;
int& GSM::GameRefCount(void* game)
{
  if(_GameRefCount.IsDefined(game))
    return _GameRefCount(game);
  else
  {
    _GameRefCount.Define(game, 0);
    return _GameRefCount(game);
  }
}


//----------------------------------------------------------------
//       function list sorting functions
//----------------------------------------------------------------


class gFuncListSorter : public gListSorter<FuncDescObj*>
{
protected:
  CompareResult Compare(FuncDescObj* const& a, FuncDescObj* const& b) const
  {
    if((a->FuncName()) < (b->FuncName()))
      return GreaterThan;
    else if((a->FuncName()) > (b->FuncName()))
      return LessThan;
    else
      return Equal;
  }
public:
  gFuncListSorter(gSortList<FuncDescObj*>& list)
    : gListSorter<FuncDescObj*>(list)
    {}
};

class gTextListSorter : public gListSorter<gString>
{
protected:
  CompareResult Compare(gString const& a, gString const& b) const
  {
    if(a < b)
      return GreaterThan;
    else if(a > b)
      return LessThan;
    else
      return Equal;
  }
public:
  gTextListSorter(gSortList<gString>& list)
    : gListSorter<gString>(list)
    {}
};




//--------------------------------------------------------------------
//              implementation of GSM (Stack machine)
//--------------------------------------------------------------------

int GSM::_NumObj = 0;

GSM::GSM(int size, gInput& s_in, gOutput& s_out, gOutput& s_err)
:_Verbose(true), _StdIn(s_in), _StdOut(s_out), _StdErr(s_err)
{
#ifndef NDEBUG
  if(size <= 0)
  {
    gerr << "  Illegal stack size specified during initialization\n";
  }
  assert(size > 0);
#endif // NDEBUG
  
  // global function default variables initialization
  // these should be done before InitFunctions() is called

  _StackStack    = new gStack< gStack< Portion* >* >(1);
  _StackStack->Push(new gStack< Portion* >(size));
  _CallFuncStack = new gStack< CallFuncObj* >(1);
  _RefTableStack = new gStack< RefHashTable* >(1);
  _RefTableStack->Push(new RefHashTable);

  _FuncTable     = new FunctionHashTable;
  InitFunctions();  // This function is located in gsmfunc.cc

  _NumObj++;
}


GSM::~GSM()
{
  _NumObj--;

  assert(_CallFuncStack->Depth() == 0);
  delete _CallFuncStack;

  delete _FuncTable;

  assert(_RefTableStack->Depth() == 1);
  delete _RefTableStack->Pop();
  delete _RefTableStack;

  assert(_StackStack->Depth() == 1);
  delete _StackStack->Pop();
  delete _StackStack;

}




//--------------------------------------------------------------------
//        Stack access related functions
//--------------------------------------------------------------------



bool GSM::VarIsDefined(const gString& var_name) const
{
  assert(var_name != "");

  return _RefTableStack->Peek()->IsDefined(var_name);
}


bool GSM::VarDefine(const gString& var_name, Portion* p)
{
  Portion* old_value = 0;
  bool type_match = true;
  bool result = true;

  assert(var_name != "");

  p = _ResolveRef(p);

  if(_RefTableStack->Peek()->IsDefined(var_name))
  {
    old_value = (*_RefTableStack->Peek())(var_name);
    if(p->Spec().ListDepth > 0)
    {
      assert(old_value->Spec().ListDepth > 0);
      if(((ListPortion*) old_value)->Spec().Type != 
	 ((ListPortion*) p)->Spec().Type)
      {
	if(((ListPortion*) p)->Spec().Type == porUNDEFINED)
	  ((ListPortion*) p)->SetDataType(old_value->Spec().Type);
	else if(old_value->Spec().Type != porUNDEFINED)
	  type_match = false;
      }
    }
    else
    {      
      PortionSpec ospec = old_value->Spec();
      PortionSpec pspec = p->Spec();
      if(ospec.Type == porNULL)
	ospec = ((NullPortion*) old_value)->DataType();
      if(pspec.Type == porNULL)
	pspec = ((NullPortion*) p)->DataType();
      if(ospec.Type != pspec.Type)
	if(!PortionSpecMatch(ospec, pspec))
	  type_match = false;
    }
  }

  if(!type_match)
  {
    _ErrorMessage(_StdErr, 42, 0, 0, var_name);
    delete p;
    result = false;
  }
  else
  {
    if(old_value)
      delete _VarRemove(var_name);
    _RefTableStack->Peek()->Define(var_name, p);
  }
  return result;
}


Portion* GSM::VarValue(const gString& var_name) const
{
  assert(var_name != "");
  return (*_RefTableStack->Peek())(var_name);
}


Portion* GSM::_VarRemove(const gString& var_name)
{
  Portion* p;

  assert(var_name != "");

  p = _RefTableStack->Peek()->Remove(var_name);
  assert(p);
  return p;
}

//---------------------------------------------------------------------
//     Reference related functions
//---------------------------------------------------------------------


bool GSM::Assign(Portion *p1, Portion *p2)
{
  Portion* p_old;
  bool result = true;
  gString varname;

  if(p1->Spec().Type == porREFERENCE)
    varname = ((ReferencePortion*) p1)->Value();

  p2 = _ResolveRef(p2);
  p1 = _ResolveRef(p1);

  // deals with problems with assigning a variable to itself
  if(p1->Original() == p2->Original())
  {
    delete p2;
    return p1;
  }

  PortionSpec p1Spec(p1->Spec());
  PortionSpec p2Spec(p2->Spec());
  
  if(p2Spec.Type == porREFERENCE)
  {
    _ErrorMessage(_StdErr, 63, 0, 0, ((ReferencePortion*) p2)->Value());
    result = false;
  }
  else if(p1Spec.Type == porREFERENCE)
  {
    if(p2->IsReference())
    {
      p_old = p2;
      p2 = p2->ValCopy();
      delete p_old;
    }      
    delete p1;
/*
    if( _VarDefine(varname, p2) )
      _Push(p2->RefCopy());
    else
      _Push( new ErrorPortion );
    */
  }
  else if( p1Spec == p2Spec && 
	   p1Spec.Type != porNULL && 
	   !(p1Spec.Type & porMIXED) &&
	   !(p1Spec.Type & porBEHAV) )
  {
    if(p1Spec.ListDepth == 0) // not a list
    {
      result = true;
      switch(p1Spec.Type)
      {
      case porINTEGER:
	((IntPortion*) p1)->Value() = ((IntPortion*) p2)->Value();
	break;
      case porNUMBER:
	((NumberPortion*) p1)->Value()=((NumberPortion*) p2)->Value();
	break;
      case porTEXT:
	((TextPortion*) p1)->Value() = ((TextPortion*) p2)->Value();
	break;
      case porBOOL:
	((BoolPortion*) p1)->Value() = ((BoolPortion*) p2)->Value();
	break;

      case porEFOUTCOME:
	((EfOutcomePortion *) p1)->SetValue(((EfOutcomePortion *) p2)->Value());
	break;

      case porNFSUPPORT:
	((NfSupportPortion *) p1)->SetValue(new NFSupport(*((NfSupportPortion *) p2)->Value()));
	break;

      case porEFSUPPORT:
	((EfSupportPortion *) p1)->SetValue(new EFSupport(*((EfSupportPortion *) p2)->Value()));
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
	((ActionPortion *) p1)->SetValue(((ActionPortion *) p2)->Value());
	break;

      case porNFG:
	((NfgPortion *) p1)->SetValue(((NfgPortion *) p2)->Value());
	break;

      case porEFG:
	((EfgPortion *) p1)->SetValue(((EfgPortion *) p2)->Value());
	break;

      case porINPUT:
      case porOUTPUT:
	_ErrorMessage(_StdErr, 64);
	result = false;
	break;

      default:
	_ErrorMessage(_StdErr, 67, 0, 0, PortionSpecToText(p1Spec));
	assert(0);
      }
      if(result)
      {
//	_Push(p1);
	delete p2;
      }
    }
    // both p1 and p2 are lists
    else if((p1Spec.Type == p2Spec.Type) ||
	    (p1Spec.Type == porUNDEFINED))
    {
      if(!(p1Spec.Type & (porINPUT|porOUTPUT)))
      {
	((ListPortion*) p1)->AssignFrom(p2);
//	_Push(p1);
	delete p2;
      }
      else // error: assigning to (list of) INPUT or OUTPUT
      {
	_ErrorMessage(_StdErr, 64);
	result = false;
      }
    }
    else // error: changing the type of a list
    {
      _ErrorMessage(_StdErr, 65);
      result = false;
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
      if(p2->IsReference())
      {
	p_old = p2;
	p2 = p2->ValCopy();
	delete p_old;
      }
      assert(varname != "");
      delete p1;
/*
      if( _VarDefine(varname, p2) )
	_Push(p2->RefCopy());
      else
	_Push( new ErrorPortion );
      */
    }
    else // error: changing the type of variable
    {
      _ErrorMessage(_StdErr, 66, 0, 0, varname, 
		    PortionSpecToText(p1Spec),
		    PortionSpecToText(p2Spec));
      result = false;
    }
  }
  else
  {
    _ErrorMessage(_StdErr, 71);
    result = false;
  }

//  if(!result)
//  { delete p2; delete p1; }

  return result;
}





bool GSM::UnAssign(Portion *p)
{
  if(p->Spec().Type == porREFERENCE)
  {
    if(VarIsDefined(((ReferencePortion*) p)->Value()))
    {
      delete _VarRemove(((ReferencePortion *) p)->Value());
      delete p;
//      _Push(new BoolPortion(true));
      return true;
    }
    else
    {
      delete p;
//      _Push(new BoolPortion(false));
      return true;
    }
  }
  else
  {
//    _Push(p);
    _ErrorMessage(_StdErr, 53);
    return false;
  }
}


Portion* GSM::UnAssignExt(Portion *p)
{
  gString txt;

  if(p->Spec().Type == porREFERENCE)
  {
    gString varname = ((ReferencePortion*) p)->Value(); 
    if(VarIsDefined( varname ) )
    {
      delete p;
      delete _VarRemove( varname );
      return new BoolPortion(true);
    }
    else
    {
      delete p;
      return new BoolPortion(false);
    }
  }
  else
  {
//    _Push(p);
    txt = "UnAssign[] called on a non-reference value";
    return new ErrorPortion(txt);
  }
}


//-----------------------------------------------------------------------
//                        _ResolveRef functions
//-----------------------------------------------------------------------

Portion* GSM::_ResolveRef(Portion* p)
{
  Portion*  result = 0;
  gString ref;
  
  if(p->Spec().Type == porREFERENCE)
  {
    ref = ((ReferencePortion*) p)->Value();

    if(!VarIsDefined(ref))
    {
      result = p;
    }
    else
    {
      result = VarValue(ref)->RefCopy();
      delete p;

      /* temporary
      if(_VarValue(ref)->IsValid())
      {
	result = _VarValue(ref)->RefCopy();
	delete p;
      }
      else
      {
	delete _VarRemove(ref);
	result = p;
      }
      */
    }
  }
  else
  {
    result = p;
  }
  return result;
}


//-------------------------------------------------------------------
//               CallFunction() related functions
//-------------------------------------------------------------------

bool GSM::AddFunction(FuncDescObj* func)
{
  FuncDescObj *old_func;
  bool result;
  assert(func != 0);
  if(!_FuncTable->IsDefined(func->FuncName()))
  {
    _FuncTable->Define(func->FuncName(), func);
    return true;
  }
  else
  {
    old_func = (*_FuncTable)(func->FuncName());
    result = old_func->Combine(func);
    if(!result)
      _ErrorMessage(_StdErr, 60, 0, 0, old_func->FuncName());
    return result;
  }
}


bool GSM::DeleteFunction(FuncDescObj* func)
{
  FuncDescObj *old_func = 0;
  bool result;
  assert(func != 0);
  if(!_FuncTable->IsDefined(func->FuncName()))
  {
    _ErrorMessage(_StdErr, 73, 0, 0, old_func->FuncName());
    return false;
  }
  else
  {
    old_func = (*_FuncTable)(func->FuncName());
    result = old_func->Delete(func);
    if(!result)
      _ErrorMessage(_StdErr, 72, 0, 0, old_func->FuncName());
    return result;
  }
}




//----------------------------------------------------------------------------
//                       Execute function
//----------------------------------------------------------------------------

Portion *GSM::Execute(gclExpression *expr, bool /*user_func*/)
{
  Portion*        p               = 0;

  p = expr->Evaluate();

  return p;
}


Portion* GSM::ExecuteUserFunc(gclExpression& program, 
			      const FuncInfoType& func_info,
			      Portion** param)
{
  int rc_result;
  Portion* result;
  Portion* result_copy;
  int i;

  _RefTableStack->Push(new RefHashTable);
  _StackStack->Push(new gStack< Portion* >);


  for(i = 0; i < func_info.NumParams; i++)
  {
    if(param[i] != 0 && param[i]->Spec().Type != porREFERENCE)
    {
      if( VarDefine(func_info.ParamInfo[i].Name, param[i]) )
	param[i] = param[i]->RefCopy();
      else
	param[i] = new ErrorPortion;
    }
  }


  result = Execute(&program, true);

  rc_result = rcSUCCESS;

  switch(rc_result)
  {
  case rcSUCCESS:
    result = _ResolveRef(result);
    result_copy = result->ValCopy();
    delete result;
    result = result_copy;
    result_copy = 0;
    if(result->Spec().Type == porERROR)  {
      gout << result << '\n'; 
//      delete result;
//      result = 0;
    }
    break;
  case rcQUIT:
    result = 
      new ErrorPortion((gString)
		       "Error: Interruption by user");
    break;

  default:
    if(rc_result >= 0)
      result = new ErrorPortion((gString)
				"Error at line " +
				ToString(rc_result / 65536) + 
				" in function, line " +
				ToString(rc_result % 65536) +
				" in source code");
    else
      result = 0;
    break;
    }


  for(i = 0; i < func_info.NumParams; i++)
  {
    if(func_info.ParamInfo[i].PassByReference)
    {
      if(VarIsDefined(func_info.ParamInfo[i].Name))
      {
	assert(VarValue(func_info.ParamInfo[i].Name) != 0);
	delete param[i];
	param[i] = _VarRemove(func_info.ParamInfo[i].Name);
      }
    }
  }


  delete _StackStack->Pop();
  delete _RefTableStack->Pop();

  return result;
}



//----------------------------------------------------------------------------
//                   miscellaneous functions
//----------------------------------------------------------------------------


void GSM::Clear(void)
{
  assert(_RefTableStack->Depth() > 0);
  delete _RefTableStack->Pop();

  _RefTableStack->Push(new RefHashTable);

}



Portion* GSM::Help(gString funcname, bool udf, bool bif, bool getdesc)
{
  int i;
  int j;
  int fk;
  int ck;
  int cfk;
  bool match;
  int found = 0;
  gString curname;
  const gList<FuncDescObj*>* funcs = _FuncTable->Value();
  FuncDescObj *func;
  gList<FuncDescObj*> funclist;
  gSortList<FuncDescObj*> funcslist;
  Portion* result = 0;

  if(_FuncTable->IsDefined(funcname))
  {
    func = (*_FuncTable)(funcname);
    gList<gString> list = func->FuncList( udf, bif, getdesc );
    result = new ListPortion();
    for(i=1; i<=list.Length(); i++)
      ((ListPortion*) result)->Append(new TextPortion(list[i]));
  }
  else
  {
    funcname = funcname.dncase();
    for(i=0; i<_FuncTable->NumBuckets(); i++)
      for(j=1; j<=funcs[i].Length(); j++)
	funclist.Append(funcs[i][j]);

    for(i=1; i<=funclist.Length(); i++)
    {
      match = true;
      curname = funclist[i]->FuncName().dncase();
      fk = 0; 
      ck = 0;
      cfk = -1;
      while(match && (fk<funcname.length()) && (ck<curname.length()))
      {
	if(funcname[fk]=='*')
	{
	  if(fk+1==funcname.length())
	    break;
	  cfk = fk;
	  fk++;
	  while(ck<curname.length() && funcname[fk]!=curname[ck])
	    ck++;
	  if(ck==curname.length())
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

      if((fk>=funcname.length()) != (ck>=curname.length()))	
	match = false;
      if(fk+1==funcname.length() && funcname[fk]=='*')
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

    gFuncListSorter sorter(funcslist);
    if(found==1)
    {
      gList<gString> list = func->FuncList( udf, bif, getdesc );
      result = new ListPortion();
      for(i=1; i<=list.Length(); i++)
	((ListPortion*) result)->Append(new TextPortion(list[i]));
    }
    else
    {
      sorter.Sort();
      result = new ListPortion();
      for(i=1; i<=funcslist.Length(); i++)
	((ListPortion*) result)->
	  Append(new TextPortion(funcslist[i]->FuncName()));
    }
  }

  if(!result)
    result = new ErrorPortion("No match found");      
  return result;
}


Portion* GSM::HelpVars(gString varname)
{
  int i;
  int j;
  int fk;
  int ck;
  int cfk;
  bool match;
  int found = 0;
  gString curname;
  const gList<gString>* vars = _RefTableStack->Peek()->Key();
  gString var;
  gList<gString> varlist;
  gSortList<gString> varslist;
  Portion* result = 0;

  if(_RefTableStack->Peek()->IsDefined(varname))
  {
    result = new ListPortion();
    ((ListPortion*) result)->Append(new TextPortion(varname + ":" + 
      PortionSpecToText((*(_RefTableStack->Peek()))(varname)->Spec())));
  }
  else
  {
    varname = varname.dncase();
    for(i=0; i<_RefTableStack->Peek()->NumBuckets(); i++)
      for(j=1; j<=vars[i].Length(); j++)
	varlist.Append(vars[i][j]);

    for(i=1; i<=varlist.Length(); i++)
    {
      match = true;
      curname = varlist[i].dncase();
      fk = 0; 
      ck = 0;
      cfk = -1;
      while(match && (fk<varname.length()) && (ck<curname.length()))
      {
	if(varname[fk]=='*')
	{
	  if(fk+1==varname.length())
	    break;
	  cfk = fk;
	  fk++;
	  while(ck<curname.length() && varname[fk]!=curname[ck])
	    ck++;
	  if(ck==curname.length())
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

      if((fk>=varname.length()) != (ck>=curname.length()))	
	match = false;
      if(fk+1==varname.length() && varname[fk]=='*')
	match = true;
      if(match)
      {	
	var = varlist[i];
	varslist.Append(var);
	found++;
      }      
    }

    gTextListSorter sorter(varslist);
    sorter.Sort();
    result = new ListPortion();
    for(i=1; i<=varslist.Length(); i++)
      ((ListPortion*) result)->Append(new TextPortion(varslist[i] + ":" + 
	PortionSpecToText((*(_RefTableStack->Peek()))(varslist[i])->Spec())));
  }

  if(!result)
    result = new ErrorPortion("No match found");      
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
    int i = 0;
    int j = 0;
    
    for(i=0; i<_RefTableStack->Peek()->NumBuckets(); i++)
      for(j=1; j<=vars[i].Length(); j++)
	varslist.Append(vars[i][j]);

    for( i = 1; i <= varslist.Length(); i++ )
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

void GSM::UnAssignGameElement( void* game, bool IsEfg, PortionSpec spec )
{
  assert( spec.ListDepth == 0 );


  gStack< RefHashTable* > tempRefTableStack;

  while( _RefTableStack->Depth() > 0 )
  {

    const gList<Portion*>* vars = _RefTableStack->Peek()->Value();
    gList<Portion*> varslist;
    int i = 0;
    int j = 0;
    
    for(i=0; i<_RefTableStack->Peek()->NumBuckets(); i++)
      for(j=1; j<=vars[i].Length(); j++)
	varslist.Append(vars[i][j]);

    for( i = 1; i <= varslist.Length(); i++ )
    {
      if( varslist[i]->Spec().ListDepth == 0 )
      {
	if( varslist[i]->Game() == game )
	{
	  assert( varslist[i]->GameIsEfg() == IsEfg );
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

void GSM::UnAssignEfgElement( Efg* game, PortionSpec spec, void* data )
{
  assert( spec.ListDepth == 0 );

  gStack< RefHashTable* > tempRefTableStack;

  while( _RefTableStack->Depth() > 0 )
  {

    const gList<Portion*>* vars = _RefTableStack->Peek()->Value();
    gList<Portion*> varslist;
    int i = 0;
    int j = 0;
    
    assert( ( spec.Type & porEFSUPPORT) ||
	   ( spec.Type & porEFPLAYER ) ||
	   ( spec.Type & porINFOSET ) ||
	   ( spec.Type & porNODE ) ||
	   ( spec.Type & porACTION ) ||
	   ( spec.Type & porEFOUTCOME ) );
    
    for(i=0; i<_RefTableStack->Peek()->NumBuckets(); i++)
      for(j=1; j<=vars[i].Length(); j++)
	varslist.Append(vars[i][j]);

    for( i = 1; i <= varslist.Length(); i++ )
    {
      if( varslist[i]->Spec().ListDepth == 0 )
      {
	if( varslist[i]->Game() == game )
	{
	  assert( varslist[i]->GameIsEfg() );
	  if( spec.Type & porEFSUPPORT )
	  {
	    if( ((EfSupportPortion*) varslist[i])->Value() == data )
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



void GSM::UnAssignEfgInfoset( Efg* game, Infoset* infoset )
{
  assert( game );
  assert( infoset );
  int i = 0;
  for( i = 1; i <= infoset->NumActions(); i++ )
    UnAssignEfgElement( game, porACTION, infoset->Actions()[ i ] );
  UnAssignEfgElement( game, porINFOSET, infoset );  
}


void GSM::UnAssignEfgSubTree( Efg* game, Node* node )
{
  assert( game );
  assert( node );
  int i = 0;
  int j = 0;
  for( i = 1; i <= node->NumChildren(); i++ )
  {
    Infoset* infoset = node->GetInfoset();
    if( infoset != NULL )
    {
      const gArray<Action *>& actions = infoset->Actions();
      for( j = actions.First(); j <= actions.Last(); j++ )
	UnAssignEfgElement( game, porACTION, actions[j] );
      UnAssignEfgElement( game, porINFOSET, infoset );
    }
    UnAssignEfgSubTree( game, node->GetChild( i ) );
  }
  UnAssignEfgElement( game, porNODE, node );
}





//-----------------------------------------------------------------------
//                         _ErrorMessage
//-----------------------------------------------------------------------

void GSM::_ErrorMessage
(
 gOutput&        s,
 const int       error_num,
 const long&     num1, 
 const long&     num2,
 const gString&  str1,
 const gString&  str2,
 const gString&  str3
)
{
#if 0
  s << "GSM Error " << error_num << ":\n";
#endif // 0

  s << "GCL: ";

  switch(error_num)
  {
  case 11:
    s << "NthElement[]: Subscript out of range\n";
    s << "Requested #" << num1 << " out of " << num2 << " elements\n";
    break;
  case 12:
    s << "NthChar[]: Subscript out of range\n";
    break;
  case 25:
    s << "Function " << str1 << "[] undefined\n";
    break;
  case 35:
    s << "Cannot create a list of mixed types\n";
    break;
  case 42:
    s << "Cannot change the type of variable \"" << str1 << "\"\n";
    break;
  case 46:
    s << "Cannot assign to read-only variable \"" << str1 <<"\"\n";
    break;
  case 49:
    s << "Cannot insert undefined reference \"" << str1 << "\" into a list\n";
    break;
  case 53:
    s << "UnAssign[] called on a non-reference value\n";
    break;
  case 54:
    s << "UnAssign[] called on undefined reference \"" << str1 << "\"\n";
    break;
  case 55:
    s << "Cannot remove read-only variable \"" + str1 + "\"\n";
    break;
  case 59:
    s << "Cannot to pass an undefined reference to a function\n";
    break;
  case 60:
    s << "New " << str1 << "[] ambiguous with existing built-in function\n";
    break;
  case 61:
    s << "Cannot pass an undefined reference to a function\n";
    break;
  case 62:
    s << "Function " << str1 << "[] not found\n";
    break;
  case 63:
    s << "Undefined variable " << str1 << " passed to Assign[]\n";
    break;
  case 64:
    s << "Cannot assign from an INPUT or OUTPUT variable\n";
    break;
  case 65:
    s << "Attempted to change the type of a list\n";
    break;
  case 66:
    s << "Cannot change the type of variable \"" << str1 << "\" from ";
    s << str2 << " to " << str3 << '\n';
    break;
  case 67:
    s << "Assigning to an unknown type: " << str1 << "\n";
    break;
  case 71:
    s << "Cannot assign to a value not directly associated with a variable\n";
    break;
  case 72:
    s << "No matching function prototype found\n";
    break;
  case 73:
    s << "Function " << str1 << "[] not found\n";
    break;
  default:
    s << "General error " << error_num << "\n";
  }
}





