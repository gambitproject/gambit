//#
//# FILE: portion.cc -- implementation of Portion base and descendent classes
//#                     companion to GSM
//#
//# $Id$
//#


#include <assert.h>
#include <stdio.h>
#include "basic.h"

#include "portion.h"
#include "gsm.h"
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
  _ShadowOf = 0;

  // The following two lines are for detecting memory leakage.
#ifdef MEMCHECK
  _NumPortions++;
  gout << ">>> Portion ctor - count: " << _NumPortions << "\n";
#endif // MEMCHECK
}


Portion::~Portion()
{
  _ShadowOf = 0;

  // The following two lines are for detecting memory leakage.
#ifdef MEMCHECK
  _NumPortions--;
  gout << ">>> Portion dtor - count: " << _NumPortions << "\n";
#endif // MEMCHECK
}



Portion*& Portion::ShadowOf( void )
{ return _ShadowOf; }


List_Portion*& Portion::ParentList( void )
{ return _ParentList; }


Portion* Portion::Operation( Portion* p, OperationMode mode )
{
  if( p !=0 )
  {
    delete p;
  }
  return new Error_Portion( _ErrorMessage( 1 ) );
}





//-----------------------------------------------------------------------
//                          error type 
//-----------------------------------------------------------------------



Error_Portion::Error_Portion( const gString& value )
     : _Value( value )
{ }


gString Error_Portion::Value( void ) const
{ return _Value; }

gString& Error_Portion::Value( void )
{ return _Value; }

PortionType Error_Portion::Type( void ) const
{ return porERROR; }

Portion* Error_Portion::Copy( bool new_data ) const
{ return new Error_Portion( _Value ); }

void Error_Portion::Output( gOutput& s ) const
{
  if( _Value == "" )
    s << " (Error)";
  else
    s << _Value;
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

template <class T> Portion* numerical_Portion<T>::Copy( bool new_data ) const
{ 
  Portion* p;
  p = new numerical_Portion<T>( _Value ); 
  return p;
}


template <class T> 
Portion* numerical_Portion<T>::Operation( Portion* p, OperationMode mode )
{
  Portion*  result = 0;
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
      if( p_value != 0 )
      {
	_Value /= p_value;
      }
      else
      {
	_Value = 0;
	result = new Error_Portion( _ErrorMessage( 2 ) );
      }
      break;

    case opINTEGER_DIVIDE:
      if( p_value != 0 )
      {
	if( Type() == porINTEGER )
	{
	  _Value /= p_value;
	}
	else
	{
	  result = Portion::Operation( p, mode );
	}
      }
      else
      {
	_Value = 0;
	result = new Error_Portion( _ErrorMessage( 2 ) );
      }
      break;

    case opMODULUS:
      if( p_value != 0 )
      {
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
      }
      else
      {
	_Value = 0;
	result = new Error_Portion( _ErrorMessage( 2 ) );
      }
      break;

    case opEQUAL_TO:
      result = new bool_Portion( _Value == p_value );
      break;
    case opNOT_EQUAL_TO:
      result = new bool_Portion( _Value != p_value );
      break;
    case opGREATER_THAN:
      result = new bool_Portion( _Value > p_value );
      break;
    case opLESS_THAN:
      result = new bool_Portion( _Value < p_value );
      break;
    case opGREATER_THAN_OR_EQUAL_TO:
      result = new bool_Portion( _Value >= p_value );
      break;
    case opLESS_THAN_OR_EQUAL_TO:
      result = new bool_Portion( _Value <= p_value );
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

Portion* bool_Portion::Copy( bool new_data ) const
{ return new bool_Portion( _Value ); }


Portion* bool_Portion::Operation( Portion* p, OperationMode mode )
{
  Portion* result = 0;
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
      result = new bool_Portion( _Value == p_value );
      break;
    case opNOT_EQUAL_TO:
      result = new bool_Portion( _Value != p_value );
      break;

    case opLOGICAL_AND:
      _Value = _Value && p_value;
      break;
    case opLOGICAL_OR:
      _Value = _Value || p_value;
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
  if( _Value == true )
    s << " true";
  else
    s << " false";
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

Portion* gString_Portion::Copy( bool new_data ) const
{ return new gString_Portion( _Value ); }


Portion* gString_Portion::Operation( Portion* p, OperationMode mode )
{
  Portion*  result = 0;
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
      result = new bool_Portion( _Value == p_value );
      break;
    case opNOT_EQUAL_TO:
      result = new bool_Portion( _Value != p_value );
      break;
    case opGREATER_THAN:
      result = new bool_Portion( _Value > p_value );
      break;
    case opLESS_THAN:
      result = new bool_Portion( _Value < p_value );
      break;
    case opGREATER_THAN_OR_EQUAL_TO:
      result = new bool_Portion( _Value >= p_value );
      break;
    case opLESS_THAN_OR_EQUAL_TO:
      result = new bool_Portion( _Value <= p_value );
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
//                          MixedProfile type
//---------------------------------------------------------------------

template <class T> 
  Mixed_Portion<T>::Mixed_Portion( const MixedProfile<T>& value )
    : _Value( value )
{
  _Owner = 0;
}


template <class T> bool Mixed_Portion<T>::SetOwner( NormalForm<T>* owner )
{
  if( owner != 0 )
  {
    _Owner = owner;
    return true;
  }
  return false;
}

template <class T> MixedProfile<T> Mixed_Portion<T>::Value( void ) const
{ return _Value; }

template <class T> MixedProfile<T>& Mixed_Portion<T>::Value( void )
{ return _Value; }

template <class T> PortionType Mixed_Portion<T>::Type( void ) const
{ return porMIXED; }

template <class T> Portion* Mixed_Portion<T>::Copy( bool new_data ) const
{ return new Mixed_Portion<T>( _Value ); }

template <class T> void Mixed_Portion<T>::Output( gOutput& s ) const
{
  _Value.Dump( s );
}




//---------------------------------------------------------------------
//                          BehavProfile type
//---------------------------------------------------------------------

template <class T> 
  Behav_Portion<T>::Behav_Portion( const BehavProfile<T>& value )
    : _Value( value )
{
  _Owner = 0;
}


template <class T> bool Behav_Portion<T>::SetOwner( ExtForm<T>* owner )
{
  if( owner != 0 )
  {
    _Owner = owner;
    return true;
  }
  return false;
}

template <class T> BehavProfile<T> Behav_Portion<T>::Value( void ) const
{ return _Value; }

template <class T> BehavProfile<T>& Behav_Portion<T>::Value( void )
{ return _Value; }

template <class T> PortionType Behav_Portion<T>::Type( void ) const
{ return porBEHAV; }

template <class T> Portion* Behav_Portion<T>::Copy( bool new_data ) const
{ return new Behav_Portion<T>( _Value ); }

template <class T> void Behav_Portion<T>::Output( gOutput& s ) const
{
  _Value.Dump( s );
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

Portion* Reference_Portion::Copy( bool new_data ) const
{ return new Reference_Portion( _Value, _SubValue ); }

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
  int result;

  for( i = 1, length = value.Length(); i <= length; i++ )
  {
    result = Insert( value[ i ]->Copy(), i );
    assert( result != 0 );
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

Portion* List_Portion::Copy( bool new_data ) const
{ return new List_Portion( _Value ); }


bool List_Portion::TypeCheck( Portion* item )
{
  bool result = false;

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


void List_Portion::Output( gOutput& s ) const
{
  int i;
  int length = _Value.Length();

  s << " {";
  if( length >= 1 )
  {
    s << _Value[ 1 ];
    for( i = 2; i <= length; i++ )
    {
      s << "," << _Value[ i ];
    }
  }
  else
  {
    s << "empty";
  }
  s << " }";
}



int List_Portion::Append( Portion* item )
{ return Insert( item, _Value.Length() + 1 ); }


int List_Portion::Insert( Portion* item, int index )
{
  int result = 0;
  int type_match;

#ifndef NDEBUG
  if( item->Type() == porREFERENCE )
  {
    gerr << "Portion Error:\n";
    gerr << "  Attempted to insert a Reference_Portion into\n";
    gerr << "  a List_Portion\n";
  }
  assert( item->Type() != porREFERENCE );
#endif
  
  if( _Value.Length() == 0 )  // creating a new list
  {
    if( item->Type() != porERROR )
    {
      if( item->Type() == porLIST )
	_DataType = ( (List_Portion*) item )->_DataType;
      else
	_DataType = item->Type();
      item->ParentList() = this;
      result = _Value.Insert( item, index );
    }
    else
    {
      delete item;
    }
  }
  else  // inserting into an existing list
  {
    type_match = TypeCheck( item );
    if( !type_match )
    {
      delete item;
    }
    else
    {
      item->ParentList() = this;
      result = _Value.Insert( item, index );
    }
  }

  return result;
}


Portion* List_Portion::Remove( int index )
{ 
  Portion* p;
  p = _Value.Remove( index );
  p->ParentList() = 0;
}

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


Portion* List_Portion::SetSubscript( int index, Portion *p )
{
  bool type_match;
  Portion* result = 0;

  type_match = TypeCheck( p );
  if( type_match )
  {
    if( index >= 1 && index <= _Value.Length() )
    {
      delete _Value[ index ];
      p->ParentList() = this;
      _Value[ index ] = p;
    }
    else
    {
      result = new Error_Portion( _ErrorMessage( 6 ) );
    }
  }
  else
  {
    result = new Error_Portion( _ErrorMessage( 7 ) );
    delete p;
  }
  return result;
}


Portion* List_Portion::GetSubscript( int index ) const
{
  if( index >= 1 && index <= _Value.Length() )
  {
    return _Value[ index ];
  }
  else
  {
    return new Error_Portion( _ErrorMessage( 6 ) );
  }
}





//---------------------------------------------------------------------
//                            Nfg type
//---------------------------------------------------------------------

#ifdef MEMCHECK
int Nfg_Portion<double>::_NumObj = 0;
int Nfg_Portion<gRational>::_NumObj = 0;
#endif // MEMCHECK

RefCountHashTable< NormalForm<double>* > Nfg_Portion<double>::_RefCountTable;
RefCountHashTable< NormalForm<gRational>* > Nfg_Portion<gRational>::_RefCountTable;

template <class T> Nfg_Portion<T>::Nfg_Portion( NormalForm<T>& value )
{
  _RefTable = new RefHashTable;
  _Value = &value;
  if( !_RefCountTable.IsDefined( _Value ) )
  {
    _RefCountTable.Define( _Value, 1 );
#ifdef MEMCHECK
    _NumObj++;
    gout << ">>> NormalForm Ctor - count: " << _NumObj << "\n";
#endif // MEMCHECK
  }
  else
  {
    _RefCountTable( _Value )++;
  }
}

template <class T> Nfg_Portion<T>::~Nfg_Portion()
{
  assert( _RefCountTable.IsDefined( _Value ) );
  assert( _RefCountTable( _Value ) > 0 );
  _RefCountTable( _Value )--;
  if( _RefCountTable( _Value ) == 0 )
  {
    _RefCountTable.Remove( _Value );
    delete _Value;
#ifdef MEMCHECK
    _NumObj--;
    gout << ">>> NormalForm Dtor - count: " << _NumObj << "\n";
#endif // MEMCHECK
  }
  delete _RefTable;
}


template <class T> NormalForm<T>& Nfg_Portion<T>::Value( void )
{ return *_Value; }

template <class T> PortionType Nfg_Portion<T>::Type( void ) const
{ return porNFG; }

template <class T> Portion* Nfg_Portion<T>::Copy( bool new_data ) const
{ 
  Portion* p;
  NormalForm<T>* new_value;

  if( new_data )
  {
    new_value = new 
      NormalForm<T>( *_Value );
    p = new Nfg_Portion<T>( *new_value );
  }
  else
  {
    p = new Nfg_Portion<T>( *_Value ); 
  }
  return p;
}


template <class T> void Nfg_Portion<T>::Output( gOutput& s ) const
{ s << "NormalForm[ "; _Value->GetTitle(); s << ']'; }





template <class T> 
  bool Nfg_Portion<T>::Assign( const gString& ref, Portion *p )
{
#ifndef NDEBUG
  if( p->Type() == porREFERENCE )
  {
    gerr << "Portion Error:\n";
    gerr << "  Attempted to Assign a Reference_Portion as the\n";
    gerr << "  value of a sub-reference in a Nfg_Portion type\n";
  }
  assert( p->Type() != porREFERENCE );
#endif // NDEBUG

  _RefTable->Define( ref, p );

  return true;
}


template <class T> bool Nfg_Portion<T>::UnAssign( const gString& ref )
{
  if( _RefTable->IsDefined( ref ) )
  {
    _RefTable->Remove( ref );
  }
  return true;
}


template <class T> bool Nfg_Portion<T>::IsDefined( const gString& ref ) const
{
  return _RefTable->IsDefined( ref );
}


template <class T> 
  Portion* Nfg_Portion<T>::operator()( const gString& ref ) const
{
  Portion* result = 0;

  if( _RefTable->IsDefined( ref ) )
  {
    result = (*_RefTable)( ref );
  }

  return result;
}





//---------------------------------------------------------------------
//                            Efg type
//---------------------------------------------------------------------

#ifdef MEMCHECK
int Efg_Portion<double>::_NumObj = 0;
int Efg_Portion<gRational>::_NumObj = 0;
#endif // MEMCHECK

RefCountHashTable< ExtForm<double>* > Efg_Portion<double>::_RefCountTable;
RefCountHashTable< ExtForm<gRational>* > Efg_Portion<gRational>::_RefCountTable;

template <class T> Efg_Portion<T>::Efg_Portion( ExtForm<T>& value )
{
  _RefTable = new RefHashTable;
  _Value = &value;
  if( !_RefCountTable.IsDefined( _Value ) )
  {
    _RefCountTable.Define( _Value, 1 );
#ifdef MEMCHECK
    _NumObj++;
    gout << ">>> ExtForm Ctor - count: " << _NumObj << "\n";
#endif // MEMCHECK
  }
  else
  {
    _RefCountTable( _Value )++;
  }
}

template <class T> Efg_Portion<T>::~Efg_Portion()
{
  assert( _RefCountTable.IsDefined( _Value ) );
  assert( _RefCountTable( _Value ) > 0 );
  _RefCountTable( _Value )--;
  if( _RefCountTable( _Value ) == 0 )
  {
    _RefCountTable.Remove( _Value );
    delete _Value;
#ifdef MEMCHECK
    _NumObj--;
    gout << ">>> ExtForm Dtor - count: " << _NumObj << "\n";
#endif // MEMCHECK
  }
  delete _RefTable;
}


template <class T> ExtForm<T>& Efg_Portion<T>::Value( void )
{ return *_Value; }

template <class T> PortionType Efg_Portion<T>::Type( void ) const
{ return porEFG; }

template <class T> Portion* Efg_Portion<T>::Copy( bool new_data ) const
{ 
  Portion* p;
  ExtForm<T>* new_value;

  if( new_data )
  {
    new_value = new ExtForm<T>(); // ( *_Value );
    p = new Efg_Portion<T>( *new_value );
  }
  else
  {
    p = new Efg_Portion<T>( *_Value );
  }
  return p;
}


template <class T> void Efg_Portion<T>::Output( gOutput& s ) const
{ s << "ExtForm[ "; _Value->GetTitle(); s << ']'; }



template <class T> 
  bool Efg_Portion<T>::Assign( const gString& ref, Portion *p )
{
#ifndef NDEBUG
  if( p->Type() == porREFERENCE )
  {
    gerr << "Portion Error:\n";
    gerr << "  Attempted to Assign a Reference_Portion as the\n";
    gerr << "  value of a sub-reference in a Efg_Portion type\n";
  }
  assert( p->Type() != porREFERENCE );
#endif // NDEBUG

  _RefTable->Define( ref, p );

  return true;
}


template <class T> bool Efg_Portion<T>::UnAssign( const gString& ref )
{
  if( _RefTable->IsDefined( ref ) )
  {
    _RefTable->Remove( ref );
  }
  return true;
}


template <class T> bool Efg_Portion<T>::IsDefined( const gString& ref ) const
{
  return _RefTable->IsDefined( ref );
}


template <class T> 
  Portion* Efg_Portion<T>::operator()( const gString& ref ) const
{
  Portion* result = 0;

  if( _RefTable->IsDefined( ref ) )
  {
    result = (*_RefTable)( ref );
  }

  return result;
}





//---------------------------------------------------------------------
//                            Outcome type
//---------------------------------------------------------------------
Outcome_Portion::Outcome_Portion( Outcome* value ) 
     : _Value( value )
{ }


Outcome* Outcome_Portion::Value( void ) const
{ return _Value; }

Outcome*& Outcome_Portion::Value( void )
{ return _Value; }

PortionType Outcome_Portion::Type( void ) const
{ return porOUTCOME; }

Portion* Outcome_Portion::Copy( bool new_data ) const
{ return new Outcome_Portion( _Value ); }

void Outcome_Portion::Output( gOutput& s ) const
{
  s << " (Outcome) ";
}



//---------------------------------------------------------------------
//                            Player type
//---------------------------------------------------------------------
Player_Portion::Player_Portion( Player* value ) 
     : _Value( value )
{ }


Player* Player_Portion::Value( void ) const
{ return _Value; }

Player*& Player_Portion::Value( void )
{ return _Value; }

PortionType Player_Portion::Type( void ) const
{ return porPLAYER; }

Portion* Player_Portion::Copy( bool new_data ) const
{ return new Player_Portion( _Value ); }

void Player_Portion::Output( gOutput& s ) const
{
  s << " (Player) ";
}



//---------------------------------------------------------------------
//                            Infoset type
//---------------------------------------------------------------------
Infoset_Portion::Infoset_Portion( Infoset* value ) 
     : _Value( value )
{ }


Infoset* Infoset_Portion::Value( void ) const
{ return _Value; }

Infoset*& Infoset_Portion::Value( void )
{ return _Value; }

PortionType Infoset_Portion::Type( void ) const
{ return porINFOSET; }

Portion* Infoset_Portion::Copy( bool new_data ) const
{ return new Infoset_Portion( _Value ); }

void Infoset_Portion::Output( gOutput& s ) const
{
  s << " (Infoset) ";
}



//---------------------------------------------------------------------
//                            Action type
//---------------------------------------------------------------------
Action_Portion::Action_Portion( Action* value ) 
     : _Value( value )
{ }


Action* Action_Portion::Value( void ) const
{ return _Value; }

Action*& Action_Portion::Value( void )
{ return _Value; }

PortionType Action_Portion::Type( void ) const
{ return porACTION; }

Portion* Action_Portion::Copy( bool new_data ) const
{ return new Action_Portion( _Value ); }

void Action_Portion::Output( gOutput& s ) const
{
  s << " (Action) ";
}



//---------------------------------------------------------------------
//                            Node type
//---------------------------------------------------------------------
Node_Portion::Node_Portion( Node* value ) 
     : _Value( value )
{ }


Node* Node_Portion::Value( void ) const
{ return _Value; }

Node*& Node_Portion::Value( void )
{ return _Value; }

PortionType Node_Portion::Type( void ) const
{ return porNODE; }

Portion* Node_Portion::Copy( bool new_data ) const
{ return new Node_Portion( _Value ); }

void Node_Portion::Output( gOutput& s ) const
{
  s << " (Node) ";
}





//---------------------------------------------------------------------
//                            Stream type
//---------------------------------------------------------------------

#ifdef MEMCHECK
int Stream_Portion::_NumObj = 0;
#endif // MEMCHECK

RefCountHashTable< gOutput* > Stream_Portion::_RefCountTable;

Stream_Portion::Stream_Portion( gOutput& value )
{
  _Value = &value;
  if( !_RefCountTable.IsDefined( _Value ) )
  {
    _RefCountTable.Define( _Value, 1 );
#ifdef MEMCHECK
    _NumObj++;
    gout << ">>> gOutput Ctor - count: " << _NumObj << "\n";
#endif // MEMCHECK
  }
  else
  {
    _RefCountTable( _Value )++;
  }
}

Stream_Portion::~Stream_Portion()
{
  assert( _RefCountTable.IsDefined( _Value ) );
  assert( _RefCountTable( _Value ) > 0 );
  _RefCountTable( _Value )--;
  if( _RefCountTable( _Value ) == 0 )
  {
    delete _Value;
#ifdef MEMCHECK
    _NumObj--;
    gout << ">>> gOutput Dtor - count: " << _NumObj << "\n";
#endif // MEMCHECK
  }
}


gOutput& Stream_Portion::Value( void )
{ return *( (gFileOutput*) _Value ); }

PortionType Stream_Portion::Type( void ) const
{ return porSTREAM; }

Portion* Stream_Portion::Copy( bool new_data ) const
{ return new Stream_Portion( *_Value ); }


void Stream_Portion::Output( gOutput& s ) const
{
  s << " (Stream) ";
}






//--------------------------------------------------------------------
//                      _ErrorMessage
//--------------------------------------------------------------------


gString Portion::_ErrorMessage( const int error_num, const gString& str )
{
  gString result = "Portion Error:\n";
  switch( error_num )
  {
  case 1:
    result += "  Attempted to execute an unsupported operation\n";
    break;
  case 2:
    result += "  Division by zero\n";
    break;
  case 6:
    result += "  An out-of-range list subscript specified\n";
    break;
  case 7:
    result += "  Attempted to set an element of a List_Portion\n";
    result += "  to one with a conflicting Portion type\n";
    break;
  }
  return result;
}



//--------------------------------------------------------------------
//             miscellaneous PortionType functions
//--------------------------------------------------------------------



void PrintPortionTypeSpec( gOutput& s, PortionType type )
{
  if( type == porERROR )
  {
    s << "porERROR ";
  }
  else
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

    if( type & porNFG_DOUBLE )
      s << "porNFG_DOUBLE ";
    if( type & porNFG_RATIONAL )
      s << "porNFG_RATIONAL ";
    if( type & porEFG_DOUBLE )
      s << "porEFG_DOUBLE ";
    if( type & porEFG_RATIONAL )
      s << "porEFG_RATIONAL ";
    if( type & porMIXED_DOUBLE )
      s << "porMIXED_DOUBLE ";
    if( type & porMIXED_RATIONAL )
      s << "porMIXED_RATIONAL ";
    if( type & porBEHAV_DOUBLE )
      s << "porBEHAV_DOUBLE ";
    if( type & porBEHAV_RATIONAL )
      s << "porBEHAV_RATIONAL ";

    if( type & porOUTCOME )
      s << "porOUTCOME ";
    if( type & porPLAYER )
      s << "porPLAYER ";
    if( type & porINFOSET )
      s << "porINFOSET ";
    if( type & porNODE )
      s << "porNODE ";
    if( type & porACTION )
      s << "porACTION ";

    if( type & porSTREAM )
      s << "porSTREAM ";

    if( type & porREFERENCE )
      s << "porREFERENCE ";
  }
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



TEMPLATE class Mixed_Portion<double>;
PortionType Mixed_Portion<double>::Type( void ) const
{ return porMIXED_DOUBLE; }

TEMPLATE class Mixed_Portion<gRational>;
PortionType Mixed_Portion<gRational>::Type( void ) const
{ return porMIXED_RATIONAL; }


TEMPLATE class Behav_Portion<double>;
PortionType Behav_Portion<double>::Type( void ) const
{ return porBEHAV_DOUBLE; }

TEMPLATE class Behav_Portion<gRational>;
PortionType Behav_Portion<gRational>::Type( void ) const
{ return porBEHAV_RATIONAL; }



TEMPLATE class Nfg_Portion<double>;
PortionType Nfg_Portion<double>::Type( void ) const
{ return porNFG_DOUBLE; }

TEMPLATE class Nfg_Portion<gRational>;
PortionType Nfg_Portion<gRational>::Type( void ) const
{ return porNFG_RATIONAL; }


TEMPLATE class Efg_Portion<double>;
PortionType Efg_Portion<double>::Type( void ) const
{ return porEFG_DOUBLE; }

TEMPLATE class Efg_Portion<gRational>;
PortionType Efg_Portion<gRational>::Type( void ) const
{ return porEFG_RATIONAL; }





#include "garray.imp"

TEMPLATE class gArray<Portion *>;

#include "gblock.imp"

TEMPLATE class gBlock<Portion*>;


