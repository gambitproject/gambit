//#
//# FILE: portion.h -- header file for Portion class
//#                    companion to GSM
//#
//# $Id$
//#




#ifndef PORTION_H
#define PORTION_H


#include "gblock.h"
#include "rational.h"
#include "gstring.h"
#include "nfg.h"


typedef enum 
{ 
  opERROR, 
  opADD, opSUBTRACT, opMULTIPLY, opDIVIDE, opNEGATE,
  opEQUAL_TO, opNOT_EQUAL_TO, opGREATER_THAN, opLESS_THAN,
  opGREATER_THAN_OR_EQUAL_TO, opLESS_THAN_OR_EQUAL_TO,
  opLOGICAL_AND, opLOGICAL_OR, opLOGICAL_NOT,
  opCONCATENATE
} OperationMode;



typedef enum 
{ 
  porERROR, porBOOL, 
  porNUMERICAL, porDOUBLE, porINTEGER, porRATIONAL, 
  porSTRING, porLIST, porNFG,
  porREFERENCE 
} PortionType;





class RefHashTable;
class GSM;


class Portion
{
 private:
  // variable used to detect memory leakage
  static int _NumPortions;

 public:
  Portion();
  virtual ~Portion();

  virtual PortionType Type         ( void ) const = 0;
  virtual Portion*    Copy         ( void ) const = 0;
  virtual bool        Operation    ( Portion* p, OperationMode mode );
  virtual void        Output       ( gOutput& s ) const = 0;
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

  gString&    Value     ( void );
  gString     Value     ( void ) const;
  gString&    SubValue  ( void );
  gString     SubValue  ( void ) const;
  Portion*    Copy      ( void ) const;
  PortionType Type      ( void ) const;
  void        Output    ( gOutput& s ) const;
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
  Nfg_Portion( const double& value );
  Nfg_Portion( Nfg& value );
  Nfg_Portion( const Nfg_Portion& value );
  ~Nfg_Portion();

  Nfg&        Value     ( void );
  Nfg         Value     ( void ) const;
  Portion*    Copy      ( void ) const;
  PortionType Type      ( void ) const;
  bool        Operation ( Portion* p, OperationMode mode );
  void        Output    ( gOutput& s ) const;

  bool        Assign     ( const gString& ref, Portion* data );
  bool        UnAssign   ( const gString& ref );
  Portion*    operator() ( const gString& ref ) const;
};





#endif // PORTION_H
