//#
//# FILE: portion.h -- header file for Portion class
//#                    companion to GSM
//#
//# $Id$
//#




#ifndef PORTION_H
#define PORTION_H


#include "gambitio.h"

#include "rational.h"
#include "gstring.h"

#include "gsmhash.h"




typedef enum 
{ 
  opERROR, 
  opADD, opSUBTRACT, opMULTIPLY, opDIVIDE, opNEGATE,
  opINTEGER_DIVIDE, opMODULUS,
  opEQUAL_TO, opNOT_EQUAL_TO, opGREATER_THAN, opLESS_THAN,
  opGREATER_THAN_OR_EQUAL_TO, opLESS_THAN_OR_EQUAL_TO,
  opLOGICAL_AND, opLOGICAL_OR, opLOGICAL_NOT,
  opSUBSCRIPT
} OperationMode;



typedef unsigned int PortionType;

#define  porERROR      ( 0x0000 )
#define  porBOOL       ( 0x0001 )
#define  porDOUBLE     ( 0x0002 )
#define  porINTEGER    ( 0x0004 )
#define  porRATIONAL   ( 0x0008 )
#define  porSTRING     ( 0x0010 )
#define  porLIST       ( 0x0020 )

#define  porNFG_DOUBLE     ( 0x0040 )
#define  porNFG_RATIONAL   ( 0x0080 )
#define  porNFG            ( porNFG_DOUBLE | porNFG_RATIONAL )
#define  porEFG_DOUBLE     ( 0x0100 )
#define  porEFG_RATIONAL   ( 0x0200 )
#define  porEFG            ( porEFG_DOUBLE | porEFG_RATIONAL )
#define  porMIXED_DOUBLE   ( 0x0400 )
#define  porMIXED_RATIONAL ( 0x0800 )
#define  porMIXED          ( porMIXED_DOUBLE | porMIXED_RATIONAL )
#define  porBEHAV_DOUBLE   ( 0x1000 )
#define  porBEHAV_RATIONAL ( 0x2000 )
#define  porBEHAV          ( porBEHAV_DOUBLE | porBEHAV_RATIONAL )

#define  porOUTCOME    ( 0x4000 )
#define  porPLAYER     ( 0x8000 )
#define  porINFOSET    ( 0x010000 )
#define  porNODE       ( 0x020000 )
#define  porACTION     ( 0x040000 )

#define  porSTREAM     ( 0x080000 )

#define  porREFERENCE  ( 0x100000 )

#define  porALLOWS_SUBVARIABLES ( porNFG | porEFG )
				  
#define  porNUMERICAL  ( porDOUBLE | porINTEGER | porRATIONAL )
#define  porALL        ( 0xFFFFFFFF )
#define  porVALUE      ( porALL & ~porREFERENCE )




class List_Portion;









class Portion
{
 private:
  // variable used to detect memory leakage
  static int _NumPortions;

 protected:
  // the following two are only used by List operations (so far)
  Portion*       _ShadowOf;
  List_Portion*  _ParentList;

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
  gString     Value     ( void ) const;
  Portion*    Copy      ( bool new_data ) const;
  PortionType Type      ( void ) const;
  void        Output    ( gOutput& s ) const;
};



template <class T> class numerical_Portion : public Portion
{
 private:
  T _Value;

 public:
  numerical_Portion( const T& value );

  T&          Value     ( void );
  T           Value     ( void ) const;
  Portion*    Copy      ( bool new_data ) const;
  PortionType Type      ( void ) const;
  Portion*    Operation ( Portion* p, OperationMode mode );
  void        Output    ( gOutput& s ) const;
};


class bool_Portion : public Portion
{
 private:
  bool _Value;

 public:
  bool_Portion( const bool& value );

  bool&       Value     ( void );
  bool        Value     ( void ) const;
  Portion*    Copy      ( bool new_data ) const;
  PortionType Type      ( void ) const;
  Portion*    Operation ( Portion* p, OperationMode mode );
  void        Output    ( gOutput& s ) const;
};


class gString_Portion : public Portion
{
 private:
  gString _Value;

 public:
  gString_Portion( const gString& value );

  gString&    Value     ( void );
  gString     Value     ( void ) const;
  Portion*    Copy      ( bool new_data ) const;
  PortionType Type      ( void ) const;
  Portion*    Operation ( Portion* p, OperationMode mode );
  void        Output    ( gOutput& s ) const;
};



class Reference_Portion : public Portion
{
 private:
  gString _Value;
  gString _SubValue;

 public:
  Reference_Portion( const gString& value );
  Reference_Portion( const gString& value, const gString& subvalue );

  gString&    Value    ( void );
  gString     Value    ( void ) const;
  gString&    SubValue ( void );
  gString     SubValue ( void ) const;
  Portion*    Copy     ( bool new_data ) const;
  PortionType Type     ( void ) const;
  void        Output   ( gOutput& s ) const;
};




#include "gblock.h"

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
  gBlock<Portion*>  Value     ( void ) const;
  Portion*          Copy      ( bool new_data ) const;
  PortionType       Type      ( void ) const;
  PortionType       DataType  ( void ) const;
  void              Output    ( gOutput& s ) const;

  int      Append     ( Portion* item );
  int      Insert     ( Portion* item, int index );
  Portion* Remove     ( int index );
  int      Length     ( void ) const;
  void     Flush      ( void );

  Portion* GetSubscript( int index ) const;
  Portion* SetSubscript( int index, Portion* p );
};




#include "mixed.h"

template <class T> class Mixed_Portion : public Portion
{
 private:
  MixedProfile<T> _Value;
  NormalForm<T>*  _Owner;

 public:
  Mixed_Portion( const MixedProfile<T>& value );

  bool SetOwner( NormalForm<T>* owner );

  MixedProfile<T>& Value     ( void );
  MixedProfile<T>  Value     ( void ) const;
  Portion*         Copy      ( bool new_data ) const;
  PortionType      Type      ( void ) const;
  void             Output    ( gOutput& s ) const;
};



#include "behav.h"

template <class T> class Behav_Portion : public Portion
{
 private:
  BehavProfile<T> _Value;
  ExtForm<T>*     _Owner;

 public:
  Behav_Portion( const BehavProfile<T>& value );

  bool SetOwner( ExtForm<T>* owner );

  BehavProfile<T>& Value     ( void );
  BehavProfile<T>  Value     ( void ) const;
  Portion*         Copy      ( bool new_data ) const;
  PortionType      Type      ( void ) const;
  void             Output    ( gOutput& s ) const;
};






#include "normal.h"

template <class T> class Nfg_Portion : public Portion
{
 private:
  static int _NumObj;
  static RefCountHashTable< NormalForm<T>* > _RefCountTable;
  NormalForm<T>* _Value;
  RefHashTable*  _RefTable;

 public:
  Nfg_Portion( NormalForm<T>& value );
  ~Nfg_Portion();

  // Only the passing by reference version of Value() is provided in 
  // order to eliminate unecessary copying
  NormalForm<T>& Value          ( void );
  Portion*       Copy           ( bool new_data ) const;
  PortionType    Type           ( void ) const;
  void           Output         ( gOutput& s ) const;

  bool        Assign     ( const gString& ref, Portion* data );
  bool        UnAssign   ( const gString& ref );
  bool        IsDefined  ( const gString& ref ) const;
  Portion*    operator() ( const gString& ref ) const;
};






#include "extform.h"

template <class T> class Efg_Portion : public Portion
{
 private:
  static int _NumObj;
  static RefCountHashTable< ExtForm<T>* > _RefCountTable;
  ExtForm<T>*    _Value;
  RefHashTable*  _RefTable;

 public:
  Efg_Portion( ExtForm<T>& value );
  ~Efg_Portion();

  // Only the passing by reference version of Value() is provided in 
  // order to eliminate unecessary copying
  ExtForm<T>&    Value          ( void );
  Portion*       Copy           ( bool new_data ) const;
  PortionType    Type           ( void ) const;
  void           Output         ( gOutput& s ) const;

  bool        Assign     ( const gString& ref, Portion* data );
  bool        UnAssign   ( const gString& ref );
  bool        IsDefined  ( const gString& ref ) const;
  Portion*    operator() ( const gString& ref ) const;
};




#include "outcome.h"

class Outcome_Portion : public Portion
{
 private:
  Outcome* _Value;

 public:
  Outcome_Portion( Outcome* value );

  Outcome*&   Value     ( void );
  Outcome*    Value     ( void ) const;
  Portion*    Copy      ( bool new_data ) const;
  PortionType Type      ( void ) const;
  void        Output    ( gOutput& s ) const;
};



#include "player.h"

class Player_Portion : public Portion
{
 private:
  Player* _Value;

 public:
  Player_Portion( Player* value );

  Player*&    Value     ( void );
  Player*     Value     ( void ) const;
  Portion*    Copy      ( bool new_data ) const;
  PortionType Type      ( void ) const;
  void        Output    ( gOutput& s ) const;
};



#include "infoset.h"

class Infoset_Portion : public Portion
{
 private:
  Infoset* _Value;

 public:
  Infoset_Portion( Infoset* value );

  Infoset*&   Value     ( void );
  Infoset*    Value     ( void ) const;
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
  Action*     Value     ( void ) const;
  Portion*    Copy      ( bool new_data ) const;
  PortionType Type      ( void ) const;
  void        Output    ( gOutput& s ) const;
};



#include "node.h"

class Node_Portion : public Portion
{
 private:
  Node* _Value;

 public:
  Node_Portion( Node* value );

  Node*&      Value     ( void );
  Node*       Value     ( void ) const;
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

