//
// FILE: portion.h -- header file for Portion class
//                    companion to GSM
//
// $Id$
//

#ifndef PORTION_H
#define PORTION_H

#include "gsmincl.h"
#include "gblock.h"
#include "glist.h"
#include "gtext.h"
#include "gnumber.h"

#include "gpool.h"


//-------------
// Portion
//-------------

class Efg;
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

  void SetGame(const Efg *game);
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


//-------------
// EFOutcome
//-------------

class EFOutcome;

class EfOutcomePortion : public Portion  {
protected:
  EFOutcome** _Value;
  bool _ref;

  static gPool pool;

  EfOutcomePortion(EFOutcome *&, bool);

public:
  EfOutcomePortion(EFOutcome *);
  virtual ~EfOutcomePortion();

  EFOutcome *Value(void) const;
  void SetValue(EFOutcome *);
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gText OutputString(void) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;

  void *operator new(size_t) { return pool.Alloc(); }
  void operator delete(void *p) { pool.Free(p); }
};


//------------
// NfPlayer
//------------

class NFPlayer;

class NfPlayerPortion : public Portion  {
protected:
  NFPlayer** _Value;
  bool _ref;

  static gPool pool;

  NfPlayerPortion(NFPlayer *&, bool);

public:
  NfPlayerPortion(NFPlayer *); 
  virtual ~NfPlayerPortion();

  NFPlayer *Value(void) const;
  void SetValue(NFPlayer *); 
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gText OutputString(void) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;

  void *operator new(size_t) { return pool.Alloc(); }
  void operator delete(void *p) { pool.Free(p); }
};

//-----------
// Strategy
//-----------

class Strategy;

class StrategyPortion : public Portion  {
protected:
  Strategy** _Value;
  bool _ref;

  static gPool pool;

  StrategyPortion(Strategy *&, bool);

public:
  StrategyPortion(Strategy *);
  virtual ~StrategyPortion();

  Strategy *Value(void) const;
  void SetValue(Strategy *); 
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gText OutputString(void) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;

  void *operator new(size_t) { return pool.Alloc(); }
  void operator delete(void *p) { pool.Free(p); }
};


//------------
// NfOutcome
//------------

class NFOutcome;

class NfOutcomePortion : public Portion  {
protected:
  NFOutcome** _Value;
  bool _ref;

  static gPool pool;

  NfOutcomePortion(NFOutcome *&, bool);

public:
  NfOutcomePortion(NFOutcome *); 
  virtual ~NfOutcomePortion();

  NFOutcome *Value(void) const;
  void SetValue(NFOutcome *); 
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
// NfSupport
//-------------

class NFSupport;
class NFPayoffs;

class NfSupportPortion : public Portion   {
protected:
  NFSupport** _Value;
  bool _ref;

  static gPool pool;

  NfSupportPortion(NFSupport *&, bool);

public:
  NfSupportPortion(NFSupport *);
  virtual ~NfSupportPortion();

  NFSupport *Value(void) const;
  void SetValue(NFSupport *);
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
// EfSupport
//-------------

class EFSupport;

class EfSupportPortion : public Portion  {
protected:
  EFSupport** _Value;
  bool _ref;

  static gPool pool;

  EfSupportPortion(EFSupport *&, bool);

public:
  EfSupportPortion(EFSupport *);
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


//------------
// EfPlayer
//------------

class EFPlayer;

class EfPlayerPortion : public Portion  {
protected:
  EFPlayer** _Value;
  bool _ref;

  static gPool pool;

  EfPlayerPortion(EFPlayer *&, bool);

public:
  EfPlayerPortion(EFPlayer *);
  virtual ~EfPlayerPortion();

  EFPlayer *Value(void) const;
  void SetValue(EFPlayer *);
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gText OutputString(void) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;

  void *operator new(size_t) { return pool.Alloc(); }
  void operator delete(void *p) { pool.Free(p); }
};


//----------
// Infoset
//----------

class Infoset;

class InfosetPortion : public Portion  {
protected:
  Infoset** _Value;
  bool _ref;

  static gPool pool;

  InfosetPortion(Infoset *&, bool);

public:
  InfosetPortion(Infoset *);
  virtual ~InfosetPortion();

  Infoset *Value(void) const;
  void SetValue(Infoset *);
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



//----------
// Action
//----------

class Action;

class ActionPortion : public Portion  {
protected:
  const Action** _Value;
  bool _ref;

  static gPool pool;

  ActionPortion(const Action *&, bool); 

public:
  ActionPortion(Action *);
  ActionPortion(const Action *);
  virtual ~ActionPortion();

  const Action *Value(void) const;
  void SetValue(const Action *);
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
// Mixed
//---------

#include "mixedsol.h"

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



//--------
// Behav
//--------

#include "behavsol.h"

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


//-------
// Efg
//-------


class EfgPortion : public Portion   {
protected:
  Efg** _Value;
  bool _ref;

  static gPool pool;

  EfgPortion(Efg *&, bool);

public:
  EfgPortion(Efg *value);
  virtual ~EfgPortion();

  Efg *Value(void) const;
  void SetValue(Efg *);
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
// Output
//---------

class OutputPortion : public Portion  {
protected:
  struct outputrep  {
    gOutput *value;
    int nref;

    outputrep(gOutput *v) : value(v), nref(1)  { }
    ~outputrep()  { if (value != &gout && value != &gnull)  delete value; }
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
    ~inputrep()  { if (value != &gin)  delete value; }
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
    int _ListDepth;

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



