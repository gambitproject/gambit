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

#include "infoset.h"
#include "node.h"

#include "garray.h"
#include "normal.h"
#include "extform.h"


//--------------------------------------------------------------------
//                 global variables
//--------------------------------------------------------------------

Portion* _DefaultNfgShadow = 0;
Portion* _DefaultEfgShadow = 0;

Portion* _OUTPUT = 0;
Portion* _INPUT = 0;
Portion* _NULL = 0;


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
    gArray<int> dim( 2 );
    dim[ 1 ] = 2;
    dim[ 2 ] = 2;
    _DefaultNfgShadow = new Error_Portion;
    _DefaultNfgShadow->ShadowOf() = 
      new Nfg_Portion<double>( * new NormalForm<double>( dim ) );
    
    _DefaultEfgShadow = new Error_Portion;
    _DefaultEfgShadow->ShadowOf() = 
      new Efg_Portion<double>( * new ExtForm<double> );
    
    _INPUT  = new Input_Portion ( _StdIn,   true );
    _OUTPUT = new Output_Portion( _StdOut,  true );
    _NULL   = new Output_Portion( gnull,    true );
  }

  _StackStack    = new gGrowableStack< gGrowableStack< Portion* >* >( 1 );
  _StackStack->Push( new gGrowableStack< Portion* >( size ) );
  _CallFuncStack = new gGrowableStack< CallFuncObj* >( size ) ;
  _RefTableStack = new gGrowableStack< RefHashTable* >( 1 );
  _RefTableStack->Push( new RefHashTable );

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

  assert( _RefTableStack->Depth() == 1 );
  delete _RefTableStack->Pop();
  delete _RefTableStack;

  assert( _StackStack->Depth() == 1 );
  delete _StackStack->Pop();
  delete _StackStack;

  if( _NumObj == 0 )
  {
    delete _DefaultNfgShadow->ShadowOf();
    delete _DefaultNfgShadow;
    
    delete _DefaultEfgShadow->ShadowOf();
    delete _DefaultEfgShadow;
    
    delete _INPUT;
    delete _OUTPUT;
    delete _NULL;
  }
}


int GSM::Depth( void ) const
{
  return _StackStack->Peek()->Depth();
}


int GSM::MaxDepth( void ) const
{
  return _StackStack->Peek()->MaxDepth();
}



//------------------------------------------------------------------------
//                           Push() functions
//------------------------------------------------------------------------

bool GSM::Push( const bool& data )
{
  _StackStack->Peek()->Push( new bool_Portion( data ) );
  return true;
}


bool GSM::Push( const double& data )
{
  _StackStack->Peek()->Push( new numerical_Portion<double>( data ) );
  return true;
}


bool GSM::Push( const gInteger& data )
{
  _StackStack->Peek()->Push( new numerical_Portion<gInteger>( data ) );
  return true;
}


bool GSM::Push( const gRational& data )
{
  _StackStack->Peek()->Push( new numerical_Portion<gRational>( data ) );
  return true;
}


bool GSM::Push( const gString& data )
{
  _StackStack->Peek()->Push( new gString_Portion( data ) );
  return true;
}


bool GSM::PushOutput( const gString& data )
{
  gOutput* g;
  g = new gFileOutput( data );
  _StackStack->Peek()->Push( new Output_Portion( *g ) );
  return true;
}


bool GSM::PushInput( const gString& data )
{
  gInput* g;
  g = new gFileInput( data );
  _StackStack->Peek()->Push( new Input_Portion( *g ) );
  return true;
}


bool GSM::PushList( const int num_of_elements )
{ 
  int            i;
  Portion*       p;
  List_Portion*  list;
  int            insert_result;
  bool           result = true;

#ifndef NDEBUG
  if( num_of_elements < 0 )
    _ErrorMessage( _StdErr, 2, num_of_elements );

  if( num_of_elements > _StackStack->Peek()->Depth() )
    _ErrorMessage( _StdErr, 3, num_of_elements, _StackStack->Peek()->Depth() );
#endif // NDEBUG

  list = new List_Portion;
  for( i = 1; i <= num_of_elements; i++ )
  {
    p = _StackStack->Peek()->Pop();
    p = _ResolveRef( p );

    insert_result = list->Insert( p, 1 );
    if( insert_result == 0 )
    {
      _ErrorMessage( _StdErr, 35 );
      result = false;
    }
  }
  _StackStack->Peek()->Push( list );

  return result;
}



//--------------------------------------------------------------------
//        Stack access related functions
//--------------------------------------------------------------------



bool GSM::_VarIsDefined( const gString& var_name ) const
{
  bool result;
  RefHashTable* ref_table;

  if( var_name == "OUTPUT" || var_name == "INPUT" || var_name == "NULL" )
    result = true;
  else
    result = _RefTableStack->Peek()->IsDefined( var_name );
  return result;
}


bool GSM::_VarDefine( const gString& var_name, Portion* p )
{
  RefHashTable* ref_table;
  Portion* old_value;
  bool type_match = true;
  bool read_only = false;
  bool result = true;

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
	   ( ( old_value->Type() & porNFG   ) && ( p->Type() & porNFG   ) ) ||
	   ( ( old_value->Type() & porEFG   ) && ( p->Type() & porEFG   ) ) 
	   ) 
	 )
	type_match = false;
    }
    else if( p->Type() == porLIST )
    {
      assert( old_value->Type() == porLIST );
      if( ( (List_Portion*) old_value )->DataType() != 
	 ( (List_Portion*) p )->DataType() )
      {
	if( ( (List_Portion*) p )->DataType() == porUNKNOWN )
	{
	  ( (List_Portion*) p )->
	    SetDataType( ( (List_Portion*) old_value )->DataType() );
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
  RefHashTable* ref_table;

  if( var_name == "INPUT" )
    result = _INPUT;
  else if( var_name == "OUTPUT" )
    result = _OUTPUT;
  else if( var_name == "NULL" )
    result = _NULL;
  else
    result = (*_RefTableStack->Peek())( var_name );

  return result;
}


//---------------------------------------------------------------------
//     Reference related functions: PushRef(), Assign(), UnAssign()
//---------------------------------------------------------------------


bool GSM::PushRef( const gString& ref )
{
  _StackStack->Peek()->Push( new Reference_Portion( ref ) );
  return true;
}


bool GSM::Assign( void )
{
  Portion*  p2_copy;
  Portion*  p2;
  Portion*  p1;
  Portion*  primary_ref;
  Portion*  por_result;
  bool      result = true;

#ifndef NDEBUG
  if( _StackStack->Peek()->Depth() < 2 )
    _ErrorMessage( _StdErr, 4 );
#endif // NDEBUG

  p2 = _StackStack->Peek()->Pop();
  p1 = _StackStack->Peek()->Pop();

  if ( p1->Type() == porREFERENCE )
  {
    if( p2->Type() == porREFERENCE )
    {
      p2 = _ResolveRef( (Reference_Portion*) p2 );
      p2_copy = p2->Copy( true );
    }
    else
    {
      p2_copy = p2->Copy();
    }
    result = _VarDefine( ( (Reference_Portion*) p1 )->Value(), p2_copy );
    delete p1;
    _StackStack->Peek()->Push( p2 );
  }

  else // ( p1->Type() != porREFERENCE )
  {
    int index = 0;
    if( p1->ShadowOf() != 0 )
    {
      index = p1->ShadowOf()->ParentList()->Value().Find( p1->ShadowOf() );
    }
    if( index > 0 )
    {
      por_result = p1->ShadowOf()->ParentList()->SetSubscript( index, p2 );

      if( por_result != 0 )
      {
	por_result->Output( _StdErr );
	delete por_result;
      }

      delete p1;
      _StackStack->Peek()->Push( p2->Copy() );
      result = true;
    }
    else
    {
      _ErrorMessage( _StdErr, 7 );
      delete p1;
      delete p2;
      _StackStack->Peek()->Push( new Error_Portion );
      result = false;
    }
  }
  return result;
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
    ref = ( (Reference_Portion*) p )->Value();
    result = _ResolveRefWithoutError( (Reference_Portion*) p );
    if( result == 0 )
    {
      _ErrorMessage( _StdErr, 13, 0, 0, ref );
      result = new Error_Portion;
    }
  }
  else
  {
    result = p;
  }
  return result;
}


Portion* GSM::_ResolveRefWithoutError( Reference_Portion* p )
{
  Portion*  result = 0;
  gString&  ref = p->Value();

  if( _VarIsDefined( ref ) )
  {
    result = _VarValue( ref )->Copy();
  }
  delete p;

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

bool GSM::_BinaryOperation( OperationMode mode )
{
  Portion*   p2;
  Portion*   p1;
  Portion*   p;
  Portion*   result = 0;

#ifndef NDEBUG
  if( _StackStack->Peek()->Depth() < 2 )
    _ErrorMessage( _StdErr, 16 );
#endif // NDEBUG
  
  p2 = _StackStack->Peek()->Pop();
  p1 = _StackStack->Peek()->Pop();
  
  p2 = _ResolveRef( p2 );
  p1 = _ResolveRef( p1 );

  if( p1->Type() == p2->Type() )
  {
    // SPECIAL CASE HANDLING - Integer division to produce gRationals
    if( mode == opDIVIDE && p1->Type() == porINTEGER &&
       ( (numerical_Portion<gInteger>*) p2 )->Value() != 0 )
    {
      p = new numerical_Portion<gRational>
	( ( (numerical_Portion<gInteger>*) p1 )->Value() );
      ( (numerical_Portion<gRational>*) p )->Value() /=
	( ( (numerical_Portion<gInteger>*) p2 )->Value() );
      delete p2;
      delete p1;
      p1 = p;
    }
    else
    {
      // Main operations dispatcher
      result = p1->Operation( p2, mode );
    }

    // SPECIAL CASE HANDLING - Boolean operators
    if( result != 0 && result->Type() == porBOOL )
    {
      delete p1;
      p1 = result;
      result = 0;
    }
  }

  else // ( p1->Type() != p2->Type() )
  {
    _ErrorMessage( _StdErr, 17, (int) p1->Type(), (int) p2->Type() );
    delete p1;
    delete p2;
    p1 = new Error_Portion;
    result = new Error_Portion;
  }


  if( result == 0 )
  {
    _StackStack->Peek()->Push( p1 );
    return true;
  }
  else
  {
    assert( result->Type() == porERROR );
    if( ( (Error_Portion*) result )->Value() != "" )
      result->Output( _StdErr );
    delete result;
    delete p1;
    p1 = new Error_Portion;
    _StackStack->Peek()->Push( p1 );
    return false;
  }
}




//-----------------------------------------------------------------------
//                        unary operations
//-----------------------------------------------------------------------

bool GSM::_UnaryOperation( OperationMode mode )
{
  Portion*  p1;
  Portion*  result = 0;

  if( _StackStack->Peek()->Depth() >= 1 )
  {
    p1 = _StackStack->Peek()->Pop();
    p1 = _ResolveRef( p1 );

    result = p1->Operation( 0, mode );
    _StackStack->Peek()->Push( p1 );
  }
  else
  {
    _ErrorMessage( _StdErr, 18 );
    result = new Error_Portion;
  }

  if( result == 0 )
    return true;
  else
  {
    assert( result->Type() == porERROR );
    if( ( (Error_Portion*) result )->Value() != "" )
      result->Output( _StdErr );
    delete result;
    return false;
  }
}




//-----------------------------------------------------------------
//                      built-in operations
//-----------------------------------------------------------------

bool GSM::Add ( void )
{ return _BinaryOperation( opADD ); }

bool GSM::Subtract ( void )
{ return _BinaryOperation( opSUBTRACT ); }

bool GSM::Multiply ( void )
{ return _BinaryOperation( opMULTIPLY ); }

bool GSM::Divide ( void )
{ return _BinaryOperation( opDIVIDE ); }

bool GSM::Negate( void )
{ return _UnaryOperation( opNEGATE ); }


bool GSM::IntegerDivide ( void )
{ return _BinaryOperation( opINTEGER_DIVIDE ); }

bool GSM::Modulus ( void )
{ return _BinaryOperation( opMODULUS ); }


bool GSM::EqualTo ( void )
{ return _BinaryOperation( opEQUAL_TO ); }

bool GSM::NotEqualTo ( void )
{ return _BinaryOperation( opNOT_EQUAL_TO ); }

bool GSM::GreaterThan ( void )
{ return _BinaryOperation( opGREATER_THAN ); }

bool GSM::LessThan ( void )
{ return _BinaryOperation( opLESS_THAN ); }

bool GSM::GreaterThanOrEqualTo ( void )
{ return _BinaryOperation( opGREATER_THAN_OR_EQUAL_TO ); }

bool GSM::LessThanOrEqualTo ( void )
{ return _BinaryOperation( opLESS_THAN_OR_EQUAL_TO ); }


bool GSM::AND ( void )
{ return _BinaryOperation( opLOGICAL_AND ); }

bool GSM::OR ( void )
{ return _BinaryOperation( opLOGICAL_OR ); }

bool GSM::NOT ( void )
{ return _UnaryOperation( opLOGICAL_NOT ); }


bool GSM::Subscript ( void )
{
  Portion* p2;
  Portion* p1;
  Portion* p;
  Portion* refp;
  Portion* real_list;
  Portion* element;
  Portion* shadow;

  gString  old_string;
  gString  new_string;
  int      subscript;
  bool     result = true;
  bool     was_ref = false;

  assert( _StackStack->Peek()->Depth() >= 2 );
  p2 = _StackStack->Peek()->Pop();
  p1 = _StackStack->Peek()->Pop();

  if( p1->Type() == porREFERENCE )
  {
    was_ref = true;
    refp = p1;

    if( _VarIsDefined( ( (Reference_Portion*) refp )->Value() ) )
      p1 = _VarValue( ( (Reference_Portion*) refp )->Value() );
    else
      p1 = 0;

    if( p1 != 0 )
    {
      if( p1->Type() == porLIST )
      {
	delete refp;
      }
      else if( p1->Type() == porSTRING )
      {
	p1 = p1->Copy();
	delete refp;
      }
      else
      {
	p1 = refp;
      }
    }
    else
    {
      p1 = refp;
    }
  }

  p2 = _ResolveRef( p2 );

  if( p2->Type() == porINTEGER )
  {
    subscript = (int) ( (numerical_Portion<gInteger>*) p2 )->Value().as_long();
    delete p2;
  }
  else
  {
    _ErrorMessage( _StdErr, 37 );
    if( p1->Type() != porLIST || !was_ref )
      delete p1;
    delete p2;
    _StackStack->Peek()->Push( new Error_Portion );
    result = false;
    return result;
  }


  if( p1->Type() == porLIST )
  {
    if( p1->ShadowOf() == 0 )
    {
      real_list = p1;
    }
    else
    {
      real_list = p1->ShadowOf();
    }
    element = ( (List_Portion* ) real_list )->GetSubscript( subscript );
    assert( element != 0 );
    if( element->Type() != porERROR )
    {
      shadow = element->Copy();
      shadow->ShadowOf() = element;
      _StackStack->Peek()->Push( shadow );
    }
    else
    {
      element->Output( _StdErr );
      delete element;
      _StackStack->Peek()->Push( new Error_Portion );
    }
    delete p1;
  }
  else if( p1->Type() == porSTRING )
  {
    old_string = ( (gString_Portion*) p1 )->Value();
    if( subscript >= 1 && subscript <= old_string.length() )
    {
      new_string = old_string[ subscript - 1 ];
      delete p1;
      p1 = new gString_Portion( new_string );
      _StackStack->Peek()->Push( p1 );
    }
    else
    {
      _ErrorMessage( _StdErr, 36 );
      delete p1;
      _StackStack->Peek()->Push( new Error_Portion );
      result = false;
    }
  }
  else
  {
    _ErrorMessage( _StdErr, 20 );
    delete p1;
    _StackStack->Peek()->Push( new Error_Portion );
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

  assert( _StackStack->Peek()->Depth() >= 2 );
  p2 = _StackStack->Peek()->Pop();
  p1 = _StackStack->Peek()->Pop();

  p2 = _ResolveRef( p2 );
  p1 = _ResolveRef( p1 );

  if( p1->Type() == porNODE )
  {
    if( p2->Type() == porINTEGER )
    {
      subscript = (int) ((numerical_Portion<gInteger>*)p2 )->Value().as_long();
      old_node = ( (Node_Portion*) p1 )->Value();
      delete p1;
      if( subscript >= 1 && subscript <= old_node->NumChildren() )
      {
	new_node = old_node->GetChild( subscript );
	p1 = new Node_Portion( new_node );
	_StackStack->Peek()->Push( p1 );
      }
      else
      {
	_ErrorMessage( _StdErr, 40, old_node->NumChildren() );
	_StackStack->Peek()->Push( new Error_Portion );
	result = false;
      }
    }
    else
    {
      _ErrorMessage( _StdErr, 38 );
      delete p1;
      _StackStack->Peek()->Push( new Error_Portion );
      result = false;
    }
  }
  else
  {
    _ErrorMessage( _StdErr, 39 );
    delete p1;
    _StackStack->Peek()->Push( new Error_Portion );
    result = false;
  }

  delete p2;
  return result;
}



//-------------------------------------------------------------------
//               CallFunction() related functions
//-------------------------------------------------------------------

void GSM::AddFunction( FuncDescObj* func )
{
  _FuncTable->Define( func->FuncName(), func );
}


#ifndef NDEBUG
void GSM::_BindCheck( void ) const
{
  if( _CallFuncStack->Depth() <= 0 )
    _ErrorMessage( _StdErr, 21 );

  if( _StackStack->Peek()->Depth() <= 0 )
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
  bool         result;

#ifndef NDEBUG
  _BindCheck();
#endif // NDEBUG

  func = _CallFuncStack->Pop();
  param = _StackStack->Peek()->Pop();
  
  param = _ResolveRef( param );

  param->ShadowOf() = 0;
  result = func->SetCurrParam( param ); 

  _CallFuncStack->Push( func );
  return result;
}


bool GSM::BindRef( void )
{
  CallFuncObj*       func;
  Portion*           param;
  Reference_Portion* ref_param = 0;
  bool               result    = true;

#ifndef NDEBUG
  _BindCheck();
#endif // NDEBUG

  func = _CallFuncStack->Pop();
  param = _StackStack->Peek()->Pop();
  
  if( param->Type() == porREFERENCE )
  {
    ref_param = (Reference_Portion*)( param->Copy() );
    param = _ResolveRefWithoutError( (Reference_Portion*) param );
    if( param != 0 )
    {
      if( param->Type() == porERROR )
      {
	delete param;
	delete ref_param;
	result = false;
      }
    }
  }
  else // ( param->Type() != porREFERENCE )
  {
    if( param->ShadowOf() == 0 )
    {
      _CallFuncStack->Push( func );
      _StackStack->Peek()->Push( param );
      result = BindVal();
      return result;
    }
  }

  
  if( result )  // == true
  {
    result = func->SetCurrParam( param, ref_param );
  }
  else
  {
    func->SetErrorOccurred();
  }

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
  int                 listindex;
  gString             ref;
  Reference_Portion*  refp;
  Portion*            return_value;
  Portion*            p;
  Portion*            shadowof;
  Portion*            por_result;
  bool                define_result;
  bool                result = true;

#ifndef NDEBUG
  if( _CallFuncStack->Depth() <= 0 )
    _ErrorMessage( _StdErr, 26 );
#endif // NDEBUG

  func = _CallFuncStack->Pop();

  num_params = func->NumParams();
  param = new Portion*[ num_params ];

  return_value = func->CallFunction( this, param );

  if( return_value == 0 )
  {
    _ErrorMessage( _StdErr, 27, 0, 0, func->FuncName() );
    return_value = new Error_Portion;
    result = false;
  }

  _StackStack->Peek()->Push( return_value );


  for( index = 0; index < num_params; index++ )
  {
    if( func->ParamPassByReference( index ) )
    {
      func->SetCurrParamIndex( index );
      refp = func->GetCurrParamRef();

      if( refp != 0 && param[ index ] != 0 )
      {
	define_result = _VarDefine( refp->Value(), param[ index ] );
	if( !define_result )
	  result = false;
	delete refp;
      }
      else // ( !( refp != 0 && param[ index ] != 0 ) )
      {
	if( ( refp == 0 ) && ( param[ index ] != 0 ) )
	{
	  listindex = 0;
	  shadowof = func->GetCurrParamShadowOf();
	  if( shadowof != 0 )
	  {
	    listindex = shadowof->ParentList()->Value().Find( shadowof );
	    if( listindex > 0 )
	    {
	      por_result = shadowof->ParentList()->
		SetSubscript( listindex, param[index]->Copy() );

	      if( por_result != 0 )
	      {
		por_result->Output( _StdErr );
		delete por_result;
	      }
	    }
#ifndef NDEBUG
	    else
	    {
	      _ErrorMessage( _StdErr, 30 );
	    }
#endif // NDEBUG
	  }
	  delete param[ index ];
	}
#ifndef NDEBUG
	else if( ( refp != 0 ) && ( param[ index ] == 0 ) )
	{
	  _ErrorMessage( _StdErr, 31, index, 0, func->FuncName(), "", refp );
	}
#endif // NDEBUG
      }
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
      p = _StackStack->Peek()->Pop();
      if( p->Type() == porBOOL )
      {
	if( ( (bool_Portion*) p )->Value() )
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
	_StackStack->Peek()->Push( p );
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
      _ErrorMessage( _StdErr, 33, program_counter, 0, "", "", 0, instruction );
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
  int i;

  _RefTableStack->Push( new RefHashTable );
  _StackStack->Push( new gGrowableStack< Portion* > );


  for( i = 0; i < func_info.NumParams; i++ )
  {
    if( param[ i ] != 0 )
    {
      _VarDefine( func_info.ParamInfo[ i ].Name, param[ i ]->Copy() );
    }
  }


  rc_result = Execute( program, true );

  switch( rc_result )
  {
  case rcSUCCESS:
    switch( _StackStack->Peek()->Depth() )
    {
    case 1:
      result = _StackStack->Peek()->Pop();
      result = _ResolveRef( result );
      if( result->Type() == porERROR )
      {
	delete result;
	result = 0;
      }
      break;
    case 0:
      result = new Error_Portion( (gString)
				 "GSM Error 43 :\n" +
				 "  User-defined function Error:\n" +
				 "    No return value\n" );
      break;
    default:
      result = new Error_Portion( (gString)
				 "GSM Error 44 :\n" + 
				 "  User-defined function Error:\n" +
				 "    Too many values left on stack;\n" +
				 "    return value ambiguous\n" );
    }
    break;
  case rcFAIL:
    result = 0;
    break;
  case rcQUIT:
    result = new Error_Portion( (gString)
			       "GSM Error 45 :\n" + 
			       "  User-defined function Error:\n" +
			       "    Interruption by user\n" );
    break;
  }


  for( i = 0; i < func_info.NumParams; i++ )
  {
    if( param[ i ] != 0 && func_info.ParamInfo[ i ].PassByReference )
    {
      delete param[ i ];
      param[ i ] = _VarValue( func_info.ParamInfo[ i ].Name )->Copy();
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

  assert( _StackStack->Peek()->Depth() >= 0 );

  if( _StackStack->Peek()->Depth() == 0 )
  {
    _StdOut << "Stack : NULL\n";
  }
  else
  {
    p = _StackStack->Peek()->Pop();
    p = _ResolveRef( p );

    p->Output( _StdOut );
    _StdOut << "\n";
    delete p;
  }
}


void GSM::Dump( void )
{
  int  i;

  assert( _StackStack->Peek()->Depth() >= 0 );

  if( _StackStack->Peek()->Depth() == 0 )
  {
    _StdOut << "Stack : NULL\n";
  }
  else
  {
    for( i = _StackStack->Peek()->Depth() - 1; i >= 0; i-- )
    {
      _StdOut << "Stack element " << i << " : ";
      Output();
    }
  }
  _StdOut << "\n";
  
  assert( _StackStack->Peek()->Depth() == 0 );
}


bool GSM::Pop( void )
{
  Portion* p;
  bool result = false;

  if( _StackStack->Peek()->Depth() > 0 )
  {
    p = _StackStack->Peek()->Pop();
    delete p;
    result = true;
  }
  else
  {
    _ErrorMessage( _StdErr, 34 );
  }
  return result;
}


void GSM::Flush( void )
{
  int       i;
  Portion*  p;
  bool result;

  assert( _StackStack->Peek()->Depth() >= 0 );
  for( i = _StackStack->Peek()->Depth() - 1; i >= 0; i-- )
  {
    result = Pop();
    assert( result == true );
  }

  assert( _StackStack->Peek()->Depth() == 0 );
}



//-----------------------------------------------------------------------
//                         _ErrorMessage
//-----------------------------------------------------------------------

void GSM::_ErrorMessage
(
 gOutput&        s,
 const int       error_num,
 const gInteger& num1, 
 const gInteger& num2,
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
    PrintPortionTypeSpec( s, (PortionType) num1.as_long() );
    s << "\n";
    s << "  Type of Operand 2: ";
    PrintPortionTypeSpec( s, (PortionType) num2.as_long() );
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
    s << "  types into a List_Portion.\n";
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


