//
// FILE: portion.h -- header file for Portion class
//                    companion to GSM
//
// $Id$
//




#ifndef PORTION_H
#define PORTION_H


#include "gsmincl.h"



#include "glist.h"
#include "gstring.h"
#include "rational.h"



//---------------------------------------------------------------------
//                          base class
//---------------------------------------------------------------------

class Portion
{
friend class ListPortion; 
private:
  static int _NumObj;

  Portion* _Original;
  void* _Game;
  bool _GameIsEfg;


protected:
  Portion(void);

  static long _WriteWidth;
  static long _WritePrecis;
  static bool _WriteExpmode;
  static bool _WriteQuoted;
  static bool _WriteListBraces;
  static bool _WriteListCommas;
  static long _WriteListLF;
  static long _WriteListIndent;
  static long _WriteSolutionInfo;

  void SetGame(void* game, bool efg);

public:
  static void _SetWriteWidth(long);
  static void _SetWritePrecis(long);
  static void _SetWriteExpmode(bool);
  static void _SetWriteQuoted(bool);
  static void _SetWriteListBraces(bool);
  static void _SetWriteListCommas(bool);
  static void _SetWriteListLF(long);
  static void _SetWriteListIndent(long);
  static void _SetWriteSolutionInfo(long);


  virtual ~Portion();

  void SetOriginal(const Portion* p);
  Portion* Original(void) const;

  virtual PortionSpec Spec(void) const = 0;
  virtual DataType SubType( void ) const { return DT_ERROR; }

  virtual void Output(gOutput& s) const;
  virtual gString OutputString( void ) const = 0;

  virtual Portion* ValCopy(void) const = 0;
  virtual Portion* RefCopy(void) const = 0;
  virtual bool IsReference(void) const = 0;

  void* Game(void) const;
  bool GameIsEfg(void) const;
};



//---------------------------------------------------------------------
//                          Error class
//---------------------------------------------------------------------

class ErrorPortion : public Portion
{
protected:
  gString _Value;

public:
  ErrorPortion(const gString& value = "");
  virtual ~ErrorPortion();

  gString Value(void) const;
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;
  bool IsReference(void) const;
};



//---------------------------------------------------------------------
//                          Null class
//---------------------------------------------------------------------

class NullPortion : public Portion
{
protected:
  unsigned long _DataType;

public:
  NullPortion(const unsigned long datatype);
  virtual ~NullPortion();

  PortionSpec Spec(void) const;
  unsigned long DataType(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;
  bool IsReference(void) const;
};



//---------------------------------------------------------------------
//                          Reference class
//---------------------------------------------------------------------

class ReferencePortion : public Portion
{
protected:
  gString _Value;

public:
  ReferencePortion(const gString& value);
  virtual ~ReferencePortion();

  gString Value(void);
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;
  bool IsReference(void) const;
};





//---------------------------------------------------------------------
//                          int class
//---------------------------------------------------------------------

class IntPortion : public Portion
{
protected:
  long* _Value;
  bool _ref;

public:
  IntPortion(long);
  IntPortion(long &, bool);
  virtual ~IntPortion();

  long& Value(void) const;
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;
};


//---------------------------------------------------------------------
//                          float class
//---------------------------------------------------------------------

class FloatPortion : public Portion
{
protected:
  double* _Value;
  bool _ref;

  FloatPortion(double &, bool);

public:
  FloatPortion(double);
  virtual ~FloatPortion();

  double& Value(void) const;
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;
};


//---------------------------------------------------------------------
//                          Rational class
//---------------------------------------------------------------------

class RationalPortion : public Portion
{
protected:
  gRational* _Value;
  bool _ref;

  RationalPortion(gRational &, bool);

public:
  RationalPortion(const gRational &);
  virtual ~RationalPortion();

  gRational& Value(void) const;
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;
};


//---------------------------------------------------------------------
//                          Text class
//---------------------------------------------------------------------

class TextPortion : public Portion
{
protected:
  gString* _Value;
  bool _ref;

public:
  TextPortion(const gString &);
  TextPortion(gString &, bool);
  virtual ~TextPortion();

  gString& Value(void) const;
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;
};


//---------------------------------------------------------------------
//                          Bool class
//---------------------------------------------------------------------

class BoolPortion : public Portion
{
protected:
  bool* _Value;
  bool _ref;


public:
  BoolPortion(bool);
  BoolPortion(bool &, bool);
  virtual ~BoolPortion();

  bool& Value(void) const;
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;
};


//---------------------------------------------------------------------
//                          EFOutcome class
//---------------------------------------------------------------------

class EFOutcome;

class EfOutcomePortion : public Portion
{
protected:
  EFOutcome** _Value;
  bool _ref;

  EfOutcomePortion(EFOutcome *&, bool);

public:
  EfOutcomePortion(EFOutcome *);
  virtual ~EfOutcomePortion();

  EFOutcome *Value(void) const;
  void SetValue(EFOutcome *);
  PortionSpec Spec(void) const;
  DataType SubType(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;
};


//---------------------------------------------------------------------
//                          NfPlayer class
//---------------------------------------------------------------------

class NFPlayer;

class NfPlayerPortion : public Portion
{
protected:
  NFPlayer** _Value;
  bool _ref;

  NfPlayerPortion(NFPlayer *&, bool);

public:
  NfPlayerPortion(NFPlayer *); 
  virtual ~NfPlayerPortion();

  NFPlayer *Value(void) const;
  void SetValue(NFPlayer *); 
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;
};

//---------------------------------------------------------------------
//                          Strategy class
//---------------------------------------------------------------------

class Strategy;

class StrategyPortion : public Portion
{
protected:
  Strategy** _Value;
  bool _ref;

  StrategyPortion(Strategy *&, bool);

public:
  StrategyPortion(Strategy *);
  virtual ~StrategyPortion();

  Strategy *Value(void) const;
  void SetValue(Strategy *); 
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;
};


//---------------------------------------------------------------------
//                          NfOutcome class
//---------------------------------------------------------------------

class NFOutcome;

class NfOutcomePortion : public Portion
{
protected:
  NFOutcome** _Value;
  bool _ref;

  NfOutcomePortion(NFOutcome *&, bool);

public:
  NfOutcomePortion(NFOutcome *); 
  virtual ~NfOutcomePortion();

  NFOutcome *Value(void) const;
  void SetValue(NFOutcome *); 
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;
};



//---------------------------------------------------------------------
//                          NfSupport class
//---------------------------------------------------------------------

class NFSupport;
class NFPayoffs;

class NfSupportPortion : public Portion   {
protected:
  NFSupport** _Value;
  bool _ref;
  NFPayoffs *paytable;

  NfSupportPortion(NFSupport *&, NFPayoffs *, bool);

public:
  NfSupportPortion(NFSupport *, NFPayoffs *);
  virtual ~NfSupportPortion();

  NFSupport *Value(void) const;
  void SetValue(NFSupport *);
  PortionSpec Spec(void) const;
  DataType SubType(void) const;

  NFPayoffs *PayoffTable(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;
};


//---------------------------------------------------------------------
//                          EfSupport class
//---------------------------------------------------------------------


class EFSupport;

class EfSupportPortion : public Portion
{
protected:
  EFSupport** _Value;
  bool _ref;

  EfSupportPortion(EFSupport *&, bool);

public:
  EfSupportPortion(EFSupport *);
  virtual ~EfSupportPortion();

  EFSupport *Value(void) const;
  void SetValue(EFSupport *);
  PortionSpec Spec(void) const;
  DataType SubType( void ) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;
};



//---------------------------------------------------------------------
//                          EfPlayer class
//---------------------------------------------------------------------

class EFPlayer;

class EfPlayerPortion : public Portion
{
protected:
  EFPlayer** _Value;
  bool _ref;

  EfPlayerPortion(EFPlayer *&, bool);

public:
  EfPlayerPortion(EFPlayer *);
  virtual ~EfPlayerPortion();

  EFPlayer *Value(void) const;
  void SetValue(EFPlayer *);
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;
};


//---------------------------------------------------------------------
//                          Infoset class
//---------------------------------------------------------------------

class Infoset;

class InfosetPortion : public Portion
{
protected:
  Infoset** _Value;
  bool _ref;

  InfosetPortion(Infoset *&, bool);

public:
  InfosetPortion(Infoset *);
  virtual ~InfosetPortion();

  Infoset *Value(void) const;
  void SetValue(Infoset *);
  PortionSpec Spec(void) const;
  DataType SubType( void ) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;
};



//---------------------------------------------------------------------
//                          Node class
//---------------------------------------------------------------------

class Node;

class NodePortion : public Portion
{
protected:
  Node** _Value;
  bool _ref;  

  NodePortion(Node *&, bool);

public:
  NodePortion(Node *);
  virtual ~NodePortion();

  Node *Value(void) const;
  void SetValue(Node *);
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;
};



//---------------------------------------------------------------------
//                          Action class
//---------------------------------------------------------------------

class Action;

class ActionPortion : public Portion
{
protected:
  Action** _Value;
  bool _ref;

  ActionPortion(Action *&, bool); 

public:
  ActionPortion(Action *);
  virtual ~ActionPortion();

  Action *Value(void) const;
  void SetValue(Action *);
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;

  bool IsReference(void) const;
};

//---------------------------------------------------------------------
//                          new Mixed class
//---------------------------------------------------------------------

template <class T> class MixedPortion : public Portion  {
protected:
  MixedProfile<T> ** _Value;
  MixedPortion(void);

public:
  virtual ~MixedPortion();

  MixedProfile<T> *Value(void) const;
  void SetValue(MixedProfile<T> *);
  PortionSpec Spec(void) const;
  DataType SubType(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;
};


template <class T> class MixedValPortion : public MixedPortion<T>  {
public:
  MixedValPortion(MixedProfile<T> *value);
  virtual ~MixedValPortion();
  bool IsReference(void) const;
};

template <class T> class MixedRefPortion : public MixedPortion<T>  {
public:
  MixedRefPortion(MixedProfile<T> *& value);
  virtual ~MixedRefPortion();
  bool IsReference(void) const;
};





//---------------------------------------------------------------------
//                          new Behav class
//---------------------------------------------------------------------

template <class T> class BehavPortion : public Portion  {
protected:
  BehavProfile<T> ** _Value;
  BehavPortion(void);

public:
  virtual ~BehavPortion();

  BehavProfile<T> *Value(void) const;
  void SetValue(BehavProfile<T> *);
  PortionSpec Spec(void) const;
  DataType SubType(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;
};


template <class T> class BehavValPortion : public BehavPortion<T>  {
public:
  BehavValPortion(BehavProfile<T> *value);
  virtual ~BehavValPortion();
  bool IsReference(void) const;
};

template <class T> class BehavRefPortion : public BehavPortion<T>  {
public:
  BehavRefPortion(BehavProfile<T> *& value);
  virtual ~BehavRefPortion();
  bool IsReference(void) const;
};



//---------------------------------------------------------------------
//                          new Nfg class
//---------------------------------------------------------------------

class BaseNfg;

template <class T> class NfgPortion : public Portion   {
protected:
  Nfg<T> ** _Value;
  NfgPortion(void);

public:
  virtual ~NfgPortion();

  Nfg<T> *Value(void) const;
  void SetValue(Nfg<T> *);
  PortionSpec Spec(void) const;
  DataType SubType(void) const;

  void Output(gOutput& s) const;
  gString OutputString(void) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;
};


template <class T> class NfgValPortion : public NfgPortion<T>  {
public:
  NfgValPortion(Nfg<T> *value);
  virtual ~NfgValPortion();
  bool IsReference(void) const;
};

template <class T> class NfgRefPortion : public NfgPortion<T>  {
public:
  NfgRefPortion(Nfg<T> *&value);
  virtual ~NfgRefPortion();
  bool IsReference(void) const;
};




//---------------------------------------------------------------------
//                          new Efg class
//---------------------------------------------------------------------

class BaseEfg;

class EfgPortion : public Portion
{
protected:
  BaseEfg** _Value;
  EfgPortion(void);

public:
  virtual ~EfgPortion();

  BaseEfg *Value(void) const;
  void SetValue(BaseEfg *);
  PortionSpec Spec(void) const;
  DataType SubType( void ) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;
};


class EfgValPortion : public EfgPortion
{
public:
  EfgValPortion(BaseEfg* value);
  virtual ~EfgValPortion();
  bool IsReference(void) const;
};

class EfgRefPortion : public EfgPortion
{
public:
  EfgRefPortion(BaseEfg*& value);
  virtual ~EfgRefPortion();
  bool IsReference(void) const;
};





//---------------------------------------------------------------------
//                          Output class
//---------------------------------------------------------------------

class OutputPortion : public Portion
{
protected:
  gOutput* _Value;
  OutputPortion(void);

public:
  virtual ~OutputPortion();

  gOutput& Value(void) const;
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;
};

class OutputValPortion : public OutputPortion
{
public:
  OutputValPortion(gOutput& value);
  virtual ~OutputValPortion();
  bool IsReference(void) const;
};

class OutputRefPortion : public OutputPortion
{
public:
  OutputRefPortion(gOutput& value);
  virtual ~OutputRefPortion();
  bool IsReference(void) const;
};




//---------------------------------------------------------------------
//                          Input class
//---------------------------------------------------------------------

class InputPortion : public Portion
{
protected:
  gInput* _Value;
  InputPortion(void);

public:
  virtual ~InputPortion();

  gInput& Value(void) const;
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;
};

class InputValPortion : public InputPortion
{
public:
  InputValPortion(gInput& value);
  virtual ~InputValPortion();
  bool IsReference(void) const;
};

class InputRefPortion : public InputPortion
{
public:
  InputRefPortion(gInput& value);
  virtual ~InputRefPortion();
  bool IsReference(void) const;
};



//---------------------------------------------------------------------
//                          List class
//---------------------------------------------------------------------

// template <class T> class gBlock;
template <class T> class gList;

class ListPortion : public Portion
{
protected:
  gList< Portion* >* _Value;
  ListPortion(void);

  bool _ContainsListsOnly;
  unsigned long _DataType;

  bool _IsNull;
  int _ListDepth;

  // bool _IsNull(void) const;
  // int _ListDepth(void) const;

public:
  virtual ~ListPortion();

  bool BelongsToGame( void* game ) const;  
  bool MatchGameData( void* game, void* data ) const;  

  bool ContainsListsOnly(void) const;


  // gBlock< Portion* >& Value(void) const;
  gList< Portion* >& Value(void) const;
  void SetDataType(unsigned long type);
  PortionSpec Spec(void) const;
  DataType SubType( void ) const;

  void Output(gOutput& s) const;
  void Output(gOutput& s, long ListLF) const;
  gString OutputString( void ) const;

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
  //          don't calling ValCopy() or RefCopy() on Subscript() !
  Portion* SubscriptCopy(int index) const;
};

class ListValPortion : public ListPortion
{
public:
  ListValPortion(void);
  // ListValPortion(gBlock< Portion* >& value);
  ListValPortion(gList< Portion* >& value);
  virtual ~ListValPortion();
  bool IsReference(void) const;
};

class ListRefPortion : public ListPortion
{
public:
  ListRefPortion(gList< Portion* >& value);
  virtual ~ListRefPortion();
  bool IsReference(void) const;
};





//-----------------------------------------------------------------
//                 Miscellaneous Portion functions
//-----------------------------------------------------------------


gOutput& operator << (gOutput& s, Portion* p);

bool PortionEqual(Portion* p1, Portion* p2, bool& type_found);


#endif // PORTION_H



