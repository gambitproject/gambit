//
//  FILE: portion.cc  implementation of Portion base and descendent classes
//                    companion to gsm.cc
//




#include "portion.h"



//--------------------------------------------------------------------
// implementation of Portion base and descendent classes
//--------------------------------------------------------------------

//--------------------------- base type -------------------------------

Portion::~Portion()
{
}


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
      break;
    case opSUBTRACT:
      value -= p_value;
      break;
    case opMULTIPLY:
      value *= p_value;
      break;
    case opDIVIDE:
      value /= p_value;
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
    delete p;
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
    delete p;
  }
  return result;
}

void bool_Portion::Output( gOutput& s ) const
{
  if( value == true )
    s << "true";
  else
    s << "false";
  s << "     type: bool\n";
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
    delete p;
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





//---------------------------------------------------------------------
//------------------------- List type ------------------------------
//---------------------------------------------------------------------
List_Portion::List_Portion( void )
{
  data_type = porERROR;
}

List_Portion::List_Portion( const gBlock<Portion *>& new_value )
     : value( new_value )
{
  int i, length;
  int type_match;

  data_type = new_value[ 1 ]->Type();
  for( i = 2, length = new_value.Length(); i <= length; i++ )
  {
    type_match = TypeCheck( new_value[ i ] );
    if( !type_match )
    {
      gerr << "** Portion Error: attempted to initialize a List_Portion with a\n";
      gerr << "                  gBlock<Portion *> that contains mixed types\n";
      assert(0);
    }
  }
}

List_Portion::~List_Portion()
{
  Flush();
}

gBlock<Portion *> List_Portion::Value( void ) const
{ return value; }
gBlock<Portion *>& List_Portion::Value( void )
{ return value; }
PortionType List_Portion::Type( void ) const
{ return porLIST; }
PortionType List_Portion::DataType( void ) const
{ return data_type; }
Portion *List_Portion::Copy( void ) const
{ return new List_Portion( value ); }


int List_Portion::TypeCheck( Portion *item )
{
  int result = false;
  if( item->Type() == data_type )
    result = true;
  else 
    if( item->Type() == porLIST )
      if( ( (List_Portion *)item )->data_type == data_type )
	result = true;
  return result;
}




int List_Portion:: Operation( Portion *p, OperationMode mode )
{
  int result = 0;
  gBlock<Portion *>& p_value = ( (List_Portion *)p )->Value();

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
    default:
      result = Portion::Operation( p, mode );
    }
    delete p;
  }
  return result;
}

void List_Portion::Output( gOutput& s ) const
{
  int i, length;

  s << "type: List\n";
  for( i = 1, length = value.Length(); i <= length; i++ )
  {
    s << "    list item " << i << " : ";
    value[i]->Output( s );
  }
}

Portion *List_Portion::operator[] ( int index )
{
  return value[ index ];
}

int List_Portion::Append( Portion *item )
{
  return Insert( item, value.Length() + 1 );
}

int List_Portion::Insert( Portion *item, int index )
{
  int result;
  int type_match;

  if( value.Length() > 0 )
  {
    type_match = TypeCheck( item );
    if( !type_match )
    {
      gerr << "** Portion Error: attempted to insert a conflicting Portion type\n";
      gerr << "                  into a List_Portion.\n";
      assert(0);
    }
  }
  else
  {
    if( item->Type() != porLIST )
      data_type = item->Type();
    else
      data_type = ( (List_Portion *)item )->data_type;
  }
  
  if( item->Type() != porREFERENCE )
    result = value.Insert( item, index );
  else
  {
    gerr << "** Portion Error: attempted to insert a Reference type into List_Portion\n";
    assert(0);
  }
  return result;
}

Portion *List_Portion::Remove( int index )
{
  return value.Remove( index );
}

int List_Portion::Length( void ) const
{
  return value.Length();
}

void List_Portion::Flush( void )
{
  int i, length;
  for( i = 1, length = value.Length(); i <= length; i++ )
  {
    delete value[ i ];
  }
  value.Flush();
}


//----------------------------------------------------------------------
  // class instantiations
//----------------------------------------------------------------------




#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__



TEMPLATE class numerical_Portion<double>;
PortionType numerical_Portion<double>::Type( void ) const
{ return porDOUBLE; }

TEMPLATE class numerical_Portion<gInteger>;
PortionType numerical_Portion<gInteger>::Type( void ) const
{ return porINTEGER; }

TEMPLATE class numerical_Portion<gRational>;
PortionType numerical_Portion<gRational>::Type( void ) const
{ return porRATIONAL; }



#include "gblock.imp"

TEMPLATE class gBlock<Portion *>;
