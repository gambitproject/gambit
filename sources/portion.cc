//#
//# FILE: portion.cc -- implementation of Portion base and descendent classes
//#                     companion to GSM
//#
//# $Id$
//#


#include <assert.h>
#include <string.h>
//----------------------------------------------------------------------
//                         class instantiations
//----------------------------------------------------------------------



class Portion;
#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

#include "garray.imp"

//TEMPLATE class gArray<Portion *>;

#include "gblock.imp"

//TEMPLATE class gBlock<Portion*>;


#ifdef __BORLANDC__
#pragma option -Jgx
#endif

#include "portion.h"
#include "gsmhash.h"

#include "gambitio.h"

#include "nfg.h"
#include "efg.h"



//---------------------------------------------------------------------
//                          base class
//---------------------------------------------------------------------


long Portion::_WriteWidth = 0;
long Portion::_WritePrecis = 6;
bool Portion::_WriteExpmode = false;
bool Portion::_WriteQuoted = true;
bool Portion::_WriteListBraces = true;
bool Portion::_WriteListCommas = true;
long Portion::_WriteListLF = 0;
long Portion::_WriteListIndent = 2;

void Portion::_SetWriteWidth( long x )
{ _WriteWidth = x; }
void Portion::_SetWritePrecis( long x )
{ _WritePrecis = x; }
void Portion::_SetWriteExpmode( bool x )
{ _WriteExpmode = x; }
void Portion::_SetWriteQuoted( bool x )
{ _WriteQuoted = x; }
void Portion::_SetWriteListBraces( bool x )
{ _WriteListBraces = x; }
void Portion::_SetWriteListCommas( bool x )
{ _WriteListCommas = x; }
void Portion::_SetWriteListLF( long x )
{ _WriteListLF = x; }
void Portion::_SetWriteListIndent( long x )
{ _WriteListIndent = x; }



void Portion::Output( gOutput& s ) const
{
  s.SetWidth( (int) _WriteWidth );
  s.SetPrec( (int) _WritePrecis );
  if( _WriteExpmode )
    s.SetExpMode();
  else
    s.SetFloatMode();
}




#ifdef MEMCHECK
int Portion::_NumObj = 0;
#endif

Portion::Portion( void )
{
  _IsValid = true;
  _Owner = 0;
  _Original = 0;

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


bool Portion::operator == ( Portion* ) const
{ return false; }


bool Portion::IsValid( void ) const
{ return Original()->_IsValid; }


void Portion::SetIsValid( bool is_valid )
{ 
  Original()->_IsValid = is_valid; 
  if( !IsValid() ) 
    SetOwner( 0 );
}


void Portion::SetOwner( Portion* p )
{ Original()->_Owner = p; }

Portion* Portion::Owner( void ) const
{ return Original()->_Owner; }


void Portion::SetOriginal( const Portion* p )
{ _Original = (Portion*) p; }

Portion* Portion::Original( void ) const
{ 
  if( !IsReference() || _Original == 0 )
    return (Portion*) this;
  else
    return _Original; 
}



void Portion::AddDependency( void )
{
  if( Owner() != 0 )
  {
    if( Owner()->Type() & porNFG )
      ( (NfgPortion*) Owner() )->AddDependent( Original() );
    else if( Owner()->Type() & porEFG )
      ( (EfgPortion*) Owner() )->AddDependent( Original() );
  }
}


void Portion::RemoveDependency( void )
{
  if( Owner() != 0 )
  {
    if( Owner()->Type() & porNFG )
      ( (NfgPortion*) Owner() )->RemoveDependent( Original() );
    else if( Owner()->Type() & porEFG )
      ( (EfgPortion*) Owner() )->RemoveDependent( Original() );
  }
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
{
  Portion::Output( s );
  if( _Value == "" )
    s << "(Error)";
  else
    s << _Value;
}

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

bool ErrorPortion::IsReference( void ) const
{ return false; }

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
{ 
  Portion::Output( s );
  s << "(Reference) \"" << _Value << "\""; 
}

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

bool ReferencePortion::IsReference( void ) const
{ return false; }


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
{
  Portion::Output( s );
  s << *_Value; 
}

Portion* IntPortion::ValCopy( void ) const
{ return new IntValPortion( *_Value ); }

Portion* IntPortion::RefCopy( void ) const
{ 
  Portion* p = new IntRefPortion( *_Value ); 
  p->SetOriginal( Original() );
  return p;
}

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

bool IntValPortion::IsReference( void ) const
{ return false; }


IntRefPortion::IntRefPortion( long& value )
{ _Value = &value; }

IntRefPortion::~IntRefPortion()
{ }

bool IntRefPortion::IsReference( void ) const
{ return true; }


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
{
  Portion::Output( s );
  s << *_Value; 
}

Portion* FloatPortion::ValCopy( void ) const
{ return new FloatValPortion( *_Value ); }

Portion* FloatPortion::RefCopy( void ) const
{
  Portion* p = new FloatRefPortion( *_Value );
  p->SetOriginal( Original() );
  return p;
}

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

bool FloatValPortion::IsReference( void ) const
{ return false; }


FloatRefPortion::FloatRefPortion( double& value )
{ _Value = &value; }

FloatRefPortion::~FloatRefPortion()
{ }

bool FloatRefPortion::IsReference( void ) const
{ return true; }




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
{
  Portion::Output( s );
  s << *_Value; 
}

Portion* RationalPortion::ValCopy( void ) const
{ return new RationalValPortion( *_Value ); }

Portion* RationalPortion::RefCopy( void ) const
{ 
  Portion* p = new RationalRefPortion( *_Value ); 
  p->SetOriginal( Original() );
  return p;
}

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

bool RationalValPortion::IsReference( void ) const
{ return false; }


RationalRefPortion::RationalRefPortion( gRational& value )
{ _Value = &value; }

RationalRefPortion::~RationalRefPortion()
{ }

bool RationalRefPortion::IsReference( void ) const
{ return true; }




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
{ 
  Portion::Output( s );
  if( _WriteQuoted ) s << "\"";
  s << *_Value;
  if( _WriteQuoted ) s << "\""; 
}

Portion* TextPortion::ValCopy( void ) const
{ return new TextValPortion( *_Value ); }

Portion* TextPortion::RefCopy( void ) const
{ 
  Portion* p = new TextRefPortion( *_Value ); 
  p->SetOriginal( Original() );
  return p;
}

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

bool TextValPortion::IsReference( void ) const
{ return false; }


TextRefPortion::TextRefPortion( gString& value )
{ _Value = &value; }

TextRefPortion::~TextRefPortion()
{ }

bool TextRefPortion::IsReference( void ) const
{ return true; }







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
{
  Portion::Output( s );
  s << ( *_Value ? "True" : "False" ); 
}

Portion* BoolPortion::ValCopy( void ) const
{ return new BoolValPortion( *_Value ); }

Portion* BoolPortion::RefCopy( void ) const
{ 
  Portion* p = new BoolRefPortion( *_Value );
  p->SetOriginal( Original() );
  return p;
}

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

bool BoolValPortion::IsReference( void ) const
{ return false; }


BoolRefPortion::BoolRefPortion( bool& value )
{ _Value = &value; }

BoolRefPortion::~BoolRefPortion()
{ }

bool BoolRefPortion::IsReference( void ) const
{ return true; }





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
{ 
  assert( (*_Value)->BelongsTo() != 0 );
  switch( (*_Value)->BelongsTo()->Type() )
  {
  case DOUBLE:
    return porOUTCOME_FLOAT;
  case RATIONAL:
    return porOUTCOME_RATIONAL;
  default:
    assert( 0 );
  }
  return porUNDEFINED; 
}

void OutcomePortion::Output( gOutput& s ) const
{
    Portion::Output( s );

  s << "(Outcome) " << *_Value << " \"" << (*_Value)->GetName() << "\" ";
  (*_Value)->PrintValues( s );
}

Portion* OutcomePortion::ValCopy( void ) const
{ 
  Portion* p = new OutcomeValPortion( *_Value ); 
  p->SetOwner( Owner() );
  p->SetIsValid( IsValid() );
  p->AddDependency();
  return p;
}

Portion* OutcomePortion::RefCopy( void ) const
{ 
  Portion* p = new OutcomeRefPortion( *_Value ); 
  p->SetOriginal( Original() );
  p->SetOwner( Owner() );
  return p;
}

void OutcomePortion::AssignFrom( Portion* p )
{
  assert( p->Type() == Type() );
  RemoveDependency();
  *_Value = *( ( (OutcomePortion*) p )->_Value );
  SetOwner( p->Owner() );
  SetIsValid( p->IsValid() );
  AddDependency();
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
{
  RemoveDependency();
  delete _Value; 
}

bool OutcomeValPortion::IsReference( void ) const
{ return false; }


OutcomeRefPortion::OutcomeRefPortion( Outcome*& value )
{ _Value = &value; }

OutcomeRefPortion::~OutcomeRefPortion()
{ }

bool OutcomeRefPortion::IsReference( void ) const
{ return true; }





//---------------------------------------------------------------------
//                          NfPlayer class
//---------------------------------------------------------------------



NfPlayerPortion::NfPlayerPortion( void )
{ }

NfPlayerPortion::~NfPlayerPortion()
{ }

NFPlayer*& NfPlayerPortion::Value( void ) const
{ return *_Value; }

PortionType NfPlayerPortion::Type( void ) const
{ return porPLAYER_NFG; }

void NfPlayerPortion::Output( gOutput& s ) const
{
  Portion::Output( s );
  s << "(NfPlayer) " << *_Value << " \"" << (*_Value)->GetName() << "\""; 
}

Portion* NfPlayerPortion::ValCopy( void ) const
{
  Portion* p = new NfPlayerValPortion( *_Value ); 
  p->SetOwner( Owner() );
  p->SetIsValid( IsValid() );
  p->AddDependency();
  return p;
}

Portion* NfPlayerPortion::RefCopy( void ) const
{
  Portion* p = new NfPlayerRefPortion( *_Value ); 
  p->SetOriginal( Original() );
  p->SetOwner( Owner() );
  return p;
}

void NfPlayerPortion::AssignFrom( Portion* p )
{
  assert( p->Type() == Type() );
  RemoveDependency();
  *_Value = *( ( (NfPlayerPortion*) p )->_Value );
  SetOwner( p->Owner() );
  SetIsValid( p->IsValid() );
  AddDependency();
}

bool NfPlayerPortion::operator == ( Portion *p ) const
{
  if( p->Type() == Type() )
    return ( *_Value == *( (NfPlayerPortion*) p )->_Value );
  else
    return false;
}


NfPlayerValPortion::NfPlayerValPortion( NFPlayer* value )
{ _Value = new NFPlayer*( value ); }

NfPlayerValPortion::~NfPlayerValPortion()
{ 
  RemoveDependency();
  delete _Value; 
}

bool NfPlayerValPortion::IsReference( void ) const
{ return false; }


NfPlayerRefPortion::NfPlayerRefPortion( NFPlayer*& value )
{ _Value = &value; }

NfPlayerRefPortion::~NfPlayerRefPortion()
{ }

bool NfPlayerRefPortion::IsReference( void ) const
{ return true; }














//---------------------------------------------------------------------
//                          Strategy class
//---------------------------------------------------------------------



StrategyPortion::StrategyPortion( void )
{ }

StrategyPortion::~StrategyPortion()
{ }

Strategy*& StrategyPortion::Value( void ) const
{ return *_Value; }

PortionType StrategyPortion::Type( void ) const
{ return porSTRATEGY; }

void StrategyPortion::Output( gOutput& s ) const
{ 
  Portion::Output( s );
  s << "(Strategy) " << *_Value << " \"" << (*_Value)->name << "\""; 
}

Portion* StrategyPortion::ValCopy( void ) const
{
  Portion* p = new StrategyValPortion( *_Value ); 
  p->SetOwner( Owner() );
  p->SetIsValid( IsValid() );
  p->AddDependency();
  return p;
}

Portion* StrategyPortion::RefCopy( void ) const
{
  Portion* p = new StrategyRefPortion( *_Value ); 
  p->SetOriginal( Original() );
  p->SetOwner( Owner() );
  return p;
}

void StrategyPortion::AssignFrom( Portion* p )
{
  assert( p->Type() == Type() );
  RemoveDependency();
  *_Value = *( ( (StrategyPortion*) p )->_Value );
  SetOwner( p->Owner() );
  SetIsValid( p->IsValid() );
  AddDependency();
}

bool StrategyPortion::operator == ( Portion *p ) const
{
  if( p->Type() == Type() )
    return ( *_Value == *( (StrategyPortion*) p )->_Value );
  else
    return false;
}


StrategyValPortion::StrategyValPortion( Strategy* value )
{ _Value = new Strategy*( value ); }

StrategyValPortion::~StrategyValPortion()
{ 
  RemoveDependency();
  delete _Value; 
}

bool StrategyValPortion::IsReference( void ) const
{ return false; }


StrategyRefPortion::StrategyRefPortion( Strategy*& value )
{ _Value = &value; }

StrategyRefPortion::~StrategyRefPortion()
{ }

bool StrategyRefPortion::IsReference( void ) const
{ return true; }






//---------------------------------------------------------------------
//                          NfSupport class
//---------------------------------------------------------------------



NfSupportPortion::NfSupportPortion( void )
{ }

NfSupportPortion::~NfSupportPortion()
{ }

NFSupport*& NfSupportPortion::Value( void ) const
{ return *_Value; }

PortionType NfSupportPortion::Type( void ) const
{ return porNF_SUPPORT; }

void NfSupportPortion::Output( gOutput& s ) const
{ 
  Portion::Output( s );
  s << "(NfSupport) " << *_Value << ' ' << **_Value;  
}

Portion* NfSupportPortion::ValCopy( void ) const
{
  Portion* p = new NfSupportValPortion( *_Value ); 
  p->SetOwner( Owner() );
  p->SetIsValid( IsValid() );
  p->AddDependency();
  return p;
}

Portion* NfSupportPortion::RefCopy( void ) const
{
  Portion* p = new NfSupportRefPortion( *_Value ); 
  p->SetOriginal( Original() );
  p->SetOwner( Owner() );
  return p;
}

void NfSupportPortion::AssignFrom( Portion* p )
{
  assert( p->Type() == Type() );
  RemoveDependency();
  *_Value = *( ( (NfSupportPortion*) p )->_Value );
  SetOwner( p->Owner() );
  SetIsValid( p->IsValid() );
  AddDependency();
}

bool NfSupportPortion::operator == ( Portion *p ) const
{
  if( p->Type() == Type() )
  {
    assert( *_Value != 0 && *( (NfSupportPortion*) p )->_Value != 0 );
    // this calls the operator == in NFSupport
    return ( **_Value == **( (NfSupportPortion*) p )->_Value );
  }
  else
    return false;
}


NfSupportValPortion::NfSupportValPortion( NFSupport* value )
{ _Value = new NFSupport*( value ); }

NfSupportValPortion::~NfSupportValPortion()
{ 
  RemoveDependency();
  delete _Value; 
}

bool NfSupportValPortion::IsReference( void ) const
{ return false; }


NfSupportRefPortion::NfSupportRefPortion( NFSupport*& value )
{ _Value = &value; }

NfSupportRefPortion::~NfSupportRefPortion()
{ }

bool NfSupportRefPortion::IsReference( void ) const
{ return true; }





//---------------------------------------------------------------------
//                          EfSupport class
//---------------------------------------------------------------------


EfSupportPortion::EfSupportPortion( void )
{ }

EfSupportPortion::~EfSupportPortion()
{ }

EFSupport*& EfSupportPortion::Value( void ) const
{ return *_Value; }

PortionType EfSupportPortion::Type( void ) const
{ return porEF_SUPPORT; }

void EfSupportPortion::Output( gOutput& s ) const
{ 
  Portion::Output( s );
  s << "(EfSupport) " << *_Value << ' ' << **_Value;

}

Portion* EfSupportPortion::ValCopy( void ) const
{
  Portion* p = new EfSupportValPortion( *_Value ); 
  p->SetOwner( Owner() );
  p->SetIsValid( IsValid() );
  p->AddDependency();
  return p;
}

Portion* EfSupportPortion::RefCopy( void ) const
{
  Portion* p = new EfSupportRefPortion( *_Value ); 
  p->SetOriginal( Original() );
  p->SetOwner( Owner() );
  return p;
}

void EfSupportPortion::AssignFrom( Portion* p )
{
  assert( p->Type() == Type() );
  RemoveDependency();
  *_Value = *( ( (EfSupportPortion*) p )->_Value );
  SetOwner( p->Owner() );
  SetIsValid( p->IsValid() );
  AddDependency();
}

bool EfSupportPortion::operator == ( Portion *p ) const
{
  if( p->Type() == Type() )
  {
    assert( *_Value != 0 && *( (EfSupportPortion*) p )->_Value != 0 );
    // this calls the operator == in NFSupport
    return ( **_Value == **( (EfSupportPortion*) p )->_Value );
  }
  else
    return false;
}


EfSupportValPortion::EfSupportValPortion( EFSupport* value )
{ _Value = new EFSupport*( value ); }

EfSupportValPortion::~EfSupportValPortion()
{ 
  RemoveDependency();
  delete _Value; 
}

bool EfSupportValPortion::IsReference( void ) const
{ return false; }


EfSupportRefPortion::EfSupportRefPortion( EFSupport*& value )
{ _Value = &value; }

EfSupportRefPortion::~EfSupportRefPortion()
{ }

bool EfSupportRefPortion::IsReference( void ) const
{ return true; }





//---------------------------------------------------------------------
//                          EfPlayer class
//---------------------------------------------------------------------



EfPlayerPortion::EfPlayerPortion( void )
{ }

EfPlayerPortion::~EfPlayerPortion()
{ }

EFPlayer*& EfPlayerPortion::Value( void ) const
{ return *_Value; }

PortionType EfPlayerPortion::Type( void ) const
{ return porPLAYER_EFG; }

void EfPlayerPortion::Output( gOutput& s ) const
{
  Portion::Output( s );
  s << "(EfPlayer) " << *_Value << " \"" << (*_Value)->GetName() << "\""; 
}

Portion* EfPlayerPortion::ValCopy( void ) const
{
  Portion* p = new EfPlayerValPortion( *_Value ); 
  p->SetOwner( Owner() );
  p->SetIsValid( IsValid() );
  p->AddDependency();
  return p;
}

Portion* EfPlayerPortion::RefCopy( void ) const
{
  Portion* p = new EfPlayerRefPortion( *_Value ); 
  p->SetOriginal( Original() );
  p->SetOwner( Owner() );
  return p;
}

void EfPlayerPortion::AssignFrom( Portion* p )
{
  assert( p->Type() == Type() );
  RemoveDependency();
  *_Value = *( ( (EfPlayerPortion*) p )->_Value );
  SetOwner( p->Owner() );
  SetIsValid( p->IsValid() );
  AddDependency();
}

bool EfPlayerPortion::operator == ( Portion *p ) const
{
  if( p->Type() == Type() )
    return ( *_Value == *( (EfPlayerPortion*) p )->_Value );
  else
    return false;
}


EfPlayerValPortion::EfPlayerValPortion( EFPlayer* value )
{ _Value = new EFPlayer*( value ); }

EfPlayerValPortion::~EfPlayerValPortion()
{ 
  RemoveDependency();
  delete _Value; 
}

bool EfPlayerValPortion::IsReference( void ) const
{ return false; }


EfPlayerRefPortion::EfPlayerRefPortion( EFPlayer*& value )
{ _Value = &value; }

EfPlayerRefPortion::~EfPlayerRefPortion()
{ }

bool EfPlayerRefPortion::IsReference( void ) const
{ return true; }




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
{
  Portion::Output( s );
  s << "(Infoset) " << *_Value << " \"" << (*_Value)->GetName() << "\""; 
}

Portion* InfosetPortion::ValCopy( void ) const
{ 
  Portion* p = new InfosetValPortion( *_Value );
  p->SetOwner( Owner() );
  p->SetIsValid( IsValid() );
  p->AddDependency();
  return p;
}

Portion* InfosetPortion::RefCopy( void ) const
{
  Portion* p = new InfosetRefPortion( *_Value ); 
  p->SetOriginal( Original() );
  p->SetOwner( Owner() );
  return p;
}

void InfosetPortion::AssignFrom( Portion* p )
{
  assert( p->Type() == Type() );
  RemoveDependency();
  *_Value = *( ( (InfosetPortion*) p )->_Value );
  SetOwner( p->Owner() );
  SetIsValid( p->IsValid() );
  AddDependency();
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
{
  RemoveDependency();
  delete _Value; 
}

bool InfosetValPortion::IsReference( void ) const
{ return false; }


InfosetRefPortion::InfosetRefPortion( Infoset*& value )
{ _Value = &value; }

InfosetRefPortion::~InfosetRefPortion()
{ }

bool InfosetRefPortion::IsReference( void ) const
{ return true; }




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
{
  Portion::Output( s );
  s << "(Node) " << *_Value << " \"" << (*_Value)->GetName() << "\""; 
}

Portion* NodePortion::ValCopy( void ) const
{
  Portion* p = new NodeValPortion( *_Value ); 
  p->SetOwner( Owner() );
  p->SetIsValid( IsValid() );
  p->AddDependency();
  return p;
}

Portion* NodePortion::RefCopy( void ) const
{
  Portion* p = new NodeRefPortion( *_Value ); 
  p->SetOriginal( Original() );
  p->SetOwner( Owner() );
  return p;
}

void NodePortion::AssignFrom( Portion* p )
{
  assert( p->Type() == Type() );
  RemoveDependency();
  *_Value = *( ( (NodePortion*) p )->_Value );
  SetOwner( p->Owner() );
  SetIsValid( p->IsValid() );
  AddDependency();
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
{ 
  RemoveDependency();
  delete _Value; 
}

bool NodeValPortion::IsReference( void ) const
{ return false; }


NodeRefPortion::NodeRefPortion( Node*& value )
{ _Value = &value; }

NodeRefPortion::~NodeRefPortion()
{ }

bool NodeRefPortion::IsReference( void ) const
{ return true; }






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
{
  Portion::Output( s );
  s << "(Action) " << *_Value << " \"" << (*_Value)->GetName() << "\""; 
}

Portion* ActionPortion::ValCopy( void ) const
{
  Portion* p = new ActionValPortion( *_Value ); 
  p->SetOwner( Owner() );
  p->SetIsValid( IsValid() );
  p->AddDependency();
  return p;
}

Portion* ActionPortion::RefCopy( void ) const
{
  Portion* p = new ActionRefPortion( *_Value ); 
  p->SetOriginal( Original() );
  p->SetOwner( Owner() );
  return p;
}

void ActionPortion::AssignFrom( Portion* p )
{
  assert( p->Type() == Type() );
  RemoveDependency();
  *_Value = *( ( (ActionPortion*) p )->_Value );
  SetOwner( p->Owner() );
  SetIsValid( p->IsValid() );
  AddDependency();
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
{ 
  RemoveDependency();
  delete _Value; 
}

bool ActionValPortion::IsReference( void ) const
{ return false; }


ActionRefPortion::ActionRefPortion( Action*& value )
{ _Value = &value; }

ActionRefPortion::~ActionRefPortion()
{ }

bool ActionRefPortion::IsReference( void ) const
{ return true; }







//---------------------------------------------------------------------
//                            new Mixed class
//---------------------------------------------------------------------



MixedPortion::MixedPortion( void )
{ }

MixedPortion::~MixedPortion()
{ }

BaseMixedProfile*& MixedPortion::Value( void ) const
{ return *_Value; }

PortionType MixedPortion::Type( void ) const
{ 
  if( !*_Value )
  {
    return porMIXED;
  }
  else
  {
    switch( (*_Value)->Type() )
    {
    case DOUBLE:
      return porMIXED_FLOAT;
    case RATIONAL:
      return porMIXED_RATIONAL;
    default:
      assert( 0 );
    }
  }
  return porUNDEFINED;
}

void MixedPortion::Output( gOutput& s ) const
{
  Portion::Output( s );
  if( !*_Value )
  {
    s << "(Mixed) NULL"; 
  }
  else
  {
    switch( (*_Value)->Type() )
    {
    case DOUBLE:
      s << "(Mixed) " << * (MixedSolution<double>*) (*_Value); 
      break;
    case RATIONAL:
      s << "(Mixed) " << * (MixedSolution<gRational>*) (*_Value); 
      break;
    default:
      assert( 0 );
    }
  }
}

Portion* MixedPortion::ValCopy( void ) const
{ 
  Portion* p = 0;
  if( !*_Value )
  {
    p = new MixedValPortion( 0 );
  }
  else
  {
    switch( (*_Value)->Type() )
    {
    case DOUBLE:
      p = new MixedValPortion
	( new MixedSolution<double>
	 ( * (MixedSolution<double>*) (*_Value) ) ); 
      break;
    case RATIONAL:
      p = new MixedValPortion
	( new MixedSolution<gRational>
	 ( * (MixedSolution<gRational>*) (*_Value) ) );       
      break;
    default:
      assert( 0 );
    }
  }
  p->SetOwner( Owner() );
  p->SetIsValid( IsValid() );
  p->AddDependency();
  return p;
}

Portion* MixedPortion::RefCopy( void ) const
{ 
  Portion* p = new MixedRefPortion( *_Value ); 
  p->SetOriginal( Original() );
  p->SetOwner( Owner() );
  return p;
}

void MixedPortion::AssignFrom( Portion* p )
{
  assert( p->Type() & porMIXED );

  RemoveDependency();

  delete *_Value;

  if( !( (MixedPortion*) p )->Value() )
  {
    *_Value = 0;
  }
  else
  {
    switch( ( (MixedPortion*) p )->Value()->Type() )
    {
    case DOUBLE:
      *_Value = new MixedSolution<double>
	( * (MixedSolution<double>*) ( (MixedPortion*) p )->Value() ); 
      break;
    case RATIONAL:
      *_Value =  new MixedSolution<gRational>
	( * (MixedSolution<gRational>*) ( (MixedPortion*) p )->Value() ); 
      break;
    default:
      assert( 0 );
    }
  }

  SetOwner( p->Owner() );
  SetIsValid( p->IsValid() );
  AddDependency();
}

bool MixedPortion::operator == ( Portion *p ) const
{
  if( p->Type() == Type() )
    return ( *_Value == *( (MixedPortion*) p )->_Value );
  else
    return false;
}


MixedValPortion::MixedValPortion( BaseMixedProfile* value )
{ _Value = new BaseMixedProfile*( value ); }

MixedValPortion::~MixedValPortion()
{ 
  RemoveDependency();
  delete *_Value;
  delete _Value; 
}

bool MixedValPortion::IsReference( void ) const
{ return false; }


MixedRefPortion::MixedRefPortion( BaseMixedProfile*& value )
{ _Value = &value; }

MixedRefPortion::~MixedRefPortion()
{ }

bool MixedRefPortion::IsReference( void ) const
{ return true; }







//---------------------------------------------------------------------
//                            new Behav class
//---------------------------------------------------------------------



BehavPortion::BehavPortion( void )
{ }

BehavPortion::~BehavPortion()
{ }

BaseBehavProfile*& BehavPortion::Value( void ) const
{ return *_Value; }

PortionType BehavPortion::Type( void ) const
{ 
  if( !*_Value )
  {
    return porBEHAV;
  }
  else
  {
    switch( (*_Value)->Type() )
    {
    case DOUBLE:
      return porBEHAV_FLOAT;
    case RATIONAL:
      return porBEHAV_RATIONAL;
    default:
      assert( 0 );
    }
    return porUNDEFINED;
  }
}

void BehavPortion::Output( gOutput& s ) const
{
  Portion::Output( s );
  if( !*_Value )
  {
    s << "(Behav) NULL"; 
  }
  else
  {
    switch( (*_Value)->Type() )
    {
    case DOUBLE:
      s << "(Behav) " << * (BehavSolution<double>*) (*_Value); 
      break;
    case RATIONAL:
      s << "(Behav) " << * (BehavSolution<gRational>*) (*_Value); 
      break;
    default:
      assert( 0 );
    }
  }
}

Portion* BehavPortion::ValCopy( void ) const
{ 
  Portion* p;

  if( !*_Value )
  {
    p = new BehavValPortion( 0 );
  }
  else
  {
    switch( (*_Value)->Type() )
    {
    case DOUBLE:
      p = new BehavValPortion
	( new BehavSolution<double>
	 ( * (BehavSolution<double>*) (*_Value) ) ); 
      break;
    case RATIONAL:
      p = new BehavValPortion
	( new BehavSolution<gRational>
	 ( * (BehavSolution<gRational>*) (*_Value) ) ); 
      break;
    default:
      assert( 0 );
    }
  }
  p->SetOwner( Owner() );
  p->SetIsValid( IsValid() );
  p->AddDependency();
  return p;
}

Portion* BehavPortion::RefCopy( void ) const
{ 
  Portion* p = new BehavRefPortion( *_Value ); 
  p->SetOriginal( Original() );
  p->SetOwner( Owner() );
  return p;
}

void BehavPortion::AssignFrom( Portion* p )
{
  assert( p->Type() & porBEHAV );

  RemoveDependency();

  delete *_Value;

  if( !( (BehavPortion*) p )->Value() )
  {
    *_Value = 0;
  }
  else
  {
    switch( ( (BehavPortion*) p )->Value()->Type() )
    {
    case DOUBLE:
      *_Value = new BehavSolution<double>
	( * (BehavSolution<double>*) ( (BehavPortion*) p )->Value() ); 
      break;
    case RATIONAL:
      *_Value =  new BehavSolution<gRational>
	( * (BehavSolution<gRational>*) ( (BehavPortion*) p )->Value() ); 
      break;
    default:
      assert( 0 );
    }
  }

  SetOwner( p->Owner() );
  SetIsValid( p->IsValid() );
  AddDependency();
}

bool BehavPortion::operator == ( Portion *p ) const
{
  if( p->Type() == Type() )
    return ( *_Value == *( (BehavPortion*) p )->_Value );
  else
    return false;
}


BehavValPortion::BehavValPortion( BaseBehavProfile* value )
{ _Value = new BaseBehavProfile*( value ); }

BehavValPortion::~BehavValPortion()
{ 
  RemoveDependency();
  delete *_Value;
  delete _Value; 
}

bool BehavValPortion::IsReference( void ) const
{ return false; }


BehavRefPortion::BehavRefPortion( BaseBehavProfile*& value )
{ _Value = &value; }

BehavRefPortion::~BehavRefPortion()
{ }

bool BehavRefPortion::IsReference( void ) const
{ return true; }








//---------------------------------------------------------------------
//                            new Nfg class
//---------------------------------------------------------------------



NfgPortion::NfgPortion( void )
{ 
  _Dependent = 0;
}

NfgPortion::~NfgPortion()
{ }

BaseNfg*& NfgPortion::Value( void ) const
{ return *_Value; }

PortionType NfgPortion::Type( void ) const
{ 
  assert( (*_Value) != 0 );
  switch( (*_Value)->Type() )
  {
  case DOUBLE:
    return porNFG_FLOAT;
  case RATIONAL:
    return porNFG_RATIONAL;
  default:
    assert( 0 );
  }
  return porUNDEFINED;
}

void NfgPortion::Output( gOutput& s ) const
{
  Portion::Output( s );
  s << "(Nfg) \"" << (*_Value)->GetTitle() << "\""; 
}

Portion* NfgPortion::ValCopy( void ) const
{ 
  switch( (*_Value)->Type() )
  {
  case DOUBLE:
    return new NfgValPortion
      ( new Nfg<double>( * (Nfg<double>*) (*_Value) ) ); 
    break;
  case RATIONAL:
    return new NfgValPortion
      ( new Nfg<gRational>( * (Nfg<gRational>*) (*_Value) ) ); 
    break;
  default:
    assert( 0 );
  }
  return 0;
}

Portion* NfgPortion::RefCopy( void ) const
{ 
  Portion* p = new NfgRefPortion( *_Value ); 
  p->SetOriginal( Original() );
  p->SetOwner( Owner() );
  return p;
}

void NfgPortion::AssignFrom( Portion* p )
{
  assert( p->Type() & porNFG );

  while( ( (NfgPortion*) Original() )->_Dependent->Length() > 0 )
    ( (NfgPortion*) Original() )->_Dependent->Remove( 1 )->SetIsValid( false );
  delete *_Value;

  switch( ( (NfgPortion*) p )->Value()->Type() )
  {
  case DOUBLE:
    *_Value = new Nfg<double>
      ( * (Nfg<double>*) ( (NfgPortion*) p )->Value() ); 
    break;
  case RATIONAL:
    *_Value =  new Nfg<gRational>
      ( * (Nfg<gRational>*) ( (NfgPortion*) p )->Value() ); 
    break;
  default:
    assert( 0 );
  }
}

bool NfgPortion::operator == ( Portion *p ) const
{
  if( p->Type() == Type() )
    return ( *_Value == *( (NfgPortion*) p )->_Value );
  else
    return false;
}




void NfgPortion::AddDependent( Portion* p )
{
  if( ! ( (NfgPortion*) Original() )->_Dependent->Find( p ) )
    ( * ( (NfgPortion*) Original() )->_Dependent ) += p;
}


void NfgPortion::RemoveDependent( Portion* p )
{
  int index;
  index = ( (NfgPortion*) Original() )->_Dependent->Find( p );
  if( index )
    ( (NfgPortion*) Original() )->_Dependent->Remove( index );
}




NfgValPortion::NfgValPortion( BaseNfg* value )
{
  _Value = new BaseNfg*( value ); 
  _Dependent = new gList< Portion* >;
}

NfgValPortion::~NfgValPortion()
{ 
  while( _Dependent->Length() > 0 )
    _Dependent->Remove( 1 )->SetIsValid( false );
  delete _Dependent;
  delete *_Value;
  delete _Value; 
}

bool NfgValPortion::IsReference( void ) const
{ return false; }


NfgRefPortion::NfgRefPortion( BaseNfg*& value )
{ _Value = &value; }

NfgRefPortion::~NfgRefPortion()
{ }

bool NfgRefPortion::IsReference( void ) const
{ return true; }









//---------------------------------------------------------------------
//                            new Efg class
//---------------------------------------------------------------------



EfgPortion::EfgPortion( void )
{
  _Dependent = 0;
}

EfgPortion::~EfgPortion()
{ }

BaseEfg*& EfgPortion::Value( void ) const
{ return *_Value; }

PortionType EfgPortion::Type( void ) const
{ 
  assert( (*_Value) != 0 );
  switch( (*_Value)->Type() )
  {
  case DOUBLE:
    return porEFG_FLOAT;
  case RATIONAL:
    return porEFG_RATIONAL;
  default:
    assert( 0 );
  }
  return porUNDEFINED;
}

void EfgPortion::Output( gOutput& s ) const
{
  Portion::Output( s );
  s << "(Efg) \"" << (*_Value)->GetTitle() << "\""; 
}

Portion* EfgPortion::ValCopy( void ) const
{ 
  switch( (*_Value)->Type() )
  {
  case DOUBLE:
    return new EfgValPortion
      ( new Efg<double>( * (Efg<double>*) (*_Value) ) ); 
    break;
  case RATIONAL:
    return new EfgValPortion
      ( new Efg<gRational>( * (Efg<gRational>*) (*_Value) ) ); 
    break;
  default:
    assert( 0 );
  }
  return 0;
}

Portion* EfgPortion::RefCopy( void ) const
{ 
  Portion* p = new EfgRefPortion( *_Value ); 
  p->SetOriginal( Original() );
  p->SetOwner( Owner() );
  return p;
}

void EfgPortion::AssignFrom( Portion* p )
{
  assert( p->Type() & porEFG );

  while( ( (EfgPortion*) Original() )->_Dependent->Length() > 0 )
    ( (EfgPortion*) Original() )->_Dependent->Remove( 1 )->SetIsValid( false );
  delete *_Value;

  switch( ( (EfgPortion*) p )->Value()->Type() )
  {
  case DOUBLE:
    *_Value = new Efg<double>
      ( * (Efg<double>*) ( (EfgPortion*) p )->Value() ); 
    break;
  case RATIONAL:
    *_Value =  new Efg<gRational>
      ( * (Efg<gRational>*) ( (EfgPortion*) p )->Value() ); 
    break;
  default:
    assert( 0 );
  }
}

bool EfgPortion::operator == ( Portion *p ) const
{
  if( p->Type() == Type() )
    return ( *_Value == *( (EfgPortion*) p )->_Value );
  else
    return false;
}


void EfgPortion::AddDependent( Portion* p )
{
  if( ! ( (EfgPortion*) Original() )->_Dependent->Find( p ) )
    ( * ( (EfgPortion*) Original() )->_Dependent ) += p;
}


void EfgPortion::RemoveDependent( Portion* p )
{
  int index;
  index = ( (EfgPortion*) Original() )->_Dependent->Find( p );
  if( index )
    ( (EfgPortion*) Original() )->_Dependent->Remove( index );
}



EfgValPortion::EfgValPortion( BaseEfg* value )
{ 
  _Value = new BaseEfg*( value ); 
  _Dependent = new gList< Portion* >;
}

EfgValPortion::~EfgValPortion()
{ 
  while( _Dependent->Length() > 0 )
    _Dependent->Remove( 1 )->SetIsValid( false );
  delete _Dependent;
  delete *_Value;
  delete _Value; 
}

bool EfgValPortion::IsReference( void ) const
{ return false; }


EfgRefPortion::EfgRefPortion( BaseEfg*& value )
{ _Value = &value; }

EfgRefPortion::~EfgRefPortion()
{ }

bool EfgRefPortion::IsReference( void ) const
{ return true; }




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
{
  Portion::Output( s );
  s << "(Output)"; 
}

Portion* OutputPortion::ValCopy( void ) const
{ return RefCopy(); }

Portion* OutputPortion::RefCopy( void ) const
{ 
  Portion* p = new OutputRefPortion( *_Value ); 
  p->SetOriginal( Original() );
  return p;
}

void OutputPortion::AssignFrom( Portion* )
{
/*
  assert( p->Type() == Type() );
  *_Value = *( ( (OutputPortion*) p )->_Value );
*/
  assert( 0 );
}

bool OutputPortion::operator == ( Portion * ) const
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

bool OutputValPortion::IsReference( void ) const
{ return false; }


OutputRefPortion::OutputRefPortion( gOutput& value )
{ _Value = &value; }

OutputRefPortion::~OutputRefPortion()
{ }

bool OutputRefPortion::IsReference( void ) const
{ return true; }




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
{
  Portion::Output( s );
  s << "(Input)"; 
}

Portion* InputPortion::ValCopy( void ) const
{  return RefCopy(); }

Portion* InputPortion::RefCopy( void ) const
{ 
  Portion* p = new InputRefPortion( *_Value ); 
  p->SetOriginal( Original() );
  return p;
}

void InputPortion::AssignFrom( Portion* )
{
/*
  assert( p->Type() == Type() );
  *_Value = *( ( (InputPortion*) p )->_Value );
*/
  assert( 0 );
}

bool InputPortion::operator == ( Portion *) const
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

bool InputValPortion::IsReference( void ) const
{ return false; }


InputRefPortion::InputRefPortion( gInput& value )
{ _Value = &value; }

InputRefPortion::~InputRefPortion()
{ }

bool InputRefPortion::IsReference( void ) const
{ return true; }



//---------------------------------------------------------------------
//                          List class
//---------------------------------------------------------------------

// #include "gblock.h"
#include "glist.h"

ListPortion::ListPortion( void )
{ 
  _ContainsListsOnly = true;
  _DataType = porUNDEFINED;
}

ListPortion::~ListPortion()
{ }


gList< Portion* >& ListPortion::Value( void ) const
{ return *_Value; }


bool ListPortion::IsValid( void ) const
{
  bool result;
  int i;
  int length = _Value->Length();

  // Portion::AddDependency();
  if( length > 0 )
  {
    result = false;
    for( i = 1; i <= length; i++ )
    {
      result = result || (*_Value)[ i ]->IsValid();
    }
  }
  else
    result = true;
  
  return result && Portion::IsValid();
}


void ListPortion::AddDependency( void )
{ 
  int i;
  int length;

  // Portion::AddDependency();
  for( i = 1, length = _Value->Length(); i <= length; i++ )
  {
    (*_Value)[ i ]->AddDependency();
  }
}

void ListPortion::RemoveDependency( void )
{ 
  int i;
  int length;

  // Portion::RemoveDependency();
  for( i = 1, length = _Value->Length(); i <= length; i++ )
  {
    (*_Value)[ i ]->RemoveDependency();
  }
}

void ListPortion::SetOwner( Portion* p )
{ 
  int i;
  int length;

  // Portion::SetOwner( p );
  for( i = 1, length = _Value->Length(); i <= length; i++ )
  {
    (*_Value)[ i ]->SetOwner( p );
  }
}



PortionType ListPortion::Type( void ) const
{ return porLIST; }

Portion* ListPortion::ValCopy( void ) const
{ 
  ListPortion* p =new ListValPortion( *_Value ); 
  // p->SetOwner( Owner() );
  if( p->DataType() == porUNDEFINED )
    p->SetDataType( _DataType );
  p->AddDependency();
  return p;
}

Portion* ListPortion::RefCopy( void ) const
{ 
  ListPortion* p = new ListRefPortion( *_Value ); 
  ( (ListPortion*) p )->_DataType = _DataType;
  p->SetOriginal( Original() );
  // p->SetOwner( Owner() );
  return p;
}

void ListPortion::AssignFrom( Portion* p )
{
  int i;
  int length;
  int result;
  //gBlock< Portion* >& value = *( ( (ListPortion*) p )->_Value );
  gList< Portion* >& value = *( ( (ListPortion*) p )->_Value );

  assert( p->Type() == Type() );
  assert( PortionTypeMatch( ( (ListPortion*) p )->_DataType, _DataType ) || 
	 _DataType == porUNDEFINED || 
	 ( (ListPortion*) p )->_DataType == porUNDEFINED );

  RemoveDependency();

  Flush();

  for( i = 1, length = value.Length(); i <= length; i++ )
  {
    result = Insert( value[ i ]->ValCopy(), i );
    assert( result != 0 );
  }
  if( _DataType == porUNDEFINED )
    _DataType = ( (ListPortion*) p )->_DataType;

  // SetOwner( p->Owner() );

  AddDependency();
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
{ 
  // _Value = new gBlock< Portion* >;
  _Value = new gList< Portion* >;
}

ListValPortion::ListValPortion( gList< Portion* >& value )
{ 
  int i;
  int length;
  int result;

  _Value = new gList< Portion* >; 

  for( i = 1, length = value.Length(); i <= length; i++ )
  {
    result = Insert( value[ i ]->ValCopy(), i );
    assert( result != 0 );
  }
}

ListValPortion::~ListValPortion()
{
  RemoveDependency();
  Flush();
  delete _Value;
}

bool ListValPortion::IsReference( void ) const
{ return false; }


// ListRefPortion::ListRefPortion( gBlock< Portion* >& value )
ListRefPortion::ListRefPortion( gList< Portion* >& value )
{ _Value = &value; }

ListRefPortion::~ListRefPortion()
{ }

bool ListRefPortion::IsReference( void ) const
{ return true; }









bool ListPortion::ContainsListsOnly( void ) const
{
  if( _Value->Length() == 0 )
    return false;
  else
    return _ContainsListsOnly;
}


void ListPortion::SetDataType( PortionType data_type )
{
  assert( _DataType == porUNDEFINED );
  ( (ListPortion*) Original() )->_DataType = data_type;
  _DataType = data_type;
}

PortionType ListPortion::DataType( void ) const
{ return _DataType; }


void ListPortion::Output( gOutput& s ) const
{ Output( s, 0 ); }

void ListPortion::Output( gOutput& s, long ListLF ) const
{
  Portion::Output( s );
  int i;
  int c;
  int length = _Value->Length();

  if( _WriteListBraces ) s << '{';
  if( _WriteListLF > ListLF ) s << '\n';
  if( length >= 1 )
  {
    for( i = 1; i <= length; i++ )
    {
      if( i > 1 )
      {
	if( _WriteListCommas ) s << ',';
	if( _WriteListLF > ListLF ) 
	  s << '\n';
      }
      if( _WriteListLF > ListLF ) 
	for( c = 0; c < (ListLF+1) * _WriteListIndent; c++ )
	  s << ' ';
      if( (*_Value)[ i ]->IsValid() )
      {
	if( _WriteListLF <= ListLF )
	  s << ' ';
	if( (*_Value)[ i ]->Type() != porLIST )
	  s << (*_Value)[ i ];
	else
	  ((ListPortion*) (*_Value)[ i ])->Output( s, ListLF + 1 );
      }
      else
	s << " (undefined)";
    }
  }
  else
  {
    s << " empty";
  }

  s << ' ';
  if( _WriteListLF > ListLF ) 
  {
    s << '\n';
    for( c = 0; c < ListLF * _WriteListIndent; c++ )
      s << ' ';
  }
  if( _WriteListBraces )
    s << '}';
}



int ListPortion::Append( Portion* item )
{ return Insert( item, _Value->Length() + 1 ); }


int ListPortion::Insert( Portion* item, int index )
{
  int result = 0;
  PortionType item_type;

#ifndef NDEBUG
  if( item->Type() == porREFERENCE )
  {
    gerr << "Portion Error:\n";
    gerr << "  Attempted to insert a ReferencePortion into\n";
    gerr << "  a ListPortion\n";
  }
  assert( item->Type() != porREFERENCE );
#endif
  
  if( item->Type() == porLIST )
    item_type = ( (ListPortion*) item )->_DataType;
  else
  {
    item_type = item->Type();
    _ContainsListsOnly = false;
  }


  if( _DataType == porUNDEFINED )
  {
    if( _Value->Length() == 0 )
      _Owner = item->Original()->Owner();
    _DataType = item_type;
    ((ListPortion*) Original())->_DataType = item_type;
    result = _Value->Insert( item, index );
  }
  else  // inserting into an existing list
  {
    if( PortionTypeMatch( item_type, _DataType ) )
    {
      if( _Value->Length() == 0 )
	_Owner = item->Original()->Owner();
      result = _Value->Insert( item, index );
    }
    else if( item_type == porUNDEFINED )
    {
      if( _Value->Length() == 0 )
	_Owner = item->Original()->Owner();
      result = _Value->Insert( item, index );
      assert( item->Type() == porLIST );
      ((ListPortion*) item)->SetDataType( _DataType );
    }
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
  Portion* result;
  if( index >= 1 && index <= _Value->Length() )
    result = _Value->Remove( index );
  else
    result = 0;
  if( _Value->Length() == 0 )
    _ContainsListsOnly = true;
  return result;
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


Portion* ListPortion::operator[]( int index )
{
  if( index >= 1 && index <= _Value->Length() )
  {
    assert( (*_Value)[ index ] != 0 );
    return (*_Value)[ index ];
  }
  else
    return 0;
}



Portion* ListPortion::Subscript( int index )
{
  Portion* p;
  if( index >= 1 && index <= _Value->Length() )
  {
    assert( (*_Value)[ index ] != 0 );

    if( IsReference() )
      p = (*_Value)[ index ]->RefCopy();
    else
      p = (*_Value)[ index ]->ValCopy();
      
    p->SetIsValid( (*_Value)[ index ]->IsValid() );
    return p;
  }
  else
    return 0;
}
//-------------------------------------------------------------------
//--------------------------------------------------------------------



bool PortionTypeMatch( const PortionType& t1, const PortionType& t2 )
{
  if( t1 == t2 )
    return true;
  else if(( ( t1 & porMIXED   ) && ( t2 & porMIXED   ) ) ||
	  ( ( t1 & porBEHAV   ) && ( t2 & porBEHAV   ) ) ||
	  ( ( t1 & porOUTCOME ) && ( t2 & porOUTCOME ) ) ||
	  ( ( t1 & porNFG     ) && ( t2 & porNFG     ) ) ||
	  ( ( t1 & porEFG     ) && ( t2 & porEFG     ) ))
    return true;
  else
    return false;
}





struct PortionTypeTextType
{
  PortionType Type;
  char * Text;
};  


#define NumPortionTypes 37

PortionTypeTextType _PortionTypeText[] =
{
  { porERROR,            "ERROR" },
  
  { porBOOL,             "BOOL" },
  { porFLOAT,            "FLOAT" },
  { porINTEGER,          "INTEGER" },
  { porRATIONAL,         "RATIONAL" },
  { porTEXT,             "TEXT" },
  { porLIST,             "LIST" },
  { porNFG_FLOAT,        "NFG(FLOAT)" },
  { porNFG_RATIONAL,     "NFG(RATIONAL)" },
  { porEFG_FLOAT,        "EFG(FLOAT)" },
  { porEFG_RATIONAL,     "EFG(RATIONAL)" },
  { porMIXED_FLOAT,      "MIXED(FLOAT)" },
  { porMIXED_RATIONAL,   "MIXED(RATIONAL)" },
  { porBEHAV_FLOAT,      "BEHAV(FLOAT)" },
  { porBEHAV_RATIONAL,   "BEHAV(RATIONAL)" },

  { porOUTCOME_FLOAT,    "OUTCOME(FLOAT)" },
  { porOUTCOME_RATIONAL, "OUTCOME(RATIONAL)" },
  { porPLAYER_NFG,       "PLAYER(NFG)" },
  { porPLAYER_EFG,       "PLAYER(EFG)" },
  { porNF_SUPPORT,       "SUPPORT(NFG)" },
  { porEF_SUPPORT,       "SUPPORT(EFG)" },
  { porINFOSET,          "INFOSET" },
  { porNODE,             "NODE" },
  { porACTION,           "ACTION" },

  { porREFERENCE,        "REFERENCE" },

  { porOUTPUT,           "OUTPUT" },
  { porINPUT,            "INPUT" },

  { porUNDEFINED,        "UNDEFINED" },

  { porNFG,              "NFG" },
  { porEFG,              "EFG" },
  { porMIXED,            "MIXED" },
  { porBEHAV,            "BEHAV" },
  { porOUTCOME,          "OUTCOME" },
  { porPLAYER,           "PLAYER" },

  { porNUMERICAL,        "NUMERICAL" },
  { porANYLIST,          "ANYLIST" },
  { porANYTYPE,          "ANYTYPE" }
};




gString PortionTypeToText( const PortionType& type )
{
  int i;
  gString result = "";


  for( i = NumPortionTypes - 9; i < NumPortionTypes; i++ )
  {
    if( type == _PortionTypeText[ i ].Type )
      result = (gString) " " + _PortionTypeText[ i ].Text;
  }

  if( result == "" )
  {
    if( type & porLIST )
    {
      for( i = 0; i < NumPortionTypes - 9; i++ )
      {
	if( ( _PortionTypeText[ i ].Type & type ) && 
	   ( _PortionTypeText[ i ].Type != porLIST ) )
	{
	  result = result + " LIST(" + _PortionTypeText[ i ].Text + ")";
	}
      }
      if( result == "" )
	result = " LIST";
    }
    else
    {
      for( i = 0; i < NumPortionTypes - 9; i++ )
      {
	if( _PortionTypeText[ i ].Type & type )
	{
	  result = result + " " + _PortionTypeText[ i ].Text;
	}
      }
    }
  }

  if( result == "" )
    result = (gString) " " + _PortionTypeText[ 0 ].Text;
  return result;
}


PortionType TextToPortionType( const gString& text )
{
  int i;
  PortionType result = _PortionTypeText[ 0 ].Type;

  for( i = 0; i < NumPortionTypes; i++ )
  {
    if( strstr( (const char *)text, _PortionTypeText[ i ].Text ) )
    {
      if( !strstr( (const char *)text, (gString) "(" + _PortionTypeText[ i ].Text + ")" ) )
      {
	if( !( result & _PortionTypeText[ i ].Type ) )
	  result = result | _PortionTypeText[ i ].Type;
      }
      else if( strstr((const char *) text, (gString) "LIST(" + 
		      _PortionTypeText[ i ].Text + ")" ) )
	result = result | _PortionTypeText[ i ].Type;	
    }
  }
  return result;
}

int TextToPortionListDepth( const gString& text )
{
  int result = 0;
  while( ( text.length() > result * 5 ) &&
	( strstr((const char *) text.right(text.length()-result*5),
		 (gString) "LIST(" ) ) )
    result++;
  return result;
}



void PrintPortionTypeSpec( gOutput& s, PortionType type )
{
  s << PortionTypeToText( type );
}




gOutput& operator << ( gOutput& s, Portion* p )
{
  p->Output( s );
  return s;
}

