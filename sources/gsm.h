//#
//# FILE: gsm.h -- definition of GSM (stack machine) type
//#
//# $Id$
//#


#ifndef GSM_H
#define GSM_H



#include "gsmincl.h"
#include "gambitio.h"
#include "gsmfunc.h"

class Instruction;
class FuncDescObj;
class CallFuncObj;
class Portion;
class Reference_Portion;
class RefHashTable;
class FunctionHashTable;

class gString;
class gRational;

template <class T> class gList;
template <class T> class gGrowableStack;


class GSM
{
private:
  static int _NumObj;

  gInput&  _StdIn;
  gOutput& _StdOut;
  gOutput& _StdErr;

  Portion* _OUTPUT;
  Portion* _INPUT;
  Portion* _NULL;

  gGrowableStack< gGrowableStack< Portion* >* >* _StackStack;
  gGrowableStack< CallFuncObj* >*                _CallFuncStack;
  gGrowableStack< RefHashTable* >*               _RefTableStack;
  FunctionHashTable*                             _FuncTable;

  Portion* _ResolveRef             ( Portion* p );

  bool _UnaryOperation  ( const gString& funcname );
  bool _BinaryOperation ( const gString& funcname );

  void _BindCheck ( void ) const;
  bool _Bind ( const gString& param_name ) const;

  bool     _VarIsDefined ( const gString& var_name ) const;
  bool     _VarDefine    ( const gString& var_name, Portion* p );
  Portion* _VarValue     ( const gString& var_name ) const;
  Portion* _VarRemove    ( const gString& var_name );

  int      _Depth( void ) const;
  void     _Push( Portion* p );
  Portion* _Pop( void );

  static void _ErrorMessage
    (
     gOutput&        s,
     const int       error_num = 0,
     const long& num1      = 0, 
     const long& num2      = 0,
     const gString&  str1      = "",
     const gString&  str2      = ""
     );

  // This function is located in gsmfunc.cc
  void InitFunctions( void );


public:
  GSM( int size = 10, 
      gInput&  s_in  = gin, 
      gOutput& s_out = gout, 
      gOutput& s_err = gerr );
  ~GSM();

  int Depth    ( void ) const;
  int MaxDepth ( void ) const;


  Portion*& DefaultNfg( void );
  Portion*& DefaultEfg( void );



  bool Push ( const bool&      data );
  bool Push ( const long&      data );
  bool Push ( const double&    data );
  bool Push ( const gRational& data );
  bool Push ( const gString&   data );

  bool PushList ( const int num_of_elements );

  bool PushRef  ( const gString& ref );
  bool Assign   ( void );
  bool UnAssign ( void );


  bool Add      ( void );
  bool Subtract ( void );
  bool Multiply ( void );
  bool Dot      ( void );
  bool Divide   ( void );
  bool Negate   ( void );
  bool Power    ( void );

  bool Concat   ( void );

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
  
  bool AddFunction( FuncDescObj* func );

  bool InitCallFunction ( const gString& funcname );
  bool Bind           ( const gString& param_name = "" );
  bool BindVal        ( const gString& param_name = "" );
  bool BindRef        ( const gString& param_name = "", 
		       bool auto_val_or_ref = false );
  bool CallFunction   ( void );

  int Execute( gList<Instruction*>& program, 
	      bool user_func = false );
  Portion* ExecuteUserFunc( gList<Instruction*>& program, 
			   const FuncInfoType& func_info,
			   Portion** param );
  
  void Output ( void );
  void Dump   ( void );

  bool Pop    ( void );
  void Flush  ( void );
  void Clear  ( void );
};




#endif  // GSM_H

