//#
//# FILE: portion.h -- header file for Portion class
//#                    companion to GSM
//#
//# $Id$
//#




#ifndef PORTION_H
#define PORTION_H

#include <assert.h>
#include "basic.h"

#include "rational.h"
#include "gstring.h"
#include "gblock.h"
#include "gambitio.h"


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
  porSTRING, porLIST,
  porREFERENCE 
} PortionType;



class GSM;


class Portion
{
 private:
  // variable used to detect memory leakage
#ifdef MEMCHECK
  static int num_of_Portions;
#endif

 public:
  Portion();
  virtual ~Portion();

  virtual PortionType Type      ( void ) const = 0;
  virtual Portion*    Copy      ( void ) const = 0;
  virtual bool        Operation ( Portion* p, OperationMode mode );
  virtual void        Output    ( gOutput& s ) const = 0;
};




template <class T> class numerical_Portion : public Portion
{
 private:
  T value;

 public:
  numerical_Portion( const T& new_value );

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
  bool value;

 public:
  bool_Portion( const bool& new_value );

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
  gString value;

 public:
  gString_Portion( const gString& new_value );

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
  gString value;

 public:
  Reference_Portion( const gString& new_value );

  gString&    Value  ( void );
  gString     Value  ( void ) const;
  Portion*    Copy   ( void ) const;
  PortionType Type   ( void ) const;
  void        Output ( gOutput& s ) const;
};


class List_Portion : public Portion
{
 private:
  gBlock<Portion*> value;
  PortionType      data_type;

  int TypeCheck( Portion* item );

 public:
  List_Portion( void );
  List_Portion( const gBlock<Portion*>& new_value );
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



#endif // PORTION_H
