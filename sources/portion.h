//
//  File:  header file for Portion class, campanion to GSM
//
//



#ifndef PORTION_H
#define PORTION_H

#include <assert.h>

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
 public:
  virtual ~Portion();
  virtual PortionType Type( void ) const = 0;
  virtual Portion *Copy( void ) const = 0;

  virtual int Operation( Portion *p, OperationMode mode );
  virtual void Output( gOutput& s ) const = 0;
};



template <class T> class numerical_Portion : public Portion
{
 private:
  T value;

 public:
  numerical_Portion( const T& new_value );

  T Value( void ) const;
  T& Value( void );
  PortionType Type( void ) const;
  Portion *Copy( void ) const;
  int Operation( Portion *p, OperationMode mode );
  void Output( gOutput& s ) const;
};



class bool_Portion : public Portion
{
 private:
  bool value;

 public:
  bool_Portion( const bool& new_value );

  bool Value( void ) const;
  bool& Value( void );
  PortionType Type( void ) const;
  Portion *Copy( void ) const;
  int Operation( Portion *p, OperationMode mode );
  void Output( gOutput& s ) const;
};

class gString_Portion : public Portion
{
 private:
  gString value;

 public:
  gString_Portion( const gString& new_value );

  gString Value( void ) const;
  gString& Value( void );
  PortionType Type( void ) const;
  Portion *Copy( void ) const;
  int Operation( Portion *p, OperationMode mode );
  void Output( gOutput& s ) const;
};

class Reference_Portion : public Portion
{
 private:
  gString value;

 public:
  Reference_Portion( const gString& new_value );

  gString Value( void ) const;
  gString& Value( void );
  PortionType Type( void ) const;
  Portion *Copy( void ) const;
  void Output( gOutput& s ) const;
};



class List_Portion : public Portion
{
 private:
  gBlock<Portion *> value;
  PortionType data_type;
  int TypeCheck( Portion *item );

 public:
  List_Portion( void );
  List_Portion( const gBlock<Portion *>& new_value );
  ~List_Portion();

  gBlock<Portion *> Value( void ) const;
  gBlock<Portion *>& Value( void );
  PortionType Type( void ) const;
  PortionType DataType( void ) const;
  Portion *Copy( void ) const;
  int Operation( Portion *p, OperationMode mode );
  void Output( gOutput& s ) const;

  Portion *operator[] ( int index );
  int Append( Portion *item );
  int Insert( Portion *item, int index );
  Portion *Remove( int index );
  int Length( void ) const;
  void Flush( void );
};



#endif // PORTION_H
