//#
//# FILE: gsm.cc  implementation of GSM (Stack machine)
//#
//# $Id$
//#


#include <assert.h>
#include "gambitio.h"


#include "gsm.h"


//--------------------------------------------------------------------
//              implementation of GSM (Stack machine)
//--------------------------------------------------------------------

GSM::GSM( int size, gOutput& s_out, gOutput& s_err )
:_StdOut( s_out ), _StdErr( s_err )
{
  
#ifndef NDEBUG
  if( size <= 0 )
    _ErrorMessage( 1, size );
#endif // NDEBUG
  
  _Stack         = new gGrowableStack< Portion* >( size );
  _CallFuncStack = new gGrowableStack< CallFuncObj* >( size ) ;
  _RefTable      = new RefHashTable;
  _FuncTable     = new FunctionHashTable;
  
  InitFunctions();  // This function is located in gsmfunc.cc
}


GSM::~GSM()
{
  Flush();
  delete _FuncTable;
  delete _RefTable;
  delete _CallFuncStack;
  delete _Stack;
}


gOutput& GSM::StdErr( void ) const
{
  return _StdErr;
}


int GSM::Depth( void ) const
{
  return _Stack->Depth();
}


int GSM::MaxDepth( void ) const
{
  return _Stack->MaxDepth();
}


void GSM::_StackPush( Portion* p )
{
  p->SetGSM( this );
  _Stack->Push( p );
}


//------------------------------------------------------------------------
//                           Push() functions
//------------------------------------------------------------------------

bool GSM::Push( const bool& data )
{
  _StackPush( new bool_Portion( data ) );
  return true;
}


bool GSM::Push( const double& data )
{
  _StackPush( new numerical_Portion<double>( data ) );
  return true;
}


bool GSM::Push( const gInteger& data )
{
  _StackPush( new numerical_Portion<gInteger>( data ) );
  return true;
}


bool GSM::Push( const gRational& data )
{
  _StackPush( new numerical_Portion<gRational>( data ) );
  return true;
}


bool GSM::Push( const gString& data )
{
  _StackPush( new gString_Portion( data ) );
  return true;
}


/* These are commented out because the don't seem to be necessary */
#if 0
bool GSM::Push( Outcome* data )
{
  _StackPush( Outcome_Portion( data ) );
  return true;
}


bool GSM::Push( Player* data )
{
  _StackPush( new Player_Portion( data ) );
  return true;
}


bool GSM::Push( Infoset* data )
{
  _StackPush( new Infoset_Portion( data ) );
  return true;
}


bool GSM::Push( Action* data )
{
  _StackPush( new Action_Portion( data ) );
  return true;
}


bool GSM::Push( Node* data )
{
  _StackPush( new Node_Portion( data ) );
  return true;
}
#endif


bool GSM::PushStream( const gString& data )
{
  _StackPush( new Stream_Portion( data ) );
  return true;
}


bool GSM::PushList( const int num_of_elements )
{ 
  int            i;
  Portion*       p;
  List_Portion*  list;
  bool           result = true;

#ifndef NDEBUG
  if( num_of_elements <= 0 )
    _ErrorMessage( 2, num_of_elements );

  if( num_of_elements > _Stack->Depth() )
    _ErrorMessage( 3, num_of_elements, _Stack->Depth() );
#endif // NDEBUG

  list = new List_Portion;
  list->SetGSM( this );
  for( i = 1; i <= num_of_elements; i++ )
  {
    p = _Stack->Pop();
    if( p->Type() == porREFERENCE )
      p = _ResolveRef( (Reference_Portion*) p );
    if( list->Insert( p, 1 ) == 0 )
    {
      result = false;
    }
  }
  _StackPush( list );

  return result;
}



//---------------------------------------------------------------------
//     Reference related functions: PushRef(), Assign(), UnAssign()
//---------------------------------------------------------------------


bool GSM::PushRef( const gString& ref, const gString& subref )
{
  _StackPush( new Reference_Portion( ref, subref ) );
  return true;
}


bool GSM::Assign( void )
{
  Portion*  p2_copy;
  Portion*  p2;
  Portion*  p1;
  Portion*  p;
  Portion*  primary_ref;
  gString   p1_subvalue;
  bool      result = true;

#ifndef NDEBUG
  if( _Stack->Depth() < 2 )
    _ErrorMessage( 4 );
#endif // NDEBUG

  p2 = _Stack->Pop();
  p1 = _Stack->Pop();

  if ( p1->Type() == porREFERENCE )
  {
    p1_subvalue = ( (Reference_Portion*) p1 )->SubValue();

    if( p1_subvalue == "" )
    {
      if( p2->Type() == porREFERENCE )
      {
	p2 = _ResolveRef( (Reference_Portion*) p2 );
	p2_copy = p2->Copy();
	p2_copy->MakeCopyOfData( p2 );
      }
      else
      {
	p2_copy = p2->Copy();
      }
      p2_copy->Temporary() = false;
      p2->Temporary() = true;
      p2_copy->SetGSM( this );
      _RefTable->Define( ( (Reference_Portion*) p1 )->Value(), p2_copy );
      delete p2;
      
      p1 = _ResolveRef( (Reference_Portion*) p1 );
      _StackPush( p1 );
    }

    else // ( p1_subvalue != "" )
    {
      primary_ref = _ResolvePrimaryRefOnly( (Reference_Portion*) p1 );

      if( primary_ref->Type() & porALLOWS_SUBVARIABLES )
      {
	if( p2->Type() == porREFERENCE )
	{
	  p2 = _ResolveRef( (Reference_Portion*) p2 );
	  p2_copy = p2->Copy();
	  p2_copy->MakeCopyOfData( p2 );
	}
	else
	{
	  p2_copy = p2->Copy();
	}
	p2_copy->Temporary() = false;
	p2->Temporary() = true;

	switch( primary_ref->Type() )
	{
	case porNFG_DOUBLE:
	  ( (Nfg_Portion<double>*) primary_ref )->
	    Assign( p1_subvalue, p2_copy );
	  break;
	case porNFG_RATIONAL:
	  ( (Nfg_Portion<gRational>*) primary_ref )->
	    Assign( p1_subvalue, p2_copy );
	  break;
	  
	default:
	  _ErrorMessage( 5 );
	}

	delete p2;
	p1 = _ResolveRef( (Reference_Portion*) p1 );
      }
      else // ( !( primary_ref->Type() & porALLOWS_SUBVARIABLES ) )
      {
	_ErrorMessage( 6 );
	if( primary_ref->Type() == porERROR )
	{
	  delete primary_ref;
	}
	delete p2;
	delete p1;
	p1 = new Error_Portion;
      }
      _StackPush( p1 );
    }
  }
  else // ( p1->Type() != porREFERENCE )
  {
    int index = 0;
    if( p1->ShadowOf() != 0 )
    {
      p1->ShadowOf()->ParentList()->SetGSM( this );
      index = p1->ShadowOf()->ParentList()->Value().Find( p1->ShadowOf() );
    }
    if( index > 0 )
    {
      p1->ShadowOf()->ParentList()->SetSubscript( index, p2 );
      delete p1;
      _StackPush( p2->Copy() );
      result = true;
    }
    else
    {
      _ErrorMessage( 7 );
      delete p1;
      delete p2;
      _StackPush( new Error_Portion );
      result = false;
    }
  }
  return result;
}



bool GSM::UnAssign( void )
{
  Portion*  p1;
  Portion*  primary_ref;
  gString   ref;
  gString   p1_subvalue;
  bool      result = true;

#ifndef NDEBUG
  if( _Stack->Depth() < 1 )
    _ErrorMessage( 8 );
#endif // NDEBUG

  p1 = _Stack->Pop();

  if ( p1->Type() == porREFERENCE )
  {
    ref = ( (Reference_Portion*) p1 )->Value();
    p1_subvalue = ( (Reference_Portion*) p1 )->SubValue();
    if( p1_subvalue == "" )
    {
      if( _RefTable->IsDefined( ref ) )
      {
	_RefTable->Remove( ref );
      }
    }

    else // ( p1_subvalue != "" )
    {
      primary_ref = _ResolvePrimaryRefOnly( (Reference_Portion*) p1 );

      if( primary_ref->Type() & porALLOWS_SUBVARIABLES )
      {
	switch( primary_ref->Type() )
	{
	case porNFG_DOUBLE:
	  ( (Nfg_Portion<double>*) primary_ref )->UnAssign( p1_subvalue );
	  break;
	case porNFG_RATIONAL:
	  ( (Nfg_Portion<gRational>*) primary_ref )->UnAssign( p1_subvalue );
	  break;
	  
	default:
	  _ErrorMessage( 9 );
	}
      }
      else
      {
	_ErrorMessage( 10 );
	if( primary_ref->Type() == porERROR )
	{
	  delete primary_ref;
	}
	delete p1;
	p1 = new Error_Portion;
      }
    }
    delete p1;
  }
  else // ( p1->Type() != porREFERENCE )
  {
    _ErrorMessage( 11 );
    result = false;
  }
  return result;
}




//---------------------------------------------------------------------
//                        _ResolveRef functions
//-----------------------------------------------------------------------

Portion* GSM::_ResolveRef( Reference_Portion* p )
{
  Portion*  result = 0;
  gString&  ref = p->Value();
  gString&  subvalue = p->SubValue();


  if( _RefTable->IsDefined( ref ) )
  {
    if( subvalue == "" )
    {
      result = (*_RefTable)( ref )->Copy();
    }
    else
    {
      result = (*_RefTable)( ref );
      switch( result->Type() )
      {
      case porNFG_DOUBLE:
	result = ((Nfg_Portion<double>*) result )->operator()( subvalue );
	break;
      case porNFG_RATIONAL:
	result = ((Nfg_Portion<gRational>*) result )->operator()( subvalue );
	break;

      default:
	_ErrorMessage( 12 );
	result = new Error_Portion;
      }
      if( result != 0 )
      {
	if( result->Type() != porERROR )
	  result = result->Copy();
      }
      else
      {
	result = new Error_Portion;
      }
    }
  }
  else
  {
    _ErrorMessage( 13, 0, 0, ref );
    result = new Error_Portion;
  }
  delete p;

  if( result != 0 )
    result->SetGSM( this );
  return result;
}


Portion* GSM::_ResolveRefWithoutError( Reference_Portion* p )
{
  Portion*  result = 0;
  gString&  ref = p->Value();
  gString&  subvalue = p->SubValue();

  if( _RefTable->IsDefined( ref ) )
  {
    if( subvalue == "" )
    {
      result = (*_RefTable)( ref )->Copy();
    }
    else
    {
      result = (*_RefTable)( ref );
      switch( result->Type() )
      {
      case porNFG_DOUBLE:
	if( ((Nfg_Portion<double>*) result )->IsDefined( subvalue ) )
	{
	  result = ((Nfg_Portion<double>*) result )->
	    operator()( subvalue )->Copy();
	}
	else
	{
	  result = 0;
	}
	break;
      case porNFG_RATIONAL:
	if( ((Nfg_Portion<gRational>*) result )->IsDefined( subvalue ) )
	{
	  result = ((Nfg_Portion<gRational>*) result )
	    ->operator()( subvalue )->Copy();
	}
	else
	{
	  result = 0;
	}
	break;

      default:
	_ErrorMessage( 14 );
	result = new Error_Portion;
      }
    }
  }
  else
  {
    result = 0;
  }
  delete p;

  if( result != 0 )
    result->SetGSM( this );
  return result;
}



Portion* GSM::_ResolvePrimaryRefOnly( Reference_Portion* p )
{
  Portion*  result = 0;
  gString&  ref = p->Value();

  if( _RefTable->IsDefined( ref ) )
  {
    result = (*_RefTable)( ref );
  }
  else
  {
    _ErrorMessage( 15, 0, 0, ref );
    result = new Error_Portion;
  }

  if( result != 0 )
    result->SetGSM( this );
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
  bool       result = true;

#ifndef NDEBUG
  if( _Stack->Depth() < 2 )
    _ErrorMessage( 16 );
#endif // NDEBUG
  
  p2 = _Stack->Pop();
  p1 = _Stack->Pop();
  
  if( p2->Type() == porREFERENCE )
    p2 = _ResolveRef( (Reference_Portion*) p2 );
  
  if( p1->Type() == porREFERENCE )
    p1 = _ResolveRef( (Reference_Portion*) p1 );


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
    if(
       mode == opEQUAL_TO ||
       mode == opNOT_EQUAL_TO ||
       mode == opGREATER_THAN ||
       mode == opLESS_THAN ||
       mode == opGREATER_THAN_OR_EQUAL_TO ||
       mode == opLESS_THAN_OR_EQUAL_TO 
       )
    {
      delete p1;
      p1 = new bool_Portion( result );
      result = true;
    }
  }

  else // ( p1->Type() != p2->Type() )
  {
    _ErrorMessage( 17, (int) p1->Type(), (int) p2->Type() );
    delete p1;
    delete p2;
    p1 = new Error_Portion;
    result = false;
  }

  _StackPush( p1 );

  return result;
}




//-----------------------------------------------------------------------
//                        unary operations
//-----------------------------------------------------------------------

bool GSM::_UnaryOperation( OperationMode mode )
{
  Portion*  p1;
  bool      result = true;

  if( _Stack->Depth() >= 1 )
  {
    p1 = _Stack->Pop();
    
    if( p1->Type() == porREFERENCE )
      p1 = _ResolveRef( (Reference_Portion*) p1 );

    p1->Operation( 0, mode );
    _StackPush( p1 );
  }
  else
  {
    _ErrorMessage( 18 );
    result = false;
  }

  return result;
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
  bool     result = true;

  assert( _Stack->Depth() >= 2 );
  p2 = _Stack->Pop();
  p1 = _Stack->Pop();

  if( p1->Type() == porREFERENCE )
  {
    refp = p1;
    if( _RefTable->IsDefined( ( (Reference_Portion*) refp )->Value() ) )
      p1 = (*_RefTable)( ( (Reference_Portion*) refp )->Value() );
    else
      p1 = 0;
    
    if( p1 != 0 && p1->Type() == porLIST )
    {
      delete refp;
    }
    else
    {
      p1 = refp;
    }
  }

  if( p2->Type() == porREFERENCE )
    p2 = _ResolveRef( (Reference_Portion*) p2 );

  if( p1->Type() == porLIST )
  {
    if( p2->Type() == porINTEGER )
    {
      if( p1->ShadowOf() == 0 )
      {
	real_list = p1;
      }
      else
      {
	real_list = p1->ShadowOf();
	delete p1;
      }
      element = ( (List_Portion* ) real_list )->
	GetSubscript( ((numerical_Portion<gInteger>*)p2 )->Value().as_long() );
      if( element != 0 )
      {
	shadow = element->Copy();
	shadow->ShadowOf() = element;
	_StackPush( shadow );
      }
      else
      {
	_StackPush( new Error_Portion );
      }
    }
    else
    {
      _ErrorMessage( 19 );
      _StackPush( new Error_Portion );
      result = false;
    }
  }
  else
  {
    _ErrorMessage( 20 );
    delete p1;
    _StackPush( new Error_Portion );
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
    _ErrorMessage( 21 );

  if( _Stack->Depth() <= 0 )
    _ErrorMessage( 22 );
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
    _ErrorMessage( 23, 0, 0, param_name, func->FuncName() );
    result = false;
  }
  else // ( new_index == PARAM_AMBIGUOUS )
  {
    _ErrorMessage( 24, 0, 0, param_name, func->FuncName() );
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
    func = new CallFuncObj( (*_FuncTable)( funcname ), _StdErr );
    _CallFuncStack->Push( func );
  }
  else // ( !_FuncTable->IsDefined( funcname ) )
  {
    _ErrorMessage( 25, 0, 0, funcname );
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
  CallFuncObj*        func;
  PortionType         curr_param_type;
  Portion*            param = 0;
  gString             funcname;
  int                 i;
  int                 type_match;
  gString             ref;
  Reference_Portion*  refp;
  bool                result = true;

#ifndef NDEBUG
  _BindCheck();
#endif // NDEBUG

  func = _CallFuncStack->Pop();
  param = _Stack->Pop();
  
  if( param->Type() == porREFERENCE )
    param = _ResolveRef( (Reference_Portion *)param );

  param->ShadowOf() = 0;
  result = func->SetCurrParam( param ); 

  if( !result )  // == false
  {
    func->SetErrorOccurred();
  }

  _CallFuncStack->Push( func );
  return result;
}


bool GSM::BindRef( void )
{
  CallFuncObj*  func;
  PortionType   curr_param_type;
  Portion*      param;
  Portion*      subparam;
  gString       funcname;
  int           i;
  int           type_match;
  bool          result = true;
  gString       ref;
  gString       subref;

#ifndef NDEBUG
  _BindCheck();
#endif // NDEBUG

  func = _CallFuncStack->Pop();
  param = _Stack->Pop();
  
  if( param->Type() == porREFERENCE )
  {
    ref = ( (Reference_Portion*) param )->Value();
    subref = ( (Reference_Portion*) param )->SubValue();
    func->SetCurrParamRef( (Reference_Portion*)( param->Copy() ) );
    param = _ResolveRefWithoutError( (Reference_Portion*) param );
    if( param != 0 )
    {
      if( param->Type() == porERROR )
      {
	delete param;
	result = false;
      }
    }
  }
  else // ( param->Type() != porREFERENCE )
  {
    if( param->ShadowOf() == 0 )
    {
      _CallFuncStack->Push( func );
      _StackPush( param );
      result = BindVal();
      return result;
    }
  }

  
  if( result )  // == true
  {
    result = func->SetCurrParam( param );
  }

  if( !result )  // == false
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
  bool                result = true;

#ifndef NDEBUG
  if( _CallFuncStack->Depth() <= 0 )
    _ErrorMessage( 26 );
#endif // NDEBUG

  func = _CallFuncStack->Pop();

  num_params = func->NumParams();
  param = new Portion*[ num_params ];

  return_value = func->CallFunction( param );

  if( return_value == 0 )
  {
    _ErrorMessage( 27, 0, 0, func->FuncName() );
    return_value = new Error_Portion;
    result = false;
  }


  _StackPush( return_value );


  for( index = 0; index < num_params; index++ )
  {
    if( func->ParamPassByReference( index ) )
    {
      func->SetCurrParamIndex( index );
      refp = func->GetCurrParamRef();

      if( refp != 0 && param[ index ] != 0 )
      {
	if( refp->SubValue() == "" )
	{
	  param[ index ]->SetGSM( this );
	  _RefTable->Define( refp->Value(), param[ index ] );
	}
	else // ( refp->SubValue != "" )
	{
	  if( _RefTable->IsDefined( refp->Value() ) )
	  {
	    p = ( *_RefTable )( refp->Value() );
	    switch( p->Type() )
	    {
	    case porNFG_DOUBLE:
	      ( (Nfg_Portion<double>*) p )->
		Assign( refp->SubValue(), param[ index ]->Copy() );
	      break;
	    case porNFG_RATIONAL:
	      ( (Nfg_Portion<gRational>*) p )->
		Assign( refp->SubValue(), param[ index ]->Copy() );
	      break;

	    default:
	      _ErrorMessage( 28 );
	      result = false;
	    }
	    delete param[ index ];
	  }
	  else // ( !_RefTable->IsDefined( refp->Value() ) )
	  {
	    _ErrorMessage( 29 );
	    delete param[ index ];
	    result = false;
	  }
	}
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
	    shadowof->ParentList()->SetGSM( this );
	    listindex = shadowof->ParentList()->Value().Find( shadowof );
	    if( listindex > 0 )
	    {
	      shadowof->ParentList()->SetSubscript( listindex, 
						   param[index]->Copy() );
	    }
#ifndef NDEBUG
	    else
	    {
	      _ErrorMessage( 30 );
	    }
#endif // NDEBUG
	  }
	  delete param[ index ];
	}
#ifndef NDEBUG
	else if( ( refp != 0 ) && ( param[ index ] == 0 ) )
	{
	  _ErrorMessage( 31, index, 0, func->FuncName(), "", refp );
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

GSM_ReturnCode GSM::Execute( gList< Instruction* >& program )
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
      p = _Stack->Pop();
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
	_ErrorMessage( 32 );
	_StackPush( p );
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
      _ErrorMessage( 33, program_counter, 0, "", "", 0, instruction );
      result = rcFAIL;
      break;
    }
  }

  while( program.Length() > 0 )
  {
    instruction = program.Remove( 1 );
    delete instruction;
  }

  return result;
}


//----------------------------------------------------------------------------
//                   miscellaneous functions
//----------------------------------------------------------------------------


bool GSM::Pop( void )
{
  Portion* p;
  bool result = false;

  if( _Stack->Depth() > 0 )
  {
    p = _Stack->Pop();
    delete p;
    result = true;
  }
  else
  {
    _ErrorMessage( 34 );
  }
  return result;
}


void GSM::Output( void )
{
  Portion*  p;

  assert( _Stack->Depth() >= 0 );

  if( _Stack->Depth() == 0 )
  {
    _StdOut << "Stack : NULL\n";
  }
  else
  {
    p = _Stack->Pop();
    if( p->Type() == porREFERENCE )
      p = _ResolveRef( (Reference_Portion*) p );
    p->Output( _StdOut );
    _StdOut << "\n";
    delete p;
  }
}


void GSM::Dump( void )
{
  int  i;

  assert( _Stack->Depth() >= 0 );

  if( _Stack->Depth() == 0 )
  {
    _StdOut << "Stack : NULL\n";
  }
  else
  {
    for( i = _Stack->Depth() - 1; i >= 0; i-- )
    {
      _StdOut << "Stack element " << i << " : ";
      Output();
    }
  }
  _StdOut << "\n";
  
  assert( _Stack->Depth() == 0 );
}


void GSM::Flush( void )
{
  int       i;
  Portion*  p;

  for( i = _Stack->Depth() - 1; i >= 0; i-- )
  {
    p = _Stack->Pop();
    delete p;
  }

  assert( _Stack->Depth() == 0 );
}



//-----------------------------------------------------------------------
//                         _ErrorMessage
//-----------------------------------------------------------------------

void GSM::_ErrorMessage
(
 const int       error_num,
 const gInteger& num1, 
 const gInteger& num2,
 const gString&  str1,
 const gString&  str2,
 Portion*        por,
 Instruction*    instr
 ) const
{
  _StdErr << "GSM Error " << error_num << " :\n";

  switch( error_num )
  {
  case 1:
    _StdErr << "  Illegal stack size specified during initialization\n";
    _StdErr << "  Stack size requested: " << num1 << "\n";
    assert( 0 );
    break;
  case 2:
    _StdErr << "  Illegal number of elements requested to PushList()\n";
    _StdErr << "  Elements requested: " << num1 << "\n";
    assert( 0 );
    break;
  case 3:
    _StdErr << "  Not enough elements in GSM to PushList()\n";
    _StdErr << "  Elements requested: " << num1 << "\n";
    _StdErr << "  Elements available: " << num2 << "\n";
    assert( 0 );
    break;
  case 4:
    _StdErr << "  Not enough operands to execute Assign()\n";
    assert( 0 );
    break;
  case 5:
    _StdErr << "  Unknown type supports subvariables\n";
    assert(0);
    break;
  case 6:
    _StdErr << "  Attempted to assign a sub-reference to a type\n";
    _StdErr << "  that doesn't support such structures\n";
    break;
  case 7:
    _StdErr << "  No reference found to be assigned\n";
    break;
  case 8:
    _StdErr << "  Not enough operands to execute UnAssign()\n";
    assert( 0 );
    break;
  case 9:
    _StdErr << "  Unknown type supports subvariables\n";
    assert(0);
    break;
  case 10:
    _StdErr << "  Attempted to unassign a sub-reference of a type\n";
    _StdErr << "  that doesn't support such structures\n";
    break;
  case 11:
    _StdErr << "  No reference found to be unassigned\n";
    break;
  case 12:
    _StdErr << "  Attempted to resolve a subvariable of a type\n";
    _StdErr << "  that does not support subvariables\n";
    break;
  case 13:
    _StdErr << "  Attempted to resolve an undefined reference\n";
    _StdErr << "  \"" << str1 << "\"\n";
    break;
  case 14:
    _StdErr << "  Attempted to resolve the subvariable of a type\n";
    _StdErr << "  that does not support subvariables\n";
    break;
  case 15:
    _StdErr << "  Attempted to resolve an undefined reference\n";
    _StdErr << "  \"" << str1 << "\"\n";
    break;
  case 16:
    _StdErr << "  Not enough operands to perform binary operation\n";
    assert( 0 );
    break;
  case 17:
    _StdErr << "  Attempted operating on different types\n";
    _StdErr << "  Type of Operand 1: " << num1 << "\n";
    _StdErr << "  Type of Operand 2: " << num2 << "\n";
    break;
  case 18:
    _StdErr << "  Not enough operands to perform unary operation\n";
    break;
  case 19:
    _StdErr << "  A non-integer element number passed as the\n";
    _StdErr << "  subscript of a List\n";
    break;
  case 20:
    _StdErr << "  Attempted to take the subscript of a non-List\n";
    _StdErr << "  Portion type\n";
    break;
  case 21:
    _StdErr << "  The CallFunction() subsystem was not initialized by\n";
    _StdErr << "  calling InitCallFunction() first\n";
    assert( 0 );
    break;
  case 22:
    _StdErr << "  No value found to assign to a function parameter\n";
    assert( 0 );
    break;
  case 23:
    _StdErr << "  Parameter \"" << str1 << "\" is not defined for\n";
    _StdErr << "  the function \"" << str2 << "\"\n";
    break;
  case 24:
    _StdErr << "  Parameter \"" << str1 << "\" is ambiguous in\n";
    _StdErr << "  the function \"" << str2 << "\"\n";
    break;
  case 25:
    _StdErr << "  Undefined function name:\n";
    _StdErr << "  InitCallFunction( \"" << str1 << "\" )\n";
    break;
  case 26:
    _StdErr << "  The CallFunction() subsystem was not initialized by\n";
    _StdErr << "  calling InitCallFunction() first\n";
    assert( 0 );
    break;
  case 27:
    _StdErr << "  An error occurred while attempting to execute\n";
    _StdErr << "  CallFunction( \"" << str1 << "\", ... )\n";
    break;
  case 28:
    _StdErr << "  Attempted to assign the subvariable of a\n";
    _StdErr << "  type that does not support subvariables\n";
    break;
  case 29:
    _StdErr << "  Attempted to assign the sub-variable of\n";
    _StdErr << "  an undefined variable\n";
    break;
  case 30:
    _StdErr << "  Fatal Error:\n";
    _StdErr << "    Returning function parameter information\n";
    _StdErr << "    (regarding lists) is invalid\n";
    assert(0);
    break;
  case 31:
    _StdErr << "  Fatal Error:\n";
    _StdErr << "    Attempting to assign a null Portion to a reference\n";
    _StdErr << "    Function: \"" << str1 << "\"\n";
    _StdErr << "    Parameter index: " << num1 << "\n";
    _StdErr << "    Reference: " << por << "\n";
    assert(0);
    break;
  case 32:
    _StdErr << "  Instruction IfGoto called on a non-boolean data type\n";
    break;
  case 33:
    _StdErr << "  Instruction #" << num1 << ": " << instr << "\n";
    _StdErr << "           was not executed successfully\n";
    _StdErr << "           Program abnormally terminated.\n";
    break;
  case 34:
    _StdErr << "GSM Error: Pop() called on an empty stack\n";
    break;
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



#include "glist.imp"

TEMPLATE class gList< Portion* >;
TEMPLATE class gNode< Portion* >;

/* already declared in readefg.y
   TEMPLATE class gList< gString >;
   TEMPLATE class gNode< gString >;
   */

TEMPLATE class gList< FuncDescObj* >;
TEMPLATE class gNode< FuncDescObj* >;


#include "gstack.imp"

TEMPLATE class gStack< Portion* >;
TEMPLATE class gStack< CallFuncObj* >;
TEMPLATE class gStack< List_Portion* >;


#include "ggrstack.imp"

TEMPLATE class gGrowableStack< Portion* >;
TEMPLATE class gGrowableStack< CallFuncObj* >;
TEMPLATE class gGrowableStack< List_Portion* >;


gOutput& operator << ( class gOutput& s, class Portion* (*funcname)() )
{ return s << funcname; }




