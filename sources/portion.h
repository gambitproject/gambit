//#
//# FILE: portion.h -- header file for Portion class
//#                    companion to GSM
//#
//# $Id$
//#




#ifndef PORTION_H
#define PORTION_H


#include "gsmincl.h"

#include "gambitio.h"





//---------------------------------------------------------------------
//                          base class
//---------------------------------------------------------------------

class Portion
{
private:
  static int _NumObj;

protected:
  Portion* _Owner;
  Portion* _Original;
  Portion( void );

public:
  virtual ~Portion();

  virtual void SetOwner( Portion* p );
  Portion* Owner( void ) const;

  void SetOriginal( const Portion* p );
  Portion* Original( void ) const;

  virtual PortionType Type( void ) const = 0;
  virtual void Output( gOutput& s ) const = 0;
  virtual Portion* ValCopy( void ) const = 0;
  virtual Portion* RefCopy( void ) const = 0;

  virtual void AssignFrom( Portion* p ) = 0;
  virtual bool operator == ( Portion* p ) const;

  virtual bool IsReference( void ) const = 0;
};



//---------------------------------------------------------------------
//                          Error class
//---------------------------------------------------------------------

class ErrorPortion : public Portion
{
protected:
  gString _Value;

public:
  ErrorPortion( const gString& value = "" );
  virtual ~ErrorPortion();

  gString Value( void );
  PortionType Type( void ) const;
  void Output( gOutput& s ) const;
  Portion* ValCopy( void ) const;
  Portion* RefCopy( void ) const;
  void AssignFrom( Portion* p );
  bool operator == ( Portion* p ) const;
  bool IsReference( void ) const;
};



//---------------------------------------------------------------------
//                          Reference class
//---------------------------------------------------------------------

class ReferencePortion : public Portion
{
protected:
  gString _Value;

public:
  ReferencePortion( const gString& value );
  virtual ~ReferencePortion();

  gString Value( void );
  PortionType Type( void ) const;
  void Output( gOutput& s ) const;
  Portion* ValCopy( void ) const;
  Portion* RefCopy( void ) const;
  void AssignFrom( Portion* p );  
  bool operator == ( Portion* p ) const;
  bool IsReference( void ) const;
};





//---------------------------------------------------------------------
//                          int class
//---------------------------------------------------------------------

class IntPortion : public Portion
{
protected:
  long* _Value;
  IntPortion( void );

public:
  virtual ~IntPortion();

  long& Value( void ) const;
  PortionType Type( void ) const;
  void Output( gOutput& s ) const;
  Portion* ValCopy( void ) const;
  Portion* RefCopy( void ) const;
  void AssignFrom( Portion* p );
  bool operator == ( Portion* p ) const;
};

class IntValPortion : public IntPortion
{
public:
  IntValPortion( long value );
  virtual ~IntValPortion();
  bool IsReference( void ) const;
};

class IntRefPortion : public IntPortion
{
public:
  IntRefPortion( long& value );
  virtual ~IntRefPortion();
  bool IsReference( void ) const;
};


//---------------------------------------------------------------------
//                          float class
//---------------------------------------------------------------------

class FloatPortion : public Portion
{
protected:
  double* _Value;
  FloatPortion( void );

public:
  virtual ~FloatPortion();

  double& Value( void ) const;
  PortionType Type( void ) const;
  void Output( gOutput& s ) const;
  Portion* ValCopy( void ) const;
  Portion* RefCopy( void ) const;
  void AssignFrom( Portion* p );
  bool operator == ( Portion* p ) const;
};

class FloatValPortion : public FloatPortion
{
public:
  FloatValPortion( double value );
  virtual ~FloatValPortion();
  bool IsReference( void ) const;
};

class FloatRefPortion : public FloatPortion
{
public:
  FloatRefPortion( double& value );
  virtual ~FloatRefPortion();
  bool IsReference( void ) const;
};



//---------------------------------------------------------------------
//                          Rational class
//---------------------------------------------------------------------

class RationalPortion : public Portion
{
protected:
  gRational* _Value;
  RationalPortion( void );

public:
  virtual ~RationalPortion();

  gRational& Value( void ) const;
  PortionType Type( void ) const;
  void Output( gOutput& s ) const;
  Portion* ValCopy( void ) const;
  Portion* RefCopy( void ) const;
  void AssignFrom( Portion* p );
  bool operator == ( Portion* p ) const;
};

class RationalValPortion : public RationalPortion
{
public:
  RationalValPortion( gRational value );
  virtual ~RationalValPortion();
  bool IsReference( void ) const;
};

class RationalRefPortion : public RationalPortion
{
public:
  RationalRefPortion( gRational& value );
  virtual ~RationalRefPortion();
  bool IsReference( void ) const;
};





//---------------------------------------------------------------------
//                          Text class
//---------------------------------------------------------------------

class TextPortion : public Portion
{
protected:
  gString* _Value;
  TextPortion( void );

public:
  virtual ~TextPortion();

  gString& Value( void ) const;
  PortionType Type( void ) const;
  void Output( gOutput& s ) const;
  Portion* ValCopy( void ) const;
  Portion* RefCopy( void ) const;
  void AssignFrom( Portion* p );
  bool operator == ( Portion* p ) const;
};

class TextValPortion : public TextPortion
{
public:
  TextValPortion( gString value );
  virtual ~TextValPortion();
  bool IsReference( void ) const;
};

class TextRefPortion : public TextPortion
{
public:
  TextRefPortion( gString& value );
  virtual ~TextRefPortion();
  bool IsReference( void ) const;
};



//---------------------------------------------------------------------
//                          Bool class
//---------------------------------------------------------------------

class BoolPortion : public Portion
{
protected:
  bool* _Value;
  BoolPortion( void );

public:
  virtual ~BoolPortion();

  bool& Value( void ) const;
  PortionType Type( void ) const;
  void Output( gOutput& s ) const;
  Portion* ValCopy( void ) const;
  Portion* RefCopy( void ) const;
  void AssignFrom( Portion* p );
  bool operator == ( Portion* p ) const;
};

class BoolValPortion : public BoolPortion
{
public:
  BoolValPortion( bool value );
  virtual ~BoolValPortion();
  bool IsReference( void ) const;
};

class BoolRefPortion : public BoolPortion
{
public:
  BoolRefPortion( bool& value );
  virtual ~BoolRefPortion();
  bool IsReference( void ) const;
};











//---------------------------------------------------------------------
//                          Outcome class
//---------------------------------------------------------------------

class Outcome;

class OutcomePortion : public Portion
{
protected:
  Outcome** _Value;
  OutcomePortion( void );

public:
  virtual ~OutcomePortion();

  Outcome*& Value( void ) const;
  PortionType Type( void ) const;
  void Output( gOutput& s ) const;
  Portion* ValCopy( void ) const;
  Portion* RefCopy( void ) const;
  void AssignFrom( Portion* p );
  bool operator == ( Portion* p ) const;
};

class OutcomeValPortion : public OutcomePortion
{
public:
  OutcomeValPortion( Outcome* value );
  virtual ~OutcomeValPortion();
  bool IsReference( void ) const;
};

class OutcomeRefPortion : public OutcomePortion
{
public:
  OutcomeRefPortion( Outcome*& value );
  virtual ~OutcomeRefPortion();
  bool IsReference( void ) const;
};



//---------------------------------------------------------------------
//                          EfPlayer class
//---------------------------------------------------------------------

class Player;

class EfPlayerPortion : public Portion
{
protected:
  Player** _Value;
  EfPlayerPortion( void );

public:
  virtual ~EfPlayerPortion();

  Player*& Value( void ) const;
  PortionType Type( void ) const;
  void Output( gOutput& s ) const;
  Portion* ValCopy( void ) const;
  Portion* RefCopy( void ) const;
  void AssignFrom( Portion* p );
  bool operator == ( Portion* p ) const;
};

class EfPlayerValPortion : public EfPlayerPortion
{
public:
  EfPlayerValPortion( Player* value );
  virtual ~EfPlayerValPortion();
  bool IsReference( void ) const;
};

class EfPlayerRefPortion : public EfPlayerPortion
{
public:
  EfPlayerRefPortion( Player*& value );
  virtual ~EfPlayerRefPortion();
  bool IsReference( void ) const;
};




//---------------------------------------------------------------------
//                          Infoset class
//---------------------------------------------------------------------

class Infoset;

class InfosetPortion : public Portion
{
protected:
  Infoset** _Value;
  InfosetPortion( void );

public:
  virtual ~InfosetPortion();

  Infoset*& Value( void ) const;
  PortionType Type( void ) const;
  void Output( gOutput& s ) const;
  Portion* ValCopy( void ) const;
  Portion* RefCopy( void ) const;
  void AssignFrom( Portion* p );
  bool operator == ( Portion* p ) const;
};

class InfosetValPortion : public InfosetPortion
{
public:
  InfosetValPortion( Infoset* value );
  virtual ~InfosetValPortion();
  bool IsReference( void ) const;
};

class InfosetRefPortion : public InfosetPortion
{
public:
  InfosetRefPortion( Infoset*& value );
  virtual ~InfosetRefPortion();
  bool IsReference( void ) const;
};



//---------------------------------------------------------------------
//                          Node class
//---------------------------------------------------------------------

class Node;

class NodePortion : public Portion
{
protected:
  Node** _Value;
  NodePortion( void );

public:
  virtual ~NodePortion();

  Node*& Value( void ) const;
  PortionType Type( void ) const;
  void Output( gOutput& s ) const;
  Portion* ValCopy( void ) const;
  Portion* RefCopy( void ) const;
  void AssignFrom( Portion* p );
  bool operator == ( Portion* p ) const;
};

class NodeValPortion : public NodePortion
{
public:
  NodeValPortion( Node* value );
  virtual ~NodeValPortion();
  bool IsReference( void ) const;
};

class NodeRefPortion : public NodePortion
{
public:
  NodeRefPortion( Node*& value );
  virtual ~NodeRefPortion();
  bool IsReference( void ) const;
};



//---------------------------------------------------------------------
//                          Action class
//---------------------------------------------------------------------

class Action;

class ActionPortion : public Portion
{
protected:
  Action** _Value;
  ActionPortion( void );

public:
  virtual ~ActionPortion();

  Action*& Value( void ) const;
  PortionType Type( void ) const;
  void Output( gOutput& s ) const;
  Portion* ValCopy( void ) const;
  Portion* RefCopy( void ) const;
  void AssignFrom( Portion* p );
  bool operator == ( Portion* p ) const;
};

class ActionValPortion : public ActionPortion
{
public:
  ActionValPortion( Action* value );
  virtual ~ActionValPortion();
  bool IsReference( void ) const;
};

class ActionRefPortion : public ActionPortion
{
public:
  ActionRefPortion( Action*& value );
  virtual ~ActionRefPortion();
  bool IsReference( void ) const;
};






//---------------------------------------------------------------------
//                          Mixed class
//---------------------------------------------------------------------

template <class T> class MixedProfile;

template <class T> class MixedPortion : public Portion
{
protected:
  MixedProfile<T>* _Value;
  MixedPortion( void );

public:
  virtual ~MixedPortion();

  bool IsValid( void ) const;

  MixedProfile<T>& Value( void ) const;
  PortionType Type( void ) const;
  void Output( gOutput& s ) const;
  Portion* ValCopy( void ) const;
  Portion* RefCopy( void ) const;
  void AssignFrom( Portion* p );
  bool operator == ( Portion* p ) const;
};

template <class T> class MixedValPortion : public MixedPortion<T>
{
public:
  MixedValPortion( void );
  MixedValPortion( MixedProfile<T>& value );
  virtual ~MixedValPortion();
  bool IsReference( void ) const;
};

template <class T> class MixedRefPortion : public MixedPortion<T>
{
public:
  MixedRefPortion( MixedProfile<T>& value );
  virtual ~MixedRefPortion();
  bool IsReference( void ) const;
};




//---------------------------------------------------------------------
//                          Behav class
//---------------------------------------------------------------------

template <class T> class BehavProfile;

template <class T> class BehavPortion : public Portion
{
protected:
  BehavProfile<T>* _Value;
  BehavPortion( void );

public:
  virtual ~BehavPortion();

  bool IsValid( void ) const;

  BehavProfile<T>& Value( void ) const;
  PortionType Type( void ) const;
  void Output( gOutput& s ) const;
  Portion* ValCopy( void ) const;
  Portion* RefCopy( void ) const;
  void AssignFrom( Portion* p );
  bool operator == ( Portion* p ) const;
};

template <class T> class BehavValPortion : public BehavPortion<T>
{
public:
  BehavValPortion( void );
  BehavValPortion( BehavProfile<T>& value );
  virtual ~BehavValPortion();
  bool IsReference( void ) const;
};

template <class T> class BehavRefPortion : public BehavPortion<T>
{
public:
  BehavRefPortion( BehavProfile<T>& value );
  virtual ~BehavRefPortion();
  bool IsReference( void ) const;
};





//---------------------------------------------------------------------
//                          BaseNfg class
//---------------------------------------------------------------------

class BaseNormalForm;

class BaseNfgPortion : public Portion
{
protected:
  BaseNormalForm* _Value;
  BaseNfgPortion( void );

public:
  virtual ~BaseNfgPortion();

  BaseNormalForm& Value( void ) const;
  virtual PortionType Type( void ) const = 0;
  virtual void Output( gOutput& s ) const = 0;
  virtual Portion* ValCopy( void ) const = 0;
  virtual Portion* RefCopy( void ) const = 0;
  virtual void AssignFrom( Portion* p );
  virtual bool operator == ( Portion* p ) const;
};


//---------------------------------------------------------------------
//                          Nfg class
//---------------------------------------------------------------------

template <class T> class NormalForm;

template <class T> class NfgPortion : public BaseNfgPortion
{
public:
  NormalForm<T>& Value( void ) const;

  virtual PortionType Type( void ) const;
  virtual void Output( gOutput& s ) const;
  virtual Portion* ValCopy( void ) const;
  virtual Portion* RefCopy( void ) const;
};

template <class T> class NfgValPortion : public NfgPortion<T>
{
public:
  NfgValPortion( NormalForm<T>& value );
  ~NfgValPortion();
  bool IsReference( void ) const;
};

template <class T> class NfgRefPortion : public NfgPortion<T>
{
public:
  NfgRefPortion( NormalForm<T>& value );
  ~NfgRefPortion();
  bool IsReference( void ) const;
};






//---------------------------------------------------------------------
//                          BaseEfg class
//---------------------------------------------------------------------

class BaseExtForm;

class BaseEfgPortion : public Portion
{
protected:
  BaseExtForm* _Value;
  BaseEfgPortion( void );

public:
  virtual ~BaseEfgPortion();

  BaseExtForm& Value( void ) const;
  virtual PortionType Type( void ) const = 0;
  virtual void Output( gOutput& s ) const = 0;
  virtual Portion* ValCopy( void ) const = 0;
  virtual Portion* RefCopy( void ) const = 0;
  virtual void AssignFrom( Portion* p );
  virtual bool operator == ( Portion* p ) const;
};


//---------------------------------------------------------------------
//                          Efg class
//---------------------------------------------------------------------

template <class T> class ExtForm;

template <class T> class EfgPortion : public BaseEfgPortion
{
public:
  ExtForm<T>& Value( void ) const;

  virtual PortionType Type( void ) const;
  virtual void Output( gOutput& s ) const;
  virtual Portion* ValCopy( void ) const;
  virtual Portion* RefCopy( void ) const;
};

template <class T> class EfgValPortion : public EfgPortion<T>
{
public:
  EfgValPortion( ExtForm<T>& value );
  ~EfgValPortion();
  bool IsReference( void ) const;
};

template <class T> class EfgRefPortion : public EfgPortion<T>
{
public:
  EfgRefPortion( ExtForm<T>& value );
  ~EfgRefPortion();
  bool IsReference( void ) const;
};


//---------------------------------------------------------------------
//                          Output class
//---------------------------------------------------------------------

class OutputPortion : public Portion
{
protected:
  gOutput* _Value;
  OutputPortion( void );

public:
  virtual ~OutputPortion();

  gOutput& Value( void ) const;
  PortionType Type( void ) const;
  void Output( gOutput& s ) const;
  Portion* ValCopy( void ) const;
  Portion* RefCopy( void ) const;
  void AssignFrom( Portion* p );
  bool operator == ( Portion* p ) const;
};

class OutputValPortion : public OutputPortion
{
public:
  OutputValPortion( gOutput& value );
  virtual ~OutputValPortion();
  bool IsReference( void ) const;
};

class OutputRefPortion : public OutputPortion
{
public:
  OutputRefPortion( gOutput& value );
  virtual ~OutputRefPortion();
  bool IsReference( void ) const;
};




//---------------------------------------------------------------------
//                          Input class
//---------------------------------------------------------------------

class InputPortion : public Portion
{
protected:
  gInput* _Value;
  InputPortion( void );

public:
  virtual ~InputPortion();

  gInput& Value( void ) const;
  PortionType Type( void ) const;
  void Output( gOutput& s ) const;
  Portion* ValCopy( void ) const;
  Portion* RefCopy( void ) const;
  void AssignFrom( Portion* p );
  bool operator == ( Portion* p ) const;
};

class InputValPortion : public InputPortion
{
public:
  InputValPortion( gInput& value );
  virtual ~InputValPortion();
  bool IsReference( void ) const;
};

class InputRefPortion : public InputPortion
{
public:
  InputRefPortion( gInput& value );
  virtual ~InputRefPortion();
  bool IsReference( void ) const;
};



//---------------------------------------------------------------------
//                          List class
//---------------------------------------------------------------------

template <class T> class gBlock;

class ListPortion : public Portion
{
protected:
  gBlock< Portion* >* _Value;
  ListPortion( void );

  PortionType      _DataType;
  bool TypeCheck( Portion* item );

public:
  virtual ~ListPortion();

  void SetOwner( Portion* p );

  gBlock< Portion* >& Value( void ) const;
  PortionType Type( void ) const;
  void Output( gOutput& s ) const;
  Portion* ValCopy( void ) const;
  Portion* RefCopy( void ) const;
  void AssignFrom( Portion* p );
  bool operator == ( Portion* p ) const;

  void              SetDataType ( PortionType data_type );
  PortionType       DataType    ( void ) const;

  int      Append     ( Portion* item );
  int      Insert     ( Portion* item, int index );
  int      Contains   ( Portion* item ) const;
  Portion* Remove     ( int index );
  int      Length     ( void ) const;
  void     Flush      ( void );

  Portion* Subscript( int index ) const;
};

class ListValPortion : public ListPortion
{
public:
  ListValPortion( void );
  ListValPortion( gBlock< Portion* >& value );
  virtual ~ListValPortion();
  bool IsReference( void ) const;
};

class ListRefPortion : public ListPortion
{
public:
  ListRefPortion( gBlock< Portion* >& value );
  virtual ~ListRefPortion();
  bool IsReference( void ) const;
};







gString PortionTypeToText( const PortionType& type );
PortionType TextToPortionType( const gString& text );


void PrintPortionTypeSpec( gOutput& s, PortionType type );

gOutput& operator << ( gOutput& s, Portion* p );


#endif // PORTION_H





