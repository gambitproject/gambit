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


UnAssign::UnAssign( const gString& ref )
     :_Ref( ref )
{ }
Opcode UnAssign::Type( void ) const
{ return iUNASSIGN; }
bool UnAssign::Execute( GSM& gsm ) const
{ gsm.UnAssign( _Ref ); }


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


//----------------------------- relational operators ----------------------

Opcode EqualTo::Type( void ) const
{ return iEQUAL_TO; }
bool EqualTo::Execute( GSM& gsm ) const
{ return gsm.EqualTo(); }


Opcode NotEqualTo::Type( void ) const
{ return iNOT_EQUAL_TO; }
bool NotEqualTo::Execute( GSM& gsm ) const
{ return gsm.NotEqualTo(); }


Opcode GreaterThan::Type( void ) const
{ return iGREATER_THAN; }
bool GreaterThan::Execute( GSM& gsm ) const
{ return gsm.GreaterThan(); }


Opcode LessThan::Type( void ) const
{ return iLESS_THAN; }
bool LessThan::Execute( GSM& gsm ) const
{ return gsm.LessThan(); }


Opcode GreaterThanOrEqualTo::Type( void ) const
{ return iGREATER_THAN_OR_EQUAL_TO; }
bool GreaterThanOrEqualTo::Execute( GSM& gsm ) const
{ return gsm.GreaterThanOrEqualTo(); }


Opcode LessThanOrEqualTo::Type( void ) const
{ return iLESS_THAN_OR_EQUAL_TO; }
bool LessThanOrEqualTo::Execute( GSM& gsm ) const
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


//--------------------------- string operations ------------------------

Opcode Concatenate::Type( void ) const
{ return iCONCATENATE; }
bool Concatenate::Execute( GSM& gsm ) const
{ return gsm.Concatenate(); }


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

