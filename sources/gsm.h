//#
//# FILE: gsm.h -- definition of GSM (stack machine) type
//#
//# $Id$
//#


#ifndef GSM_H
#define GSM_H



#include "gsmincl.h"
#include "gambitio.h"


class Instruction;
class FuncDescObj;
class CallFuncObj;
class Portion;
class Reference_Portion;
class RefHashTable;
class FunctionHashTable;

class gString;
class gInteger;
class gRational;

template <class T> class gList;
template <class T> class gGrowableStack;


class GSM
{
private:
  gOutput&                         _StdOut;
  gOutput&                         _StdErr;
  
  gGrowableStack< gGrowableStack< Portion* >* >* _StackStack;
  gGrowableStack< CallFuncObj* >*                _CallFuncStack;
  gGrowableStack< RefHashTable* >*               _RefTableStack;
  FunctionHashTable*                             _FuncTable;

  Portion* _ResolveRef             ( Portion* p );
  Portion* _ResolveRefWithoutError ( Reference_Portion* p );

  bool _UnaryOperation  ( OperationMode mode );
  bool _BinaryOperation ( OperationMode mode );

  void _BindCheck ( void ) const;
  bool _BindCheck ( const gString& param_name ) const;

  bool     _VarIsDefined ( const gString& var_name ) const;
  bool     _VarDefine    ( const gString& var_name, Portion* p );
  Portion* _VarValue     ( const gString& var_name ) const;

  static void _ErrorMessage
    (
     gOutput&        s,
     const int       error_num = 0,
     const gInteger& num1      = 0, 
     const gInteger& num2      = 0,
     const gString&  str1      = "",
     const gString&  str2      = "",
     Portion*        por       = 0,
     Instruction*    instr     = 0
     );

  // This function is located in gsmfunc.cc
  void InitFunctions( void );


public:
  GSM( int size, gOutput& s_out = gout, gOutput& s_err = gerr );
  ~GSM();

  int Depth    ( void ) const;
  int MaxDepth ( void ) const;


  bool Push ( const bool&      data );
  bool Push ( const double&    data );
  bool Push ( const gInteger&  data );
  bool Push ( const gRational& data );
  bool Push ( const gString&   data );

  bool PushStream( const gString& data );

  bool PushList ( const int num_of_elements );

  bool PushRef  ( const gString& ref );
  bool Assign   ( void );


  bool Add      ( void );
  bool Subtract ( void );
  bool Multiply ( void );
  bool Divide   ( void );
  bool Negate   ( void );

  bool IntegerDivide ( void );
  bool Modulus       ( void );

  bool EqualTo              ( void );
  bool NotEqualTo           ( void );
  bool GreaterThan          ( void );
  bool LessThan             ( void );
  bool GreaterThanOrEqualTo ( void );
  bool LessThanOrEqualTo    ( void );
  
  bool AND ( void );
  bool OR  ( void );
  bool NOT ( void );
  
  bool Subscript ( void );
  bool Child ( void );
  
  void AddFunction( FuncDescObj* func );

  bool InitCallFunction ( const gString& funcname );
  bool Bind             ( void );
  bool BindVal          ( void );
  bool BindRef          ( void );
  bool Bind             ( const gString& param_name );
  bool BindVal          ( const gString& param_name );
  bool BindRef          ( const gString& param_name );
  bool CallFunction     ( void );

  GSM_ReturnCode Execute( gList<Instruction*>& program, 
			 bool user_func = false );
  Portion* ExecuteUserFunc( gList<Instruction*>& program );
  
  void Output ( void );
  void Dump   ( void );

  bool Pop    ( void );
  void Flush  ( void );
};




#endif  // GSM_H

