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
#include "nfplayer.h"
#include "nfstrat.h"
#include "efstrat.h"

#include "mixedsol.h"
#include "behavsol.h"


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
long Portion::_WriteSolutionInfo = 2;

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
void Portion::_SetWriteSolutionInfo( long x )
{ _WriteSolutionInfo = x; }



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
  printf("--- Portion Ctor, count: %ld\n", _NumObj);
#endif
}

Portion::~Portion()
{ 
#ifdef MEMCHECK
  _NumObj--;
  printf("--- Portion Dtor, count: %ld\n", _NumObj);
#endif
}

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
    if( Owner()->Spec().Type & porNFG )
      ( (NfgPortion*) Owner() )->AddDependent( Original() );
    else if( Owner()->Spec().Type & porEFG )
      ( (EfgPortion*) Owner() )->AddDependent( Original() );
  }
}


void Portion::RemoveDependency( void )
{
  if( Owner() != 0 )
  {
    if( Owner()->Spec().Type & porNFG )
      ( (NfgPortion*) Owner() )->RemoveDependent( Original() );
    else if( Owner()->Spec().Type & porEFG )
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

PortionSpec ErrorPortion::Spec( void ) const
{ return PortionSpec(porERROR); }

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

PortionSpec ReferencePortion::Spec( void ) const
{ return PortionSpec(porREFERENCE); }

void ReferencePortion::Output( gOutput& s ) const
{ 
  Portion::Output( s );
  s << "(Reference) \"" << _Value << "\""; 
}

Portion* ReferencePortion::ValCopy( void ) const
{ return new ReferencePortion( _Value ); }

Portion* ReferencePortion::RefCopy( void ) const
{ return new ReferencePortion( _Value ); }

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

PortionSpec IntPortion::Spec( void ) const
{ return PortionSpec(porINTEGER); }

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

PortionSpec FloatPortion::Spec( void ) const
{ return PortionSpec(porFLOAT); }

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

PortionSpec RationalPortion::Spec( void ) const
{ return PortionSpec(porRATIONAL); }

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

PortionSpec TextPortion::Spec( void ) const
{ return PortionSpec(porTEXT); }

void TextPortion::Output( gOutput& s ) const
{ 
  Portion::Output( s );
  gString text = *_Value;
  int i;
  for( i = 0; i < text.length(); i++ )
    if( text[ i ] == '\\' && text[ i + 1 ] == 'n' )
    {
      text.remove( i );
      text[ i ] = '\n';
    }

  if( _WriteQuoted ) s << "\"";
  s << text;
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

PortionSpec BoolPortion::Spec( void ) const
{ return PortionSpec(porBOOL); }

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

PortionSpec OutcomePortion::Spec( void ) const
{ 
  assert( (*_Value)->BelongsTo() != 0 );
  switch( (*_Value)->BelongsTo()->Type() )
  {
  case DOUBLE:
    return PortionSpec(porOUTCOME_FLOAT);
  case RATIONAL:
    return PortionSpec(porOUTCOME_RATIONAL);
  default:
    assert( 0 );
  }
  return porUNDEFINED; 
}

void OutcomePortion::Output( gOutput& s ) const
{
  Portion::Output( s );
  
  s << "(Outcome) " << *_Value;
  if(*_Value)
  {
    s << " \"" << (*_Value)->GetName() << "\" ";
    (*_Value)->PrintValues( s );
  }
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

PortionSpec NfPlayerPortion::Spec( void ) const
{ return PortionSpec(porPLAYER_NFG); }

void NfPlayerPortion::Output( gOutput& s ) const
{
  Portion::Output( s );
  s << "(NfPlayer) " << *_Value;
  if(*_Value)
    s << " \"" << (*_Value)->GetName() << "\""; 
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

PortionSpec StrategyPortion::Spec( void ) const
{ return PortionSpec(porSTRATEGY); }

void StrategyPortion::Output( gOutput& s ) const
{ 
  Portion::Output( s );
  s << "(Strategy) " << *_Value;
  if(*_Value)
    s << " \"" << (*_Value)->name << "\""; 
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

PortionSpec NfSupportPortion::Spec( void ) const
{ return PortionSpec(porNF_SUPPORT); }

void NfSupportPortion::Output( gOutput& s ) const
{ 
  Portion::Output( s );
  s << "(NfSupport) " << *_Value;
  if(*_Value) 
    s << ' ' << **_Value;  
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

PortionSpec EfSupportPortion::Spec( void ) const
{ return PortionSpec(porEF_SUPPORT); }

void EfSupportPortion::Output( gOutput& s ) const
{ 
  Portion::Output( s );
  s << "(EfSupport) " << *_Value;
  if(*_Value) 
    s << ' ' << **_Value;
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

PortionSpec EfPlayerPortion::Spec( void ) const
{ return PortionSpec(porPLAYER_EFG); }

void EfPlayerPortion::Output( gOutput& s ) const
{
  Portion::Output( s );
  s << "(EfPlayer) " << *_Value;
  if(*_Value)
    s << " \"" << (*_Value)->GetName() << "\""; 
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

bool InfosetPortion::IsValid( void ) const
{ 
  assert(*_Value);
  return _IsValid && (*_Value)->IsValid();
}

Infoset*& InfosetPortion::Value( void ) const
{ return *_Value; }

PortionSpec InfosetPortion::Spec( void ) const
{ return PortionSpec(porINFOSET); }

void InfosetPortion::Output( gOutput& s ) const
{
  Portion::Output( s );
  s << "(Infoset) " << *_Value;
  if(*_Value)
    s << " \"" << (*_Value)->GetName() << "\""; 
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

bool NodePortion::IsValid( void ) const
{ 
  assert(*_Value);
  return _IsValid && (*_Value)->IsValid();
}

Node*& NodePortion::Value( void ) const
{ return *_Value; }

PortionSpec NodePortion::Spec( void ) const
{ return PortionSpec(porNODE); }

void NodePortion::Output( gOutput& s ) const
{
  Portion::Output( s );
  s << "(Node) " << *_Value;
  if(*_Value)
    s << " \"" << (*_Value)->GetName() << "\""; 
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

PortionSpec ActionPortion::Spec( void ) const
{ return PortionSpec(porACTION); }

void ActionPortion::Output( gOutput& s ) const
{
  Portion::Output( s );
  s << "(Action) " << *_Value;
  if(*_Value)
    s << " \"" << (*_Value)->GetName() << "\""; 
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
{}

MixedPortion::~MixedPortion()
{ }

BaseMixedProfile*& MixedPortion::Value( void ) const
{ return *_Value; }

PortionSpec MixedPortion::Spec( void ) const
{ 
  static unsigned long _DataType = porMIXED;
  if(_DataType == porMIXED)
    if( !*_Value )
    {
      _DataType = porMIXED;
    }
    else
    {
      switch( (*_Value)->Type() )
      {
      case DOUBLE:
	_DataType = porMIXED_FLOAT;
	break;
      case RATIONAL:
	_DataType = porMIXED_RATIONAL;
	break;
      default:
	assert( 0 );
      }
    }
  return PortionSpec(_DataType);
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
    s << "(Mixed) ";
    switch( (*_Value)->Type() )
    {
    case DOUBLE: 
      if(_WriteSolutionInfo>1)
	s << (MixedSolution<double>) *(MixedSolution<double>*)(*_Value); 
      else
	s << (MixedProfile<double>) *(MixedSolution<double>*)(*_Value); 
      break;
    case RATIONAL:
      if(_WriteSolutionInfo>1)
	s << (MixedSolution<gRational>) *(MixedSolution<gRational>*)(*_Value); 
      else
	s << (MixedProfile<gRational>) *(MixedSolution<gRational>*)(*_Value); 
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

PortionSpec BehavPortion::Spec( void ) const
{ 
  static unsigned long _DataType = porBEHAV;
  if(_DataType == porBEHAV)
    if( !*_Value )
    {
      _DataType = porBEHAV;
    }
    else
    {
      switch( (*_Value)->Type() )
      {
      case DOUBLE:
	_DataType = porBEHAV_FLOAT;
	break;
      case RATIONAL:
	_DataType = porBEHAV_RATIONAL;
	break;
      default:
	assert( 0 );
      }
    }
  return PortionSpec(_DataType);
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
    s << "(Behav) ";
    switch( (*_Value)->Type() )
    {
    case DOUBLE:
      if(_WriteSolutionInfo>1)
	s << (BehavSolution<double>) *(BehavSolution<double>*)(*_Value); 
      else
	s << (BehavProfile<double>) *(BehavSolution<double>*)(*_Value); 
      break;
    case RATIONAL:
      if(_WriteSolutionInfo>1)
	s << (BehavSolution<gRational>) *(BehavSolution<gRational>*)(*_Value); 
      else
	s << (BehavProfile<gRational>) *(BehavSolution<gRational>*)(*_Value); 
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

PortionSpec NfgPortion::Spec( void ) const
{ 
  assert( (*_Value) != 0 );
  switch( (*_Value)->Type() )
  {
  case DOUBLE:
    return PortionSpec(porNFG_FLOAT);
  case RATIONAL:
    return PortionSpec(porNFG_RATIONAL);
  default:
    assert( 0 );
  }
  return PortionSpec(porUNDEFINED);
}

void NfgPortion::Output( gOutput& s ) const
{
  Portion::Output( s );
  assert(*_Value);
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

void NfgPortion::RemoveAllDependents(void)
{
  while( ( (NfgPortion*) Original() )->_Dependent->Length() > 0 )
    ( (NfgPortion*) Original() )->_Dependent->Remove( 1 )->SetIsValid( false );
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

PortionSpec EfgPortion::Spec( void ) const
{ 
  assert( (*_Value) != 0 );
  switch( (*_Value)->Type() )
  {
  case DOUBLE:
    return PortionSpec(porEFG_FLOAT);
  case RATIONAL:
    return PortionSpec(porEFG_RATIONAL);
  default:
    assert( 0 );
  }
  return PortionSpec(porUNDEFINED);
}

void EfgPortion::Output( gOutput& s ) const
{
  Portion::Output( s );
  assert(*_Value);
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

void EfgPortion::RemoveAllDependents(void)
{
  while( ( (EfgPortion*) Original() )->_Dependent->Length() > 0 )
    ( (EfgPortion*) Original() )->_Dependent->Remove( 1 )->SetIsValid( false );
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

PortionSpec OutputPortion::Spec( void ) const
{ return PortionSpec(porOUTPUT); }

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

PortionSpec InputPortion::Spec( void ) const
{ return PortionSpec(porINPUT); }

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



PortionSpec ListPortion::Spec( void ) const
{ return PortionSpec(_DataType, _ListDepth()); }

Portion* ListPortion::ValCopy( void ) const
{ 
  ListPortion* p =new ListValPortion( *_Value ); 
  // p->SetOwner( Owner() );
  if( p->_DataType == porUNDEFINED )
    p->_DataType = _DataType;
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

  assert( p->Spec() == Spec() );
  assert( PortionSpecMatch( ( (ListPortion*) p )->_DataType, _DataType ) || 
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

bool ListPortion::operator == ( Portion* p ) const
{
  bool result = true;
  int i;
  int length = _Value->Length();
  Portion* p1;
  Portion* p2;
  bool type_found;

  if( p->Spec() == Spec() )
  {
    if( _Value->Length() == ( (ListPortion*) p )->_Value->Length() )
    {
      for( i = 1; i <= length; i++ )
      {
	p1 = (*_Value)[i];
	p2 = (*(((ListPortion*) p)->_Value))[i];
	if(p1->Spec() == p2->Spec())
	{
	  if(p1->Spec().ListDepth > 0)
	    result = result &
	      ( ((ListPortion*) p1)->operator==(p2));
	  else
	    result = result &
	      PortionEqual(p1, p2, type_found);
	}
	else
	  result = false;
      }
    }
    else
      result = false;
  }
  else
    result = false;
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








int ListPortion::_ListDepth(void) const
{
  int i;
  int Depth = 1;

  if(Length() > 0)
  {
    Depth = operator[](1)->Spec().ListDepth + 1;
    for(i=2; i<=Length(); i++)
    {
      assert(i>=0 && i<=Length());
      if(operator[](i)->Spec().ListDepth + 1 < Depth)
	Depth = operator[](i)->Spec().ListDepth + 1;
    }
  }

  return Depth;
}

bool ListPortion::ContainsListsOnly( void ) const
{
  if( _Value->Length() == 0 )
    return false;
  else
    return _ContainsListsOnly;
}

void ListPortion::SetDataType( unsigned long type )
{ _DataType = type; }

void ListPortion::Output( gOutput& s ) const
{ Output(s, 0); }

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
	if( (*_Value)[ i ]->Spec().ListDepth == 0)
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
  PortionSpec item_type;

#ifndef NDEBUG
  if( item->Spec().Type == porREFERENCE )
  {
    gerr << "Portion Error:\n";
    gerr << "  Attempted to insert a ReferencePortion into\n";
    gerr << "  a ListPortion\n";
  }
  assert( item->Spec().Type != porREFERENCE );
#endif
  
  if( item->Spec().ListDepth > 0 )
    item_type.Type = ( (ListPortion*) item )->_DataType;
  else
  {
    item_type = item->Spec();
    _ContainsListsOnly = false;
  }


  if( _DataType == porUNDEFINED )
  {
    if( _Value->Length() == 0 )
      _Owner = item->Original()->Owner();
    _DataType = item_type.Type;
    ((ListPortion*) Original())->_DataType = item_type.Type;
    result = _Value->Insert( item, index );
  }
  else  // inserting into an existing list
  {
    if( PortionSpecMatch( item_type, _DataType ) )
    {
      if( _Value->Length() == 0 )
	_Owner = item->Original()->Owner();
      result = _Value->Insert( item, index );
    }
    else if( item_type.Type == porUNDEFINED )
    {
      if( _Value->Length() == 0 )
	_Owner = item->Original()->Owner();
      result = _Value->Insert( item, index );
      assert( item->Spec().ListDepth > 0 );
      ((ListPortion*) item)->_DataType = _DataType;
    }
    else if(item_type.Type == porERROR)
      result = _Value->Insert( item, index );
    else
      delete item;
  }
  return result;
}


bool ListPortion::Contains( Portion* p2 ) const
{
  int i;
  int length = _Value->Length();
  bool type_found;
  Portion* p1;

  for( i = 1; i <= length; i++ )
  {
    p1 = (*_Value)[i];
    if(p1->Spec().ListDepth == 0)
    {
      if(PortionEqual(p1, p2, type_found))
	return true;
    }      
    else 
    {
      if(p2->Spec().ListDepth > 0 && ((ListPortion*) p1)->operator==(p2))
	return true;
      if(((ListPortion*) p1)->Contains(p2))
	return true;
    }
  }
  return false;
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


Portion* ListPortion::operator[]( int index ) const
{
  if( index >= 1 && index <= _Value->Length() )
  {
    assert( (*_Value)[ index ] != 0 );
    return (*_Value)[ index ];
  }
  else
    return 0;
}



Portion* ListPortion::Subscript( int index ) const
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





gOutput& operator << ( gOutput& s, Portion* p )
{
  p->Output( s );
  return s;
}


bool PortionEqual(Portion* p1, Portion* p2, bool& type_found)
{
  bool b = false;

  if(!(p1->Spec() == p2->Spec())) 
    return false;

  type_found = true;

  if(p1->Spec().Type==porBOOL)   
    b = (((BoolPortion*) p1)->Value() == ((BoolPortion*) p2)->Value());
  else if(p1->Spec().Type==porINTEGER)   
    b = (((IntPortion*) p1)->Value() == ((IntPortion*) p2)->Value());
  else if(p1->Spec().Type==porFLOAT)
    b = (((FloatPortion*) p1)->Value() == ((FloatPortion*) p2)->Value());
  else if(p1->Spec().Type==porRATIONAL)
    b = (((RationalPortion*) p1)->Value()==((RationalPortion*) p2)->Value());
  else if(p1->Spec().Type==porTEXT)
      b = (((TextPortion*) p1)->Value() == ((TextPortion*) p2)->Value());
  
  else if(p1->Spec().Type==porNODE)
    b = (((NodePortion*) p1)->Value() == ((NodePortion*) p2)->Value());
  else if(p1->Spec().Type==porACTION)
      b = (((ActionPortion*) p1)->Value() == ((ActionPortion*) p2)->Value());
  else if(p1->Spec().Type==porINFOSET)
    b = (((InfosetPortion*) p1)->Value() == ((InfosetPortion*) p2)->Value());
  else if(p1->Spec().Type==porOUTCOME)
    b = (((OutcomePortion*) p1)->Value() == ((OutcomePortion*) p2)->Value());
  else if(p1->Spec().Type==porPLAYER_NFG)
    b = (((NfPlayerPortion*) p1)->Value() == ((NfPlayerPortion*) p2)->Value());
  else if(p1->Spec().Type==porSTRATEGY)
    b = (((StrategyPortion*) p1)->Value() == ((StrategyPortion*) p2)->Value());
  else if(p1->Spec().Type==porNF_SUPPORT)
    b = (((NfSupportPortion*) p1)->Value()==((NfSupportPortion*) p2)->Value());
  else if(p1->Spec().Type==porEF_SUPPORT)
    b = (((EfSupportPortion*) p1)->Value()==((EfSupportPortion*) p2)->Value());
  
  else if(p1->Spec().Type==porMIXED_FLOAT)
    b = ((*((MixedSolution<double>*) ((MixedPortion*) p1)->Value())) == 
	 (*((MixedSolution<double>*) ((MixedPortion*) p2)->Value())));
  else if(p1->Spec().Type==porMIXED_RATIONAL)
    b = ((*((MixedSolution<gRational>*) ((MixedPortion*) p1)->Value())) == 
	 (*((MixedSolution<gRational>*) ((MixedPortion*) p2)->Value())));
  else if(p1->Spec().Type==porBEHAV_FLOAT)
    b = ((*((BehavSolution<double>*) ((BehavPortion*) p1)->Value())) == 
	 (*((BehavSolution<double>*) ((BehavPortion*) p2)->Value())));
  else if(p1->Spec().Type==porBEHAV_RATIONAL)
    b = ((*((BehavSolution<gRational>*) ((BehavPortion*) p1)->Value())) == 
	 (*((BehavSolution<gRational>*) ((BehavPortion*) p2)->Value())));
  else
    type_found = false;
  return b;
}
