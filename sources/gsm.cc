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
#ifndef NDEBUG
  if( size <= 0 )
  {
    gerr << "GSM Error: illegal stack size specified during initialization\n";
    gerr << "           stack size requested: " << size << "\n";
  }
  assert( size > 0 );
#endif // NDEBUG

  stack = new gStack<Portion*>( size );
  RefTable = new RefHashTable;

  FuncTable = new FunctionHashTable;
  InitFunctions();           // This function is located in gsmfunc.cc
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

bool GSM::Push( const bool& data )
{
  Portion*  p;
  bool      result = true;
  
  if( stack->Depth() < stack->MaxDepth() )
  {
    p = new bool_Portion( data );
    stack->Push( p );
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
  
  if( stack->Depth() < stack->MaxDepth() )
  {
    p = new numerical_Portion<double>( data );
    stack->Push( p );
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

  if( stack->Depth() < stack->MaxDepth() )
  {
    p = new numerical_Portion<gInteger>( data );
    stack->Push( p );
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
  
  if( stack->Depth() < stack->MaxDepth() )
  {
    p = new numerical_Portion<gRational>( data );
    stack->Push( p );
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
  
  if( stack->Depth() < stack->MaxDepth() )
  {
    p = new gString_Portion( data );
    stack->Push( p );
  }
  else
  {
    gerr << "GSM Error: out of stack space\n";
    result = false;
  }
  return result;
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

  if( num_of_elements > stack->Depth() )
  {
    gerr << "GSM Error: not enough elements in GSM to PushList()\n";
    gerr << "           elements requested: " << num_of_elements << "\n";
    gerr << "           elements available: " << stack->Depth() << "\n";
  }
  assert( num_of_elements <= stack->Depth() );
#endif // NDEBUG

  list = new List_Portion;
  for( i = 1; i <= num_of_elements; i++ )
  {
    p = stack->Pop();
    list->Insert( p, 1 );
  }
  stack->Push( list );

  return result;
}



//---------------------------------------------------------------------
//     Reference related functions: PushRef(), Assign(), UnAssign()
//---------------------------------------------------------------------

bool GSM::PushRef( const gString& data )
{
  Portion*  p;
  bool      result = true;
  
  if( stack->Depth() < stack->MaxDepth() )
  {
    p = new Reference_Portion( data );
    stack->Push( p );
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
  Portion*  p2;
  Portion*  p1;
  bool      result = true;

#ifndef NDEBUG
  if( stack->Depth() < 2 )
  {
    gerr << "GSM Error: not enough operands to execute Assign()\n";
  }
  assert( stack->Depth() >= 2 );
#endif // NDEBUG

  p2 = stack->Pop();
  p1 = stack->Pop();

  if ( p1->Type() == porREFERENCE )
  {
    if( p2->Type() == porREFERENCE )
      p2 = resolve_ref( (Reference_Portion*) p2 );
    
    RefTable->Define( ( (Reference_Portion*) p1 )->Value(), p2->Copy() );
    delete p2;
    
    p1 = resolve_ref( (Reference_Portion*) p1 );
    stack->Push( p1 );
  }
  else // ( p1->Type() != porREFERENCE )
  {
    gerr << "GSM Error: no reference found to be assigned\n";
    result = false;
  }

  return result;
}


bool GSM::UnAssign( const gString& ref )
{
  bool  result = true;

  if( RefTable->IsDefined( ref ) )
  {
    RefTable->Remove( ref );
  }
  else
  {
    result = false;
  }
  return result;
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

bool GSM::binary_operation( OperationMode mode )
{
  Portion*   p2;
  Portion*   p1;
  bool       result = true;

#ifndef NDEBUG
  if( stack->Depth() < 2 )
  {
    gerr << "GSM Error: not enough operands to perform binary operation\n";
    result = false;
  }
  assert( stack->Depth() >= 2 );
#endif // NDEBUG
  
  p2 = stack->Pop();
  p1 = stack->Pop();
  
  if( p2->Type() == porREFERENCE )
    p2 = resolve_ref( (Reference_Portion*) p2 );
  if( p1->Type() == porREFERENCE )
    p1 = resolve_ref( (Reference_Portion*) p1 );

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
    stack->Push( p1 );
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

bool GSM::unary_operation( OperationMode mode )
{
  Portion*  p1;
  bool      result = true;

  if( stack->Depth() >= 1 )
  {
    p1 = stack->Pop();
    
    if( p1->Type() == porREFERENCE )
      p1 = resolve_ref( (Reference_Portion*) p1 );
    
    p1->Operation( 0, mode );
    stack->Push( p1 );
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
{ return binary_operation( opADD ); }

bool GSM::Subtract ( void )
{ return binary_operation( opSUBTRACT ); }

bool GSM::Multiply ( void )
{ return binary_operation( opMULTIPLY ); }

bool GSM::Divide ( void )
{ return binary_operation( opDIVIDE ); }

bool GSM::Negate( void )
{ return unary_operation( opNEGATE ); }



bool GSM::EqualTo ( void )
{ return binary_operation( opEQUAL_TO ); }

bool GSM::NotEqualTo ( void )
{ return binary_operation( opNOT_EQUAL_TO ); }

bool GSM::GreaterThan ( void )
{ return binary_operation( opGREATER_THAN ); }

bool GSM::LessThan ( void )
{ return binary_operation( opLESS_THAN ); }

bool GSM::GreaterThanOrEqualTo ( void )
{ return binary_operation( opGREATER_THAN_OR_EQUAL_TO ); }

bool GSM::LessThanOrEqualTo ( void )
{ return binary_operation( opLESS_THAN_OR_EQUAL_TO ); }


bool GSM::AND ( void )
{ return binary_operation( opLOGICAL_AND ); }

bool GSM::OR ( void )
{ return binary_operation( opLOGICAL_OR ); }

bool GSM::NOT ( void )
{ return unary_operation( opLOGICAL_NOT ); }


bool GSM::Concatenate( void )
{ return binary_operation( opCONCATENATE ); }







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


bool GSM::CallFunction
  ( 
   const gString&  funcname,             // name of the function to be called
   const int       num_of_params_passed, // number of parameters passed in
   const gString*  name                  // the correponding formal names
   )
{
  FuncDescObj*  func;          // Function Descriptor Object
  Portion*      return_value;  // stores the function return value
  Portion**     param;         // parameter list to pass to the function
  int           i;             // index to the parameter list
  int           old_i;         // used to check parameter redefinitions
  Portion**     passed_param;  // parameter list received
  int           passed_i;      // index to the parameter list received
  int           num_of_params; // actual number of paramater of the function
  int           type_match;    // status variable
  int           result = true;


  func = (*FuncTable)( funcname );
  num_of_params = func->NumParams();


  if( !FuncTable->IsDefined( funcname ) )
  {
    gerr << "GSM Error: CallFunction() called with an undefined function\n";
    gerr << "           CallFunction( \"" << funcname << "\", ... )\n";
    return false;
  }

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
    if( i >= num_of_params )
    {
      gerr << "FuncDescObj Error: CallFunction( \"" << funcname;
      gerr << "\", ... ) executed with\n";
      gerr << "                   too many function parameters\n";
      return false;
    }
    
    if( ( func->ParamName( i ) != name[ passed_i ] ) &&
       ( name[ passed_i ] != "" ) )
    {
      old_i = i;
      i = func->FindParamName( name[ passed_i ] );
      
      if( i <= old_i )
      {
	gerr << "GSM Error: multiple definitions found for parameter \"";
	gerr << name[ passed_i ] << "\" while executing\n";
	gerr << "           CallFunction( \"" << funcname << "\", ... )\n";
	return false;
      }
      
      if( i == -1 )
      {
	gerr << "FuncDescObj Error: parameter \"" << name[ passed_i ];
	gerr << "\" is not defined for\n";
	gerr << "                   the function \"" << funcname << "\"\n";
	return false;
      }
    }
    
    param[ i ] = passed_param[ passed_i ];
    i++;
  }
  
  for( i = 0; i < num_of_params; i++ )
  {
    if( param[ i ] == 0 )
    {
      gerr << "GSM Error: required parameter found missing while executing\n";
      gerr << "           CallFunction( \"" << funcname << "\", ... )\n";
      gerr << "   Missing Parameter: \"" << func->ParamName( i ) << "\"\n";
      return false;
    }
    
    if( param[ i ]->Type() == porREFERENCE )
      param[ i ] = resolve_ref( (Reference_Portion *)param[ i ] );
    
    type_match = FuncParamCheck( param[ i ]->Type(), func->ParamType( i ) );
    if( !type_match )
    {
      gerr << "GSM Error: mismatched parameter type found while executing\n";
      gerr << "           CallFunction( \"" << funcname << "\", ... )\n\n";
      gerr << "    Error at Parameter #: " << i << "\n";
      gerr << "           Expected type: " << func->ParamType( i ) << "\n";
      gerr << "           Type found:    " << param[ i ]->Type() << "\n";
      return false;
    }
  }
  
  return_value = func->CallFunction( param );
  
#ifndef NDEBUG
  if( return_value == 0 )
  {
    gerr << "GSM Error: an error occurred while attempting to execute\n";
    gerr << "           CallFunction( \"" << funcname << "\", ... )\n";
  }
  assert( return_value != 0 );
#endif // NDEBUG
  
  stack->Push( return_value );

  return result;
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



#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__


#include "hash.imp"

TEMPLATE class HashTable<gString, Portion*>;
TEMPLATE class HashTable<gString, FuncDescObj*>;


#include "glist.imp"

TEMPLATE class gList<Portion*>;
TEMPLATE class gNode<Portion*>;

TEMPLATE class gList<gString>;
TEMPLATE class gNode<gString>;

TEMPLATE class gList<FuncDescObj*>;
TEMPLATE class gNode<FuncDescObj*>;


#include "gstack.imp"

TEMPLATE class gStack<Portion*>;
