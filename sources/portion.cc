//#
//# FILE: portion.h -- implementation of Portion base and descendent classes
//#                    companion to GSM
//#
//# $Id$
//#




#include <assert.h>
#include "basic.h"
#include "gambitio.h"


#include "portion.h"
#include "gsmhash.h"




//---------------------------------------------------------------------
//                          base class
//---------------------------------------------------------------------


// variable used to detect memory leakage
#ifdef MEMCHECK
int Portion::_NumPortions = 0;
#endif

Portion::Portion()
{
  // The following two lines are for detecting memory leakage.
#ifdef MEMCHECK
  _NumPortions++;
  gout << ">>> Portion ctor -- count: " << _NumPortions << "\n";
#endif
}


Portion::~Portion()
{
  // The following two lines are for detecting memory leakage.
#ifdef MEMCHECK
  _NumPortions--;
  gout << ">>> Portion dtor -- count: " << _NumPortions << "\n";
#endif
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
template <class T> numerical_Portion<T>::numerical_Portion( const T& value )
     : _Value( value )
{ }


template <class T> T numerical_Portion<T>::Value( void ) const
{ return _Value; }

template <class T> T& numerical_Portion<T>::Value( void )
{ return _Value; }

template <class T> PortionType numerical_Portion<T>::Type( void ) const
{ return porERROR; }

template <class T> Portion* numerical_Portion<T>::Copy( void ) const
{ return new numerical_Portion<T>( _Value ); }


template <class T> bool numerical_Portion<T>::Operation
  ( 
   Portion* p, 
   OperationMode mode 
   )
{
  bool  result = true;
  T&    p_value = ( (numerical_Portion<T>*) p )->_Value;

  if( p == 0 )      // unary operations
  {
    switch( mode )
    {
    case opNEGATE:
      _Value = - _Value;
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
      _Value += p_value;
      break;
    case opSUBTRACT:
      _Value -= p_value;
      break;
    case opMULTIPLY:
      _Value *= p_value;
      break;
    case opDIVIDE:
      _Value /= p_value;
      break;
    case opEQUAL_TO:
      result = ( _Value == p_value );
      break;
    case opNOT_EQUAL_TO:
      result = ( _Value != p_value );
      break;
    case opGREATER_THAN:
      result = ( _Value > p_value );
      break;
    case opLESS_THAN:
      result = ( _Value < p_value );
      break;
    case opGREATER_THAN_OR_EQUAL_TO:
      result = ( _Value >= p_value );
      break;
    case opLESS_THAN_OR_EQUAL_TO:
      result = ( _Value <= p_value );
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
  s << _Value << "\n";
}




//---------------------------------------------------------------------
//                            bool type
//---------------------------------------------------------------------
bool_Portion::bool_Portion( const bool& value ) 
     : _Value( value )
{ }


bool bool_Portion::Value( void ) const
{ return _Value; }

bool& bool_Portion::Value( void )
{ return _Value; }

PortionType bool_Portion::Type( void ) const
{ return porBOOL; }

Portion* bool_Portion::Copy( void ) const
{ return new bool_Portion( _Value ); }


bool bool_Portion::Operation( Portion* p, OperationMode mode )
{
  bool   result = true;
  bool&  p_value = ( (bool_Portion*) p )->_Value;

  if( p == 0 )      // unary operations
  {
    switch( mode )
    {
    case opLOGICAL_NOT:
      _Value = ! _Value;
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
      _Value = ( _Value && p_value );
      break;
    case opLOGICAL_OR:
      _Value = ( _Value || p_value );
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
  if( _Value == true )
    s << "true\n";
  else
    s << "false\n";
}



//---------------------------------------------------------------------
//                          gString type
//---------------------------------------------------------------------
gString_Portion::gString_Portion( const gString& value )
     : _Value( value )
{ }


gString gString_Portion::Value( void ) const
{ return _Value; }

gString& gString_Portion::Value( void )
{ return _Value; }

PortionType gString_Portion::Type( void ) const
{ return porSTRING; }

Portion* gString_Portion::Copy( void ) const
{ return new gString_Portion( _Value ); }


bool gString_Portion::Operation( Portion* p, OperationMode mode )
{
  bool      result = true;
  gString&  p_value = ( (gString_Portion*) p )->_Value;

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
      _Value += p_value;
      break;
    case opEQUAL_TO:
      result = ( _Value == p_value );
      break;
    case opNOT_EQUAL_TO:
      result = ( _Value != p_value );
      break;
    case opGREATER_THAN:
      result = ( _Value > p_value );
      break;
    case opLESS_THAN:
      result = ( _Value < p_value );
      break;
    case opGREATER_THAN_OR_EQUAL_TO:
      result = ( _Value >= p_value );
      break;
    case opLESS_THAN_OR_EQUAL_TO:
      result = ( _Value <= p_value );
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
  s << " (gString) " << _Value << "\n";
}




//---------------------------------------------------------------------
//                        Reference type
//---------------------------------------------------------------------
Reference_Portion::Reference_Portion( const gString& value )
     : _Value( value ), _SubValue( "" )
{ }

Reference_Portion::Reference_Portion( const gString& value, 
				     const gString& subvalue )
     : _Value( value ), _SubValue( subvalue )
{ }


gString& Reference_Portion::Value( void )
{ return _Value; }

gString Reference_Portion::Value( void ) const
{ return _Value; }

gString& Reference_Portion::SubValue( void )
{ return _SubValue; }

gString Reference_Portion::SubValue( void ) const
{ return _SubValue; }

PortionType Reference_Portion::Type( void ) const
{ return porREFERENCE; }

Portion* Reference_Portion::Copy( void ) const
{ return new Reference_Portion( _Value ); }

void Reference_Portion::Output( gOutput& s ) const
{
  s << " (Reference) " << _Value << "\n";
}




//---------------------------------------------------------------------
//                             List type
//---------------------------------------------------------------------
List_Portion::List_Portion( void )
{
  _DataType = porERROR;
}


List_Portion::List_Portion( const gBlock<Portion*>& value )
{
  int i;
  int length;
  int type_match;

  for( i = 1, length = value.Length(); i <= length; i++ )
  {
    Insert( value[ i ]->Copy(), i );
  }
}


List_Portion::~List_Portion()
{
  Flush();
}


gBlock<Portion*> List_Portion::Value( void ) const
{ return _Value; }

gBlock<Portion*>& List_Portion::Value( void )
{ return _Value; }

PortionType List_Portion::Type( void ) const
{ return porLIST; }

PortionType List_Portion::DataType( void ) const
{ return _DataType; }

Portion* List_Portion::Copy( void ) const
{ return new List_Portion( _Value ); }


int List_Portion::TypeCheck( Portion* item )
{
  int result = false;

  if( item->Type() == _DataType )
  {
    result = true;
  }
  else if( item->Type() == porLIST )
  {
    if( ( (List_Portion*) item )->_DataType == _DataType )
      result = true;
  }
  return result;
}


bool List_Portion::Operation( Portion* p, OperationMode mode )
{
  bool               result = true;
  gBlock<Portion*>&  p_value = ( (List_Portion*) p )->_Value;

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
  for( i = 1, length = _Value.Length(); i <= length; i++ )
  {
    s << "    list item " << i << " : ";
    _Value[i]->Output( s );
  }
  s << "  }\n";
}


Portion* List_Portion::operator[] ( int index )
{
  return _Value[ index ];
}


int List_Portion::Append( Portion* item )
{
  return Insert( item, _Value.Length() + 1 );
}


int List_Portion::Insert( Portion* item, int index )
{
  int result;
  int type_match;

  if( item->Type() == porREFERENCE )
  {
    gerr << "Portion Error: attempted to insert a Reference_Portion into\n";
    gerr << "               a List_Portion\n";
    assert(0);
  }

  if( _Value.Length() == 0 )  // creating a new list
  {
    if( item->Type() == porLIST )
      _DataType = ( (List_Portion*) item )->_DataType;
    else
      _DataType = item->Type();
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
  result = _Value.Insert( item, index );
  return result;
}


Portion* List_Portion::Remove( int index )
{
  return _Value.Remove( index );
}


int List_Portion::Length( void ) const
{
  return _Value.Length();
}


void List_Portion::Flush( void )
{
  int i, length;
  for( i = 1, length = _Value.Length(); i <= length; i++ )
  {
    delete _Value[ i ];
  }
  _Value.Flush();
}



//---------------------------------------------------------------------
//                            Nfg type
//---------------------------------------------------------------------


Nfg_Portion::Nfg_Portion( const double& value )
{
  _RefTable = new RefHashTable;
  _Value = new Nfg;
  _Value->value = value;
}

Nfg_Portion::Nfg_Portion( Nfg& value )
{
  _RefTable = new RefHashTable;
  _Value = new Nfg;
  _Value = &value;
}

Nfg_Portion::Nfg_Portion( const Nfg_Portion& value )
     : _Value( value._Value )
{
  _RefTable = new RefHashTable;
  _Value->refs++;
}

Nfg_Portion::~Nfg_Portion()
{
  delete _RefTable;
  _Value->refs--;

#ifndef NDEBUG
  if( _Value->refs < 0 )
    gerr << "Nfg_Portion Critical Error\n";
  assert( _Value->refs >= 0 );
#endif // NDEBUG

  if( _Value->refs == 0 )
    delete _Value;
}


Nfg Nfg_Portion::Value( void ) const
{ return *_Value; }

Nfg& Nfg_Portion::Value( void )
{ return *_Value; }

PortionType Nfg_Portion::Type( void ) const
{ return porNFG; }

Portion* Nfg_Portion::Copy( void ) const
{ return new Nfg_Portion( *this ); }


bool Nfg_Portion::Operation( Portion* p, OperationMode mode )
{
  bool   result = true;
  Nfg&   p_value = *( ( (Nfg_Portion*) p )->_Value );

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


void Nfg_Portion::Output( gOutput& s ) const
{
  s << " (Nfg) " << _Value->value;
#ifdef MEMCHECK
  s << ", refs: " << _Value->refs;
#endif
  s << "\n";
}



//---------------------------------------------------------------------
//     Assign() and UnAssign() for Nfg_Portion
//---------------------------------------------------------------------

bool Nfg_Portion::Assign( const gString& ref, Portion *p )
{
  assert( p->Type() != porREFERENCE );
  
  _RefTable->Define( ref, p );
  
  return true;
}


bool Nfg_Portion::UnAssign( const gString& ref )
{
  if( _RefTable->IsDefined( ref ) )
  {
    _RefTable->Remove( ref );
    return true;
  }
  else
  {
    return false;
  }
}


Portion* Nfg_Portion::operator()( const gString& ref ) const
{
  Portion* result = 0;

#ifndef NDEBUG
  if( !_RefTable->IsDefined( ref ) )
  {
    gerr << "Portion Error: attempted to access an undefined reference\n";
    gerr << "               \"" << ref << "\"\n";
  }
  assert( _RefTable->IsDefined( ref ) );
#endif // NDEBUG

  result = (*_RefTable)( ref );
  return result;
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




#include "hash.imp"

TEMPLATE class HashTable< gString, Portion* >;



#include "gblock.imp"

TEMPLATE class gBlock<Portion*>;
