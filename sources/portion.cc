//
//  FILE: portion.cc  implementation ofPortion base and descendent classes
//                    companion to gsm.cc
//




#include "portion.h"



#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__



//--------------------------------------------------------------------
// implementation of Portion base and descendent classes
//--------------------------------------------------------------------

//--------------------------- base type -------------------------------
int Portion::Operation( Portion *p, OperationMode mode )
{
  gerr << "** Portion Error: attempted to execute an unsupported operation\n";
  assert(0);
  return 0;
}


//-----------------------------------------------------------------------
//------------------------- numerical type ------------------------------
//-----------------------------------------------------------------------
template <class T> 
  numerical_Portion<T>::numerical_Portion( const T& new_value )
     : value( new_value )
{
}

template <class T> T numerical_Portion<T>::Value( void ) const
{ return value; }
template <class T> T& numerical_Portion<T>::Value( void )
{ return value; }
template <class T> PortionType numerical_Portion<T>::Type( void ) const
{ return porERROR; }
template <class T> Portion *numerical_Portion<T>::Copy( void ) const
{ return new numerical_Portion<T>( value ); }

template <class T> 
  int numerical_Portion<T>::
  Operation( Portion *p, OperationMode mode )
{
  int result = 0;
  T& p_value = ( (numerical_Portion<T> *)p )->Value();

  if( p == 0 )      // unary operations
  {
    switch( mode )
    {
    case opNEGATE:
      value = -value;
      break;
    default:
      result = Portion::Operation( p, mode );      
    }
  }
  else               // binary operations
  {
    switch( mode )
    {
    case opADD:
      value += p_value;
      delete p;
      break;
    case opSUBTRACT:
      value -= p_value;
      delete p;
      break;
    case opMULTIPLY:
      value *= p_value;
      delete p;
      break;
    case opDIVIDE:
      value /= p_value;
      delete p;
      break;
    case opEQUAL_TO:
      result = ( value == p_value );
      break;
    case opNOT_EQUAL_TO:
      result = ( value != p_value );
      break;
    case opGREATER_THAN:
      result = ( value > p_value );
      break;
    case opLESS_THAN:
      result = ( value < p_value );
      break;
    case opGREATER_THAN_OR_EQUAL_TO:
      result = ( value >= p_value );
      break;
    case opLESS_THAN_OR_EQUAL_TO:
      result = ( value <= p_value );
      break;
    default:
      result = Portion::Operation( p, mode );
    }
  }
  return result;
}


template <class T> void numerical_Portion<T>::Output( gOutput& s ) const
{
  s << value;
  switch( Type() )
  {
  case porDOUBLE:
    s << "     type: double\n";
    break;
  case porINTEGER:
    s << "     type: gInteger\n";
    break;
  case porRATIONAL:
    s << "     type: gRational\n";
    break;
  default:
    s << "     type: unknown numerical\n";
  }
}


//---------------------- numerical type subtypes ----------------------------

TEMPLATE class numerical_Portion<double>;
PortionType numerical_Portion<double>::Type( void ) const
{ return porDOUBLE; }

TEMPLATE class numerical_Portion<gInteger>;
PortionType numerical_Portion<gInteger>::Type( void ) const
{ return porINTEGER; }

TEMPLATE class numerical_Portion<gRational>;
PortionType numerical_Portion<gRational>::Type( void ) const
{ return porRATIONAL; }



//---------------------------------------------------------------------
//------------------------- bool type ------------------------------
//---------------------------------------------------------------------
bool_Portion::bool_Portion( const bool& new_value )
     : value( new_value )
{
}

bool bool_Portion::Value( void ) const
{ return value; }
bool& bool_Portion::Value( void )
{ return value; }
PortionType bool_Portion::Type( void ) const
{ return porBOOL; }
Portion *bool_Portion::Copy( void ) const
{ return new bool_Portion( value ); }

int bool_Portion:: Operation( Portion *p, OperationMode mode )
{
  int result = 0;
  bool& p_value = ( (bool_Portion *)p )->Value();

  if( p == 0 )      // unary operations
  {
    switch( mode )
    {
    case opLOGICAL_NOT:
      value = !value;
      break;
    default:
      result = Portion::Operation( p, mode );      
    }
  }
  else               // binary operations
  {
    switch( mode )
    {
    case opLOGICAL_AND:
      value = ( value && p_value );
      break;
    case opLOGICAL_OR:
      value = ( value || p_value );
      break;
    default:
      result = Portion::Operation( p, mode );
    }
  }
  return result;
}

void bool_Portion::Output( gOutput& s ) const
{
  s << value << "     type: bool\n";
}



//---------------------------------------------------------------------
//------------------------- gString type ------------------------------
//---------------------------------------------------------------------
gString_Portion::gString_Portion( const gString& new_value )
     : value( new_value )
{
}

gString gString_Portion::Value( void ) const
{ return value; }
gString& gString_Portion::Value( void )
{ return value; }
PortionType gString_Portion::Type( void ) const
{ return porSTRING; }
Portion *gString_Portion::Copy( void ) const
{ return new gString_Portion( value ); }

int gString_Portion:: Operation( Portion *p, OperationMode mode )
{
  int result = 0;
  gString& p_value = ( (gString_Portion *)p )->Value();

  if( p == 0 )      // unary operations
  {
    switch( mode )
    {
    default:
      result = Portion::Operation( p, mode );      
    }
  }
  else               // binary operations
  {
    switch( mode )
    {
    case opCONCATENATE:
      value += p_value;
      delete p;
      break;
    case opEQUAL_TO:
      result = ( value == p_value );
      break;
    case opNOT_EQUAL_TO:
      result = ( value != p_value );
      break;
    case opGREATER_THAN:
      result = ( value > p_value );
      break;
    case opLESS_THAN:
      result = ( value < p_value );
      break;
    case opGREATER_THAN_OR_EQUAL_TO:
      result = ( value >= p_value );
      break;
    case opLESS_THAN_OR_EQUAL_TO:
      result = ( value <= p_value );
      break;
    default:
      result = Portion::Operation( p, mode );
    }
  }
  return result;
}

void gString_Portion::Output( gOutput& s ) const
{
  s << value << "     type: gString\n";
}




//---------------------------------------------------------------------
//------------------------- Reference type ------------------------------
//---------------------------------------------------------------------
Reference_Portion::Reference_Portion( const gString& new_value )
     : value( new_value )
{
}

gString Reference_Portion::Value( void ) const
{ return value; }
gString& Reference_Portion::Value( void )
{ return value; }
PortionType Reference_Portion::Type( void ) const
{ return porREFERENCE; }
Portion *Reference_Portion::Copy( void ) const
{ return new Reference_Portion( value ); }

void Reference_Portion::Output( gOutput& s ) const
{
  s << value << "     type: Reference\n";
}


