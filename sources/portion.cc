//#
//# FILE: portion.h -- implementation of Portion base and descendent classes
//#                    companion to GSM
//#
//# $Id$
//#




#include <assert.h>
#include "basic.h"


#include "portion.h"
#include "gsmhash.h"




//---------------------------------------------------------------------
//                          base class
//---------------------------------------------------------------------


// variable used to detect memory leakage
#ifdef MEMCHECK
int Portion::_NumPortions = 0;
#endif // MEMCHECK


Portion::Portion()
{
  _Temporary = true;

  // The following two lines are for detecting memory leakage.
#ifdef MEMCHECK
  _NumPortions++;
  gout << ">>> Portion ctor -- count: " << _NumPortions << "\n";
#endif // MEMCHECK
}


Portion::~Portion()
{
  // The following two lines are for detecting memory leakage.
#ifdef MEMCHECK
  _NumPortions--;
  gout << ">>> Portion dtor -- count: " << _NumPortions << "\n";
#endif // MEMCHECK
}


bool& Portion::Temporary( void )
{ return _Temporary; }


void Portion::MakeCopyOfData( Portion* p )
{ 
#ifndef NDEBUG
  if( this->Type() != p->Type() )
  {
    gerr << "Portion Error: attempting to MakeCopyOfData() a different\n";
    gerr << "               Portion type\n";
  }
  assert( this->Type() == p->Type() );
#endif // NDEBUG
}


bool Portion::Operation( Portion* p, OperationMode mode )
{
  gerr << "Portion Error: attempted to execute an unsupported operation\n";
  assert(0);
  return false;
}





//-----------------------------------------------------------------------
//                          error type 
//-----------------------------------------------------------------------

PortionType Error_Portion::Type( void ) const
{ return porERROR; }

Portion* Error_Portion::Copy( void ) const
{ return new Error_Portion; }

void Error_Portion::Output( gOutput& s ) const
{ s << " Error"; }


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


template <class T> 
  bool numerical_Portion<T>::Operation( Portion* p, OperationMode mode )
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

    case opINTEGER_DIVIDE:
      if( Type() == porINTEGER )
      {
	_Value /= p_value;
      }
      else
      {
	result = Portion::Operation( p, mode );
      }
      break;
    case opMODULUS:
      if( Type() == porINTEGER )
      {
	// This is coded as is because the compiler complains when 
	// instantiating for double and gRational types 
        // if the % operator is used.  This version is about as fast as
        // the original C operator %.
	_Value = _Value - _Value / p_value * p_value;
      }
      else
      {
	result = Portion::Operation( p, mode );
      }
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
  s << " " << _Value;
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
    case opEQUAL_TO:
      result = ( _Value == p_value );
      break;
    case opNOT_EQUAL_TO:
      result = ( _Value != p_value );
      break;

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
  s << "(bool) ";
  if( _Value == true )
    s << "true";
  else
    s << "false";
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
    case opADD:
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
  s << " \"" << _Value << "\"";
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
  s << "(Reference) \"" << _Value << "\", \"" << _SubValue << "\"";
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
  int length = _Value.Length();

  if( length >= 1 )
  {
    gout << " {" << _Value[ 1 ];
  }
  for( i = 2; i <= length; i++ )
  {
    gout << "," << _Value[ i ];
  }
  s << " }";
}


Portion* List_Portion::operator[] ( int index )
{ return _Value[ index ]; }

int List_Portion::Append( Portion* item )
{ return Insert( item, _Value.Length() + 1 ); }


int List_Portion::Insert( Portion* item, int index )
{
  int result = 0;
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
{ return _Value.Remove( index ); }

int List_Portion::Length( void ) const
{ return _Value.Length(); }


void List_Portion::Flush( void )
{
  int i, length;
  for( i = 1, length = _Value.Length(); i <= length; i++ )
  {
    delete _Value[ i ];
  }
  _Value.Flush();
}


Portion* List_Portion::Subscript( Portion* p ) const
{
  long i;
  Portion* result;

  if( p->Type() == porINTEGER )
  {
    i = (int) ( (numerical_Portion<gInteger>*) p )->Value().as_long();
    result = _Value[ i ]->Copy();
  }
  else
  {
    gerr << "List_Portion Error: a non-Integer subscript specified\n";
    result = new Error_Portion;
  }
  delete p;
  return result;
}

//---------------------------------------------------------------------
//                            Nfg type
//---------------------------------------------------------------------


Nfg_Portion::Nfg_Portion( Nfg& value )
{
  _RefTable = new RefHashTable;
  _Value = &value;
}


Nfg_Portion::~Nfg_Portion()
{
  delete _RefTable;

  if( !_Temporary )
    delete _Value;
}


Nfg& Nfg_Portion::Value( void )
{ return *_Value; }

PortionType Nfg_Portion::Type( void ) const
{ return porNFG; }

Portion* Nfg_Portion::Copy( void ) const
{ return new Nfg_Portion( *_Value ); }


void Nfg_Portion::MakeCopyOfData( Portion* p )
{
  Portion::MakeCopyOfData( p );
  _Value = new Nfg;
  _Value->value = ( (Nfg_Portion*) p )->_Value->value;
}


bool Nfg_Portion::Operation( Portion* p, OperationMode mode )
{
  bool  result = true;
  Nfg&  p_value = *( ( (Nfg_Portion*) p )->_Value );

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
{ s << "(Nfg) " << _Value->value; }



//---------------------------------------------------------------------
//     Assign() and UnAssign() for Nfg_Portion
//---------------------------------------------------------------------

bool Nfg_Portion::Assign( const gString& ref, Portion *p )
{
#ifndef NDEBUG
  if( p->Type() == porREFERENCE )
  {
    gerr << "Portion Error: attempted to Assign a Reference_Portion as the\n";
    gerr << "               value of a sub-reference in a Nfg_Portion type\n";
  }
  assert( p->Type() != porREFERENCE );
#endif // NDEBUG

  _RefTable->Define( ref, p );

  return true;
}


bool Nfg_Portion::UnAssign( const gString& ref )
{
  if( _RefTable->IsDefined( ref ) )
  {
    _RefTable->Remove( ref );
  }
#ifndef NDEBUG
  else
  {
    gerr << "GSM Warning: calling UnAssign() on a undefined reference\n";
  }
#endif // NDEBUG

  return true;
}


Portion* Nfg_Portion::operator()( const gString& ref ) const
{
  Portion* result = 0;

  if( _RefTable->IsDefined( ref ) )
  {
    result = (*_RefTable)( ref );
  }
  else
  {
    gerr << "Portion Error: attempted to access an undefined reference\n";
    gerr << "               \"" << ref << "\"\n";
  }

  return result;
}






//--------------------------------------------------------------------
//             miscellaneous PortionType functions
//--------------------------------------------------------------------

void PrintPortionTypeSpec( gOutput& s, PortionType type )
{
  if( type & porBOOL )
    s << "porBOOL ";
  if( type & porDOUBLE )
    s << "porDOUBLE ";
  if( type & porINTEGER )
    s << "porINTEGER ";
  if( type & porRATIONAL )
    s << "porRATIONAL ";
  if( type & porSTRING )
    s << "porSTRING ";
  if( type & porLIST )
    s << "porLIST ";
  if( type & porNFG )
    s << "porNFG ";
  if( type & porREFERENCE )
    s << "porREFERENCE ";

  s << "\n";
}







//----------------------------------------------------------------------
//                         class instantiations
//----------------------------------------------------------------------



gOutput& operator << ( gOutput& s, Portion* p )
{
  p->Output( s );
  return s;
}




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
