//#
//# FILE: portion.h -- header file for Portion class
//#                    companion to GSM
//#
//# $Id$
//#




#ifndef PORTION_H
#define PORTION_H


#include "gambitio.h"

#include "gblock.h"
#include "rational.h"
#include "gstring.h"
#include "nfg.h"


typedef enum 
{ 
  opERROR, 
  opADD, opSUBTRACT, opMULTIPLY, opDIVIDE, opNEGATE,
  opINTEGER_DIVIDE, opMODULUS,
  opEQUAL_TO, opNOT_EQUAL_TO, opGREATER_THAN, opLESS_THAN,
  opGREATER_THAN_OR_EQUAL_TO, opLESS_THAN_OR_EQUAL_TO,
  opLOGICAL_AND, opLOGICAL_OR, opLOGICAL_NOT
} OperationMode;



typedef unsigned int PortionType;

#define  porERROR      ( 0x00 )
#define  porBOOL       ( 0x01 )
#define  porDOUBLE     ( 0x02 )
#define  porINTEGER    ( 0x04 )
#define  porRATIONAL   ( 0x08 )
#define  porNUMERICAL  ( porDOUBLE | porINTEGER | porRATIONAL )
#define  porSTRING     ( 0x10 )
#define  porLIST       ( 0x20 )
#define  porNFG        ( 0x40 )
#define  porREFERENCE  ( 0x80 )






class RefHashTable;
class GSM;


class Portion
{
 private:
  // variable used to detect memory leakage
  static int _NumPortions;

 protected:
  bool       _Temporary;

 public:
  Portion();
  virtual ~Portion();

  bool&               Temporary    ( void );
  virtual PortionType Type         ( void ) const = 0;
  virtual Portion*    Copy         ( void ) const = 0;
  virtual void        CopyDataFrom ( Portion* p );
  virtual bool        Operation    ( Portion* p, OperationMode mode );
  virtual void        Output       ( gOutput& s ) const = 0;
};




class Error_Portion : public Portion
{
 public:
  PortionType Type         ( void ) const;
  Portion*    Copy         ( void ) const;
  void        Output       ( gOutput& s ) const;
};


template <class T> class numerical_Portion : public Portion
{
 private:
  T _Value;

 public:
  numerical_Portion( const T& value );

  T&          Value     ( void );
  T           Value     ( void ) const;
  Portion*    Copy      ( void ) const;
  PortionType Type      ( void ) const;
  bool        Operation ( Portion* p, OperationMode mode );
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
  Portion*    Copy      ( void ) const;
  PortionType Type      ( void ) const;
  bool        Operation ( Portion* p, OperationMode mode );
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
  Portion*    Copy      ( void ) const;
  PortionType Type      ( void ) const;
  bool        Operation ( Portion* p, OperationMode mode );
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
  Portion*    Copy     ( void ) const;
  PortionType Type     ( void ) const;
  void        Output   ( gOutput& s ) const;
};


class List_Portion : public Portion
{
 private:
  gBlock<Portion*> _Value;
  PortionType      _DataType;

  int TypeCheck( Portion* item );

 public:
  List_Portion( void );
  List_Portion( const gBlock<Portion*>& value );
  ~List_Portion();

  gBlock<Portion*>& Value     ( void );
  gBlock<Portion*>  Value     ( void ) const;
  Portion*          Copy      ( void ) const;
  PortionType       Type      ( void ) const;
  PortionType       DataType  ( void ) const;
  bool              Operation ( Portion* p, OperationMode mode );
  void              Output    ( gOutput& s ) const;

  Portion* operator[] ( int index );
  int      Append     ( Portion* item );
  int      Insert     ( Portion* item, int index );
  Portion* Remove     ( int index );
  int      Length     ( void ) const;
  void     Flush      ( void );
};


class Nfg_Portion : public Portion
{
 private:
  Nfg*           _Value;
  RefHashTable*  _RefTable;

 public:
  Nfg_Portion( Nfg& value );
  ~Nfg_Portion();

  // Only the passing by reference version of Value() is provided in 
  // order to eliminate unecessary copying
  Nfg&        Value        ( void );
  Portion*    Copy         ( void ) const;
  void        CopyDataFrom ( Portion* p );
  PortionType Type         ( void ) const;
  bool        Operation    ( Portion* p, OperationMode mode );
  void        Output       ( gOutput& s ) const;

  bool        Assign     ( const gString& ref, Portion* data );
  bool        UnAssign   ( const gString& ref );
  Portion*    operator() ( const gString& ref ) const;
};




void PrintPortionTypeSpec( gOutput& s, PortionType type );



#endif // PORTION_H
