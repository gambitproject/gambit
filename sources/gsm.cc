//#
//# FILE: gsm.cc  implementation of GSM (stack machine)
//#
//# $Id$
//#



#include "gsm.h"
#include "hash.h"



//--------------------------------------------------------------------
//                     hash tables used by GSM
//-------------------------------------------------------------------

gOutput& operator << ( class gOutput& s, class Portion* (*funcname)() )
{
  return s << funcname;
}

class FunctionHashTable : public HashTable<gString, FuncDescObj*>
{
 private:
  int NumBuckets() const 
  { 
    return 26; 
  }
  
  int Hash( const gString& funcname ) const 
  { 
    return (int)( funcname[0] % 26 ); 
  }

  void DeleteAction( FuncDescObj* func ) 
  { 
    delete func; 
  }

 public:
  FunctionHashTable() 
  { 
    Init(); 
  }

  ~FunctionHashTable() 
  { 
    Flush(); 
  }  
};


int GSM::FunctionsInitialized = false;
FunctionHashTable* GSM::FuncTable = new FunctionHashTable;



class RefHashTable : public HashTable<gString, Portion*>
{
 private:
  int NumBuckets( void ) const 
  { 
    return 26; 
  }
  
  int Hash( const gString& ref ) const 
  { 
    return (int)( ref[0] % 26 ); 
  }
  
  void DeleteAction( Portion* value ) 
  { 
    delete value; 
  }
  
 public:
  RefHashTable() 
  { 
    Init(); 
  }
  
  ~RefHashTable() 
  { 
    Flush(); 
  }
};





//--------------------------------------------------------------------
//              implementation of GSM (stack machine)
//--------------------------------------------------------------------

GSM::GSM( int size )
{
  assert( size > 0 );

  stack = new gStack<Portion*>( size );
  RefTable = new RefHashTable;

  if( !FunctionsInitialized )
  {
    InitFunctions();           // This function is located in gsmfunc.cc
    FunctionsInitialized = true;
  }
}


GSM::~GSM()
{
  Flush();
  delete RefTable;
  delete stack;
}


int GSM::Depth( void ) const
{
  return stack->Depth();
}


int GSM::MaxDepth( void ) const
{
  return stack->MaxDepth();
}





//------------------------------------------------------------------------
//                           Push() functions
//------------------------------------------------------------------------

void GSM::Push( const bool& data )
{
  Portion*  p;

  p = new bool_Portion( data );
  stack->Push( p );
}


void GSM::Push( const double& data )
{
  Portion*  p;

  p = new numerical_Portion<double>( data );
  stack->Push( p );
}


void GSM::Push( const gInteger& data )
{
  Portion*  p;

  p = new numerical_Portion<gInteger>( data );
  stack->Push( p );
}


void GSM::Push( const gRational& data )
{
  Portion*  p;

  p = new numerical_Portion<gRational>( data );
  stack->Push( p );
}


void GSM::Push( const gString& data )
{
  Portion*  p;

  p = new gString_Portion( data );
  stack->Push( p );
}


void GSM::PushList( const int num_of_elements )
{ 
  int            i;
  Portion*       p;
  List_Portion*  list;

  assert( num_of_elements > 0 );

  list = new List_Portion;

#ifndef NDEBUG
  if( num_of_elements > stack->Depth() )
  {
    gerr << "GSM Error: not enough elements in GSM to PushList()\n";
    gerr << "           elements requested: " << num_of_elements << "\n";
    gerr << "           elements available: " << stack->Depth() << "\n";
  }
  assert( num_of_elements <= stack->Depth() );
#endif // NDEBUG

  for( i = 1; i <= num_of_elements; i++ )
  {
    p = stack->Pop();
    list->Insert( p, 1 );
  }
  stack->Push( list );
}



//----------------------------------------------------------------------
//     Reference related functions: PushRef(), Assign(), UnAssign()
//---------------------------------------------------------------------

void GSM::PushRef( const gString& data )
{
  Portion*  p;

  p = new Reference_Portion( data );
  stack->Push( p );
}



void GSM::Assign( void )
{
  Portion*  p2;
  Portion*  p1;

#ifndef NDEBUG
  if( stack->Depth() < 2 )
  {
    gerr << "GSM Error: not enough operands to execute Assign()\n";
  }
  assert( stack->Depth() >= 2 );
#endif // NDEBUG

  p2 = stack->Pop();
  p1 = stack->Pop();

#ifndef NDEBUG
  if ( p1->Type() != porREFERENCE )
  {
    gerr << "GSM Error: no reference found to be assigned\n";
  }
  assert( p1->Type() == porREFERENCE );
#endif // NDEBUG

  if( p2->Type() == porREFERENCE )
    p2 = resolve_ref( (Reference_Portion*) p2 );
  
  RefTable->Define( ( (Reference_Portion*) p1 )->Value(), p2->Copy() );
  delete p2;
  
  p1 = resolve_ref( (Reference_Portion*) p1 );
  stack->Push( p1 );
}


void GSM::UnAssign( const gString& ref )
{
  if( RefTable->IsDefined( ref ) )
  {
    RefTable->Remove( ref );
  }
}





//---------------------------------------------------------------------
//                        operation functions
//-----------------------------------------------------------------------

Portion* GSM::resolve_ref( Reference_Portion* p )
{
  Portion*  result = 0;
  gString&  ref = p->Value();

#ifndef NDEBUG
  if( !RefTable->IsDefined( ref ) )
  {
    gerr << "GSM Error: attempted to resolve an undefined reference\n";
    gerr << "           \"" << ref << "\"\n";
  }
  assert( RefTable->IsDefined( ref ) );
#endif // NDEBUG

  result = (*RefTable)( ref )->Copy();
  delete p;

  return result;
}



//------------------------------------------------------------------------
//                       binary operations
//------------------------------------------------------------------------

void GSM::binary_operation( OperationMode mode )
{
  Portion*   p2;
  Portion*   p1;
  int        result = 0;

#ifndef NDEBUG
  if( stack->Depth() < 2 )
  {
    gerr << "GSM Error: not enough operands to perform binary operation\n";
  }
  assert( stack->Depth() >= 2 );
#endif // NDEBUG

  p2 = stack->Pop();
  p1 = stack->Pop();
  
  if( p2->Type() == porREFERENCE )
    p2 = resolve_ref( (Reference_Portion*) p2 );
  if( p1->Type() == porREFERENCE )
    p1 = resolve_ref( (Reference_Portion*) p1 );

#ifndef NDEBUG
  if( p1->Type() != p2->Type() )
  {
    gerr << "GSM Error: attempted operating on different types\n";
    gerr << "           Type of Operand 1: " << p1->Type() << "\n";
    gerr << "           Type of Operand 2: " << p2->Type() << "\n";
  }
  assert( p1->Type() == p2->Type() );
#endif // NDEBUG

  result = p1->Operation( p2, mode );
  if(mode == opEQUAL_TO ||
     mode == opNOT_EQUAL_TO ||
     mode == opGREATER_THAN ||
     mode == opLESS_THAN ||
     mode == opGREATER_THAN_OR_EQUAL_TO ||
     mode == opLESS_THAN_OR_EQUAL_TO )
  {
    delete p1;
    p1 = new bool_Portion( (bool) result );
  }
  stack->Push( p1 );
}




//-----------------------------------------------------------------------
//                        unary operations
//-----------------------------------------------------------------------

void GSM::unary_operation( OperationMode mode )
{
  Portion*  p1;

#ifndef NDEBUG
  if( stack->Depth() < 1 )
  {
    gerr << "GSM Error: not enough operands to perform unary operation\n";
  }
  assert( stack->Depth() >= 1 );
#endif // NDEBUG

  p1 = stack->Pop();
  
  if( p1->Type() == porREFERENCE )
    p1 = resolve_ref( (Reference_Portion*) p1 );
  
  p1->Operation( 0, mode );
  stack->Push( p1 );
}




//-----------------------------------------------------------------
//                      built-in operations
//-----------------------------------------------------------------

void GSM::Add ( void )
{ binary_operation( opADD ); }

void GSM::Subtract ( void )
{ binary_operation( opSUBTRACT ); }

void GSM::Multiply ( void )
{ binary_operation( opMULTIPLY ); }

void GSM::Divide ( void )
{ binary_operation( opDIVIDE ); }

void GSM::Negate( void )
{ unary_operation( opNEGATE ); }



void GSM::EqualTo ( void )
{ binary_operation( opEQUAL_TO ); }

void GSM::NotEqualTo ( void )
{ binary_operation( opNOT_EQUAL_TO ); }

void GSM::GreaterThan ( void )
{ binary_operation( opGREATER_THAN ); }

void GSM::LessThan ( void )
{ binary_operation( opLESS_THAN ); }

void GSM::GreaterThanOrEqualTo ( void )
{ binary_operation( opGREATER_THAN_OR_EQUAL_TO ); }

void GSM::LessThanOrEqualTo ( void )
{ binary_operation( opLESS_THAN_OR_EQUAL_TO ); }


void GSM::AND ( void )
{ binary_operation( opLOGICAL_AND ); }

void GSM::OR ( void )
{ binary_operation( opLOGICAL_OR ); }

void GSM::NOT ( void )
{ unary_operation( opLOGICAL_NOT ); }


void GSM::Concatenate( void )
{ binary_operation( opCONCATENATE ); }







//-------------------------------------------------------------------
//               CallFunction() related functions
//-------------------------------------------------------------------

void GSM::AddFunction( const gString& funcname, FuncDescObj* func )
{
  FuncTable->Define( funcname, func );
}


int GSM::FuncParamCheck( const PortionType stack_param_type, 
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


void GSM::CallFunction( const gString& funcname )
{
  FuncDescObj*  func;
  Portion**     param;
  Portion*      result;
  int           i;
  int           num_of_params;
  int           type_match;

#ifndef NDEBUG
  if( !FuncTable->IsDefined( funcname ) )
  {
    gerr << "GSM Error: CallFunction() called with an undefined function\n";
    gerr << "           CallFunction( \"" << funcname << "\" )\n";
  }
  assert( FuncTable->IsDefined( funcname ) );
#endif // NDEBUG

  func = (*FuncTable)( funcname );
  num_of_params = func->NumParams();
  param = new Portion* [ num_of_params ];

  for( i = num_of_params - 1; i >= 0; i-- )
  {
    param[ i ] = stack->Pop();
    
    if( param[ i ]->Type() == porREFERENCE )
      param[ i ] = resolve_ref( (Reference_Portion*) param[ i ] );

#ifndef NDEBUG
    type_match = FuncParamCheck( param[ i ]->Type(), func->ParamType( i ) );
    if( !type_match )
    {
      gerr << "GSM Error: mismatched parameter type found while executing\n";
      gerr << "           CallFunction( \"" << funcname << "\" )\n\n";
      gerr << "    Error at Parameter #: " << i << "\n";
      gerr << "           Expected type: " << func->ParamType( i ) << "\n";
      gerr << "           Type found:    " << param[i]->Type() << "\n";
    }
    assert( type_match );
#endif // NDEBUG
  }
  
  result = func->CallFunction( param );
  
#ifndef NDEBUG
  if( result == 0 )
  {
    gerr << "GSM Error: an error occurred while attempting to execute\n";
    gerr << "           CallFunction( \"" << funcname << "\" )\n";
  }
  assert( result != 0 );
#endif // NDEBUG

  stack->Push( result );
}


void GSM::CallFunction3
  ( 
   const gString& funcname,    // name of the function to be called
   const gString* name,        // the formal names of all parameters passed in
   const int num_of_names      // the number of parameters passed in
   )
{
  FuncDescObj*  func;          // Function Descriptor Object
  Portion*      result;        // stores the function return value
  Portion**     param;         // parameter list to pass to the function
  int           i;             // index to the parameter list
  int           name_i;        // index to the formal names passed in
  int           num_of_params; // actual number of parameters of the function
  int           type_match;    // status variable

#ifndef NDEBUG
  if( !FuncTable->IsDefined( funcname ) )
  {
    gerr << "GSM Error: CallFunction() called with an undefined function\n";
    gerr << "           CallFunction( \"" << funcname << "\", ... )\n";
  }
  assert( FuncTable->IsDefined( funcname ) );
#endif // NDEBUG

  func = (*FuncTable)( funcname );
  num_of_params = func->NumParams();
  param = new Portion* [ num_of_params ];
  
  for( i = 0; i < num_of_params; i++ )
  {
    param[ i ] = func->ParamDefaultValue( i );
  }
  
  for( name_i = num_of_names - 1; name_i >= 0; name_i-- )
  {
    i = func->FindParamName( name[ name_i ] );

#ifndef NDEBUG
    if( i == -1 )
    {
      gerr << "FuncDescObj Error: parameter \"" << name[ name_i ];
      gerr << "\" is not defined for\n";
      gerr << "                   the function \"" << funcname << "\"\n"; 
    }
    assert( i >= 0 );
#endif // NDEBUG
    
    param[ i ] = stack->Pop();
    
    if( param[ i ]->Type() == porREFERENCE )
      param[ i ] = resolve_ref( (Reference_Portion*) param[ i ] );
    
#ifndef NDEBUG
    type_match = FuncParamCheck( param[ i ]->Type(), func->ParamType( i ) );
    if( !type_match )
    {
      gerr << "GSM Error: mismatched parameter type found while executing\n";
      gerr << "           CallFunction( \"" << funcname << "\", ... )\n\n";
      gerr << "    Error at Parameter #: " << i << "\n";
      gerr << "           Expected type: " << func->ParamType( i ) << "\n";
      gerr << "           Type found:    " << param[ i ]->Type() << "\n";
    }
    assert( type_match );
#endif // NDEBUG
  }
  
#ifndef NDEBUG
  for( i = 0; i < num_of_params; i++ )
  {
    if( param[ i ] == 0 )
    {
      gerr << "GSM Error: required parameter found missing while executing\n";
      gerr << "           CallFunction( \"" << funcname << "\" )\n\n";
      gerr << "   Missing Parameter: \"" << func->ParamName( i ) << "\"\n";
    }
    assert( param[ i ] != 0 );
  }
#endif // NDEBUG
  
  result = func->CallFunction( param );

#ifndef NDEBUG
  if( result == 0 )
  {
    gerr << "GSM Error: an error occurred while attempting to execute\n";
    gerr << "           CallFunction( \"" << funcname << "\", ... )\n";
  }
  assert( result != 0 );
#endif // NDEBUG

  stack->Push( result );
}


void GSM::CallFunction4
  ( 
   const gString&  funcname,             // name of the function to be called
   const int       num_of_params_passed, // number of parameters passed in
   const gString*  name                  // the correponding formal names
   )
{
  FuncDescObj*  func;          // Function Descriptor Object
  Portion*      result;        // stores the function return value
  Portion**     param;         // parameter list to pass to the function
  int           i;             // index to the parameter list
  int           old_i;         // used to check parameter redefinitions
  Portion**     passed_param;  // parameter list received
  int           passed_i;      // index to the parameter list received
  int           num_of_params; // actual number of paramater of the function
  int           type_match;    // status variable

  func = (*FuncTable)( funcname );
  num_of_params = func->NumParams();

#ifndef NDEBUG
  if( !FuncTable->IsDefined( funcname ) )
  {
    gerr << "GSM Error: CallFunction() called with an undefined function\n";
    gerr << "           CallFunction( \"" << funcname << "\", ... )\n";
  }
  assert( FuncTable->IsDefined( funcname ) );
#endif // NDEBUG

  param = new Portion* [ num_of_params ];
  for( i = 0; i < num_of_params; i++ )
  {
    param[ i ] = func->ParamDefaultValue( i );
  }

  passed_param = new Portion* [ num_of_params_passed ];
  for( passed_i = num_of_params_passed - 1; passed_i >= 0; passed_i-- )
  {
    passed_param[ passed_i ] = stack->Pop();
  }

  // Maps the parameters into their correct positions in the parameter list
  i = 0;
  for( passed_i = 0; passed_i < num_of_params_passed; passed_i++ )
  {
#ifndef NDEBUG
    if( i >= num_of_params )
    {
      gerr << "FuncDescObj Error: CallFunction( \"" << funcname;
      gerr << "\", ... ) executed with\n";
      gerr << "                   too many function parameters\n"; 
    }
    assert( i < num_of_params );
#endif // NDEBUG

    if( ( func->ParamName( i ) != name[ passed_i ] ) &&
       ( name[ passed_i ] != "" ) )
    {
      old_i = i;
      i = func->FindParamName( name[ passed_i ] );

#ifndef NDEBUG
      if( i <= old_i )
      {
	gerr << "GSM Error: multiple definitions found for parameter \"";
	gerr << name[ passed_i ] << "\" while executing\n";
	gerr << "           CallFunction( \"" << funcname << "\", ... )\n";
      }
      assert( i > old_i );

      if( i == -1 )
      {
	gerr << "FuncDescObj Error: parameter \"" << name[ passed_i ];
	gerr << "\" is not defined for\n";
	gerr << "                   the function \"" << funcname << "\"\n"; 
      }
      assert( i >= 0 );
#endif // NDEBUG
    }

    param[ i ] = passed_param[ passed_i ];
    i++;
  }

  for( i = 0; i < num_of_params; i++ )
  {
#ifndef NDEBUG
    if( param[ i ] == 0 )
    {
      gerr << "GSM Error: required parameter found missing while executing\n";
      gerr << "           CallFunction( \"" << funcname << "\", ... )\n";
      gerr << "   Missing Parameter: \"" << func->ParamName( i ) << "\"\n";
    }
    assert( param[ i ] != 0 );
#endif // NDEBUG

    if( param[ i ]->Type() == porREFERENCE )
      param[ i ] = resolve_ref( (Reference_Portion *)param[ i ] );
    
#ifndef NDEBUG
    type_match = FuncParamCheck( param[ i ]->Type(), func->ParamType( i ) );
    if( !type_match )
    {
      gerr << "GSM Error: mismatched parameter type found while executing\n";
      gerr << "           CallFunction( \"" << funcname << "\", ... )\n\n";
      gerr << "    Error at Parameter #: " << i << "\n";
      gerr << "           Expected type: " << func->ParamType( i ) << "\n";
      gerr << "           Type found:    " << param[ i ]->Type() << "\n";
    }
    assert( type_match );
#endif // NDEBUG
  }
  
  result = func->CallFunction( param );
  
#ifndef NDEBUG
  if( result == 0 )
  {
    gerr << "GSM Error: an error occurred while attempting to execute\n";
    gerr << "           CallFunction( \"" << funcname << "\", ... )\n";
  }
  assert( result != 0 );
#endif // NDEBUG

  stack->Push( result );
}






//----------------------------------------------------------------------------
//                   miscellaneous functions
//----------------------------------------------------------------------------

void GSM::Output( void )
{
  Portion*  p;

  p = stack->Pop();
  if( p->Type() == porREFERENCE )
  {
    p = resolve_ref( (Reference_Portion*) p );
  }
  p->Output( gout );
  delete p;
}


void GSM::Dump( void )
{
  int  i;

  for( i = stack->Depth() - 1; i >= 0; i-- )
  {
    gout << "Stack element " << i << " : ";
    Output();
  }
  gout << "\n";

  assert( stack->Depth() == 0 );
}


void GSM::Flush( void )
{
  int       i;
  Portion*  p;

  for( i = stack->Depth() - 1; i >= 0; i-- )
  {
    p = stack->Pop();
    delete p;
  }

  assert( stack->Depth() == 0 );
}




//-----------------------------------------------------------------------
//                       Template instantiations
//-----------------------------------------------------------------------

#include "hash.imp"

template class HashTable<gString, Portion*>;
template class HashTable<gString, FuncDescObj*>;


#include "glist.imp"

template class gList<Portion*>;
template class gNode<Portion*>;

template class gList<gString>;
template class gNode<gString>;

template class gList<FuncDescObj*>;
template class gNode<FuncDescObj*>;


#include "gstack.imp"

template class gStack<Portion*>;
