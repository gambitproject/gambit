//#
//# FILE: gsm.cc  implementation of GSM (Stack machine)
//#
//# $Id$
//#

//-----------------------------------------------------------------------
//                       Template instantiations
//-----------------------------------------------------------------------


class Portion;
class CallFuncObj;
class RefHashTable;
template <class T> class gStack;
#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

#include "gstack.imp"

TEMPLATE class gStack< Portion* >;
TEMPLATE class gStack< gStack< Portion* >* >;
TEMPLATE class gStack< CallFuncObj* >;
TEMPLATE class gStack< RefHashTable* >;

#ifdef __BORLANDC__
#pragma option -Jgx
#endif

#include "gsm.h"

#include <assert.h>

#include "glist.h"
#include "gstack.h"

#include "portion.h"
#include "gsmfunc.h"
#include "gsminstr.h"
#include "gsmhash.h"

#include "gblock.h"





#include "gslist.h"

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

GSM::GSM( int size, gInput& s_in, gOutput& s_out, gOutput& s_err )
:_StdIn( s_in ), _StdOut( s_out ), _StdErr( s_err )
{
#ifndef NDEBUG
  if( size <= 0 )
  {
    gerr << "  Illegal stack size specified during initialization\n";
  }
  assert( size > 0 );
#endif // NDEBUG
  
  // global function default variables initialization
  // these should be done before InitFunctions() is called

  _StackStack    = new gStack< gStack< Portion* >* >( 1 );
  _StackStack->Push( new gStack< Portion* >( size ) );
  _CallFuncStack = new gStack< CallFuncObj* >( 1 );
  _RefTableStack = new gStack< RefHashTable* >( 1 );
  _RefTableStack->Push( new RefHashTable );

  _FuncTable     = new FunctionHashTable;
  InitFunctions();  // This function is located in gsmfunc.cc

  _NumObj++;
}


GSM::~GSM()
{
  _NumObj--;

  assert( _CallFuncStack->Depth() == 0 );
  delete _CallFuncStack;

  Flush();
  delete _FuncTable;

  assert( _RefTableStack->Depth() == 1 );
  delete _RefTableStack->Pop();
  delete _RefTableStack;

  assert( _StackStack->Depth() == 1 );
  delete _StackStack->Pop();
  delete _StackStack;

}


int GSM::Depth( void ) const
{
  return _Depth();
}


int GSM::MaxDepth( void ) const
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

bool GSM::Push( const bool& data )
{
  _Push( new BoolValPortion( data ) );
  return true;
}


bool GSM::Push( const long& data )
{
  _Push( new IntValPortion( data ) );
  return true;
}


bool GSM::Push( const double& data )
{
  _Push( new FloatValPortion( data ) );
  return true;
}


bool GSM::Push( const gRational& data )
{
  _Push( new RationalValPortion( data ) );
  return true;
}


bool GSM::Push( const gString& data )
{
  _Push( new TextValPortion( data ) );
  return true;
}

bool GSM::Push( gInput& data )
{
  _Push( new InputRefPortion( data ) );
  return true;
}

bool GSM::Push( gOutput& data )
{
  _Push( new OutputRefPortion( data ) );
  return true;
}


bool GSM::PushList( const int num_of_elements )
{ 
  int            i;
  Portion*       p;
  ListPortion*  list;
  int            insert_result;
  bool           result = true;

#ifndef NDEBUG
  if( num_of_elements < 0 )
  {
    gerr << "  Illegal number of elements requested to PushList()\n";
  }
  assert( num_of_elements >= 0 );

  if( num_of_elements > _Depth() )
  {
    gerr << "  Not enough elements in GSM to PushList()\n";
  }
  assert( num_of_elements <= Depth() );
#endif // NDEBUG

  list = new ListValPortion;
  for( i = 1; i <= num_of_elements; i++ )
  {
    p = _Pop();
    p = _ResolveRef( p );

    if( p->Type() != porREFERENCE )
    {
      insert_result = list->Insert( p->ValCopy(), 1 );
      delete p;
      if( insert_result == 0 )
      {
	_ErrorMessage( _StdErr, 35 );
	result = false;
      }
    }
    else
    {
      delete p;
      _ErrorMessage( _StdErr, 49, 0, 0, ((ReferencePortion*) p)->Value() );
      result = false;
    }
  }
  _Push( list );

  return result;
}



//--------------------------------------------------------------------
//        Stack access related functions
//--------------------------------------------------------------------



bool GSM::_VarIsDefined( const gString& var_name ) const
{
  bool result;

  assert( var_name != "" );

  result = _RefTableStack->Peek()->IsDefined( var_name );
  return result;
}


bool GSM::_VarDefine( const gString& var_name, Portion* p )
{
  Portion* old_value;
  bool type_match = true;
  bool read_only = false;
  bool result = true;

  assert( var_name != "" );

  if( _RefTableStack->Peek()->IsDefined( var_name ) )
  {
    old_value = (*_RefTableStack->Peek())( var_name );
    if( old_value->Type() != p->Type() )
    {
      if( !PortionTypeMatch( old_value->Type(), p->Type() ) )
	type_match = false;
    }
    else if( p->Type() == porLIST )
    {
      assert( old_value->Type() == porLIST );
      if( ( (ListPortion*) old_value )->DataType() != 
	 ( (ListPortion*) p )->DataType() )
      {
	if( ( (ListPortion*) p )->DataType() == porUNDEFINED )
	{
	  ( (ListPortion*) p )->
	    SetDataType( ( (ListPortion*) old_value )->DataType() );
	}
	else
	{
	  type_match = false;
	}
      }
    }
  }

  if( read_only )
  {
    _ErrorMessage( _StdErr, 46, 0, 0, var_name );
    delete p;
    result = false;
  }
  else if( !type_match )
  {
    _ErrorMessage( _StdErr, 42, 0, 0, var_name );
    delete p;
    result = false;
  }
  else
  {
    _RefTableStack->Peek()->Define( var_name, p );
  }
  return result;
}


Portion* GSM::_VarValue( const gString& var_name ) const
{
  Portion* result;

  assert( var_name != "" );
  result = (*_RefTableStack->Peek())( var_name );
  return result;
}


Portion* GSM::_VarRemove( const gString& var_name )
{
  Portion* result;

  assert( var_name != "" );

  result = _RefTableStack->Peek()->Remove( var_name );
  return result;
}



int GSM::_Depth( void ) const
{
  return _StackStack->Peek()->Depth();
}

void GSM::_Push( Portion* p )
{
  _StackStack->Peek()->Push( p );
}

Portion* GSM::_Pop( void )
{
  return _StackStack->Peek()->Pop();
}

//---------------------------------------------------------------------
//     Reference related functions
//---------------------------------------------------------------------


bool GSM::PushRef( const gString& ref )
{
  assert( ref != "" );
  _Push( new ReferencePortion( ref ) );
  return true;
}



bool GSM::Assign( void )
{
  Portion* p2;
  Portion* p1;
  bool result = true;
  gString varname;

  p2 = _Pop();
  p1 = _Pop();
  if(p1->Type() == porREFERENCE)
    varname = ((ReferencePortion*) p1)->Value();
  p2 = _ResolveRef(p2);
  p1 = _ResolveRef(p1);
  
  if(p1->Type() == porREFERENCE)
  {
    if(p2->Type() != porREFERENCE)
    {
      _VarDefine(((ReferencePortion*) p1)->Value(), p2);
      delete p1;
      _Push(p2->RefCopy());
    }
    else
    {
      _ErrorMessage(_StdErr, 63, 0, 0, ((ReferencePortion*) p2)->Value());
      result = false;
    } 
  }
  else if(p2->Type() == porREFERENCE)
  {
    _ErrorMessage(_StdErr, 63, 0, 0, ((ReferencePortion*) p2)->Value());
    result = false;
  }
  else if(p1->Type() == p2->Type())
  {
    if(p1->Type() != porLIST)
    {
      if(!(p1->Type() & (porINPUT|porOUTPUT))) 
      {
	switch(p1->Type())
	{
	case porINTEGER:
	  ((IntPortion*) p1)->Value() = ((IntPortion*) p2)->Value();
	  break;
	case porFLOAT:
	  ((FloatPortion*) p1)->Value() = ((FloatPortion*) p2)->Value();
	  break;
	case porRATIONAL:
	  ((RationalPortion*) p1)->Value() = ((RationalPortion*) p2)->Value();
	  break;
	case porTEXT:
	  ((TextPortion*) p1)->Value() = ((TextPortion*) p2)->Value();
	  break;
	case porBOOL:
	  ((BoolPortion*) p1)->Value() = ((BoolPortion*) p2)->Value();
	  break;
	case porOUTCOME_FLOAT:
	case porOUTCOME_RATIONAL:
	  p1->RemoveDependency();
	  ((OutcomePortion*) p1)->Value() = ((OutcomePortion*) p2)->Value();
	  p1->SetOwner( p2->Owner() );
	  p1->SetIsValid( p2->IsValid() );
	  p1->AddDependency();
	  break;
	case porPLAYER_NFG:
	  p1->RemoveDependency();
	  ((NfPlayerPortion*) p1)->Value() = ((NfPlayerPortion*) p2)->Value();
	  p1->SetOwner( p2->Owner() );
	  p1->SetIsValid( p2->IsValid() );
	  p1->AddDependency();
	  break;
	case porSTRATEGY:
	  p1->RemoveDependency();
	  ((StrategyPortion*) p1)->Value() = ((StrategyPortion*) p2)->Value();
	  p1->SetOwner( p2->Owner() );
	  p1->SetIsValid( p2->IsValid() );
	  p1->AddDependency();
	  break;
	case porNF_SUPPORT:
	  p1->RemoveDependency();
	  ((NfSupportPortion*) p1)->Value() =((NfSupportPortion*) p2)->Value();
	  p1->SetOwner( p2->Owner() );
	  p1->SetIsValid( p2->IsValid() );
	  p1->AddDependency();
	  break;
	case porEF_SUPPORT:
	  p1->RemoveDependency();
	  ((EfSupportPortion*) p1)->Value() =((EfSupportPortion*) p2)->Value();
	  p1->SetOwner( p2->Owner() );
	  p1->SetIsValid( p2->IsValid() );
	  p1->AddDependency();
	  break;
	case porPLAYER_EFG:
	  p1->RemoveDependency();
	  ((EfPlayerPortion*) p1)->Value() = ((EfPlayerPortion*) p2)->Value();
	  p1->SetOwner( p2->Owner() );
	  p1->SetIsValid( p2->IsValid() );
	  p1->AddDependency();
	  break;
	case porINFOSET:
	  p1->RemoveDependency();
	  ((InfosetPortion*) p1)->Value() = ((InfosetPortion*) p2)->Value();
	  p1->SetOwner( p2->Owner() );
	  p1->SetIsValid( p2->IsValid() );
	  p1->AddDependency();
	  break;
	case porNODE:
	  p1->RemoveDependency();
	  ((NodePortion*) p1)->Value() = ((NodePortion*) p2)->Value();
	  p1->SetOwner( p2->Owner() );
	  p1->SetIsValid( p2->IsValid() );
	  p1->AddDependency();
	  break;
	case porACTION:
	  p1->RemoveDependency();
	  ((ActionPortion*) p1)->Value() = ((ActionPortion*) p2)->Value();
	  p1->SetOwner( p2->Owner() );
	  p1->SetIsValid( p2->IsValid() );
	  p1->AddDependency();
	  break;
	case porMIXED_FLOAT:
	  p1->RemoveDependency();
	  (*((MixedSolution<double>*) ((MixedPortion*) p1)->Value())) = 
	    (*((MixedSolution<double>*) ((MixedPortion*) p2)->Value()));
	  p1->SetOwner( p2->Owner() );
	  p1->SetIsValid( p2->IsValid() );
	  p1->AddDependency();
	  break;
	case porMIXED_RATIONAL:
	  p1->RemoveDependency();
	  (*((MixedSolution<gRational>*) ((MixedPortion*) p1)->Value())) = 
	    (*((MixedSolution<gRational>*) ((MixedPortion*) p2)->Value()));
	  p1->SetOwner( p2->Owner() );
	  p1->SetIsValid( p2->IsValid() );
	  p1->AddDependency();
	  break;
	case porBEHAV_FLOAT:
	  p1->RemoveDependency();
	  (*((BehavSolution<double>*) ((BehavPortion*) p1)->Value())) = 
	    (*((BehavSolution<double>*) ((BehavPortion*) p2)->Value()));
	  p1->SetOwner( p2->Owner() );
	  p1->SetIsValid( p2->IsValid() );
	  p1->AddDependency();
	  break;
	case porBEHAV_RATIONAL:
	  p1->RemoveDependency();
	  (*((BehavSolution<gRational>*) ((BehavPortion*) p1)->Value())) = 
	    (*((BehavSolution<gRational>*) ((BehavPortion*) p2)->Value()));
	  p1->SetOwner( p2->Owner() );
	  p1->SetIsValid( p2->IsValid() );
	  p1->AddDependency();
	  break;
	case porNFG_FLOAT:
	  ((NfgPortion*) p1)->RemoveAllDependents();
	  delete ((NfgPortion*) p1)->Value();
	  ((NfgPortion*) p1)->Value() = new Nfg<double>
	    (*(Nfg<double>*) ((NfgPortion*) p2)->Value()); 
	  break;
	case porNFG_RATIONAL:
	  ((NfgPortion*) p1)->RemoveAllDependents();
	  delete ((NfgPortion*) p1)->Value();
	  ((NfgPortion*) p1)->Value() =  new Nfg<gRational>
	    (*(Nfg<gRational>*) ((NfgPortion*) p2)->Value()); 
	  break;
	case porEFG_FLOAT:
	  ((EfgPortion*) p1)->RemoveAllDependents();
	  delete ((EfgPortion*) p1)->Value();
	  ((EfgPortion*) p1)->Value() = new Efg<double>
	    (*(Efg<double>*) ((EfgPortion*) p2)->Value()); 
	  break;
	case porEFG_RATIONAL:
	  ((EfgPortion*) p1)->RemoveAllDependents();
	  delete ((EfgPortion*) p1)->Value();
	  ((EfgPortion*) p1)->Value() =  new Efg<gRational>
	    (*(Efg<gRational>*) ((EfgPortion*) p2)->Value()); 
	  break;
	default:
	  _ErrorMessage(_StdErr, 67, 0, 0, PortionTypeToText(p1->Type()));
	  assert(0);	  
	}
	_Push(p1->RefCopy()); 
	delete p2;
      }
      else 
      {
	_ErrorMessage(_StdErr, 64);
	result = false;
      }
    }
    else if((((ListPortion*) p1)->DataType() ==
	     ((ListPortion*) p2)->DataType()) ||
	    (((ListPortion*) p1)->DataType() == porUNDEFINED) )
    {
      if( !( ((ListPortion*) p1)->DataType() & (porINPUT|porOUTPUT) ) )
      {
	((ListPortion*) p1)->AssignFrom(p2);
	_Push(p1->RefCopy());
	delete p2;
      }
      else
      {
	_ErrorMessage(_StdErr, 64);
	result = false;
      }
    }
    else
    {      
      _ErrorMessage(_StdErr, 65);
      result = false;
    }
  }
  else if(PortionTypeMatch(p1->Type(), p2->Type()))
  {
    _VarDefine(varname, p2);
    delete p1;
    _Push(p2->RefCopy());
  }
  else
  {
    _ErrorMessage(_StdErr, 66);
    result = false;
  }

  if(!result)
  { delete p2; delete p1; }
  return result;
}





bool GSM::UnAssign( void )
{
  Portion* p;

#ifndef NDEBUG
  if( _Depth() < 1 )
  {
    gerr << "  Not enough operands to execute UnAssign[]\n";
  }
  assert( _Depth() >= 1 );
#endif // NDEBUG

  p = _Pop();
  if( p->Type() == porREFERENCE )
  {
    if( _VarIsDefined( ( (ReferencePortion*) p )->Value() ) )
    {
      _Push( _VarRemove( ( (ReferencePortion*) p )->Value() ) );
      delete p;
      return true;
    }
    else
    {
      _Push( p );
      _ErrorMessage( _StdErr, 54, 0, 0, ((ReferencePortion*) p)->Value() );
      return false;
    }
  }
  else
  {
    _Push( p );
    _ErrorMessage( _StdErr, 53 );
    return false;
  }
}


Portion* GSM::UnAssignExt( void )
{
  Portion* p;
  gString txt;

#ifndef NDEBUG
  if( _Depth() < 1 )
  {
    gerr << "  Not enough operands to execute UnAssign[]\n";
  }
  assert( _Depth() >= 1 );
#endif // NDEBUG

  p = _Pop();
  if( p->Type() == porREFERENCE )
  {
    if( _VarIsDefined( ( (ReferencePortion*) p )->Value() ) )
    {
      delete p;
      p = _VarRemove( ( (ReferencePortion*) p )->Value() );
      return p;
    }
    else
    {
      _Push( p );
      txt = "UnAssign[] called on undefined reference \"";
      txt += ((ReferencePortion*) p)->Value();
      txt += "\"\n";
      p = new ErrorPortion(txt);
      return p;
    }
  }
  else
  {
    _Push( p );
    txt = "UnAssign[] called on a non-reference value\n";
    p = new ErrorPortion(txt);
    return p;
  }
}


//-----------------------------------------------------------------------
//                        _ResolveRef functions
//-----------------------------------------------------------------------

Portion* GSM::_ResolveRef( Portion* p )
{
  Portion*  result = 0;
  gString ref;
  
  if( p->Type() == porREFERENCE )
  {
    ref = ( (ReferencePortion*) p )->Value();

    if( !_VarIsDefined( ref ) )
    {
      result = p;
    }
    else
    {
      if( _VarValue( ref )->IsValid() )
      {
	result = _VarValue( ref )->RefCopy();
	delete p;
      }
      else
      {
	delete _VarRemove( ref );
	result = p;
      }
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

bool GSM::_BinaryOperation( const gString& funcname )
{
  Portion*   p2;
  Portion*   p1;
  gList< Instruction* > prog;
  int result;

#ifndef NDEBUG
  if( _Depth() < 2 )
  {
    gerr << "  Not enough operands to perform binary operation\n";
  }
  assert( _Depth() >= 2 );
#endif // NDEBUG

  // bind the parameters in correct order
  p2 = _Pop();
  p1 = _Pop();
  _Push( p2 );
  _Push( p1 );
  
  prog.Append( new /* class */ ::InitCallFunction( funcname ) );
  prog.Append( new /* class */ ::Bind );
  prog.Append( new /* class */ ::Bind );
  prog.Append( new /* class */ ::CallFunction );
  result = Execute( prog );

  if( result == rcSUCCESS )
    return true;
  else
    return false;
}




//-----------------------------------------------------------------------
//                        unary operations
//-----------------------------------------------------------------------

bool GSM::_UnaryOperation( const gString& funcname )
{
  gList< Instruction* > prog;
  int result;

#ifndef NDEBUG
  if( _Depth() < 1 )
  {
    gerr << "  Not enough operands to perform unary operation\n";
  }
  assert( _Depth() >= 1 );
#endif // NDEBUG

  prog.Append( new /*class*/ :: InitCallFunction( funcname ) );
  prog.Append( new /*class*/ :: Bind );
  prog.Append( new /*class*/ :: CallFunction );
  result = Execute( prog );

  if( result == rcSUCCESS )
    return true;
  else
    return false;
}




//-----------------------------------------------------------------
//                      built-in operations
//-----------------------------------------------------------------

bool GSM::Add ( void )
{ 
  Portion* p1;
  Portion* p2;
  bool result = false;

  p2 = _Pop();
  p2 = _ResolveRef(p2);
  p1 = _Pop();
  p1 = _ResolveRef(p1);  

  if(p1->Type()==p2->Type())
  {
    result = true;
    if(p1->Type()==porINTEGER)
      ((IntPortion*) p1)->Value() += ((IntPortion*) p2)->Value();
    else if(p1->Type()==porFLOAT)
      ((FloatPortion*) p1)->Value() += ((FloatPortion*) p2)->Value();
    else if(p1->Type()==porRATIONAL)
      ((RationalPortion*) p1)->Value() += ((RationalPortion*) p2)->Value();
    else if(p1->Type()==porMIXED_FLOAT)
      (*((MixedSolution<double>*) ((MixedPortion*) p1)->Value())) += 
	(*((MixedSolution<double>*) ((MixedPortion*) p2)->Value()));
    else if(p1->Type()==porMIXED_RATIONAL)
      (*((MixedSolution<gRational>*) ((MixedPortion*) p1)->Value())) += 
	(*((MixedSolution<gRational>*) ((MixedPortion*) p2)->Value()));
    else if(p1->Type()==porBEHAV_FLOAT)
      (*((BehavSolution<double>*) ((BehavPortion*) p1)->Value())) += 
	(*((BehavSolution<double>*) ((BehavPortion*) p2)->Value()));
    else if(p1->Type()==porBEHAV_RATIONAL)
      (*((BehavSolution<gRational>*) ((BehavPortion*) p1)->Value())) += 
	(*((BehavSolution<gRational>*) ((BehavPortion*) p2)->Value()));
    else
      result = false;
  }

  if(result)
  { delete p2; _Push(p1); }
  else
  { _Push(p1); _Push(p2); result = _BinaryOperation( "Plus" ); }
  return result;
}

bool GSM::Subtract ( void )
{ 
  Portion* p1;
  Portion* p2;
  bool result = false;

  p2 = _Pop();
  p2 = _ResolveRef(p2);
  p1 = _Pop();
  p1 = _ResolveRef(p1);  

  if(p1->Type()==p2->Type())
  {
    result = true;
    if(p1->Type()==porINTEGER)
      ((IntPortion*) p1)->Value() -= ((IntPortion*) p2)->Value();
    else if(p1->Type()==porFLOAT)
      ((FloatPortion*) p1)->Value() -= ((FloatPortion*) p2)->Value();
    else if(p1->Type()==porRATIONAL)
      ((RationalPortion*) p1)->Value() -= ((RationalPortion*) p2)->Value();
    else if(p1->Type()==porMIXED_FLOAT)
      (*((MixedSolution<double>*) ((MixedPortion*) p1)->Value())) -= 
	(*((MixedSolution<double>*) ((MixedPortion*) p2)->Value()));
    else if(p1->Type()==porMIXED_RATIONAL)
      (*((MixedSolution<gRational>*) ((MixedPortion*) p1)->Value())) -= 
	(*((MixedSolution<gRational>*) ((MixedPortion*) p2)->Value()));
    else if(p1->Type()==porBEHAV_FLOAT)
      (*((BehavSolution<double>*) ((BehavPortion*) p1)->Value())) -= 
	(*((BehavSolution<double>*) ((BehavPortion*) p2)->Value()));
    else if(p1->Type()==porBEHAV_RATIONAL)
      (*((BehavSolution<gRational>*) ((BehavPortion*) p1)->Value())) -= 
	(*((BehavSolution<gRational>*) ((BehavPortion*) p2)->Value()));
    else
      result = false;
  }

  if(result)
  { delete p2; _Push(p1); }
  else
  { _Push(p1); _Push(p2); result = _BinaryOperation( "Minus" ); }
  return result;
}

bool GSM::Multiply ( void )
{ 
  Portion* p1;
  Portion* p2;
  bool result = false;

  p2 = _Pop();
  p2 = _ResolveRef(p2);
  p1 = _Pop();
  p1 = _ResolveRef(p1);  

  if(p1->Type()==p2->Type())
  {
    result = true;
    if(p1->Type()==porINTEGER)
      ((IntPortion*) p1)->Value() *= ((IntPortion*) p2)->Value();
    else if(p1->Type()==porFLOAT)
      ((FloatPortion*) p1)->Value() *= ((FloatPortion*) p2)->Value();
    else if(p1->Type()==porRATIONAL)
      ((RationalPortion*) p1)->Value() *= ((RationalPortion*) p2)->Value();
    else
      result = false;
  }

  if(result)
  { delete p2; _Push(p1); }
  else
  { _Push(p1); _Push(p2); result = _BinaryOperation( "Times" ); }
  return result;
}


bool GSM::Dot ( void )
{ return _BinaryOperation( "Dot" ); }

bool GSM::Divide ( void )
{ 
  Portion* p1;
  Portion* p2;
  Portion* p;
  bool result = false;

  p2 = _Pop();
  p2 = _ResolveRef(p2);
  p1 = _Pop();
  p1 = _ResolveRef(p1);  

  if(p1->Type()==p2->Type())
  {
    result = true;
    if(p1->Type()==porINTEGER && ((IntPortion*) p2)->Value() != 0)
    {
      p = new RationalValPortion(((IntPortion*) p1)->Value());
      delete p1; 
      p1 = p;
      ((RationalPortion*) p1)->Value() /= ((IntPortion*) p2)->Value();
    }
    else if(p1->Type()==porFLOAT && ((FloatPortion*) p2)->Value() != 0)
      ((FloatPortion*) p1)->Value() /= ((FloatPortion*) p2)->Value();
    else if(p1->Type()==porRATIONAL && ((RationalPortion*) p2)->Value() != 0)
      ((RationalPortion*) p1)->Value() /= ((RationalPortion*) p2)->Value();
    else
      result = false;
  }

  if(result)
  { delete p2; _Push(p1); }
  else
  { _Push(p1); _Push(p2); result = _BinaryOperation( "Divide" ); }
  return result;
}

bool GSM::Negate( void )
{
  Portion* p1;
  bool result = true;

  p1 = _Pop();
  p1 = _ResolveRef(p1);  

  if(p1->Type()==porINTEGER)
    ((IntPortion*) p1)->Value() = -(((IntPortion*) p1)->Value());
  else if(p1->Type()==porFLOAT)
    ((FloatPortion*) p1)->Value() = -(((FloatPortion*) p1)->Value());
  else if(p1->Type()==porRATIONAL)
    ((RationalPortion*) p1)->Value() = -(((RationalPortion*) p1)->Value());
  else
    result = false;

  if(result)
    _Push(p1);
  else
  { _Push(p1); result = _UnaryOperation( "Negate" ); }
  return result;
}

bool GSM::Power( void )
{ return _BinaryOperation( "Power" ); }

bool GSM::Concat ( void )
{ return _BinaryOperation( "Concat" ); }


bool GSM::IntegerDivide ( void )
{ 
  Portion* p1;
  Portion* p2;
  bool result = false;

  p2 = _Pop();
  p2 = _ResolveRef(p2);
  p1 = _Pop();
  p1 = _ResolveRef(p1);  

  if(p1->Type()==p2->Type())
  {
    result = true;
    if(p1->Type()==porINTEGER && ((IntPortion*) p2)->Value() != 0)
      ((IntPortion*) p1)->Value() /= ((IntPortion*) p2)->Value();
    else
      result = false;
  }

  if(result)
  { delete p2; _Push(p1); }
  else
  { _Push(p1); _Push(p2); result = _BinaryOperation( "IntegerDivide" ); }
  return result;
}

bool GSM::Modulus ( void )
{
  Portion* p1;
  Portion* p2;
  bool result = false;

  p2 = _Pop();
  p2 = _ResolveRef(p2);
  p1 = _Pop();
  p1 = _ResolveRef(p1);  

  if(p1->Type()==p2->Type())
  {
    result = true;
    if(p1->Type()==porINTEGER && ((IntPortion*) p2)->Value() != 0)
      ((IntPortion*) p1)->Value() %= ((IntPortion*) p2)->Value();
    else
      result = false;
  }

  if(result)
  { delete p2; _Push(p1); }
  else
  { _Push(p1); _Push(p2); result = _BinaryOperation( "Modulus" ); }
  return result;
}


bool GSM::EqualTo ( void )
{ 
  Portion* p1;
  Portion* p2;
  bool b = false;
  bool result = false;

  p2 = _Pop();
  p2 = _ResolveRef(p2);
  p1 = _Pop();
  p1 = _ResolveRef(p1);  

  if(p1->Type()==p2->Type())
  {
    result = true;
    b = PortionEqual(p1, p2, result);
  }

  if(result)
  { delete p2; delete p1; _Push(new BoolValPortion(b)); }
  else
  { _Push(p1); _Push(p2); result = _BinaryOperation( "Equal" ); }
  return result;
}

bool GSM::NotEqualTo ( void )
{ 
  Portion* p1;
  Portion* p2;
  bool b = false;
  bool result = false;

  p2 = _Pop();
  p2 = _ResolveRef(p2);
  p1 = _Pop();
  p1 = _ResolveRef(p1);  

  if(p1->Type()==p2->Type())
  {
    result = true;
    b = !PortionEqual(p1, p2, result);
  }

  if(result)
  { delete p2; delete p1; _Push(new BoolValPortion(b)); }
  else
  { _Push(p1); _Push(p2); result = _BinaryOperation( "NotEqual" ); }
  return result;
}

bool GSM::GreaterThan ( void )
{ 
  Portion* p1;
  Portion* p2;
  bool b = false;
  bool result = false;

  p2 = _Pop();
  p2 = _ResolveRef(p2);
  p1 = _Pop();
  p1 = _ResolveRef(p1);  

  if(p1->Type()==p2->Type())
  {
    result = true;
    if(p1->Type()==porINTEGER)   
      b = (((IntPortion*) p1)->Value() > ((IntPortion*) p2)->Value());
    else if(p1->Type()==porFLOAT)
      b = (((FloatPortion*) p1)->Value() > ((FloatPortion*) p2)->Value());
    else if(p1->Type()==porRATIONAL)
      b = (((RationalPortion*) p1)->Value()>((RationalPortion*) p2)->Value());
    else if(p1->Type()==porTEXT)
      b = (((TextPortion*) p1)->Value() > ((TextPortion*) p2)->Value());
    else
      result = false;
  }

  if(result)
  { delete p2; delete p1; _Push(new BoolValPortion(b)); }
  else
  { _Push(p1); _Push(p2); result = _BinaryOperation( "Greater" ); }
  return result;
}

bool GSM::LessThan ( void )
{ 
  Portion* p1;
  Portion* p2;
  bool b = false;
  bool result = false;

  p2 = _Pop();
  p2 = _ResolveRef(p2);
  p1 = _Pop();
  p1 = _ResolveRef(p1);  

  if(p1->Type()==p2->Type())
  {
    result = true;
    if(p1->Type()==porINTEGER)   
      b = (((IntPortion*) p1)->Value() < ((IntPortion*) p2)->Value());
    else if(p1->Type()==porFLOAT)
      b = (((FloatPortion*) p1)->Value() < ((FloatPortion*) p2)->Value());
    else if(p1->Type()==porRATIONAL)
      b = (((RationalPortion*) p1)->Value()<((RationalPortion*) p2)->Value());
    else if(p1->Type()==porTEXT)
      b = (((TextPortion*) p1)->Value() < ((TextPortion*) p2)->Value());
    else
      result = false;
  }

  if(result)
  { delete p2; delete p1; _Push(new BoolValPortion(b)); }
  else
  { _Push(p1); _Push(p2); result = _BinaryOperation( "LessEqual" ); }
  return result;
}

bool GSM::GreaterThanOrEqualTo ( void )
{ 
  Portion* p1;
  Portion* p2;
  bool b = false;
  bool result = false;

  p2 = _Pop();
  p2 = _ResolveRef(p2);
  p1 = _Pop();
  p1 = _ResolveRef(p1);  

  if(p1->Type()==p2->Type())
  {
    result = true;
    if(p1->Type()==porINTEGER)   
      b = (((IntPortion*) p1)->Value() >= ((IntPortion*) p2)->Value());
    else if(p1->Type()==porFLOAT)
      b = (((FloatPortion*) p1)->Value() >= ((FloatPortion*) p2)->Value());
    else if(p1->Type()==porRATIONAL)
      b = (((RationalPortion*) p1)->Value()>=((RationalPortion*) p2)->Value());
    else if(p1->Type()==porTEXT)
      b = (((TextPortion*) p1)->Value() >= ((TextPortion*) p2)->Value());
    else
      result = false;
  }

  if(result)
  { delete p2; delete p1; _Push(new BoolValPortion(b)); }
  else
  { _Push(p1); _Push(p2); result = _BinaryOperation( "GreaterEqual" ); }
  return result;
}

bool GSM::LessThanOrEqualTo ( void )
{ 
  Portion* p1;
  Portion* p2;
  bool b = false;
  bool result = false;

  p2 = _Pop();
  p2 = _ResolveRef(p2);
  p1 = _Pop();
  p1 = _ResolveRef(p1);  

  if(p1->Type()==p2->Type())
  {
    result = true;
    if(p1->Type()==porINTEGER)   
      b = (((IntPortion*) p1)->Value() <= ((IntPortion*) p2)->Value());
    else if(p1->Type()==porFLOAT)
      b = (((FloatPortion*) p1)->Value() <= ((FloatPortion*) p2)->Value());
    else if(p1->Type()==porRATIONAL)
      b = (((RationalPortion*) p1)->Value()<=((RationalPortion*) p2)->Value());
    else if(p1->Type()==porTEXT)
      b = (((TextPortion*) p1)->Value() <= ((TextPortion*) p2)->Value());
    else
      result = false;
  }

  if(result)
  { delete p2; delete p1; _Push(new BoolValPortion(b)); }
  else
  { _Push(p1); _Push(p2); result = _BinaryOperation( "LessEqual" ); }
  return result;
}


bool GSM::AND ( void )
{ 
  Portion* p1;
  Portion* p2;
  bool b = false;
  bool result = false;

  p2 = _Pop();
  p2 = _ResolveRef(p2);
  p1 = _Pop();
  p1 = _ResolveRef(p1);  

  if(p1->Type()==p2->Type())
  {
    result = true;
    if(p1->Type()==porBOOL)   
      b = (((BoolPortion*) p1)->Value() && ((BoolPortion*) p2)->Value());
    else
      result = false;
  }

  if(result)
  { delete p2; delete p1; _Push(new BoolValPortion(b)); }
  else
  { _Push(p1); _Push(p2); result = _BinaryOperation( "And" ); }
  return result;
}

bool GSM::OR ( void )
{ 
  Portion* p1;
  Portion* p2;
  bool b = false;
  bool result = false;

  p2 = _Pop();
  p2 = _ResolveRef(p2);
  p1 = _Pop();
  p1 = _ResolveRef(p1);  

  if(p1->Type()==p2->Type())
  {
    result = true;
    if(p1->Type()==porBOOL)   
      b = (((BoolPortion*) p1)->Value() || ((BoolPortion*) p2)->Value());
    else
      result = false;
  }

  if(result)
  { delete p2; delete p1; _Push(new BoolValPortion(b)); }
  else
  { _Push(p1); _Push(p2); result = _BinaryOperation( "Or" ); }
  return result;
}

bool GSM::NOT ( void )
{
  Portion* p1;
  bool result = true;

  p1 = _Pop();
  p1 = _ResolveRef(p1);  

  if(p1->Type()==porBOOL)
    ((BoolPortion*) p1)->Value() = !((BoolPortion*) p1)->Value();
  else
    result = false;

  if(result)
    _Push(p1);
  else
  { _Push(p1); result = _UnaryOperation( "Not" ); }
  return result;
}


bool GSM::Read ( void )
{ return _BinaryOperation( "Read" ); }

bool GSM::Write ( void )
{ return _BinaryOperation( "Write" ); }


bool GSM::Subscript ( void )
{
  Portion* p2;
  Portion* p1;

  assert( _Depth() >= 2 );
  p2 = _Pop();
  p1 = _Pop();

  p2 = _ResolveRef( p2 );
  p1 = _ResolveRef( p1 );

  _Push( p1 );
  _Push( p2 );

  if( p1->Type() == porTEXT )
    return _BinaryOperation( "NthChar" );
  else
    return _BinaryOperation( "NthElement" );
}


bool GSM::Child ( void )
{
  return _BinaryOperation( "NthChild" );
}



//-------------------------------------------------------------------
//               CallFunction() related functions
//-------------------------------------------------------------------

bool GSM::AddFunction( FuncDescObj* func )
{
  FuncDescObj *old_func;
  bool result;
  assert(func != 0);
  if( !_FuncTable->IsDefined( func->FuncName() ) )
  {
    _FuncTable->Define( func->FuncName(), func );
    return true;
  }
  else
  {
    old_func = (*_FuncTable)( func->FuncName() );
    result = old_func->Combine( func );
    if( !result )
      _ErrorMessage( _StdErr, 60, 0, 0, old_func->FuncName() );
    return result;
  }
}


#ifndef NDEBUG
void GSM::_BindCheck( void ) const
{
  if( _CallFuncStack->Depth() <= 0 )
  {
    gerr << "  The CallFunction() subsystem was not initialized by\n";
    gerr << "  calling InitCallFunction() first\n";
  }
  assert( _CallFuncStack->Depth() > 0 );

  if( _Depth() <= 0 )
  {
    gerr << "  No value found to assign to a function parameter\n";
  }
  assert( _Depth() > 0 );
}
#endif // NDEBUG


bool GSM::_Bind( const gString& param_name ) const
{
  return _CallFuncStack->Peek()->SetCurrParamIndex( param_name );
}


bool GSM::InitCallFunction( const gString& funcname )
{
  if( _FuncTable->IsDefined( funcname ) )
  {
    _CallFuncStack->Push
      ( new CallFuncObj( (*_FuncTable)( funcname ), _StdOut, _StdErr ) );
    return true;
  }
  else // ( !_FuncTable->IsDefined( funcname ) )
  {
    _ErrorMessage( _StdErr, 25, 0, 0, funcname );
    return false;
  }
}


bool GSM::Bind( const gString& param_name )
{
  return BindRef( param_name, AUTO_VAL_OR_REF );
}


bool GSM::BindVal( const gString& param_name )
{
  Portion*     param;
  bool         result = true;

#ifndef NDEBUG
  _BindCheck();
#endif // NDEBUG

  if( param_name != "" )
    result = _Bind( param_name );

  if( result )
  {
    param = _ResolveRef( _Pop() );

    if( param->IsValid() )
    {
      if( param->Type() != porREFERENCE )
	result = _CallFuncStack->Peek()->SetCurrParam( param->ValCopy() );
      else
	result = _CallFuncStack->Peek()->SetCurrParam( 0 );
    }
    else
    {
      _CallFuncStack->Peek()->SetErrorOccurred();
      _ErrorMessage( _StdErr, 61 );
      result = false;
    }
    delete param;
  }

  return result;
}


bool GSM::BindRef( const gString& param_name, bool auto_val_or_ref )
{
  Portion*           param;
  bool               result    = true;

#ifndef NDEBUG
  _BindCheck();
#endif // NDEBUG

  if( param_name != "" )
    result = _Bind( param_name );

  if( result )
  {
    param = _ResolveRef( _Pop() );

    if( param->IsValid() )
    {
      result = _CallFuncStack->Peek()->SetCurrParam( param, auto_val_or_ref );
    }
    else
    {
      _CallFuncStack->Peek()->SetErrorOccurred();
      _ErrorMessage( _StdErr, 59 );
      delete param;
      result = false;
    }
  }

  return result;
}



bool GSM::CallFunction( void )
{
  CallFuncObj*        func;
  Portion**           param;
  int                 index;
  ReferencePortion*   refp;
  Portion*            return_value;
  bool                define_result;
  bool                result = true;

#ifndef NDEBUG
  if( _CallFuncStack->Depth() <= 0 )
  {
    gerr << "  The CallFunction() subsystem was not initialized by\n";
    gerr << "  calling InitCallFunction() first\n";
  }
  assert( _CallFuncStack->Depth() > 0 );
#endif // NDEBUG

  func = _CallFuncStack->Pop();

  param = new Portion*[ func->NumParams() ];

  return_value = func->CallFunction( this, param );

  assert( return_value != 0 );

  if( return_value->Type() == porERROR )
    result = false;


  _Push( return_value );
  

  for( index = 0; index < func->NumParams(); index++ )
  {
    refp = func->GetParamRef( index );

    assert( (refp == 0) == (param[index] == 0) );

    if( refp != 0 )
    {
      define_result = _VarDefine( refp->Value(), param[ index ] );
      if( !define_result )
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


#include "garray.h"

//----------------------------------------------------------------------------
//                       Execute function
//----------------------------------------------------------------------------

int GSM::Execute( gList< Instruction* >& prog, bool user_func )
{
  int             result          = rcSUCCESS;
  bool            instr_success;
  bool            done            = false;
  Portion*        p;
  Instruction*    instruction;
  int             program_counter = 1;
  int             program_length  = prog.Length();
  int             initial_num_of_funcs = _CallFuncStack->Depth();
  int             i;
  CallFuncObj*    funcobj;
  gArray<Instruction*> program(prog.Length());

  for(i=1; i<=prog.Length(); i++)
    program[i] = prog[i];
  

  while( ( program_counter <= program_length ) && ( !done ) )
  {
    instruction = program[ program_counter ];
    switch( instruction->Type() )
    {
    case iQUIT:
      instr_success = true;
      result = rcQUIT;
      done = true;
      break;

    case iIF_GOTO:
      p = _Pop();
      if( p->Type() == porBOOL )
      {
	if( ( (BoolPortion*) p )->Value() )
	{
	  program_counter = ( (IfGoto*) instruction )->WhereTo();
	  assert( program_counter >= 1 && program_counter <= program_length );
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
	gerr << "Instruction IfGoto called on a non-boolean data type\n";
	assert( p->Type() == porBOOL );	
	_Push( p );
	program_counter++;
	instr_success = false;
      }
#endif // NDEBUG
      break;

    case iGOTO:
      program_counter = ( (Goto*) instruction )->WhereTo();
      assert( program_counter >= 1 && program_counter <= program_length );
      instr_success = true;
      break;

    default:
      instr_success = instruction->Execute( *this );
      program_counter++;
    }

    if( !instr_success )
    {
      result = instruction->LineNumber();
      done = true;
      break;
    }
  }


  for( i = _CallFuncStack->Depth(); i > initial_num_of_funcs; i-- )
  {
    funcobj = _CallFuncStack->Pop();
    delete funcobj;
  }
  assert( _CallFuncStack->Depth() == initial_num_of_funcs );



  if( !user_func )
  {
    while( prog.Length() > 0 )
    {
      delete prog.Remove( 1 );
    }
  }

  return result;
}





Portion* GSM::ExecuteUserFunc( gList< Instruction* >& program, 
			      const FuncInfoType& func_info,
			      Portion** param )
{
  int rc_result;
  Portion* result;
  Portion* result_copy;
  int i;

  _RefTableStack->Push( new RefHashTable );
  _StackStack->Push( new gStack< Portion* > );


  for( i = 0; i < func_info.NumParams; i++ )
  {
    if( param[ i ] != 0 && param[ i ]->Type() != porREFERENCE )
    {
      _VarDefine( func_info.ParamInfo[ i ].Name, param[ i ] );
      param[ i ] = param[ i ]->RefCopy();
    }
  }


  rc_result = Execute( program, true );


  switch( rc_result )
  {
  case rcSUCCESS:
    switch( _Depth() )
    {
    case 0:
      result = 
	new ErrorPortion( (gString)
			 "Error: No return value" );
      break;

    default:
      result = _Pop();
      result = _ResolveRef( result );
      result_copy = result->ValCopy();
      delete result;
      result = result_copy;
      result_copy = 0;
      if( result->Type() == porERROR )
      {
	delete result;
	result = 0;
      }
      break;
    }
    break;
  case rcQUIT:
    result = 
      new ErrorPortion( (gString)
		       "Error: Interruption by user" );
    break;

  default:
    if( rc_result >= 0 )
      result = new ErrorPortion( (gString)
				"Error at line " +
				ToString( rc_result / 65536) + 
				" in function, line " +
				ToString( rc_result % 65536) +
				" in source code" );
    else
      result = 0;
    Dump();
    break;
  }


  for( i = 0; i < func_info.NumParams; i++ )
  {
    if( func_info.ParamInfo[ i ].PassByReference )
    {
      if( _VarIsDefined( func_info.ParamInfo[ i ].Name ) )
      {
	assert( _VarValue( func_info.ParamInfo[ i ].Name ) != 0 );
	delete param[ i ];
	param[ i ] = _VarRemove( func_info.ParamInfo[ i ].Name );
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


void GSM::Output( void )
{
  Portion*  p;

  assert( _Depth() >= 0 );

  if( _Depth() == 0 )
  {
    // _StdOut << "\n";
  }
  else
  {
    p = _Pop();
    p = _ResolveRef( p );

    if( p->IsValid() )
    {
      p->Output( _StdOut );
      if( p->Type() == porREFERENCE )
	_StdOut << " (undefined)";
      _StdOut << "\n";
    }
    else
    {
      _StdOut << "(undefined)\n";
    }
    
    _Push( p );
  }
}


void GSM::Dump( void )
{
  int  i;

  assert( _Depth() >= 0 );

  if( _Depth() == 0 )
  {
    _StdOut << "Stack : NULL\n";
  }
  else
  {
    for( i = _Depth() - 1; i >= 0; i-- )
    {
      _StdOut << "Stack element " << i << " : ";
      Output();
      Pop();
    }
  }

  assert( _Depth() == 0 );
}


bool GSM::Pop( void )
{
  Portion* p;
  bool result = false;

  assert( _Depth() >= 0 );

  if( _Depth() > 0 )
  {
    p = _Pop();
    delete p;
    result = true;
  }
  else
  {
    result = true;
  }
  return result;
}


void GSM::Flush( void )
{
  int       i;
  bool result;

  assert( _Depth() >= 0 );
  for( i = _Depth() - 1; i >= 0; i-- )
  {
    result = Pop();
    assert( result == true );
  }

  assert( _Depth() == 0 );
}


void GSM::Clear( void )
{
  Flush();

  assert( _RefTableStack->Depth() == 1 );
  delete _RefTableStack->Pop();
  delete _RefTableStack;

  _RefTableStack = new gStack< RefHashTable* >( 1 );
  _RefTableStack->Push( new RefHashTable );

}




Portion* GSM::Help(gString funcname)
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

  if( _FuncTable->IsDefined( funcname ) )
  {
    func = (*_FuncTable)( funcname );
    gList<gString> list = func->FuncList();
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
	func = funclist[i];
	funcslist.Append(func);
	found++;
      }      
    }

    gFuncListSorter sorter(funcslist);
    if(found==1)
    {
      gList<gString> list = func->FuncList();
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

  if( _RefTableStack->Peek()->IsDefined( varname ) )
  {
    result = new ListValPortion();
    ((ListPortion*) result)->Append(new TextValPortion(varname + ":" + PortionTypeToText( (*(_RefTableStack->Peek()))(varname)->Type() )));
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
      ((ListPortion*) result)->Append(new TextValPortion(varslist[i] + ":" + PortionTypeToText( (*(_RefTableStack->Peek()))(varslist[i])->Type() )));
  }

  if(!result)
    result = new ErrorPortion("No match found");      
  return result;
}





//-----------------------------------------------------------------------
//                         _ErrorMessage
//-----------------------------------------------------------------------

void GSM::_ErrorMessage
(
 gOutput&        s,
 const int       error_num,
 const long& /*num1*/, 
 const long& /*num2*/,
 const gString&  str1,
 const gString& /*str2*/
 )
{
#if 0
  s << "GSM Error " << error_num << ":\n";
#endif // 0

  s << "GCL: ";

  switch( error_num )
  {
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
    s << "New " << str1 << "[] parameters ambiguous with existing function\n";
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
    s << "Attempted to change the type of a variable\n";
    break;
  case 67:
    s << "Assigning to an unknown type: " << str1 << "\n";
  default:
    s << "General error " << error_num << "\n";
  }
}


#include "garray.imp"
#include "gslist.imp"



#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#pragma option -Jgd
#define TEMPLATE
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class gArray<Instruction*>;
TEMPLATE class gSortList<FuncDescObj*>;
TEMPLATE class gListSorter<FuncDescObj*>;
TEMPLATE class gSortList<gString>;
TEMPLATE class gListSorter<gString>;


GSM __gsm__(256);
GSM& _gsm = __gsm__;

