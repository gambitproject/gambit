//#
//# FILE: portion.h -- header file for Portion class
//#                    companion to GSM
//#
//# $Id$
//#




#ifndef PORTION_H
#define PORTION_H


#include "gsmincl.h"



#include "glist.h"
#include "gstring.h"
#include "rational.h"


//#include "gambitio.h"

//#include "mixedsol.h"
//#include "behavsol.h"

//#include "nfg.h"
//#include "efg.h"
//#include "nfplayer.h"
//#include "nfstrat.h"
//#include "efstrat.h"



//---------------------------------------------------------------------
//                          base class
//---------------------------------------------------------------------

class Portion
{
private:
  static int _NumObj;

protected:
  Portion* _Original;
  void* _Game;
  bool _GameIsEfg;

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

  virtual void Output(gOutput& s) const;
  virtual gString OutputString( void ) const = 0;

  virtual Portion* ValCopy(void) const = 0;
  virtual Portion* RefCopy(void) const = 0;
  virtual bool IsReference(void) const = 0;

  //bool IsValid(void) const { return true; }

  void* Game(void) const;
  bool GameIsEfg(void) const;
  virtual void SetGame(void* game, bool efg);
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
  IntPortion(void);

public:
  virtual ~IntPortion();

  long& Value(void) const;
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;
};

class IntValPortion : public IntPortion
{
public:
  IntValPortion(long value);
  virtual ~IntValPortion();
  bool IsReference(void) const;
};

class IntRefPortion : public IntPortion
{
public:
  IntRefPortion(long& value);
  virtual ~IntRefPortion();
  bool IsReference(void) const;
};


//---------------------------------------------------------------------
//                          float class
//---------------------------------------------------------------------

class FloatPortion : public Portion
{
protected:
  double* _Value;
  FloatPortion(void);

public:
  virtual ~FloatPortion();

  double& Value(void) const;
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;
};

class FloatValPortion : public FloatPortion
{
public:
  FloatValPortion(double value);
  virtual ~FloatValPortion();
  bool IsReference(void) const;
};

class FloatRefPortion : public FloatPortion
{
public:
  FloatRefPortion(double& value);
  virtual ~FloatRefPortion();
  bool IsReference(void) const;
};



//---------------------------------------------------------------------
//                          Rational class
//---------------------------------------------------------------------

class RationalPortion : public Portion
{
protected:
  gRational* _Value;
  RationalPortion(void);

public:
  virtual ~RationalPortion();

  gRational& Value(void) const;
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;
};

class RationalValPortion : public RationalPortion
{
public:
  RationalValPortion(gRational value);
  virtual ~RationalValPortion();
  bool IsReference(void) const;
};

class RationalRefPortion : public RationalPortion
{
public:
  RationalRefPortion(gRational& value);
  virtual ~RationalRefPortion();
  bool IsReference(void) const;
};





//---------------------------------------------------------------------
//                          Text class
//---------------------------------------------------------------------

class TextPortion : public Portion
{
protected:
  gString* _Value;
  TextPortion(void);

public:
  virtual ~TextPortion();

  gString& Value(void) const;
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;
};

class TextValPortion : public TextPortion
{
public:
  TextValPortion(gString value);
  virtual ~TextValPortion();
  bool IsReference(void) const;
};

class TextRefPortion : public TextPortion
{
public:
  TextRefPortion(gString& value);
  virtual ~TextRefPortion();
  bool IsReference(void) const;
};



//---------------------------------------------------------------------
//                          Bool class
//---------------------------------------------------------------------

class BoolPortion : public Portion
{
protected:
  bool* _Value;
  BoolPortion(void);

public:
  virtual ~BoolPortion();

  bool& Value(void) const;
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;
};

class BoolValPortion : public BoolPortion
{
public:
  BoolValPortion(bool value);
  virtual ~BoolValPortion();
  bool IsReference(void) const;
};

class BoolRefPortion : public BoolPortion
{
public:
  BoolRefPortion(bool& value);
  virtual ~BoolRefPortion();
  bool IsReference(void) const;
};











//---------------------------------------------------------------------
//                          Outcome class
//---------------------------------------------------------------------

class Outcome;

class OutcomePortion : public Portion
{
protected:
  Outcome** _Value;
  OutcomePortion(void);

public:
  virtual ~OutcomePortion();

  Outcome*& Value(void) const;
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;
};

class OutcomeValPortion : public OutcomePortion
{
public:
  OutcomeValPortion(Outcome* value);
  virtual ~OutcomeValPortion();
  bool IsReference(void) const;
};

class OutcomeRefPortion : public OutcomePortion
{
public:
  OutcomeRefPortion(Outcome*& value);
  virtual ~OutcomeRefPortion();
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
  NfPlayerPortion(void);

public:
  virtual ~NfPlayerPortion();

  NFPlayer*& Value(void) const;
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;
};

class NfPlayerValPortion : public NfPlayerPortion
{
public:
  NfPlayerValPortion(NFPlayer* value);
  virtual ~NfPlayerValPortion();
  bool IsReference(void) const;
};

class NfPlayerRefPortion : public NfPlayerPortion
{
public:
  NfPlayerRefPortion(NFPlayer*& value);
  virtual ~NfPlayerRefPortion();
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
  StrategyPortion(void);

public:
  virtual ~StrategyPortion();

  Strategy*& Value(void) const;
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;
};

class StrategyValPortion : public StrategyPortion
{
public:
  StrategyValPortion(Strategy* value);
  virtual ~StrategyValPortion();
  bool IsReference(void) const;
};

class StrategyRefPortion : public StrategyPortion
{
public:
  StrategyRefPortion(Strategy*& value);
  virtual ~StrategyRefPortion();
  bool IsReference(void) const;
};





//---------------------------------------------------------------------
//                          NfSupport class
//---------------------------------------------------------------------

class NFSupport;

class NfSupportPortion : public Portion
{
protected:
  NFSupport** _Value;
  NfSupportPortion(void);

public:
  virtual ~NfSupportPortion();

  NFSupport*& Value(void) const;
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;
};

class NfSupportValPortion : public NfSupportPortion
{
public:
  NfSupportValPortion(NFSupport* value);
  virtual ~NfSupportValPortion();
  bool IsReference(void) const;
};

class NfSupportRefPortion : public NfSupportPortion
{
public:
  NfSupportRefPortion(NFSupport*& value);
  virtual ~NfSupportRefPortion();
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
  EfSupportPortion(void);

public:
  virtual ~EfSupportPortion();

  EFSupport*& Value(void) const;
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;
};

class EfSupportValPortion : public EfSupportPortion
{
public:
  EfSupportValPortion(EFSupport* value);
  virtual ~EfSupportValPortion();
  bool IsReference(void) const;
};

class EfSupportRefPortion : public EfSupportPortion
{
public:
  EfSupportRefPortion(EFSupport*& value);
  virtual ~EfSupportRefPortion();
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
  EfPlayerPortion(void);

public:
  virtual ~EfPlayerPortion();

  EFPlayer*& Value(void) const;
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;
};

class EfPlayerValPortion : public EfPlayerPortion
{
public:
  EfPlayerValPortion(EFPlayer* value);
  virtual ~EfPlayerValPortion();
  bool IsReference(void) const;
};

class EfPlayerRefPortion : public EfPlayerPortion
{
public:
  EfPlayerRefPortion(EFPlayer*& value);
  virtual ~EfPlayerRefPortion();
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
  InfosetPortion(void);

public:
  virtual ~InfosetPortion();

  Infoset*& Value(void) const;
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;
};

class InfosetValPortion : public InfosetPortion
{
public:
  InfosetValPortion(Infoset* value);
  virtual ~InfosetValPortion();
  bool IsReference(void) const;
};

class InfosetRefPortion : public InfosetPortion
{
public:
  InfosetRefPortion(Infoset*& value);
  virtual ~InfosetRefPortion();
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
  NodePortion(void);

public:
  virtual ~NodePortion();

  Node*& Value(void) const;
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;
};

class NodeValPortion : public NodePortion
{
public:
  NodeValPortion(Node* value);
  virtual ~NodeValPortion();
  bool IsReference(void) const;
};

class NodeRefPortion : public NodePortion
{
public:
  NodeRefPortion(Node*& value);
  virtual ~NodeRefPortion();
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
  ActionPortion(void);

public:
  virtual ~ActionPortion();

  Action*& Value(void) const;
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;
};

class ActionValPortion : public ActionPortion
{
public:
  ActionValPortion(Action* value);
  virtual ~ActionValPortion();
  bool IsReference(void) const;
};

class ActionRefPortion : public ActionPortion
{
public:
  ActionRefPortion(Action*& value);
  virtual ~ActionRefPortion();
  bool IsReference(void) const;
};








//---------------------------------------------------------------------
//                          new Mixed class
//---------------------------------------------------------------------

class BaseMixedProfile;

class MixedPortion : public Portion
{
protected:
  BaseMixedProfile** _Value;
  MixedPortion(void);

public:
  virtual ~MixedPortion();

  BaseMixedProfile*& Value(void) const;
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;
};


class MixedValPortion : public MixedPortion
{
public:
  MixedValPortion(BaseMixedProfile* value);
  virtual ~MixedValPortion();
  bool IsReference(void) const;
};

class MixedRefPortion : public MixedPortion
{
public:
  MixedRefPortion(BaseMixedProfile*& value);
  virtual ~MixedRefPortion();
  bool IsReference(void) const;
};





//---------------------------------------------------------------------
//                          new Behav class
//---------------------------------------------------------------------

class BaseBehavProfile;

class BehavPortion : public Portion
{
protected:
  BaseBehavProfile** _Value;
  BehavPortion(void);

public:
  virtual ~BehavPortion();

  BaseBehavProfile*& Value(void) const;
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;
};


class BehavValPortion : public BehavPortion
{
public:
  BehavValPortion(BaseBehavProfile* value);
  virtual ~BehavValPortion();
  bool IsReference(void) const;
};

class BehavRefPortion : public BehavPortion
{
public:
  BehavRefPortion(BaseBehavProfile*& value);
  virtual ~BehavRefPortion();
  bool IsReference(void) const;
};






//---------------------------------------------------------------------
//                          new Nfg class
//---------------------------------------------------------------------

class BaseNfg;

class NfgPortion : public Portion
{
protected:
  BaseNfg** _Value;
  NfgPortion(void);

public:
  virtual ~NfgPortion();

  BaseNfg*& Value(void) const;
  PortionSpec Spec(void) const;

  void Output(gOutput& s) const;
  gString OutputString( void ) const;

  Portion* ValCopy(void) const;
  Portion* RefCopy(void) const;
};


class NfgValPortion : public NfgPortion
{
public:
  NfgValPortion(BaseNfg* value);
  virtual ~NfgValPortion();
  bool IsReference(void) const;
};

class NfgRefPortion : public NfgPortion
{
public:
  NfgRefPortion(BaseNfg*& value);
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

  BaseEfg*& Value(void) const;
  PortionSpec Spec(void) const;

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
  bool _IsNull(void) const;
  int _ListDepth(void) const;

public:
  virtual ~ListPortion();

  virtual void SetGame(void* game, bool efg);
  bool ContainsListsOnly(void) const;


  // gBlock< Portion* >& Value(void) const;
  gList< Portion* >& Value(void) const;
  void SetDataType(unsigned long type);
  PortionSpec Spec(void) const;

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



