//
// FILE: portion.cc -- implementation of Portion base and descendent classes
//                     companion to GSM
//
// @(#)portion.cc	2.23 19 Jul 1997
//


#include <assert.h>
#include <string.h>


//----------------------------------------------------------------------
//                         class instantiations
//----------------------------------------------------------------------



class Portion;

#include "garray.imp"
#include "gblock.imp"

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
  : _Original(0), _Game(0), _GameIsEfg(false)
{
#ifdef MEMCHECK
  _NumObj++;
  printf("--- Portion Ctor, count: %d\n", _NumObj);
#endif
}

Portion::~Portion()
{ 
  if (_Game && _GameIsEfg)
    SetGame((Efg *) 0);
  else if (_Game)
    SetGame((Nfg *) 0);

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
  return _Game;
}

bool Portion::GameIsEfg(void) const
{
  switch(Spec().Type)
  {
  case porNFG:
    return false;
  case porEFG:
    return true;
  default:
    return _GameIsEfg;
  }
}

void Portion::SetGame(const Nfg *game)
{
  if (game != _Game)  {
    if (_Game)  { 
#ifdef MEMCHECK
      gout<<"Game "<<_Game<<" ref count-: "<<(_gsm->GameRefCount(_Game)-1)<<'\n';
#endif
      if (--_gsm->GameRefCount(_Game) == 0)
	delete (Nfg *) _Game;
    }    
     
    _Game = (void *) game;
    _GameIsEfg = false;
      
    if (_Game)  {
      _gsm->GameRefCount(_Game)++;
#ifdef MEMCHECK
      gout<<"Game "<<_Game<<" ref count+: "<<_gsm->GameRefCount(_Game)<<'\n';
#endif
    }
  }
}


void Portion::SetGame(const Efg *game)
{
  if (game != _Game)  {
    if (_Game)  {
#ifdef MEMCHECK
      gout<<"Game "<<_Game<<" ref count-: "<<(_gsm->GameRefCount(_Game)-1)<<'\n';
#endif
      if (--_gsm->GameRefCount(_Game) == 0)   
// Removed to temporarily solve a problem somewhere with reference counting
// of efgs.  Of course, note that this causes a memory leak!  This needs
// to be tracked down in the near future.
//	delete (Efg*) _Game;
	;
    }
    
    _Game = (void *) game;
    _GameIsEfg = true;
    
    if (_Game)  {
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
//                          Precision class
//---------------------------------------------------------------------

PrecisionPortion::PrecisionPortion(Precision value)
  : _Value(new Precision(value)), _ref(false)
{ }

PrecisionPortion::PrecisionPortion(Precision &value, bool ref)
  : _Value(&value), _ref(ref)
{ }

PrecisionPortion::~PrecisionPortion()
{ }

Precision& PrecisionPortion::Value(void) const
{ return *_Value; }

PortionSpec PrecisionPortion::Spec(void) const
{ return PortionSpec(porPRECISION); }

void PrecisionPortion::Output(gOutput& s) const
{
  Portion::Output(s);
  s << ((*_Value == precDOUBLE) ? "Machine" : "Rational");
}

gString PrecisionPortion::OutputString( void ) const
{
  return (*_Value == precDOUBLE) ? "Machine" : "Rational";
}

Portion* PrecisionPortion::ValCopy(void) const
{ return new PrecisionPortion(*_Value); }

Portion* PrecisionPortion::RefCopy(void) const
{ 
  Portion* p = new PrecisionPortion(*_Value, true);
  p->SetOriginal(Original());
  return p;
}

bool PrecisionPortion::IsReference(void) const
{ return _ref; }


//---------------------------------------------------------------------
//                          int class
//---------------------------------------------------------------------

IntPortion::IntPortion(long value)
  : _Value(new long(value)), _ref(false)
{ }

IntPortion::IntPortion(long &value, bool ref)
  : _Value(&value), _ref(ref)
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
{ return new IntPortion(*_Value); }

Portion* IntPortion::RefCopy(void) const
{ 
  Portion* p = new IntPortion(*_Value, true);
  p->SetOriginal(Original());
  return p;
}

bool IntPortion::IsReference(void) const
{ return _ref; }


//---------------------------------------------------------------------
//                          Rational class
//---------------------------------------------------------------------

NumberPortion::NumberPortion(const gRational &value)
  : _Value(new gRational(value)), _ref(false)
{ }

NumberPortion::NumberPortion(gRational &value, bool ref)
  : _Value(&value), _ref(ref)
{ }

NumberPortion::~NumberPortion()
{ }

gRational& NumberPortion::Value(void) const
{ return *_Value; }

PortionSpec NumberPortion::Spec(void) const
{ return PortionSpec(porNUMBER); }

void NumberPortion::Output(gOutput& s) const
{
  Portion::Output(s);
  s << *_Value; 
}

gString NumberPortion::OutputString( void ) const
{
  return ToString( *_Value );
}

Portion* NumberPortion::ValCopy(void) const
{ return new NumberPortion(*_Value); }

Portion* NumberPortion::RefCopy(void) const
{ 
  Portion* p = new NumberPortion(*_Value, true); 
  p->SetOriginal(Original());
  return p;
}

bool NumberPortion::IsReference(void) const
{ return _ref; }




//---------------------------------------------------------------------
//                          Text class
//---------------------------------------------------------------------

TextPortion::TextPortion(const gString &value)
  : _Value(new gString(value)), _ref(false)
{ }

TextPortion::TextPortion(gString &value, bool ref)
  : _Value(&value), _ref(ref)
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
{ return new TextPortion(*_Value); }

Portion* TextPortion::RefCopy(void) const
{ 
  Portion* p = new TextPortion(*_Value, true); 
  p->SetOriginal(Original());
  return p;
}

bool TextPortion::IsReference(void) const
{ return _ref; }


//---------------------------------------------------------------------
//                          Bool class
//---------------------------------------------------------------------

BoolPortion::BoolPortion(bool value)
  : _Value(new bool(value)), _ref(false)
{ }

BoolPortion::BoolPortion(bool &value, bool ref)
  : _Value(&value), _ref(ref)
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
{ return new BoolPortion(*_Value); }

Portion* BoolPortion::RefCopy(void) const
{ 
  Portion* p = new BoolPortion(*_Value, true);
  p->SetOriginal(Original());
  return p;
}

bool BoolPortion::IsReference(void) const
{ return _ref; }


//---------------------------------------------------------------------
//                          EFOutcome class
//---------------------------------------------------------------------

EfOutcomePortion::EfOutcomePortion(EFOutcome *value)
  : _Value(new EFOutcome *(value)), _ref(false)
{
  SetGame(value->BelongsTo());
}

EfOutcomePortion::EfOutcomePortion(EFOutcome *&value, bool ref)
  : _Value(&value), _ref(ref)
{
  SetGame(value->BelongsTo());
}

EfOutcomePortion::~EfOutcomePortion()
{
  if (!_ref)  delete _Value;
}

EFOutcome *EfOutcomePortion::Value(void) const
{ return *_Value; }

void EfOutcomePortion::SetValue(EFOutcome *value)
{
  SetGame(value->BelongsTo());
  *_Value = value;
}

PortionSpec EfOutcomePortion::Spec(void) const
{
  return porEFOUTCOME;
}

void EfOutcomePortion::Output(gOutput& s) const
{
  Portion::Output(s);
  
  s << "(EFOutcome) " << *_Value;
  if(*_Value)
    s << " \"" << (*_Value)->GetName() << "\"\n";
}

gString EfOutcomePortion::OutputString( void ) const
{
  return "(EFOutcome)";
}

Portion* EfOutcomePortion::ValCopy(void) const
{ 
  return new EfOutcomePortion(*_Value);
}

Portion* EfOutcomePortion::RefCopy(void) const
{ 
  Portion* p = new EfOutcomePortion(*_Value, true); 
  p->SetOriginal(Original());
  return p;
}

bool EfOutcomePortion::IsReference(void) const
{ return _ref; }


//---------------------------------------------------------------------
//                          NfPlayer class
//---------------------------------------------------------------------


NfPlayerPortion::NfPlayerPortion(NFPlayer *value)
  : _Value(new NFPlayer *(value)), _ref(false)
{
  SetGame(&value->Game());
}

NfPlayerPortion::NfPlayerPortion(NFPlayer *&value, bool ref)
  : _Value(&value), _ref(ref)
{
  SetGame(&value->Game());
}

NfPlayerPortion::~NfPlayerPortion()
{
  if (!_ref)   delete _Value;
}

NFPlayer *NfPlayerPortion::Value(void) const
{ return *_Value; }

void NfPlayerPortion::SetValue(NFPlayer *value)
{
  SetGame(&value->Game());
  *_Value = value;
}

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
  return new NfPlayerPortion(*_Value); 
}

Portion* NfPlayerPortion::RefCopy(void) const
{
  Portion* p = new NfPlayerPortion(*_Value, true); 
  p->SetOriginal(Original());
  return p;
}

bool NfPlayerPortion::IsReference(void) const
{ return _ref; }


//---------------------------------------------------------------------
//                          Strategy class
//---------------------------------------------------------------------

StrategyPortion::StrategyPortion(Strategy *value)
  : _Value(new Strategy *(value)), _ref(false)
{
  SetGame(&value->nfp->Game());
}

StrategyPortion::StrategyPortion(Strategy *&value, bool ref)
  : _Value(&value), _ref(ref)
{
  SetGame(&value->nfp->Game());
}

StrategyPortion::~StrategyPortion()
{
  if (!_ref)   delete _Value;
}

Strategy *StrategyPortion::Value(void) const
{ return *_Value; }

void StrategyPortion::SetValue(Strategy *value)
{
  SetGame(&value->nfp->Game());
  *_Value = value;
}

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
  return new StrategyPortion(*_Value); 
}

Portion* StrategyPortion::RefCopy(void) const
{
  Portion* p = new StrategyPortion(*_Value, true); 
  p->SetOriginal(Original());
  return p;
}

bool StrategyPortion::IsReference(void) const
{ return _ref; }



//---------------------------------------------------------------------
//                          NfOutcome class
//---------------------------------------------------------------------


NfOutcomePortion::NfOutcomePortion(NFOutcome *value)
  : _Value(new NFOutcome *(value)), _ref(false)
{
  SetGame(value->Game());
}

NfOutcomePortion::NfOutcomePortion(NFOutcome *&value, bool ref)
  : _Value(&value), _ref(ref)
{
  SetGame(value->Game());
}

NfOutcomePortion::~NfOutcomePortion()
{
  if (!_ref)   delete _Value;
}

NFOutcome *NfOutcomePortion::Value(void) const
{ return *_Value; }

void NfOutcomePortion::SetValue(NFOutcome *value)
{
  SetGame(value->Game());
  *_Value = value;
}

PortionSpec NfOutcomePortion::Spec(void) const
{ 
  return porNFOUTCOME;
}

void NfOutcomePortion::Output(gOutput& s) const
{
  Portion::Output(s);
  
  s << "(NFOutcome) " << *_Value;
  if (*_Value)
    s << " \"" << (*_Value)->GetName() << "\"";
}

gString NfOutcomePortion::OutputString( void ) const
{
  return "(Outcome)";
}

Portion* NfOutcomePortion::ValCopy(void) const
{ 
  return new NfOutcomePortion(*_Value);
}

Portion* NfOutcomePortion::RefCopy(void) const
{ 
  Portion* p = new NfOutcomePortion(*_Value, true); 
  p->SetOriginal(Original());
  return p;
}

bool NfOutcomePortion::IsReference(void) const
{ return _ref; }


//---------------------------------------------------------------------
//                          NfSupport class
//---------------------------------------------------------------------

NfSupportPortion::NfSupportPortion(NFSupport *value)
  : _Value(new NFSupport *(value)), _ref(false)
{
  SetGame(&value->Game());
}

NfSupportPortion::NfSupportPortion(NFSupport *&value, bool ref)
  : _Value(&value), _ref(ref)
{
  SetGame(&value->Game());
}

NfSupportPortion::~NfSupportPortion()
{
  if (!_ref)   {
    delete *_Value;
    delete _Value;
  }
}

NFSupport *NfSupportPortion::Value(void) const
{ return *_Value; }

void NfSupportPortion::SetValue(NFSupport *value)
{
  SetGame(&value->Game());
  delete *_Value;
  *_Value = value;
}

PortionSpec NfSupportPortion::Spec(void) const
{
  return porNFSUPPORT;
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
  return new NfSupportPortion(new NFSupport(**_Value));
}

Portion* NfSupportPortion::RefCopy(void) const
{
  Portion* p = new NfSupportPortion(*_Value, true);
  p->SetOriginal(Original());
  return p;
}


bool NfSupportPortion::IsReference(void) const
{ return _ref; }





//---------------------------------------------------------------------
//                          EfSupport class
//---------------------------------------------------------------------


EfSupportPortion::EfSupportPortion(EFSupport *value)
  : _Value(new EFSupport *(value)), _ref(false)
{
  SetGame(&value->Game());
}

EfSupportPortion::EfSupportPortion(EFSupport *&value, bool ref)
  : _Value(&value), _ref(ref)
{
  SetGame(&value->Game());
}

EfSupportPortion::~EfSupportPortion()
{
  if (!_ref)   {
    delete *_Value;
    delete _Value;
  }
}

EFSupport *EfSupportPortion::Value(void) const
{ return *_Value; }

void EfSupportPortion::SetValue(EFSupport *value)
{
  SetGame(&value->Game());
  delete *_Value;
  *_Value = value;
}

PortionSpec EfSupportPortion::Spec(void) const
{
  return PortionSpec(porEFSUPPORT);
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
  return new EfSupportPortion(new EFSupport(**_Value)); 
}

Portion* EfSupportPortion::RefCopy(void) const
{
  Portion* p = new EfSupportPortion(*_Value, true); 
  p->SetOriginal(Original());
  return p;
}

bool EfSupportPortion::IsReference(void) const
{ return _ref; }


//---------------------------------------------------------------------
//                          EfPlayer class
//---------------------------------------------------------------------

EfPlayerPortion::EfPlayerPortion(EFPlayer *value)
  : _Value(new EFPlayer *(value)), _ref(false)
{
  SetGame(value->Game());
}

EfPlayerPortion::EfPlayerPortion(EFPlayer *& value, bool ref)
  : _Value(&value), _ref(ref)
{
  SetGame(value->Game());
}

EfPlayerPortion::~EfPlayerPortion()
{
  if (!_ref)   delete _Value;
}

EFPlayer *EfPlayerPortion::Value(void) const
{ return *_Value; }

void EfPlayerPortion::SetValue(EFPlayer *value)
{
  SetGame(value->Game());
  *_Value = value;
}

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
  return new EfPlayerPortion(*_Value); 
}

Portion* EfPlayerPortion::RefCopy(void) const
{
  Portion* p = new EfPlayerPortion(*_Value, true); 
  p->SetOriginal(Original());
  return p;
}

bool EfPlayerPortion::IsReference(void) const
{
  return _ref;
}

//---------------------------------------------------------------------
//                          Infoset class
//---------------------------------------------------------------------

InfosetPortion::InfosetPortion(Infoset *value)
  : _Value(new Infoset *(value)), _ref(false)
{
  SetGame(value->Game());
}

InfosetPortion::InfosetPortion(Infoset *& value, bool ref)
  : _Value(&value), _ref(ref)
{
  SetGame(value->Game());
}

InfosetPortion::~InfosetPortion()
{
  if (!_ref)   delete _Value;
}


Infoset *InfosetPortion::Value(void) const
{ return *_Value; }

void InfosetPortion::SetValue(Infoset *value)
{
  SetGame(value->Game());
  *_Value = value;
}

PortionSpec InfosetPortion::Spec(void) const
{
  return PortionSpec(porINFOSET);
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
  return new InfosetPortion(*_Value);
}

Portion* InfosetPortion::RefCopy(void) const
{
  Portion* p = new InfosetPortion(*_Value, true); 
  p->SetOriginal(Original());
  return p;
}

bool InfosetPortion::IsReference(void) const
{
  return _ref;
}

//---------------------------------------------------------------------
//                          Node class
//---------------------------------------------------------------------

NodePortion::NodePortion(Node *value)
  : _Value(new Node *(value)), _ref(false)
{
  SetGame(value->Game());
}

NodePortion::NodePortion(Node *&value, bool ref)
  : _Value(&value), _ref(ref)
{
  SetGame(value->Game());
}  

NodePortion::~NodePortion()
{
  if (!_ref)   delete _Value;
}

Node *NodePortion::Value(void) const
{ return *_Value; }

void NodePortion::SetValue(Node *value)
{
  SetGame(value->Game());
  *_Value = value;
}

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
  return new NodePortion(*_Value); 
}

Portion* NodePortion::RefCopy(void) const
{
  Portion* p = new NodePortion(*_Value, true); 
  p->SetOriginal(Original());
  return p;
}

bool NodePortion::IsReference(void) const
{
  return _ref;
}

//---------------------------------------------------------------------
//                          Action class
//---------------------------------------------------------------------

ActionPortion::ActionPortion(Action *value)
  : _Value(new Action *(value)), _ref(false)
{
  SetGame(value->BelongsTo()->Game());
}

ActionPortion::ActionPortion(Action *& value, bool ref)
  : _Value(&value), _ref(ref)
{
  SetGame(value->BelongsTo()->Game());
}

ActionPortion::~ActionPortion()
{
  if (!_ref)   delete _Value;
}

Action *ActionPortion::Value(void) const
{ return *_Value; }

void ActionPortion::SetValue(Action *value)
{
  SetGame(value->BelongsTo()->Game());
  *_Value = value;
}

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
  return new ActionPortion(*_Value); 
}

Portion* ActionPortion::RefCopy(void) const
{
  Portion* p = new ActionPortion(*_Value, true); 
  p->SetOriginal(Original());
  return p;
}

bool ActionPortion::IsReference(void) const
{
  return _ref;
}

//---------------------------------------------------------------------
//                            Mixed class
//---------------------------------------------------------------------

MixedPortion<double>::MixedPortion(MixedProfile<double> *value)
  : _Value(new MixedProfile<double> *(value)), _ref(false)
{
  SetGame(&value->Game());
}

MixedPortion<double>::MixedPortion(MixedProfile<double> *&value, bool ref)
  : _Value(&value), _ref(ref)
{
  SetGame(&value->Game());
}

MixedPortion<double>::~MixedPortion()
{
  if (!_ref)  {
    delete *_Value;
    delete _Value;
  }
}

MixedProfile<double> *MixedPortion<double>::Value(void) const
{ return *_Value; }

void MixedPortion<double>::SetValue(MixedProfile<double> *value)
{
  SetGame(&value->Game());
  delete *_Value;
  *_Value = value;
}

PortionSpec MixedPortion<double>::Spec(void) const
{ 
  return PortionSpec(porMIXED_FLOAT);
}

Precision MixedPortion<double>::SubType( void ) const
{
  assert( Value() );
  return precDOUBLE;
}

void MixedPortion<double>::Output(gOutput& s) const
{
  Portion::Output(s);
  s << "(Mixed) ";
  if (_WriteSolutionInfo>1)
    s << (MixedSolution<double>) *(MixedSolution<double>*)(*_Value); 
  else
    s << (MixedProfile<double>) *(MixedSolution<double>*)(*_Value); 
}


gString MixedPortion<double>::OutputString( void ) const
{
  return "(Mixed)";
}

Portion* MixedPortion<double>::ValCopy(void) const
{ 
  return new MixedPortion<double>
      (new MixedSolution<double>
       (* (MixedSolution<double>*) (*_Value))); 
}

Portion* MixedPortion<double>::RefCopy(void) const
{ 
  Portion* p = new MixedPortion<double>(*_Value, true); 
  p->SetOriginal(Original());
  return p;
}

bool MixedPortion<double>::IsReference(void) const
{
  return _ref;
}



MixedPortion<gRational>::MixedPortion(MixedProfile<gRational> *value)
  : _Value(new MixedProfile<gRational> *(value)), _ref(false)
{
  SetGame(&value->Game());
}

MixedPortion<gRational>::MixedPortion(MixedProfile<gRational> *&value, bool ref)
  : _Value(&value), _ref(ref)
{
  SetGame(&value->Game());
}

MixedPortion<gRational>::~MixedPortion()
{
  if (!_ref)  {
    delete *_Value;
    delete _Value;
  }
}

MixedProfile<gRational> *MixedPortion<gRational>::Value(void) const
{ return *_Value; }

void MixedPortion<gRational>::SetValue(MixedProfile<gRational> *value)
{
  SetGame(&value->Game());
  delete *_Value;
  *_Value = value;
}

PortionSpec MixedPortion<gRational>::Spec(void) const
{ 
  return PortionSpec(porMIXED_RATIONAL);
}

Precision MixedPortion<gRational>::SubType( void ) const
{
  assert( Value() );
  return precRATIONAL;
}

void MixedPortion<gRational>::Output(gOutput& s) const
{
  Portion::Output(s);
  s << "(Mixed) ";
  if (_WriteSolutionInfo>1)
    s << (MixedSolution<gRational>) *(MixedSolution<gRational>*)(*_Value); 
  else
    s << (MixedProfile<gRational>) *(MixedSolution<gRational>*)(*_Value); 
}

gString MixedPortion<gRational>::OutputString( void ) const
{
  return "(Mixed)";
}

Portion* MixedPortion<gRational>::ValCopy(void) const
{ 
  return new MixedPortion<gRational>
      (new MixedSolution<gRational>
       (* (MixedSolution<gRational>*) (*_Value))); 
}

Portion* MixedPortion<gRational>::RefCopy(void) const
{ 
  Portion* p = new MixedPortion<gRational>(*_Value, true); 
  p->SetOriginal(Original());
  return p;
}

bool MixedPortion<gRational>::IsReference(void) const
{
  return _ref;
}



//---------------------------------------------------------------------
//                            Behav class
//---------------------------------------------------------------------

BehavPortion<double>::BehavPortion(BehavProfile<double> *value)
  : _Value(new BehavProfile<double> *(value)), _ref(false)
{
  SetGame(&value->Support().Game());
}

BehavPortion<double>::BehavPortion(BehavProfile<double> *& value, bool ref)
  : _Value(&value), _ref(ref)
{
  SetGame(&value->Support().Game());
}

BehavPortion<double>::~BehavPortion()
{
  if (!_ref)   {
    delete *_Value;
    delete _Value;
  }
}

BehavProfile<double> *BehavPortion<double>::Value(void) const
{ return *_Value; }

void BehavPortion<double>::SetValue(BehavProfile<double> *value)
{
  SetGame(&value->Support().Game());
  delete *_Value;
  *_Value = value;
}

PortionSpec BehavPortion<double>::Spec(void) const
{ 
  return PortionSpec(porBEHAV_FLOAT);
}

Precision BehavPortion<double>::SubType( void ) const
{
  assert( Value() );
  return precDOUBLE;
}

void BehavPortion<double>::Output(gOutput& s) const
{
  Portion::Output(s);
  s << "(Behav) ";
  if (_WriteSolutionInfo>1)
    s << (BehavSolution<double>) *(BehavSolution<double>*)(*_Value); 
  else
    s << (BehavProfile<double>) *(BehavSolution<double>*)(*_Value); 
}

gString BehavPortion<double>::OutputString( void ) const
{
  return "(Behav)";
}

Portion* BehavPortion<double>::ValCopy(void) const
{ 
  return new BehavPortion<double>
      (new BehavSolution<double>
       (* (BehavSolution<double>*) (*_Value))); 
}

Portion* BehavPortion<double>::RefCopy(void) const
{ 
  Portion* p = new BehavPortion<double>(*_Value, true); 
  p->SetOriginal(Original());
  return p;
}

bool BehavPortion<double>::IsReference(void) const
{ return _ref; }


BehavPortion<gRational>::BehavPortion(BehavProfile<gRational> *value)
  : _Value(new BehavProfile<gRational> *(value)), _ref(false)
{
  SetGame(&value->Support().Game());
}

BehavPortion<gRational>::BehavPortion(BehavProfile<gRational> *& value, bool ref)
  : _Value(&value), _ref(ref)
{
  SetGame(&value->Support().Game());
}

BehavPortion<gRational>::~BehavPortion()
{
  if (!_ref)   {
    delete *_Value;
    delete _Value;
  }
}

BehavProfile<gRational> *BehavPortion<gRational>::Value(void) const
{ return *_Value; }

void BehavPortion<gRational>::SetValue(BehavProfile<gRational> *value)
{
  SetGame(&value->Support().Game());
  delete *_Value;
  *_Value = value;
}

PortionSpec BehavPortion<gRational>::Spec(void) const
{ 
  return PortionSpec(porBEHAV_RATIONAL);
}

Precision BehavPortion<gRational>::SubType( void ) const
{
  assert( Value() );
  return precRATIONAL;
}

void BehavPortion<gRational>::Output(gOutput& s) const
{
  Portion::Output(s);
  s << "(Behav) ";
  if (_WriteSolutionInfo>1)
    s << (BehavSolution<gRational>) *(BehavSolution<gRational>*)(*_Value); 
  else
    s << (BehavProfile<gRational>) *(BehavSolution<gRational>*)(*_Value); 
}

gString BehavPortion<gRational>::OutputString( void ) const
{
  return "(Behav)";
}

Portion* BehavPortion<gRational>::ValCopy(void) const
{ 
  return new BehavPortion<gRational>
      (new BehavSolution<gRational>
       (* (BehavSolution<gRational>*) (*_Value))); 
}

Portion* BehavPortion<gRational>::RefCopy(void) const
{ 
  Portion* p = new BehavPortion<gRational>(*_Value, true); 
  p->SetOriginal(Original());
  return p;
}

bool BehavPortion<gRational>::IsReference(void) const
{ return _ref; }




//---------------------------------------------------------------------
//                            new Nfg class
//---------------------------------------------------------------------



NfgPortion::NfgPortion(void)
{ }

NfgPortion::~NfgPortion()
{ }

Nfg *NfgPortion::Value(void) const
{ return *_Value; }

void NfgPortion::SetValue(Nfg *value)
{
  SetGame(value);
  *_Value = value;
}

PortionSpec NfgPortion::Spec(void) const
{ 
  return PortionSpec(porNFG);
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
  return new NfgValPortion(*_Value);
}

Portion* NfgPortion::RefCopy(void) const
{ 
  Portion* p = new NfgRefPortion(*_Value);
  p->SetOriginal(Original());
  return p;
}



NfgValPortion::NfgValPortion(Nfg *value)
{
  _Value = new Nfg *(value);
  SetGame(*_Value);
}

NfgValPortion::~NfgValPortion()
{ 
  delete _Value; 
}

bool NfgValPortion::IsReference(void) const
{ return false; }


NfgRefPortion::NfgRefPortion(Nfg *&value)
{
  _Value = &value; 
  SetGame(*_Value);
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

Efg *EfgPortion::Value(void) const
{ return *_Value; }

void EfgPortion::SetValue(Efg *value)
{
  *_Value = value;
  SetGame(*_Value);
}

PortionSpec EfgPortion::Spec(void) const
{
  return PortionSpec(porEFG);
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
  return new EfgValPortion(*_Value);
}

Portion* EfgPortion::RefCopy(void) const
{ 
  Portion* p = new EfgRefPortion(*_Value); 
  p->SetOriginal(Original());
  return p;
}



EfgValPortion::EfgValPortion(Efg* value)
{ 
  _Value = new Efg*(value); 
  SetGame(*_Value);
}

EfgValPortion::~EfgValPortion()
{ 
  delete _Value; 
}

bool EfgValPortion::IsReference(void) const
{ return false; }


EfgRefPortion::EfgRefPortion(Efg*& value)
{
  _Value = &value;
  SetGame(*_Value);
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

  _IsNull = false;
  _ListDepth = 1;
}

ListPortion::~ListPortion()
{ }


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
{ 
  if (IsReference())
    return Original()->Spec();
  else
    return PortionSpec(_DataType, _ListDepth, _IsNull); 
}

Precision ListPortion::SubType( void ) const
{
  Precision subtype = precERROR;
  int i = 0;
  for( i = 1; i <= Length(); i++ )
  {
    Precision el_subtype = operator[]( i )->SubType();
    if( el_subtype != precERROR )
    {
      if( subtype == precERROR )
	subtype = el_subtype;
      else if( subtype != el_subtype )
	subtype = precMIXED;
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






/*
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
*/

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

  if( result > 0 )
  {
    if( item->Spec().ListDepth + 1 > _ListDepth )
      _ListDepth = item->Spec().ListDepth + 1;
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
    if(PortionEqual(p1, p2, type_found))
      return true;

    /* uncomment this to do recursive checking
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
    */
  }
  return false;
}


Portion* ListPortion::Remove(int index)
{ 
  Portion* result = 0;
  if(index >= 1 && index <= _Value->Length())
    result = _Value->Remove(index);

  _ContainsListsOnly = true;
  _ListDepth = 1;
  if(_Value->Length() > 0)
  {
    int i = 0;
    for( i = 1; i <= _Value->Length(); ++i )
    {
      if( (*_Value)[i]->Spec().ListDepth == 0 )
	_ContainsListsOnly = false;
      if( (*_Value)[i]->Spec().ListDepth >= _ListDepth )
	_ListDepth = (*_Value)[i]->Spec().ListDepth + 1;
    }
  }

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
  else if(p1->Spec().Type & porNUMBER)
    b = (((NumberPortion*) p1)->Value()==((NumberPortion*) p2)->Value());
  else if(p1->Spec().Type & porTEXT)
      b = (((TextPortion*) p1)->Value() == ((TextPortion*) p2)->Value());
  
  else if(p1->Spec().Type & porNODE)
    b = (((NodePortion*) p1)->Value() == ((NodePortion*) p2)->Value());
  else if(p1->Spec().Type & porACTION)
      b = (((ActionPortion*) p1)->Value() == ((ActionPortion*) p2)->Value());
  else if(p1->Spec().Type & porINFOSET)
    b = (((InfosetPortion*) p1)->Value() == ((InfosetPortion*) p2)->Value());
  else if(p1->Spec().Type & porEFOUTCOME)
    b = (((EfOutcomePortion*) p1)->Value() == ((EfOutcomePortion*) p2)->Value());
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
    b = ((*((MixedSolution<double>*) ((MixedPortion<double>*) p1)->Value())) == 
	 (*((MixedSolution<double>*) ((MixedPortion<double>*) p2)->Value())));
  else if(p1->Spec().Type & porMIXED_RATIONAL)
    b = ((*((MixedSolution<gRational>*) ((MixedPortion<gRational>*) p1)->Value())) == 
	 (*((MixedSolution<gRational>*) ((MixedPortion<gRational>*) p2)->Value())));
  else if(p1->Spec().Type & porBEHAV_FLOAT)
    b = ((*((BehavSolution<double>*) ((BehavPortion<double>*) p1)->Value())) == 
	 (*((BehavSolution<double>*) ((BehavPortion<double>*) p2)->Value())));
  else if(p1->Spec().Type & porBEHAV_RATIONAL)
    b = ((*((BehavSolution<gRational>*) ((BehavPortion<gRational>*) p1)->Value())) == 
	 (*((BehavSolution<gRational>*) ((BehavPortion<gRational>*) p2)->Value())));

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






template class MixedPortion<double>;
template class MixedPortion<gRational>;

template class BehavPortion<double>;
template class BehavPortion<gRational>;
