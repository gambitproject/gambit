//#
//# FILE: gsminstr.cc -- implementation of Instruction classes for GSM's
//#                      instruction queue subsystem
//#                      companion to GSM
//#
//# $Id$
//#



#include "gsminstr.h"



#include "gambitio.h"

#ifdef MEMCHECK
int Instruction::_NumInstructions = 0;
#endif

Instruction::Instruction( void )
{
#ifdef MEMCHECK
  _NumInstructions++;
  gout << "num of Instructions: " << _NumInstructions << "\n";
#endif
}

Instruction::~Instruction()
{
#ifdef MEMCHECK
  _NumInstructions--;
  gout << "num of Instructions: " << _NumInstructions << "\n";
#endif
}



//--------------------------------------------------------------------
//                     Descendent Instruction classes
//--------------------------------------------------------------------


//------------------------- null operation --------------------------

Opcode NOP::Type( void ) const
{ return iNOP; }
bool NOP::Execute( GSM& gsm ) const
{ return true; }


//------------------------- branch operations ------------------------

IfGoto::IfGoto( int index )
     : _InstructionIndex( index )
{ }
int IfGoto::WhereTo( void ) const
{ return _InstructionIndex; }
Opcode IfGoto::Type( void ) const
{ return iIF_GOTO; }
bool IfGoto::Execute( GSM& gsm ) const
{ return true; }


Goto::Goto( int index )
     : _InstructionIndex( index )
{ }
int Goto::WhereTo( void ) const
{ return _InstructionIndex; }
Opcode Goto::Type( void ) const
{ return iGOTO; }
bool Goto::Execute( GSM& gsm ) const
{ return true; }



//-------------------------  Push operations -------------------------

template <class T> Push<T>::Push( const T& value )
  :_Value( value )
{ }
template <class T> Opcode Push<T>::Type( void ) const
{ return iPUSH; }
template <class T> bool Push<T>::Execute( GSM& gsm ) const
{ return gsm.Push( _Value ); }


PushList::PushList( const int num_elements )
     :_NumElements( num_elements )
{ }
Opcode PushList::Type( void ) const
{ return iPUSHLIST; }
bool PushList::Execute( GSM& gsm ) const
{ return gsm.PushList( _NumElements ); }


PushRef::PushRef( const gString& ref )
     :_Ref( ref )
{ }
Opcode PushRef::Type( void ) const
{ return iPUSHREF; }
bool PushRef::Execute( GSM& gsm ) const
{ return gsm.PushRef( _Ref ); }


Opcode Assign::Type( void ) const
{ return iASSIGN; }
bool Assign::Execute( GSM& gsm ) const
{ return gsm.Assign(); }



Opcode UnAssign::Type( void ) const
{ return iUNASSIGN; }
bool UnAssign::Execute( GSM& gsm ) const
{ gsm.UnAssign(); }


//--------------------------- math operations ---------------------------

Opcode Add::Type( void ) const
{ return iADD; }
bool Add::Execute( GSM &gsm ) const
{ return gsm.Add(); }



Opcode Sub::Type( void ) const
{ return iSUB; }
bool Sub::Execute( GSM &gsm ) const
{ return gsm.Subtract(); }


Opcode Mul::Type( void ) const
{ return iMUL; }
bool Mul::Execute( GSM &gsm ) const
{ return gsm.Multiply(); }


Opcode Div::Type( void ) const
{ return iDIV; }
bool Div::Execute( GSM &gsm ) const
{ return gsm.Divide(); }


Opcode Neg::Type( void ) const
{ return iNEG; }
bool Neg::Execute( GSM &gsm ) const
{ return gsm.Negate(); }




//---------------------------- integer math operators ---------------------

Opcode IntDiv::Type( void ) const
{ return iINT_DIV; }
bool IntDiv::Execute( GSM &gsm ) const
{ return gsm.IntegerDivide(); }

Opcode Mod::Type( void ) const
{ return iMOD; }
bool Mod::Execute( GSM &gsm ) const
{ return gsm.Modulous(); }



//----------------------------- relational operators ----------------------

Opcode EQU::Type( void ) const
{ return iEQU; }
bool EQU::Execute( GSM& gsm ) const
{ return gsm.EqualTo(); }


Opcode NEQ::Type( void ) const
{ return iNEQ; }
bool NEQ::Execute( GSM& gsm ) const
{ return gsm.NotEqualTo(); }


Opcode GTN::Type( void ) const
{ return iGTN; }
bool GTN::Execute( GSM& gsm ) const
{ return gsm.GreaterThan(); }


Opcode LTN::Type( void ) const
{ return iLTN; }
bool LTN::Execute( GSM& gsm ) const
{ return gsm.LessThan(); }


Opcode GEQ::Type( void ) const
{ return iGEQ; }
bool GEQ::Execute( GSM& gsm ) const
{ return gsm.GreaterThanOrEqualTo(); }


Opcode LEQ::Type( void ) const
{ return iLEQ; }
bool LEQ::Execute( GSM& gsm ) const
{ return gsm.LessThanOrEqualTo(); }


//----------------------------- logical operators ------------------------

Opcode AND::Type( void ) const
{ return iAND; }
bool AND::Execute( GSM& gsm ) const
{ return gsm.AND(); }


Opcode OR::Type( void ) const
{ return iOR; }
bool OR::Execute( GSM& gsm ) const
{ return gsm.OR(); }


Opcode NOT::Type( void ) const
{ return iNOT; }
bool NOT::Execute( GSM& gsm ) const
{ return gsm.NOT(); }


//------------------------ function call operations -------------------

InitCallFunction::InitCallFunction( const gString& func_name )
     :_FuncName( func_name )
{ }
Opcode InitCallFunction::Type( void ) const
{ return iINIT_CALL_FUNCTION; }
bool InitCallFunction::Execute( GSM& gsm ) const
{ return gsm.InitCallFunction( _FuncName ); }


Bind::Bind( void )
{ _FuncName = ""; }
Bind::Bind( const gString& func_name )
{ _FuncName = func_name; }
Opcode Bind::Type( void ) const
{ return iBIND; }
bool Bind::Execute( GSM& gsm ) const
{ 
  if( _FuncName == "" )
    return gsm.Bind();
  else
    return gsm.Bind( _FuncName );
}


Opcode CallFunction::Type( void ) const
{ return iCALL_FUNCTION; }
bool CallFunction::Execute( GSM& gsm ) const
{ return gsm.CallFunction(); }


//------------------------- miscellaneous instructions --------------------
  
Opcode Output::Type( void ) const
{ return iOUTPUT; }
bool Output::Execute( GSM& gsm ) const
{
  gsm.Output();
  return true;
}


Opcode Dump::Type( void ) const
{ return iDUMP; }
bool Dump::Execute( GSM& gsm ) const
{ 
  gsm.Dump(); 
  return true;
}


Opcode Flush::Type( void ) const
{ return iFLUSH; }
bool Flush::Execute( GSM& gsm ) const
{ 
  gsm.Flush();
  return true;
}


//--------------------------------------------------------------------------



#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__



TEMPLATE class Push< bool >;
TEMPLATE class Push< double >;
TEMPLATE class Push< gInteger >;
TEMPLATE class Push< gRational >;
TEMPLATE class Push< gString >;



#include "glist.imp"

TEMPLATE class gList< Instruction* >;
TEMPLATE class gNode< Instruction* >;

