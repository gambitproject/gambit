//#
//# FILE: portion.h -- implementation of Portion base and descendent classes
//#                    companion to GSM
//#
//# $Id$
//#




#include "portion.h"



//--------------------------------------------------------------------
// implementation of Portion base and descendent classes
//--------------------------------------------------------------------

//--------------------------- base type -------------------------------


// variable used to detect memory leakage
// int Portion::num_of_Portions = 0;

Portion::Portion()
{
  // The following two lines are for detecting memory leakage.
  // num_of_Portions++;
  // gout << "Portions:" << num_of_Portions << "\n";
}


Portion::~Portion()
{
  // The following two lines are for detecting memory leakage.
  // num_of_Portions--;
  // gout << "Portions:" << num_of_Portions << "\n";
}


bool Portion::Operation( Portion *p, OperationMode mode )
{
  gerr << "Portion Error: attempted to execute an unsupported operation\n";
  assert(0);
  return false;
}


//-----------------------------------------------------------------------
//                        numerical type 
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

template <class T> Portion* numerical_Portion<T>::Copy( void ) const
{ return new numerical_Portion<T>( value ); }


template <class T> bool numerical_Portion<T>::Operation
  ( 
   Portion* p, 
   OperationMode mode 
   )
{
  bool  result = true;
  T&    p_value = ( (numerical_Portion<T>*) p )->Value();

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
  switch( Type() )
  {
  case porDOUBLE:
    s << " (double) ";
    break;
  case porINTEGER:
    s << " (gInteger) ";
    break;
  case porRATIONAL:
    s << " (gRational) ";
    break;
  default:
    s << " (unknown numerical) ";
  }
  s << value << "\n";
}




//---------------------------------------------------------------------
//                            bool type
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

Portion* bool_Portion::Copy( void ) const
{ return new bool_Portion( value ); }


bool bool_Portion::Operation( Portion* p, OperationMode mode )
{
  bool   result = true;
  bool&  p_value = ( (bool_Portion*) p )->Value();

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
  s << " (bool) ";
  if( value == true )
    s << "true\n";
  else
    s << "false\n";
}



//---------------------------------------------------------------------
//                          gString type
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

Portion* gString_Portion::Copy( void ) const
{ return new gString_Portion( value ); }


bool gString_Portion::Operation( Portion* p, OperationMode mode )
{
  bool      result = 0;
  gString&  p_value = ( (gString_Portion*) p )->Value();

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
  s << " (gString) " << value << "\n";
}




//---------------------------------------------------------------------
//                        Reference type
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

Portion* Reference_Portion::Copy( void ) const
{ return new Reference_Portion( value ); }

void Reference_Portion::Output( gOutput& s ) const
{
  s << " (Reference) " << value << "\n";
}




//---------------------------------------------------------------------
//                             List type
//---------------------------------------------------------------------
List_Portion::List_Portion( void )
{
  data_type = porERROR;
}


List_Portion::List_Portion( const gBlock<Portion*>& new_value )
     : value( new_value )
{
  int i;
  int length;
  int type_match;

  data_type = new_value[ 1 ]->Type();
  for( i = 2, length = new_value.Length(); i <= length; i++ )
  {
    type_match = TypeCheck( new_value[ i ] );
    if( !type_match )
    {
      gerr << "Portion Error: attempted to initialize a List_Portion with a\n";
      gerr << "               gBlock<Portion*> that contains mixed types\n";
      assert(0);
    }
  }
}


List_Portion::~List_Portion()
{
  Flush();
}


gBlock<Portion*> List_Portion::Value( void ) const
{ return value; }

gBlock<Portion*>& List_Portion::Value( void )
{ return value; }

PortionType List_Portion::Type( void ) const
{ return porLIST; }

PortionType List_Portion::DataType( void ) const
{ return data_type; }

Portion* List_Portion::Copy( void ) const
{ return new List_Portion( value ); }


int List_Portion::TypeCheck( Portion* item )
{
  int result = false;

  if( item->Type() == data_type )
  {
    result = true;
  }
  else if( item->Type() == porLIST )
  {
    if( ( (List_Portion*) item )->data_type == data_type )
      result = true;
  }
  return result;
}


bool List_Portion::Operation( Portion* p, OperationMode mode )
{
  bool               result = true;
  gBlock<Portion*>&  p_value = ( (List_Portion*) p )->Value();

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
  int i;
  int length;

  s << " (List)\n"; 
  s << "  {\n";
  for( i = 1, length = value.Length(); i <= length; i++ )
  {
    s << "    list item " << i << " : ";
    value[i]->Output( s );
  }
  s << "  }\n";
}


Portion* List_Portion::operator[] ( int index )
{
  return value[ index ];
}


int List_Portion::Append( Portion* item )
{
  return Insert( item, value.Length() + 1 );
}


int List_Portion::Insert( Portion* item, int index )
{
  int result;
  int type_match;

  if( item->Type() == porREFERENCE )
  {
    gerr << "Portion Error: attempted to insert a Reference into a List_Portion\n";
    assert(0);
  }

  if( value.Length() == 0 )  // creating a new list
  {
    if( item->Type() == porLIST )
      data_type = ( (List_Portion*) item )->data_type;
    else
      data_type = item->Type();
  }
  else  // inserting into an existing list
  {
    type_match = TypeCheck( item );
    if( !type_match )
    {
      gerr << "Portion Error: attempted to insert conflicting Portion types\n";
      gerr << "               into a List_Portion.\n";
      assert(0);
    }
  }
  result = value.Insert( item, index );
  return result;
}


Portion* List_Portion::Remove( int index )
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
//                         class instantiations
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

TEMPLATE class gBlock<Portion*>;
