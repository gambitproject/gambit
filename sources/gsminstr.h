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
  iERROR,

  iNOP,

  iIF_GOTO, iGOTO,

  iPUSH, iPUSHLIST, iPUSHREF,
  iASSIGN, iUNASSIGN,

  iADD, iSUB, iMUL, iDIV, iNEG,
  iINT_DIV, iMOD,

  iEQU, iNEQ, iGTN, iLTN, iGEQ, iLEQ,

  iAND, iOR, iNOT,

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
  virtual ~Instruction();

  virtual Opcode Type( void ) const = 0;
  virtual bool Execute( GSM& gsm ) const = 0;
};


//--------------------------------------------------------------------
//                      descendent Instruction classes
//--------------------------------------------------------------------


//-------------------------- null operation --------------------------

class NOP : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
};


//----------------------- branch operators ---------------------------

class IfGoto : public Instruction
{
 private:
  int _InstructionIndex;
 public:
  IfGoto( int index );
  int WhereTo( void ) const;
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
};


class Goto : public Instruction
{
 private:
  int _InstructionIndex;
 public:
  Goto( int index );
  int WhereTo( void ) const;
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
};



//------------------------- push opeerations ------------------------

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
 public:
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


class Neg : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM &gsm ) const;
};



//----------------------------- integer math operators -------------------

class IntDiv : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM &gsm ) const;
};

class Mod : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM &gsm ) const;
};



//----------------------------- relational operators ----------------------

class EQU : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
};


class NEQ : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
};


class GTN : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
};


class LTN : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
};


class GEQ : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
};


class LEQ : public Instruction
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
