//#
//# FILE: portion.cc -- implementation of Portion base and descendent classes
//#                     companion to GSM
//#
//# @(#)portion.cc	1.154 12/17/96
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


#include "gsm.h"

extern GSM* _gsm;  // defined at the end of gsm.cc



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
long Portion::_WriteSolutionInfo = 1;

void Portion::_SetWriteWidth(long x)
{ _WriteWidth = x; }
void Portion::_SetWritePrecis(long x)
{ _WritePrecis = x; }
void Portion::_SetWriteExpmode(bool x)
{ _WriteExpmode = x; }
void Portion::_SetWriteQuoted(bool x)
{ _WriteQuoted = x; }
void Portion::_SetWriteListBraces(bool x)
{ _WriteListBraces = x; }
void Portion::_SetWriteListCommas(bool x)
{ _WriteListCommas = x; }
void Portion::_SetWriteListLF(long x)
{ _WriteListLF = x; }
void Portion::_SetWriteListIndent(long x)
{ _WriteListIndent = x; }
void Portion::_SetWriteSolutionInfo(long x)
{ _WriteSolutionInfo = x; }



void Portion::Output(gOutput& s) const
{
  s.SetWidth((int) _WriteWidth);
  s.SetPrec((int) _WritePrecis);
  if(_WriteExpmode)
    s.SetExpMode();
  else
    s.SetFloatMode();
}



#ifdef MEMCHECK
int Portion::_NumObj = 0;
#endif

Portion::Portion(void)
{
  _Original = 0;
  _Game = 0;
  _GameIsEfg = false;

#ifdef MEMCHECK
  _NumObj++;
  printf("--- Portion Ctor, count: %d\n", _NumObj);
#endif
}

Portion::~Portion()
{ 
  SetGame(0, false);

#ifdef MEMCHECK
  _NumObj--;
  printf("--- Portion Dtor, count: %d\n", _NumObj);
#endif
}


void Portion::SetOriginal(const Portion* p)
{ 
  _Original = (Portion*) p;
}

Portion* Portion::Original(void) const
{ 
  if(!IsReference())
  {
    assert(!_Original);
    return (Portion*) this;
  }
  else
  {
    return _Original; 
  }
}



void* Portion::Game(void) const
{
  if(Spec().ListDepth == 0)
    switch(Spec().Type)
    {
    case porNFG_FLOAT:
    case porNFG_RATIONAL:
      return ((NfgPortion*) this)->Value();
    case porEFG_FLOAT:
    case porEFG_RATIONAL:
      return ((EfgPortion*) this)->Value();
    default:
      return _Game;
    }
  else
    // lists aren't owned by games; only the elements are.
    return 0;
}

bool Portion::GameIsEfg(void) const
{
  switch(Spec().Type)
  {
  case porNFG_FLOAT:
  case porNFG_RATIONAL:
    return false;
  case porEFG_FLOAT:
  case porEFG_RATIONAL:
    return true;
  default:
    return _GameIsEfg;
  }
}

void Portion::SetGame(void* game, bool efg)
{
  if(game != _Game)
  {
    if(_Game)
    {
      _gsm->GameRefCount(_Game)--;
#ifdef MEMCHECK
      gout<<"Game "<<_Game<<" ref count-: "<<_gsm->GameRefCount(_Game)<<'\n';
#endif
      if(_gsm->GameRefCount(_Game) == 0)
      {
	if(!_GameIsEfg)
	  delete (BaseNfg*) _Game;
	else
	  delete (BaseEfg*) _Game;
	_Game = 0;
      }
    }
    
    _Game = game;
    _GameIsEfg = efg;
    
    if(_Game)
    {
      _gsm->GameRefCount(_Game)++;
#ifdef MEMCHECK
      gout<<"Game "<<_Game<<" ref count+: "<<_gsm->GameRefCount(_Game)<<'\n';
#endif
    }
  }
}


//---------------------------------------------------------------------
//                          Error class
//---------------------------------------------------------------------

ErrorPortion::ErrorPortion(const gString& value)
: _Value(value)
{ }

ErrorPortion::~ErrorPortion()
{ }

gString ErrorPortion::Value(void) const
{ return _Value; }

PortionSpec ErrorPortion::Spec(void) const
{ return PortionSpec(porERROR); }

void ErrorPortion::Output(gOutput& s) const
{
  Portion::Output(s);
  s << OutputString();
}

gString ErrorPortion::OutputString( void ) const
{
  if(_Value == "")
    return "(Error)";
  else
    return _Value;  
}


Portion* ErrorPortion::ValCopy(void) const
{ return new ErrorPortion(_Value); }

Portion* ErrorPortion::RefCopy(void) const
{ return new ErrorPortion(_Value); }

bool ErrorPortion::IsReference(void) const
{ return false; }


//---------------------------------------------------------------------
//                          Null class
//---------------------------------------------------------------------

NullPortion::NullPortion(const unsigned long datatype)
: _DataType(datatype)
{ }

NullPortion::~NullPortion()
{ }

unsigned long NullPortion::DataType(void) const
{ return _DataType; }

PortionSpec NullPortion::Spec(void) const
{ return PortionSpec(porNULL, 0, true); }

void NullPortion::Output(gOutput& s) const
{
  Portion::Output(s);
  s << OutputString();
}

gString NullPortion::OutputString( void ) const
{
  return (gString) "Null(" + PortionSpecToText(_DataType) + ")";
}

Portion* NullPortion::ValCopy(void) const
{ return new NullPortion(_DataType); }

Portion* NullPortion::RefCopy(void) const
{ return new NullPortion(_DataType); }

bool NullPortion::IsReference(void) const
{ return false; }


//---------------------------------------------------------------------
//                          Reference class
//---------------------------------------------------------------------

ReferencePortion::ReferencePortion(const gString& value)
: _Value(value)
{ }

ReferencePortion::~ReferencePortion()
{ }

gString ReferencePortion::Value(void)
{ return _Value; }

PortionSpec ReferencePortion::Spec(void) const
{ return PortionSpec(porREFERENCE); }

void ReferencePortion::Output(gOutput& s) const
{ 
  Portion::Output(s);
  s << OutputString();
}

gString ReferencePortion::OutputString( void ) const
{
  return (gString) "(Reference) \"" + _Value + "\""; 
}

Portion* ReferencePortion::ValCopy(void) const
{ return new ReferencePortion(_Value); }

Portion* ReferencePortion::RefCopy(void) const
{ return new ReferencePortion(_Value); }

bool ReferencePortion::IsReference(void) const
{ return false; }


//---------------------------------------------------------------------
//                          int class
//---------------------------------------------------------------------

IntPortion::IntPortion(void)
{ }

IntPortion::~IntPortion()
{ }

long& IntPortion::Value(void) const
{ return *_Value; }

PortionSpec IntPortion::Spec(void) const
{ return PortionSpec(porINTEGER); }

void IntPortion::Output(gOutput& s) const
{
  Portion::Output(s);
  s << *_Value; 
}

gString IntPortion::OutputString( void ) const
{
  return ToString( *_Value );
}

Portion* IntPortion::ValCopy(void) const
{ return new IntValPortion(*_Value); }

Portion* IntPortion::RefCopy(void) const
{ 
  Portion* p = new IntRefPortion(*_Value);
  p->SetOriginal(Original());
  return p;
}


IntValPortion::IntValPortion(long value)
{ _Value = new long(value); }

IntValPortion::~IntValPortion()
{ delete _Value; }

bool IntValPortion::IsReference(void) const
{ return false; }


IntRefPortion::IntRefPortion(long& value)
{ _Value = &value; }

IntRefPortion::~IntRefPortion()
{ }

bool IntRefPortion::IsReference(void) const
{ return true; }


//---------------------------------------------------------------------
//                          Float class
//---------------------------------------------------------------------

FloatPortion::FloatPortion(void)
{ }

FloatPortion::~FloatPortion()
{ }

double& FloatPortion::Value(void) const
{ return *_Value; }

PortionSpec FloatPortion::Spec(void) const
{ return PortionSpec(porFLOAT); }

void FloatPortion::Output(gOutput& s) const
{
  Portion::Output(s);
  s << *_Value; 
}

gString FloatPortion::OutputString( void ) const
{
  return ToString( *_Value );
}

Portion* FloatPortion::ValCopy(void) const
{ return new FloatValPortion(*_Value); }

Portion* FloatPortion::RefCopy(void) const
{
  Portion* p = new FloatRefPortion(*_Value);
  p->SetOriginal(Original());
  return p;
}


FloatValPortion::FloatValPortion(double value)
{ _Value = new double(value); }

FloatValPortion::~FloatValPortion()
{ delete _Value; }

bool FloatValPortion::IsReference(void) const
{ return false; }


FloatRefPortion::FloatRefPortion(double& value)
{ _Value = &value; }

FloatRefPortion::~FloatRefPortion()
{ }

bool FloatRefPortion::IsReference(void) const
{ return true; }




//---------------------------------------------------------------------
//                          Rational class
//---------------------------------------------------------------------

RationalPortion::RationalPortion(void)
{ }

RationalPortion::~RationalPortion()
{ }

gRational& RationalPortion::Value(void) const
{ return *_Value; }

PortionSpec RationalPortion::Spec(void) const
{ return PortionSpec(porRATIONAL); }

void RationalPortion::Output(gOutput& s) const
{
  Portion::Output(s);
  s << *_Value; 
}

gString RationalPortion::OutputString( void ) const
{
  return ToString( *_Value );
}

Portion* RationalPortion::ValCopy(void) const
{ return new RationalValPortion(*_Value); }

Portion* RationalPortion::RefCopy(void) const
{ 
  Portion* p = new RationalRefPortion(*_Value); 
  p->SetOriginal(Original());
  return p;
}



RationalValPortion::RationalValPortion(gRational value)
{ _Value = new gRational(value); }

RationalValPortion::~RationalValPortion()
{ delete _Value; }

bool RationalValPortion::IsReference(void) const
{ return false; }


RationalRefPortion::RationalRefPortion(gRational& value)
{ _Value = &value; }

RationalRefPortion::~RationalRefPortion()
{ }

bool RationalRefPortion::IsReference(void) const
{ return true; }




//---------------------------------------------------------------------
//                          Text class
//---------------------------------------------------------------------

TextPortion::TextPortion(void)
{ }

TextPortion::~TextPortion()
{ }

gString& TextPortion::Value(void) const
{ return *_Value; }

PortionSpec TextPortion::Spec(void) const
{ return PortionSpec(porTEXT); }

void TextPortion::Output(gOutput& s) const
{ 
  Portion::Output(s);
  s << OutputString();
}

gString TextPortion::OutputString( void ) const
{
  gString text = *_Value;
  int i = 0;
  for(i = 0; i < text.length(); i++)
    if(text[i] == '\\' && text[i + 1] == 'n')
    {
      text.remove(i);
      text[i] = '\n';
    }

  if(_WriteQuoted) 
	 text = (gString) '\"' + text + "\"";
  return text;
}

Portion* TextPortion::ValCopy(void) const
{ return new TextValPortion(*_Value); }

Portion* TextPortion::RefCopy(void) const
{ 
  Portion* p = new TextRefPortion(*_Value); 
  p->SetOriginal(Original());
  return p;
}




TextValPortion::TextValPortion(gString value)
{ _Value = new gString(value); }

TextValPortion::~TextValPortion()
{ delete _Value; }

bool TextValPortion::IsReference(void) const
{ return false; }


TextRefPortion::TextRefPortion(gString& value)
{ _Value = &value; }

TextRefPortion::~TextRefPortion()
{ }

bool TextRefPortion::IsReference(void) const
{ return true; }







//---------------------------------------------------------------------
//                          Bool class
//---------------------------------------------------------------------

BoolPortion::BoolPortion(void)
{ }

BoolPortion::~BoolPortion()
{ }

bool& BoolPortion::Value(void) const
{ return *_Value; }

PortionSpec BoolPortion::Spec(void) const
{ return PortionSpec(porBOOL); }

void BoolPortion::Output(gOutput& s) const
{
  Portion::Output(s);
  s << OutputString();
}

gString BoolPortion::OutputString( void ) const
{
  return (*_Value ? "True" : "False");  
}

Portion* BoolPortion::ValCopy(void) const
{ return new BoolValPortion(*_Value); }

Portion* BoolPortion::RefCopy(void) const
{ 
  Portion* p = new BoolRefPortion(*_Value);
  p->SetOriginal(Original());
  return p;
}



BoolValPortion::BoolValPortion(bool value)
{ _Value = new bool(value); }

BoolValPortion::~BoolValPortion()
{ delete _Value; }

bool BoolValPortion::IsReference(void) const
{ return false; }


BoolRefPortion::BoolRefPortion(bool& value)
{ _Value = &value; }

BoolRefPortion::~BoolRefPortion()
{ }

bool BoolRefPortion::IsReference(void) const
{ return true; }





//---------------------------------------------------------------------
//                          Outcome class
//---------------------------------------------------------------------



OutcomePortion::OutcomePortion(void)
{ }

OutcomePortion::~OutcomePortion()
{ }

Outcome*& OutcomePortion::Value(void) const
{ return *_Value; }

PortionSpec OutcomePortion::Spec(void) const
{ 
  switch( SubType() )
  {
  case DOUBLE:
    return PortionSpec(porOUTCOME_FLOAT);
  case RATIONAL:
    return PortionSpec(porOUTCOME_RATIONAL);
  default:
    assert(0);
  }
  return porUNDEFINED; 
}


DataType OutcomePortion::SubType( void ) const
{
  assert( Value() );
  assert( Value()->BelongsTo() );
  return Value()->BelongsTo()->Type();
}


void OutcomePortion::Output(gOutput& s) const
{
  Portion::Output(s);
  
  s << "(Outcome) " << *_Value;
  if(*_Value)
  {
    s << " \"" << (*_Value)->GetName() << "\" ";
    (*_Value)->PrintValues(s);
  }
}

gString OutcomePortion::OutputString( void ) const
{
  return "(Outcome)";
}

Portion* OutcomePortion::ValCopy(void) const
{ 
  Portion* p = new OutcomeValPortion(*_Value);
  p->SetGame(Game(), GameIsEfg());
  return p;
}

Portion* OutcomePortion::RefCopy(void) const
{ 
  Portion* p = new OutcomeRefPortion(*_Value); 
  p->SetGame(Game(), GameIsEfg());
  p->SetOriginal(Original());
  return p;
}


OutcomeValPortion::OutcomeValPortion(Outcome* value)
{ _Value = new Outcome*(value); }

OutcomeValPortion::~OutcomeValPortion()
{
  delete _Value; 
}

bool OutcomeValPortion::IsReference(void) const
{ return false; }


OutcomeRefPortion::OutcomeRefPortion(Outcome*& value)
{ _Value = &value; }

OutcomeRefPortion::~OutcomeRefPortion()
{ }

bool OutcomeRefPortion::IsReference(void) const
{ return true; }





//---------------------------------------------------------------------
//                          NfPlayer class
//---------------------------------------------------------------------



NfPlayerPortion::NfPlayerPortion(void)
{ }

NfPlayerPortion::~NfPlayerPortion()
{ }

NFPlayer*& NfPlayerPortion::Value(void) const
{ return *_Value; }

PortionSpec NfPlayerPortion::Spec(void) const
{ return PortionSpec(porNFPLAYER); }

void NfPlayerPortion::Output(gOutput& s) const
{
  Portion::Output(s);
  s << "(NfPlayer) " << *_Value;
  if(*_Value)
    s << " \"" << (*_Value)->GetName() << "\""; 
}

gString NfPlayerPortion::OutputString( void ) const
{
  return "(NfPlayer)";
}

Portion* NfPlayerPortion::ValCopy(void) const
{
  Portion* p = new NfPlayerValPortion(*_Value); 
  p->SetGame(Game(), GameIsEfg());
  return p;
}

Portion* NfPlayerPortion::RefCopy(void) const
{
  Portion* p = new NfPlayerRefPortion(*_Value); 
  p->SetGame(Game(), GameIsEfg());
  p->SetOriginal(Original());
  return p;
}


NfPlayerValPortion::NfPlayerValPortion(NFPlayer* value)
{ _Value = new NFPlayer*(value); }

NfPlayerValPortion::~NfPlayerValPortion()
{ 
  delete _Value; 
}

bool NfPlayerValPortion::IsReference(void) const
{ return false; }


NfPlayerRefPortion::NfPlayerRefPortion(NFPlayer*& value)
{ _Value = &value; }

NfPlayerRefPortion::~NfPlayerRefPortion()
{ }

bool NfPlayerRefPortion::IsReference(void) const
{ return true; }














//---------------------------------------------------------------------
//                          Strategy class
//---------------------------------------------------------------------



StrategyPortion::StrategyPortion(void)
{ }

StrategyPortion::~StrategyPortion()
{ }

Strategy*& StrategyPortion::Value(void) const
{ return *_Value; }

PortionSpec StrategyPortion::Spec(void) const
{ return PortionSpec(porSTRATEGY); }

void StrategyPortion::Output(gOutput& s) const
{ 
  Portion::Output(s);
  s << "(Strategy) " << *_Value;
  if(*_Value)
    s << " \"" << (*_Value)->name << "\""; 
}

gString StrategyPortion::OutputString( void ) const
{
  return "(Strategy)";
}

Portion* StrategyPortion::ValCopy(void) const
{
  Portion* p = new StrategyValPortion(*_Value); 
  p->SetGame(Game(), GameIsEfg());
  return p;
}

Portion* StrategyPortion::RefCopy(void) const
{
  Portion* p = new StrategyRefPortion(*_Value); 
  p->SetGame(Game(), GameIsEfg());
  p->SetOriginal(Original());
  return p;
}


StrategyValPortion::StrategyValPortion(Strategy* value)
{ _Value = new Strategy*(value); }

StrategyValPortion::~StrategyValPortion()
{ 
  delete _Value; 
}

bool StrategyValPortion::IsReference(void) const
{ return false; }


StrategyRefPortion::StrategyRefPortion(Strategy*& value)
{ _Value = &value; }

StrategyRefPortion::~StrategyRefPortion()
{ }

bool StrategyRefPortion::IsReference(void) const
{ return true; }






//---------------------------------------------------------------------
//                          NfSupport class
//---------------------------------------------------------------------



NfSupportPortion::NfSupportPortion(void)
{ }

NfSupportPortion::~NfSupportPortion()
{ }

NFSupport*& NfSupportPortion::Value(void) const
{ return *_Value; }

PortionSpec NfSupportPortion::Spec(void) const
{
  switch( SubType() )
  {
  case DOUBLE:
    return PortionSpec(porNFSUPPORT_FLOAT);
  case RATIONAL:
    return PortionSpec(porNFSUPPORT_RATIONAL);
  default:
    assert(0);
  }
  return porUNDEFINED; 
}

DataType NfSupportPortion::SubType( void ) const
{
  assert( Value() );
  return Value()->BelongsTo().Type();
}

void NfSupportPortion::Output(gOutput& s) const
{ 
  Portion::Output(s);
  s << "(NfSupport) " << *_Value;
  if(*_Value) 
    s << ' ' << **_Value;  
}

gString NfSupportPortion::OutputString( void ) const
{
  return "(NfSupport)";
}

Portion* NfSupportPortion::ValCopy(void) const
{
  Portion* p = new NfSupportValPortion(*_Value); 
  p->SetGame(Game(), GameIsEfg());
  return p;
}

Portion* NfSupportPortion::RefCopy(void) const
{
  Portion* p = new NfSupportRefPortion(*_Value); 
  p->SetGame(Game(), GameIsEfg());
  p->SetOriginal(Original());
  return p;
}


NfSupportValPortion::NfSupportValPortion(NFSupport* value)
{ _Value = new NFSupport*(value); }

NfSupportValPortion::~NfSupportValPortion()
{ 
  delete _Value; 
}

bool NfSupportValPortion::IsReference(void) const
{ return false; }


NfSupportRefPortion::NfSupportRefPortion(NFSupport*& value)
{ _Value = &value; }

NfSupportRefPortion::~NfSupportRefPortion()
{ }

bool NfSupportRefPortion::IsReference(void) const
{ return true; }





//---------------------------------------------------------------------
//                          EfSupport class
//---------------------------------------------------------------------


EfSupportPortion::EfSupportPortion(void)
{ }

EfSupportPortion::~EfSupportPortion()
{ }

EFSupport*& EfSupportPortion::Value(void) const
{ return *_Value; }

PortionSpec EfSupportPortion::Spec(void) const
{
  switch( SubType() )
  {
  case DOUBLE:
    return PortionSpec(porEFSUPPORT_FLOAT);
  case RATIONAL:
    return PortionSpec(porEFSUPPORT_RATIONAL);
  default:
    assert(0);
  }
  return porUNDEFINED; 
}

DataType EfSupportPortion::SubType( void ) const
{
  assert( Value() );
  return Value()->BelongsTo().Type();
}


void EfSupportPortion::Output(gOutput& s) const
{ 
  Portion::Output(s);
  s << "(EfSupport) " << *_Value;
  if(*_Value) 
    s << ' ' << **_Value;
}

gString EfSupportPortion::OutputString( void ) const
{
  return "(EfSupport)";
}

Portion* EfSupportPortion::ValCopy(void) const
{
  Portion* p = new EfSupportValPortion(*_Value); 
  p->SetGame(Game(), GameIsEfg());
  return p;
}

Portion* EfSupportPortion::RefCopy(void) const
{
  Portion* p = new EfSupportRefPortion(*_Value); 
  p->SetGame(Game(), GameIsEfg());
  p->SetOriginal(Original());
  return p;
}


EfSupportValPortion::EfSupportValPortion(EFSupport* value)
{ _Value = new EFSupport*(value); }

EfSupportValPortion::~EfSupportValPortion()
{ 
  delete _Value; 
}

bool EfSupportValPortion::IsReference(void) const
{ return false; }


EfSupportRefPortion::EfSupportRefPortion(EFSupport*& value)
{ _Value = &value; }

EfSupportRefPortion::~EfSupportRefPortion()
{ }

bool EfSupportRefPortion::IsReference(void) const
{ return true; }





//---------------------------------------------------------------------
//                          EfPlayer class
//---------------------------------------------------------------------



EfPlayerPortion::EfPlayerPortion(void)
{ }

EfPlayerPortion::~EfPlayerPortion()
{ }

EFPlayer*& EfPlayerPortion::Value(void) const
{ return *_Value; }

PortionSpec EfPlayerPortion::Spec(void) const
{ return PortionSpec(porEFPLAYER); }

void EfPlayerPortion::Output(gOutput& s) const
{
  Portion::Output(s);
  s << "(EfPlayer) " << *_Value;
  if(*_Value)
    s << " \"" << (*_Value)->GetName() << "\""; 
}

gString EfPlayerPortion::OutputString( void ) const
{
  return "(EfPlayer)";
}

Portion* EfPlayerPortion::ValCopy(void) const
{
  Portion* p = new EfPlayerValPortion(*_Value); 
  p->SetGame(Game(), GameIsEfg());
  return p;
}

Portion* EfPlayerPortion::RefCopy(void) const
{
  Portion* p = new EfPlayerRefPortion(*_Value); 
  p->SetGame(Game(), GameIsEfg());
  p->SetOriginal(Original());
  return p;
}


EfPlayerValPortion::EfPlayerValPortion(EFPlayer* value)
{ _Value = new EFPlayer*(value); }

EfPlayerValPortion::~EfPlayerValPortion()
{ 
  delete _Value; 
}

bool EfPlayerValPortion::IsReference(void) const
{ return false; }


EfPlayerRefPortion::EfPlayerRefPortion(EFPlayer*& value)
{ _Value = &value; }

EfPlayerRefPortion::~EfPlayerRefPortion()
{ }

bool EfPlayerRefPortion::IsReference(void) const
{ return true; }




//---------------------------------------------------------------------
//                          Infoset class
//---------------------------------------------------------------------



InfosetPortion::InfosetPortion(void)
{ }

InfosetPortion::~InfosetPortion()
{ }


Infoset*& InfosetPortion::Value(void) const
{ return *_Value; }

PortionSpec InfosetPortion::Spec(void) const
{ return PortionSpec(porINFOSET); }

DataType InfosetPortion::SubType( void ) const
{
  assert( Value() );
  assert( Value()->BelongsTo() );
  return Value()->BelongsTo()->Type();
}


void InfosetPortion::Output(gOutput& s) const
{
  Portion::Output(s);
  s << "(Infoset) " << *_Value;
  if(*_Value)
    s << " \"" << (*_Value)->GetName() << "\""; 
}

gString InfosetPortion::OutputString( void ) const
{
  return "(Infoset)";
}

Portion* InfosetPortion::ValCopy(void) const
{ 
  Portion* p = new InfosetValPortion(*_Value);
  p->SetGame(Game(), GameIsEfg());
  return p;
}

Portion* InfosetPortion::RefCopy(void) const
{
  Portion* p = new InfosetRefPortion(*_Value); 
  p->SetGame(Game(), GameIsEfg());
  p->SetOriginal(Original());
  return p;
}


InfosetValPortion::InfosetValPortion(Infoset* value)
{ _Value = new Infoset*(value); }

InfosetValPortion::~InfosetValPortion()
{
  delete _Value; 
}

bool InfosetValPortion::IsReference(void) const
{ return false; }


InfosetRefPortion::InfosetRefPortion(Infoset*& value)
{ _Value = &value; }

InfosetRefPortion::~InfosetRefPortion()
{ }

bool InfosetRefPortion::IsReference(void) const
{ return true; }




//---------------------------------------------------------------------
//                          Node class
//---------------------------------------------------------------------



NodePortion::NodePortion(void)
{ }

NodePortion::~NodePortion()
{ }

Node*& NodePortion::Value(void) const
{ return *_Value; }

PortionSpec NodePortion::Spec(void) const
{ return PortionSpec(porNODE); }

void NodePortion::Output(gOutput& s) const
{
  Portion::Output(s);
  s << "(Node) " << *_Value;
  if(*_Value)
    s << " \"" << (*_Value)->GetName() << "\""; 
}

gString NodePortion::OutputString( void ) const
{
  return "(Node)";
}

Portion* NodePortion::ValCopy(void) const
{
  Portion* p = new NodeValPortion(*_Value); 
  p->SetGame(Game(), GameIsEfg());
  return p;
}

Portion* NodePortion::RefCopy(void) const
{
  Portion* p = new NodeRefPortion(*_Value); 
  p->SetGame(Game(), GameIsEfg());
  p->SetOriginal(Original());
  return p;
}


NodeValPortion::NodeValPortion(Node* value)
{ _Value = new Node*(value); }

NodeValPortion::~NodeValPortion()
{ 
  delete _Value; 
}

bool NodeValPortion::IsReference(void) const
{ return false; }


NodeRefPortion::NodeRefPortion(Node*& value)
{ _Value = &value; }

NodeRefPortion::~NodeRefPortion()
{ }

bool NodeRefPortion::IsReference(void) const
{ return true; }






//---------------------------------------------------------------------
//                          Action class
//---------------------------------------------------------------------



ActionPortion::ActionPortion(void)
{ }

ActionPortion::~ActionPortion()
{ }

Action*& ActionPortion::Value(void) const
{ return *_Value; }

PortionSpec ActionPortion::Spec(void) const
{ return PortionSpec(porACTION); }

void ActionPortion::Output(gOutput& s) const
{
  Portion::Output(s);
  s << "(Action) " << *_Value;
  if(*_Value)
    s << " \"" << (*_Value)->GetName() << "\""; 
}

gString ActionPortion::OutputString( void ) const
{
  return "(Action)";
}

Portion* ActionPortion::ValCopy(void) const
{
  Portion* p = new ActionValPortion(*_Value); 
  p->SetGame(Game(), GameIsEfg());
  return p;
}

Portion* ActionPortion::RefCopy(void) const
{
  Portion* p = new ActionRefPortion(*_Value); 
  p->SetGame(Game(), GameIsEfg());
  p->SetOriginal(Original());
  return p;
}


ActionValPortion::ActionValPortion(Action* value)
{ _Value = new Action*(value); }

ActionValPortion::~ActionValPortion()
{ 
  delete _Value; 
}

bool ActionValPortion::IsReference(void) const
{ return false; }


ActionRefPortion::ActionRefPortion(Action*& value)
{ _Value = &value; }

ActionRefPortion::~ActionRefPortion()
{ }

bool ActionRefPortion::IsReference(void) const
{ return true; }







//---------------------------------------------------------------------
//                            new Mixed class
//---------------------------------------------------------------------



MixedPortion::MixedPortion(void)
{}

MixedPortion::~MixedPortion()
{ }

BaseMixedProfile*& MixedPortion::Value(void) const
{ return *_Value; }

PortionSpec MixedPortion::Spec(void) const
{ 
  unsigned long _DataType = porMIXED;
  if(*_Value)
    switch((*_Value)->Type())
    {
    case DOUBLE:
      _DataType = porMIXED_FLOAT;
      break;
    case RATIONAL:
      _DataType = porMIXED_RATIONAL;
      break;
    default:
      assert(0);
    }
  return PortionSpec(_DataType);
}

DataType MixedPortion::SubType( void ) const
{
  assert( Value() );
  return Value()->Type();
}

void MixedPortion::Output(gOutput& s) const
{
  Portion::Output(s);
  if(!*_Value)
  {
    s << "(Mixed) NULL"; 
  }
  else
  {
    s << "(Mixed) ";
    switch((*_Value)->Type())
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
      assert(0);
    }
  }
}

gString MixedPortion::OutputString( void ) const
{
  return "(Mixed)";
}

Portion* MixedPortion::ValCopy(void) const
{ 
  Portion* p = 0;
  if(!*_Value)
  {
    p = new MixedValPortion(0);
  }
  else
  {
    switch((*_Value)->Type())
    {
    case DOUBLE:
      p = new MixedValPortion
	(new MixedSolution<double>
	 (* (MixedSolution<double>*) (*_Value))); 
      break;
    case RATIONAL:
      p = new MixedValPortion
	(new MixedSolution<gRational>
	 (* (MixedSolution<gRational>*) (*_Value)));       
      break;
    default:
      assert(0);
    }
  }
  p->SetGame(Game(), GameIsEfg());
  return p;
}

Portion* MixedPortion::RefCopy(void) const
{ 
  Portion* p = new MixedRefPortion(*_Value); 
  p->SetGame(Game(), GameIsEfg());
  p->SetOriginal(Original());
  return p;
}


MixedValPortion::MixedValPortion(BaseMixedProfile* value)
{ _Value = new BaseMixedProfile*(value); }

MixedValPortion::~MixedValPortion()
{ 
  delete *_Value;
  delete _Value; 
}

bool MixedValPortion::IsReference(void) const
{ return false; }


MixedRefPortion::MixedRefPortion(BaseMixedProfile*& value)
{ _Value = &value; }

MixedRefPortion::~MixedRefPortion()
{ }

bool MixedRefPortion::IsReference(void) const
{ return true; }







//---------------------------------------------------------------------
//                            new Behav class
//---------------------------------------------------------------------



BehavPortion::BehavPortion(void)
{ }

BehavPortion::~BehavPortion()
{ }

BaseBehavProfile*& BehavPortion::Value(void) const
{ return *_Value; }

PortionSpec BehavPortion::Spec(void) const
{ 
  unsigned int _DataType = porBEHAV;
  if(*_Value)
    switch((*_Value)->Type())
    {
    case DOUBLE:
      _DataType = porBEHAV_FLOAT;
      break;
    case RATIONAL:
      _DataType = porBEHAV_RATIONAL;
      break;
    default:
      assert(0);
    }
  return PortionSpec(_DataType);
}

DataType BehavPortion::SubType( void ) const
{
  assert( Value() );
  return Value()->Type();
}


void BehavPortion::Output(gOutput& s) const
{
  Portion::Output(s);
  if(!*_Value)
  {
    s << "(Behav) NULL"; 
  }
  else
  {
    s << "(Behav) ";
    switch((*_Value)->Type())
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
      assert(0);
    }
  }
}

gString BehavPortion::OutputString( void ) const
{
  return "(Behav)";
}

Portion* BehavPortion::ValCopy(void) const
{ 
  Portion* p;

  if(!*_Value)
  {
    p = new BehavValPortion(0);
  }
  else
  {
    switch((*_Value)->Type())
    {
    case DOUBLE:
      p = new BehavValPortion
	(new BehavSolution<double>
	 (* (BehavSolution<double>*) (*_Value))); 
      break;
    case RATIONAL:
      p = new BehavValPortion
	(new BehavSolution<gRational>
	 (* (BehavSolution<gRational>*) (*_Value))); 
      break;
    default:
      assert(0);
    }
  }
  p->SetGame(Game(), GameIsEfg());
  return p;
}

Portion* BehavPortion::RefCopy(void) const
{ 
  Portion* p = new BehavRefPortion(*_Value); 
  p->SetGame(Game(), GameIsEfg());
  p->SetOriginal(Original());
  return p;
}


BehavValPortion::BehavValPortion(BaseBehavProfile* value)
{ _Value = new BaseBehavProfile*(value); }

BehavValPortion::~BehavValPortion()
{ 
  delete *_Value;
  delete _Value; 
}

bool BehavValPortion::IsReference(void) const
{ return false; }


BehavRefPortion::BehavRefPortion(BaseBehavProfile*& value)
{ _Value = &value; }

BehavRefPortion::~BehavRefPortion()
{ }

bool BehavRefPortion::IsReference(void) const
{ return true; }








//---------------------------------------------------------------------
//                            new Nfg class
//---------------------------------------------------------------------



NfgPortion::NfgPortion(void)
{ 
}

NfgPortion::~NfgPortion()
{ }

BaseNfg*& NfgPortion::Value(void) const
{ return *_Value; }

PortionSpec NfgPortion::Spec(void) const
{ 
  assert((*_Value) != 0);
  switch((*_Value)->Type())
  {
  case DOUBLE:
    return PortionSpec(porNFG_FLOAT);
  case RATIONAL:
    return PortionSpec(porNFG_RATIONAL);
  default:
    assert(0);
  }
  return PortionSpec(porUNDEFINED);
}

DataType NfgPortion::SubType( void ) const
{
  assert( Value() );
  return Value()->Type();
}


void NfgPortion::Output(gOutput& s) const
{
  Portion::Output(s);
  assert(*_Value);
  s << "(Nfg) \"" << (*_Value)->GetTitle() << "\""; 
}

gString NfgPortion::OutputString( void ) const
{
  return "(Nfg)";
}

Portion* NfgPortion::ValCopy(void) const
{ 
  Portion* p = new NfgValPortion(*_Value); 
  // don't call SetGame() here because they are called in constructor
  // p->SetGame(Game(), GameIsEfg());
  return p;

  /*
  switch((*_Value)->Type())
  {
  case DOUBLE:
    return new NfgValPortion
      (new Nfg<double>(* (Nfg<double>*) (*_Value))); 
    break;
  case RATIONAL:
    return new NfgValPortion
      (new Nfg<gRational>(* (Nfg<gRational>*) (*_Value))); 
    break;
  default:
    assert(0);
  }
  return 0;
  */
}

Portion* NfgPortion::RefCopy(void) const
{ 
  Portion* p = new NfgRefPortion(*_Value); 
  // don't call SetGame() here because they are called in constructor
  // p->SetGame(Game(), GameIsEfg());
  p->SetOriginal(Original());
  return p;
}



NfgValPortion::NfgValPortion(BaseNfg* value)
{
  _Value = new BaseNfg*(value);
  SetGame(*_Value, false);
}

NfgValPortion::~NfgValPortion()
{ 
  //delete *_Value;
  delete _Value; 
}

bool NfgValPortion::IsReference(void) const
{ return false; }


NfgRefPortion::NfgRefPortion(BaseNfg*& value)
{
  _Value = &value; 
  SetGame(*_Value, false);
}

NfgRefPortion::~NfgRefPortion()
{ }

bool NfgRefPortion::IsReference(void) const
{ return true; }









//---------------------------------------------------------------------
//                            new Efg class
//---------------------------------------------------------------------



EfgPortion::EfgPortion(void)
{
}

EfgPortion::~EfgPortion()
{ }

BaseEfg*& EfgPortion::Value(void) const
{ return *_Value; }

PortionSpec EfgPortion::Spec(void) const
{ 
  assert((*_Value) != 0);
  switch((*_Value)->Type())
  {
  case DOUBLE:
    return PortionSpec(porEFG_FLOAT);
  case RATIONAL:
    return PortionSpec(porEFG_RATIONAL);
  default:
    assert(0);
  }
  return PortionSpec(porUNDEFINED);
}

DataType EfgPortion::SubType( void ) const
{
  assert( Value() );
  return Value()->Type();
}


void EfgPortion::Output(gOutput& s) const
{
  Portion::Output(s);
  assert(*_Value);
  s << "(Efg) \"" << (*_Value)->GetTitle() << "\""; 
}

gString EfgPortion::OutputString( void ) const
{
  return "(Efg)";
}

Portion* EfgPortion::ValCopy(void) const
{ 
  Portion* p = new EfgValPortion(*_Value); 
  // don't call SetGame() here because they are called in constructor
  // p->SetGame(Game(), GameIsEfg());
  return p;

  /*
  switch((*_Value)->Type())
  {
  case DOUBLE:
    return new EfgValPortion
      (new Efg<double>(* (Efg<double>*) (*_Value))); 
    break;
  case RATIONAL:
    return new EfgValPortion
      (new Efg<gRational>(* (Efg<gRational>*) (*_Value))); 
    break;
  default:
    assert(0);
  }
  return 0;
  */
}

Portion* EfgPortion::RefCopy(void) const
{ 
  Portion* p = new EfgRefPortion(*_Value); 
  // don't call SetGame() here because they are called in constructor
  // p->SetGame(Game(), GameIsEfg());
  p->SetOriginal(Original());
  return p;
}



EfgValPortion::EfgValPortion(BaseEfg* value)
{ 
  _Value = new BaseEfg*(value); 
  SetGame(*_Value, true);
}

EfgValPortion::~EfgValPortion()
{ 
  //delete *_Value;
  delete _Value; 
}

bool EfgValPortion::IsReference(void) const
{ return false; }


EfgRefPortion::EfgRefPortion(BaseEfg*& value)
{
  _Value = &value;
  SetGame(*_Value, true);
}

EfgRefPortion::~EfgRefPortion()
{ }

bool EfgRefPortion::IsReference(void) const
{ return true; }




//---------------------------------------------------------------------
//                          Output class
//---------------------------------------------------------------------

OutputPortion::OutputPortion(void)
{ }

OutputPortion::~OutputPortion()
{ }

gOutput& OutputPortion::Value(void) const
{ return *_Value; }

PortionSpec OutputPortion::Spec(void) const
{ return PortionSpec(porOUTPUT); }

void OutputPortion::Output(gOutput& s) const
{
  Portion::Output(s);
  s << "(Output)"; 
}

gString OutputPortion::OutputString( void ) const
{
  return "(Output)";
}

Portion* OutputPortion::ValCopy(void) const
{ return RefCopy(); }

Portion* OutputPortion::RefCopy(void) const
{ 
  Portion* p = new OutputRefPortion(*_Value); 
  p->SetOriginal(Original());
  return p;
}



OutputValPortion::OutputValPortion(gOutput& value)
{ _Value = &value; }

OutputValPortion::~OutputValPortion()
{ delete _Value; }

bool OutputValPortion::IsReference(void) const
{ return false; }


OutputRefPortion::OutputRefPortion(gOutput& value)
{ _Value = &value; }

OutputRefPortion::~OutputRefPortion()
{ }

bool OutputRefPortion::IsReference(void) const
{ return true; }




//---------------------------------------------------------------------
//                          Input class
//---------------------------------------------------------------------

InputPortion::InputPortion(void)
{ }

InputPortion::~InputPortion()
{ }

gInput& InputPortion::Value(void) const
{ return *_Value; }

PortionSpec InputPortion::Spec(void) const
{ return PortionSpec(porINPUT); }

void InputPortion::Output(gOutput& s) const
{
  Portion::Output(s);
  s << "(Input)"; 
}

gString InputPortion::OutputString( void ) const
{
  return "(Input)";
}

Portion* InputPortion::ValCopy(void) const
{  return RefCopy(); }

Portion* InputPortion::RefCopy(void) const
{ 
  Portion* p = new InputRefPortion(*_Value); 
  p->SetOriginal(Original());
  return p;
}



InputValPortion::InputValPortion(gInput& value)
{ _Value = &value; }

InputValPortion::~InputValPortion()
{ delete _Value; }

bool InputValPortion::IsReference(void) const
{ return false; }


InputRefPortion::InputRefPortion(gInput& value)
{ _Value = &value; }

InputRefPortion::~InputRefPortion()
{ }

bool InputRefPortion::IsReference(void) const
{ return true; }



//---------------------------------------------------------------------
//                          List class
//---------------------------------------------------------------------

// #include "gblock.h"
#include "glist.h"

ListPortion::ListPortion(void)
{ 
  _ContainsListsOnly = true;
  _DataType = porUNDEFINED;
}

ListPortion::~ListPortion()
{ }


void ListPortion::SetGame(void* game, bool efg)
{
  int i;
  for(i=1; i<=_Value->Length(); i++)
    (*_Value)[i]->SetGame(game, efg);
}


bool ListPortion::BelongsToGame( void* game ) const
{
  int i;
  for(i=1; i<=_Value->Length(); i++)
    if( (*_Value)[i]->Spec().ListDepth == 0 )
    {
      if( (*_Value)[i]->Game() == game )
	return true;
    }
    else
    {
      if( ((ListPortion*) (*_Value)[i])->BelongsToGame( game ) )
	return true;
    }
  return false;
}



bool ListPortion::MatchGameData( void* game, void* data ) const
{
  int i;
  for(i=1; i<=_Value->Length(); i++)
  {
    PortionSpec spec = (*_Value)[i]->Spec();
    if( (*_Value)[i]->Spec().ListDepth == 0 )
    {
      if( spec.Type & porEFSUPPORT )
      {
	if( ((EfSupportPortion*) (*_Value)[i])->Value() == data )
	  return true;
      }
      if( spec.Type & porEFPLAYER )
      {
	if( ((EfPlayerPortion*) (*_Value)[i])->Value() == data )
	  return true;
      }
      if( spec.Type & porINFOSET )
      {
	if( ((InfosetPortion*) (*_Value)[i])->Value() == data )
	  return true;
      }
      if( spec.Type & porNODE )
      {
	if( ((NodePortion*) (*_Value)[i])->Value() == data )
	  return true;
      }
      if( spec.Type & porACTION )
      {
	if( ((ActionPortion*) (*_Value)[i])->Value() == data )
	  return true;
      }
    }
    else
    {
      if( ((ListPortion*) (*_Value)[i])->MatchGameData( game, data ) )
	return true;
    }
  }
  return false;
}



gList< Portion* >& ListPortion::Value(void) const
{ return *_Value; }



PortionSpec ListPortion::Spec(void) const
{ return PortionSpec(_DataType, _ListDepth(), _IsNull()); }

DataType ListPortion::SubType( void ) const
{
  DataType subtype = DT_ERROR;
  int i = 0;
  for( i = 1; i <= Length(); i++ )
  {
    DataType el_subtype = operator[]( i )->SubType();
    if( el_subtype != DT_ERROR )
    {
      if( subtype == DT_ERROR )
	subtype = el_subtype;
      else if( subtype != el_subtype )
	subtype = DT_MIXED;	
    }
  }
  return subtype;
}


Portion* ListPortion::ValCopy(void) const
{ 
  ListPortion* p = new ListValPortion(*_Value); 
  if(p->_DataType == porUNDEFINED)
    p->_DataType = _DataType;
  return p;
}

Portion* ListPortion::RefCopy(void) const
{ 
  ListPortion* p = new ListRefPortion(*_Value); 
  ((ListPortion*) p)->_DataType = _DataType;
  p->SetOriginal(Original());
  return p;
}


void ListPortion::AssignFrom(Portion* p)
{
  int i;
  int length;
  int result;
  gList< Portion* >& value = *(((ListPortion*) p)->_Value);

  assert(p->Spec() == Spec());
  assert(PortionSpecMatch(((ListPortion*) p)->_DataType, _DataType) || 
	 _DataType == porUNDEFINED || 
	 ((ListPortion*) p)->_DataType == porUNDEFINED);


  Flush();

  for(i = 1, length = value.Length(); i <= length; i++)
  {
    result = Insert(value[i]->ValCopy(), i);
    assert(result != 0);
  }
  if(_DataType == porUNDEFINED)
    _DataType = ((ListPortion*) p)->_DataType;
}

bool ListPortion::operator == (Portion* p) const
{
  bool result = true;
  int i;
  int length = _Value->Length();
  Portion* p1;
  Portion* p2;
  bool type_found;

  if(p->Spec() == Spec())
  {
    if(_Value->Length() == ((ListPortion*) p)->_Value->Length())
    {
      for(i = 1; i <= length; i++)
      {
	p1 = (*_Value)[i];
	p2 = (*(((ListPortion*) p)->_Value))[i];
	if(p1->Spec() == p2->Spec())
	{
	  if(p1->Spec().ListDepth > 0)
	    result = result &
	      (((ListPortion*) p1)->operator==(p2));
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


ListValPortion::ListValPortion(void)
{ 
  // _Value = new gBlock< Portion* >;
  _Value = new gList< Portion* >;
}

ListValPortion::ListValPortion(gList< Portion* >& value)
{ 
  int i;
  int length;
  int result;

  _Value = new gList< Portion* >; 

  for(i = 1, length = value.Length(); i <= length; i++)
  {
    result = Insert(value[i]->ValCopy(), i);
    assert(result != 0);
  }
}

ListValPortion::~ListValPortion()
{
  Flush();
  delete _Value;
}

bool ListValPortion::IsReference(void) const
{ return false; }


// ListRefPortion::ListRefPortion(gBlock< Portion* >& value)
ListRefPortion::ListRefPortion(gList< Portion* >& value)
{ _Value = &value; }

ListRefPortion::~ListRefPortion()
{ }

bool ListRefPortion::IsReference(void) const
{ return true; }







bool ListPortion::_IsNull(void) const
{
  int i;
  bool null = false;

  if(Length() > 0)
  {
    for(i=1; i<=Length(); i++)
    {
      assert(i>=1 && i<=Length());
      null = null | operator[](i)->Spec().Null;
    }
  }

  return null;
}

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

bool ListPortion::ContainsListsOnly(void) const
{
  if(_Value->Length() == 0)
    return false;
  else
    return _ContainsListsOnly;
}

void ListPortion::SetDataType(unsigned long type)
{ _DataType = type; }

void ListPortion::Output(gOutput& s) const
{ Output(s, 0); }

void ListPortion::Output(gOutput& s, long ListLF) const
{
  Portion::Output(s);
  int i;
  int c;
  int length = _Value->Length();
  
  if(_WriteListBraces) 
    s << '{';
  else 
    s << ' ';
  // if(_WriteListLF > ListLF) s << '\n';
  if(length >= 1)
  {
    for(i = 1; i <= length; i++)
    {
      if(i > 1)
      {
	if(_WriteListCommas) 
	  s << ',';
	else
	  s << ' ';
	if(_WriteListLF > ListLF) 
	  s << '\n';
	if(_WriteListLF > ListLF) 
	  for(c = 0; c < (ListLF+1) * _WriteListIndent; c++)
	    s << ' ';
      }
      else
	if(_WriteListLF > ListLF) 
	  s << ' ';
      if(_WriteListLF <= ListLF)
	s << ' ';
      if((*_Value)[i]->Spec().ListDepth == 0)
	s << (*_Value)[i];
      else
	((ListPortion*) (*_Value)[i])->Output(s, ListLF + 1);
    }
  }
  else
  {
    if(_WriteListLF > ListLF) 
      for(c = 0; c < (ListLF+1) * _WriteListIndent-1; c++)
	s << ' ';
    s << " (" << PortionSpecToText(_DataType) << ')';
  }

  s << ' ';
  if(_WriteListBraces) 
    s << '}';
  else
    s << ' ';
}


gString ListPortion::OutputString( void ) const
{
  gString text( "{ " );
  int i = 0;
  for( i = 1; i <= Length(); i++ )
  {
    if( i > 1 )
      text += ", ";
    text += operator[]( i )->OutputString();
  }
  text += " }";
  return text;
}


int ListPortion::Append(Portion* item)
{ return Insert(item, _Value->Length() + 1); }


int ListPortion::Insert(Portion* item, int index)
{
  int result = 0;
  PortionSpec item_type;

#ifndef NDEBUG
  if(item->Spec().Type == porREFERENCE)
  {
    gerr << "Portion Error:\n";
    gerr << "  Attempted to insert a ReferencePortion into\n";
    gerr << "  a ListPortion\n";
  }
  assert(item->Spec().Type != porREFERENCE);
#endif
  

  item_type = item->Spec();
  if(item->Spec().ListDepth == 0)
  {
    if(item_type.Type == porNULL)
      item_type = ((NullPortion*) item)->DataType();
    _ContainsListsOnly = false;
  }


  /*
  if(item->Spec().Type == porNULL) // inserting a null object
  {
    if(_DataType == porUNDEFINED)
    {
      _DataType = ((NullPortion*) item)->DataType();
      ((ListPortion*) Original())->_DataType = _DataType;
    }
    delete item;
    result = -1;
  }
  else 
  */
  if(_DataType == porUNDEFINED) // inserting into an empty list
  {
    _DataType = item_type.Type;
    ((ListPortion*) Original())->_DataType = _DataType;
    result = _Value->Insert(item, index);
  }
  else  // inserting into an existing list
  {
    if(PortionSpecMatch(item_type.Type, _DataType))
    {
      result = _Value->Insert(item, index);
    }
    else if(item_type.Type == porUNDEFINED) // inserting an empty list
    {
      result = _Value->Insert(item, index);
      assert(item->Spec().ListDepth > 0);
      ((ListPortion*) item)->_DataType = _DataType;
    }
    else if(item_type.Type == porERROR)
      result = _Value->Insert(item, index);
    else
      delete item;
  }
  return result;
}


bool ListPortion::Contains(Portion* p2) const
{
  int i;
  int length = _Value->Length();
  bool type_found;
  Portion* p1;

  for(i = 1; i <= length; i++)
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


Portion* ListPortion::Remove(int index)
{ 
  Portion* result;
  if(index >= 1 && index <= _Value->Length())
    result = _Value->Remove(index);
  else
    result = 0;
  if(_Value->Length() == 0)
    _ContainsListsOnly = true;
  return result;
}

int ListPortion::Length(void) const
{ return _Value->Length(); }


void ListPortion::Flush(void)
{
  int i, length;
  for(i = 1, length = _Value->Length(); i <= length; i++)
  {
    delete Remove(1);
  }
  assert(_Value->Length() == 0);
}


Portion* ListPortion::operator[](int index) const
{
  if(index >= 1 && index <= _Value->Length())
  {
    assert((*_Value)[index] != 0);
    return (*_Value)[index];
  }
  else
    return 0;
}



Portion* ListPortion::SubscriptCopy(int index) const
{
  Portion* p;
  if(index >= 1 && index <= _Value->Length())
  {
    assert((*_Value)[index] != 0);

    if(IsReference())
      p = (*_Value)[index]->RefCopy();
    else
      p = (*_Value)[index]->ValCopy();
      
    return p;
  }
  else
    return 0;
}
//-------------------------------------------------------------------
//--------------------------------------------------------------------





gOutput& operator << (gOutput& s, Portion* p)
{
  p->Output(s);
  return s;
}


bool PortionEqual(Portion* p1, Portion* p2, bool& type_found)
{
  bool b = false;

  if(!(p1->Spec() == p2->Spec())) 
    return false;
  

  if( p1->Spec().ListDepth > 0 )
    return ((ListPortion*) p1)->operator==( (ListPortion*) p2 );



  type_found = true;

  if(p1->Spec().Type & porBOOL)   
    b = (((BoolPortion*) p1)->Value() == ((BoolPortion*) p2)->Value());
  else if(p1->Spec().Type & porINTEGER)   
    b = (((IntPortion*) p1)->Value() == ((IntPortion*) p2)->Value());
  else if(p1->Spec().Type & porFLOAT)
    b = (((FloatPortion*) p1)->Value() == ((FloatPortion*) p2)->Value());
  else if(p1->Spec().Type & porRATIONAL)
    b = (((RationalPortion*) p1)->Value()==((RationalPortion*) p2)->Value());
  else if(p1->Spec().Type & porTEXT)
      b = (((TextPortion*) p1)->Value() == ((TextPortion*) p2)->Value());
  
  else if(p1->Spec().Type & porNODE)
    b = (((NodePortion*) p1)->Value() == ((NodePortion*) p2)->Value());
  else if(p1->Spec().Type & porACTION)
      b = (((ActionPortion*) p1)->Value() == ((ActionPortion*) p2)->Value());
  else if(p1->Spec().Type & porINFOSET)
    b = (((InfosetPortion*) p1)->Value() == ((InfosetPortion*) p2)->Value());
  else if(p1->Spec().Type & porOUTCOME)
    b = (((OutcomePortion*) p1)->Value() == ((OutcomePortion*) p2)->Value());
  else if(p1->Spec().Type & porNFPLAYER)
    b = (((NfPlayerPortion*) p1)->Value() == ((NfPlayerPortion*) p2)->Value());
  else if(p1->Spec().Type & porEFPLAYER)
    b = (((EfPlayerPortion*) p1)->Value() == ((EfPlayerPortion*) p2)->Value());
  else if(p1->Spec().Type & porSTRATEGY)
    b = (((StrategyPortion*) p1)->Value() == ((StrategyPortion*) p2)->Value());
  else if(p1->Spec().Type & porNFSUPPORT)
    b = (*(((NfSupportPortion*) p1)->Value()) ==
	 *(((NfSupportPortion*) p2)->Value()));
  else if(p1->Spec().Type & porEFSUPPORT)
    b = (*(((EfSupportPortion*) p1)->Value()) ==
	 *(((EfSupportPortion*) p2)->Value()));
  
  else if(p1->Spec().Type & porMIXED_FLOAT)
    b = ((*((MixedSolution<double>*) ((MixedPortion*) p1)->Value())) == 
	 (*((MixedSolution<double>*) ((MixedPortion*) p2)->Value())));
  else if(p1->Spec().Type & porMIXED_RATIONAL)
    b = ((*((MixedSolution<gRational>*) ((MixedPortion*) p1)->Value())) == 
	 (*((MixedSolution<gRational>*) ((MixedPortion*) p2)->Value())));
  else if(p1->Spec().Type & porBEHAV_FLOAT)
    b = ((*((BehavSolution<double>*) ((BehavPortion*) p1)->Value())) == 
	 (*((BehavSolution<double>*) ((BehavPortion*) p2)->Value())));
  else if(p1->Spec().Type & porBEHAV_RATIONAL)
    b = ((*((BehavSolution<gRational>*) ((BehavPortion*) p1)->Value())) == 
	 (*((BehavSolution<gRational>*) ((BehavPortion*) p2)->Value())));

  else if(p1->Spec().Type & porNFG)
    b = false;
  else if(p1->Spec().Type & porEFG)
    b = false;
  else if(p1->Spec().Type & porINPUT)
    b = false;
  else if(p1->Spec().Type & porOUTPUT)
    b = false;
  else if(p1->Spec().Type & porNULL)
    b = false;

  else
  {
    type_found = false;
    assert( 0 );
  }
  return b;
}


