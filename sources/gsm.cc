//
//  FILE: GSM.cc  implementation of GSM (stack machine)
//
//


#include "gsm.h"
#include <assert.h>
#include <stdio.h>


#include "hash.imp"


class RefHashTable : public HashTable<gString, Portion *>
{
 private:
  int NumBuckets( void ) const { return 1; }
  int Hash( gString ref ) const { return 0; }
 public:
  RefHashTable() { Init(); }
};



//--------------------------------------------------------------------
// implementation of Portion base and descendent classes
//--------------------------------------------------------------------

Portion::Portion( void )
{
  type = porERROR;
}

PortionType Portion::Type( void ) const
{
  return type;
}

//------------------------- double type ------------------------------
double_Portion::double_Portion( const double new_value )
     : value( new_value )
{
  type = porDOUBLE;
}

double double_Portion::Value( void ) const
{ return value; }
double& double_Portion::Value( void )
{ return value; }


//------------------------- gInteger type ------------------------------
gInteger_Portion::gInteger_Portion( const gInteger new_value )
     : value( new_value )
{
  type = porINTEGER;
}

gInteger gInteger_Portion::Value( void ) const
{ return value; }
gInteger& gInteger_Portion::Value( void )
{ return value; }


//------------------------- gRational type ------------------------------
gRational_Portion::gRational_Portion( const gRational new_value )
     : value( new_value )
{
  type = porRATIONAL;
}

gRational gRational_Portion::Value( void ) const
{ return value; }
gRational& gRational_Portion::Value( void )
{ return value; }


//------------------------- Reference type ------------------------------
Reference_Portion::Reference_Portion( const gString new_value )
     : value( new_value )
{
  type = porREFERENCE;
}

gString Reference_Portion::Value( void ) const
{ return value; }
gString& Reference_Portion::Value( void )
{ return value; }





//--------------------------------------------------------------------
// implementation of GSM (stach machine)
//--------------------------------------------------------------------

GSM::GSM( int size )
{
  assert( size > 0 );
  stack = new gStack<Portion *>( size );
  RefStack = new gStack<gString>( size );
  RefTable = new RefHashTable;
  assert( stack != 0 );
}

GSM::~GSM()
{
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
void GSM::Assign( const gString ref, const double data )
{
  Portion *data_portion;

  if( !RefTable->IsDefined( ref ) )
  {
    data_portion = new double_Portion( data );
    RefTable->Define( ref, data_portion );
  }
  else
  {
    data_portion = (*RefTable)( ref );
    delete data_portion;
    data_portion = new double_Portion( data );
    (*RefTable)( ref ) = data_portion;
  }
}

void GSM::Assign( const gString ref, const gInteger data )
{
  Portion *data_portion;

  if( !RefTable->IsDefined( ref ) )
  {
    data_portion = new gInteger_Portion( data );
    RefTable->Define( ref, data_portion );
  }
  else
  {
    data_portion = (*RefTable)( ref );
    delete data_portion;
    data_portion = new gInteger_Portion( data );
    (*RefTable)( ref ) = data_portion;
  }
}

void GSM::Assign( const gString ref, const gRational data )
{
  Portion *data_portion;

  if( !RefTable->IsDefined( ref ) )
  {
    data_portion = new gRational_Portion( data );
    RefTable->Define( ref, data_portion );
  }
  else
  {
    data_portion = (*RefTable)( ref );
    delete data_portion;
    data_portion = new gRational_Portion( data );
    (*RefTable)( ref ) = data_portion;
  }
}


void GSM::UnAssign( const gString ref )
{
  if( RefTable->IsDefined( ref ) )
  {
    RefTable->Remove( ref );
  }
}




//------------------------------------------------------------------------
  // Push() functions
//------------------------------------------------------------------------

void GSM::Push( const double data )
{
  Portion *p;
  p = new double_Portion( data );
  stack->Push( p );
}

void GSM::Push( const gInteger data )
{
  Portion *p;
  p = new gInteger_Portion( data );
  stack->Push( p );
}

void GSM::Push( const gRational data )
{
  Portion *p;
  p = new gRational_Portion( data );
  stack->Push( p );
}

void GSM::Push( const gString data )
{
  Portion *p;
  p = new Reference_Portion( data );
  stack->Push( p );
  RefStack->Push( data );
}

void GSM::PushRef( const gString data )
{
  Push( data );
}




//------------------------------------------------------------------
  // PushVal() functions
//------------------------------------------------------------------

void GSM::PushVal( const double data )
{
  gString ref;
  while( RefStack->Depth() > 0 )
  {
    ref = RefStack->Pop();
    if( !RefTable->IsDefined( ref ) )
    {
      Assign( ref, data );
      break;
    }
  }
  if( RefStack->Depth() <= 0 )
    gerr << "** GSM Error: no undefined variable to assign PushVal()\n";
}

void GSM::PushVal( const gInteger data )
{
  gString ref;
  while( RefStack->Depth() > 0 )
  {
    ref = RefStack->Pop();
    if( !RefTable->IsDefined( ref ) )
    {
      Assign( ref, data );
      break;
    }
  }
  if( RefStack->Depth() <= 0 )
    gerr << "** GSM Error: no undefined variable to assign PushVal()\n";
}

void GSM::PushVal( const gRational data )
{
  gString ref;
  while( RefStack->Depth() > 0 )
  {
    ref = RefStack->Pop();
    if( !RefTable->IsDefined( ref ) )
    {
      Assign( ref, data );
      break;
    }
  }
  if( RefStack->Depth() <= 0 )
    gerr << "** GSM Error: no undefined variable to assign PushVal()\n";
}





//---------------------------------------------------------------------
  // operation functions
//---------------------------------------------------------------------

Portion *GSM::resolve_ref( Reference_Portion *p )
{
  Portion *data, *result;
  gString ref;

  ref = p->Value();
  if( RefTable->IsDefined( ref ) )
  {
    data = (*RefTable)( ref );

    // A new Portion is allocated because the operations will free it;
    // and we do not want the Portion stored in the hash table to be freed.
    switch( data->Type() )
    {
    case porDOUBLE:
      result = new double_Portion( ((double_Portion *)data)->Value() );
      break;
    case porINTEGER:
      result = new gInteger_Portion( ((gInteger_Portion *)data)->Value() );
      break;
    case porRATIONAL:
      result = new gRational_Portion( ((gRational_Portion *)data)->Value() );
      break;
    default:     
      gerr << "** GSM Error: a variable refered to an unknown type\n";
      assert(0);
      result = 0;
    }
  }
  else
  {
    gerr << "** GSM Error: attempted to operate with an undefined variable\n";
    assert(0);
    result = 0;
  }
  return result;
}




void GSM::operation( OperationMode mode )
{
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
      switch( p2->Type() )
      {
      case porDOUBLE:
	double_operation( (double_Portion *)p1, 
			 (double_Portion *)p2, 
			 mode );
	break;
      case porINTEGER:
	gInteger_operation( (gInteger_Portion *)p1, 
			   (gInteger_Portion *)p2, 
			   mode );
	break;
      case porRATIONAL:
	gRational_operation( (gRational_Portion *)p1, 
			    (gRational_Portion *)p2,
			    mode );
	break;

      default:
	stack->Push( p1 );
	stack->Push( p2 );
	gout << "** GSM Error: operating on unsupported types\n";
      }
    }
    else
    {  
      stack->Push( p1 );
      stack->Push( p2 );
      gout << "** GSM Error: attempted operating on different types\n";
    }
  }
  else
  {
    gout << "** GSM Error: not enough operands to perform binary operation\n";
  }  
}



void GSM::double_operation( double_Portion *p1, 
			   double_Portion *p2, 
			   OperationMode mode )
{
  switch( mode )
  {
  case opADD:
    p1->Value() += p2->Value();
    break;
  case opSUBTRACT:
    p1->Value() -= p2->Value();
    break;
  case opMULTIPLY:
    p1->Value() *= p2->Value();
    break;
  case opDIVIDE:
    p1->Value() /= p2->Value();
    break;
  default:
    stack->Push( p1 );
    stack->Push( p2 );
    gerr << "** GSM Error: attempted an unsupported operation\n";
    assert(0);
    return;
  }
  stack->Push( p1 );
  delete p2;
}

void GSM::gInteger_operation( gInteger_Portion *p1, 
					  gInteger_Portion *p2, 
					  OperationMode mode )
{
  switch( mode )
  {
  case opADD:
    p1->Value() += p2->Value();
    break;
  case opSUBTRACT:
    p1->Value() -= p2->Value();
    break;
  case opMULTIPLY:
    p1->Value() *= p2->Value();
    break;
  case opDIVIDE:
    p1->Value() /= p2->Value();
    break;
  default:
    stack->Push( p1 );
    stack->Push( p2 );
    gerr << "** GSM Error: attempted an unsupported operation\n";
    assert(0);
    return;
  }
  stack->Push( p1 );
  delete p2;
}

void GSM::gRational_operation( gRational_Portion *p1, 
				    gRational_Portion *p2, 
				    OperationMode mode )
{
  switch( mode )
  {
  case opADD:
    p1->Value() += p2->Value();
    break;
  case opSUBTRACT:
    p1->Value() -= p2->Value();
    break;
  case opMULTIPLY:
    p1->Value() *= p2->Value();
    break;
  case opDIVIDE:
    p1->Value() /= p2->Value();
    break;
  default:
    stack->Push( p1 );
    stack->Push( p2 );
    gerr << "** GSM Error: attempted an unsupported operation\n";
    assert(0);
    return;
  }
  stack->Push( p1 );
  delete p2;
}








void GSM::Add( void )
{
  operation( opADD );
}

void GSM::Subtract( void )
{
  operation( opSUBTRACT );
}

void GSM::Multiply( void )
{
  operation( opMULTIPLY );
}

void GSM::Divide( void )
{
  operation( opDIVIDE );
}



void GSM::Negate( void )
{
  Portion *p;
  if( stack->Depth() > 0 )
  {
    p = stack->Pop();
    if( p->Type() == porREFERENCE )
      p = resolve_ref( (Reference_Portion*)p );

    switch( p->Type() )
    {
    case porDOUBLE:
      ((double_Portion *)p)->Value() = - ((double_Portion *)p)->Value() ;
      break;
    case porINTEGER:
      ((gInteger_Portion *)p)->Value() = - ((gInteger_Portion *)p)->Value() ;
      break;
    case porRATIONAL:
      ((gRational_Portion *)p)->Value() = - ((gRational_Portion *)p)->Value() ;
      break;
    default:
      gerr << "** GSM Error: attempted to negate an supported type\n";
    }
    stack->Push( p );
  }
}


void GSM::Dump( void )
{
  int i;
  Portion *p;
  for( i = stack->Depth() - 1; i >= 0; i-- )
  {
    gout << "Stack element " << i << " : ";
    p = stack->Pop();
    switch( p->Type() )
    {
    case porDOUBLE:
      gout << ((double_Portion *)p)->Value() << "    type: double\n" ;
      break;
    case porINTEGER:
      gout << ((gInteger_Portion *)p)->Value() << "    type: gInteger\n" ;
      break;
    case porRATIONAL:
      gout << ((gRational_Portion *)p)->Value() << "    type: gRational\n" ;
      break;
    case porREFERENCE:
      gout << ((Reference_Portion *)p)->Value() << "    type: Reference\n" ;
      break;
    default:
      gerr << "** GSM Error: unknown type found in stack\n";
    }
  }
  gout << "\n";
  assert( stack->Depth() == 0 );
}

