//
//  FILE: GSM.cc  implementation of GSM (stack machine)
//
//


#include "gsm.h"

#include "hash.imp"





//--------------------------------------------------------------------
  // hash tables used by GSM
//-------------------------------------------------------------------

gOutput& operator << (class gOutput &s, class Portion *(*funcname)( void ) )
{
  return s << funcname;
}

class FunctionHashTable : public HashTable<gString, Portion *(*)( void )>
{
 private:
  int NumBuckets() const { return 1; }
  int Hash( const gString &funcname ) const { return 0; }
  void DeleteAction( Portion *(*function)( void ) ) { return; }
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

void GSM::AddFunction( const gString& funcname, Portion *(*function)( void ) )
{
  FuncTable->Define( funcname, function );
}

void GSM::CallFunction( const gString& funcname )
{
  Portion *(*function)();
  Portion *p;

  if( FuncTable->IsDefined( funcname ) )
  {
    function = (*FuncTable)( funcname );
    p = function();
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




//-------------------------------------------------------
  //  Assign & UnAssign functions 
//-------------------------------------------------------


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

      if( !RefTable->IsDefined( ref ) )
	RefTable->Define( ref, p2 );
      else
	(*RefTable)( ref ) = p2;

      stack->Push( p2 );
      delete p1;
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

void GSM::PushRef( const gString& data )
{
  Portion *p;
  p = new Reference_Portion( data );
  stack->Push( p );
}





//---------------------------------------------------------------------
  // operation functions
//-----------------------------------------------------------------------

Portion *GSM::resolve_ref( Reference_Portion *p )
{
  Portion *data, *result;
  gString ref;

  ref = p->Value();
  if( RefTable->IsDefined( ref ) )
  {
    result = (*RefTable)( ref )->Copy();
  }
  else
  {
    gerr << "** GSM Error: attempted to operate with an undefined variable\n";
    assert(0);
    result = 0;
  }
  return result;
}



//------------------------------------------------------------------------
  // binary operations
//------------------------------------------------------------------------

int GSM::binary_operation( OperationMode mode )
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
      stack->Push( p1 );
      if(mode == opEQUAL_TO ||
	 mode == opNOT_EQUAL_TO ||
	 mode == opGREATER_THAN ||
	 mode == opLESS_THAN ||
	 mode == opGREATER_THAN_OR_EQUAL_TO ||
	 mode == opLESS_THAN_OR_EQUAL_TO )
	stack->Push( p2 );
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

  return result;
}


//-----------------------------------------------------------------------
  // Unary operations
//-----------------------------------------------------------------------

int GSM::unary_operation( OperationMode mode )
{
  int result = 0;
  Portion *p1;
  if( stack->Depth() > 0 )
  {
    p1 = stack->Pop();
    
    if( p1->Type() == porREFERENCE )
      p1 = resolve_ref( (Reference_Portion *)p1 );

    result = p1->Operation( 0, mode );
    stack->Push( p1 );
  }
  else
  {
    gerr << "** GSM Error: not enough operands to perform unary operation\n";
  }  

  return result;
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



int  GSM::EqualTo ( void )
{ return binary_operation( opEQUAL_TO ); }

int  GSM::NotEqualTo ( void )
{ return binary_operation( opNOT_EQUAL_TO ); }

int  GSM::GreaterThan ( void )
{ return binary_operation( opGREATER_THAN ); }

int  GSM::LessThan ( void )
{ return binary_operation( opLESS_THAN ); }

int  GSM::GreaterThanOrEqualTo ( void )
{ return binary_operation( opGREATER_THAN_OR_EQUAL_TO ); }

int  GSM::LessThanOrEqualTo ( void )
{ return binary_operation( opLESS_THAN_OR_EQUAL_TO ); }


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
  RefTable->Flush();
  assert( stack->Depth() == 0 );
}
