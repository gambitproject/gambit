//#
//# FILE: gsm.cc  implementation of GSM (Stack machine)
//#
//# $Id$
//#


#include "gsm.h"

#include <assert.h>

#include "glist.h"
#include "ggrstack.h"

#include "portion.h"
#include "gsmfunc.h"
#include "gsminstr.h"
#include "gsmhash.h"

#include "gblock.h"
#include "infoset.h"
#include "node.h"

#include "garray.h"
#include "normal.h"
#include "extform.h"


//--------------------------------------------------------------------
//              global variables
//--------------------------------------------------------------------

// There must be a better way to do this
GSM* _CurrentGSM = 0;


//--------------------------------------------------------------------
//              implementation of GSM (Stack machine)
//--------------------------------------------------------------------

int GSM::_NumObj = 0;

GSM::GSM( int size, gInput& s_in, gOutput& s_out, gOutput& s_err )
:_StdIn( s_in ), _StdOut( s_out ), _StdErr( s_err )
{
#ifndef NDEBUG
  if( size <= 0 )
    _ErrorMessage( _StdErr, 1, size );
#endif // NDEBUG
  
  // global function default variables initialization
  // these should be done before InitFunctions() is called
  if( _NumObj == 0 )
  {
    _INPUT  = new InputRefPortion( _StdIn );
    _OUTPUT = new OutputRefPortion( _StdOut );
    _NULL   = new OutputRefPortion( gnull );

    _DefaultNfg = new NfgValPortion<double>( * new NormalForm<double> );
    _DefaultEfg = new EfgValPortion<gRational>( * new ExtForm<gRational> );
  }

  _StackStack    = new gGrowableStack< gGrowableStack< Portion* >* >( 1 );
  _StackStack->Push( new gGrowableStack< Portion* >( size ) );
  _CallFuncStack = new gGrowableStack< CallFuncObj* >( size ) ;
  _RefTableStack = new gGrowableStack< RefHashTable* >( 1 );
  _RefTableStack->Push( new RefHashTable );

  _RefCountTable = new RefCountHashTable< Portion* >;

  _FuncTable     = new FunctionHashTable;
  InitFunctions();  // This function is located in gsmfunc.cc

  _NumObj++;
}


GSM::~GSM()
{
  int i;

  _NumObj--;

  for( i = _CallFuncStack->Depth(); i > 0; i-- )
  {
    _ErrorMessage( _StdErr, 47 );
    CallFunction();
  }
  assert( _CallFuncStack->Depth() == 0 );
  delete _CallFuncStack;

  Flush();
  delete _FuncTable;

  delete _RefCountTable;

  assert( _RefTableStack->Depth() == 1 );
  delete _RefTableStack->Pop();
  delete _RefTableStack;

  assert( _StackStack->Depth() == 1 );
  delete _StackStack->Pop();
  delete _StackStack;

  if( _NumObj == 0 )
  {
    delete _DefaultNfg;
    delete _DefaultEfg;
    
    delete _INPUT;
    delete _OUTPUT;
    delete _NULL;
  }
}


int GSM::Depth( void ) const
{
  return _Depth();
}


int GSM::MaxDepth( void ) const
{
  return _StackStack->Peek()->MaxDepth();
}


Portion*& GSM::DefaultNfg( void )
{ return _DefaultNfg; }

Portion*& GSM::DefaultEfg( void )
{ return _DefaultEfg; }



//------------------------------------------------------------------------
//                           Push() functions
//------------------------------------------------------------------------

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


bool GSM::PushList( const int num_of_elements )
{ 
  int            i;
  Portion*       p;
  ListPortion*  list;
  int            insert_result;
  bool           result = true;

#ifndef NDEBUG
  if( num_of_elements < 0 )
    _ErrorMessage( _StdErr, 2, num_of_elements );

  if( num_of_elements > _Depth() )
    _ErrorMessage( _StdErr, 3, num_of_elements, _Depth() );
#endif // NDEBUG

  list = new ListValPortion;
  for( i = 1; i <= num_of_elements; i++ )
  {
    p = _Pop();
    p = _ResolveRef( p );

    if( p->Type() != porREFERENCE )
    {
      insert_result = list->Insert( p, 1 );
      if( insert_result == 0 )
      {
	_ErrorMessage( _StdErr, 35 );
	result = false;
      }
    }
    else
    {
      delete p;
      _ErrorMessage( _StdErr, 49 );
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

  if( var_name == "OUTPUT" || var_name == "INPUT" || var_name == "NULL" )
    result = true;
  else
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

  if( var_name == "OUTPUT" || var_name == "INPUT" || var_name == "NULL" )
  {
    read_only = true;
  }
  else if( _RefTableStack->Peek()->IsDefined( var_name ) )
  {
    old_value = (*_RefTableStack->Peek())( var_name );
    if( old_value->Type() != p->Type() )
    {
      if( 
	 !( 
	   ( ( old_value->Type() & porMIXED ) && ( p->Type() & porMIXED ) ) ||
	   ( ( old_value->Type() & porBEHAV ) && ( p->Type() & porBEHAV ) ) ||
	   ( ( old_value->Type() & porOUTCOME ) && 
	    ( p->Type() & porOUTCOME ) ) ||
	   ( ( old_value->Type() & porNFG   ) && ( p->Type() & porNFG   ) ) ||
	   ( ( old_value->Type() & porEFG   ) && ( p->Type() & porEFG   ) ) 
	   ) 
	 )
	type_match = false;
    }
    else if( p->Type() == porLIST )
    {
      assert( old_value->Type() == porLIST );
      if( ( (ListPortion*) old_value )->DataType() != 
	 ( (ListPortion*) p )->DataType() )
      {
	if( ( (ListPortion*) p )->DataType() == porUNKNOWN )
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
    if( _VarIsDefined( var_name ) )
    {
      _RefCountTable->Remove( _VarValue( var_name ) );
    }
    _RefTableStack->Peek()->Define( var_name, p );
    _RefCountTable->Define( p, 1 );
  }
  return result;
}


Portion* GSM::_VarValue( const gString& var_name ) const
{
  Portion* result;

  assert( var_name != "" );

  if( var_name == "INPUT" )
    result = _INPUT;
  else if( var_name == "OUTPUT" )
    result = _OUTPUT;
  else if( var_name == "NULL" )
    result = _NULL;
  else
  {
    result = (*_RefTableStack->Peek())( var_name );
    assert( _RefCountTable->IsDefined( result ) );
  }

  return result;
}


Portion* GSM::_VarRemove( const gString& var_name )
{
  Portion* result;

  assert( var_name != "" );

  if( var_name == "INPUT" || var_name == "OUTPUT" || var_name == "NULL" )
  {
    _ErrorMessage( _StdErr, 55, 0, 0, var_name );
    result = _VarValue( var_name )->ValCopy();
  }
  else
  {
    result = _VarValue( var_name )->ValCopy();
    _RefCountTable->Remove( _VarValue( var_name ) );
    _RefTableStack->Peek()->Remove( var_name );
  }
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
  _Push( new ReferencePortion( ref ) );
  return true;
}


bool GSM::Assign( void )
{
  Portion*  p2_copy;
  Portion*  p2;
  Portion*  p1;
  bool      result = true;
  PortionType p1_type;
  PortionType p2_type;
  
#ifndef NDEBUG
  if( _Depth() < 2 )
    _ErrorMessage( _StdErr, 4 );
#endif // NDEBUG

  p2 = _Pop();
  p1 = _Pop();

  p2 = _ResolveRef( p2 );
  p2_type = p2->Type();

  p1_type = p1->Type();

  if( p2->Type() == porREFERENCE )
  {
    _ErrorMessage( _StdErr, 13, 0, 0, ( (ReferencePortion*) p2 )->Value() );
    result = false;
    _Push( new ErrorPortion );
    delete p1;
    delete p2;
    return result;
  }

  else if( p1_type == porREFERENCE )
  { 
    gString& RefName = ( (ReferencePortion*) p1 )->Value();
    if( _VarIsDefined( RefName ) )
      p1_type = _VarValue( RefName )->Type();
    
    if( ( p1_type & porMIXED && p2_type & porMIXED ) ||
       ( p1_type & porBEHAV && p2_type & porBEHAV ) ||
       ( p1_type & porOUTCOME && p2_type & porOUTCOME ) ||
       ( p1_type & porNFG && p2_type & porNFG ) ||
       ( p1_type & porEFG && p2_type & porEFG ) )
    {
      delete _VarRemove( RefName );
      _VarDefine( RefName, p2 );
      delete p1;
      _Push( p2->RefCopy() );
      
      return result;
    }
  }

  

  p1 = _ResolveRef( p1 );
  
  if ( p1->Type() == porREFERENCE )
  {
    if( p2->IsReference() )
    {
      p2_copy = p2->ValCopy();
      result = _VarDefine( ( (ReferencePortion*) p1 )->Value(), p2_copy );
      delete p1;
      delete p2;
      _Push( p2_copy->RefCopy() );
    }
    else
    {
      result = _VarDefine( ( (ReferencePortion*) p1 )->Value(), p2 );
      delete p1;
      _Push( p2->RefCopy() );
    }
  }
  
  else // ( p1->Type() != porREFERENCE )
  {
    if( p1->IsReference() )
    {
      if( p1->Type() == p2->Type() )
      {
	switch( p1->Type() )
	{
	case porLIST:
	  if( ( ( (ListPortion*) p1 )->DataType() == 
	       ( (ListPortion*) p2 )->DataType() ) ||
	     ( (ListPortion*) p1 )->DataType() == porUNKNOWN )
	  {
	    p1->AssignFrom( p2 );
	    delete p2;
	    _Push( p1 );
	  }
	  else
	  {
	    _ErrorMessage( _StdErr, 56 );
	    _Push( p2 );
	    result = false;
	    delete p1;
	  }
	  break;
	  
	case porOUTPUT:
	case porINPUT:	
	  _ErrorMessage( _StdErr, 52 );
	  _Push( p2 );
	  result = false;
	  delete p1;
	  break;
	  
	case porNFG_FLOAT:
	case porNFG_RATIONAL:
	case porEFG_FLOAT:
	case porEFG_RATIONAL:
	  assert( 0 );
	  break;
	  
	default:
	  p1->AssignFrom( p2 );
	  delete p2;
	  _Push( p1 );
	}
      }
      else
      {
	_ErrorMessage( _StdErr, 48 );
	_Push( p2 );
	result = false;
	delete p1;
      }
    }
    else
    {
      _ErrorMessage( _StdErr, 57 );
      _Push( p1 );
      delete p2;
      result = false;
    }
  }

  return result;
}



bool GSM::UnAssign( void )
{
  Portion* p;

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
      _ErrorMessage( _StdErr, 54 );
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
    else if( _VarValue( ref )->Owner() != 0 && 
	    !_RefCountTable->IsDefined( _VarValue( ref )->Owner() ) )
    {
      delete _VarRemove( ref );
      result = p;
    }
    else
    {
      result = _VarValue( ref )->RefCopy();
      delete p;
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

// operations are dispatched to the appropriate Portion classes,
// except in cases where the return type is differen from parameter
// #1, in which case the operation implementation would be placed here
// labeled as SPECIAL CASE HANDLING

bool GSM::_BinaryOperation( const gString& funcname )
{
  Portion*   p2;
  Portion*   p1;
  Portion*   p;
  Portion*   result = 0;

#ifndef NDEBUG
  if( _Depth() < 2 )
    _ErrorMessage( _StdErr, 16 );
#endif // NDEBUG
  
  p2 = _Pop();
  p1 = _Pop();
  
  p2 = _ResolveRef( p2 );
  p1 = _ResolveRef( p1 );

  if( p1->Type() == p2->Type() )
  {
    // SPECIAL CASE HANDLING - Integer division to produce gRationals
    if( funcname == "Divide" && p1->Type() == porINTEGER &&
       ( (IntPortion*) p2 )->Value() != 0 )
    {
      p = new RationalValPortion( ( (IntPortion*) p1 )->Value() );
      ( (RationalPortion*) p )->Value() /= ( ( (IntPortion*) p2 )->Value() );
      delete p2;
      delete p1;
      p1 = p;
    }
    else
    {
      // Main operations dispatcher
      InitCallFunction( funcname );
      _Push( p1 );
      Bind();
      _Push( p2 );
      Bind();
      CallFunction();
      result = _Pop();
      if( result->Type() != porERROR )
      {
	p1 = result;
	result = 0;
      }
      else
      {
	p1 = new ErrorPortion;
      }
    }
  }

  else // ( p1->Type() != p2->Type() )
  {
    _ErrorMessage( _StdErr, 17, (int) p1->Type(), (int) p2->Type() );
    delete p1;
    delete p2;
    p1 = new ErrorPortion;
    result = new ErrorPortion;
  }


  if( result == 0 )
  {
    _Push( p1 );
    return true;
  }
  else
  {
    assert( result->Type() == porERROR );
    if( ( (ErrorPortion*) result )->Value() != "" )
      result->Output( _StdErr );
    delete result;
    delete p1;
    p1 = new ErrorPortion;
    _Push( p1 );
    return false;
  }
}




//-----------------------------------------------------------------------
//                        unary operations
//-----------------------------------------------------------------------

bool GSM::_UnaryOperation( const gString& funcname )
{
  Portion*  result = 0;

  if( _Depth() >= 1 )
  {
    InitCallFunction( funcname );
    Bind();
    CallFunction();
    result = _StackStack->Peek()->Peek();
    if( result->Type() != porERROR )
      result = 0;
    else
      result = result->ValCopy();
  }
  else
  {
    _ErrorMessage( _StdErr, 18 );
    result = new ErrorPortion;
  }

  if( result == 0 )
    return true;
  else
  {
    assert( result->Type() == porERROR );
    if( ( (ErrorPortion*) result )->Value() != "" )
      result->Output( _StdErr );
    delete result;
    return false;
  }
}




//-----------------------------------------------------------------
//                      built-in operations
//-----------------------------------------------------------------

bool GSM::Add ( void )
{ return _BinaryOperation( "Plus" ); }

bool GSM::Subtract ( void )
{ return _BinaryOperation( "Minus" ); }

bool GSM::Multiply ( void )
{ return _BinaryOperation( "Times" ); }

bool GSM::Divide ( void )
{ return _BinaryOperation( "Divide" ); }

bool GSM::Negate( void )
{ return _UnaryOperation( "Negate" ); }


bool GSM::IntegerDivide ( void )
{ return _BinaryOperation( "IntegerDivide" ); }

bool GSM::Modulus ( void )
{ return _BinaryOperation( "Modulus" ); }


bool GSM::EqualTo ( void )
{ return _BinaryOperation( "Equal" ); }

bool GSM::NotEqualTo ( void )
{ return _BinaryOperation( "NotEqual" ); }

bool GSM::GreaterThan ( void )
{ return _BinaryOperation( "Greater" ); }

bool GSM::LessThan ( void )
{ return _BinaryOperation( "Less" ); }

bool GSM::GreaterThanOrEqualTo ( void )
{ return _BinaryOperation( "GreaterEqual" ); }

bool GSM::LessThanOrEqualTo ( void )
{ return _BinaryOperation( "LessEqual" ); }


bool GSM::AND ( void )
{ return _BinaryOperation( "And" ); }

bool GSM::OR ( void )
{ return _BinaryOperation( "Or" ); }

bool GSM::NOT ( void )
{ return _UnaryOperation( "Not" ); }


bool GSM::Subscript ( void )
{
  Portion* p2;
  Portion* p1;
  Portion* real_list;
  Portion* element;

  gString  old_string;
  gString  new_string;
  int      subscript;
  bool     result = true;

  assert( _Depth() >= 2 );
  p2 = _Pop();
  p1 = _Pop();

  p2 = _ResolveRef( p2 );
  p1 = _ResolveRef( p1 );

  if( p2->Type() == porINTEGER )
  {
    subscript = ( (IntPortion*) p2 )->Value();
    delete p2;
  }
  else
  {
    _ErrorMessage( _StdErr, 37 );
    delete p1;
    delete p2;
    _Push( new ErrorPortion );
    result = false;
    return result;
  }


  if( p1->Type() == porLIST )
  {
    real_list = p1;
    element = ( (ListPortion* ) real_list )->Subscript( subscript );
    if( element != 0 )
    {
      _Push( element );
    }
    else
    {
      _ErrorMessage( _StdErr, 36 );
      _Push( new ErrorPortion );
    }
    delete p1;
  }
  else if( p1->Type() == porTEXT )
  {
    old_string = ( (TextPortion*) p1 )->Value();
    if( subscript >= 1 && subscript <= old_string.length() )
    {
      new_string = old_string[ subscript - 1 ];
      delete p1;
      p1 = new TextValPortion( new_string );
      _Push( p1 );
    }
    else
    {
      _ErrorMessage( _StdErr, 36 );
      delete p1;
      _Push( new ErrorPortion );
      result = false;
    }
  }
  else
  {
    _ErrorMessage( _StdErr, 20 );
    delete p1;
    _Push( new ErrorPortion );
    result = false;
  }
  
  return result;
}


bool GSM::Child ( void )
{
  Portion* p2;
  Portion* p1;

  Node* old_node;
  Node* new_node;
  
  int      subscript;
  bool     result = true;

  assert( _Depth() >= 2 );
  p2 = _Pop();
  p1 = _Pop();

  p2 = _ResolveRef( p2 );
  p1 = _ResolveRef( p1 );

  if( p1->Type() == porNODE )
  {
    if( p2->Type() == porINTEGER )
    {
      subscript = ( (IntPortion*) p2 )->Value();
      old_node = ( (NodePortion*) p1 )->Value();
      delete p1;
      if( subscript >= 1 && subscript <= old_node->NumChildren() )
      {
	new_node = old_node->GetChild( subscript );
	p1 = new NodeValPortion( new_node );
	_Push( p1 );
      }
      else
      {
	_ErrorMessage( _StdErr, 40, old_node->NumChildren() );
	_Push( new ErrorPortion );
	result = false;
      }
    }
    else
    {
      _ErrorMessage( _StdErr, 38 );
      delete p1;
      _Push( new ErrorPortion );
      result = false;
    }
  }
  else
  {
    _ErrorMessage( _StdErr, 39 );
    delete p1;
    _Push( new ErrorPortion );
    result = false;
  }

  delete p2;
  return result;
}



//-------------------------------------------------------------------
//               CallFunction() related functions
//-------------------------------------------------------------------

bool GSM::AddFunction( FuncDescObj* func )
{
  FuncDescObj *old_func;
  if( !_FuncTable->IsDefined( func->FuncName() ) )
  {
    _FuncTable->Define( func->FuncName(), func );
    return true;
  }
  else
  {
    old_func = (*_FuncTable)( func->FuncName() );
    old_func->Combine( func );
    return true;
    /*
    _ErrorMessage( _StdErr, 50, 0, 0, func->FuncName() );
    delete func;
    return false;
    */
  }
}


#ifndef NDEBUG
void GSM::_BindCheck( void ) const
{
  if( _CallFuncStack->Depth() <= 0 )
    _ErrorMessage( _StdErr, 21 );

  if( _Depth() <= 0 )
    _ErrorMessage( _StdErr, 22 );
}
#endif // NDEBUG


bool GSM::_BindCheck( const gString& param_name ) const
{
  CallFuncObj*  func;
  int           new_index;
  bool          result = true;

#ifndef NDEBUG
  _BindCheck();
#endif // NDEBUG
  
  func = _CallFuncStack->Peek();
  new_index = func->FindParamName( param_name );
  
  if( new_index >= 0 )
  {
    func->SetCurrParamIndex( new_index );
  }
  else if ( new_index == PARAM_NOT_FOUND )
  {
    _ErrorMessage( _StdErr, 23, 0, 0, param_name, func->FuncName() );
    result = false;
  }
  else // ( new_index == PARAM_AMBIGUOUS )
  {
    _ErrorMessage( _StdErr, 24, 0, 0, param_name, func->FuncName() );
    result = false;
  }
  return result;
}


bool GSM::InitCallFunction( const gString& funcname )
{
  CallFuncObj*  func;
  bool          result = true;

  if( _FuncTable->IsDefined( funcname ) )
  {
    func = new CallFuncObj( (*_FuncTable)( funcname ), _StdOut, _StdErr );
    _CallFuncStack->Push( func );
  }
  else // ( !_FuncTable->IsDefined( funcname ) )
  {
    _ErrorMessage( _StdErr, 25, 0, 0, funcname );
    result = false;
  }
  return result;
}


bool GSM::Bind( void )
{
  return BindRef();
}


bool GSM::BindVal( void )
{
  CallFuncObj* func;
  Portion*     param;
  Portion*     org_param;
  bool         result;

#ifndef NDEBUG
  _BindCheck();
#endif // NDEBUG

  func = _CallFuncStack->Pop();
  param = _Pop();
  
  org_param = _ResolveRef( param );
  param = org_param->ValCopy();
  delete org_param;

  if( param->Type() == porREFERENCE )
  {
    delete param;
    param = new ErrorPortion;
  }

  result = func->SetCurrParam( param ); 

  _CallFuncStack->Push( func );

  return result;
}


bool GSM::BindRef( void )
{
  CallFuncObj*       func;
  Portion*           param;
  bool               result    = true;

#ifndef NDEBUG
  _BindCheck();
#endif // NDEBUG

  func = _CallFuncStack->Pop();
  param = _Pop();
  
  param = _ResolveRef( param );

  result = func->SetCurrParam( param );

  _CallFuncStack->Push( func );

  return result;
}




bool GSM::Bind( const gString& param_name )
{
  int result = false;

  if( _BindCheck( param_name ) )
  {
    result = Bind();
  }
  return result;
}


bool GSM::BindVal( const gString& param_name )
{
  int result = false;

  if( _BindCheck( param_name ) )
  {
    result = BindVal();
  }
  return result;
}


bool GSM::BindRef( const gString& param_name )
{
  int result = false;

  if( _BindCheck( param_name ) )
  {
    result = BindRef();
  }
  return result;
}


bool GSM::CallFunction( void )
{
  CallFuncObj*        func;
  Portion**           param;
  int                 num_params;
  int                 index;
  gString             ref;
  ReferencePortion*  refp;
  Portion*            return_value;
  bool                define_result;
  bool                result = true;

#ifndef NDEBUG
  if( _CallFuncStack->Depth() <= 0 )
    _ErrorMessage( _StdErr, 26 );
#endif // NDEBUG

  func = _CallFuncStack->Pop();

  num_params = func->NumParams();
  param = new Portion*[ num_params ];

  _CurrentGSM = this;
  return_value = func->CallFunction( this, param );

  if( return_value == 0 )
  {
    _ErrorMessage( _StdErr, 27, 0, 0, func->FuncName() );
    return_value = new ErrorPortion;
    result = false;
  }

  _Push( return_value );


  for( index = 0; index < num_params; index++ )
  {
    func->SetCurrParamIndex( index );
    refp = func->GetCurrParamRef();

    if( refp != 0 )
    {
      assert( param[ index ] != 0 );
      define_result = _VarDefine( refp->Value(), param[ index ] );
      if( !define_result )
	result = false;
      delete refp;
    }
    else
    {
      assert( param[ index ] == 0 );
    }
  }

  delete func;

  delete[] param;
  
  return result;
}


//----------------------------------------------------------------------------
//                       Execute function
//----------------------------------------------------------------------------

GSM_ReturnCode GSM::Execute( gList< Instruction* >& program, bool user_func )
{
  GSM_ReturnCode  result          = rcSUCCESS;
  bool            instr_success;
  bool            done            = false;
  Portion*        p;
  Instruction*    instruction;
  int             program_counter = 1;
  int             program_length  = program.Length();


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
      else
      {
	_ErrorMessage( _StdErr, 32 );
	_Push( p );
	program_counter++;
	instr_success = false;
      }
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
      _ErrorMessage( _StdErr, 33, program_counter - 1, 0,"","",0, instruction);
      result = rcFAIL;
      done = true;
      break;
    }
  }

  if( !user_func )
  {
    while( program.Length() > 0 )
    {
      delete program.Remove( 1 );
    }
  }

  return result;
}





Portion* GSM::ExecuteUserFunc( gList< Instruction* >& program, 
			      const FuncInfoType& func_info,
			      Portion** param )
{
  GSM_ReturnCode rc_result;
  Portion* result;
  Portion* result_copy;
  int i;

  _RefTableStack->Push( new RefHashTable );
  _StackStack->Push( new gGrowableStack< Portion* > );


  for( i = 0; i < func_info.NumParams; i++ )
  {
    if( param[ i ] != 0 && param[ i ]->Type() != porREFERENCE )
    {
      _VarDefine( func_info.ParamInfo[ i ].Name, param[ i ]->RefCopy() );
    }
  }


  rc_result = Execute( program, true );


  switch( rc_result )
  {
  case rcSUCCESS:
    switch( _Depth() )
    {
    case 0:
      result = new ErrorPortion( (gString)
				 "GSM Error 43 :\n" +
				 "  User-defined function Error:\n" +
				 "    No return value\n" );
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
  case rcFAIL:
    result = 0;
    break;
  case rcQUIT:
    result = new ErrorPortion( (gString)
			       "GSM Error 45 :\n" + 
			       "  User-defined function Error:\n" +
			       "    Interruption by user\n" );
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
    _StdOut << "\n";
  }
  else
  {
    p = _Pop();
    p = _ResolveRef( p );

    p->Output( _StdOut );
    if( p->Type() == porREFERENCE )
      _StdOut << " (undefined)";
    _StdOut << "\n";
    
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
  _StdOut << "\n";
  
  assert( _Depth() == 0 );
}


bool GSM::Pop( void )
{
  Portion* p;
  bool result = false;

  if( _Depth() > 0 )
  {
    p = _Pop();
    delete p;
    result = true;
  }
  else
  {
    result = true;
    // _ErrorMessage( _StdErr, 34 );
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



//-----------------------------------------------------------------------
//                         _ErrorMessage
//-----------------------------------------------------------------------

void GSM::_ErrorMessage
(
 gOutput&        s,
 const int       error_num,
 const long& num1, 
 const long& num2,
 const gString&  str1,
 const gString&  str2,
 Portion*        por,
 Instruction*    instr
 )
{
  s << "GSM Error " << error_num << " :\n";

  switch( error_num )
  {
  case 1:
    s << "  Illegal stack size specified during initialization\n";
    s << "  Stack size requested: " << num1 << "\n";
    assert( 0 );
    break;
  case 2:
    s << "  Illegal number of elements requested to PushList()\n";
    s << "  Elements requested: " << num1 << "\n";
    assert( 0 );
    break;
  case 3:
    s << "  Not enough elements in GSM to PushList()\n";
    s << "  Elements requested: " << num1 << "\n";
    s << "  Elements available: " << num2 << "\n";
    assert( 0 );
    break;
  case 4:
    s << "  Not enough operands to execute Assign()\n";
    assert( 0 );
    break;
  case 5:
    s << "  Unknown type supports subvariables\n";
    assert(0);
    break;
  case 6:
    s << "  Attempted to assign a sub-reference to a type\n";
    s << "  that doesn't support such structures\n";
    break;
  case 7:
    s << "  No reference found to be assigned\n";
    break;
  case 8:
    s << "  Not enough operands to execute UnAssign()\n";
    assert( 0 );
    break;
  case 9:
    s << "  Unknown type supports subvariables\n";
    assert(0);
    break;
  case 10:
    s << "  Attempted to unassign a sub-reference of a type\n";
    s << "  that doesn't support such structures\n";
    break;
  case 11:
    s << "  No reference found to be unassigned\n";
    break;
  case 13:
    s << "  Attempted to resolve an undefined reference";
    s << " \"" << str1 << "\"\n";
    break;
  case 14:
    s << "  Attempted to resolve the subvariable of a type\n";
    s << "  that does not support subvariables\n";
    break;
  case 15:
    s << "  Attempted to resolve an undefined reference\n";
    s << "  \"" << str1 << "\"\n";
    break;
  case 16:
    s << "  Not enough operands to perform binary operation\n";
    assert( 0 );
    break;
  case 17:
    s << "  Attempted operating on different types\n";
    s << "  Type of Operand 1: ";
    PrintPortionTypeSpec( s, (PortionType) num1 );
    s << "\n";
    s << "  Type of Operand 2: ";
    PrintPortionTypeSpec( s, (PortionType) num2 );
    s << "\n";
    break;
  case 18:
    s << "  Not enough operands to perform unary operation\n";
    break;
  case 19:
    s << "  A non-integer element number passed as the\n";
    s << "  subscript of a List\n";
    break;
  case 20:
    s << "  Attempted to take the subscript of a non-List\n";
    s << "  Portion type\n";
    break;
  case 21:
    s << "  The CallFunction() subsystem was not initialized by\n";
    s << "  calling InitCallFunction() first\n";
    assert( 0 );
    break;
  case 22:
    s << "  No value found to assign to a function parameter\n";
    assert( 0 );
    break;
  case 23:
    s << "  Parameter \"" << str1 << "\" is not defined for\n";
    s << "  the function \"" << str2 << "\"\n";
    break;
  case 24:
    s << "  Parameter \"" << str1 << "\" is ambiguous in\n";
    s << "  the function \"" << str2 << "\"\n";
    break;
  case 25:
    s << "  Undefined function name:\n";
    s << "  InitCallFunction( \"" << str1 << "\" )\n";
    break;
  case 26:
    s << "  The CallFunction() subsystem was not initialized by\n";
    s << "  calling InitCallFunction() first\n";
    assert( 0 );
    break;
  case 27:
    s << "  An error occurred while attempting to execute\n";
    s << "  CallFunction( \"" << str1 << "\", ... )\n";
    break;
  case 28:
    s << "  Attempted to assign the subvariable of a\n";
    s << "  type that does not support subvariables\n";
    break;
  case 29:
    s << "  Attempted to assign the sub-variable of\n";
    s << "  an undefined variable\n";
    break;
  case 30:
    s << "  Fatal Error:\n";
    s << "    Returning function parameter information\n";
    s << "    (regarding lists) is invalid\n";
    assert(0);
    break;
  case 31:
    s << "  Fatal Error:\n";
    s << "    Attempting to assign a null Portion to a reference\n";
    s << "    Function: \"" << str1 << "\"\n";
    s << "    Parameter index: " << num1 << "\n";
    s << "    Reference: " << por << "\n";
    assert(0);
    break;
  case 32:
    s << "  Instruction IfGoto called on a non-boolean data type\n";
    break;
  case 33:
    s << "  Instruction #" << num1 << ": " << instr << "\n";
    s << "  was not executed successfully\n";
    s << "  Program abnormally terminated.\n";
    break;
  case 34:
    s << "  Pop() called on an empty stack\n";
    break;
  case 35:
    s << "  Attempted to insert conflicting Portion\n";
    s << "  types into a ListPortion.\n";
    break;
  case 36:
    s << "  Subscript out of range\n";
    break;
  case 37:
    s << "  A non-integer index specified\n";
    break;
  case 38:
    s << "  A non-integer child number passed to a Node\n";
    break;
  case 39:
    s << "  Attempted to find the child of a non-Node\n";
    s << "  Portion type\n";
    break;
  case 40:
    s << "  Node child number out of range\n";
    s << "  Only " << num1 << " child(ren) available\n";
    break;
  case 41:
    s << "  Warning: variable \"" << str1 << "\" has changed type\n";
    break;
  case 42:
    s << "  Attempted to change the type of variable \"" << str1 << "\"\n";
    break;
  case 43:
  case 44:
  case 45:
    s << "  User-defined function error\n";
    break;
  case 46:
    s << "  Attempted to assign to a read-only variable \"" << str1 << "\"\n";
    break;
  case 47:
    s << "  Mismatched InitCallFunction() and CallFunction() calls\n";
    break;
  case 48:
    s << "  Attempted to change the type of a variable\n";
    break;
  case 49:
    s << "  Attempted to insert an undefined reference into a list\n";
    break;
  case 50:
    s << "  Function \"" << str1 << "\" is already defined\n";
    break;
  case 51:
    s << "  Unable to assign to a read-only variable\n";
    break;
  case 52:
    s << "  Cannot assign to a Output or Input variable\n";
    break;
  case 53:
    s << "  Attempted calling UnAssign() on a non-reference value\n";
    break;
  case 54:
    s << "  Attempted calling UnAssign() on a undefined reference\n";
    break;
  case 55:
    s << "  Attempted to remove read-only variable \"" + str1 + "\"\n";
    break;
  case 56:
    s << "  Attempted to change the type of a List\n";
    break;
  case 57:
    s << "  Atttempted to assign to a non-reference type\n";
    break;
  default:
    s << "  General error\n";
  }
}






//-----------------------------------------------------------------------
//                       Template instantiations
//-----------------------------------------------------------------------



#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__




#include "gstack.imp"

TEMPLATE class gStack< Portion* >;
TEMPLATE class gStack< gGrowableStack< Portion* >* >;
TEMPLATE class gStack< CallFuncObj* >;
TEMPLATE class gStack< RefHashTable* >;

#include "ggrstack.imp"

TEMPLATE class gGrowableStack< Portion* >;
TEMPLATE class gGrowableStack< gGrowableStack< Portion* >* >;
TEMPLATE class gGrowableStack< CallFuncObj* >;
TEMPLATE class gGrowableStack< RefHashTable* >;


