//#
//# FILE: gsm.cc  implementation of GSM (stack machine)
//#
//# $Id$
//#

#include "gsm.h"
#include "hash.h"



//--------------------------------------------------------------------
  // hash tables used by GSM
//-------------------------------------------------------------------

gOutput& operator << ( class gOutput &s, class Portion * (*funcname)() )
{
  return s << funcname;
}

class FunctionHashTable : public HashTable<gString, FuncDescObj *>
{
 private:
  int NumBuckets() const { return 1; }
  int Hash( const gString &funcname ) const { return 0; }
  void DeleteAction( FuncDescObj *func ) { delete func; }
 public:
  FunctionHashTable() { Init(); }
  ~FunctionHashTable() { Flush(); }  
};

int GSM::FunctionsInitialized = false;
FunctionHashTable *GSM::FuncTable = new FunctionHashTable;


class RefHashTable : public HashTable<gString, Portion *>
{
 private:
  int NumBuckets( void ) const { return 1; }
  int Hash( const gString& ref ) const { return 0; }
  void DeleteAction( Portion *value ) { delete value; }
 public:
  RefHashTable() { Init(); }
  ~RefHashTable() { Flush(); }
};


//-------------------------------------------------------------------
  // definable function functions
//-------------------------------------------------------------------

void GSM::AddFunction(const gString& funcname, FuncDescObj *func )
{
  FuncTable->Define( funcname, func );
}

int GSM::FunctionParamCheck(const PortionType stack_param_type, 
                            const PortionType func_param_type )
{
  int result = false;
  if( stack_param_type == func_param_type )
    result = true;
  else if(func_param_type == porNUMERICAL &&
          (stack_param_type == porDOUBLE ||
           stack_param_type == porINTEGER ||
           stack_param_type == porRATIONAL ) )
    result = true;
  return result;
}

void GSM::CallFunction( const gString& funcname )
{
  FuncDescObj *func;
  Portion **param_list;
  Portion *p;
  int i, current_index, num_of_params;
  int type_match;

  if( FuncTable->IsDefined( funcname ) )
  {
    func = (*FuncTable)( funcname );
    num_of_params = func->NumParams();
    param_list = new (Portion*) [ num_of_params ];
    for( i = num_of_params - 1; i >= 0; i-- )
    {
      param_list[ i ] = stack->Pop();
      type_match = FunctionParamCheck( param_list[ i ]->Type(), func->ParamType( i ) );
      if( !type_match )
      {
        gerr << "** GSM Error: a mismatched parameter type found while executing\n";
        gerr << "              CallFunction( \"" << funcname << "\" )\n\n";
        gerr << "   Error at Parameter # : i " << "\n";
        gerr << "            Expected type: " << func->ParamType( i ) << "\n";
        gerr << "            Type found:    " << param_list[i]->Type() << "\n";
        assert(0);
      }
    }
    p = func->CallFunction( param_list );
    if( p == 0 )
    {
      gerr << "** GSM Error: an error occurred while attempting to execute\n";
      gerr << "              CallFunction( \"" << funcname << "\" )\n";
      assert(0);
    }
    stack->Push( p );
  }
  else
  {
    gerr << "** GSM Error: CallFunction() called with an undefined function\n";
  }
}



//--------------------------------------------------------------------
// implementation of GSM (stack machine)
//--------------------------------------------------------------------

GSM::GSM( int size )
{
  assert( size > 0 );
  stack = new gStack<Portion *>( size );
  RefTable = new RefHashTable;

  if( !FunctionsInitialized )
  {
    InitFunctions();           // This function is located in gsmfunc.cc
    FunctionsInitialized = true;
  }
  assert( stack != 0 );
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
  // Push() functions
//------------------------------------------------------------------------

void GSM::Push( const bool& data )
{
  Portion *p;
  p = new bool_Portion( data );
  stack->Push( p );
}

void GSM::Push( const double& data )
{
  Portion *p;
  p = new numerical_Portion<double>( data );
  stack->Push( p );
}

void GSM::Push( const gInteger& data )
{
  Portion *p;
  p = new numerical_Portion<gInteger>( data );
  stack->Push( p );
}

void GSM::Push( const gRational& data )
{
  Portion *p;
  p = new numerical_Portion<gRational>( data );
  stack->Push( p );
}

void GSM::Push( const gString& data )
{
  Portion *p;
  p = new gString_Portion( data );
  stack->Push( p );
}


void GSM::PushList( const int num_of_elements )
{
  int i;
  Portion *p;
  List_Portion *list;
  PortionType type;

  assert( num_of_elements > 0 );

  list = new List_Portion;
  if( num_of_elements <= stack->Depth() )
  {
    for( i = 1; i <= num_of_elements; i++ )
    {
      p = stack->Pop();
      list->Insert( p, 1 );
    }
    stack->Push( list );
  }
  else
  {
    gerr << "** GSM Error: not enough elements in GSM to PushList()\n";
    gerr << "              elements requested: " << num_of_elements << "\n";
    gerr << "              elements available: " << stack->Depth() << "\n";
    assert(0);
  }
}



//----------------------------------------------------------------------
  // Reference related functions: PushRef(), Assign(), UnAssign()
//---------------------------------------------------------------------

void GSM::PushRef( const gString& data )
{
  Portion *p;
  p = new Reference_Portion( data );
  stack->Push( p );
}




void GSM::Assign( void )
{
  int result = 0;
  Portion *p2, *p1;
  gString ref;

  if( stack->Depth() > 1 )
  {
    p2 = stack->Pop();
    p1 = stack->Pop();

    if( ( p2->Type() != porREFERENCE ) && ( p1->Type() == porREFERENCE ) )
    {
      ref = ( (Reference_Portion *)p1 )->Value();
      RefTable->Define( ref, p2->Copy() );
      delete p2;

      p1 = resolve_ref( (Reference_Portion *)p1 );
      stack->Push( p1 );
    }
    else
    {
      if( p2->Type() == porREFERENCE )
        gerr << "** GSM Error: no value found to assign to a reference\n";
      if ( p1->Type() != porREFERENCE )
        gerr << "** GSM Error: no reference found to be assigned\n";
      assert(0);
    }
  }
}


void GSM::UnAssign( const gString& ref )
{
  if( RefTable->IsDefined( ref ) )
  {
    RefTable->Remove( ref );
  }
}





//---------------------------------------------------------------------
  // operation functions
//-----------------------------------------------------------------------

Portion *GSM::resolve_ref( Reference_Portion *p )
{
  Portion *result = 0;
  gString ref;

  ref = p->Value();
  if( RefTable->IsDefined( ref ) )
  {
    result = (*RefTable)( ref )->Copy();
    delete p;
  }
  else
  {
    gerr << "** GSM Error: attempted to operate with an undefined variable\n";
    assert(0);
  }
  return result;
}



//------------------------------------------------------------------------
  // binary operations
//------------------------------------------------------------------------

void GSM::binary_operation( OperationMode mode )
{
  int result = 0;
  Portion *p2, *p1;
  if( stack->Depth() > 1 )
  {
    p2 = stack->Pop();
    p1 = stack->Pop();
    
    if( p2->Type() == porREFERENCE )
      p2 = resolve_ref( (Reference_Portion *)p2 );
    if( p1->Type() == porREFERENCE )
      p1 = resolve_ref( (Reference_Portion *)p1 );

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
        p1 = new bool_Portion( (bool) result );
      }
      stack->Push( p1 );
    }
    else
    {  
      stack->Push( p1 );
      stack->Push( p2 );
      gerr << "** GSM Error: attempted operating on different types\n";
    }
  }
  else
  {
    gerr << "** GSM Error: not enough operands to perform binary operation\n";
  }  
}


//-----------------------------------------------------------------------
  // Unary operations
//-----------------------------------------------------------------------

void GSM::unary_operation( OperationMode mode )
{
  Portion *p1;
  if( stack->Depth() > 0 )
  {
    p1 = stack->Pop();
    
    if( p1->Type() == porREFERENCE )
      p1 = resolve_ref( (Reference_Portion *)p1 );

    p1->Operation( 0, mode );
    stack->Push( p1 );
  }
  else
  {
    gerr << "** GSM Error: not enough operands to perform unary operation\n";
  }  
}




//-----------------------------------------------------------------
  // operations
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


//----------------------------------------------------------------------------

void GSM::Output( void )
{
  Portion *p;
  p = stack->Pop();
  if( p->Type() == porREFERENCE )
    p = resolve_ref( (Reference_Portion *)p );
  p->Output( gout );
  delete p;
}


void GSM::Dump( void )
{
  int i;
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
  int i;
  Portion *p;

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

template class HashTable<gString, Portion *>;
template class HashTable<gString, FuncDescObj *>;

#include "glist.imp"

template class gList<Portion *>;
template class gNode<Portion *>;

template class gList<gString>;
template class gNode<gString>;

template class gList<FuncDescObj *>;
template class gNode<FuncDescObj *>;

#include "gstack.imp"

template class gStack<Portion *>;
