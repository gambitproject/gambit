//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of GCL data types
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include "base/base.h"

#include "portion.h"
#include "gsmhash.h"

#include "gsm.h"

#include "game/nfg.h"
#include "game/efg.h"


//-----------
// Portion
//-----------

gNumber Portion::_WriteWidth = 0;
gNumber Portion::_WritePrecis = 6;
gTriState Portion::_WriteExpmode = triFALSE;
gTriState Portion::_WriteQuoted = triTRUE;
gTriState Portion::_WriteListBraces = triTRUE;
gTriState Portion::_WriteListCommas = triTRUE;
gNumber Portion::_WriteListLF = 0;
gNumber Portion::_WriteListIndent = 2;
gTriState Portion::_WriteSolutionInfo = triFALSE;
gTriState Portion::_WriteSolutionLabels = triTRUE;

void Portion::_SetWriteWidth(long x)
{ _WriteWidth = x; }
void Portion::_SetWritePrecis(long x)
{ _WritePrecis = x; }
void Portion::_SetWriteExpmode(bool x)
{ _WriteExpmode = (x) ? triTRUE : triFALSE; }
void Portion::_SetWriteQuoted(bool x)
{ _WriteQuoted = (x) ? triTRUE : triFALSE; }
void Portion::_SetWriteListBraces(bool x)
{ _WriteListBraces = (x) ? triTRUE : triFALSE; }
void Portion::_SetWriteListCommas(bool x)
{ _WriteListCommas = (x) ? triTRUE : triFALSE; }
void Portion::_SetWriteListLF(long x)
{ _WriteListLF = x; }
void Portion::_SetWriteListIndent(long x)
{ _WriteListIndent = x; }
void Portion::_SetWriteSolutionInfo(bool x)
{ _WriteSolutionInfo = (x) ? triTRUE : triFALSE; }
void Portion::_SetWriteSolutionLabels(bool x)
{ _WriteSolutionLabels = (x) ? triTRUE : triFALSE; }


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
  //  printf("--- Portion Ctor, count: %d\n", _NumObj);
#endif
}

Portion::~Portion()
{ 
  if (_Game && _GameIsEfg) {
    SetGame((efgGame *) 0);
  }
  else if (_Game) {
    SetGame((Nfg *) 0);
  }

#ifdef MEMCHECK
  _NumObj--;
  //printf("--- Portion Dtor, count: %d\n", _NumObj);
#endif
}


void Portion::SetOriginal(const Portion* p)
{ 
  _Original = (Portion*) p;
}

Portion *Portion::Original(void) const
{ 
  if (!IsReference()) {
    return (Portion *) this;
  }
  else  {
    return _Original; 
  }
}

void *Portion::Game(void) const
{
  return _Game;
}

bool Portion::GameIsEfg(void) const
{
  switch (Spec().Type)  {
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
      GSM::GameRefCount(_Game)--;
#ifdef MEMCHECK
      gout<<"Game "<<_Game<<" ref count-: "<< _gsm->GameRefCount(_Game) <<'\n';
#endif
      if (GSM::GameRefCount(_Game) == 0) 
        delete (Nfg *) _Game;
    }    
     
    _Game = (void *) game;
    _GameIsEfg = false;
      
    if (_Game)  {
      GSM::GameRefCount(_Game)++;
#ifdef MEMCHECK
      gout<<"Game "<<_Game<<" ref count+: "<<_gsm->GameRefCount(_Game)<<'\n';
#endif
    }
  }
}

void Portion::SetGame(const efgGame *game)
{
  if (game != _Game)  {
    if (_Game)  {
      GSM::GameRefCount(_Game)--;
#ifdef MEMCHECK
      gout<<"Game "<<_Game<<" ref count-: "<< _gsm->GameRefCount(_Game) <<'\n';
#endif
      if (GSM::GameRefCount(_Game) == 0)   
	delete (efgGame *) _Game;
    }
    
    _Game = (void *) game;
    _GameIsEfg = true;
    
    if (_Game)  {
      GSM::GameRefCount(_Game)++;
#ifdef MEMCHECK
      gout<<"Game "<<_Game<<" ref count+: "<<_gsm->GameRefCount(_Game)<<'\n';
#endif
    }
  }
}


//--------
// Null
//--------

gPool NullPortion::pool(sizeof(NullPortion));

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

gText NullPortion::OutputString(void) const
{
  return (gText) "Null(" + PortionSpecToText(_DataType) + ")";
}

Portion* NullPortion::ValCopy(void) const
{ return new NullPortion(_DataType); }

Portion* NullPortion::RefCopy(void) const
{ return new NullPortion(_DataType); }

bool NullPortion::IsReference(void) const
{ return false; }


//-------------
// Reference
//-------------

gPool ReferencePortion::pool(sizeof(ReferencePortion));

ReferencePortion::ReferencePortion(const gText& value)
: _Value(value)
{ }

ReferencePortion::~ReferencePortion()
{ }

gText ReferencePortion::Value(void)
{ return _Value; }

PortionSpec ReferencePortion::Spec(void) const
{ return PortionSpec(porREFERENCE); }

void ReferencePortion::Output(gOutput& s) const
{ 
  Portion::Output(s);
  s << OutputString();
}

gText ReferencePortion::OutputString(void) const
{
  return (gText) "(Reference) \"" + _Value + "\""; 
}

Portion* ReferencePortion::ValCopy(void) const
{ return new ReferencePortion(_Value); }

Portion* ReferencePortion::RefCopy(void) const
{ return new ReferencePortion(_Value); }

bool ReferencePortion::IsReference(void) const
{ return false; }


//-------------
// Precision
//-------------

gPool PrecisionPortion::pool(sizeof(PrecisionPortion));

PrecisionPortion::PrecisionPortion(gPrecision value)
  : _Value(new gPrecision(value)), _ref(false)
{ }

PrecisionPortion::PrecisionPortion(gPrecision &value, bool ref)
  : _Value(&value), _ref(ref)
{ }

PrecisionPortion::~PrecisionPortion()
{
  if (!_ref)
    delete _Value;
}

gPrecision PrecisionPortion::Value(void) const
{ return *_Value; }

void PrecisionPortion::SetValue(gPrecision p)
{ *_Value = p; }

PortionSpec PrecisionPortion::Spec(void) const
{ return PortionSpec(porPRECISION); }

void PrecisionPortion::Output(gOutput& s) const
{
  Portion::Output(s);
  s << ((*_Value == precDOUBLE) ? "Float" : "Rational");
}

gText PrecisionPortion::OutputString(void) const
{
  return (*_Value == precDOUBLE) ? "Float" : "Rational";
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



//---------
// Number
//---------

gPool NumberPortion::pool(sizeof(NumberPortion));

NumberPortion::NumberPortion(const gNumber &value)
  : _Value(new gNumber(value)), _ref(false)
{ }

NumberPortion::NumberPortion(gNumber &value, bool ref)
  : _Value(&value), _ref(ref)
{ }

NumberPortion::~NumberPortion()
{
  if (!_ref)
    delete _Value;
}

const gNumber &NumberPortion::Value(void) const
{ return *_Value; }

void NumberPortion::SetValue(const gNumber &n)
{ *_Value = n; }

PortionSpec NumberPortion::Spec(void) const
{ return PortionSpec(porNUMBER); }

void NumberPortion::Output(gOutput& s) const
{
  Portion::Output(s);
  s << *_Value; 
}

gText NumberPortion::OutputString(void) const
{
  return ToText(*_Value);
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


//--------
// Text
//--------

gPool TextPortion::pool(sizeof(TextPortion));

TextPortion::TextPortion(const gText &value)
  : _Value(new gText(value)), _ref(false)
{ }

TextPortion::TextPortion(gText &value, bool ref)
  : _Value(&value), _ref(ref)
{ }

TextPortion::~TextPortion()
{
  if (!_ref)
    delete _Value;
}

const gText &TextPortion::Value(void) const
{ return *_Value; }

void TextPortion::SetValue(const gText &v)
{ *_Value = v; }

PortionSpec TextPortion::Spec(void) const
{ return PortionSpec(porTEXT); }

void TextPortion::Output(gOutput& s) const
{ 
  Portion::Output(s);
  s << OutputString();
}

gText TextPortion::OutputString(void) const
{
  gText text = *_Value;
  if(_WriteQuoted) 
    text = (gText) '\"' + text + "\"";
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


//--------
// Bool
//--------

gPool BoolPortion::pool(sizeof(BoolPortion));

BoolPortion::BoolPortion(bool value)
  : _Value(new gTriState((value) ? triTRUE : triFALSE)), _ref(false)
{ }

BoolPortion::BoolPortion(gTriState value)
  : _Value(new gTriState(value)), _ref(false)
{ }

BoolPortion::BoolPortion(gTriState &value, bool ref)
  : _Value(&value), _ref(ref)
{ }

BoolPortion::~BoolPortion()
{
  if (!_ref)  
    delete _Value;
}

gTriState BoolPortion::Value(void) const
{ return *_Value; }

void BoolPortion::SetValue(gTriState v)
{ *_Value = v; }

PortionSpec BoolPortion::Spec(void) const
{ return PortionSpec(porBOOLEAN); }

void BoolPortion::Output(gOutput& s) const
{
  Portion::Output(s);
  s << OutputString();
}

gText BoolPortion::OutputString(void) const
{
  if (*_Value == triTRUE)
    return "True";
  else if (*_Value == triFALSE)
    return "False";
  else /* (*_Value == triMAYBE) */
    return "Unknown";
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


//-------------
// EFOutcome
//-------------

gPool EfOutcomePortion::pool(sizeof(EfOutcomePortion));

EfOutcomePortion::EfOutcomePortion(gbtEfgOutcome p_value)
  : m_value(new gbtEfgOutcome(p_value)), m_ref(false)
{
  SetGame((efgGame *) p_value.GetGame());
}

EfOutcomePortion::EfOutcomePortion(gbtEfgOutcome *&p_value, bool p_ref)
  : m_value(p_value), m_ref(p_ref)
{
  if (!p_ref) {
    SetGame((efgGame *) p_value->GetGame());
  }
}

EfOutcomePortion::~EfOutcomePortion()
{
  if (!m_ref)  {
    delete m_value;
  }
}

gbtEfgOutcome EfOutcomePortion::Value(void) const
{ return *m_value; }

void EfOutcomePortion::SetValue(gbtEfgOutcome p_value)
{
  if (m_ref) {
    ((EfOutcomePortion *) Original())->SetValue(p_value);
  }
  else {
    SetGame((efgGame *) p_value.GetGame());
    *m_value = p_value;
  }
}

PortionSpec EfOutcomePortion::Spec(void) const
{
  return porEFOUTCOME;
}

void EfOutcomePortion::Output(gOutput& s) const
{
  Portion::Output(s);
  
  s << "(EFOutcome) ";
  s << " \"" << (*m_value).GetLabel() << "\"\n";
}

gText EfOutcomePortion::OutputString(void) const
{
  return "(EFOutcome)";
}

Portion* EfOutcomePortion::ValCopy(void) const
{ 
  return new EfOutcomePortion(*m_value);
}

Portion* EfOutcomePortion::RefCopy(void) const
{ 
  Portion* p = new EfOutcomePortion((gbtEfgOutcome *) m_value, true); 
  p->SetOriginal(Original());
  return p;
}

bool EfOutcomePortion::IsReference(void) const
{ return m_ref; }


//-----------
// NfPlayer
//-----------

gPool NfPlayerPortion::pool(sizeof(NfPlayerPortion));

NfPlayerPortion::NfPlayerPortion(gbtNfgPlayer p_value)
  : m_value(new gbtNfgPlayer(p_value)), m_ref(false)
{
  SetGame(p_value.GetGame());
}

NfPlayerPortion::NfPlayerPortion(gbtNfgPlayer *&p_value, bool p_ref)
  : m_value(p_value), m_ref(p_ref)
{
  if (!p_ref) {
    SetGame(p_value->GetGame());
  }
}

NfPlayerPortion::~NfPlayerPortion()
{
  if (!m_ref) {
    delete m_value;
  }
}

gbtNfgPlayer NfPlayerPortion::Value(void) const
{ return *m_value; }

void NfPlayerPortion::SetValue(gbtNfgPlayer p_value)
{
  if (m_ref) {
    ((NfPlayerPortion *) Original())->SetValue(p_value);
  }
  else {
    SetGame(p_value.GetGame());
    *m_value = p_value;
  }
}

PortionSpec NfPlayerPortion::Spec(void) const
{ return porNFPLAYER; }

void NfPlayerPortion::Output(gOutput& s) const
{
  Portion::Output(s);
  s << "(NfPlayer) ";
  s << " \"" << (*m_value).GetLabel() << "\""; 
}

gText NfPlayerPortion::OutputString(void) const
{
  return "(NfPlayer)";
}

Portion* NfPlayerPortion::ValCopy(void) const
{
  return new NfPlayerPortion(*m_value); 
}

Portion* NfPlayerPortion::RefCopy(void) const
{
  Portion *p = new NfPlayerPortion((gbtNfgPlayer *) m_value, true); 
  p->SetOriginal(Original());
  return p;
}

bool NfPlayerPortion::IsReference(void) const
{ return m_ref; }


//------------
// Strategy
//------------

gPool StrategyPortion::pool(sizeof(StrategyPortion));

StrategyPortion::StrategyPortion(Strategy *value)
  : _Value(new Strategy *(value)), _ref(false)
{
  SetGame(value->GetPlayer().GetGame());
}

StrategyPortion::StrategyPortion(Strategy *&value, bool ref)
  : _Value(&value), _ref(ref)
{
  if (!_ref) {
    SetGame(value->GetPlayer().GetGame());
  }
}

StrategyPortion::~StrategyPortion()
{
  if (!_ref)   delete _Value;
}

Strategy *StrategyPortion::Value(void) const
{ return *_Value; }

void StrategyPortion::SetValue(Strategy *value)
{
  if (_ref) {
    ((StrategyPortion *) Original())->SetValue(value);
  }
  else {
    SetGame(value->GetPlayer().GetGame());
    *_Value = value;
  }
}

PortionSpec StrategyPortion::Spec(void) const
{ return PortionSpec(porSTRATEGY); }

void StrategyPortion::Output(gOutput& s) const
{ 
  Portion::Output(s);
  s << "(Strategy) " << *_Value;
  if (*_Value) {
    s << " \"" << (*_Value)->GetLabel() << "\""; 
  }
}

gText StrategyPortion::OutputString(void) const
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



//-------------
// NfOutcome
//-------------

gPool NfOutcomePortion::pool(sizeof(NfOutcomePortion));

NfOutcomePortion::NfOutcomePortion(gbtNfgOutcome p_value)
  : m_value(new gbtNfgOutcome(p_value)), m_ref(false)
{
  SetGame(p_value.GetGame());
}

NfOutcomePortion::NfOutcomePortion(gbtNfgOutcome *&p_value, bool p_ref)
  : m_value(p_value), m_ref(p_ref)
{
  if (!p_ref) {
    SetGame(p_value->GetGame());
  }
}

NfOutcomePortion::~NfOutcomePortion()
{
  if (!m_ref) {
    delete m_value;
  }
}

gbtNfgOutcome NfOutcomePortion::Value(void) const
{ return *m_value; }

void NfOutcomePortion::SetValue(gbtNfgOutcome p_value)
{
  if (m_ref) {
    ((NfOutcomePortion *) Original())->SetValue(p_value);
  }
  else {
    SetGame(p_value.GetGame());
    *m_value = p_value;
  }
}

PortionSpec NfOutcomePortion::Spec(void) const
{ 
  return porNFOUTCOME;
}

void NfOutcomePortion::Output(gOutput& s) const
{
  Portion::Output(s);
  
  s << "(NFOutcome) " << *m_value;
  s << " \"" << (*m_value).GetLabel() << "\"\n";
}

gText NfOutcomePortion::OutputString(void) const
{
  return "(NFOutcome)";
}

Portion* NfOutcomePortion::ValCopy(void) const
{ 
  return new NfOutcomePortion(*m_value);
}

Portion* NfOutcomePortion::RefCopy(void) const
{ 
  Portion *p = new NfOutcomePortion((gbtNfgOutcome *) m_value, true); 
  p->SetOriginal(Original());
  return p;
}

bool NfOutcomePortion::IsReference(void) const
{ return m_ref; }


//-------------
// NfSupport
//-------------

gPool NfSupportPortion::pool(sizeof(NfSupportPortion));

NfSupportPortion::rep::~rep()
{ delete value; }

NfSupportPortion::NfSupportPortion(gbtNfgSupport *value)
  : m_rep(new struct rep(value)), m_ref(false)
{
  SetGame(&m_rep->value->Game());
}

NfSupportPortion::NfSupportPortion(gbtNfgSupport &value)
  : m_rep(new struct rep(new gbtNfgSupport(value))), m_ref(false)
{
  SetGame(&m_rep->value->Game());
}

NfSupportPortion::NfSupportPortion(const NfSupportPortion *p, bool ref)
  : m_rep(p->m_rep), m_ref(ref)
{
  m_rep->nref++;
  if (!m_ref) {
    SetGame(&m_rep->value->Game());
  }
}

NfSupportPortion::~NfSupportPortion()
{
  if (--m_rep->nref == 0)  delete m_rep;
}

gbtNfgSupport *NfSupportPortion::Value(void) const
{ return m_rep->value; }

void NfSupportPortion::SetValue(gbtNfgSupport *value)
{
  if (m_ref) {
    ((NfSupportPortion *) Original())->SetValue(value);
    m_rep = ((NfSupportPortion *) Original())->m_rep;
    m_rep->nref++;
  }
  else {
    if (--m_rep->nref == 0)  delete m_rep;
    m_rep = new rep(value);
    SetGame(&value->Game());
  }
}

PortionSpec NfSupportPortion::Spec(void) const
{
  return porNFSUPPORT;
}

void NfSupportPortion::Output(gOutput& s) const
{ 
  Portion::Output(s);
  s << "(NfSupport) " << m_rep->value;
  if (m_rep->value) 
    s << ' ' << *m_rep->value;  
}

gText NfSupportPortion::OutputString(void) const
{
  return "(NfSupport)";
}

Portion* NfSupportPortion::ValCopy(void) const
{
  return new NfSupportPortion(this, false);
}

Portion* NfSupportPortion::RefCopy(void) const
{
  Portion* p = new NfSupportPortion(this, true);
  p->SetOriginal(Original());
  return p;
}

bool NfSupportPortion::IsReference(void) const
{ return m_ref; }


//-------------
// EfSupport
//-------------

gPool EfSupportPortion::pool(sizeof(EfSupportPortion));

EfSupportPortion::rep::~rep()
{ delete value; }

EfSupportPortion::EfSupportPortion(EFSupport *value)
  : m_rep(new struct rep(value)), m_ref(false)
{
  SetGame((efgGame *) &m_rep->value->GetGame());
}

EfSupportPortion::EfSupportPortion(EFSupport &value)
  : m_rep(new struct rep(new EFSupport(value))), m_ref(false)
{
  SetGame((efgGame *) &m_rep->value->GetGame());
}

EfSupportPortion::EfSupportPortion(const EfSupportPortion *p, bool ref)
  : m_rep(p->m_rep), m_ref(ref)
{
  m_rep->nref++;
  if (!m_ref) {
    SetGame((efgGame *) &m_rep->value->GetGame());
  }
}

EfSupportPortion::~EfSupportPortion()
{
  if (--m_rep->nref == 0)  delete m_rep;
}

EFSupport *EfSupportPortion::Value(void) const
{ return m_rep->value; }

void EfSupportPortion::SetValue(EFSupport *value)
{
  if (m_ref) {
    ((EfSupportPortion *) Original())->SetValue(value);
    m_rep = ((EfSupportPortion *) Original())->m_rep;
    m_rep->nref++;
  }
  else {
    if (--m_rep->nref == 0)  delete m_rep;
    m_rep = new rep(value);
    SetGame((efgGame *) &value->GetGame());
  }
}

PortionSpec EfSupportPortion::Spec(void) const
{
  return PortionSpec(porEFSUPPORT);
}

void EfSupportPortion::Output(gOutput& s) const
{ 
  Portion::Output(s);
  s << "(EfSupport) " << m_rep->value;
  if (m_rep->value) 
    s << ' ' << *m_rep->value;
}

gText EfSupportPortion::OutputString(void) const
{
  return "(EfSupport)";
}

Portion* EfSupportPortion::ValCopy(void) const
{
  return new EfSupportPortion(this, false);
}

Portion* EfSupportPortion::RefCopy(void) const
{
  Portion* p = new EfSupportPortion(this, true); 
  p->SetOriginal(Original());
  return p;
}

bool EfSupportPortion::IsReference(void) const
{ return m_ref; }


//----------
// EfBasis
//----------

#include "game/efbasis.h"

gPool EfBasisPortion::pool(sizeof(EfBasisPortion));

EfBasisPortion::EfBasisPortion(EFBasis *value)
  : _Value(new EFBasis *(value)), _ref(false)
{
  SetGame((efgGame *) &value->GetGame());
}

EfBasisPortion::EfBasisPortion(EFBasis *&value, bool ref)
  : _Value(&value), _ref(ref)
{
  if (!_ref) {
    SetGame((efgGame *) &value->GetGame());
  }
}

EfBasisPortion::~EfBasisPortion()
{
  if (!_ref)   {
    delete *_Value;
    delete _Value;
  }
}

EFBasis *EfBasisPortion::Value(void) const
{ return *_Value; }

void EfBasisPortion::SetValue(EFBasis *value)
{
  if (_ref) {
    ((EfBasisPortion *) Original())->SetValue(value);
  }
  else {
    SetGame((efgGame *) &value->GetGame());
    delete *_Value;
    *_Value = value;
  }
}

PortionSpec EfBasisPortion::Spec(void) const
{
  return PortionSpec(porEFBASIS);
}

void EfBasisPortion::Output(gOutput& s) const
{ 
  Portion::Output(s);
  s << "(EfBasis) " << *_Value;
  if(*_Value) 
    s << ' ' << **_Value;
}

gText EfBasisPortion::OutputString(void) const
{
  return "(EfBasis)";
}

Portion* EfBasisPortion::ValCopy(void) const
{
  return new EfBasisPortion(new EFBasis(**_Value)); 
}

Portion* EfBasisPortion::RefCopy(void) const
{
  Portion* p = new EfBasisPortion(*_Value, true); 
  p->SetOriginal(Original());
  return p;
}

bool EfBasisPortion::IsReference(void) const
{ return _ref; }


//------------
// EfPlayer
//------------

gPool EfPlayerPortion::pool(sizeof(EfPlayerPortion));

EfPlayerPortion::EfPlayerPortion(EFPlayer *value)
  : _Value(new EFPlayer *(value)), _ref(false)
{
  SetGame(value->Game());
}

EfPlayerPortion::EfPlayerPortion(EFPlayer *& value, bool ref)
  : _Value(&value), _ref(ref)
{
  if (!_ref)
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
  if (_ref) {
    ((EfPlayerPortion *) Original())->SetValue(value);
  }
  else {
    *_Value = value;
    SetGame(value->Game());
  }
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

gText EfPlayerPortion::OutputString(void) const
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

//----------
// Infoset
//----------

gPool InfosetPortion::pool(sizeof(InfosetPortion));

InfosetPortion::InfosetPortion(Infoset *value)
  : _Value(new Infoset *(value)), _ref(false)
{
  SetGame(value->Game());
}

InfosetPortion::InfosetPortion(Infoset *& value, bool ref)
  : _Value(&value), _ref(ref)
{
  if (!_ref) {
    SetGame(value->Game());
  }
}

InfosetPortion::~InfosetPortion()
{
  if (!_ref)   delete _Value;
}

Infoset *InfosetPortion::Value(void) const
{ return *_Value; }

void InfosetPortion::SetValue(Infoset *value)
{
  if (_ref) {
    ((InfosetPortion *) Original())->SetValue(value);
  }
  else {
    SetGame(value->Game());
    *_Value = value;
  }
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

gText InfosetPortion::OutputString(void) const
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

//--------
// Node
//--------

gPool NodePortion::pool(sizeof(NodePortion));

NodePortion::NodePortion(Node *value)
  : _Value(new Node *(value)), _ref(false)
{
  SetGame(value->Game());
}

NodePortion::NodePortion(Node *&value, bool ref)
  : _Value(&value), _ref(ref)
{
  if (!_ref) {
    SetGame(value->Game());
  }
}  

NodePortion::~NodePortion()
{
  if (!_ref)   delete _Value;
}

Node *NodePortion::Value(void) const
{ return *_Value; }

void NodePortion::SetValue(Node *value)
{
  if (_ref) {
    ((NodePortion *) Original())->SetValue(value);
  }
  else {
    SetGame(value->Game());
    *_Value = value;
  }
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

gText NodePortion::OutputString(void) const
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

//----------
// Action
//----------

gPool ActionPortion::pool(sizeof(ActionPortion));

ActionPortion::ActionPortion(Action *value)
  : _Value(new Action *(value)), _ref(false)
{
  SetGame(value->BelongsTo()->Game());
}

ActionPortion::ActionPortion(Action *& value, bool ref)
  : _Value(&value), _ref(ref)
{
  if (!_ref) {
    SetGame(value->BelongsTo()->Game());
  }
}

ActionPortion::~ActionPortion()
{
  if (!_ref)   delete _Value;
}

Action *ActionPortion::Value(void) const
{ return *_Value; }

void ActionPortion::SetValue(Action *value)
{
  if (_ref) {
    ((ActionPortion *) Original())->SetValue(value);
  }
  else {
    SetGame(value->BelongsTo()->Game());
    *_Value = value;
  }
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

gText ActionPortion::OutputString(void) const
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

//---------
// Mixed
//---------

gPool MixedPortion::pool(sizeof(MixedPortion));

MixedPortion::MixedPortion(MixedSolution *value)
  : rep(new struct mixedrep(value)), _ref(false)
{
  SetGame(&rep->value->Game());
}

MixedPortion::MixedPortion(const MixedPortion *p, bool ref)
  : rep(p->rep), _ref(ref)
{
  rep->nref++;
  if (!_ref) {
    SetGame(&rep->value->Game());
  }
}

MixedPortion::~MixedPortion()
{
  if (--rep->nref == 0)   delete rep;
}

MixedSolution *MixedPortion::Value(void) const
{ return rep->value; }

void MixedPortion::SetValue(MixedSolution *value)
{
  if (_ref)   {
    ((MixedPortion *) Original())->SetValue(value);
    rep = ((MixedPortion *) Original())->rep;
    rep->nref++;
  }
  else  {
    if (--rep->nref == 0)  delete rep;
    rep = new mixedrep(value);
    SetGame(&value->Game());
  }
}

PortionSpec MixedPortion::Spec(void) const
{ 
  return PortionSpec(porMIXED);
}

void MixedPortion::Output(gOutput& s) const
{
  Portion::Output(s);
  s << "(Mixed) ";

  for (int pl = 1; pl <= rep->value->Game().NumPlayers(); pl++)  {
    s << "{ ";
    gbtNfgPlayer player = rep->value->Game().GetPlayer(pl);
    for (int st = 1; st <= player.NumStrategies(); st++) {
      Strategy *strategy = player.GetStrategy(st);
      if (_WriteSolutionLabels == triTRUE) {
	if ((*rep->value)(strategy) > gNumber(0)) {
	  s << strategy->GetLabel() << '=';
	  s << (*rep->value)(strategy) << ' ';
	}
      }
      else
	s << (*rep->value)(strategy) << ' ';
      }
    s << "}";
  }

  if (_WriteSolutionInfo == triTRUE)
    rep->value->DumpInfo(s);
}


gText MixedPortion::OutputString(void) const
{
  return "(Mixed)";
}

Portion* MixedPortion::ValCopy(void) const
{ 
  return new MixedPortion(this, false);
}

Portion* MixedPortion::RefCopy(void) const
{ 
  Portion* p = new MixedPortion(this, true);
  p->SetOriginal(this);
  return p;
}

bool MixedPortion::IsReference(void) const
{
  return _ref;
}


//---------
// Behav
//---------

gPool BehavPortion::pool(sizeof(BehavPortion));

BehavPortion::BehavPortion(BehavSolution *value)
  : rep(new struct behavrep(value)), _ref(false)
{
  SetGame((efgGame *) &rep->value->GetGame());
}

BehavPortion::BehavPortion(const BehavPortion *p, bool ref)
  : rep(p->rep), _ref(ref)
{
  rep->nref++;
  if (!_ref) {
    SetGame((efgGame *) &rep->value->GetGame());
  }
}

BehavPortion::~BehavPortion()
{
  if (--rep->nref == 0)   delete rep;
}

BehavSolution *BehavPortion::Value(void) const
{ return rep->value; }

void BehavPortion::SetValue(BehavSolution *value)
{
  if (_ref)   {
    ((BehavPortion *) Original())->SetValue(value);
    rep = ((BehavPortion *) Original())->rep;
    rep->nref++;
  }
  else  {
    if (--rep->nref == 0)  delete rep;
    rep = new behavrep(value);
    SetGame((efgGame *) &value->GetGame());
  }
}

PortionSpec BehavPortion::Spec(void) const
{ 
  return PortionSpec(porBEHAV);
}

void BehavPortion::Output(gOutput& s) const
{
  Portion::Output(s);
  s << "(Behav) ";

  for (int pl = 1; pl <= rep->value->GetGame().NumPlayers(); pl++)  {
    s << "{ ";
    EFPlayer *player = rep->value->GetGame().Players()[pl];
    for (int iset = 1; iset <= player->NumInfosets(); iset++)  {
      s << "{ ";
      Infoset *infoset = player->Infosets()[iset];
      for (int act = 1; act <= infoset->NumActions(); act++) {
	if (_WriteSolutionLabels == triTRUE) {
	  if ((*rep->value)(infoset->Actions()[act]) > gNumber(0)) {
	    s << infoset->Actions()[act]->GetName() << '=';
	    s << (*rep->value)(infoset->Actions()[act]) << ' ';
	  }
	}
	else
	  s << (*rep->value)(infoset->Actions()[act]) << ' ';
      }
      s << "}";
    }
    s << " }";
  }

  if (_WriteSolutionInfo == triTRUE)
    rep->value->DumpInfo(s);
}

gText BehavPortion::OutputString(void) const
{
  return "(Behav)";
}

Portion* BehavPortion::ValCopy(void) const
{ 
  return new BehavPortion(this, false);
}

Portion* BehavPortion::RefCopy(void) const
{ 
  Portion* p = new BehavPortion(this, true); 
  p->SetOriginal(this);
  return p;
}

bool BehavPortion::IsReference(void) const
{ return _ref; }


//-------
// Nfg
//-------

gPool NfgPortion::pool(sizeof(NfgPortion));

NfgPortion::NfgPortion(Nfg *value)
  : _Value(new Nfg *(value)), _ref(false)
{
  SetGame(*_Value);
}

NfgPortion::NfgPortion(Nfg *&value, bool ref)
  : _Value(&value), _ref(ref)
{ 
  // for games, only call SetGame for ValPortions, not RefPortions!
  if( !_ref )
    SetGame(*_Value);
}

NfgPortion::~NfgPortion()
{ 
  // for games, only call SetGame for ValPortions, not RefPortions!
  if (!_ref) {
    delete _Value; 
  }
}

Nfg *NfgPortion::Value(void) const
{ return *_Value; }

void NfgPortion::SetValue(Nfg *value)
{
  // for games, only call SetGame for ValPortions, not RefPortions!
  ((NfgPortion*) Original())->SetGame( value );
  *_Value = value;
}

PortionSpec NfgPortion::Spec(void) const
{ 
  return PortionSpec(porNFG);
}

void NfgPortion::Output(gOutput& s) const
{
  Portion::Output(s);
  s << "(Nfg) \"" << (*_Value)->GetTitle() << "\"";
}

gText NfgPortion::OutputString(void) const
{
  return "(Nfg)";
}

Portion* NfgPortion::ValCopy(void) const
{ 
  return new NfgPortion(*_Value);
}

Portion* NfgPortion::RefCopy(void) const
{ 
  Portion* p = new NfgPortion(*_Value, true);
  p->SetOriginal(Original());
  return p;
}

bool NfgPortion::IsReference(void) const
{ return _ref; }



//-------
// Efg
//-------

gPool EfgPortion::pool(sizeof(EfgPortion));

EfgPortion::EfgPortion(efgGame *value)
  : _Value(new efgGame *(value)), _ref(false)
{
  SetGame(*_Value);
}

EfgPortion::EfgPortion(efgGame *&value, bool ref)
  : _Value(&value), _ref(ref)
{ 
  // for games, only call SetGame for ValPortions, not RefPortions!
  if( !_ref )
    SetGame(*_Value);
}

EfgPortion::~EfgPortion()
{ 
  if (!_ref) {
    delete _Value; 
  }
}

efgGame *EfgPortion::Value(void) const
{ return *_Value; }

void EfgPortion::SetValue(efgGame *value)
{
  // for games, only call SetGame for ValPortions, not RefPortions!
  ((EfgPortion*) Original())->SetGame( value );
  *_Value = value;
}

PortionSpec EfgPortion::Spec(void) const
{
  return PortionSpec(porEFG);
}

void EfgPortion::Output(gOutput& s) const
{
  Portion::Output(s);
  s << "(Efg) \"" << (*_Value)->GetTitle() << "\""; 
}

gText EfgPortion::OutputString(void) const
{
  return "(Efg)";
}

Portion* EfgPortion::ValCopy(void) const
{ 
  return new EfgPortion(*_Value);
}

Portion* EfgPortion::RefCopy(void) const
{ 
  Portion* p = new EfgPortion(*_Value, true); 
  p->SetOriginal(Original());
  return p;
}

bool EfgPortion::IsReference(void) const
{
  return _ref;
}


//----------
// Output
//----------

gPool OutputPortion::pool(sizeof(OutputPortion));

OutputPortion::OutputPortion(gOutput& value)
  : rep(new struct outputrep(&value)), _ref(false)
{ }

OutputPortion::OutputPortion(const OutputPortion *p, bool ref)
  : rep(p->rep), _ref(ref)
{
  rep->nref++;
}

OutputPortion::~OutputPortion()
{
  if (--rep->nref == 0)   delete rep;
}

gOutput& OutputPortion::Value(void) const
{ return *rep->value; }

PortionSpec OutputPortion::Spec(void) const
{ return PortionSpec(porOUTPUT); }

void OutputPortion::Output(gOutput& s) const
{
  Portion::Output(s);
  s << "(Output)"; 
}

gText OutputPortion::OutputString(void) const
{
  return "(Output)";
}

Portion* OutputPortion::ValCopy(void) const
{ 
  return new OutputPortion(this, false);
}

Portion* OutputPortion::RefCopy(void) const
{ 
  Portion* p = new OutputPortion(this, true); 
  p->SetOriginal(Original());
  return p;
}

bool OutputPortion::IsReference(void) const
{ return _ref; }



//--------
// Input
//--------

gPool InputPortion::pool(sizeof(InputPortion));

InputPortion::InputPortion(gInput& value)
  : rep(new struct inputrep(&value)), _ref(false)
{ }

InputPortion::InputPortion(const InputPortion *p, bool ref)
  : rep(p->rep), _ref(ref)
{
  rep->nref++;
}

InputPortion::~InputPortion()
{
  if (--rep->nref == 0)   delete rep;
}


gInput& InputPortion::Value(void) const
{ return *rep->value; }

PortionSpec InputPortion::Spec(void) const
{ return PortionSpec(porINPUT); }

void InputPortion::Output(gOutput& s) const
{
  Portion::Output(s);
  s << "(Input)"; 
}

gText InputPortion::OutputString(void) const
{
  return "(Input)";
}

Portion* InputPortion::ValCopy(void) const
{ 
  return new InputPortion(this, false);
}

Portion* InputPortion::RefCopy(void) const
{ 
  Portion* p = new InputPortion(this, true); 
  p->SetOriginal(Original());
  return p;
}

bool InputPortion::IsReference(void) const
{ return _ref; }


//--------
// List
//--------

gPool ListPortion::pool(sizeof(ListPortion));

ListPortion::listrep::listrep(void)
  : value(new gList<Portion *>), _ContainsListsOnly(true),
    _DataType(porUNDEFINED), _IsNull(false), _ListDepth(1), nref(1)
{ }

ListPortion::listrep::~listrep()
{ 
  for (int i = 1; i <= value->Length(); delete (*value)[i++]);
  delete value;
}

ListPortion::ListPortion(void)
  : rep(new listrep), _ref(false)
{ }

ListPortion::ListPortion(const gList<Portion *> &value)
  : rep(new listrep), _ref(false)
{ 
  for (int i = 1, length = value.Length(); i <= length; i++) 
    Insert(value[i]->ValCopy(), i);
}

ListPortion::ListPortion(const ListPortion *p, bool ref)
  : rep(p->rep), _ref(ref)
{
  rep->nref++;
}


ListPortion::~ListPortion()
{
  if (--rep->nref == 0)  delete rep;
}


bool ListPortion::BelongsToGame( void* game ) const
{
  for (int i = 1; i <= rep->value->Length(); i++)
    if ((*rep->value)[i]->Spec().ListDepth == 0)  {
      if ((*rep->value)[i]->Game() == game)
	return true;
    }
    else  {
      if (((ListPortion*) (*rep->value)[i])->BelongsToGame(game))
	return true;
    }
  return false;
}



bool ListPortion::MatchGameData( void* game, void* data ) const
{
  for (int i = 1; i <= rep->value->Length(); i++)  {
    PortionSpec spec = (*rep->value)[i]->Spec();
    if ((*rep->value)[i]->Spec().ListDepth == 0)  {
      if (spec.Type & porEFSUPPORT)  {
	if (((EfSupportPortion*) (*rep->value)[i])->Value() == data)
	  return true;
      }
      if (spec.Type & porEFBASIS)  {
	if (((EfBasisPortion*) (*rep->value)[i])->Value() == data)
	  return true;
      }
      if (spec.Type & porEFPLAYER)  {
	if (((EfPlayerPortion*) (*rep->value)[i])->Value() == data)
	  return true;
      }
      if (spec.Type & porINFOSET)  {
	if (((InfosetPortion*) (*rep->value)[i])->Value() == data)
	  return true;
      }
      if (spec.Type & porNODE)  {
	if (((NodePortion*) (*rep->value)[i])->Value() == data)
	  return true;
      }
      if (spec.Type & porACTION)  {
	if (((ActionPortion*) (*rep->value)[i])->Value() == data)
	  return true;
      }
    }
    else  {
      if (((ListPortion*) (*rep->value)[i])->MatchGameData(game, data))
	return true;
    }
  }
  return false;
}

const gList<Portion *> &ListPortion::Value(void) const
{ return *rep->value; }

bool ListPortion::IsInteger(void) const
{
  if (rep->_DataType != porNUMBER)  return false;

  bool result = true;

  for (int i = 1; result && i <= rep->value->Length(); i++) { 
    Portion *p = (*rep->value)[i];
    if (!p->Spec().Null) {  
      if (p->Spec().ListDepth > 0)
	result = result && ((ListPortion *) p)->IsInteger();
      else
	result = result && ((NumberPortion *) p)->Value().IsInteger();
    }
  }
  return result;
}

PortionSpec ListPortion::Spec(void) const
{ 
  if (IsReference())
    return Original()->Spec();
  else
    return PortionSpec(rep->_DataType, rep->_ListDepth, rep->_IsNull); 
}

Portion* ListPortion::ValCopy(void) const
{ 
  ListPortion* p = new ListPortion(*rep->value);
  if(p->rep->_DataType == porUNDEFINED)
    p->rep->_DataType = rep->_DataType;
  return p;
}

Portion* ListPortion::RefCopy(void) const
{ 
  ListPortion* p = new ListPortion(this, true); 
  p->rep->_DataType = rep->_DataType;
  p->SetOriginal(Original());
  return p;
}


void ListPortion::AssignFrom(Portion* p)
{
  int i;
  int length;
  int result;
  gList <Portion *>& value = *(((ListPortion*) p)->rep->value);

  Flush();

  for(i = 1, length = value.Length(); i <= length; i++) {
    result = Insert(value[i]->ValCopy(), i);
    if (!result)
      throw gclRuntimeError("Internal error in ListPortion");
  }
  if (rep->_DataType == porUNDEFINED)
    rep->_DataType = ((ListPortion*) p)->rep->_DataType;

}

bool ListPortion::operator == (Portion* p) const
{
  bool result = true;
  int i;
  int length = rep->value->Length();
  Portion* p1;
  Portion* p2;
  bool type_found;

  if(p->Spec() == Spec())
  {
    if(rep->value->Length() == ((ListPortion*) p)->rep->value->Length())
    {
      for(i = 1; i <= length; i++)
      {
	p1 = (*rep->value)[i];
	p2 = (*(((ListPortion*) p)->rep->value))[i];
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


bool ListPortion::ContainsListsOnly(void) const
{
  if (rep->value->Length() == 0)
    return false;
  else
    return rep->_ContainsListsOnly;
}

void ListPortion::SetDataType(unsigned long type)
{ rep->_DataType = type; }

void ListPortion::Output(gOutput& s) const
{ Output(s, 0); }

void ListPortion::Output(gOutput& s, long ListLF) const
{
  Portion::Output(s);
  int i;
  int c;
  int length = rep->value->Length();
  
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
	if(_WriteListLF > gNumber(ListLF))
	  s << '\n';
	if(_WriteListLF > gNumber(ListLF))
	  for(c = 0; c < (double) ((gNumber) (ListLF+1) * _WriteListIndent); c++)
	    s << ' ';
      }
      else
	if(_WriteListLF > gNumber(ListLF))
	  s << ' ';
      if(_WriteListLF <= gNumber(ListLF))
	s << ' ';
      if((*rep->value)[i]->Spec().ListDepth == 0)
	s << (*rep->value)[i];
      else
	((ListPortion*) (*rep->value)[i])->Output(s, ListLF + 1);
    }
  }
  else
  {
    if(_WriteListLF > gNumber(ListLF))
      for(c = 0; c < (double) ((gNumber) (ListLF+1) * _WriteListIndent) - 1; c++)
	s << ' ';
    s << " (" << PortionSpecToText(rep->_DataType) << ')';
  }

  s << ' ';
  if(_WriteListBraces) 
    s << '}';
  else
    s << ' ';
}


gText ListPortion::OutputString(void) const
{
  gText text( "{ " );
  for (int i = 1; i <= Length(); i++)  {
    if( i > 1 )
      text += ", ";
    text += operator[](i)->OutputString();
  }
  text += " }";
  return text;
}


int ListPortion::Append(Portion* item)
{ return Insert(item, rep->value->Length() + 1); }


int ListPortion::Insert(Portion* item, int index)
{
  int result = 0;
  PortionSpec item_type = item->Spec();

  if (item->Spec().ListDepth == 0) {
    if (item_type.Type == porNULL)
      item_type = ((NullPortion*) item)->DataType();
    rep->_ContainsListsOnly = false;
  }

  if (rep->_DataType == porUNDEFINED) { // inserting into an empty list
    rep->_DataType = item_type.Type;
    ((ListPortion*) Original())->rep->_DataType = rep->_DataType;
    result = rep->value->Insert(item, index);
  }
  else  {  // inserting into an existing list
    if (PortionSpecMatch(item_type.Type, rep->_DataType)) {
      result = rep->value->Insert(item, index);
    }
    else if (item_type.Type == porUNDEFINED) { // inserting an empty list
      result = rep->value->Insert(item, index);
      if (item->Spec().ListDepth <= 0)
	throw gclRuntimeError("Internal error in ListPortion");
      ((ListPortion*) item)->rep->_DataType = rep->_DataType;
    }
    else {
      delete item;
      throw gclRuntimeError("Type mismatch in list; list type = " +
			    PortionSpecToText(rep->_DataType) +
			    ", item type = " + 
			    PortionSpecToText(item_type.Type));
    }
  }

  if (result > 0)  {
    if (item->Spec().ListDepth + 1 > rep->_ListDepth)
      rep->_ListDepth = item->Spec().ListDepth + 1;
  }

  return result;
}


bool ListPortion::Contains(Portion* p2) const
{
  int i;
  int length = rep->value->Length();
  bool type_found;
  Portion* p1;

  for(i = 1; i <= length; i++)
  {
    p1 = (*rep->value)[i];
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
  if(index >= 1 && index <= rep->value->Length())
    result = rep->value->Remove(index);

  rep->_ContainsListsOnly = true;
  rep->_ListDepth = 1;
  if(rep->value->Length() > 0)
  {
    for (int i = 1; i <= rep->value->Length(); ++i )
    {
      if( (*rep->value)[i]->Spec().ListDepth == 0 )
	rep->_ContainsListsOnly = false;
      if( (*rep->value)[i]->Spec().ListDepth >= rep->_ListDepth )
	rep->_ListDepth = (*rep->value)[i]->Spec().ListDepth + 1;
    }
  }

  return result;
}

int ListPortion::Length(void) const
{ return rep->value->Length(); }


void ListPortion::Flush(void)
{
  int i, length;
  for (i = 1, length = rep->value->Length(); i <= length; i++) {
    delete Remove(1);
  }
}


Portion* ListPortion::operator[](int index) const
{
  if (index >= 1 && index <= rep->value->Length()) {
    if (!(*rep->value)[index])
      throw gclRuntimeError("Internal error in ListPortion");
    return (*rep->value)[index];
  }
  else
    return 0;
}



Portion* ListPortion::SubscriptCopy(int index) const
{
  Portion* p;
  if (index >= 1 && index <= rep->value->Length()) {
    if (!(*rep->value)[index])  
      throw gclRuntimeError("Internal error in ListPortion");

    if(IsReference())
      p = (*rep->value)[index]->RefCopy();
    else
      p = (*rep->value)[index]->ValCopy();
      
    return p;
  }
  else
    return 0;
}

bool ListPortion::IsReference(void) const
{
  return _ref;
}


//---------------------------
// Miscellaneous functions
//---------------------------

gOutput& operator<<(gOutput& s, Portion* p)
{
  p->Output(s);
  return s;
}

bool PortionEqual(Portion* p1, Portion* p2, bool &type_found)
{
  bool b = false;

  if(!(p1->Spec() == p2->Spec())) 
    return false;

  if( p1->Spec().ListDepth > 0 )
    return ((ListPortion*) p1)->operator==( (ListPortion*) p2 );

  type_found = true;

  if(p1->Spec().Type & porBOOLEAN)   
    b = (((BoolPortion*) p1)->Value() == ((BoolPortion*) p2)->Value());
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
  else if(p1->Spec().Type & porEFBASIS)
    b = (*(((EfBasisPortion*) p1)->Value()) ==
	 *(((EfBasisPortion*) p2)->Value()));
  
  else if(p1->Spec().Type & porMIXED)
    b = (*((MixedPortion*) p1)->Value() == *((MixedPortion*) p2)->Value());
  else if(p1->Spec().Type & porBEHAV)
    b = (*((BehavPortion*) p1)->Value() == *((BehavPortion*) p2)->Value());

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

  else  {
    type_found = false;
    throw gclRuntimeError("Internal error in PortionEqual()"); 
  }
  return b;
}



