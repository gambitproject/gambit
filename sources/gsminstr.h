//#
//# FILE: gsminstr.h -- definition of Instruction classes for GSM's
//#                     instruction queue subsystem
//#                     companion to GSM
//#
//# $Id$
//#



#ifndef GSMINSTR_H
#define GSMINSTR_H


#include "gstring.h"


class GSM;
class gOutput;


//-------------------------------------------------------------------
//                      Opcodes
//-------------------------------------------------------------------

typedef enum
{
  iERROR,

  iNOP,

  iQUIT, iIF_GOTO, iGOTO, iCLEAR,

  iPUSH, iPUSHLIST, iPUSHREF,
  iASSIGN, iUNASSIGN, iSUBSCRIPT, iCHILD, iWRITE,

  iADD, iSUB, iCONCAT, iMUL, iDOT, iDIV, iINTDIV, iNEG,
  iMOD, iPOWER,

  iEQU, iNEQ, iGTN, iLTN, iGEQ, iLEQ,

  iAND, iOR, iNOT,

  iINIT_CALL_FUNCTION, iBIND, iBINDREF, iBINDVAL, iCALL_FUNCTION,

  iPOP, iOUTPUT, iDUMP, iFLUSH
} Opcode;

//--------------------------------------------------------------------
//                      Base Instructiong class
//--------------------------------------------------------------------

class Instruction
{
private:
  static int _NumInstructions;
  
protected:
  int _LineNumber;
 
public:
  Instruction( void );
  virtual ~Instruction();

  int& LineNumber( void );

  virtual Opcode Type( void ) const = 0;
  virtual bool Execute( GSM& gsm ) const = 0;
  virtual void Output( gOutput& s ) const = 0;
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
  void Output( gOutput& s ) const;
};


//----------------------- branch operators ---------------------------

class Quit : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
  void Output( gOutput& s ) const;
};


class Clear : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
  void Output( gOutput& s ) const;
};

class IfGoto : public Instruction
{
 private:
  int _InstructionIndex;
 public:
  IfGoto( int index );
  int WhereTo( void ) const;
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
  void Output( gOutput& s ) const;
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
  void Output( gOutput& s ) const;
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
  void Output( gOutput& s ) const;
};


class PushList : public Instruction
{
 private:
  int _NumElements;
 public:
  PushList( const int num_elements );
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
  void Output( gOutput& s ) const;
};


class PushRef : public Instruction
{
 private:
  gString _Ref;
 public:
  PushRef( const gString& ref );
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
  void Output( gOutput& s ) const;
};


class Assign : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
  void Output( gOutput& s ) const;
};


class UnAssign : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
  void Output( gOutput& s ) const;
};


class Subscript : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
  void Output( gOutput& s ) const;
};


class Child : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
  void Output( gOutput& s ) const;
};

class Write : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
  void Output( gOutput& s ) const;
};

//--------------------------- math operations ---------------------------

class Add : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM &gsm ) const;
  void Output( gOutput& s ) const;
};


class Sub : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM &gsm ) const;
  void Output( gOutput& s ) const;
};


class Concat : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM &gsm ) const;
  void Output( gOutput& s ) const;
};

class Mul : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM &gsm ) const;
  void Output( gOutput& s ) const;
};

class Dot : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM &gsm ) const;
  void Output( gOutput& s ) const;
};

class Div : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM &gsm ) const;
  void Output( gOutput& s ) const;
};

class IntDiv : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM &gsm ) const;
  void Output( gOutput& s ) const;
};


class Neg : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM &gsm ) const;
  void Output( gOutput& s ) const;
};

class Power : public Instruction
{
 public:
  Opcode Type( void) const;
  bool Execute( GSM &gsm ) const;
  void Output( gOutput& s ) const;
};


//----------------------------- integer math operators -------------------


class Mod : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM &gsm ) const;
  void Output( gOutput& s ) const;
};



//----------------------------- relational operators ----------------------

class Equ : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
  void Output( gOutput& s ) const;
};


class Neq : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
  void Output( gOutput& s ) const;
};


class Gtn : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
  void Output( gOutput& s ) const;
};


class Ltn : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
  void Output( gOutput& s ) const;
};


class Geq : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
  void Output( gOutput& s ) const;
};


class Leq : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
  void Output( gOutput& s ) const;
};


//----------------------------- logical operators ------------------------

class AND : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
  void Output( gOutput& s ) const;
};


class OR : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
  void Output( gOutput& s ) const;
};


class NOT : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
  void Output( gOutput& s ) const;
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
  void Output( gOutput& s ) const;
};


class Bind : public Instruction
{
 private:
  gString _FuncName;
 public:
  Bind( const gString& func_name = "" );
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
  void Output( gOutput& s ) const;
};


class BindRef : public Instruction
{
 private:
  gString _FuncName;
 public:
  BindRef( const gString& func_name = "" );
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
  void Output( gOutput& s ) const;
};


class BindVal : public Instruction
{
 private:
  gString _FuncName;
 public:
  BindVal( const gString& func_name = "" );
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
  void Output( gOutput& s ) const;
};


class CallFunction : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
  void Output( gOutput& s ) const;
};


//------------------------- miscellaneous instructions --------------------


class Pop : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
  void Output( gOutput& s ) const;
};


class Display : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
  void Output( gOutput& s ) const;
};


class Dump : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
  void Output( gOutput& s ) const;
};


class Flush : public Instruction
{
 public:
  Opcode Type( void ) const;
  bool Execute( GSM& gsm ) const;
  void Output( gOutput& s ) const;
};




gOutput& operator << ( gOutput& s, Instruction* p );

#endif // GSMINSTR_H
