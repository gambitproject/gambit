//#
//# FILE: gsminstr.cc -- implementation of Instruction classes for GSM's
//#                      instruction queue subsystem
//#                      companion to GSM
//#
//# $Id$
//#




#include "gmisc.h"
#include "gsminstr.h"
#include "gsm.h"
#include "gambitio.h"
#include "integer.h"
#include "rational.h"



#ifdef MEMCHECK
int Instruction::_NumInstructions = 0;
#endif

Instruction::Instruction( void )
{
#ifdef MEMCHECK
  _NumInstructions++;
  gout << "num of Instructions: " << _NumInstructions << "\n";
#endif

  _LineNumber = 0;
}

Instruction::~Instruction()
{
#ifdef MEMCHECK
  _NumInstructions--;
  gout << "num of Instructions: " << _NumInstructions << "\n";
#endif
}

int& Instruction::LineNumber( void )
{ return _LineNumber; }


//--------------------------------------------------------------------
//                     Descendent Instruction classes
//--------------------------------------------------------------------


//------------------------- null operation --------------------------

Opcode NOP::Type( void ) const
{ return iNOP; }
bool NOP::Execute( GSM& gsm ) const
{ return true; }
void NOP::Output( gOutput& s ) const
{ s << "NOP"; }


//------------------------- branch operations ------------------------


Opcode Quit::Type( void ) const
{ return iQUIT; }
bool Quit::Execute( GSM& gsm ) const
{ return true; }
void Quit::Output( gOutput& s ) const
{ s << "Quit"; }

Opcode Clear::Type( void ) const
{ return iCLEAR; }
bool Clear::Execute( GSM& gsm ) const
{ gsm.Clear(); return true; }
void Clear::Output( gOutput& s ) const
{ s << "Clear"; }


IfGoto::IfGoto( int index )
     : _InstructionIndex( index )
{ }
int IfGoto::WhereTo( void ) const
{ return _InstructionIndex; }
Opcode IfGoto::Type( void ) const
{ return iIF_GOTO; }
bool IfGoto::Execute( GSM& gsm ) const
{ return true; }
void IfGoto::Output( gOutput& s ) const
{ s << "IfGoto( " << _InstructionIndex << " )"; }


Goto::Goto( int index )
     : _InstructionIndex( index )
{ }
int Goto::WhereTo( void ) const
{ return _InstructionIndex; }
Opcode Goto::Type( void ) const
{ return iGOTO; }
bool Goto::Execute( GSM& gsm ) const
{ return true; }
void Goto::Output( gOutput& s ) const
{ s << "Goto( " << _InstructionIndex << " )"; }



//-------------------------  Push operations -------------------------

template <class T> Push<T>::Push( const T& value )
  :_Value( value )
{ }
template <class T> Opcode Push<T>::Type( void ) const
{ return iPUSH; }
template <class T> bool Push<T>::Execute( GSM& gsm ) const
{ return gsm.Push( _Value ); }
template <class T> void Push<T>::Output( gOutput& s ) const
{ s << "Push( " << _Value << " )"; }


PushList::PushList( const int num_elements )
     :_NumElements( num_elements )
{ }
Opcode PushList::Type( void ) const
{ return iPUSHLIST; }
bool PushList::Execute( GSM& gsm ) const
{ return gsm.PushList( _NumElements ); }
void PushList::Output( gOutput& s ) const
{ s << "PushList( " << _NumElements << " )"; }


PushRef::PushRef( const gString& ref )
     :_Ref( ref )
{ }
Opcode PushRef::Type( void ) const
{ return iPUSHREF; }
bool PushRef::Execute( GSM& gsm ) const
{ 
  return gsm.PushRef( _Ref ); 
}
void PushRef::Output( gOutput& s ) const
{ 
  s << "PushRef( \"" << _Ref << "\" )";
}


Opcode Assign::Type( void ) const
{ return iASSIGN; }
bool Assign::Execute( GSM& gsm ) const
{ return gsm.Assign(); }
void Assign::Output( gOutput& s ) const
{ s << "Assign"; }


Opcode UnAssign::Type( void ) const
{ return iUNASSIGN; }
bool UnAssign::Execute( GSM& gsm ) const
{ return gsm.UnAssign(); }
void UnAssign::Output( gOutput& s ) const
{ s << "UnAssign"; }


Opcode Subscript::Type( void ) const
{ return iSUBSCRIPT; }
bool Subscript::Execute( GSM& gsm ) const
{ return gsm.Subscript(); }
void Subscript::Output( gOutput& s ) const
{ s << "Subscript"; }


Opcode Child::Type( void ) const
{ return iCHILD; }
bool Child::Execute( GSM& gsm ) const
{ return gsm.Child(); }
void Child::Output( gOutput& s ) const
{ s << "Child"; }


//--------------------------- math operations ---------------------------

Opcode Add::Type( void ) const
{ return iADD; }
bool Add::Execute( GSM &gsm ) const
{ return gsm.Add(); }
void Add::Output( gOutput& s ) const
{ s << "Add"; }


Opcode Sub::Type( void ) const
{ return iSUB; }
bool Sub::Execute( GSM &gsm ) const
{ return gsm.Subtract(); }
void Sub::Output( gOutput& s ) const
{ s << "Sub"; }


Opcode Mul::Type( void ) const
{ return iMUL; }
bool Mul::Execute( GSM &gsm ) const
{ return gsm.Multiply(); }
void Mul::Output( gOutput& s ) const
{ s << "Mul"; }


Opcode Div::Type( void ) const
{ return iDIV; }
bool Div::Execute( GSM &gsm ) const
{ return gsm.Divide(); }
void Div::Output( gOutput& s ) const
{ s << "Div"; }


Opcode IntDiv::Type( void ) const
{ return iINTDIV; }
bool IntDiv::Execute( GSM &gsm ) const
{ return gsm.IntegerDivide(); }
void IntDiv::Output( gOutput& s ) const
{ s << "Intdiv"; }


Opcode Neg::Type( void ) const
{ return iNEG; }
bool Neg::Execute( GSM &gsm ) const
{ return gsm.Negate(); }
void Neg::Output( gOutput& s ) const
{ s << "Neg"; }

Opcode Power::Type( void ) const
{ return iPOWER; }
bool Power::Execute( GSM &gsm ) const
{ return gsm.Power(); }
void Power::Output( gOutput& s ) const
{ s << "Power"; }



//---------------------------- integer math operators ---------------------


Opcode Mod::Type( void ) const
{ return iMOD; }
bool Mod::Execute( GSM &gsm ) const
{ return gsm.Modulus(); }
void Mod::Output( gOutput& s ) const
{ s << "Mod"; }



//----------------------------- relational operators ----------------------

Opcode Equ::Type( void ) const
{ return iEQU; }
bool Equ::Execute( GSM& gsm ) const
{ return gsm.EqualTo(); }
void Equ::Output( gOutput& s ) const
{ s << "Equ"; }


Opcode Neq::Type( void ) const
{ return iNEQ; }
bool Neq::Execute( GSM& gsm ) const
{ return gsm.NotEqualTo(); }
void Neq::Output( gOutput& s ) const
{ s << "Neq"; }


Opcode Gtn::Type( void ) const
{ return iGTN; }
bool Gtn::Execute( GSM& gsm ) const
{ return gsm.GreaterThan(); }
void Gtn::Output( gOutput& s ) const
{ s << "Gtn"; }


Opcode Ltn::Type( void ) const
{ return iLTN; }
bool Ltn::Execute( GSM& gsm ) const
{ return gsm.LessThan(); }
void Ltn::Output( gOutput& s ) const
{ s << "Ltn"; }


Opcode Geq::Type( void ) const
{ return iGEQ; }
bool Geq::Execute( GSM& gsm ) const
{ return gsm.GreaterThanOrEqualTo(); }
void Geq::Output( gOutput& s ) const
{ s << "Geq"; }


Opcode Leq::Type( void ) const
{ return iLEQ; }
bool Leq::Execute( GSM& gsm ) const
{ return gsm.LessThanOrEqualTo(); }
void Leq::Output( gOutput& s ) const
{ s << "Leq"; }


//----------------------------- logical operators ------------------------

Opcode AND::Type( void ) const
{ return iAND; }
bool AND::Execute( GSM& gsm ) const
{ return gsm.AND(); }
void AND::Output( gOutput& s ) const
{ s << "AND"; }


Opcode OR::Type( void ) const
{ return iOR; }
bool OR::Execute( GSM& gsm ) const
{ return gsm.OR(); }
void OR::Output( gOutput& s ) const
{ s << "OR"; }


Opcode NOT::Type( void ) const
{ return iNOT; }
bool NOT::Execute( GSM& gsm ) const
{ return gsm.NOT(); }
void NOT::Output( gOutput& s ) const
{ s << "NOT"; }


//------------------------ function call operations -------------------

InitCallFunction::InitCallFunction( const gString& func_name )
     :_FuncName( func_name )
{ }
Opcode InitCallFunction::Type( void ) const
{ return iINIT_CALL_FUNCTION; }
bool InitCallFunction::Execute( GSM& gsm ) const
{ return gsm.InitCallFunction( _FuncName ); }
void InitCallFunction::Output( gOutput& s ) const
{ s << "InitCallFunction( \"" << _FuncName << "\" )"; }


Bind::Bind( const gString& func_name )
{ _FuncName = func_name; }
Opcode Bind::Type( void ) const
{ return iBIND; }
bool Bind::Execute( GSM& gsm ) const
{ 
  return gsm.Bind( _FuncName );
}
void Bind::Output( gOutput& s ) const
{
  s << "Bind( " << _FuncName << " )";
}


BindRef::BindRef( const gString& func_name )
{ _FuncName = func_name; }
Opcode BindRef::Type( void ) const
{ return iBINDREF; }
bool BindRef::Execute( GSM& gsm ) const
{ 
  return gsm.BindRef( _FuncName );
}
void BindRef::Output( gOutput& s ) const
{
  s << "BindRef( " << _FuncName << " )";
}


BindVal::BindVal( const gString& func_name )
{ _FuncName = func_name; }
Opcode BindVal::Type( void ) const
{ return iBINDVAL; }
bool BindVal::Execute( GSM& gsm ) const
{ 
  return gsm.BindVal( _FuncName );
}
void BindVal::Output( gOutput& s ) const
{
  s << "BindVal( " << _FuncName << " )";
}


Opcode CallFunction::Type( void ) const
{ return iCALL_FUNCTION; }
bool CallFunction::Execute( GSM& gsm ) const
{ return gsm.CallFunction(); }
void CallFunction::Output( gOutput& s ) const
{ s << "CallFunction"; }


//------------------------- miscellaneous instructions --------------------
  

Opcode Pop::Type( void ) const
{ return iPOP; }
bool Pop::Execute( GSM& gsm ) const
{ return gsm.Pop(); }
void Pop::Output( gOutput& s ) const
{ s << "Pop"; }


Opcode Display::Type( void ) const
{ return iOUTPUT; }
bool Display::Execute( GSM& gsm ) const
{
  gsm.Output();
  return true;
}
void Display::Output( gOutput& s ) const
{ s << "Output"; }


Opcode Dump::Type( void ) const
{ return iDUMP; }
bool Dump::Execute( GSM& gsm ) const
{ 
  gsm.Dump(); 
  return true;
}
void Dump::Output( gOutput& s ) const
{ s << "Dump"; }


Opcode Flush::Type( void ) const
{ return iFLUSH; }
bool Flush::Execute( GSM& gsm ) const
{ 
  gsm.Flush();
  return true;
}
void Flush::Output( gOutput& s ) const
{ s << "Flush"; }


//--------------------------------------------------------------------------




#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__





TEMPLATE class Push< bool >;
TEMPLATE class Push< double >;
TEMPLATE class Push< long >;
TEMPLATE class Push< gRational >;
TEMPLATE class Push< gString >;


#include "glist.imp"

TEMPLATE class gList< Instruction* >;
TEMPLATE class gNode< Instruction* >;



gOutput& operator << ( gOutput& s, Instruction* p )
{
  p->Output( s );
  return s;
}

