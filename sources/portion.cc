//#
//# FILE: portion.cc -- implementation of Portion base and descendent classes
//#                     companion to GSM
//#
//# $Id$
//#


#include <assert.h>

#include "portion.h"
// #include "gsm.h"
#include "gsmhash.h"

#include "gambitio.h"

#include "normal.h"
#include "extform.h"






#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__





//---------------------------------------------------------------------
//                          base class
//---------------------------------------------------------------------

#ifdef MEMCHECK
int Portion::_NumObj = 0;
#endif

Portion::Portion( void )
{
#ifdef MEMCHECK
  _NumObj++;
  gout << "--- Portion Ctor, count: " << _NumObj << "\n";
#endif
}

Portion::~Portion()
{ 
#ifdef MEMCHECK
  _NumObj--;
  gout << "--- Portion Dtor, count: " << _NumObj << "\n";
#endif
}

bool Portion::operator == ( Portion* p ) const
{
  return false;
}

//---------------------------------------------------------------------
//                          Error class
//---------------------------------------------------------------------

ErrorPortion::ErrorPortion( const gString& value )
: _Value( value )
{ }

ErrorPortion::~ErrorPortion()
{ }

gString ErrorPortion::Value( void )
{ return _Value; }

PortionType ErrorPortion::Type( void ) const
{ return porERROR; }

void ErrorPortion::Output( gOutput& s ) const
{ s << "(Error) " << _Value; }

Portion* ErrorPortion::ValCopy( void ) const
{ return new ErrorPortion( _Value ); }

Portion* ErrorPortion::RefCopy( void ) const
{ return new ErrorPortion( _Value ); }

void ErrorPortion::AssignFrom( Portion* p )
{
  assert( p->Type() == Type() );
  _Value = ( (ErrorPortion*) p )->_Value;
}

bool ErrorPortion::operator == ( Portion *p ) const
{
  if( p->Type() == Type() )
    return ( _Value == ( (ErrorPortion*) p )->_Value );
  else
    return false;
}

//---------------------------------------------------------------------
//                          Reference class
//---------------------------------------------------------------------

ReferencePortion::ReferencePortion( const gString& value )
: _Value( value )
{ }

ReferencePortion::~ReferencePortion()
{ }

gString ReferencePortion::Value( void )
{ return _Value; }

PortionType ReferencePortion::Type( void ) const
{ return porREFERENCE; }

void ReferencePortion::Output( gOutput& s ) const
{ s << "(Reference) \"" << _Value << "\""; }

Portion* ReferencePortion::ValCopy( void ) const
{ return new ReferencePortion( _Value ); }

Portion* ReferencePortion::RefCopy( void ) const
{ return new ReferencePortion( _Value ); }

void ReferencePortion::AssignFrom( Portion* p )
{
  assert( p->Type() == Type() );
  _Value = ( (ReferencePortion*) p )->_Value;
}

bool ReferencePortion::operator == ( Portion *p ) const
{
  if( p->Type() == Type() )
    return ( _Value == ( (ReferencePortion*) p )->_Value );
  else
    return false;
}


//---------------------------------------------------------------------
//                          int class
//---------------------------------------------------------------------

IntPortion::IntPortion( void )
{ }

IntPortion::~IntPortion()
{ }

long& IntPortion::Value( void ) const
{ return *_Value; }

PortionType IntPortion::Type( void ) const
{ return porINTEGER; }

void IntPortion::Output( gOutput& s ) const
{ s << *_Value; }

Portion* IntPortion::ValCopy( void ) const
{ return new IntValPortion( *_Value ); }

Portion* IntPortion::RefCopy( void ) const
{ return new IntRefPortion( *_Value ); }

void IntPortion::AssignFrom( Portion* p )
{
  assert( p->Type() == Type() );
  *_Value = *( ( (IntPortion*) p )->_Value );
}

bool IntPortion::operator == ( Portion *p ) const
{
  if( p->Type() == Type() )
    return ( *_Value == *( (IntPortion*) p )->_Value );
  else
    return false;
}


IntValPortion::IntValPortion( long value )
{ _Value = new long( value ); }

IntValPortion::~IntValPortion()
{ delete _Value; }


IntRefPortion::IntRefPortion( long& value )
{ _Value = &value; }

IntRefPortion::~IntRefPortion()
{ }


//---------------------------------------------------------------------
//                          Float class
//---------------------------------------------------------------------

FloatPortion::FloatPortion( void )
{ }

FloatPortion::~FloatPortion()
{ }

double& FloatPortion::Value( void ) const
{ return *_Value; }

PortionType FloatPortion::Type( void ) const
{ return porFLOAT; }

void FloatPortion::Output( gOutput& s ) const
{ s << *_Value; }

Portion* FloatPortion::ValCopy( void ) const
{ return new FloatValPortion( *_Value ); }

Portion* FloatPortion::RefCopy( void ) const
{ return new FloatRefPortion( *_Value ); }

void FloatPortion::AssignFrom( Portion* p )
{
  assert( p->Type() == Type() );
  *_Value = *( ( (FloatPortion*) p )->_Value );
}

bool FloatPortion::operator == ( Portion *p ) const
{
  if( p->Type() == Type() )
    return ( *_Value == *( (FloatPortion*) p )->_Value );
  else
    return false;
}


FloatValPortion::FloatValPortion( double value )
{ _Value = new double( value ); }

FloatValPortion::~FloatValPortion()
{ delete _Value; }

FloatRefPortion::FloatRefPortion( double& value )
{ _Value = &value; }

FloatRefPortion::~FloatRefPortion()
{ }




//---------------------------------------------------------------------
//                          Rational class
//---------------------------------------------------------------------

RationalPortion::RationalPortion( void )
{ }

RationalPortion::~RationalPortion()
{ }

gRational& RationalPortion::Value( void ) const
{ return *_Value; }

PortionType RationalPortion::Type( void ) const
{ return porRATIONAL; }

void RationalPortion::Output( gOutput& s ) const
{ s << *_Value; }

Portion* RationalPortion::ValCopy( void ) const
{ return new RationalValPortion( *_Value ); }

Portion* RationalPortion::RefCopy( void ) const
{ return new RationalRefPortion( *_Value ); }

void RationalPortion::AssignFrom( Portion* p )
{
  assert( p->Type() == Type() );
  *_Value = *( ( (RationalPortion*) p )->_Value );
}

bool RationalPortion::operator == ( Portion *p ) const
{
  if( p->Type() == Type() )
    return ( *_Value == *( (RationalPortion*) p )->_Value );
  else
    return false;
}


RationalValPortion::RationalValPortion( gRational value )
{ _Value = new gRational( value ); }

RationalValPortion::~RationalValPortion()
{ delete _Value; }


RationalRefPortion::RationalRefPortion( gRational& value )
{ _Value = &value; }

RationalRefPortion::~RationalRefPortion()
{ }




//---------------------------------------------------------------------
//                          Text class
//---------------------------------------------------------------------

TextPortion::TextPortion( void )
{ }

TextPortion::~TextPortion()
{ }

gString& TextPortion::Value( void ) const
{ return *_Value; }

PortionType TextPortion::Type( void ) const
{ return porTEXT; }

void TextPortion::Output( gOutput& s ) const
{ s << "\"" << *_Value << "\""; }

Portion* TextPortion::ValCopy( void ) const
{ return new TextValPortion( *_Value ); }

Portion* TextPortion::RefCopy( void ) const
{ return new TextRefPortion( *_Value ); }

void TextPortion::AssignFrom( Portion* p )
{
  assert( p->Type() == Type() );
  *_Value = *( ( (TextPortion*) p )->_Value );
}

bool TextPortion::operator == ( Portion *p ) const
{
  if( p->Type() == Type() )
    return ( *_Value == *( (TextPortion*) p )->_Value );
  else
    return false;
}


TextValPortion::TextValPortion( gString value )
{ _Value = new gString( value ); }

TextValPortion::~TextValPortion()
{ delete _Value; }


TextRefPortion::TextRefPortion( gString& value )
{ _Value = &value; }

TextRefPortion::~TextRefPortion()
{ }







//---------------------------------------------------------------------
//                          Bool class
//---------------------------------------------------------------------

BoolPortion::BoolPortion( void )
{ }

BoolPortion::~BoolPortion()
{ }

bool& BoolPortion::Value( void ) const
{ return *_Value; }

PortionType BoolPortion::Type( void ) const
{ return porBOOL; }

void BoolPortion::Output( gOutput& s ) const
{ s << ( *_Value ? "true" : "false" ); }

Portion* BoolPortion::ValCopy( void ) const
{ return new BoolValPortion( *_Value ); }

Portion* BoolPortion::RefCopy( void ) const
{ return new BoolRefPortion( *_Value ); }

void BoolPortion::AssignFrom( Portion* p )
{
  assert( p->Type() == Type() );
  *_Value = *( ( (BoolPortion*) p )->_Value );
}

bool BoolPortion::operator == ( Portion *p ) const
{
  if( p->Type() == Type() )
    return ( *_Value == *( (BoolPortion*) p )->_Value );
  else
    return false;
}


BoolValPortion::BoolValPortion( bool value )
{ _Value = new bool( value ); }

BoolValPortion::~BoolValPortion()
{ delete _Value; }


BoolRefPortion::BoolRefPortion( bool& value )
{ _Value = &value; }

BoolRefPortion::~BoolRefPortion()
{ }





//---------------------------------------------------------------------
//                          Outcome class
//---------------------------------------------------------------------



OutcomePortion::OutcomePortion( void )
{ }

OutcomePortion::~OutcomePortion()
{ }

Outcome*& OutcomePortion::Value( void ) const
{ return *_Value; }

PortionType OutcomePortion::Type( void ) const
{ return porOUTCOME; }

void OutcomePortion::Output( gOutput& s ) const
{ s << *_Value; }

Portion* OutcomePortion::ValCopy( void ) const
{ return new OutcomeValPortion( *_Value ); }

Portion* OutcomePortion::RefCopy( void ) const
{ return new OutcomeRefPortion( *_Value ); }

void OutcomePortion::AssignFrom( Portion* p )
{
  assert( p->Type() == Type() );
  *_Value = *( ( (OutcomePortion*) p )->_Value );
}

bool OutcomePortion::operator == ( Portion *p ) const
{
  if( p->Type() == Type() )
    return ( *_Value == *( (OutcomePortion*) p )->_Value );
  else
    return false;
}


OutcomeValPortion::OutcomeValPortion( Outcome* value )
{ _Value = new Outcome*( value ); }

OutcomeValPortion::~OutcomeValPortion()
{ delete _Value; }


OutcomeRefPortion::OutcomeRefPortion( Outcome*& value )
{ _Value = &value; }

OutcomeRefPortion::~OutcomeRefPortion()
{ }





//---------------------------------------------------------------------
//                          EfPlayer class
//---------------------------------------------------------------------



EfPlayerPortion::EfPlayerPortion( void )
{ }

EfPlayerPortion::~EfPlayerPortion()
{ }

Player*& EfPlayerPortion::Value( void ) const
{ return *_Value; }

PortionType EfPlayerPortion::Type( void ) const
{ return porEF_PLAYER; }

void EfPlayerPortion::Output( gOutput& s ) const
{ s << *_Value; }

Portion* EfPlayerPortion::ValCopy( void ) const
{ return new EfPlayerValPortion( *_Value ); }

Portion* EfPlayerPortion::RefCopy( void ) const
{ return new EfPlayerRefPortion( *_Value ); }

void EfPlayerPortion::AssignFrom( Portion* p )
{
  assert( p->Type() == Type() );
  *_Value = *( ( (EfPlayerPortion*) p )->_Value );
}

bool EfPlayerPortion::operator == ( Portion *p ) const
{
  if( p->Type() == Type() )
    return ( *_Value == *( (EfPlayerPortion*) p )->_Value );
  else
    return false;
}


EfPlayerValPortion::EfPlayerValPortion( Player* value )
{ _Value = new Player*( value ); }

EfPlayerValPortion::~EfPlayerValPortion()
{ delete _Value; }


EfPlayerRefPortion::EfPlayerRefPortion( Player*& value )
{ _Value = &value; }

EfPlayerRefPortion::~EfPlayerRefPortion()
{ }




//---------------------------------------------------------------------
//                          Infoset class
//---------------------------------------------------------------------



InfosetPortion::InfosetPortion( void )
{ }

InfosetPortion::~InfosetPortion()
{ }

Infoset*& InfosetPortion::Value( void ) const
{ return *_Value; }

PortionType InfosetPortion::Type( void ) const
{ return porINFOSET; }

void InfosetPortion::Output( gOutput& s ) const
{ s << *_Value; }

Portion* InfosetPortion::ValCopy( void ) const
{ return new InfosetValPortion( *_Value ); }

Portion* InfosetPortion::RefCopy( void ) const
{ return new InfosetRefPortion( *_Value ); }

void InfosetPortion::AssignFrom( Portion* p )
{
  assert( p->Type() == Type() );
  *_Value = *( ( (InfosetPortion*) p )->_Value );
}

bool InfosetPortion::operator == ( Portion *p ) const
{
  if( p->Type() == Type() )
    return ( *_Value == *( (InfosetPortion*) p )->_Value );
  else
    return false;
}


InfosetValPortion::InfosetValPortion( Infoset* value )
{ _Value = new Infoset*( value ); }

InfosetValPortion::~InfosetValPortion()
{ delete _Value; }


InfosetRefPortion::InfosetRefPortion( Infoset*& value )
{ _Value = &value; }

InfosetRefPortion::~InfosetRefPortion()
{ }




//---------------------------------------------------------------------
//                          Node class
//---------------------------------------------------------------------



NodePortion::NodePortion( void )
{ }

NodePortion::~NodePortion()
{ }

Node*& NodePortion::Value( void ) const
{ return *_Value; }

PortionType NodePortion::Type( void ) const
{ return porNODE; }

void NodePortion::Output( gOutput& s ) const
{ s << *_Value; }

Portion* NodePortion::ValCopy( void ) const
{ return new NodeValPortion( *_Value ); }

Portion* NodePortion::RefCopy( void ) const
{ return new NodeRefPortion( *_Value ); }

void NodePortion::AssignFrom( Portion* p )
{
  assert( p->Type() == Type() );
  *_Value = *( ( (NodePortion*) p )->_Value );
}

bool NodePortion::operator == ( Portion *p ) const
{
  if( p->Type() == Type() )
    return ( *_Value == *( (NodePortion*) p )->_Value );
  else
    return false;
}


NodeValPortion::NodeValPortion( Node* value )
{ _Value = new Node*( value ); }

NodeValPortion::~NodeValPortion()
{ delete _Value; }


NodeRefPortion::NodeRefPortion( Node*& value )
{ _Value = &value; }

NodeRefPortion::~NodeRefPortion()
{ }






//---------------------------------------------------------------------
//                          Action class
//---------------------------------------------------------------------



ActionPortion::ActionPortion( void )
{ }

ActionPortion::~ActionPortion()
{ }

Action*& ActionPortion::Value( void ) const
{ return *_Value; }

PortionType ActionPortion::Type( void ) const
{ return porACTION; }

void ActionPortion::Output( gOutput& s ) const
{ s << *_Value; }

Portion* ActionPortion::ValCopy( void ) const
{ return new ActionValPortion( *_Value ); }

Portion* ActionPortion::RefCopy( void ) const
{ return new ActionRefPortion( *_Value ); }

void ActionPortion::AssignFrom( Portion* p )
{
  assert( p->Type() == Type() );
  *_Value = *( ( (ActionPortion*) p )->_Value );
}

bool ActionPortion::operator == ( Portion *p ) const
{
  if( p->Type() == Type() )
    return ( *_Value == *( (ActionPortion*) p )->_Value );
  else
    return false;
}


ActionValPortion::ActionValPortion( Action* value )
{ _Value = new Action*( value ); }

ActionValPortion::~ActionValPortion()
{ delete _Value; }


ActionRefPortion::ActionRefPortion( Action*& value )
{ _Value = &value; }

ActionRefPortion::~ActionRefPortion()
{ }




//---------------------------------------------------------------------
//                          Mixed class
//---------------------------------------------------------------------

#include "mixed.h"

template <class T> MixedPortion<T>::MixedPortion( void )
{ }

template <class T> MixedPortion<T>::~MixedPortion()
{ }

template <class T> MixedProfile<T>& MixedPortion<T>::Value( void ) const
{ return *_Value; }

template <class T> PortionType MixedPortion<T>::Type( void ) const
{ return porMIXED; }

template <class T> void MixedPortion<T>::Output( gOutput& s ) const
{ s << "(Mixed)"; }

template <class T> Portion* MixedPortion<T>::ValCopy( void ) const
{
  return new MixedValPortion<T>
    ( * new MixedProfile<T>( * (MixedProfile<T>*) _Value ) ); 
}

template <class T> Portion* MixedPortion<T>::RefCopy( void ) const
{ return new MixedRefPortion<T>( *_Value ); }

template <class T> void MixedPortion<T>::AssignFrom( Portion* p )
{
  assert( p->Type() == Type() );
  *_Value = *( ( (MixedPortion<T>*) p )->_Value );
}

template <class T> bool MixedPortion<T>::operator == ( Portion *p ) const
{
  if( p->Type() == Type() )
    return ( *_Value == *( (MixedPortion<T>*) p )->_Value );
  else
    return false;
}


template <class T> MixedValPortion<T>::MixedValPortion( MixedProfile<T>& value)
{ _Value = &value; }

template <class T> MixedValPortion<T>::~MixedValPortion()
{ delete _Value; }


template <class T> MixedRefPortion<T>::MixedRefPortion( MixedProfile<T>& value)
{ _Value = &value; }

template <class T> MixedRefPortion<T>::~MixedRefPortion()
{ }



TEMPLATE class MixedPortion< double >;
TEMPLATE class MixedValPortion< double >;
TEMPLATE class MixedRefPortion< double >;
PortionType MixedPortion< double >::Type( void ) const
{ return porMIXED_FLOAT; }

TEMPLATE class MixedPortion< gRational >;
TEMPLATE class MixedValPortion< gRational >;
TEMPLATE class MixedRefPortion< gRational >;
PortionType MixedPortion< gRational >::Type( void ) const
{ return porMIXED_RATIONAL; }





//---------------------------------------------------------------------
//                          Behav class
//---------------------------------------------------------------------

#include "behav.h"

template <class T> BehavPortion<T>::BehavPortion( void )
{ }

template <class T> BehavPortion<T>::~BehavPortion()
{ }

template <class T> BehavProfile<T>& BehavPortion<T>::Value( void ) const
{ return *_Value; }

template <class T> PortionType BehavPortion<T>::Type( void ) const
{ return porBEHAV; }

template <class T> void BehavPortion<T>::Output( gOutput& s ) const
{ s << "(Behav)"; }

template <class T> Portion* BehavPortion<T>::ValCopy( void ) const
{
  return new BehavValPortion<T>
    ( * new BehavProfile<T>( * (BehavProfile<T>*) _Value ) ); 
}

template <class T> Portion* BehavPortion<T>::RefCopy( void ) const
{ return new BehavRefPortion<T>( *_Value ); }

template <class T> void BehavPortion<T>::AssignFrom( Portion* p )
{
  assert( p->Type() == Type() );
  *_Value = *( ( (BehavPortion<T>*) p )->_Value );
}

template <class T> bool BehavPortion<T>::operator == ( Portion *p ) const
{
  if( p->Type() == Type() )
    return ( *_Value == *( (BehavPortion<T>*) p )->_Value );
  else
    return false;
}


template <class T> BehavValPortion<T>::BehavValPortion( BehavProfile<T>& value)
{ _Value = &value; }

template <class T> BehavValPortion<T>::~BehavValPortion()
{ delete _Value; }


template <class T> BehavRefPortion<T>::BehavRefPortion( BehavProfile<T>& value)
{ _Value = &value; }

template <class T> BehavRefPortion<T>::~BehavRefPortion()
{ }



TEMPLATE class BehavPortion< double >;
TEMPLATE class BehavValPortion< double >;
TEMPLATE class BehavRefPortion< double >;
PortionType BehavPortion< double >::Type( void ) const
{ return porBEHAV_FLOAT; }

TEMPLATE class BehavPortion< gRational >;
TEMPLATE class BehavValPortion< gRational >;
TEMPLATE class BehavRefPortion< gRational >;
PortionType BehavPortion< gRational >::Type( void ) const
{ return porBEHAV_RATIONAL; }




//---------------------------------------------------------------------
//                          BaseNfg class
//---------------------------------------------------------------------


BaseNfgPortion::BaseNfgPortion( void )
{ }

BaseNfgPortion::~BaseNfgPortion()
{ }

BaseNormalForm& BaseNfgPortion::Value( void ) const
{ return *_Value; }

void BaseNfgPortion::AssignFrom( Portion* p )
{
  assert( p->Type() == Type() );
  *_Value = *( ( (BaseNfgPortion*) p )->_Value );
}

bool BaseNfgPortion::operator == ( Portion *p ) const
{
/*
  assert( p->Type() == Type() );
  return ( *_Value == *( (BaseNfgPortion*) p )->_Value );
*/
  return false;
}


//---------------------------------------------------------------------
//                          Nfg class
//---------------------------------------------------------------------


template <class T> NormalForm<T>& NfgPortion<T>::Value( void ) const
{ return * (NormalForm<T>*) _Value; }

template <class T> void NfgPortion<T>::Output( gOutput& s ) const
{ s << "(Nfg)"; }

template <class T> Portion* NfgPortion<T>::ValCopy( void ) const
{
  return new NfgValPortion<T>
    ( * new NormalForm<T>( * (NormalForm<T>*) _Value ) ); 
}

template <class T> Portion* NfgPortion<T>::RefCopy( void ) const
{ return new NfgRefPortion<T>( * (NormalForm<T>*) _Value ); }



template <class T> NfgValPortion<T>::NfgValPortion( NormalForm<T>& value )
{ _Value = &value; }

template <class T> NfgValPortion<T>::~NfgValPortion()
{ delete _Value; }

template <class T> NfgRefPortion<T>::NfgRefPortion( NormalForm<T>& value)
{ _Value = &value; }

template <class T> NfgRefPortion<T>::~NfgRefPortion()
{ }




TEMPLATE class NfgPortion< double >;
TEMPLATE class NfgValPortion< double >;
TEMPLATE class NfgRefPortion< double >;
PortionType NfgPortion<double>::Type( void ) const
{ return porNFG_FLOAT; }

TEMPLATE class NfgPortion< gRational >;
TEMPLATE class NfgValPortion< gRational >;
TEMPLATE class NfgRefPortion< gRational >;
PortionType NfgPortion<gRational>::Type( void ) const
{ return porNFG_RATIONAL; }






//---------------------------------------------------------------------
//                          BaseEfg class
//---------------------------------------------------------------------


BaseEfgPortion::BaseEfgPortion( void )
{ }

BaseEfgPortion::~BaseEfgPortion()
{ }

BaseExtForm& BaseEfgPortion::Value( void ) const
{ return *_Value; }

void BaseEfgPortion::AssignFrom( Portion* p )
{
  assert( p->Type() == Type() );
  *_Value = *( ( (BaseEfgPortion*) p )->_Value );
}

bool BaseEfgPortion::operator == ( Portion *p ) const
{
/*
  assert( p->Type() == Type() );
  return ( *_Value == *( (BaseEfgPortion*) p )->_Value );
*/
  return false;
}


//---------------------------------------------------------------------
//                          Efg class
//---------------------------------------------------------------------


template <class T> ExtForm<T>& EfgPortion<T>::Value( void ) const
{ return * (ExtForm<T>*) _Value; }

template <class T> void EfgPortion<T>::Output( gOutput& s ) const
{ s << "(Efg)"; }

template <class T> Portion* EfgPortion<T>::ValCopy( void ) const
{
  return new EfgValPortion<T>
    ( * new ExtForm<T>( * (ExtForm<T>*) _Value ) ); 
}

template <class T> Portion* EfgPortion<T>::RefCopy( void ) const
{ return new EfgRefPortion<T>( * (ExtForm<T>*) _Value ); }




template <class T> EfgValPortion<T>::EfgValPortion( ExtForm<T>& value )
{ _Value = &value; }

template <class T> EfgValPortion<T>::~EfgValPortion()
{ delete _Value; }

template <class T> EfgRefPortion<T>::EfgRefPortion( ExtForm<T>& value)
{ _Value = &value; }

template <class T> EfgRefPortion<T>::~EfgRefPortion()
{ }




TEMPLATE class EfgPortion< double >;
TEMPLATE class EfgValPortion< double >;
TEMPLATE class EfgRefPortion< double >;
PortionType EfgPortion<double>::Type( void ) const
{ return porEFG_FLOAT; }

TEMPLATE class EfgPortion< gRational >;
TEMPLATE class EfgValPortion< gRational >;
TEMPLATE class EfgRefPortion< gRational >;
PortionType EfgPortion<gRational>::Type( void ) const
{ return porEFG_RATIONAL; }





//---------------------------------------------------------------------
//                          Output class
//---------------------------------------------------------------------

OutputPortion::OutputPortion( void )
{ }

OutputPortion::~OutputPortion()
{ }

gOutput& OutputPortion::Value( void ) const
{ return *_Value; }

PortionType OutputPortion::Type( void ) const
{ return porOUTPUT; }

void OutputPortion::Output( gOutput& s ) const
{ s << "(Output)"; }

Portion* OutputPortion::ValCopy( void ) const
{
  return new ErrorPortion
    ( (gString)
     "OutputPortion::ValCopy() not available due to lack of copy\n" +
     "constructor in class gOutput\n" );
  // return new OutputValPortion( *_Value ); 
}

Portion* OutputPortion::RefCopy( void ) const
{ return new OutputRefPortion( *_Value ); }

void OutputPortion::AssignFrom( Portion* p )
{
  assert( 0 );
/*
  assert( p->Type() == Type() );
  *_Value = *( ( (OutputPortion*) p )->_Value );
*/
}

bool OutputPortion::operator == ( Portion *p ) const
{
/*
  assert( p->Type() == Type() );
  return ( *_Value == *( (OutputPortion*) p )->_Value );
*/
  return false;
}


OutputValPortion::OutputValPortion( gOutput& value )
{ _Value = &value; }

OutputValPortion::~OutputValPortion()
{ delete _Value; }


OutputRefPortion::OutputRefPortion( gOutput& value )
{ _Value = &value; }

OutputRefPortion::~OutputRefPortion()
{ }




//---------------------------------------------------------------------
//                          Input class
//---------------------------------------------------------------------

InputPortion::InputPortion( void )
{ }

InputPortion::~InputPortion()
{ }

gInput& InputPortion::Value( void ) const
{ return *_Value; }

PortionType InputPortion::Type( void ) const
{ return porINPUT; }

void InputPortion::Output( gOutput& s ) const
{ s << "(Input)"; }

Portion* InputPortion::ValCopy( void ) const
{ 
  return new ErrorPortion
    ( (gString)
     "InputPortion::ValCopy() not available due to lack of copy\n" +
     "constructor in class gInput\n" );
  // return new InputValPortion( *_Value ); 
}

Portion* InputPortion::RefCopy( void ) const
{ return new InputRefPortion( *_Value ); }

void InputPortion::AssignFrom( Portion* p )
{
  assert( 0 );
/*
  assert( p->Type() == Type() );
  *_Value = *( ( (InputPortion*) p )->_Value );
*/
}

bool InputPortion::operator == ( Portion *p ) const
{
/*
  assert( p->Type() == Type() );
  return ( *_Value == *( (InputPortion*) p )->_Value );
*/
  return false;
}


InputValPortion::InputValPortion( gInput& value )
{ _Value = &value; }

InputValPortion::~InputValPortion()
{ delete _Value; }


InputRefPortion::InputRefPortion( gInput& value )
{ _Value = &value; }

InputRefPortion::~InputRefPortion()
{ }



//---------------------------------------------------------------------
//                          List class
//---------------------------------------------------------------------

#include "gblock.h"

ListPortion::ListPortion( void )
{ 
  _DataType = porUNKNOWN;
}

ListPortion::~ListPortion()
{ }

gBlock< Portion* >& ListPortion::Value( void ) const
{ return *_Value; }

PortionType ListPortion::Type( void ) const
{ return porLIST; }

Portion* ListPortion::ValCopy( void ) const
{ return new ListValPortion( *_Value ); }

Portion* ListPortion::RefCopy( void ) const
{ return new ListRefPortion( *_Value ); }

void ListPortion::AssignFrom( Portion* p )
{
  int i;
  int length;
  int type_match;
  int result;
  gBlock< Portion* >& value = *( ( (ListPortion*) p )->_Value );

  assert( p->Type() == Type() );
  assert( ( (ListPortion*) p )->_DataType == _DataType );

  Flush();
  delete _Value;
  _Value = new gBlock< Portion* >;   
  for( i = 1, length = value.Length(); i <= length; i++ )
  {
    result = Insert( value[ i ]->ValCopy(), i );
    assert( result != 0 );
  }
}

bool ListPortion::operator == ( Portion *p ) const
{
  bool result = true;
  int i;
  int length = _Value->Length();

  if( p->Type() == Type() )
  {
    if( _Value->Length() == ( (ListPortion*) p )->_Value->Length() )
    {
      for( i = 1; i <= length; i++ )
      {
	result = result & 
	  ( (*_Value)[ i ] == ( *( (ListPortion*) p )->_Value )[ i ] );
      }
    }
    else
    {
      result = false;
    }
  }
  else
  {
    result = false;
  }
  return result;
}


ListValPortion::ListValPortion( void )
{ _Value = new gBlock< Portion* >; }

ListValPortion::ListValPortion( gBlock< Portion* >& value )
{ 
  int i;
  int length;
  int type_match;
  int result;

  _Value = new gBlock< Portion* >; 

  for( i = 1, length = value.Length(); i <= length; i++ )
  {
    result = Insert( value[ i ]->ValCopy(), i );
    assert( result != 0 );
  }
}

ListValPortion::~ListValPortion()
{
  Flush();
  delete _Value;
}


ListRefPortion::ListRefPortion( gBlock< Portion* >& value )
{ _Value = &value; }

ListRefPortion::~ListRefPortion()
{ }












void ListPortion::SetDataType( PortionType data_type )
{
  assert( _DataType == porUNKNOWN );
  _DataType = data_type;
}

PortionType ListPortion::DataType( void ) const
{ return _DataType; }


bool ListPortion::TypeCheck( Portion* item )
{
  bool result = false;

  if( item->Type() == _DataType )
  {
    result = true;
  }
  else if( item->Type() == porLIST )
  {
    if( ( (ListPortion*) item )->_DataType == _DataType )
      result = true;
  }
  return result;
}


void ListPortion::Output( gOutput& s ) const
{
  int i;
  int length = _Value->Length();

  s << "{";
  if( length >= 1 )
  {
    s << " " << (*_Value)[ 1 ];
    for( i = 2; i <= length; i++ )
    {
      s << ", " << (*_Value)[ i ];
    }
  }
  else
  {
    s << " empty";
  }
  s << " }";
}



int ListPortion::Append( Portion* item )
{ return Insert( item, _Value->Length() + 1 ); }


int ListPortion::Insert( Portion* item, int index )
{
  int result = 0;

#ifndef NDEBUG
  if( item->Type() == porREFERENCE )
  {
    gerr << "Portion Error:\n";
    gerr << "  Attempted to insert a ReferencePortion into\n";
    gerr << "  a ListPortion\n";
  }
  assert( item->Type() != porREFERENCE );
#endif
  
  if( _Value->Length() == 0 )  // creating a new list
  {
    if( item->Type() == _DataType || _DataType == porUNKNOWN )
    {
      if( item->Type() == porLIST )
	_DataType = ( (ListPortion*) item )->_DataType;
      else
	_DataType = item->Type();
      result = _Value->Insert( item, index );
    }
    else
    {
      delete item;
    }
  }
  else  // inserting into an existing list
  {
    if( TypeCheck( item ) )
      result = _Value->Insert( item, index );
    else
      delete item;
  }
  return result;
}


int ListPortion::Contains( Portion* item ) const
{
  int result = 0;
  int i;
  int length = _Value->Length();

  for( i = 1; i < length; i++ )
  {
    if( (*_Value)[ i ]->operator == ( item ) )
    {
      result = i;
      break;
    }
  }
  return result;
}



Portion* ListPortion::Remove( int index )
{ 
  if( index >= 1 && index <= _Value->Length() )
    return _Value->Remove( index );
  else
    return 0;
}

int ListPortion::Length( void ) const
{ return _Value->Length(); }


void ListPortion::Flush( void )
{
  int i, length;
  for( i = 1, length = _Value->Length(); i <= length; i++ )
  {
    delete Remove( 1 );
  }
  assert( _Value->Length() == 0 );
}


Portion* ListPortion::Subscript( int index ) const
{
  if( index >= 1 && index <= _Value->Length() )
  {
    assert( (*_Value)[ index ] != 0 );
    return (*_Value)[ index ]->RefCopy();
  }
  else
    return 0;
}








//--------------------------------------------------------------------
//             miscellaneous PortionType functions
//--------------------------------------------------------------------



void PrintPortionTypeSpec( gOutput& s, PortionType type )
{
  if( type == porERROR )
  {
    s << " ERROR";
  }
  else
  {
    if( type & porBOOL )
      s << " BOOLEAN";
    if( type & porFLOAT )
      s << " FLOAT";
    if( type & porINTEGER )
      s << " INTEGER";
    if( type & porRATIONAL )
      s << " RATIONAL";
    if( type & porTEXT )
      s << " TEXT";
    if( type & porLIST )
      s << " LIST";

    if( type & porNFG_FLOAT )
      s << " NFG_FLOAT";
    if( type & porNFG_RATIONAL )
      s << " NFG_RATIONAL";
    if( type & porEFG_FLOAT )
      s << " EFG_FLOAT";
    if( type & porEFG_RATIONAL )
      s << " EFG_RATIONAL";
    if( type & porMIXED_FLOAT )
      s << " MIXED_FLOAT";
    if( type & porMIXED_RATIONAL )
      s << " MIXED_RATIONAL";
    if( type & porBEHAV_FLOAT )
      s << " BEHAV_FLOAT";
    if( type & porBEHAV_RATIONAL )
      s << " BEHAV_RATIONAL";

    if( type & porOUTCOME )
      s << " OUTCOME";
    if( type & porEF_PLAYER )
      s << " EF_PLAYER";
    if( type & porINFOSET )
      s << " INFOSET";
    if( type & porNODE )
      s << " NODE";
    if( type & porACTION )
      s << " ACTION";

    if( type & porOUTPUT )
      s << " OUTPUT";
    if( type & porINPUT )
      s << " INPUT";

    if( type & porREFERENCE )
      s << " REFERENCE";

    if( type & porUNKNOWN )
      s << " UNKNOWN";
  }
}




gOutput& operator << ( gOutput& s, Portion* p )
{
  p->Output( s );
  return s;
}





//----------------------------------------------------------------------
//                         class instantiations
//----------------------------------------------------------------------



#include "garray.imp"

TEMPLATE class gArray<Portion *>;

#include "gblock.imp"

TEMPLATE class gBlock<Portion*>;



