//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of GCL types
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

#ifndef PORTION_H
#define PORTION_H

#include "base/base.h"
#include "math/gnumber.h"
#include "game/efg.h"
#include "game/nfg.h"
#include "gsmincl.h"

//-------------
// Portion
//-------------

class Nfg;

class Portion
{
private:
  static int _NumObj;

  Portion* _Original;
  void* _Game;
  bool _GameIsEfg;


protected:
  Portion(void);

  static gNumber _WriteWidth;
  static gNumber _WritePrecis;
  static gTriState _WriteExpmode;
  static gTriState _WriteQuoted;
  static gTriState _WriteListBraces;
  static gTriState _WriteListCommas;
  static gNumber _WriteListLF;
  static gNumber _WriteListIndent;
  static gTriState _WriteSolutionInfo;
  static gTriState _WriteSolutionLabels;

  void SetGame(const efgGame *game);
  void SetGame(const Nfg *game);

public:
  static void _SetWriteWidth(long);
  static void _SetWritePrecis(long);
  static void _SetWriteExpmode(bool);
  static void _SetWriteQuoted(bool);
  static void _SetWriteListBraces(bool);
  static void _SetWriteListCommas(bool);
  static void _SetWriteListLF(long);
  static void _SetWriteListIndent(long);
  static void _SetWriteSolutionInfo(bool);
  static void _SetWriteSolutionLabels(bool);

  virtual ~Portion();

  void SetOriginal(const Portion* p);
  Portion* Original(void) const;

  virtual PortionSpec Spec(void) const = 0;

  virtual void Output(gOutput& s) const;
  virtual gText OutputString(void) const = 0;

  virtual Portion* ValCopy(void) const = 0;
  virtual Portion* RefCopy(void) const = 0;
  virtual bool IsReference(void) const = 0;

  void* Game(void) const;
  bool GameIsEfg(void) const;
};

//---------
// Null
//---------

class NullPortion : public Portion  {
protected:
  unsigned long _DataType;

  static gPool pool;

public:
  NullPortion(const unsigned long datatype);
  virtual ~NullPortion();

  PortionSpec Spec(void) const;
  unsigned long DataType(void) const;

  void Output(gOutput& s) const;
  gText OutputString(void) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;
  bool IsReference(void) const;

  void *operator new(size_t) { return pool.Alloc(); }
  void operator delete(void *p) { pool.Free(p); } 
};



//------------
// Reference
//------------

class ReferencePortion : public Portion  {
protected:
  gText _Value;

  static gPool pool;

public:
  ReferencePortion(const gText& value);
  virtual ~ReferencePortion();

  gText Value(void);
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gText OutputString(void) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;
  bool IsReference(void) const;

  void *operator new(size_t) { return pool.Alloc(); }
  void operator delete(void *p) { pool.Free(p); }
};


//-------------
// Precision
//-------------

class PrecisionPortion : public Portion  {
protected:
  gPrecision* _Value;
  bool _ref;

  static gPool pool;

  PrecisionPortion(gPrecision &, bool);

public:
  PrecisionPortion(gPrecision);
  virtual ~PrecisionPortion();

  gPrecision Value(void) const;
  void SetValue(gPrecision); 

  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gText OutputString(void) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;

  void *operator new(size_t) { return pool.Alloc(); }
  void operator delete(void *p) { pool.Free(p); }
};

inline gPrecision AsPrecision(Portion *portion) 
{ return (dynamic_cast<PrecisionPortion *>(portion))->Value(); }

//----------
// Number
//----------

class NumberPortion : public Portion  {
protected:
  gNumber* _Value;
  bool _ref;

  static gPool pool;

  NumberPortion(gNumber &, bool);

public:
  NumberPortion(const gNumber &);
  virtual ~NumberPortion();

  const gNumber &Value(void) const;
  void SetValue(const gNumber &);
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gText OutputString(void) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;

  void *operator new(size_t) { return pool.Alloc(); }
  void operator delete(void *p) { pool.Free(p); }
};

inline const gNumber &AsNumber(Portion *portion)
{ return (dynamic_cast<NumberPortion *>(portion))->Value(); }

//--------
// Text
//--------

class TextPortion : public Portion  {
protected:
  gText* _Value;
  bool _ref;

  static gPool pool;

  TextPortion(gText &, bool);

public:
  TextPortion(const gText &);
  virtual ~TextPortion();

  const gText &Value(void) const;
  void SetValue(const gText &);
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gText OutputString(void) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;

  void *operator new(size_t) { return pool.Alloc(); }
  void operator delete(void *p) { pool.Free(p); }
};

inline const gText &AsText(Portion *portion)
{ return (dynamic_cast<TextPortion *>(portion))->Value(); }

//--------
// Bool
//--------

class BoolPortion : public Portion  {
protected:
  gTriState *_Value;
  bool _ref;

  static gPool pool;

  BoolPortion(gTriState &, bool);

public:
  BoolPortion(bool);
  BoolPortion(gTriState);
  virtual ~BoolPortion();

  gTriState Value(void) const;
  void SetValue(gTriState);
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gText OutputString(void) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;

  void *operator new(size_t) { return pool.Alloc(); }
  void operator delete(void *p) { pool.Free(p); } 
};

inline bool AsBool(Portion *portion)
{ return (dynamic_cast<BoolPortion *>(portion))->Value(); }


//-------------
// EFOutcome
//-------------

class EfOutcomePortion : public Portion  {
protected:
  gbtEfgOutcome *m_value;
  bool m_ref;

  static gPool pool;

  EfOutcomePortion(gbtEfgOutcome *&, bool);

public:
  EfOutcomePortion(gbtEfgOutcome);
  virtual ~EfOutcomePortion();

  gbtEfgOutcome Value(void) const;
  void SetValue(gbtEfgOutcome);
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gText OutputString(void) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;

  void *operator new(size_t) { return pool.Alloc(); }
  void operator delete(void *p) { pool.Free(p); }
};

inline gbtEfgOutcome AsEfgOutcome(Portion *portion)
{ return (dynamic_cast<EfOutcomePortion *>(portion))->Value(); }

//------------
// NfPlayer
//------------

class NfPlayerPortion : public Portion  {
protected:
  gbtNfgPlayer *m_value;
  bool m_ref;

  static gPool pool;

  NfPlayerPortion(gbtNfgPlayer *&, bool);

public:
  NfPlayerPortion(gbtNfgPlayer); 
  virtual ~NfPlayerPortion();

  gbtNfgPlayer Value(void) const;
  void SetValue(gbtNfgPlayer); 
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gText OutputString(void) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;

  void *operator new(size_t) { return pool.Alloc(); }
  void operator delete(void *p) { pool.Free(p); }
};

inline gbtNfgPlayer AsNfgPlayer(Portion *portion)
{ return (dynamic_cast<NfPlayerPortion *>(portion))->Value(); }

//-----------
// Strategy
//-----------

class StrategyPortion : public Portion  {
protected:
  gbtNfgStrategy *m_value;
  bool m_ref;

  static gPool pool;

  StrategyPortion(gbtNfgStrategy *&, bool);

public:
  StrategyPortion(gbtNfgStrategy);
  virtual ~StrategyPortion();

  gbtNfgStrategy Value(void) const;
  void SetValue(gbtNfgStrategy); 
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gText OutputString(void) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;

  void *operator new(size_t) { return pool.Alloc(); }
  void operator delete(void *p) { pool.Free(p); }
};

inline gbtNfgStrategy AsNfgStrategy(Portion *portion)
{ return (dynamic_cast<StrategyPortion *>(portion))->Value(); }

//------------
// NfOutcome
//------------

class NfOutcomePortion : public Portion  {
protected:
  gbtNfgOutcome *m_value;
  bool m_ref;

  static gPool pool;

  NfOutcomePortion(gbtNfgOutcome *&, bool);

public:
  NfOutcomePortion(gbtNfgOutcome);
  virtual ~NfOutcomePortion();

  gbtNfgOutcome Value(void) const;
  void SetValue(gbtNfgOutcome);
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gText OutputString(void) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;

  void *operator new(size_t) { return pool.Alloc(); }
  void operator delete(void *p) { pool.Free(p); }
};

inline gbtNfgOutcome AsNfgOutcome(Portion *portion)
{ return (dynamic_cast<NfOutcomePortion *>(portion))->Value(); }


//-------------
// NfSupport
//-------------

class gbtNfgSupport;

class NfSupportPortion : public Portion   {
protected:
  struct rep  {
    gbtNfgSupport *value;
    int nref;

    rep(gbtNfgSupport *v) : value(v), nref(1)  { }
    ~rep();
  };
  
  struct rep *m_rep; 
  bool m_ref;

  static gPool pool;

  NfSupportPortion(const NfSupportPortion *, bool);

public:
  NfSupportPortion(gbtNfgSupport *);
  NfSupportPortion(gbtNfgSupport &);
  virtual ~NfSupportPortion();

  gbtNfgSupport *Value(void) const;
  void SetValue(gbtNfgSupport *);
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gText OutputString(void) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;

  void *operator new(size_t) { return pool.Alloc(); }
  void operator delete(void *p) { pool.Free(p); }
};

inline gbtNfgSupport &AsNfgSupport(Portion *portion)
{ return *(dynamic_cast<NfSupportPortion *>(portion))->Value(); }

//-------------
// EfSupport
//-------------

class EFSupport;

class EfSupportPortion : public Portion  {
protected:
  struct rep {
    EFSupport *value;
    int nref;

    rep(EFSupport *v) : value(v), nref(1)  { }
    ~rep();
  };

  struct rep *m_rep;
  bool m_ref;

  static gPool pool;

  EfSupportPortion(const EfSupportPortion *, bool);

public:
  EfSupportPortion(EFSupport *);
  EfSupportPortion(EFSupport &);
  virtual ~EfSupportPortion();

  EFSupport *Value(void) const;
  void SetValue(EFSupport *);
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gText OutputString(void) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;

  void *operator new(size_t) { return pool.Alloc(); }
  void operator delete(void *p) { pool.Free(p); }
};

inline EFSupport &AsEfgSupport(Portion *portion)
{ return *(dynamic_cast<EfSupportPortion *>(portion))->Value(); }


//-----------
// EfBasis
//-----------

class EFBasis;

class EfBasisPortion : public Portion  {
protected:
  EFBasis** _Value;
  bool _ref;

  static gPool pool;

  EfBasisPortion(EFBasis *&, bool);

public:
  EfBasisPortion(EFBasis *);
  virtual ~EfBasisPortion();

  EFBasis *Value(void) const;
  void SetValue(EFBasis *);
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gText OutputString(void) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;

  void *operator new(size_t) { return pool.Alloc(); }
  void operator delete(void *p) { pool.Free(p); }
};

inline EFBasis &AsEfgBasis(Portion *portion)
{ return *(dynamic_cast<EfBasisPortion *>(portion))->Value(); }

//------------
// EfPlayer
//------------

class EfPlayerPortion : public Portion  {
protected:
  gbtEfgPlayer *m_value;
  bool m_ref;

  static gPool pool;

  EfPlayerPortion(gbtEfgPlayer *&, bool);

public:
  EfPlayerPortion(gbtEfgPlayer); 
  virtual ~EfPlayerPortion();

  gbtEfgPlayer Value(void) const;
  void SetValue(gbtEfgPlayer); 
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gText OutputString(void) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;

  void *operator new(size_t) { return pool.Alloc(); }
  void operator delete(void *p) { pool.Free(p); }
};

inline gbtEfgPlayer AsEfgPlayer(Portion *portion) 
{ return (dynamic_cast<EfPlayerPortion *>(portion))->Value(); }

//----------
// Infoset
//----------

class InfosetPortion : public Portion  {
protected:
  gbtEfgInfoset *m_value;
  bool m_ref;

  static gPool pool;

  InfosetPortion(gbtEfgInfoset *&, bool);

public:
  InfosetPortion(gbtEfgInfoset); 
  virtual ~InfosetPortion();

  gbtEfgInfoset Value(void) const;
  void SetValue(gbtEfgInfoset); 
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gText OutputString(void) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;

  void *operator new(size_t) { return pool.Alloc(); }
  void operator delete(void *p) { pool.Free(p); }
};

inline gbtEfgInfoset AsEfgInfoset(Portion *portion) 
{ return (dynamic_cast<InfosetPortion *>(portion))->Value(); }

//--------
// Node
//--------

class Node;

class NodePortion : public Portion  {
protected:
  Node** _Value;
  bool _ref;  

  static gPool pool;

  NodePortion(Node *&, bool);

public:
  NodePortion(Node *);
  virtual ~NodePortion();

  Node *Value(void) const;
  void SetValue(Node *);
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gText OutputString(void) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;

  void *operator new(size_t) { return pool.Alloc(); }
  void operator delete(void *p) { pool.Free(p); }
};

inline Node *AsEfgNode(Portion *portion) 
{ return (dynamic_cast<NodePortion *>(portion))->Value(); }

//------------
// Action
//------------

class ActionPortion : public Portion  {
protected:
  gbtEfgAction *m_value;
  bool m_ref;

  static gPool pool;

  ActionPortion(gbtEfgAction *&, bool);

public:
  ActionPortion(gbtEfgAction);
  virtual ~ActionPortion();

  gbtEfgAction Value(void) const;
  void SetValue(gbtEfgAction);
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gText OutputString(void) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;

  void *operator new(size_t) { return pool.Alloc(); }
  void operator delete(void *p) { pool.Free(p); }
};

inline gbtEfgAction AsEfgAction(Portion *portion) 
{ return (dynamic_cast<ActionPortion *>(portion))->Value(); }


//---------
// Mixed
//---------

#include "nash/mixedsol.h"

class MixedPortion : public Portion  {
protected:
  struct mixedrep  {
    MixedSolution *value;
    int nref;

    mixedrep(MixedSolution *v) : value(v), nref(1)  { }
    ~mixedrep()  { delete value; }
  };
  
  struct mixedrep *rep; 
  bool _ref;

  static gPool pool;

  MixedPortion(const MixedPortion *, bool);

public:
  MixedPortion(MixedSolution *);
  virtual ~MixedPortion();

  MixedSolution *Value(void) const;
  void SetValue(MixedSolution *);
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gText OutputString(void) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;

  void *operator new(size_t) { return pool.Alloc(); }
  void operator delete(void *p) { pool.Free(p); }
};

inline const MixedSolution &AsMixed(Portion *portion)
{ return *(dynamic_cast<MixedPortion *>(portion))->Value(); }


//--------
// Behav
//--------

#include "nash/behavsol.h"

class BehavPortion : public Portion  {
protected:
  struct behavrep  {
    BehavSolution *value;
    int nref;

    behavrep(BehavSolution *v) : value(v), nref(1)  { }
    ~behavrep()  { delete value; }
  };
  
  struct behavrep *rep; 
  bool _ref;

  static gPool pool;

  BehavPortion(const BehavPortion *, bool);

public:
  BehavPortion(BehavSolution *);
  virtual ~BehavPortion();

  BehavSolution *Value(void) const;
  void SetValue(BehavSolution *);
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gText OutputString(void) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;

  void *operator new(size_t) { return pool.Alloc(); }
  void operator delete(void *p) { pool.Free(p); }
};

inline const BehavSolution &AsBehav(Portion *portion)
{ return *(dynamic_cast<BehavPortion *>(portion))->Value(); }


//-------
// Nfg
//-------

class NfgPortion : public Portion   {
protected:
  Nfg ** _Value;
  bool _ref;

  static gPool pool;

  NfgPortion(Nfg *&, bool);

public:
  NfgPortion(Nfg *value);
  virtual ~NfgPortion();

  Nfg *Value(void) const;
  void SetValue(Nfg *);
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gText OutputString(void) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;

  void *operator new(size_t) { return pool.Alloc(); }
  void operator delete(void *p) { pool.Free(p); }
};

inline Nfg &AsNfg(Portion *portion)
{ return *(dynamic_cast<NfgPortion *>(portion))->Value(); }

//-------
// Efg
//-------


class EfgPortion : public Portion   {
protected:
  efgGame ** _Value;
  bool _ref;

  static gPool pool;

  EfgPortion(efgGame *&, bool);

public:
  EfgPortion(efgGame *value);
  virtual ~EfgPortion();

  efgGame *Value(void) const;
  void SetValue(efgGame *);
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gText OutputString(void) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;

  void *operator new(size_t) { return pool.Alloc(); }
  void operator delete(void *p) { pool.Free(p); }
};

inline efgGame &AsEfg(Portion *portion)
{ return *(dynamic_cast<EfgPortion *>(portion))->Value(); }


//---------
// Output
//---------

class OutputPortion : public Portion  {
protected:
  struct outputrep  {
    gOutput *value;
    int nref;

    outputrep(gOutput *v) : value(v), nref(1)  { }
    //  ~outputrep()  { if (value != &gout && value != &gnull)  delete value; }
    ~outputrep() { }
  };
  
  struct outputrep *rep; 
  bool _ref;

  static gPool pool;

  OutputPortion(const OutputPortion *, bool);

public:
  OutputPortion(gOutput &value);

  virtual ~OutputPortion();

  gOutput& Value(void) const;
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gText OutputString(void) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;

  void *operator new(size_t) { return pool.Alloc(); }
  void operator delete(void *p) { pool.Free(p); }
};




//---------
// Input
//---------

class InputPortion : public Portion  {
protected:
  struct inputrep  {
    gInput *value;
    int nref;

    inputrep(gInput *v) : value(v), nref(1)  { }
    ~inputrep()  { delete value; }
  };
  
  struct inputrep *rep; 
  bool _ref;

  static gPool pool;

  InputPortion(const InputPortion *, bool);

public:
  InputPortion(gInput &value);
  virtual ~InputPortion();

  gInput& Value(void) const;
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gText OutputString(void) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;

  void *operator new(size_t) { return pool.Alloc(); }
  void operator delete(void *p) { pool.Free(p); }
};



//--------
// List
//--------

template <class T> class gList;

class ListPortion : public Portion  {
protected:
  struct listrep  {
    gList<Portion *> *value;
    bool _ContainsListsOnly;
    unsigned long _DataType;
    bool _IsNull;
    unsigned int _ListDepth;

    int nref;

    listrep(void);
    listrep(gList<Portion *> *value);
    ~listrep();
  };
  
  struct listrep *rep;
  bool _ref;

  static gPool pool;

  ListPortion(const ListPortion *, bool);

public:
  ListPortion(void);
  ListPortion(const gList<Portion *> &value);
  virtual ~ListPortion();

  bool BelongsToGame( void* game ) const;  
  bool MatchGameData( void* game, void* data ) const;  

  bool IsInteger(void) const;

  bool ContainsListsOnly(void) const;

  const gList<Portion *> &Value(void) const;
  void SetDataType(unsigned long type);
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  void Output(gOutput& s, long ListLF) const;
  gText OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  void AssignFrom(Portion* p);
  bool operator == (Portion* p) const;

  int      Append     (Portion* item);
  int      Insert     (Portion* item, int index);
  bool     Contains   (Portion* item) const;
  Portion* Remove     (int index);
  int      Length     (void) const;
  void     Flush      (void);

  // Use operator[] when you just want to check the info on an element;
  Portion* operator[](int index) const;

  // Use SubscriptCopy() when you want to extract a copy of an element
  // Warning: SubscriptCopy() already makes a copy; 
  //          don't call ValCopy() or RefCopy() on Subscript() !
  Portion* SubscriptCopy(int index) const;

  bool IsReference(void) const;

  void *operator new(size_t) { return pool.Alloc(); }
  void operator delete(void *p) { pool.Free(p); }
};


//-----------------------------------
// Miscellaneous Portion functions
//-----------------------------------


gOutput& operator << (gOutput& s, Portion* p);

bool PortionEqual(Portion* p1, Portion* p2, bool& type_found);


#endif   // PORTION_H



