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

GSM::GSM( int size )
{
#ifndef NDEBUG
  if( size <= 0 )
  {
    gerr << "GSM Error: illegal stack size specified during initialization\n";
    gerr << "           stack size requested: " << size << "\n";
  }
  assert( size > 0 );
#endif // NDEBUG

  _Stack         = new gStack< Portion* >( size );
  _CallFuncStack = new gStack< CallFuncObj* >( size ) ;
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


int GSM::Depth( void ) const
{
  return _Stack->Depth();
}


int GSM::MaxDepth( void ) const
{
  return _Stack->MaxDepth();
}





//------------------------------------------------------------------------
//                           Push() functions
//------------------------------------------------------------------------

bool GSM::Push( const bool& data )
{
  Portion*  p;
  bool      result = true;
  
  if( _Stack->Depth() < _Stack->MaxDepth() )
  {
    p = new bool_Portion( data );
    _Stack->Push( p );
  }
  else
  {
    gerr << "GSM Error: out of stack space\n";
    result = false;
  }
  return result;
}


bool GSM::Push( const double& data )
{
  Portion*  p;
  bool      result = true;
  
  if( _Stack->Depth() < _Stack->MaxDepth() )
  {
    p = new numerical_Portion<double>( data );
    _Stack->Push( p );
  }
  else
  {
    gerr << "GSM Error: out of stack space\n";
    result = false;
  }
  return result;
}


bool GSM::Push( const gInteger& data )
{
  Portion*  p;
  bool      result = true;

  if( _Stack->Depth() < _Stack->MaxDepth() )
  {
    p = new numerical_Portion<gInteger>( data );
    _Stack->Push( p );
  }
  else
  {
    gerr << "GSM Error: out of stack space\n";
    result = false;
  }
  return result;
}


bool GSM::Push( const gRational& data )
{
  Portion*  p;
  bool      result = true;
  
  if( _Stack->Depth() < _Stack->MaxDepth() )
  {
    p = new numerical_Portion<gRational>( data );
    _Stack->Push( p );
  }
  else
  {
    gerr << "GSM Error: out of stack space\n";
    result = false;
  }
  return result;
}


bool GSM::Push( const gString& data )
{
  Portion*  p;
  bool      result = true;
  
  if( _Stack->Depth() < _Stack->MaxDepth() )
  {
    p = new gString_Portion( data );
    _Stack->Push( p );
  }
  else
  {
    gerr << "GSM Error: out of stack space\n";
    result = false;
  }
  return result;
}





// This function is only temporarily here for testing reasons
bool GSM::GenerateNfg( const double& data )
{
  Nfg_Portion* p;
  p = new Nfg_Portion( *(new Nfg) );
  p->Value().value = data;
  p->Temporary() = false;
  _Stack->Push( p );
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
  {
    gerr << "GSM Error: illegal number of elements requested to PushList()\n";
    gerr << "           elements requested: " << num_of_elements << "\n";
  }
  assert( num_of_elements > 0 );

  if( num_of_elements > _Stack->Depth() )
  {
    gerr << "GSM Error: not enough elements in GSM to PushList()\n";
    gerr << "           elements requested: " << num_of_elements << "\n";
    gerr << "           elements available: " << _Stack->Depth() << "\n";
  }
  assert( num_of_elements <= _Stack->Depth() );
#endif // NDEBUG

  list = new List_Portion;
  for( i = 1; i <= num_of_elements; i++ )
  {
    p = _Stack->Pop();
    if( p->Type() == porREFERENCE )
      p = _ResolveRef( (Reference_Portion*) p );
    list->Insert( p, 1 );
  }
  _Stack->Push( list );

  return result;
}



//---------------------------------------------------------------------
//     Reference related functions: PushRef(), Assign(), UnAssign()
//---------------------------------------------------------------------

bool GSM::PushRef( const gString& ref )
{
  Portion*  p;
  bool      result = true;
  
  if( _Stack->Depth() < _Stack->MaxDepth() )
  {
    p = new Reference_Portion( ref );
    _Stack->Push( p );
  }
  else
  {
    gerr << "GSM Error: out of stack space\n";
    result = false;
  }
  return result;
}


bool GSM::PushRef( const gString& ref, const gString& subref )
{
  Portion*  p;
  bool      result = true;
  
  if( _Stack->Depth() < _Stack->MaxDepth() )
  {
    p = new Reference_Portion( ref, subref );
    _Stack->Push( p );
  }
  else
  {
    gerr << "GSM Error: out of stack space\n";
    result = false;
  }
  return result;
}


bool GSM::Assign( void )
{
  Portion*  p2_copy;
  Portion*  p2;
  Portion*  p1;
  Portion*  primary_ref;
  gString   p1_subvalue;
  bool      result = true;

#ifndef NDEBUG
  if( _Stack->Depth() < 2 )
  {
    gerr << "GSM Error: not enough operands to execute Assign()\n";
  }
  assert( _Stack->Depth() >= 2 );
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
	p2_copy->CopyDataFrom( p2 );
      }
      else
      {
	p2_copy = p2->Copy();
      }
      p2_copy->Temporary() = false;
      p2->Temporary() = true;
      _RefTable->Define( ( (Reference_Portion*) p1 )->Value(), p2_copy );
      delete p2;
      
      p1 = _ResolveRef( (Reference_Portion*) p1 );
      _Stack->Push( p1 );
    }

    else // ( p1_subvalue != "" )
    {
      primary_ref = _ResolvePrimaryRefOnly( (Reference_Portion*) p1 );

#ifndef NDEBUG
      if( primary_ref->Type() != porNFG )
      {
	gerr << "GSM Error: attempted to assign a sub-reference to a type\n";
	gerr << "           that doesn't support such structures\n";
      }
      assert( primary_ref->Type() == porNFG );
#endif // NDEBUG
      
      if( p2->Type() == porREFERENCE )
      {
	p2 = _ResolveRef( (Reference_Portion*) p2 );
	p2_copy = p2->Copy();
	p2_copy->CopyDataFrom( p2 );
      }
      else
      {
	p2_copy = p2->Copy();
      }
      p2_copy->Temporary() = false;
      p2->Temporary() = true;
      ( (Nfg_Portion*) primary_ref )->Assign( p1_subvalue, p2_copy );
      delete p2;

      p1 = _ResolveRef( (Reference_Portion*) p1 );
      _Stack->Push( p1 );
    }
  }
  else // ( p1->Type() != porREFERENCE )
  {
    gerr << "GSM Error: no reference found to be assigned\n";
    result = false;
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
  {
    gerr << "GSM Error: not enough operands to execute Assign()\n";
  }
  assert( _Stack->Depth() >= 1 );
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
#ifndef NDEBUG
      else
      {
	gerr << "GSM Warning: calling UnAssign() on a undefined reference\n";
      }
#endif // NDEBUG
    }

    else // ( p1_subvalue != "" )
    {
      primary_ref = _ResolvePrimaryRefOnly( (Reference_Portion*) p1 );

#ifndef NDEBUG
      if( primary_ref->Type() != porNFG )
      {
	gerr << "GSM Error: attempted to unassign a sub-reference of a type\n";
	gerr << "           that doesn't support such structures\n";
      }
      assert( primary_ref->Type() == porNFG );
#endif // NDEBUG

      ( (Nfg_Portion*) primary_ref )->UnAssign( p1_subvalue );
    }
    delete p1;
  }
  else // ( p1->Type() != porREFERENCE )
  {
    gerr << "GSM Error: no reference found to be unassigned\n";
    result = false;
  }
  return result;
}


bool GSM::UnAssign( const gString& ref )
{
  if( _RefTable->IsDefined( ref ) )
  {
    _RefTable->Remove( ref );
  }
#ifndef NDEBUG
  else
  {
    gerr << "GSM Warning: calling UnAssign() on a undefined reference\n";
  }
#endif // NDEBUG

  return true;
}




//---------------------------------------------------------------------
//                        _ResolveRef functions
//-----------------------------------------------------------------------

Portion* GSM::_ResolveRef( Reference_Portion* p )
{
  Portion*  result = 0;
  Portion*  temp;
  gString&  ref = p->Value();
  gString   subvalue;

#ifndef NDEBUG
  if( !_RefTable->IsDefined( ref ) )
  {
    gerr << "GSM Error: attempted to resolve an undefined reference\n";
    gerr << "           \"" << ref << "\"\n";
  }
  assert( _RefTable->IsDefined( ref ) );
#endif // NDEBUG

  subvalue = ( (Reference_Portion*) p )->SubValue();
  if( subvalue == "" )
  {
    result = (*_RefTable)( ref )->Copy();
  }
  else
  {
    result = (*_RefTable)( ref );
    result = ((Nfg_Portion*) result )->operator()( subvalue );
    assert( result != 0 );
    result = result->Copy();
  }
  delete p;

  return result;
}


Portion* GSM::_ResolvePrimaryRefOnly( Reference_Portion* p )
{
  Portion*  result = 0;
  Portion*  temp;
  gString&  ref = p->Value();
  gString   subvalue;

#ifndef NDEBUG
  if( !_RefTable->IsDefined( ref ) )
  {
    gerr << "GSM Error: attempted to resolve an undefined reference\n";
    gerr << "           \"" << ref << "\"\n";
  }
  assert( _RefTable->IsDefined( ref ) );
#endif // NDEBUG

  result = (*_RefTable)( ref );
  return result;
}



//------------------------------------------------------------------------
//                       binary operations
//------------------------------------------------------------------------

bool GSM::_BinaryOperation( OperationMode mode )
{
  Portion*   p2;
  Portion*   p1;
  bool       result = true;

#ifndef NDEBUG
  if( _Stack->Depth() < 2 )
  {
    gerr << "GSM Error: not enough operands to perform binary operation\n";
    result = false;
  }
  assert( _Stack->Depth() >= 2 );
#endif // NDEBUG
  
  p2 = _Stack->Pop();
  p1 = _Stack->Pop();
  
  if( p2->Type() == porREFERENCE )
    p2 = _ResolveRef( (Reference_Portion*) p2 );
  if( p1->Type() == porREFERENCE )
    p1 = _ResolveRef( (Reference_Portion*) p1 );

  if( p1->Type() == p2->Type() )
  {
    result = p1->Operation( p2, mode );
    if(mode == opEQUAL_TO ||
       mode == opNOT_EQUAL_TO ||
       mode == opGREATER_THAN ||
       mode == opLESS_THAN ||
       mode == opGREATER_THAN_OR_EQUAL_TO ||
       mode == opLESS_THAN_OR_EQUAL_TO )
    {
      delete p1;
      p1 = new bool_Portion( result );
      result = true;
    }
    _Stack->Push( p1 );
  }
  else // ( p1->Type() != p2->Type() )
  {
    gerr << "GSM Error: attempted operating on different types\n";
    gerr << "           Type of Operand 1: " << p1->Type() << "\n";
    gerr << "           Type of Operand 2: " << p2->Type() << "\n";
    result = false;
  }

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
    _Stack->Push( p1 );
  }
  else
  {
    gerr << "GSM Error: not enough operands to perform unary operation\n";
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




//-------------------------------------------------------------------
//               CallFunction() related functions
//-------------------------------------------------------------------

void GSM::AddFunction( FuncDescObj* func )
{
  _FuncTable->Define( func->FuncName(), func );
}


int FuncParamCheck( const PortionType stack_param_type, 
		   const PortionType func_param_type )
{
  int result = true;

  if( stack_param_type != func_param_type )
  {
    if( func_param_type != porNUMERICAL ||
       !( stack_param_type == porDOUBLE ||
	 stack_param_type == porINTEGER ||
	 stack_param_type == porRATIONAL ) )
    {
      result = false;
    }
  }
  return result;
}





bool GSM::InitCallFunction( const gString& funcname )
{
  CallFuncObj*  func;
  bool          result = true;

  assert( _CallFuncStack->Depth() < _CallFuncStack->MaxDepth() );

  if( _FuncTable->IsDefined( funcname ) )
  {
    func = new CallFuncObj( (*_FuncTable)( funcname ) );
    _CallFuncStack->Push( func );
  }
  else // ( !_FuncTable->IsDefined( funcname ) )
  {
    gerr << "GSM Error: undefined function name:\n";
    gerr << "           InitCallFunction( \"" << funcname << "\", ... )\n";
    result = false;
  }
  return result;
}


bool GSM::Bind( void )
{
  CallFuncObj*  func;
  PortionType          curr_param_type;
  Portion*             param;
  gString              funcname;
  int                  i;
  int                  type_match;
  bool                 result = true;

#ifndef NDEBUG
  if( _CallFuncStack->Depth() <= 0 )
  {
    gerr << "GSM Error: the CallFunction() subsystem was not initialized by\n";
    gerr << "           calling InitCallFunction() first\n";
  }
  assert( _CallFuncStack->Depth() > 0 );

  if( _Stack->Depth() <= 0 )
  {
    gerr << "GSM Error: no value found to assign to a function parameter\n";
  }
  assert( _Stack->Depth() > 0 );
#endif // NDEBUG

  func = _CallFuncStack->Pop();
  param = _Stack->Pop();
  
  if( param->Type() == porREFERENCE )
    param = _ResolveRef( (Reference_Portion *)param );
  
  curr_param_type = func->GetCurrParamType();
  type_match = FuncParamCheck( param->Type(), curr_param_type );
  if( type_match )
  {
    func->SetCurrParam( param ); 
  }
  else // ( !type_match )
  {
    if( curr_param_type != porERROR )
    {
      funcname = func->FuncName();
      i        = func->GetCurrParamIndex();
      gerr << "GSM Error: mismatched parameter type found while executing\n";
      gerr << "           CallFunction( \"" << funcname << "\", ... )\n\n";
      gerr << "Error at Parameter #: " << i << "\n";
      gerr << "       Expected type: " << func->GetCurrParamType() << "\n";
      gerr << "       Type found:    " << param->Type() << "\n";
      result = false;
    }
  }
  _CallFuncStack->Push( func );
  return result;
}


bool GSM::Bind( const gString& param_name )
{
  CallFuncObj*  func;
  int                  new_index;
  int                  result = true;
  
#ifndef NDEBUG
  if( _CallFuncStack->Depth() <= 0 )
  {
    gerr << "GSM Error: the CallFunction() subsystem was not initialized by\n";
    gerr << "           calling InitCallFunction() first\n";
  }
  assert( _CallFuncStack->Depth() > 0 );
#endif // NDEBUG

  func = _CallFuncStack->Pop();
  new_index = func->FindParamName( param_name );
  
  if( new_index >= 0 )
  {
    if( new_index >= func->GetCurrParamIndex() )
    {
      func->SetCurrParamIndex( new_index );
      _CallFuncStack->Push( func );
      result = Bind();
    }
    else // ( new_index < func->GetCurrParamIndex() )
    {
      gerr << "GSM Error: multiple definitions found for parameter ";
      gerr << "\"" << param_name << "\" while executing\n";
      gerr << "           CallFunction( \"" << func->FuncName() << "\", ... )\n";
      result = false;
    }
  }
  else // ( new_index == PARAM_NOT_FOUND )
  {
    gerr << "FuncDescObj Error: parameter \"" << param_name;
    gerr << "\" is not defined for\n";
    gerr << "                   the function \"" << func->FuncName() << "\"\n";
    result = false;
  }
  return result;
}


bool GSM::CallFunction( void )
{
  CallFuncObj*  func;
  Portion*             return_value;
  bool                 result = true;

#ifndef NDEBUG
  if( _CallFuncStack->Depth() <= 0 )
  {
    gerr << "GSM Error: the CallFunction() subsystem was not initialized by\n";
    gerr << "           calling InitCallFunction() first\n";
  }
  assert( _CallFuncStack->Depth() > 0 );
#endif // NDEBUG

  func = _CallFuncStack->Pop();
  return_value = func->CallFunction();
  delete func;

#ifndef NDEBUG
  if( return_value == 0 )
  {
    gerr << "GSM Error: an error occurred while attempting to execute\n";
    gerr << "           CallFunction( \"" << func->FuncName() << "\", ... )\n";
  }
  assert( return_value != 0 );
#endif // NDEBUG

  _Stack->Push( return_value );

  return result;
}


//----------------------------------------------------------------------------
//                       Execute function
//----------------------------------------------------------------------------

bool GSM::Execute( gList< Instruction* >& program )
{
  bool result;
  int count = 0;
  Instruction *instruction;

  while( program.Length() > 0 )
  {
    count++;
    instruction = program.Remove( 1 );
    result = instruction->Execute( *this );
    delete instruction;
    if( result == false )
    {
      gerr << "GSM Error: instruction #" << count << " was not executed\n";
      gerr << "           successfully\n";
      break;
    }
  }
  return result;
}


//----------------------------------------------------------------------------
//                   miscellaneous functions
//----------------------------------------------------------------------------

void GSM::Output( void )
{
  Portion*  p;

  p = _Stack->Pop();
  if( p->Type() == porREFERENCE )
  {
    p = _ResolveRef( (Reference_Portion*) p );
  }
  p->Output( gout );
  delete p;
}


void GSM::Dump( void )
{
  int  i;

  for( i = _Stack->Depth() - 1; i >= 0; i-- )
  {
    gout << "Stack element " << i << " : ";
    Output();
  }
  gout << "\n";

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

TEMPLATE class gList< gString >;
TEMPLATE class gNode< gString >;

TEMPLATE class gList< FuncDescObj* >;
TEMPLATE class gNode< FuncDescObj* >;


#include "gstack.imp"

TEMPLATE class gStack< Portion* >;
TEMPLATE class gStack< CallFuncObj* >;


gOutput& operator << ( class gOutput& s, class Portion* (*funcname)() )
{ return s << funcname; }


