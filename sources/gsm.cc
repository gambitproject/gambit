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
#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#pragma option -Jgd
#define TEMPLATE
#endif   // __GNUG__, __BORLANDC__

#include "gstack.imp"

TEMPLATE class gStack< Portion* >;
TEMPLATE class gStack< gStack< Portion* >* >;
TEMPLATE class gStack< CallFuncObj* >;
TEMPLATE class gStack< RefHashTable* >;

#include "garray.imp"
#include "gslist.imp"
class NewInstr;
TEMPLATE class gArray<NewInstr*>;
class FuncDescObj;
TEMPLATE class gSortList<FuncDescObj*>;
TEMPLATE class gListSorter<FuncDescObj*>;
#include "gstring.h"
TEMPLATE class gSortList<gString>;
TEMPLATE class gListSorter<gString>;


#pragma option -Jgx

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

  Flush();
  delete _FuncTable;

  assert(_RefTableStack->Depth() == 1);
  delete _RefTableStack->Pop();
  delete _RefTableStack;

  assert(_StackStack->Depth() == 1);
  delete _StackStack->Pop();
  delete _StackStack;

}


int GSM::Depth(void) const
{
  return _Depth();
}


int GSM::MaxDepth(void) const
{
  return _StackStack->Peek()->MaxDepth();
}



//------------------------------------------------------------------------
//                           Push() functions
//------------------------------------------------------------------------


bool GSM::Push(Portion* p)
{
  _Push(p);
  return true;
}

bool GSM::Push(const bool& data)
{
  _Push(new BoolValPortion(data));
  return true;
}


bool GSM::Push(const long& data)
{
  _Push(new IntValPortion(data));
  return true;
}


bool GSM::Push(const double& data)
{
  _Push(new FloatValPortion(data));
  return true;
}


bool GSM::Push(const gRational& data)
{
  _Push(new RationalValPortion(data));
  return true;
}


bool GSM::Push(const gString& data)
{
  _Push(new TextValPortion(data));
  return true;
}

bool GSM::Push(gInput& data)
{
  _Push(new InputRefPortion(data));
  return true;
}

bool GSM::Push(gOutput& data)
{
  _Push(new OutputRefPortion(data));
  return true;
}


bool GSM::PushList(const int num_of_elements)
{ 
  int            i;
  Portion*       p;
  ListPortion*  list;
  int            insert_result;
  bool           result = true;

#ifndef NDEBUG
  if(num_of_elements < 0)
  {
    gerr << "  Illegal number of elements requested to PushList()\n";
  }
  assert(num_of_elements >= 0);

  if(num_of_elements > _Depth())
  {
    gerr << "  Not enough elements in GSM to PushList()\n";
  }
  assert(num_of_elements <= Depth());
#endif // NDEBUG

  list = new ListValPortion;
  for(i = 1; i <= num_of_elements; i++)
  {
    p = _Pop();
    p = _ResolveRef(p);

    if(p->Spec().Type != porREFERENCE)
    {
      insert_result = list->Insert(p->ValCopy(), 1);
      delete p;
      if(insert_result == 0)
      {
	_ErrorMessage(_StdErr, 35);
	result = false;
      }
    }
    else
    {
      _ErrorMessage(_StdErr, 49, 0, 0, ((ReferencePortion*) p)->Value());
      delete p;
      result = false;
    }
  }
  _Push(list);

  return result;
}



//--------------------------------------------------------------------
//        Stack access related functions
//--------------------------------------------------------------------



bool GSM::_VarIsDefined(const gString& var_name) const
{
  bool result;

  assert(var_name != "");

  result = _RefTableStack->Peek()->IsDefined(var_name);
  return result;
}


bool GSM::_VarDefine(const gString& var_name, Portion* p)
{
  Portion* old_value = 0;
  bool type_match = true;
  bool read_only = false;
  bool result = true;

  assert(var_name != "");

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

  if(read_only)
  {
    _ErrorMessage(_StdErr, 46, 0, 0, var_name);
    delete p;
    result = false;
  }
  else if(!type_match)
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


Portion* GSM::_VarValue(const gString& var_name) const
{
  Portion* result;

  assert(var_name != "");
  result = (*_RefTableStack->Peek())(var_name);
  return result;
}


Portion* GSM::_VarRemove(const gString& var_name)
{
  Portion* p;

  assert(var_name != "");

  p = _RefTableStack->Peek()->Remove(var_name);
  assert(p);
  return p;
}



int GSM::_Depth(void) const
{
  return _StackStack->Peek()->Depth();
}

void GSM::_Push(Portion* p)
{
  _StackStack->Peek()->Push(p);
}

Portion* GSM::_Pop(void)
{
  return _StackStack->Peek()->Pop();
}

//---------------------------------------------------------------------
//     Reference related functions
//---------------------------------------------------------------------


bool GSM::PushRef(const gString& ref)
{
  assert(ref != "");
  _Push(new ReferencePortion(ref));
  return true;
}



bool GSM::Assign(void)
{
  Portion* p2;
  Portion* p1;
  Portion* p_old;
  bool result = true;
  gString varname;

  p2 = _Pop();
  p1 = _Pop();
  if(p1->Spec().Type == porREFERENCE)
    varname = ((ReferencePortion*) p1)->Value();

  p2 = _ResolveRef(p2);
  p1 = _ResolveRef(p1);

  // deals with problems with assigning a variable to itself
  if(p1->Original() == p2->Original())
  {
    delete p2;
    _Push(p1);
    return true;
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
    if( _VarDefine(varname, p2) )
      _Push(p2->RefCopy());
    else
      _Push( new ErrorPortion );
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
      case porFLOAT:
	((FloatPortion*) p1)->Value() = ((FloatPortion*) p2)->Value();
	break;
      case porRATIONAL:
	((RationalPortion*) p1)->Value()=((RationalPortion*) p2)->Value();
	break;
      case porTEXT:
	((TextPortion*) p1)->Value() = ((TextPortion*) p2)->Value();
	break;
      case porBOOL:
	((BoolPortion*) p1)->Value() = ((BoolPortion*) p2)->Value();
	break;

      case porEFOUTCOME_FLOAT:
      case porEFOUTCOME_RATIONAL:
	((EfOutcomePortion *) p1)->SetValue(((EfOutcomePortion *) p2)->Value());
	break;

      case porNFSUPPORT_FLOAT:
      case porNFSUPPORT_RATIONAL:
	((NfSupportPortion *) p1)->SetValue(new NFSupport(*((NfSupportPortion *) p2)->Value()));
	break;

      case porEFSUPPORT_FLOAT:
      case porEFSUPPORT_RATIONAL:
	((EfSupportPortion *) p1)->SetValue(new EFSupport(*((EfSupportPortion *) p2)->Value()));
	break;

      case porINFOSET_FLOAT:
      case porINFOSET_RATIONAL:
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



      /* commented out for MIXED and BEHAV so that assignments
         for these two types will be carried out by unassigning the
         first argument first.
         make sure that the if() condition is reflected if this is changed
         -- gwu
      case porMIXED_FLOAT:
	(*((MixedSolution<double>*) ((MixedPortion*) p1)->Value())) = 
	  (*((MixedSolution<double>*) ((MixedPortion*) p2)->Value()));
	p1->SetGame(p2->Game(), p2->GameIsEfg());
	break;
      case porMIXED_RATIONAL:
	(*((MixedSolution<gRational>*) ((MixedPortion*) p1)->Value())) = 
	  (*((MixedSolution<gRational>*) ((MixedPortion*) p2)->Value()));
	p1->SetGame(p2->Game(), p2->GameIsEfg());
	break;

      case porBEHAV_FLOAT:
	(*((BehavSolution<double>*) ((BehavPortion*) p1)->Value())) = 
	  (*((BehavSolution<double>*) ((BehavPortion*) p2)->Value()));
	p1->SetGame(p2->Game(), p2->GameIsEfg());
	break;
      case porBEHAV_RATIONAL:
	(*((BehavSolution<gRational>*) ((BehavPortion*) p1)->Value())) = 
	  (*((BehavSolution<gRational>*) ((BehavPortion*) p2)->Value()));
	p1->SetGame(p2->Game(), p2->GameIsEfg());
	break;
      */



      case porNFG_FLOAT:
	((NfgPortion<double> *) p1)->SetValue(((NfgPortion<double> *) p2)->Value());
//	p1->Original()->SetGame(p2->Game(), false);
	break;
      case porNFG_RATIONAL:
	((NfgPortion<gRational> *) p1)->SetValue(((NfgPortion<gRational> *) p2)->Value());
//	p1->Original()->SetGame(p2->Game(), false);
	break;

      case porEFG_FLOAT:
      case porEFG_RATIONAL:
	((EfgPortion *) p1)->SetValue(((EfgPortion *) p2)->Value());
//	p1->Original()->SetGame(p2->Game(), true);
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
	_Push(p1); 
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
	_Push(p1);
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
      if( _VarDefine(varname, p2) )
	_Push(p2->RefCopy());
      else
	_Push( new ErrorPortion );
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

  if(!result)
  { delete p2; delete p1; }
  return result;
}





bool GSM::UnAssign(void)
{
  Portion* p;

#ifndef NDEBUG
  if(_Depth() < 1)
  {
    gerr << "  Not enough operands to execute UnAssign[]\n";
  }
  assert(_Depth() >= 1);
#endif // NDEBUG

  p = _Pop();
  if(p->Spec().Type == porREFERENCE)
  {
    if(_VarIsDefined(((ReferencePortion*) p)->Value()))
    {
      _Push(_VarRemove(((ReferencePortion*) p)->Value()));
      delete p;
      return true;
    }
    else
    {
      _Push(p);
      _ErrorMessage(_StdErr, 54, 0, 0, ((ReferencePortion*) p)->Value());
      return false;
    }
  }
  else
  {
    _Push(p);
    _ErrorMessage(_StdErr, 53);
    return false;
  }
}


Portion* GSM::UnAssignExt(void)
{
  Portion* p;
  gString txt;

#ifndef NDEBUG
  if(_Depth() < 1)
  {
    gerr << "  Not enough operands to execute UnAssign[]\n";
  }
  assert(_Depth() >= 1);
#endif // NDEBUG

  p = _Pop();
  if(p->Spec().Type == porREFERENCE)
  {
    gString varname = ((ReferencePortion*) p)->Value(); 
    if(_VarIsDefined( varname ) )
    {
      delete p;
      return _VarRemove( varname );
    }
    else
    {
      _Push(p);
      txt = "UnAssign[] called on undefined reference \"";
      txt += varname;
      txt += '\"';
      p = new ErrorPortion(txt);
      return p;
    }
  }
  else
  {
    _Push(p);
    txt = "UnAssign[] called on a non-reference value";
    p = new ErrorPortion(txt);
    return p;
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

    if(!_VarIsDefined(ref))
    {
      result = p;
    }
    else
    {
      result = _VarValue(ref)->RefCopy();
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



//------------------------------------------------------------------------
//                       binary operations
//------------------------------------------------------------------------


// Main dispatcher of built-in binary operations

bool GSM::_BinaryOperation(const gString& funcname)
{
  Portion*   p2;
  Portion*   p1;
  gList< NewInstr* > prog;
  int result;

#ifndef NDEBUG
  if(_Depth() < 2)
  {
    gerr << "  Not enough operands to perform binary operation\n";
  }
  assert(_Depth() >= 2);
#endif // NDEBUG

  // bind the parameters in correct order
  p2 = _Pop();
  p1 = _Pop();
  _Push(p2);
  _Push(p1);
  
  prog.Append(new NewInstr(iINIT_CALL_FUNCTION, funcname));
  prog.Append(new NewInstr(iBIND));
  prog.Append(new NewInstr(iBIND));
  prog.Append(new NewInstr(iCALL_FUNCTION));
  result = Execute(prog);

  if(result == rcSUCCESS)
    return true;
  else
    return false;
}




//-----------------------------------------------------------------------
//                        unary operations
//-----------------------------------------------------------------------

bool GSM::_UnaryOperation(const gString& funcname)
{
  gList< NewInstr* > prog;
  int result;

#ifndef NDEBUG
  if(_Depth() < 1)
  {
    gerr << "  Not enough operands to perform unary operation\n";
  }
  assert(_Depth() >= 1);
#endif // NDEBUG

  prog.Append(new NewInstr(iINIT_CALL_FUNCTION, funcname));
  prog.Append(new NewInstr(iBIND));
  prog.Append(new NewInstr(iCALL_FUNCTION));
  result = Execute(prog);

  if(result == rcSUCCESS)
    return true;
  else
    return false;
}




//-----------------------------------------------------------------
//                      built-in operations
//-----------------------------------------------------------------

bool GSM::Add (void)
{ 
  Portion* p1;
  Portion* p2;
  Portion* p;
  bool result = false;

  p2 = _Pop();
  p2 = _ResolveRef(p2);
  p1 = _Pop();
  p1 = _ResolveRef(p1);  
  p = p1->ValCopy();

  if(p1->Spec() == p2->Spec() && p1->Spec().ListDepth == 0)
  {
    result = true;
    if(p->Spec().Type==porINTEGER)
      ((IntPortion*) p)->Value() += ((IntPortion*) p2)->Value();
    else if(p->Spec().Type==porFLOAT)
      ((FloatPortion*) p)->Value() += ((FloatPortion*) p2)->Value();
    else if(p->Spec().Type==porRATIONAL)
      ((RationalPortion*) p)->Value() += ((RationalPortion*) p2)->Value();
/*
    else if(p->Spec().Type==porMIXED_FLOAT)
      (*((MixedSolution<double>*) ((MixedPortion*) p)->Value())) += 
	(*((MixedSolution<double>*) ((MixedPortion*) p2)->Value()));
    else if(p->Spec().Type==porMIXED_RATIONAL)
      (*((MixedSolution<gRational>*) ((MixedPortion*) p)->Value())) += 
	(*((MixedSolution<gRational>*) ((MixedPortion*) p2)->Value()));
    else if(p->Spec().Type==porBEHAV_FLOAT)
      (*((BehavSolution<double>*) ((BehavPortion*) p)->Value())) += 
	(*((BehavSolution<double>*) ((BehavPortion*) p2)->Value()));
    else if(p->Spec().Type==porBEHAV_RATIONAL)
      (*((BehavSolution<gRational>*) ((BehavPortion*) p)->Value())) += 
	(*((BehavSolution<gRational>*) ((BehavPortion*) p2)->Value()));
*/ 
   else
      result = false;
  }

  if(result)
  { delete p2; delete p1; _Push(p); }
  else
  { delete p; _Push(p1); _Push(p2); result = _BinaryOperation("Plus"); }
  return result;
}

bool GSM::Subtract (void)
{ 
  Portion* p1;
  Portion* p2;
  Portion* p;
  bool result = false;

  p2 = _Pop();
  p2 = _ResolveRef(p2);
  p1 = _Pop();
  p1 = _ResolveRef(p1);  
  p = p1->ValCopy();

  if(p1->Spec() == p2->Spec() && p1->Spec().ListDepth == 0)
  {
    result = true;
    if(p->Spec().Type==porINTEGER)
      ((IntPortion*) p)->Value() -= ((IntPortion*) p2)->Value();
    else if(p->Spec().Type==porFLOAT)
      ((FloatPortion*) p)->Value() -= ((FloatPortion*) p2)->Value();
    else if(p->Spec().Type==porRATIONAL)
      ((RationalPortion*) p)->Value() -= ((RationalPortion*) p2)->Value();
/*
    else if(p->Spec().Type==porMIXED_FLOAT)
      (*((MixedSolution<double>*) ((MixedPortion*) p)->Value())) -= 
	(*((MixedSolution<double>*) ((MixedPortion*) p2)->Value()));
    else if(p->Spec().Type==porMIXED_RATIONAL)
      (*((MixedSolution<gRational>*) ((MixedPortion*) p)->Value())) -= 
	(*((MixedSolution<gRational>*) ((MixedPortion*) p2)->Value()));
    else if(p->Spec().Type==porBEHAV_FLOAT)
      (*((BehavSolution<double>*) ((BehavPortion*) p)->Value())) -= 
	(*((BehavSolution<double>*) ((BehavPortion*) p2)->Value()));
    else if(p->Spec().Type==porBEHAV_RATIONAL)
      (*((BehavSolution<gRational>*) ((BehavPortion*) p)->Value())) -= 
	(*((BehavSolution<gRational>*) ((BehavPortion*) p2)->Value()));
*/ 
   else
      result = false;
  }

  if(result)
  { delete p2; delete p1; _Push(p); }
  else
  { delete p; _Push(p1); _Push(p2); result = _BinaryOperation("Minus"); }
  return result;
}

bool GSM::Multiply (void)
{ 
  Portion* p1;
  Portion* p2;
  Portion* p;
  bool result = false;

  p2 = _Pop();
  p2 = _ResolveRef(p2);
  p1 = _Pop();
  p1 = _ResolveRef(p1);  
  p = p1->ValCopy();
  
  if(p1->Spec() == p2->Spec() && p1->Spec().ListDepth == 0)
  {
    result = true;
    if(p->Spec().Type==porINTEGER)
      ((IntPortion*) p)->Value() *= ((IntPortion*) p2)->Value();
    else if(p->Spec().Type==porFLOAT)
      ((FloatPortion*) p)->Value() *= ((FloatPortion*) p2)->Value();
    else if(p->Spec().Type==porRATIONAL)
      ((RationalPortion*) p)->Value() *= ((RationalPortion*) p2)->Value();
    else
      result = false;
  }

  if(result)
  { delete p2; delete p1; _Push(p); }
  else
  { delete p; _Push(p1); _Push(p2); result = _BinaryOperation("Times"); }
  return result;
}


bool GSM::Dot (void)
{ return _BinaryOperation("Dot"); }

bool GSM::Divide (void)
{ 
  Portion* p1;
  Portion* p2;
  Portion* p;
  bool result = false;

  p2 = _Pop();
  p2 = _ResolveRef(p2);
  p1 = _Pop();
  p1 = _ResolveRef(p1);  
  p = p1->ValCopy();

  if(p1->Spec() == p2->Spec() && p1->Spec().ListDepth == 0)
  {
    result = true;
    if(p->Spec().Type==porINTEGER && ((IntPortion*) p2)->Value() != 0)
    {
      delete p;
      p = new RationalValPortion(((IntPortion*) p1)->Value());
      ((RationalPortion*) p)->Value() /= ((IntPortion*) p2)->Value();
    }
    else if(p->Spec().Type==porFLOAT && ((FloatPortion*) p2)->Value() != 0)
      ((FloatPortion*) p)->Value() /= ((FloatPortion*) p2)->Value();
    else if(p->Spec().Type==porRATIONAL && ((RationalPortion*) p2)->Value() != (gRational)0)
      ((RationalPortion*) p)->Value() /= ((RationalPortion*) p2)->Value();
    else
      result = false;
  }

  if(result)
  { delete p2; delete p1; _Push(p); }
  else
  { delete p; _Push(p1); _Push(p2); result = _BinaryOperation("Divide"); }
  return result;
}

bool GSM::Negate(void)
{
  Portion* p1;
  Portion* p;
  bool result = false;

  p1 = _Pop();
  p1 = _ResolveRef(p1);  
  p = p1->ValCopy();

  
  if(p->Spec().ListDepth == 0)
  {
    result = true;
    if(p->Spec().Type==porINTEGER)
      ((IntPortion*) p)->Value() = -(((IntPortion*) p1)->Value());
    else if(p->Spec().Type==porFLOAT)
      ((FloatPortion*) p)->Value() = -(((FloatPortion*) p1)->Value());
    else if(p->Spec().Type==porRATIONAL)
      ((RationalPortion*) p)->Value() = -(((RationalPortion*) p1)->Value());
    else
      result = false;
  }

  if(result)
  { delete p1; _Push(p); }
  else
  { delete p; _Push(p1); result = _UnaryOperation("Negate"); }
  return result;
}

bool GSM::Power(void)
{ return _BinaryOperation("Power"); }

bool GSM::Concat (void)
{ return _BinaryOperation("Concat"); }


bool GSM::IntegerDivide (void)
{ 
  Portion* p1;
  Portion* p2;
  Portion* p;
  bool result = false;

  p2 = _Pop();
  p2 = _ResolveRef(p2);
  p1 = _Pop();
  p1 = _ResolveRef(p1);  
  p = p1->ValCopy();

  if(p1->Spec() == p2->Spec() && p1->Spec().ListDepth == 0)
  {
    result = true;
    if(p->Spec().Type==porINTEGER && ((IntPortion*) p2)->Value() != 0)
      ((IntPortion*) p)->Value() /= ((IntPortion*) p2)->Value();
    else
      result = false;
  }

  if(result)
  { delete p2; delete p1; _Push(p); }
  else
  { delete p; _Push(p1); _Push(p2); 
    result = _BinaryOperation("IntegerDivide"); }
  return result;
}

bool GSM::Modulus (void)
{
  Portion* p1;
  Portion* p2;
  Portion* p;
  bool result = false;

  p2 = _Pop();
  p2 = _ResolveRef(p2);
  p1 = _Pop();
  p1 = _ResolveRef(p1);  
  p = p1->ValCopy();

  if(p1->Spec() == p2->Spec() && p1->Spec().ListDepth == 0)
  {
    result = true;
    if(p->Spec().Type==porINTEGER && ((IntPortion*) p2)->Value() != 0)
      ((IntPortion*) p)->Value() %= ((IntPortion*) p2)->Value();
    else
      result = false;
  }

  if(result)
  { delete p2; delete p1; _Push(p); }
  else
  { delete p; _Push(p1); _Push(p2); result = _BinaryOperation("Modulus"); }
  return result;
}


bool GSM::EqualTo (void)
{ 
  Portion* p1;
  Portion* p2;
  bool b = false;
  bool result = false;

  p2 = _Pop();
  p2 = _ResolveRef(p2);
  p1 = _Pop();
  p1 = _ResolveRef(p1);  

  if(p1->Spec() == p2->Spec() && p1->Spec().ListDepth == 0)
  {
    result = true;
    b = PortionEqual(p1, p2, result);
  }

  if(result)
  { delete p2; delete p1; _Push(new BoolValPortion(b)); }
  else
  { _Push(p1); _Push(p2); result = _BinaryOperation("Equal"); }
  return result;
}

bool GSM::NotEqualTo (void)
{ 
  Portion* p1;
  Portion* p2;
  bool b = false;
  bool result = false;

  p2 = _Pop();
  p2 = _ResolveRef(p2);
  p1 = _Pop();
  p1 = _ResolveRef(p1);  

  if(p1->Spec() == p2->Spec() && p1->Spec().ListDepth == 0)
  {
    result = true;
    b = !PortionEqual(p1, p2, result);
  }

  if(result)
  { delete p2; delete p1; _Push(new BoolValPortion(b)); }
  else
  { _Push(p1); _Push(p2); result = _BinaryOperation("NotEqual"); }
  return result;
}

bool GSM::GreaterThan (void)
{ 
  Portion* p1;
  Portion* p2;
  bool b = false;
  bool result = false;

  p2 = _Pop();
  p2 = _ResolveRef(p2);
  p1 = _Pop();
  p1 = _ResolveRef(p1);  

  if(p1->Spec() == p2->Spec() && p1->Spec().ListDepth == 0)
  {
    result = true;
    if(p1->Spec().Type==porINTEGER)   
      b = (((IntPortion*) p1)->Value() > ((IntPortion*) p2)->Value());
    else if(p1->Spec().Type==porFLOAT)
      b = (((FloatPortion*) p1)->Value() > ((FloatPortion*) p2)->Value());
    else if(p1->Spec().Type==porRATIONAL)
      b = (((RationalPortion*) p1)->Value()>((RationalPortion*) p2)->Value());
    else if(p1->Spec().Type==porTEXT)
      b = (((TextPortion*) p1)->Value() > ((TextPortion*) p2)->Value());
    else
      result = false;
  }

  if(result)
  { delete p2; delete p1; _Push(new BoolValPortion(b)); }
  else
  { _Push(p1); _Push(p2); result = _BinaryOperation("Greater"); }
  return result;
}

bool GSM::LessThan (void)
{ 
  Portion* p1;
  Portion* p2;
  bool b = false;
  bool result = false;

  p2 = _Pop();
  p2 = _ResolveRef(p2);
  p1 = _Pop();
  p1 = _ResolveRef(p1);  

  if(p1->Spec() == p2->Spec() && p1->Spec().ListDepth == 0)
  {
    result = true;
    if(p1->Spec().Type==porINTEGER)   
      b = (((IntPortion*) p1)->Value() < ((IntPortion*) p2)->Value());
    else if(p1->Spec().Type==porFLOAT)
      b = (((FloatPortion*) p1)->Value() < ((FloatPortion*) p2)->Value());
    else if(p1->Spec().Type==porRATIONAL)
      b = (((RationalPortion*) p1)->Value()<((RationalPortion*) p2)->Value());
    else if(p1->Spec().Type==porTEXT)
      b = (((TextPortion*) p1)->Value() < ((TextPortion*) p2)->Value());
    else
      result = false;
  }

  if(result)
  { delete p2; delete p1; _Push(new BoolValPortion(b)); }
  else
  { _Push(p1); _Push(p2); result = _BinaryOperation("Less"); }
  return result;
}

bool GSM::GreaterThanOrEqualTo (void)
{ 
  Portion* p1;
  Portion* p2;
  bool b = false;
  bool result = false;

  p2 = _Pop();
  p2 = _ResolveRef(p2);
  p1 = _Pop();
  p1 = _ResolveRef(p1);  

  if(p1->Spec() == p2->Spec() && p1->Spec().ListDepth == 0)
  {
    result = true;
    if(p1->Spec().Type==porINTEGER)   
      b = (((IntPortion*) p1)->Value() >= ((IntPortion*) p2)->Value());
    else if(p1->Spec().Type==porFLOAT)
      b = (((FloatPortion*) p1)->Value() >= ((FloatPortion*) p2)->Value());
    else if(p1->Spec().Type==porRATIONAL)
      b = (((RationalPortion*) p1)->Value()>=((RationalPortion*) p2)->Value());
    else if(p1->Spec().Type==porTEXT)
      b = (((TextPortion*) p1)->Value() >= ((TextPortion*) p2)->Value());
    else
      result = false;
  }

  if(result)
  { delete p2; delete p1; _Push(new BoolValPortion(b)); }
  else
  { _Push(p1); _Push(p2); result = _BinaryOperation("GreaterEqual"); }
  return result;
}

bool GSM::LessThanOrEqualTo (void)
{ 
  Portion* p1;
  Portion* p2;
  bool b = false;
  bool result = false;

  p2 = _Pop();
  p2 = _ResolveRef(p2);
  p1 = _Pop();
  p1 = _ResolveRef(p1);  

  if(p1->Spec() == p2->Spec() && p1->Spec().ListDepth == 0)
  {
    result = true;
    if(p1->Spec().Type==porINTEGER)   
      b = (((IntPortion*) p1)->Value() <= ((IntPortion*) p2)->Value());
    else if(p1->Spec().Type==porFLOAT)
      b = (((FloatPortion*) p1)->Value() <= ((FloatPortion*) p2)->Value());
    else if(p1->Spec().Type==porRATIONAL)
      b = (((RationalPortion*) p1)->Value()<=((RationalPortion*) p2)->Value());
    else if(p1->Spec().Type==porTEXT)
      b = (((TextPortion*) p1)->Value() <= ((TextPortion*) p2)->Value());
    else
      result = false;
  }

  if(result)
  { delete p2; delete p1; _Push(new BoolValPortion(b)); }
  else
  { _Push(p1); _Push(p2); result = _BinaryOperation("LessEqual"); }
  return result;
}


bool GSM::AND (void)
{ 
  Portion* p1;
  Portion* p2;
  bool b = false;
  bool result = false;

  p2 = _Pop();
  p2 = _ResolveRef(p2);
  p1 = _Pop();
  p1 = _ResolveRef(p1);  

  if(p1->Spec() == p2->Spec() && p1->Spec().ListDepth == 0)
  {
    result = true;
    if(p1->Spec().Type==porBOOL)   
      b = (((BoolPortion*) p1)->Value() && ((BoolPortion*) p2)->Value());
    else
      result = false;
  }

  if(result)
  { delete p2; delete p1; _Push(new BoolValPortion(b)); }
  else
  { _Push(p1); _Push(p2); result = _BinaryOperation("And"); }
  return result;
}

bool GSM::OR (void)
{ 
  Portion* p1;
  Portion* p2;
  bool b = false;
  bool result = false;

  p2 = _Pop();
  p2 = _ResolveRef(p2);
  p1 = _Pop();
  p1 = _ResolveRef(p1);  

  if(p1->Spec() == p2->Spec() && p1->Spec().ListDepth == 0)
  {
    result = true;
    if(p1->Spec().Type==porBOOL)   
      b = (((BoolPortion*) p1)->Value() || ((BoolPortion*) p2)->Value());
    else
      result = false;
  }

  if(result)
  { delete p2; delete p1; _Push(new BoolValPortion(b)); }
  else
  { _Push(p1); _Push(p2); result = _BinaryOperation("Or"); }
  return result;
}

bool GSM::NOT (void)
{
  Portion* p1;
  bool b = false;
  bool result = false;

  p1 = _Pop();
  p1 = _ResolveRef(p1);  

  if(p1->Spec().ListDepth == 0)
  {
    result = true;
    if(p1->Spec().Type==porBOOL)
      b = !((BoolPortion*) p1)->Value();
    else
      result = false;
  }

  if(result)
  { delete p1; _Push(new BoolValPortion(b)); }
  else
  { _Push(p1); result = _UnaryOperation("Not"); }
  return result;
}


bool GSM::Read (void)
{ return _BinaryOperation("Read"); }

bool GSM::Write (void)
{ return _BinaryOperation("Write"); }


bool GSM::Subscript (void)
{
  Portion* p2;
  Portion* p1;

  assert(_Depth() >= 2);
  p2 = _Pop();
  p1 = _Pop();

  p2 = _ResolveRef(p2);
  p1 = _ResolveRef(p1);

  
  if(p1->Spec().ListDepth > 0 && p2->Spec() == porINTEGER)
  {
    int n = ((IntPortion *) p2)->Value();
    bool result = true;
    if (n <= 0 || n > ((ListPortion*) p1)->Length())
    {
      _ErrorMessage(_StdErr, 11, n, ((ListPortion*) p1)->Length());
      result = false;
    }
    else
    {
      _Push(((ListPortion*) p1)->SubscriptCopy(n));
      result = true;
    }
    delete p1;
    delete p2;
    return result;
  }
  if(p1->Spec().Type == porTEXT && p2->Spec() == porINTEGER)
  {
    gString text(((TextPortion *) p1)->Value());
    int n = ((IntPortion *) p2)->Value();
    delete p1;
    delete p2;
    if (n <= 0 || n > text.length())
    {
      _ErrorMessage(_StdErr, 12);
      return false;
    }
    else
    {
      _Push(new TextValPortion(text[n-1]));
      return true;
    }
  }
  else
  {
    _Push(p1);
    _Push(p2);
    
    if(p1->Spec().ListDepth > 0)
      return _BinaryOperation("NthElement");
    else
      return _BinaryOperation("NthChar");
  }
}


bool GSM::Child (void)
{
  Portion* p2;
  Portion* p1;

  assert(_Depth() >= 2);
  p2 = _Pop();
  p1 = _Pop();

  p2 = _ResolveRef(p2);
  p1 = _ResolveRef(p1);

  _Push(p1);
  _Push(p2);
  
  return _BinaryOperation("NthChild");
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



#ifndef NDEBUG
void GSM::_BindCheck(void) const
{
  if(_CallFuncStack->Depth() <= 0)
  {
    gerr << "  The CallFunction() subsystem was not initialized by\n";
    gerr << "  calling InitCallFunction() first\n";
  }
  assert(_CallFuncStack->Depth() > 0);

  if(_Depth() <= 0)
  {
    gerr << "  No value found to assign to a function parameter\n";
  }
  assert(_Depth() > 0);
}
#endif // NDEBUG


bool GSM::_Bind(const gString& param_name) const
{
  return _CallFuncStack->Peek()->SetCurrParamIndex(param_name);
}


bool GSM::InitCallFunction(const gString& funcname)
{
  if(_FuncTable->IsDefined(funcname))
  {
    _CallFuncStack->Push
      (new CallFuncObj((*_FuncTable)(funcname), _StdOut, _StdErr));
    return true;
  }
  else // (!_FuncTable->IsDefined(funcname))
  {
    _ErrorMessage(_StdErr, 25, 0, 0, funcname);
    return false;
  }
}


bool GSM::Bind(const gString& param_name)
{
  return BindRef(param_name, AUTO_VAL_OR_REF);
}


bool GSM::BindVal(const gString& param_name)
{
  Portion*     param;
  bool         result = true;

#ifndef NDEBUG
  _BindCheck();
#endif // NDEBUG

  if(param_name != "")
    result = _Bind(param_name);

  if(result)
  {
    param = _ResolveRef(_Pop());

    if(param->Spec().Type != porREFERENCE)
      result = _CallFuncStack->Peek()->SetCurrParam(param->ValCopy());
    else
      result = _CallFuncStack->Peek()->SetCurrParam(0);

    /* temporary
    if(param->IsValid())
    {
      if(param->Spec().Type != porREFERENCE)
	result = _CallFuncStack->Peek()->SetCurrParam(param->ValCopy());
      else
	result = _CallFuncStack->Peek()->SetCurrParam(0);
    }
    else
    {
      _CallFuncStack->Peek()->SetErrorOccurred();
      _ErrorMessage(_StdErr, 61);
      result = false;
    }
    */

    delete param;
  }

  return result;
}


bool GSM::BindRef(const gString& param_name, bool auto_val_or_ref)
{
  Portion*           param;
  bool               result    = true;

#ifndef NDEBUG
  _BindCheck();
#endif // NDEBUG

  if(param_name != "")
    result = _Bind(param_name);

  if(result)
  {
    param = _ResolveRef(_Pop());

    result = _CallFuncStack->Peek()->SetCurrParam(param, auto_val_or_ref);

    /*
    if(param->IsValid())
    {
      result = _CallFuncStack->Peek()->SetCurrParam(param, auto_val_or_ref);
    }
    else
    {
      _CallFuncStack->Peek()->SetErrorOccurred();
      _ErrorMessage(_StdErr, 59);
      delete param;
      result = false;
    }
    */
  }

  return result;
}



bool GSM::CallFunction(void)
{
  CallFuncObj*        func;
  Portion**           param;
  int                 index;
  ReferencePortion*   refp;
  Portion*            return_value;
  bool                define_result;
  bool                result = true;

#ifndef NDEBUG
  if(_CallFuncStack->Depth() <= 0)
  {
    gerr << "  The CallFunction() subsystem was not initialized by\n";
    gerr << "  calling InitCallFunction() first\n";
  }
  assert(_CallFuncStack->Depth() > 0);
#endif // NDEBUG

  func = _CallFuncStack->Pop();

  param = new Portion*[func->NumParams()];

  return_value = func->CallFunction( this, param );

  assert(return_value != 0);

  if(return_value->Spec().Type == porERROR)
    result = false;


  _Push(return_value);
  

  for(index = 0; index < func->NumParams(); index++)
  {
    refp = func->GetParamRef(index);

    assert((refp == 0) == (param[index] == 0));

    if(refp != 0)
    {
      define_result = _VarDefine(refp->Value(), param[index]);
      if(!define_result)
	result = false;
      delete refp;
    }
  }

  if(result==false)
  {
    _StdErr << "Function call stack: " << _CallFuncStack->Depth() << ": ";
    func->Dump(_StdErr);
  }

  delete func;

  delete[] param;
  
  return result;
}



//----------------------------------------------------------------------------
//                       Execute function
//----------------------------------------------------------------------------

int GSM::Execute(gList< NewInstr* >& prog, bool user_func)
{
  int             result          = rcSUCCESS;
  bool            instr_success   = false;
  bool            done            = false;
  Portion*        p               = 0;
  NewInstr*       instr           = 0;
  int             program_counter = 1;
  int             program_length  = prog.Length();
  int             initial_num_of_funcs = _CallFuncStack->Depth();
  int             i               = 0;
  CallFuncObj*    funcobj         = 0;

  gArray<NewInstr*> program(prog.Length());
  for(i=1; i<=prog.Length(); i++)
    program[i] = prog[i];


  while((program_counter <= program_length) && (!done))
  {
    instr = program[program_counter];
    switch(instr->Code)
    {
    case iQUIT:
      instr_success = true;
      result = rcQUIT;
      done = true;
      break;

    case iIF_GOTO:
      p = _Pop();
      if(p->Spec().Type == porBOOL)
      {
	if(((BoolPortion*) p)->Value())
	{
	  program_counter = instr->IntVal;
	  assert(program_counter >= 1 && program_counter <= program_length);
	}
	else
	{
	  program_counter++;
	}
	delete p;
	instr_success = true;
      }
#ifndef NDEBUG
      else
      {
	gerr << "NewInstr IfGoto called on a non-boolean data type\n";
	assert(p->Spec().Type == porBOOL);	
	_Push(p);
	program_counter++;
	instr_success = false;
      }
#endif // NDEBUG
      break;

    case iGOTO:
      program_counter = instr->IntVal;
      assert(program_counter >= 1 && program_counter <= program_length);
      instr_success = true;
      break;

    default:
      switch(instr->Code)
      {
      case iNOP:
	instr_success = true;
	break;
	
      case iPUSHREF:
	instr_success = PushRef(instr->TextVal);
	break;

      case iPUSH_BOOL:
	_Push(new BoolValPortion(instr->BoolVal));
	instr_success = true;
	break;
      case iPUSH_INTEGER:
	_Push(new IntValPortion(instr->IntVal));
	instr_success = true;
	break;
      case iPUSH_FLOAT:
	_Push(new FloatValPortion(instr->FloatVal));
	instr_success = true;
	break;

      case iPUSH_TEXT:
	_Push(new TextValPortion(instr->TextVal));
	instr_success = true;
	break;

      case iPUSHINPUT:
	_Push(new InputRefPortion(*(instr->InputVal)));
	instr_success = true;
	break;
      case iPUSHOUTPUT:
	_Push(new OutputRefPortion(*(instr->OutputVal)));
	instr_success = true;
	break;
      case iPUSHLIST:
	instr_success = PushList(instr->IntVal);
	break;

      case iASSIGN:
	instr_success = Assign();
	break;
      case iUNASSIGN:
	instr_success = UnAssign();
	break;
      case iSUBSCRIPT:
	instr_success = Subscript();
	break;
      case iCHILD:
	instr_success = Child();
	break;
      case iREAD:
	instr_success = Read();
	break;
      case iWRITE:
	instr_success = Write();
	break;

      case iADD:
	instr_success = Add();
	break;
      case iSUB:
	instr_success = Subtract();
	break;
      case iCONCAT:
	instr_success = Concat();
	break;
      case iMUL:
	instr_success = Multiply();
	break;
      case iDOT:
	instr_success = Dot();
	break;
      case iDIV:
	instr_success = Divide();
	break;
      case iINTDIV:
	instr_success = IntegerDivide();
	break;
      case iNEG:
	instr_success = Negate();
	break;
      case iMOD:
	instr_success = Modulus();
	break;
      case iPOWER:
	instr_success = Power();
	break;

      case iEQU:
	instr_success = EqualTo();
	break;
      case iNEQ:
	instr_success = NotEqualTo();
	break;
      case iGTN:
	instr_success = GreaterThan();
	break;
      case iLTN:
	instr_success = LessThan();
	break;
      case iGEQ:
	instr_success = GreaterThanOrEqualTo();
	break;
      case iLEQ:
	instr_success = LessThanOrEqualTo();
	break;

      case iAND:
	instr_success = AND();
	break;
      case iOR:
	instr_success = OR();
	break;
      case iNOT:
	instr_success = NOT();
	break;

      case iINIT_CALL_FUNCTION:
	instr_success = InitCallFunction(instr->TextVal);
	break;
      case iBIND:
	instr_success = Bind(instr->TextVal);
	break;
      case iBINDREF:
	instr_success = BindRef(instr->TextVal);
	break;
      case iBINDVAL:
	instr_success = BindVal(instr->TextVal);
	break;
      case iCALL_FUNCTION:
	instr_success = CallFunction();
	break;

      case iPOP:
/*
	assert(_Depth() >= 0);	
	if(_Depth() > 0)
	  delete _Pop();
	instr_success = true;
	*/
	instr_success = Pop();
	break;
      case iOUTPUT:
	Output();
	instr_success = true;
	break;
      case iDUMP:
	Dump();
	instr_success = true;
	break;
      case iFLUSH:
	Flush();
	instr_success = true;
	break;
	
      default:
	assert(0);
      }
      program_counter++;
    }

    if(!instr_success)
    {
      result = instr->LineNumber;
      done = true;
      break;
    }
  }


  for(i = _CallFuncStack->Depth(); i > initial_num_of_funcs; i--)
  {
    funcobj = _CallFuncStack->Pop();
    delete funcobj;
  }
  assert(_CallFuncStack->Depth() == initial_num_of_funcs);



  if(!user_func)
  {
    while(prog.Length() > 0)
    {
      delete prog.Remove(1);
    }
  }

  return result;
}





Portion* GSM::ExecuteUserFunc(gList< NewInstr* >& program, 
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
      if( _VarDefine(func_info.ParamInfo[i].Name, param[i]) )
	param[i] = param[i]->RefCopy();
      else
	param[i] = new ErrorPortion;
    }
  }


  rc_result = Execute(program, true);


  switch(rc_result)
  {
  case rcSUCCESS:
    switch(_Depth())
    {
    case 0:
      result = 
	new ErrorPortion((gString)
			 "Error: No return value");
      break;

    default:
      result = _Pop();
      result = _ResolveRef(result);
      result_copy = result->ValCopy();
      delete result;
      result = result_copy;
      result_copy = 0;
      if(result->Spec().Type == porERROR)
      {
	delete result;
	result = 0;
      }
      break;
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
    Dump();
    break;
  }


  for(i = 0; i < func_info.NumParams; i++)
  {
    if(func_info.ParamInfo[i].PassByReference)
    {
      if(_VarIsDefined(func_info.ParamInfo[i].Name))
      {
	assert(_VarValue(func_info.ParamInfo[i].Name) != 0);
	delete param[i];
	param[i] = _VarRemove(func_info.ParamInfo[i].Name);
      }
    }
  }


  Flush();
  delete _StackStack->Pop();
  delete _RefTableStack->Pop();

  return result;
}




//----------------------------------------------------------------------------
//                   miscellaneous functions
//----------------------------------------------------------------------------


void GSM::Output(void)
{
  Portion*  p;

  assert(_Depth() >= 0);

  if(_Depth() == 0)
  {
    // _StdOut << "\n";
  }
  else
  {
    p = _Pop();

// push it right back, as Output is now a non-destructive operation
    Portion *q = p->RefCopy();
    _Push(p);

    q = _ResolveRef(q);

    q->Output(_StdOut);
    if(q->Spec().Type == porREFERENCE)
      _StdOut << " (undefined)";
    _StdOut << "\n";
    
    /*
    if(p->IsValid())
    {
      p->Output(_StdOut);
      if(p->Spec().Type == porREFERENCE)
	_StdOut << " (undefined)";
      _StdOut << "\n";
    }
    else
    {
      _StdOut << "(undefined)\n";
    }
    */
    
//    _Push(p);
  }
}


void GSM::Dump(void)
{
  int  i;

  assert(_Depth() >= 0);

  if(_Depth() == 0)
  {
    _StdOut << "Stack : NULL\n";
  }
  else
  {
    for(i = _Depth() - 1; i >= 0; i--)
    {
      _StdOut << "Stack element " << i << " : ";
      Output();
      Pop();
    }
  }

  assert(_Depth() == 0);
}


bool GSM::Pop(void)
{
  Portion* p;
  bool result = false;

  assert(_Depth() >= 0);

  if(_Depth() > 0)
  {
//    if( _Verbose )  {
//     Output();
//      delete _Pop();
//    }
//    else
//    {
      p = _Pop();
      delete p;
//    }
    result = true;
  }
  else
  {
    result = true;
  }
  return result;
}


Portion* GSM::PopValue( void )
{
  assert(_Depth() >= 0);
  Portion* p = 0;
  Portion* p_old = 0;

  if(_Depth() > 0)
  {
    p = _ResolveRef( _Pop() );
    assert( p );
    if( p->IsReference() )
    {
      p_old = p;
      p = p_old->ValCopy();
      delete p_old;
    }
  }
  return p;
}


void GSM::Flush(void)
{
  int       i;

  assert(_Depth() >= 0);
  for(i = _Depth() - 1; i >= 0; i--)
  {
    delete _Pop();
/*
    result = Pop();
    assert(result == true);
    */
  }

  assert(_Depth() == 0);
}


void GSM::Clear(void)
{
  Flush();

  assert(_RefTableStack->Depth() > 0);
  delete _RefTableStack->Pop();
  //delete _RefTableStack;

  //_RefTableStack = new gStack< RefHashTable* >(1);
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
    result = new ListValPortion();
    for(i=1; i<=list.Length(); i++)
      ((ListPortion*) result)->Append(new TextValPortion(list[i]));
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
      result = new ListValPortion();
      for(i=1; i<=list.Length(); i++)
	((ListPortion*) result)->Append(new TextValPortion(list[i]));
    }
    else
    {
      sorter.Sort();
      result = new ListValPortion();
      for(i=1; i<=funcslist.Length(); i++)
	((ListPortion*) result)->
	  Append(new TextValPortion(funcslist[i]->FuncName()));
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
    result = new ListValPortion();
    ((ListPortion*) result)->Append(new TextValPortion(varname + ":" + 
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
    result = new ListValPortion();
    for(i=1; i<=varslist.Length(); i++)
      ((ListPortion*) result)->Append(new TextValPortion(varslist[i] + ":" + 
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
	if( !IsEfg && varslist[i]->Spec() == porMIXED )
	{
	  switch(varslist[i]->Spec().Type)
	  {
	  case porMIXED_FLOAT:
	    ((MixedSolution<double>*) ((MixedPortion<double> *) 
				       varslist[i])->Value())->Invalidate();
	    break;
	  case porMIXED_RATIONAL:
	    ((MixedSolution<gRational>*) ((MixedPortion<gRational> *) 
					  varslist[i])->Value())->Invalidate();
	    break;
	  default:
	    assert( 0 );
	  }
	}
	else if( IsEfg && varslist[i]->Spec() == porBEHAV )
	{
	  switch( ((BaseEfg*) game)->Type() )
	  {
	  case DOUBLE:
	    ((BehavSolution<double>*) ((BehavPortion<double> *) 
				       varslist[i])->Value())->Invalidate();
	    break;
	  case RATIONAL:
	    ((BehavSolution<gRational>*) ((BehavPortion<gRational> *) 
					  varslist[i])->Value())->Invalidate();
	    break;
	  default:
	    assert( 0 );
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

void GSM::UnAssignEfgElement( BaseEfg* game, PortionSpec spec, void* data )
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



void GSM::UnAssignEfgInfoset( BaseEfg* game, Infoset* infoset )
{
  assert( game );
  assert( infoset );
  int i = 0;
  for( i = 1; i <= infoset->NumActions(); i++ )
    UnAssignEfgElement( game, porACTION, infoset->Actions()[ i ] );
  UnAssignEfgElement( game, porINFOSET, infoset );  
}


void GSM::UnAssignEfgSubTree( BaseEfg* game, Node* node )
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





