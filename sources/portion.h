//#
//# FILE: portion.h -- header file for Portion class
//#                    companion to GSM
//#
//# $Id$
//#




#ifndef PORTION_H
#define PORTION_H


#include "gsmincl.h"

#include "basic.h"
#include "integer.h"
#include "rational.h"

#include "gstring.h"

#include "gblock.h"
#include "mixed.h"
#include "behav.h"


class List_Portion;

class Outcome;
class Player;
class Infoset;
class Action;
class Node;

template <class T> class RefCountHashTable;





class Portion
{
 private:
  // variable used to detect memory leakage
  static int _NumPortions;

 protected:
  // the following two are only used by List operations (so far)
  Portion*       _ShadowOf;
  List_Portion*  _ParentList;

  bool _Static;
  static gString _ErrorMessage( const int error_num, const gString& str = "" );

 public:
  Portion();
  virtual ~Portion();

  Portion*&           ShadowOf       ( void );
  List_Portion*&      ParentList     ( void );
  virtual PortionType Type           ( void ) const = 0;
  virtual Portion*    Copy           ( bool new_data = false ) const = 0;
  virtual Portion*    Operation      ( Portion* p, OperationMode mode );
  virtual void        Output         ( gOutput& s ) const = 0;
};






class Error_Portion : public Portion
{
 private:
  gString _Value;

 public:
  Error_Portion( const gString& value = "" );

  gString&    Value     ( void );
  Portion*    Copy      ( bool new_data ) const;
  PortionType Type      ( void ) const;
  void        Output    ( gOutput& s ) const;
};



template <class T> class numerical_Portion : public Portion
{
 private:
  static _NumObj;
  T* _Value;

 public:
  numerical_Portion( const T& value );
  numerical_Portion( T& value, bool var_static );
  ~numerical_Portion();
  
  T&          Value     ( void );
  Portion*    Copy      ( bool new_data ) const;
  PortionType Type      ( void ) const;
  Portion*    Operation ( Portion* p, OperationMode mode );
  void        Output    ( gOutput& s ) const;
};


class bool_Portion : public Portion
{
 private:
  static _NumObj;
  bool* _Value;

 public:
  bool_Portion( const bool& value );
  bool_Portion( bool& value, bool var_static );
  ~bool_Portion();

  bool&       Value     ( void );
  Portion*    Copy      ( bool new_data ) const;
  PortionType Type      ( void ) const;
  Portion*    Operation ( Portion* p, OperationMode mode );
  void        Output    ( gOutput& s ) const;
};


class gString_Portion : public Portion
{
 private:
  static _NumObj;
  gString *_Value;

 public:
  gString_Portion( const gString& value );
  gString_Portion( gString& value, bool var_static );
  ~gString_Portion();

  gString&    Value     ( void );
  Portion*    Copy      ( bool new_data ) const;
  PortionType Type      ( void ) const;
  Portion*    Operation ( Portion* p, OperationMode mode );
  void        Output    ( gOutput& s ) const;
};



class Reference_Portion : public Portion
{
 private:
  gString     _Value;

 public:
  Reference_Portion( const gString& value );

  gString&     Value    ( void );
  Portion*     Copy     ( bool new_data ) const;
  PortionType  Type     ( void ) const;
  void         Output   ( gOutput& s ) const;
};





class List_Portion : public Portion
{
 protected:
  gBlock<Portion*> _Value;
  PortionType      _DataType;

  bool TypeCheck( Portion* item );

 public:
  List_Portion( void );
  List_Portion( const gBlock<Portion*>& value );
  ~List_Portion();

  gBlock<Portion*>& Value     ( void );
  Portion*          Copy      ( bool new_data ) const;
  PortionType       Type      ( void ) const;
  PortionType       DataType  ( void ) const;
  Portion*          Operation ( Portion* p, OperationMode mode );
  void              Output    ( gOutput& s ) const;

  int      Append     ( Portion* item );
  int      Insert     ( Portion* item, int index );
  Portion* Remove     ( int index );
  int      Length     ( void ) const;
  void     Flush      ( void );

  Portion* GetSubscript( int index ) const;
  Portion* SetSubscript( int index, Portion* p );
};





template <class T> class Mixed_Portion : public Portion
{
 private:
  MixedProfile<T> _Value;

 public:
  Mixed_Portion( const MixedProfile<T>& value );

  MixedProfile<T>& Value     ( void );
  Portion*         Copy      ( bool new_data ) const;
  PortionType      Type      ( void ) const;
  void             Output    ( gOutput& s ) const;
};




template <class T> class Behav_Portion : public Portion
{
 private:
  BehavProfile<T> _Value;

 public:
  Behav_Portion( const BehavProfile<T>& value );

  BehavProfile<T>& Value     ( void );
  Portion*         Copy      ( bool new_data ) const;
  PortionType      Type      ( void ) const;
  void             Output    ( gOutput& s ) const;
};








class BaseNfg_Portion : public Portion
{
private:
  static int _NumObj;
  static RefCountHashTable< BaseNormalForm* > _RefCountTable;

protected:
  BaseNormalForm* _Value;

public:
  BaseNfg_Portion( BaseNormalForm& value );
  ~BaseNfg_Portion();

  BaseNormalForm&     Value          ( void );
  virtual Portion*    Copy           ( bool new_data ) const = 0;
  virtual PortionType Type           ( void ) const;
  virtual void        Output         ( gOutput& s ) const;
};


template <class T> class Nfg_Portion : public BaseNfg_Portion
{
public:
  Nfg_Portion( NormalForm<T>& value );

  NormalForm<T>& Value          ( void );
  Portion*       Copy           ( bool new_data ) const;
  PortionType    Type           ( void ) const;
  void           Output         ( gOutput& s ) const;
};








class BaseEfg_Portion : public Portion
{
private:
  static int _NumObj;
  static RefCountHashTable< BaseExtForm* > _RefCountTable;

protected:
  BaseExtForm* _Value;

public:
  BaseEfg_Portion( BaseExtForm& value );
  ~BaseEfg_Portion();

  BaseExtForm&        Value          ( void );
  virtual Portion*    Copy           ( bool new_data ) const = 0;
  virtual PortionType Type           ( void ) const;
  virtual void        Output         ( gOutput& s ) const;
};


template <class T> class Efg_Portion : public BaseEfg_Portion
{
public:
  Efg_Portion( ExtForm<T>& value );

  ExtForm<T>&    Value          ( void );
  Portion*       Copy           ( bool new_data ) const;
  PortionType    Type           ( void ) const;
  void           Output         ( gOutput& s ) const;
};


















class Outcome_Portion : public Portion
{
 private:
  Outcome* _Value;

 public:
  Outcome_Portion( Outcome* value );

  Outcome*&   Value     ( void );
  Portion*    Copy      ( bool new_data ) const;
  PortionType Type      ( void ) const;
  void        Output    ( gOutput& s ) const;
};


class Player_Portion : public Portion
{
 private:
  Player* _Value;

 public:
  Player_Portion( Player* value );

  Player*&    Value     ( void );
  Portion*    Copy      ( bool new_data ) const;
  PortionType Type      ( void ) const;
  void        Output    ( gOutput& s ) const;
};


class Infoset_Portion : public Portion
{
 private:
  Infoset* _Value;

 public:
  Infoset_Portion( Infoset* value );

  Infoset*&   Value     ( void );
  Portion*    Copy      ( bool new_data ) const;
  PortionType Type      ( void ) const;
  void        Output    ( gOutput& s ) const;
};


class Action_Portion : public Portion
{
 private:
  Action* _Value;

 public:
  Action_Portion( Action* value );

  Action*&    Value     ( void );
  Portion*    Copy      ( bool new_data ) const;
  PortionType Type      ( void ) const;
  void        Output    ( gOutput& s ) const;
};


class Node_Portion : public Portion
{
 private:
  Node* _Value;

 public:
  Node_Portion( Node* value );

  Node*&      Value     ( void );
  Portion*    Copy      ( bool new_data ) const;
  PortionType Type      ( void ) const;
  void        Output    ( gOutput& s ) const;
};




class Stream_Portion : public Portion
{
 private:
  static int _NumObj;
  static RefCountHashTable< gOutput* > _RefCountTable;
  gOutput* _Value;

 public:
  Stream_Portion( gOutput& value );
  ~Stream_Portion();

  gOutput&      Value          ( void );
  Portion*      Copy           ( bool new_data ) const;
  PortionType   Type           ( void ) const;
  void          Output         ( gOutput& s ) const;
};






void PrintPortionTypeSpec( gOutput& s, PortionType type );

gOutput& operator << ( gOutput& s, Portion* p );


#endif // PORTION_H

