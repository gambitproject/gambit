//
//  FILE: GSM.cc  implementation of GSM (stack machine)
//
//


#include "gsm.h"
#include <assert.h>
#include <stdio.h>




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





//--------------------------------------------------------------------
// implementation of GSM (stach machine)
//--------------------------------------------------------------------

GSM::GSM( int size )
{
  assert( size > 0 );
  stack = new gStack<Portion *>( size );
  assert( stack != 0 );
}

GSM::~GSM()
{
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



//-------------------------------------------------------------------------
  // Pop() functions
//-------------------------------------------------------------------------
void GSM::Pop( double &data )
{
  Portion *p;
  p = stack->Pop();
  if( p->Type() == porDOUBLE )
    data = ((double_Portion *)p)->Value();
  else
  {
    stack->Push( p );
    gout << "** GSM Error: attempted to Pop() the wrong data type; data unchanged\n";
  }
  delete p;
}

void GSM::Pop( gInteger &data )
{
  Portion *p;
  p = stack->Pop();
  if( p->Type() == porINTEGER )
    data = ((gInteger_Portion *)p)->Value();
  else
  {
    stack->Push( p );
    gout << "** GSM Error: attempted to Pop() the wrong data type; data unchanged\n";
  }  
  delete p;
}

void GSM::Pop( gRational &data )
{
  Portion *p;
  p = stack->Pop();
  if( p->Type() == porRATIONAL )
    data = ((gRational_Portion *)p)->Value();
  else
  {
    stack->Push( p );
    gout << "** GSM Error: attempted to Pop() the wrong data type; data unchanged\n";
  }
  delete p;
}



//---------------------------------------------------------------------
  // Peek() functions
//---------------------------------------------------------------------

void GSM::Peek( double &data )
{
  Portion *p;
  p = stack->Peek();
  if( p->Type() == porDOUBLE )
    data = ((double_Portion *)p)->Value();
  else
    gout << "** GSMError: attempted to Peek() the wrong data type; data unchanged\n";
}

void GSM::Peek( gInteger &data )
{
  Portion *p;
  p = stack->Peek();
  if( p->Type() == porINTEGER )
    data = ((gInteger_Portion *)p)->Value();
  else
    gout << "** GSM Error: attempted to Peek() the wrong data type; data unchanged\n";
}

void GSM::Peek( gRational &data )
{
  Portion *p;
  p = stack->Peek();
  if( p->Type() == porRATIONAL )
    data = ((gRational_Portion *)p)->Value();
  else
    gout << "** GSM Error: attempted to Peek() the wrong data type; data unchanged\n";
}




//---------------------------------------------------------------------
  // operation functions
//---------------------------------------------------------------------

void GSM::operation( OperationMode mode )
{
  Portion *p2, *p1;
  if( stack->Depth() > 1 )
  {    
    p2 = stack->Pop();
    p1 = stack->Pop();
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
    gout << "** GSM Error: attempted an unsupported operation\n";
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
    gout << "** GSM Error: attempted an unsupported operation\n";
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
    gout << "** GSM Error: attempted an unsupported operation\n";
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
      gout << "** GSM Error: attempted to negate an unknown type\n";
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
    default:
      gout << "** GSM Error: unknown type found in stack\n";
    }
  }
  gout << "\n";
  assert( stack->Depth() == 0 );
}
