//#
//# FILE: gsminstr.h -- definition of Instruction classes for GSM's
//#                     instruction queue subsystem
//#                     companion to GSM
//#
//# $Id$
//#



#ifndef GSMINSTR_H
#define GSMINSTR_H


#include "gsm.h"


//-------------------------------------------------------------------
//                      Opcodes
//-------------------------------------------------------------------

typedef enum
{
  iPUSH, iPUSHLIST, iPUSHREF,
  iASSIGN, iUNASSIGN,
  iADD, iSUB, iMUL, iDIV,
  iEQUAL_TO, iNOT_EQUAL_TO, iGREATER_THAN, iLESS_THAN,
  iGREATER_THAN_OR_EQUAL_TO, iLESS_THAN_OR_EQUAL_TO,
  iAND, iOR, iNOT,
  iCONCATENATE,
  iINIT_CALL_FUNCTION, iBIND, iCALL_FUNCTION,
  iOUTPUT, iDUMP, iFLUSH
} Opcode;

//--------------------------------------------------------------------
//                      Base Instructiong class
//--------------------------------------------------------------------

class Instruction
{
 private:
  static int _NumInstructions;

 public:
  Instruction( void );
  ~Instruction();

  virtual Opcode Type( void ) const = 0;
  virtual bool Execute( GSM& gsm ) const = 0;
};


//--------------------------------------------------------------------
//                     Descendent Instruction classes
//--------------------------------------------------------------------

template <class T> class Push : public Instruction
{
 private:
  T _Value;
 public:
  Push( const T& value );
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
};


class PushList : public Instruction
{
 private:
  int _NumElements;
 public:
  PushList( const int num_elements );
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
};


class PushRef : public Instruction
{
 private:
  gString _Ref;
 public:
  PushRef( const gString& ref );
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
};


class Assign : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
};


class UnAssign : public Instruction
{
 private:
  gString _Ref;
 public:
  UnAssign( const gString& ref );
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
};


//--------------------------- math operations ---------------------------

class Add : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM &gsm ) const;
};


class Sub : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM &gsm ) const;
};


class Mul : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM &gsm ) const;
};


class Div : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM &gsm ) const;
};


//----------------------------- relational operators ----------------------

class EqualTo : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
};


class NotEqualTo : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
};


class GreaterThan : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
};


class LessThan : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
};


class GreaterThanOrEqualTo : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
};


class LessThanOrEqualTo : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
};


//----------------------------- logical operators ------------------------

class AND : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
};


class OR : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
};


class NOT : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
};


//------------------------ function call operations -------------------

class InitCallFunction : public Instruction
{
 private:
  gString _FuncName;
 public:
  InitCallFunction( const gString& func_name );
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
};


class Bind : public Instruction
{
 private:
  gString _FuncName;
 public:
  Bind( void );
  Bind( const gString& func_name );
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
};


class CallFunction : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
};


//------------------------- miscellaneous instructions --------------------

class Output : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
};


class Dump : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
};


class Flush : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
};


#endif // GSMINSTR_H
